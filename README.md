# hx711_to_mqtt
Send hx711 sensor data to an mqtt server using ESP8266 over wifi

The sensor that I used in this project is a 5X 0-40Kpa Air Pressure Sensor Module Water Level and LIquid Level Sensor Controller Board that I attached to a nodeMCU that uses the ESP8266.  

The reason I created the project was to allow my water tanks and river water levels to be monitored remotely over wifi.  This was to prevent my pumps turning on when when there is no water in the tank or river to prevent them from burning out. 

The unit is remote with no wireing and is run from a small solar panel to provide it power.  That is why the code is setup to save power when the battery gets lower by putting the esp8266 into deep sleep mode when needed.

for deep sleep support you will need to add a jumper from D0 to RST

As noted in the code the HX711 sensor is connected to D2 (GPIO4) to DOUT and D1 (GPIO5) to SCK.  Also I get the 3v power from the nodeMCU to power the HX711.

I also later setup support to setup wifi credentials over wifi.
to setup wifi credentials power up the unit and then hit the flash button for about 10 secounds.  This should put the unit into station
mode.  you can then connect to the wifi station directly then from a browser go to 192.168.4.1 that should bring up the wifi settings page.  When that's done hopefully it has programed EEPROM with your credetials for the next boot of the device.

I had planed to add more of the settings in the wifi setup like the mqtt broker and such that is now setup in the arduino_secrets.h file.  maybe when I have time I will add that feature or others can push the change to me and I'll add it.

I'm also not sure how the calibration on this sensor works if it will be similar on other devices.  I just tested one of them in a swimming pool that was about 1.8M deep to get the values I now have programed into the arduino_secrets.h file

OK have fun good luck.
