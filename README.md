Improved version of AirGradient Arduino Library for ESP8266 (Wemos D1 MINI) with MQTT support
=====================================================================================================

This is improved fork of official [airgradienthq/arduino](https://github.com/airgradienthq/arduino) repo this version **enables** Plantower PMS5003 sensor capabilities  
**to report PM1, PM2.5 and PM10 measurments** which are not used by AirGradient project by default.  
This fork uploads to MQTT as opposed to HTTP.
  
This fork also fixes:
* PMS5003 bug. AirGradient sensor PM2.5 reports -1 value
* CO2 (RCO2) sensor bug. AirGradient sensor for CO2 reports -1 value
* CO2 (RCO2) sensor bug. AirGradient sensor for CO2 reports 65278 value
* add temperature adjustment value

Thats how this bugs looks on the graphs:  
If you have same kind of reporting from your AirGradient - go with this version.  

Installation
------------
You have to replace included AirGradient.cpp and AirGradient.h libraries in your Arduino.app Libraries directory.
ALso install https://github.com/marvinroger/async-mqtt-client

Then flash included C02_PM1_PM2_PM10_SHT_OLED_WIFI_MATT.ino as new AirGradient board firmware after customizing the WIFI and MQTT settings

AirGradient Arduino Library for ESP8266 (Wemos D1 MINI)
=====================================================================================================

Build your own low cost air quality sensor with optional display measuring PM2.5, CO2, Temperature and Humidity. 

This library makes it easy to read the sensor data from the Plantower PMS5003 PM2.5 sensor, the Senseair S8 and the SHT30/31 Temperature and Humidity sensor. Visit our DIY section for detailed build instructions and PCB layout.

https://www.airgradient.com/open-airgradient/instructions/
