/*
   DATALOGGER WORKSHOP @ KHM KÖLN
   BY CHRISTIAN DOELLER – APRIL 2022

   SENSOR CONNECTIONS

   DHT - Hum / Temp Sensor

   D2 ---> SIG
   5V ---> +
   GND ---> -

   LIGHT SENSOR (VOLTAGE DIVIDER)

   5V ---> SensorLeg1
   SensorLeg2 ---> resistorLeg1
   GND ---> resistorLeg2
   A0 ---> SensorLeg2/resistorLeg1

   SOUND SENSOR

   5V ---> pin +
   GND ---> pin -
   A1 --> pin S

   GAS SENSOR

   GND ---> GND
   VCC ---> 5V
   A2 ---> A-OUT

*/

// Libraries
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
#include <dht.h>

// SD CARD
const int chipSelect = 10; // cs pin
File myFile;

// LOGGER
long delayTime = 1000; // ******************************************************************** VARIABLE
String fileName = "log"; // plz choose very short here *************************************** VARIABLE
int counter = 0;

// RTC MODULE
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = { // 2d Array of char for weekdays
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

//                                 *** CHOOSE YOUR SENSOR
boolean temperatureSensor = false;
boolean soundSensor = false;
boolean gasSensor = false;
boolean lightSensor = true;

//                                 *** CHECK YOUR PINS
// Temperature
dht DHT;
#define DHT11_PIN    2
// Light
#define lightSensorPin    A0
// Sound
#define soundSensorPin    A1
// Gas
#define gasSensorPin    A2

// SWITCH
const int sw = 6;
boolean newReading = true;

// ********************************************* SETUP
void setup() {
  // SERIAL
  Serial.begin (9600);

  // * PINMODES
  pinMode (sw, INPUT_PULLUP); // switch
  pinMode (LED_BUILTIN, OUTPUT); // blink LED

  // * SETUP RTC MODULE
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1);
  }
  Serial.println ("RTC found");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // automatically sets the RTC to the date & time on PC
  Serial.print ("Time set to ");
  Serial.println (getDateTime());

  // * INITIALIZE SD MODULE
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  // check if sensor chosen, if not, stop and send error message
  if (!temperatureSensor && !gasSensor && !lightSensor && !soundSensor) {
    while (true) {
      Serial.println ("error: no sensor");
    }
  }
}

// ********************************************* LOOP
void loop() {
  // * CHECK SWITCH
  if (digitalRead (sw) == LOW) { // if switched
    // * CHECK IF FIRST READING
    // if yes, create file with a new file name
    // if not, save readings to file
    if (newReading == true) {
      // check if file already exists
      // if exists, make filename with new counter number
      while (SD.exists((fileName + String(counter) + ".csv"))) {
        Serial.println ("file already exists.");
        counter ++; // increase counter number by 1
      }
      // print new file name
      Serial.print ("new file name: ");
      Serial.println (fileName + String(counter) + ".csv");
      // create the new file
      myFile = SD.open((fileName + String(counter) + ".csv"), FILE_WRITE); // make or open file in directory to write in
      // save information about the reading in two columns of csv file only once in the beginning
      // datetime, delayTime-temperatureSensorgasSensor-soundSensor-lightSensor –– for example: "datetime, 1000-0-0-0-1"
      // this tells the python code what the delayTime was and which sensor has been used.
      if (myFile) { // if the file is available, write to it:
        String indicator = ("datetime," + String(delayTime) + "-" + String(temperatureSensor) + "-" + String(gasSensor) + "-" + String(soundSensor) + "-" + String(lightSensor));
        myFile.println(indicator);
        myFile.close();
        Serial.println (indicator);
      }
      else { // if the file isn't open, pop up an error:
        Serial.println("error first opening " + fileName);
      }
      newReading = false; // set newReading to false as new file was created and information has been saved
    } else { // if the reading session was already going, save new data to file
      // *** NOT FIRST READING
      // *** GET AND SAVE DATA
      String timeDate = getDateTime ();  // make a string and insert time & date                   *** GET TIME + DATE
      String sensorData = " "; // make a string and insert sensor data                             *** GET SENSOR DATA
      if (temperatureSensor) {
        sensorData = getTempSensorData ();
      }
      else if (lightSensor) {
        sensorData = getLightSensorData ();
      }
      else if (soundSensor) {
        sensorData = getSoundSensorData ();
      }
      else if (gasSensor) {
        sensorData = getGasSensorData ();
      } else {
        sensorData = "no sensor connected";
      }
      // * WRITE DATA TO FILE
      myFile = SD.open((fileName + String(counter) + ".csv"), FILE_WRITE); // make or open file in directory to write in
      if (myFile) { // if the file is available, write to it:
        myFile.print(timeDate);
        myFile.println(sensorData);
        myFile.close();
        Serial.print ("data saved: ");
        Serial.print(timeDate);
        Serial.println(sensorData);
      }
      else { // if the file isn't open, pop up an error:
        Serial.println("error opening " + fileName);
      }
      // * DELAY FOR NEXT READING AND BLINK
      digitalWrite (LED_BUILTIN, HIGH); // led on arduino board on
      delay(delayTime); // delay 1 seconds
      digitalWrite (LED_BUILTIN, LOW); // led on arduino board off
    }
  } else { // if switch is not connected
    newReading = true; // reset newReading to make new file when switched again
    Serial.println ("Switch not connected."); // print switch state
  }
}

// ********************************************* FUNCTIONS
String getDateTime () {
  DateTime now = rtc.now();
  // make always 2 digits
  String h, m, s;
  int int_h = now.hour();
  int int_m = now.minute();
  int int_s = now.second();
  if (int_h < 10) {
    h = "0" + String(int_h);
  } else {
    h = String(int_h);
  }
  if (int_m < 10) {
    m = "0" + String(int_m);
  } else {
    m = String(int_m);
  }
  if (int_s < 10) {
    s = "0" + String(int_s);
  } else {
    s = String(int_s);
  }
  // compose date and time Stirng to be saved
  //String _dateTime = String(now.year(), DEC) + "/" + String(now.month(), DEC) + "/" + String(now.day(), DEC) + " " + String(daysOfTheWeek[now.dayOfTheWeek()]) + " " + h + ":" + m + ":" + s;
  //String _dateTime = String(now.year(), DEC) + "-" + String(now.month(), DEC) + "-" + String(now.day(), DEC) + " " + h + "-" + m + "-" + s;
  String _dateTime = String(now.year(), DEC) + "/" + String(now.month(), DEC) + "/" + String(now.day(), DEC) + " " + h + ":" + m + ":" + s;
  return _dateTime;
}

// Get TEMPERATURE Sensor Data
String getTempSensorData () {
  DHT.read11(DHT11_PIN);
  // make String for data saving
  // --> "temperatur = xxxx humidity = xxxx"
  //String _humTempData = ("temperature = " + String(DHT.temperature, 1) + " humidity = " + String(DHT.humidity, 1));
  //String _humTempData = ("temperature=" + String(DHT.temperature, 1));
  String _humTempData = ("," + String(DHT.temperature, 1));
  return _humTempData;
}

// Get LIGHT Sensor Data
String getLightSensorData () {
  int _lightReading = analogRead (lightSensorPin);
  //String _lightData = ("lightData=" + String(_lightReading));
  String _lightData = ("," + String(_lightReading));
  return _lightData;
}

// Get SOUND Sensor Data
String getSoundSensorData () {
  // very simple filter: take highest value of 50 measured values
  int maxVal = 0;
  int currVal = 0;
  for (int i = 0; i < 50; i++) { // measure 50 times
    currVal = analogRead (soundSensorPin); // measure current value
    // compare with (until now) highest value
    if (currVal > maxVal) { // if current value higher than  highest value
      maxVal = currVal;  // take current value as highest value
    }
  }
  int _soundReading = maxVal;
  //String _soundData = ("soundData=" + String(_soundReading));
  String _soundData = ("," + String(_soundReading));
  return _soundData;
}

// Get GAS Sensor Data
String getGasSensorData () {
  int _gasReading = analogRead (gasSensorPin);
  //String _gasData = ("gasData=" + String(_gasReading));
  String _gasData = ("," + String(_gasReading));
  return _gasData;
}
