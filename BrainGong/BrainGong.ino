//===----------------__ Hacking STEM BrainGong Arduino __------------===//
// For use with the How Do Impacts Affect The Brain lesson plan 
// available from Microsoft Education Workshop at 
// http://aka.ms/hackingSTEM
//
// Overview:
// Project detects analog voltage levels which correspond to 
// deformation of velostat flex sensors attached to a simulated brain. 
// These values help us understand impact forces.
//
// Pins:
// A0 Frontal Sensor 
// A1 Parietal Sensor
// A2 Left Temporal Sensor
// A3 Right Temporal Sensor
// A4 Occipital Sensor
//
// This project uses an Arduino UNO microcontroller board, information at:
// https://www.arduino.cc/en/main/arduinoBoardUno
//
// Comments, contributions, suggestions, bug reports, and feature requests
// are welcome! For source code and bug reports see:
// http://github.com/[TODO github path to Hacking STEM]
//
// Copyright 2018 David Myka Microsoft EDU Workshop - HackingSTEM
// MIT License terms detailed in LICENSE.txt 
//===----------------------------------------------------------------------===//

// Change this variable to increase the frequency that data is sent to Excel
const int dataRate = 10;

// Program variables ----------------------------------------------------------
int brainSensorPin1 = A0;
int brainSensorPin2 = A1;
int brainSensorPin3 = A2;
int brainSensorPin4 = A3;
int brainSensorPin5 = A4;

int brainSensor1;
int brainSensor2;
int brainSensor3;
int brainSensor4;
int brainSensor5;

int brainSensor1Tare;
int brainSensor2Tare;
int brainSensor3Tare;
int brainSensor4Tare;
int brainSensor5Tare;


// Excel variables ------------------------------------------------------------
String incomingExcelString = "";

// Serial data variables ------------------------------------------------------
const byte kNumberOfChannelsFromExcel = 6; 
// IMPORTANT: This must be equal to number of channels set in Data Streamer
//Incoming Serial Data Array
String incomingSerialData[kNumberOfChannelsFromExcel];

const String kDelimiter = ",";    // Data Streamer expects a comma delimeter
String inputString = "";          // String variable to hold incoming data
boolean stringComplete = false;   // Indicates complete string (newline found)
const int kSerialInterval = 10;   // Interval between serial writes
unsigned long serialPreviousTime; // Timestamp to track serial interval

// SETUP ----------------------------------------------------------------------
void setup() {
  //Tare all sensors to zero
  brainSensor1Tare = tare(brainSensorPin1);
  brainSensor2Tare = tare(brainSensorPin2);
  brainSensor3Tare = tare(brainSensorPin3);
  brainSensor4Tare = tare(brainSensorPin4);
  brainSensor5Tare = tare(brainSensorPin5);

  // Initializations occur here
  Serial.begin(9600);  
}

// START OF MAIN LOOP --------------------------------------------------------- 
void loop()
{
  // Process sensors
  processSensors();

  // Read Excel variables from serial port (Data Streamer)
  processIncomingSerial();

  if (incomingSerialData[0] != "#pause")
  {
    // Process and send data to Excel via serial port (Data Streamer)
    processOutgoingSerial();  
  }

}


// SENSOR INPUT CODE-----------------------------------------------------------
void processSensors() 
{
  // Add sensor processing code here
  brainSensor1 = analogRead( brainSensorPin1 ) - brainSensor1Tare;
  brainSensor2 = analogRead( brainSensorPin2 ) - brainSensor2Tare;
  brainSensor3 = analogRead( brainSensorPin3 ) - brainSensor3Tare;
  brainSensor4 = analogRead( brainSensorPin4 ) - brainSensor4Tare;
  brainSensor5 = analogRead( brainSensorPin5 ) - brainSensor5Tare;

//  brainSensor1 = map(brainSensor1, 0, 512, 0, 100);
//  brainSensor2 = map(brainSensor2, 0, 512, 0, 100);
//  brainSensor3 = map(brainSensor3, 0, 512, 0, 100);
//  brainSensor4 = map(brainSensor4, 0, 512, 0, 100);
}

// Add any specialized methods and processing code here

// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
// Process serial data inputString from Data Streamer
void ParseSerialData()
{
  if (stringComplete) {     
    //Build an array of values from comma delimited string from Data Streamer
    BuildDataArray(inputString);

    // Set vavariables based on array index referring to columns:
    // Data Out column A5 = 0, B5 = 1, C5 = 2, etc.
//    incomingExcelFloat = incomingSerialData[0].toFloat(); // Data Out column A5
//    incomingExcelInteger = incomingSerialData[1].toInt(); // Data Out column B5
//    incomingExcelString = incomingSerialData[2]; // Data Out column C5

    inputString = ""; // reset inputString
    stringComplete = false; // reset stringComplete flag
  }
}

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void sendDataToSerial()
{
  // Send data out separated by a comma (kDelimiter)
  // Repeat next 2 lines of code for each variable sent:

  Serial.print(brainSensor1);
  Serial.print(kDelimiter);

  Serial.print(brainSensor2);
  Serial.print(kDelimiter);

  Serial.print(brainSensor3);
  Serial.print(kDelimiter);

  Serial.print(brainSensor4);
  Serial.print(kDelimiter);

  Serial.print(brainSensor5);
  Serial.print(kDelimiter);

  Serial.println(); // Add final line ending character only once
}

int tare(int inputPin)
{
  int tareValue = analogRead( inputPin );
  return tareValue;
}
//-----------------------------------------------------------------------------
// DO NOT EDIT ANYTHING BELOW THIS LINE
//-----------------------------------------------------------------------------

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void processOutgoingSerial()
{
   // Enter into this only when serial interval has elapsed
  if((millis() - serialPreviousTime) > kSerialInterval) 
  {
    serialPreviousTime = millis(); // Reset serial interval timestamp
    sendDataToSerial(); 
  }
}

// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
void processIncomingSerial()
{
  GetSerialData();
  ParseSerialData();
}

// Gathers bits from serial port to build inputString
void GetSerialData()
{
  while (Serial.available()) {
    char inChar = (char)Serial.read();    // Read new character
    inputString += inChar;                // Add it to input string
    if (inChar == '\n') {                 // If we get a newline... 
      stringComplete = true;              // Then we have a complete string
    }
  }
}

// Takes the comma delimited string from Data Streamer
// and splits the fields into an indexed array
void BuildDataArray(String data)
{
  return ParseLine(data);
}

// Parses a single string of comma delimited values with line ending character
void ParseLine(String data) 
{
    int charIndex = 0; // Tracks the character we are looking at
    int arrayIndex = 0; // Tracks the array index to set values into
    while(arrayIndex < kNumberOfChannelsFromExcel) // Loop until full
    {
        String field = ParseNextField(data, charIndex);  // Parse next field
        incomingSerialData[arrayIndex] = field; // Add field to array
        arrayIndex++;   // Increment index
    }
}

// Parses the next value field in between the comma delimiters
String ParseNextField(String data, int &charIndex)
{
    if (charIndex >= data.length() )
    {
      return ""; //end of data
    }
    
    String field = "";
    bool hitDelimiter = false; // flag for delimiter detection 
    while (hitDelimiter == false) // loop characters until next delimiter
    {
        if (charIndex >= data.length() )
        {
          break; //end of data
        }

        if (String(data[charIndex]) == "\n") // if character is a line break
        {
          break; // end of data
        }
        
       if(String(data[charIndex]) == kDelimiter) // if we hit a delimiter
        {
          hitDelimiter = true;  // flag the delimiter hit
          charIndex++; // set iterator after delimiter so we skip next comma
          break;
        }
        else
        {        
          field += data[charIndex]; // add character to field string
          charIndex++; // increment to next character in data
        }
    }
    return field;
}
