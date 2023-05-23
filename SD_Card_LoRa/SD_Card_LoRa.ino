#include <SPI.h>
#include <SD.h>
#include <LoRa.h>
#include <Arduino_MKRENV.h>
#include <TinyGPS++.h>

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"

int counter = 0;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
#define ss Serial1

// chip select for SD card
const int SD_CS_PIN = 4;  // internal sd card


// file object
File dataFile;

/* RTC */

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void displayInfo()
{
  LoRa.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    LoRa.print(gps.location.lat(), 6);
    LoRa.print(F(","));
    LoRa.print(gps.location.lng(), 6);
  }
  else
  {
    LoRa.print(F("INVALID"));
  }
}


void setup() {
  Serial.begin(9600);
  ss.begin(9600);
  while (!Serial);
  LoRa.setPins(A0, 2, A1);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    LoRa.setFrequency(433.0);
    while (1);
  }

    if (!ENV.begin()) {

    LoRa.println("Failed to initialize MKR ENV shield!");

    while (1);
  }

  // initialize SPI:
  SPI.begin();
  delay(100); 

  // RTC 
  #ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
  #endif
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

    // init SD card
  if(!SD.begin(SD_CS_PIN)) {
    Serial.println("Failed to initialize SD card!");
    while (1);
  }
  

if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print(" ");
  LoRa.print("Send Packet: ");
  
  /*Analog Pins*/
  //reads which sensor that will be detected
  int UVSensor = analogRead(A6);
  int temp = analogRead(A5);

  
  // init the logfile
  dataFile = SD.open("result.txt", FILE_WRITE);
  delay(1000);


    // read the sensors values
  float temperature = ENV.readTemperature();

  float humidity    = ENV.readHumidity();

  float pressure    = ENV.readPressure();

  float illuminance = ENV.readIlluminance();


  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.3V)
  float UVIndex = UVSensor * (3.3 / 1023.0);

    //  Temperature Calculation
  temp = temp * 0.48828125;     //turn the temperature to a degree percentage
  temp = temp * 3.3/5;    // voltage calculation

  // Pressure to Height Calculations 
  float Pa = pressure*1000;
  float height = (76 - (Pa/1333))*100;

  //  RTC Calculation Date
  DateTime now = rtc.now();
  
  LoRa.println(counter);

  LoRa.println("  ");
  
  LoRa.print(now.year(), DEC);
  LoRa.print('/');
  LoRa.print(now.month(), DEC);
  LoRa.print('/');
  LoRa.print(now.day(), DEC);
  LoRa.print(" (");
  LoRa.print(daysOfTheWeek[now.dayOfTheWeek()]);
  LoRa.print(") ");
  LoRa.print(now.hour(), DEC);
  LoRa.print(':');
  LoRa.print(now.minute(), DEC);
  LoRa.print(':');
  LoRa.print(now.second(), DEC);
  
  //space 
  LoRa.println("  ");
  LoRa.println("  ");

  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }

  LoRa.println(" ");

  //Temperature Shield
  LoRa.print("Shield Temperature = ");
  LoRa.print(temperature);

  LoRa.println(" C");
  
  //Humidity
  LoRa.print("Humidity    = ");

  LoRa.print(humidity);

  LoRa.println(" %");

  //Pressure
  LoRa.print("Pressure    = ");

  LoRa.print(pressure);

  LoRa.println(" kPa");

  //Luminance
  LoRa.print("Illuminance = ");

  LoRa.print(illuminance);

  LoRa.println(" lx");

  //Height
  LoRa.print("Height = ");

  LoRa.print(height);

  LoRa.println(" m");

  //UV Index
  LoRa.print("UV Index = ");
  
  LoRa.println(UVIndex / .1);

  LoRa.endPacket();
  /*****SD CARD PRINT*****/
/***********************/

  //space 
  dataFile.println("  ");

  //lines
  dataFile.println("///////////////////////////////");
  dataFile.println("  ");
  
  dataFile.print(now.year(), DEC);
  dataFile.print('/');
  dataFile.print(now.month(), DEC);
  dataFile.print('/');
  dataFile.print(now.day(), DEC);
  dataFile.print(" (");
  dataFile.print(daysOfTheWeek[now.dayOfTheWeek()]);
  dataFile.print(") ");
  dataFile.print(now.hour(), DEC);
  dataFile.print(':');
  dataFile.print(now.minute(), DEC);
  dataFile.print(':');
  dataFile.print(now.second(), DEC);
  
  //space 
  dataFile.println("  ");
  dataFile.println("  ");

  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
  
  if (gps.encode(ss.read()))
      dataFile.print(F("Location: ")); 
      
  if (gps.location.isValid())
  {
    dataFile.print(gps.location.lat(), 6);
    dataFile.print(F(","));
    dataFile.print(gps.location.lng(), 6);
  }
  else
  {
    dataFile.print(F("INVALID"));
  }

  dataFile.println(" ");
  dataFile.println(" ");

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    dataFile.println(F("No GPS detected: check wiring."));
    while(true);
  }
  
  //Temperature Shield
  dataFile.print("Shield Temperature = ");

  dataFile.print(temperature);

  dataFile.println(" C");

  //RTC Temperature
  dataFile.print("RTC Temperature = ");

  dataFile.print(rtc.getTemperature());

  dataFile.println(" C");

  //Outer Temperature
  dataFile.print("Outer Temperature = ");
  
  dataFile.print(temp);
  
  dataFile.println(" C");

  //space 
  dataFile.println("  ");
  
  //Average Temperature
  dataFile.print("Average Temperature = ");
  
  dataFile.print((temp + temperature + rtc.getTemperature()) / 3);

  dataFile.println("C");
  
  //Morse LED
  dataFile.print ("Status: ");
  
  if(temp <= 10)
  {
    dataFile.println("-10");
  }
  
  else if( (temp > 20) && ( temp <= 30) )
  {
    dataFile.println("20+");
  }
  
  else if( (temp > 30) && ( temp <= 40) )
  {
    dataFile.println("30+");
  }

  else if ( (temp > 40) && ( temp <= 50) )
  {
    dataFile.println("40+");
  }

  else if ( (temp > 50) && ( temp <= 60) )
  {
    dataFile.println("50+");
  }

  else if ( (temp > 60) && ( temp <= 70) )
  {
    dataFile.println("60+");
  }

  else if ( (temp > 70) && ( temp <= 80) )
  {
    dataFile.println("70+");
  }

  else if ( (temp > 80) && ( temp <= 90) )
  {
    dataFile.println("80+");

  }

  else if (temp > 90)
  {
    dataFile.println("50+");

  }

  
  //space 
  dataFile.println(" ");
  
  //Humidity
  dataFile.print("Humidity    = ");

  dataFile.print(humidity);

  dataFile.println(" %");

  //Pressure
  dataFile.print("Pressure    = ");

  dataFile.print(pressure);

  dataFile.println(" kPa");

  //Luminance
  dataFile.print("Illuminance = ");

  dataFile.print(illuminance);

  dataFile.println(" lx");

  //Height
  dataFile.print("Height = ");

  dataFile.print(height);

  dataFile.println(" m");

  //UV Index
  dataFile.print("UV Index = ");
  
  dataFile.println(UVIndex / .1);

  counter++;
  /*
    if (dataFile) {
    
    Serial.println("Read:");
    // Reading the whole file
    while (dataFile.available() ) {
      Serial.write(dataFile.read());
   }
   */

  // close the file
  dataFile.close();
  
  // wait 1 second to print again
  delay(1000);
}
