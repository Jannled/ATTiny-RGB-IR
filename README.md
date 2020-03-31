# ATTiny-RGB-IR
This Arduino-Project controls an RGB-LED with an IR-remote (NEC-protocol) you may have laying around. I am using an TSOP4838 as the IR-receiver and an ATTiny85 because of its small price and size.

To use this Projekt simply use [IRemote's IrecvDemo](https://github.com/z3t0/Arduino-IRremote) example sketch to get your button codes and place them in the definitions of this program and you are ready to upload.

## History
1. First I went with the [IRemote](https://github.com/z3t0/Arduino-IRremote) library, but I had issues getting it to run on the ATTiny, beeing it the size of the library or some timer magic I'm not sure...
2. Then I went with the [tiny_IRremote](https://gist.github.com/SeeJayDee/caa9b5cc29246df44e45b8e7d1b1cdc5) library, but then I discovered Timer1 is occupied by the library so it can't be used for PWM. So its up to No. 3
3. Writing my own library with pin-change interrupts for the NEC protocol (which is the one my cheap RGB-strip remote uses)

## Used core for the ATTiny85
This Sketch is using SpenceKonde's [ATTinyCore](https://github.com/SpenceKonde/ATTinyCore) in order for the Arduino-IDE to support Atmels tiny Microcontroller. I used the following settings:

Setting           | Value            | Description
------------------|------------------|-----------------------------------------------
Chip              | ATTiny85         |   
Clock             | 8 MHz (internal) |   
B.O.D. Level      | B.O.D. Disabled  | Brownout-Detector
Save EEPROM       | EEPROM Retained  | EEPROM used for storing data between restarts
Timer 1 Clock     | CPU              |  
LTO               | Enabled          | Link Time Optimizations
millis()/micros() | Enabled          | Free one time by disabling them
