# BusStation_Hackathon2017

Arduino projects:
1) bus 		: arduino Nano + Neo GPS + RMF95 LoRa module
	
	dependencies:
		#include <lmic.h>
		#include <hal/hal.h>
		#include <SPI.h>			-- for LoRa Module
		
		#include <SoftwareSerial.h>
		#include <TinyGPS.h>		-- for GPS Module

2)station 	: esp8266 + lcd display

	dependencies:
		CP210x UART driver - https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers
		#include <LiquidCrystal_I2C.h> - LCD 16x4 lines 
		#include <ESP8266WiFi.h>  - for wifi stuff
		#include <ArduinoJson.h>  - Json parser, but had to add some strtok for parsing an JSON array of objects (? another solution)
		

BOM:

GPS		https://www.optimusdigital.ro/en/gps/105-modul-gps-gy-neo6mv2.html
Nano	https://www.optimusdigital.ro/en/arduino-compatible-boards/1686-placa-de-dezvoltare-compatibila-cu-arduino-nano-atmega328p-i-ch340.html
esp		https://www.optimusdigital.ro/ro/placi-cu-wifi/266-placa-de-dezvoltare-wifi-nodemcu.html
LoRa	https://www.optimusdigital.ro/en/lora/2152-modul-radio-lora-adafruit-rfm95w-868-mhz.html
	
Lcd		https://www.optimusdigital.ro/ro/optoelectronice-lcd-uri/639-lcd-2004-cu-backlight-galben-verde.html
i2c		https://www.optimusdigital.ro/ro/interfaa/89-adaptor-i2c-pentru-lcd-1602.html		