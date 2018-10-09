#include <Wire.h>

// ------------------------- LIS3DH ACCEL

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

int maxValArr[4][2];

int brainLight1 = 3;
int brainLight2 = 5;
int brainLight3 = 6;
int brainLight4 = 9;
int brainLight5 = 6;

int brainSensorArry [4][3] = 
  {{brainLight1},
  {brainLight2},
  {brainLight3},
  {brainLight4}};

int brainLights [4][2];

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
  if( maxValArr[3][1] > 100){
    leds();
  }

// Print the LED pin number and max value after sorting.     
//for (int i = 0; i < 4; i++){
//      for (int j=0; j < 2; j++){
//       Serial.print( maxValArr[i][j]);
//       Serial.print( ": ");
//      }
//      Serial.println();
//    }

}


// SENSOR INPUT CODE-----------------------------------------------------------
void processSensors() 
{
  // Add sensor processing code here
//  brainSensorArry[0][2] = analogRead( brainSenorPin1 );
//  brainSensorArry[1][2] = analogRead( brainSenorPin2 );
//  brainSensorArry[2][2] = analogRead( brainSenorPin3 );
//  brainSensorArry[3][2] = analogRead( brainSenorPin4 );

  ledPeak(brainSenorPin1, 1);
  ledPeak(brainSenorPin2, 2);
  ledPeak(brainSenorPin3, 3);
  ledPeak(brainSenorPin4, 4);

  sortBrainArray();

//  for (int i = 0; i > 4; i++){
//      if (brainLights[i][1] > brainLights[i+1][1]){
//        highestBrain[0] = brainLights[i][0];
//      } else if (brainLights[i][1] > brainLights[i+2][1]){
//        highestBrain[1] = brainLights[i][0];
//      }
//  }






//  for (int i = 0; i > 4; i++){
//    for (int h = 0; h > 3; h++){
//      if (brainLights[h][1] > brainLights[h+1][1]){
//        highestBrain[i] = brainLights[h][0];
//      } 
//    }
//  }
}

// Add any specialized methods and processing code here

void sortBrainArray(){
    //int size = sizeof(brainSensorArry[0]);
    int swap [2];
    for (int i = 0; i < 4; i++){
      for (int j = 0; j < 2; j++){
        maxValArr[i][j] = brainSensorArry[i][j];
        //Serial.println(maxValArr[i][j]);
      }
    }
    for(int i=0; i<4; i++){
      for(int j=0; j<3; j++){
        if(maxValArr[j][1] > maxValArr[j+1][1]){
          swap[0] = maxValArr[j][0];
          swap[1] = maxValArr[j][1];
          maxValArr[j][0] = maxValArr[j+1][0];
          maxValArr[j][1] = maxValArr[j+1][1];
          maxValArr[j+1][0] = swap[0];
          maxValArr[j+1][1] = swap[1];
        }
      }
    }
}

void leds(){
  analogWrite(maxValArr[0][0], 0);
  analogWrite(maxValArr[1][0], 63);
  analogWrite(maxValArr[2][0], 127);
  analogWrite(maxValArr[3][0], 255);
}
void ledConversion()
{
//  int led1Val = map(brainSenor1, 0, 1023, 0, 255);
//  int led2Val = map(brainSenor2, 0, 1023, 0, 255);
//  int led3Val = map(brainSenor3, 0, 1023, 0, 255);
//  int led4Val = map(brainSenor4, 0, 1023, 0, 255);
//  int led5Val = map(brainSenor5, 0, 1023, 0, 255);
  
//  analogWrite(brainLight1, led1Val);
//  analogWrite(brainLight2, led2Val);
//  analogWrite(brainLight3, led3Val);
//  analogWrite(brainLight4, led4Val);
//  analogWrite(brainLight5, led5Val);
}

void ledPeak(int pin, int sensorNum )
{
  sensorNum = sensorNum - 1;
  brainSensorArry[sensorNum][2] = analogRead( pin );
  if (brainSensorArry[sensorNum][2] > brainSensorArry[sensorNum][1]){
    brainSensorArry[sensorNum][1] = brainSensorArry[sensorNum][2];
  }
}

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

  Serial.print(brainSensorArry[0][2]);
  Serial.print(kDelimiter);

  Serial.print(brainSensorArry[1][2]);
  Serial.print(kDelimiter);

  Serial.print(brainSensorArry[2][2]);
  Serial.print(kDelimiter);

  Serial.print(brainSensorArry[3][2]);
  Serial.print(kDelimiter);

  Serial.print(yAxis);
  Serial.print(kDelimiter);

  Serial.println(); // Add final line ending character only once
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
