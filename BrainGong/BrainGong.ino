#include <Wire.h>

// ------------------------- LIS3DH ACCEL
// TODO Get rid of Accel code
// List of registers used by accelerometer
#define LIS3DH_ADDRESS           0x18
#define LIS3DH_REG_STATUS1       0x07
#define LIS3DH_REG_WHOAMI        0x0F
#define LIS3DH_REG_TEMPCFG       0x1F
#define LIS3DH_REG_CTRL1         0x20
#define LIS3DH_REG_CTRL3         0x22
#define LIS3DH_REG_CTRL4         0x23
#define LIS3DH_REG_OUT_Y_L       0x2A
#define LIS3DH_REG_OUT_Y_H       0x2B

#define LIS3DH_8G_SCALE_FACTOR  .00024414f

#define LIS3DH_RANGE_8_G         0b10   // +/- 8g

const int dataRate = 10;      // Change this variable to increase the frequency that data is sent to Excel
float yAxis = 0;

void writeRegister8 (uint8_t reg, uint8_t val){
  Wire.beginTransmission (LIS3DH_ADDRESS);
  Wire.write (reg);
  Wire.write (val);
  Wire.endTransmission();
}

uint8_t readRegister8 (uint8_t reg){
  Wire.beginTransmission (LIS3DH_ADDRESS);
  Wire.write (reg);
  Wire.endTransmission();

  Wire.requestFrom(LIS3DH_ADDRESS, 1);
  uint8_t val = Wire.read();
  return val;
  Wire.endTransmission();
}
// ------------------------- END LIS3DH ACCEL


// Program variables ----------------------------------------------------------
int brainSenorPin1 = A0;
int brainSenorPin2 = A1;
int brainSenorPin3 = A2;
int brainSenorPin4 = A3;

int brainSenor1;
int brainSenor2;
int brainSenor3;
int brainSenor4;

int brainSenor1Tare;
int brainSenor2Tare;
int brainSenor3Tare;
int brainSenor4Tare;

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
                                              //Initilize LIS3DH and set to +/- 16g Scale Factor
  Wire.begin();

  Wire.beginTransmission(LIS3DH_ADDRESS);   //Connects to LIS3DH via i2c
  Wire.write (LIS3DH_REG_WHOAMI);            //Check that board is connected
  Wire.endTransmission(true);
  Wire.requestFrom (LIS3DH_ADDRESS, 1);
  uint8_t deviceID = Wire.read();

  while (deviceID != 0x33){
    delay(1);
  }

  writeRegister8 (LIS3DH_REG_CTRL1, 0x07);      //Turn on all axes and set to normal mode

  //set data rate
  uint8_t accelDataRate = readRegister8 (LIS3DH_REG_CTRL1);
  accelDataRate &= ~(0xF0);
  accelDataRate |= 0b0111 << 4;                //Change variable to write
  writeRegister8 (LIS3DH_REG_CTRL1, accelDataRate);      //Set data rate to 400 mHz, used to manage power consuption


  writeRegister8 (LIS3DH_REG_CTRL4, 0x88);      //Enebles High Res and BDU

  writeRegister8 (LIS3DH_REG_CTRL3, 0x10);       // DRDY on INT1

  writeRegister8 (LIS3DH_REG_TEMPCFG, 0x80);      //Activate ADC outputs

  //Set read scale
  uint8_t rangeControl = readRegister8 (LIS3DH_REG_CTRL4);
  rangeControl &= ~(0x30);
  rangeControl |= LIS3DH_RANGE_8_G << 4;                //Change variable to write make sure to also update the scale factor
  writeRegister8 (LIS3DH_REG_CTRL4, rangeControl);

  //Tare all sensors to zero
  brainSenor1Tare = tare(brainSenorPin1);
  brainSenor2Tare = tare(brainSenorPin2);
  brainSenor3Tare = tare(brainSenorPin3);
  brainSenor4Tare = tare(brainSenorPin4);

  // Initializations occur here
  Serial.begin(9600);  
}

// START OF MAIN LOOP --------------------------------------------------------- 
void loop()
{
    Wire.beginTransmission(LIS3DH_ADDRESS);
    Wire.write(LIS3DH_REG_OUT_Y_L | 0x80);
    Wire.endTransmission();

    Wire.requestFrom(LIS3DH_ADDRESS, 2);
    while (Wire.available() < 2);

    uint8_t yla = Wire.read();
    uint8_t yha = Wire.read();


    yAxis = yha << 8 | yla;

    yAxis = yAxis * LIS3DH_8G_SCALE_FACTOR;
  
  // Process sensors
  processSensors();

  // Read Excel variables from serial port (Data Streamer)
  processIncomingSerial();

  // Process and send data to Excel via serial port (Data Streamer)
  processOutgoingSerial();  
}


// SENSOR INPUT CODE-----------------------------------------------------------
void processSensors() 
{
  // Add sensor processing code here
  brainSenor1 = analogRead( brainSenorPin1 ) - brainSenor1Tare;
  brainSenor2 = analogRead( brainSenorPin2 ) - brainSenor2Tare;
  brainSenor3 = analogRead( brainSenorPin3 ) - brainSenor3Tare;
  brainSenor4 = analogRead( brainSenorPin4 ) - brainSenor4Tare;

//  brainSenor1 = map(brainSenor1, 0, 512, 0, 100);
//  brainSenor2 = map(brainSenor2, 0, 512, 0, 100);
//  brainSenor3 = map(brainSenor3, 0, 512, 0, 100);
//  brainSenor4 = map(brainSenor4, 0, 512, 0, 100);
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

  Serial.print(brainSenor1);
  Serial.print(kDelimiter);

  Serial.print(brainSenor2);
  Serial.print(kDelimiter);

  Serial.print(brainSenor3);
  Serial.print(kDelimiter);

  Serial.print(brainSenor4);
  Serial.print(kDelimiter);

  Serial.print(yAxis);
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
