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

int frontalSensorPin = A0;
int parietalSensorPin = A1;
int leftTemporalSensorPin = A2;
int rightTemporalSensorPin = A3;
int OccipitalSensorPin = A4;

const byte kNumberOfChannelsFromExcel = 6; 
//Incoming Serial Data Array
String incomingSerialData[kNumberOfChannelsFromExcel];

const String kDelimiter = ",";    // Data Streamer expects a comma delimeter
String inputString = "";          // String variable to hold incoming data
boolean stringComplete = false;   // Indicates complete string (newline found)
const int kSerialInterval = 10;   // Interval between serial writes
unsigned long serialPreviousTime; // Timestamp to track serial interval

// SETUP ----------------------------------------------------------------------
void setup() {
  // Initializations occur here
  Serial.begin(9600);  
}

// START OF MAIN LOOP --------------------------------------------------------- 
void loop()
{

  // Read Excel variables from serial port (Data Streamer)
  ProcessIncomingSerial();

  if (incomingSerialData[0] != "#pause")
  {
    // Process and send data to Excel via serial port (Data Streamer)
    ProcessOutgoingSerial();  
  }

}

// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
// Process serial data inputString from Data Streamer
void ParseSerialData()
{
  if (stringComplete) {     
    //Build an array of values from comma delimited string from Data Streamer
    ParseLine(inputString);

    inputString = ""; // reset inputString
    stringComplete = false; // reset stringComplete flag
  }
}

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void SendDataToSerial() {
  // Read sensor values
  int frontSensorReading = analogRead(frontalSensorPin);
  int parietalSensorReading = analogRead(parietalSensorPin);
  int leftTemporalSensorReading = analogRead(leftTemporalSensorPin);
  int rightTemporalSensorReading = analogRead(rightTemporalSensorPin);
  int occipitalSensorReading = analogRead(OccipitalSensorPin);

  // Send data out separated by a comma (kDelimiter)
  // Repeat next 2 lines of code for each variable sent:
  Serial.print(frontSensorReading);
  Serial.print(kDelimiter);

  Serial.print(parietalSensorReading);
  Serial.print(kDelimiter);

  Serial.print(leftTemporalSensorReading);
  Serial.print(kDelimiter);

  Serial.print(rightTemporalSensorReading);
  Serial.print(kDelimiter);

  Serial.print(occipitalSensorReading);
  Serial.print(kDelimiter);

  Serial.println(); // Add final line ending character only once
}

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void ProcessOutgoingSerial()
{
   // Enter into this only when serial interval has elapsed
  if((millis() - serialPreviousTime) > kSerialInterval) 
  {
    serialPreviousTime = millis(); // Reset serial interval timestamp
    SendDataToSerial(); 
  }
}

// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
void ProcessIncomingSerial()
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
