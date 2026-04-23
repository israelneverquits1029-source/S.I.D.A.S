#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NimBLEDevice.h>

// =========================
// Pin mapping (ESP32-C3)
// =========================
#define OLED_SDA       8
#define OLED_SCL       9

#define PIR_PIN        4
#define LDR_PIN        0   // AO from LDR module

#define LED_GREEN      1
#define LED_RED        2
#define LED_BLUE       3
#define LED_YELLOW     5

#define BUZZER_PIN     6

// =========================
// OLED
// =========================
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// =========================
// BLE UART-style service
// =========================
static BLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID rxUUID     ("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID txUUID     ("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

NimBLEServer* bleServer = nullptr;
NimBLECharacteristic* txChar = nullptr;
bool bleConnected = false;

// =========================
// System states
// =========================
enum SecurityState {
  STATE_DISARMED,
  STATE_ARMED,
  STATE_ALERT
};

SecurityState currentState = STATE_DISARMED;

// =========================
// Sensor / environment state
// =========================
int currentLight = 0;
int baselineLight = 0;
int originalBaselineLight = 0;

bool motionDetected = false;
bool lightTamperDetected = false;

// Threshold for sudden light change
int lightChangeThreshold = 350;

// =========================
// UI / logging
// =========================
int currentPage = 0; // 0=main, 1=sensors, 2=events
String lastCommand = "NONE";
String lastEvent = "BOOT";
String lastError = "NONE";

// =========================
// Timing
// =========================
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastBlinkToggle = 0;
unsigned long alertStartTime = 0;

const unsigned long SENSOR_INTERVAL = 200;
const unsigned long DISPLAY_INTERVAL = 300;
const unsigned long BLINK_INTERVAL = 250;

// =========================
// Alert behavior
// =========================
bool redLedBlinkState = false;
bool buzzerPulseState = false;

// =========================
// Command buffer
// =========================
String pendingCommand = "";
bool commandReady = false;

// =========================
// BLE callbacks
// =========================
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer) {
    bleConnected = true;
    lastEvent = "BLE connected";
  }

  void onDisconnect(NimBLEServer* pServer) {
    bleConnected = false;
    lastEvent = "BLE disconnected";
    NimBLEDevice::startAdvertising();
  }
};

class RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (!value.empty()) {
      pendingCommand = String(value.c_str());
      pendingCommand.trim();
      commandReady = true;
    }
  }
};

// =========================
// Utility
// =========================
String stateToString(SecurityState s) {
  switch (s) {
    case STATE_DISARMED: return "DISARMED";
    case STATE_ARMED:    return "ARMED";
    case STATE_ALERT:    return "ALERT";
    default:             return "UNKNOWN";
  }
}

void sendMessage(const String& msg) {
  Serial.println(msg);
  if (bleConnected && txChar) {
    txChar->setValue(msg.c_str());
    txChar->notify();
  }
}

void beepShort(int ms = 60) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void setAllLEDsOff() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_YELLOW, LOW);
}

void updateLEDsNormal() {
  setAllLEDsOff();

  if (currentState == STATE_DISARMED) {
    digitalWrite(LED_BLUE, HIGH);
  } else if (currentState == STATE_ARMED) {
    digitalWrite(LED_GREEN, HIGH);
    if (lightTamperDetected) {
      digitalWrite(LED_YELLOW, HIGH);
    }
  }
}

void updateAlertOutputs() {
  unsigned long now = millis();

  if (now - lastBlinkToggle >= BLINK_INTERVAL) {
    lastBlinkToggle = now;
    redLedBlinkState = !redLedBlinkState;
    buzzerPulseState = !buzzerPulseState;
  }

  setAllLEDsOff();
  digitalWrite(LED_RED, redLedBlinkState ? HIGH : LOW);

  if (lightTamperDetected) {
    digitalWrite(LED_YELLOW, HIGH);
  }

  digitalWrite(BUZZER_PIN, buzzerPulseState ? HIGH : LOW);
}

void stopAlertOutputs() {
  digitalWrite(BUZZER_PIN, LOW);
  redLedBlinkState = false;
  buzzerPulseState = false;
}

void captureBaseline() {
  baselineLight = analogRead(LDR_PIN);
  originalBaselineLight = baselineLight;
  lastEvent = "Baseline captured";
}

void restoreOriginalBaseline() {
  baselineLight = originalBaselineLight;
  lastEvent = "Baseline restored";
}

void readSensors() {
  currentLight = analogRead(LDR_PIN);
  motionDetected = (digitalRead(PIR_PIN) == HIGH);

  int lightDelta = abs(currentLight - baselineLight);
  lightTamperDetected = (lightDelta > lightChangeThreshold);
}

void enterAlert(const String& reason) {
  if (currentState != STATE_ALERT) {
    currentState = STATE_ALERT;
    alertStartTime = millis();
    lastEvent = reason;
    sendMessage("ALERT: " + reason);
  }
}

void processSystemLogic() {
  if (currentState == STATE_ARMED) {
    if (motionDetected) {
      enterAlert("Motion detected");
    } else if (lightTamperDetected) {
      enterAlert("Light tamper detected");
    }
  }
}

void sendStatus() {
  String msg = "STATE=" + stateToString(currentState) +
               " LIGHT=" + String(currentLight) +
               " BASE=" + String(baselineLight) +
               " PIR=" + String(motionDetected ? "1" : "0") +
               " TAMPER=" + String(lightTamperDetected ? "1" : "0");
  sendMessage(msg);
}

void sendHelp() {
  sendMessage("Commands: ARM, DISARM, STATUS, RESET, RECALIBRATE, PAGE NEXT, PAGE PREV, HELP");
}

void armSystem() {
  currentState = STATE_ARMED;
  stopAlertOutputs();
  captureBaseline();
  lastEvent = "System armed";
  lastError = "NONE";
  sendMessage("System armed");
}

void disarmSystem() {
  currentState = STATE_DISARMED;
  stopAlertOutputs();
  lightTamperDetected = false;
  motionDetected = false;
  lastEvent = "System disarmed";
  lastError = "NONE";
  sendMessage("System disarmed");
}

void resetSystem() {
  stopAlertOutputs();
  restoreOriginalBaseline();
  lightTamperDetected = false;
  motionDetected = false;
  currentState = STATE_ARMED;
  lastEvent = "Reset to armed";
  lastError = "NONE";
  sendMessage("Reset complete: armed");
}

void recalibrateSystem() {
  captureBaseline();
  lightTamperDetected = false;
  motionDetected = false;
  if (currentState == STATE_ALERT) {
    currentState = STATE_ARMED;
    stopAlertOutputs();
  }
  lastEvent = "Recalibrated";
  lastError = "NONE";
  sendMessage("Baseline recalibrated");
}

void parseCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  Serial.println("CMD RECEIVED: " + cmd);

  if (cmd.length() == 0) return;

  lastCommand = cmd;

  if (cmd == "HELP") {
    sendHelp();
  } else if (cmd == "STATUS") {
    sendStatus();
  } else if (cmd == "ARM") {
    armSystem();
  } else if (cmd == "DISARM") {
    disarmSystem();
  } else if (cmd == "RESET") {
    resetSystem();
  } else if (cmd == "RECALIBRATE") {
    recalibrateSystem();
  } else if (cmd == "PAGE NEXT") {
    currentPage = (currentPage + 1) % 3;
    lastEvent = "Page next";
    sendMessage("Page changed");
  } else if (cmd == "PAGE PREV") {
    currentPage = (currentPage + 2) % 3;
    lastEvent = "Page prev";
    sendMessage("Page changed");
  } else {
    lastError = "Invalid command";
    lastEvent = "Command error";
    beepShort(120);
    sendMessage("ERROR: Invalid command");
  }
}

void pollSerialCommands() {
  static String serialBuffer = "";

  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 0) {
        pendingCommand = serialBuffer;
        pendingCommand.trim();
        commandReady = true;
        serialBuffer = "";
      }
    } else {
      serialBuffer += c;
    }
  }
}

// =========================
// OLED pages
// =========================
void drawPage0() {
  display.setCursor(0, 0);
  display.println("Security Status");

  display.setCursor(0, 14);
  display.print("State: ");
  display.println(stateToString(currentState));

  display.setCursor(0, 26);
  display.print("Motion: ");
  display.println(motionDetected ? "YES" : "NO");

  display.setCursor(0, 38);
  display.print("Tamper: ");
  display.println(lightTamperDetected ? "YES" : "NO");

  display.setCursor(0, 52);
  display.print("Pg 1/3");
}

void drawPage1() {
  display.setCursor(0, 0);
  display.println("Sensor Data");

  display.setCursor(0, 14);
  display.print("Light: ");
  display.println(currentLight);

  display.setCursor(0, 26);
  display.print("Base : ");
  display.println(baselineLight);

  display.setCursor(0, 38);
  display.print("Delta: ");
  display.println(abs(currentLight - baselineLight));

  display.setCursor(0, 52);
  display.print("Pg 2/3");
}

void drawPage2() {
  display.setCursor(0, 0);
  display.println("Events");

  display.setCursor(0, 14);
  display.print("Cmd: ");
  display.println(lastCommand.substring(0, 14));

  display.setCursor(0, 30);
  display.print("Evt: ");
  display.println(lastEvent.substring(0, 14));

  display.setCursor(0, 46);
  display.print("Err: ");
  display.println(lastError.substring(0, 14));
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (currentPage == 0) drawPage0();
  else if (currentPage == 1) drawPage1();
  else drawPage2();

  display.display();
}

// =========================
// Setup
// =========================
void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);

  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  setAllLEDsOff();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) {}
  }
  display.clearDisplay();
  display.display();

  NimBLEDevice::init("C3 Security Hub");
  bleServer = NimBLEDevice::createServer();
  bleServer->setCallbacks(new ServerCallbacks());

  NimBLEService* service = bleServer->createService(serviceUUID);

  NimBLECharacteristic* rxChar =
      service->createCharacteristic(rxUUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  rxChar->setCallbacks(new RxCallbacks());

  txChar = service->createCharacteristic(txUUID, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
  txChar->setValue("Security hub ready");

  service->start();

  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(serviceUUID);
  advertising->start();

  readSensors();
  captureBaseline();
  updateLEDsNormal();

  beepShort(100);
  lastEvent = "System ready";
  updateDisplay();
  sendMessage("Security hub ready");
}

// =========================
// Loop
// =========================
void loop() {
  if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = millis();

    readSensors();
    processSystemLogic();

    if (currentState == STATE_ALERT) {
      updateAlertOutputs();
    } else {
      stopAlertOutputs();
      updateLEDsNormal();
    }
  }

  pollSerialCommands();

  if (commandReady) {
    commandReady = false;
    parseCommand(pendingCommand);
    pendingCommand = "";
  }

  if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL) {
    lastDisplayUpdate = millis();
    updateDisplay();
  }
}
