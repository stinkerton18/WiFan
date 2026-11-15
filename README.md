# WiFan

An ESP32 Smart Controller for 3-speed Whole House/Attic Fans

## 1. Purpose

The controller options for many whole house/attic fans are extremely poor/limited. At *best*, the only one I could find was limited to Bluetooth instead of a semi-proprietary sub-Gigahertz range RF controller. What’s worse is buying a second or replacement controller runs $100 or more and still requires you to climb up into the attic to press a pairing button to resync/pair the controller. Definitely not ideal given how dead simple these fans really are. That’s when it struck me. If I can’t buy a solution, then we’ll build one!

## 2. Requirements

* **WiFi** – This is a **MUST**. No proprietary wireless controllers/frequencies/Zigbee bullshit, etc.
* **Webpage Support** – Allow for controlling/configuring directly without forcing “Smart” methods (like apps, Amazon Echo/Apple HomeKit/Google Home devices, etc.)
* **Wireless Provisioning Method** – “Install, turn on, get out.” I don’t want there to be any reason why you have to climb back into the attic outside catastrophic failure. So all setup/configuration and resetting has to be done through the web interface
* **Maker Friendly** – Custom/bespoke PCBs are great, but having to get 5 PCBs and convince 4 of your friends to buy one also for something that is a “build/use just one” controller is not worth it IMHO. Thus the need for designing it to be easy to buy/assemble just 1.
* **Source Part Model Agnostic** – Where ever possible, parts/components need to be generic enough that it isn’t difficult to source a replacement from any manufacturer to serve the same purpose.

## 3. Design

The heart of WiFan is an ESP-32D, AKA the ESP-WROOM-32. It supports 2.4Ghz WiFi, has 4MB of flash, is low cost and supports enough of the Matter library set for Smart home Integration. I’m using this one from Amazon: [https://www.amazon.com/dp/B0DF2YJSHN](https://www.amazon.com/dp/B0DF2YJSHN)

Along with this are 4 normally open 30A high power relays with low level triggers. The fan’s speed is controlled by connecting the incoming HOT/Line AC wire to the appropriate speed wire. So the relays are needed to connect HOT/Line AC (when triggered) to the appropriate wire on the fan motor. Why 4 relays if we’re only controlling 3 speeds? The 4th is actually wired to a simple latching circuit to kill the 5v power and allow a full power cycle of the ESP32 rather than just a code reboot.

In addition, it’ll need a 5v DC power supply, and if the relays are not able to be triggered with 3.3v, then a 5v/3.3v level shifter between the relay actuator and ESP32 GPIO lines.

Fortunately, I found a dual relay module (on Amazon) that not only fits the power requirements but also provides 5v DC with AC input: [https://www.amazon.com/dp/B077W1NVLM](https://www.amazon.com/dp/B077W1NVLM). Two of these modules provide us with relays we need and redundant power for the ESP32.

Since ESP32’s support micro-Python and Arduino C code, I used the default Arduino C code and the Matter library for home automation support. This allows for WiFan to be detected and controlled as 3 different “Smart Plugs” for High, Medium, and Low speeds.

## 4. Wiring Diagram

For the 3-speed Quiet’N’Cool fan, the following is the wire mapping -> speed diagram:

| Function / Speed | Wire Color |
| :--- | :--- |
| Common Neutral | White Wire |
| Common Ground | Green/Bare Wire |
| Fast Speed | Black |
| Medium Speed | Yellow |
| Slow Speed | Red |

The common colors, for US 110/120v AC, are as follows:

| Function | Wire Color |
| :--- | :--- |
| Hot/Line AC | Black |
| Neutral | White |
| Ground | Green/Bare Wire |