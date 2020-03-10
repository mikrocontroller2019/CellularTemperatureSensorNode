# Cellular Temperature Sensor Node
This project shows how to connect an ESP32 board to the Internet using an additional SIM800L board and a SIM card data plan.

## Description
This project shows how to connect an ESP32 board to the Internet using an additional SIM800L board and a SIM card data plan.

The MCU board used for this device is a **ESP32 NodeMCU Module WiFi Development Board with CP2102**. To connect to the internet via cellular network a **SIM800L GSM GPRS Modul** with Antenna is used and the senor for temperature meassuring is a waterproof encapsuled **DS18B20** 1-wire digital temperatur sensor.

The read temperature values are published to a *MQTT* broker.

After publishing the sensor reading the GSM modem is put into power safe mode and the ESP32 is set to deep sleep mode for a specified period of time.

## BOM
* ESP32 NodeMCU Module WiFi Development Board with CP2102
* SIM800L GSM GPRS Modul
* DS18B20 1-wire digital temperatur sensor, waterproof, 3m cable
* Breadboard(s) & jumper wires
* SIM with data plan

## Libraries
Besides the Arduino core for the ESP32  the following libraries are required:

* OneWire: Access 1-wire temperature sensors, memory and other chips.
* DallasTemperature: Arduino Library for Dallas Temperature ICs
* TinyGSM:  A small Arduino library for GSM modules.
* PubSubClient: A client library for MQTT messaging.

These libraries can be installed using the Arduino IDE's board manager and library manager.

## Configuration
Configure the following lines in **CellularTemperatureSensorNode.ino** according to information specified by your SIM data plan provider:
```C
// GPRS: APN, user & password ... e.g. for VF-UK
const char *GPRS_APN = "pp.vodafone.co.uk";
const char *GPRS_USER = "wap";
const char *GPRS_PASSWORD = "wap";
```

Configure the following lines for access to your MQTT broker
```C
#define MQTT_CLIENT_ID "CTSN001"
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_SERVER "mqtt.eclipse.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC "/ACME/DevTest/Sensors"
```

The wake-up / publishing interval time can be configured by changing the value for TIME_TO_SLEEP (seconds):
```C
#define TIME_TO_SLEEP 60 
```

## Schema
See file **CellularTemperatureSensorNode.fzz** for details.

![alt text](https://raw.githubusercontent.com/mikrocontroller2019/CellularTemperatureSensorNode/master/CellularTemperatureSensorNode_bb.png "Schema")
