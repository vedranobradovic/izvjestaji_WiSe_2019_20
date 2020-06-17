#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

typedef enum
{
  READ_SERIAL,
  READ_SENSORS,
  UPLOAD,
  SLEEP,
} StateType;

Adafruit_BME680 bme; // I2C

unsigned long delayTime;

// Parametri mreze
const char *ssid = "107755";     //upisi naziv wifi mreze
const char *pass = "1y3iz41srf"; //upisi sifru wifi-ja

// ThingSpeak informacije
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 1067808;      //upisi kanal odnosno id kanala na thingspeaku
char *readAPIKey = "6G23L6NXBJGILZNC";  //api key za citat
char *writeAPIKey = "NX7OUFYQ3PNP96IT"; //api key za pisat

const unsigned long postingInterval = 120L * 1000L;
unsigned int dataFieldOne = 1;   //  Polje za upisivat temperaturu
unsigned int dataFieldTwo = 2;   //  Polje za upisivat vlažnost
unsigned int dataFieldThree = 3; //  Polje za upisivat tlak

float temp;
float hum;
float press;

unsigned long lastConnectionTime = 0;
long lastUpdateTime = 0;
WiFiClient client;

float readTSData(long TSChannel, unsigned int TSField)
{

  float data = ThingSpeak.readFloatField(TSChannel, TSField, readAPIKey);
  Serial.println(" Data read from ThingSpeak: " + String(data, 9));
  return data;
}

// Use this function if you want to write a single field
int writeTSData(long TSChannel, unsigned int TSField, float data)
{
  int writeSuccess = ThingSpeak.writeField(TSChannel, TSField, data, writeAPIKey); // Write the data to the channel
  if (writeSuccess)
  {

    Serial.println(String(data) + " written to Thingspeak.");
  }

  return writeSuccess;
}

//use this function if you want multiple fields simultaneously
int write2TSData(long TSChannel, unsigned int TSField1, float field1Data, unsigned int TSField2, long field2Data, unsigned int TSField3, long field3Data)
{

  ThingSpeak.setField(TSField1, field1Data);
  ThingSpeak.setField(TSField2, field2Data);
  ThingSpeak.setField(TSField3, field3Data);

  int writeSuccess = ThingSpeak.writeFields(TSChannel, writeAPIKey);
  return writeSuccess;
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println(F("BME680 async test"));

  if (!bme.begin())
  {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1)
      ;
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();
  Serial.println("Start");
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid); // Display confirmation msg to PC
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Display IP Address to PC
  ThingSpeak.begin(client);
}

StateType state = READ_SERIAL;

void loop()
{
  switch (state)
  {
  case READ_SERIAL:
    state = READ_SENSORS;
    break;
  case READ_SENSORS:
    temp = bme.readTemperature();
    hum = bme.readHumidity();
    press = bme.readPressure() / 100.0;
    state = UPLOAD;
    break;
  case UPLOAD:
    write2TSData(channelID, dataFieldOne, temp, dataFieldTwo, hum, dataFieldThree, press); //upisivanje podataka na ThingSpeak
    state = SLEEP;
    break;
  case SLEEP:
    ESP.deepSleep(900e6);
    break;
  }
}