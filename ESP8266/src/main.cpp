#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

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
unsigned int aField = 6;
unsigned int bField = 7;
unsigned int cField = 8;

// Globalne variable
float aConst;
float bConst;
float cConst;
float temp;
float hum;
float press;
float gass;

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
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();
  Serial.println("Start");
  WiFi.begin(ssid, pass); // Func to Connect to WiFi

  while (WiFi.status() != WL_CONNECTED)
  { // Wait for connection
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid); // Display confirmation msg to PC
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Display IP Address to PC
  ThingSpeak.begin(client);

  // Read the constants at startup.
  aConst = readTSData(channelID, aField);
  bConst = readTSData(channelID, bField);
  cConst = readTSData(channelID, cField);
}

void loop()
{
  unsigned long endTime = bme.beginReading();
  if (endTime == 0)
  {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  Serial.println(F("You can do other work during BME680 measurement."));
  delay(50); // This represents parallel work.
  // There's no need to delay() until millis() >= endTime: bme.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.

  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme.endReading())
  {
    Serial.println(F("Failed to complete reading :("));
    return;
  }

  int i;

  lastUpdateTime = millis();
  temp = bme.readTemperature();
  hum = bme.readHumidity();
  press = bme.readPressure() / 100.0;
  //gass = bme.gas_resistance() / 1000.0;

  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  Serial.print(F("Temperature = "));
  Serial.print(temp);
  Serial.println(F(" *C"));

  Serial.print(F("Pressure = "));
  Serial.print(press);
  Serial.println(F(" hPa"));

  Serial.print(F("Humidity = "));
  Serial.print(hum);
  Serial.println(F(" %"));

  Serial.print(F("Gas = "));
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(F(" KOhms"));

  Serial.println();
  write2TSData(channelID, dataFieldOne, temp, dataFieldTwo, hum, dataFieldThree, press);
  delay(20000);
}