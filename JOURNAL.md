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
