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
