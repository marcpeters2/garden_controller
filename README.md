# Garden Controller

A device that controls the environment of plants.  Code is targeted for the Arduino MKR1000.

## Developing
- If you don't have one, get an Arduino MKR1000
- __Important:__ Make sure that the MKR1000 has up-to-date firmware for the WiFi module (WINC1501 Model B).  At time of writing, the latest firmware is version 19.5.2.  When the device is running an earlier firmware version, errors have been observed: the device will sporadically lose WiFi connectivity and the software will be unable to send HTTP requests. 
- Download the Arduino IDE.  We used version 1.8.3.

Download the following additional code libraries using the Arduino Library Manager: 
- __WiFi__ version 1.2.7
- __WiFi101__ version 0.14.3
