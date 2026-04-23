# Planning and Initial Design
_Time_Spent_
3hrs

# Overview
This project is a version of a home security. I wanted to make something useful so the problem this project hopes to solve is the lack of environmental awareness, especially when it comes to unwanted intrusions. Think things like bank vaults, offices e.t.c that are only opened at scheduled time, this system should allow for quick alerts to inform about any disturbance through light and motion sensor modules that detect any sudden changes in the environment.

The project includes:

PIR motion sensor module: For detecting sudden movements (obviously)

![pir](https://stasis.hackclub-assets.com/images/1776751408235-39sc1d.png)

A list of other essential components that i dont need to add to the BOM because they are already available are:

-ESP32 C3 mini: The brains of the project, controls everthing else

-LDR light sensor: For detecting sudden light changes based on a given threshold

-LEDs: Visual feedback through flashes based on the type of trigger

-Buzzer: Audio feedback through different beep patterns based on the trigger type

-OLED: Visual display through text feedback

-Jumper wires: Connections on the breadboard

-Breadboard: For connnecting all the components

It is going to have two major modes which are ARM, DISARM. It also features bluetooth control for switching between modes.

The main idea of the project is that it should be an autonomous security system that continously scans the area for any sudden movements and/or light changes, then gives feedback in form of buzzer alerts, OLED messages and led blinks based on the event. It should have two major modes for the first version, ARM and DISARM which can be toggled through bluetooth. When in the ARMED mode, it does everything stated above by comparing the recieved data with the set trigger thresholds, and basically executes an action if the threshold is crossed. In the DISARMED mode, the system is basically on idle. It still recieves data from the snesors but does not trigger any alerts. And then the ALERT state, it is the state the system switches to when it was triggered in the ARMED state. This state is basically what controls the output devices (LEDs, OLED and stuff) Together these states control the entire system

I designed the light change detection to work with a set threshold value instead of the absolute intensity of the light. This is because, well, setting it to trigger from a particular value just doesnt make sense since the system should should function for security, a small light value would be able to bypass it that way, so obviously a no.

I dont really have any fitting image to add here as an additional so... illl just leave it like that



# Circuit and Hardware Design
_Time_Spent_
2hrs

# Overview
After I was done with the whole system planning phase, I started the process of simulating the complete circuit connections of my system. I connected the the different components to different pins on the esp32. The only mapping that I had to really consider was the analog output of the LDR, since i learned that not all pins on a microcontroller can read analog data, so i connected it to the esp 0 pin. After that I connected the OLED, buzzer, PIR e.t.c to the other pins.

![Wiring](https://stasis.hackclub-assets.com/images/1776927091031-yk0ylw.png)

After i was done with the initial connections, i decided to test it, you know, to make sure everything was wired properly, thats where i encountered my first issue, the leds refused to turn on. At first i thought i wired them wrongly, or maybe the resistor had some sort of unknown issue, but i had another project i had made with leds that works fine and wired this one exactly like it so theres no way there could be any issue with the wiring. I adjusted the resistance of the resistors, changed the orientation of the leds, tried everything i could but nothing worked. Then i saw it, I wasnt even that the wiring had any issue technically, but i totally forgot to connect the esp 3.3v pin and gnd pin to the breadboard, so it was basically a simple thing i forgot that caused all the problems.

Anyways... after i fixed that, I proceeded to test the system as a whole, it worked well... except that i wasnt able to test how it responds to commands while running. I thought the software i was using just didnt have an input panel, but i was wrong, it was there, hiding, blending in to the background, I had to go search where it is online.



# Firmware Planning
_Time_Spent_
2.5hrs

# Overview
In this session i designed the firmware structure. I knew that i would want the system to respond differently based on the type of event or environment, so decided to break the operating logic into states, that do different things based on their pre defined settings.

The main idea is that the system should continously read the environmental data and perform an action based on the data it recieves from the sensors. I used the loop function for this

![Screenshot 2026-04-23 081452](https://stasis.hackclub-assets.com/images/1776928535912-j8ou9h.png)

When the system is triggered in the ARMED state, It then activates the alert state which handles the output devices, while the ARMED state handles the sensor logic and data. The sensor handling logic works differently for the two sensors. The PIR sensor detects movement basically. The LDR is what required a bit more logic to it. Instead of just responding to light intensities at a set value, it works by measuring the change in light intensity from the baseline value you give it. That way it can work pretty well in almost any environment you put it, technically...

I also made sure that the alerts state is only triggered only when the device was previously armed. The alert state is the one that actually handles the alerts and stuff

![Screenshot 2026-04-23 081521](https://stasis.hackclub-assets.com/images/1776929340763-deul90.png)

The system also has bluetooth integration, which allows for mode switching from a remote device. It also has two other commands which are RESET and RECALIBRATE. RESET returns the system back to ARMED state if it was already triggered. But if you think about it, returning it to its original state when the environment hasnt changed will just trigger it again, so thats why i added RECALIBRATE. This one would change the baseline values for triggering alerts from the sensors, so it can work in the new environment, and when you use RESET to return the values to normal too.



# Final System and Firmware Test
_Time_Spent_
1.5hrs

# Overview
This was the final stage of the system design. I decided to test the fully integrated system and see how well the whole thing functions.

First, I tested the motion detection system. I simulated movement within the software, the the sensor captured the change, the change was displayed on the OLED and the system switched to the alert state, which triggered the led and buzzer to kick in.

![Screenshot 2026-04-23 085331](https://stasis.hackclub-assets.com/images/1776944512465-bxip31.png)

I also tested the light sensor by adjusting the light intensity in that simulation. The sensor functioned as expected too, triggering the state change and alerts, and since the state change is triggered automatically, i was also successfully able to test that too. The LEDs also worked perfectly. The blue led indicated thst the system was UNARMED, reading the environment but not doing anything, it then switches to the green led when switched to ARMED through the command window, which represents how it will work through bluetooth btw. Red flashes when motion is detected amd yellow turns on when there is a light change, all with the buzzer doing its thing the whole time. RECALIBRATE and RESET instructions also work fine. There might be some other features i want to add but ill leave that till i actually build the device, then ill decide. And with that im basically done.
