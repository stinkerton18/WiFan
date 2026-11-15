# WiFan[cite: 1]

An ESP32 Smart Controller for 3-speed Whole House/Attic Fans[cite: 1]

## 1. Purpose[cite: 1]

The controller options for many whole house/attic fans are extremely poor/limited[cite: 1]. At *best*, the only one I could find was limited to Bluetooth instead of a semi-proprietary sub-Gigahertz range RF controller[cite: 1]. What’s worse is buying a second or replacement controller runs $100 or more and still requires you to climb up into the attic to press a pairing button to resync/pair the controller[cite: 1]. Definitely not ideal given how dead simple these fans really are[cite: 1]. That’s when it struck me. If I can’t buy a solution, then we’ll build one![cite: 1]

## 2. Requirements[cite: 1]

* **WiFi** – This is a **MUST**[cite: 1]. No proprietary wireless controllers/frequencies/Zigbee bullshit, etc[cite: 1].
* **Webpage Support** – Allow for controlling/configuring directly without forcing “Smart” methods (like apps, Amazon Echo/Apple HomeKit/Google Home devices, etc.)[cite: 1]
* **Wireless Provisioning Method** – “Install, turn on, get out.” I don’t want there to be any reason why you have to climb back into the attic outside catastrophic failure[cite: 1]. So all setup/configuration and resetting has to be done through the web interface[cite: 1]
* **Maker Friendly** – Custom/bespoke PCBs are great, but having to get 5 PCBs and convince 4 of your friends to buy one also for something that is a “build/use just one” controller is not worth it IMHO[cite: 1]. Thus the need for designing it to be easy to buy/assemble just 1[cite: 1].
* **Source Part Model Agnostic** – Where ever possible, parts/components need to be generic enough that it isn’t difficult to source a replacement from any manufacturer to serve the same purpose[cite: 1].

## 3. Design[cite: 1]

The heart of WiFan is an ESP-32D, AKA the ESP-WROOM-32[cite: 1]. It supports 2.4Ghz WiFi, has 4MB of flash, is low cost and supports enough of the Matter library set for Smart home Integration[cite: 1]. I’m using this one from Amazon: [https://www.amazon.com/dp/B0DF2YJSHN](https://www.amazon.com/dp/B0DF2YJSHN)[cite: 1]

Along with this are 4 normally open 30A high power relays with low level triggers[cite: 1]. The fan’s speed is controlled by connecting the incoming HOT/Line AC wire to the appropriate speed wire[cite: 1]. So the relays are needed to connect HOT/Line AC (when triggered) to the appropriate wire on the fan motor[cite: 1]. Why 4 relays if we’re only controlling 3 speeds? The 4th is actually wired to a simple latching circuit to kill the 5v power and allow a full power cycle of the ESP32 rather than just a code reboot[cite: 1].

In addition, it’ll need a 5v DC power supply, and if the relays are not able to be triggered with 3.3v, then a 5v/3.3v level shifter between the relay actuator and ESP32 GPIO lines[cite: 1].

Fortunately, I found a dual relay module (on Amazon) that not only fits the power requirements but also provides 5v DC with AC input: [https://www.amazon.com/dp/B077W1NVLM](https://www.amazon.com/dp/B077W1NVLM)[cite: 1]. Two of these modules provide us with relays we need and redundant power for the ESP32[cite: 1].

Since ESP32’s support micro-Python and Arduino C code, I used the default Arduino C code and the Matter library for home automation support[cite: 1]. This allows for WiFan to be detected and controlled as 3 different “Smart Plugs” for High, Medium, and Low speeds[cite: 1].

## 4. Wiring Diagram[cite: 1]

For the 3-speed Quiet’N’Cool fan, the following is the wire mapping -> speed diagram[cite: 1]:

| Function / Speed | Wire Color |
| :--- | :--- |
| Common Neutral[cite: 1] | White Wire[cite: 1] |
| Common Ground[cite: 1] | Green/Bare Wire[cite: 1] |
| Fast Speed[cite: 1] | Black[cite: 1] |
| Medium Speed[cite: 1] | Yellow[cite: 1] |
| Slow Speed[cite: 1] | Red[cite: 1] |

The common colors, for US 110/120v AC, are as follows[cite: 1]:

| Function | Wire Color |
| :--- | :--- |
| Hot/Line AC[cite: 1] | Black[cite: 1] |
| Neutral[cite: 1] | White[cite: 1] |
| Ground[cite: 1] | Green/Bare Wire[cite: 1] |