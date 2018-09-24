// Program variables ----------------------------------------------------------
int brainSenorPin1 = A0;
int brainSenorPin2 = A1;
int brainSenorPin3 = A2;
int brainSenorPin4 = A3;

int brainSenor1;
int brainSenor2;
int brainSenor3;
int brainSenor4;

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

  // Process and send data to Excel via serial port (Data Streamer)
  processOutgoingSerial();  
}


// SENSOR INPUT CODE-----------------------------------------------------------
void processSensors() 
{
  // Add sensor processing code here
  brainSenor1 = analogRead( brainSenorPin1 );
  brainSenor2 = analogRead( brainSenorPin2 );
  brainSenor3 = analogRead( brainSenorPin3 );
  brainSenor4 = analogRead( brainSenorPin4 );
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