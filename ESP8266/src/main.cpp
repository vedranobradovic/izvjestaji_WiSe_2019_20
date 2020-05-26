#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

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
float aConst; //definiranje tipa konstanti
float bConst;
float cConst;

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
  Serial.println(F("BME280 test"));

  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin();
  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();
  Serial.begin(9600);
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

  int i;

  lastUpdateTime = millis();

  write2TSData(channelID, dataFieldOne, bme.readTemperature(), dataFieldTwo, bme.readHumidity(), dataFieldThree, bme.readPressure());

  delay(1000);
}