/***
 *** Celluar Temperature Sensor Node
 ***/

/*
   DEBUG
*/
#define DEBUG
#include "Debug2Serial.h"

/*
   GSM: SIM800L
*/
// GSM modem
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
HardwareSerial SerialGSM(1);
TinyGsm gsmModem(SerialGSM);
TinyGsmClient gsmClient(gsmModem);
const int SERIAL_GSM_BAUD_RATE = 9600;
const int SERIAL_GSM_RX_PIN = 16;
const int SERIAL_GSM_TX_PIN = 17;
// GPRS: APN, user & password ... e.g. for VF-UK
const char *GPRS_APN = "pp.vodafone.co.uk";
const char *GPRS_USER = "wap";
const char *GPRS_PASSWORD = "wap";

/*
   MQTT
*/
#include <PubSubClient.h>
#define MQTT_CLIENT_ID "CTSN001"
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_SERVER "mqtt.eclipse.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC "/ACME/DevTest/Sensors"
PubSubClient mqttClient(MQTT_SERVER, MQTT_PORT, gsmClient);

/*
   DS18B20
*/
#include <OneWire.h>
#include <DallasTemperature.h>
// Data wire is connected to GPIO4 on the ESP32
#define ONE_WIRE_DATA 4
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_DATA);
// Pass the oneWire reference to Dallas Temperature.
DallasTemperature ds18b20(&oneWire);

/*
   Operational
*/
#define RESTART_DELAY 10000

/*
   ESP32 deep sleep
*/
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60       /* Time ESP32 will go to sleep (in seconds) */

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : DEBUG_SERIAL_PRINTLN("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : DEBUG_SERIAL_PRINTLN("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : DEBUG_SERIAL_PRINTLN("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : DEBUG_SERIAL_PRINTLN("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : DEBUG_SERIAL_PRINTLN("Wakeup caused by ULP program"); break;
    default : DEBUG_SERIAL_PRINTF("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

void deep_sleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  DEBUG_SERIAL_PRINTLN("Setup ESP32 wakeup timer to " + String(TIME_TO_SLEEP) + " seconds.");
  DEBUG_SERIAL_PRINTLN("Going to sleep NOW.");
  delay(1000);
  DEBUG_SERIAL_FLUSH();
  esp_deep_sleep_start();
}


/*
   GPRS
*/
void initializeGPRS()
{
  DEBUG_SERIAL_PRINTLN("Initializing GSM serial ...");
  SerialGSM.begin(SERIAL_GSM_BAUD_RATE, SERIAL_8N1, SERIAL_GSM_RX_PIN, SERIAL_GSM_TX_PIN, false);
  delay(3000);
  DEBUG_SERIAL_PRINTLN(gsmModem.getModemInfo());

  DEBUG_SERIAL_PRINTLN("Restarting modem ...");
  if (!gsmModem.restart())
  {
    DEBUG_SERIAL_PRINTLN("Modem restart failed!");
    delay(RESTART_DELAY);
    ESP.restart();
    return;
  }
  DEBUG_SERIAL_PRINTLN("Modem started successfully.");

  DEBUG_SERIAL_PRINTLN("Modem: Deactivate SlowClock ...");
  gsmModem.sendAT(GF("+CSCLK=0")); // de-activate SlowClock
  gsmModem.sendAT(GF("+CSCLK=0")); // de-activate SlowClock
  DEBUG_SERIAL_PRINTLN("Modem: SlowClock deactivated.");

  DEBUG_SERIAL_PRINTLN("Aquiring cellular network ....");
  if (!gsmModem.waitForNetwork())
  {
    DEBUG_SERIAL_PRINTLN("Failed to aquire cellular network!");
    delay(RESTART_DELAY);
    ESP.restart();
    return;
  }
  DEBUG_SERIAL_PRINTLN("Cellular network OK");

  DEBUG_SERIAL_PRINTLN("Connecting (GPRS) ....");
  if (!gsmModem.gprsConnect(GPRS_APN, GPRS_USER, GPRS_PASSWORD))
  {
    DEBUG_SERIAL_PRINTLN("GPRS connection failed!");
    delay(RESTART_DELAY);
    ESP.restart();
    return;
  }
  DEBUG_SERIAL_PRINTLN("GPRS connection established!");
}

void shutdownGPRS()
{
  // Initialize serial SIM800L
  DEBUG_SERIAL_PRINTLN("Shutting down modem ...");
  DEBUG_SERIAL_PRINTLN("Disconnecting GPRS ...");
  gsmModem.gprsDisconnect();
  DEBUG_SERIAL_PRINTLN("GPRS disconnected.");
  DEBUG_SERIAL_PRINTLN("Turning of radio ...");
  gsmModem.radioOff();
  DEBUG_SERIAL_PRINTLN("Modem: radio: off.");
  DEBUG_SERIAL_PRINTLN("Modem: Activate SlowClock ...");
  gsmModem.sendAT(GF("+CSCLK=0"));
  DEBUG_SERIAL_PRINTLN("Modem: SlowClock activated.");
  delay(1000);
  DEBUG_SERIAL_PRINTLN("Modem shutdown successfully.");
}

/*
   MQTT
*/
void connectToMqttServer() {
  DEBUG_SERIAL_PRINTLN("Connect to MQTT broker ...");
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
    DEBUG_SERIAL_PRINTLN("Connected to MQTT broker.");
  } else {
    DEBUG_SERIAL_PRINTLN("Failed to connected to MQTT broker!");
    DEBUG_SERIAL_PRINTLN("MQTT client error follows ...");
    DEBUG_SERIAL_PRINTLN(mqttClient.state());
    delay(RESTART_DELAY);
    ESP.restart();
  }
}

void publishMqtt(String message)
{
  DEBUG_SERIAL_PRINTLN("Publish message (\"" + message + "\") ...");
  String topic = String(MQTT_TOPIC) + String("/") + String(MQTT_CLIENT_ID);
  int status = mqttClient.publish(topic.c_str(), message.c_str());
  DEBUG_SERIAL_PRINTLN("MQTT client status: " + String(status));
}

/*
   Temperature
*/
void initializeSensorDs18b20() {
  DEBUG_SERIAL_PRINTLN("Initializing DS18B20 sensor ...");
  ds18b20.begin();
  DEBUG_SERIAL_PRINTLN("DS18B20 sensor initialized.");
}

String getCelciusTempeatureAsString()
{
  DEBUG_SERIAL_PRINTLN("Request temperature reading from DS18B20 sensor ...");
  ds18b20.requestTemperatures();
  DEBUG_SERIAL_PRINTLN("Read celsius temperature from DS18B20 sensor ...");
  float celsius = ds18b20.getTempCByIndex(0);
  if ( celsius == -127.00) {
    DEBUG_SERIAL_PRINTLN("ERROR: Failed to get temperature reading from DS18B20 sensor!");
    return "n/a";
  } else {
    char celsiusChar[10];
    dtostrf(celsius, 5, 2, celsiusChar);
    String celsiusString = celsiusChar;
    DEBUG_SERIAL_PRINTLN("Temperature reading from DS18B20: " + celsiusString + " *C");
    return celsiusString;
  }
}

/*
   JSON
*/
String createSingleAttributeJson(String attribute, String value)
{
  String json = "{";
  json += "\"" + attribute + "\":";
  json += "\"" + value + "\"";
  json += "}";
  return json;
}

String createSingleFloatAttributeJson(String attribute, float value)
{
  String json = "{";
  if (!isnan(value))
  {
    json += "\"" + attribute + "\":";
    json += String(value, 2);
  }
  json += "}";
  return json;
}

/*
   Arduino entry points
*/
// setup
void setup() {
  DEBUG_SERIAL_INIT(115200);
  print_wakeup_reason();
  initializeGPRS();
  connectToMqttServer();
  initializeSensorDs18b20();
  String temperatureString = getCelciusTempeatureAsString();
  String message = createSingleAttributeJson("temperature", temperatureString);
  publishMqtt(message);
  shutdownGPRS();
  deep_sleep();
}

// loop
void loop() {
  // void
}
