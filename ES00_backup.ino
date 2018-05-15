#include <Wire.h>
#include <Keypad.h>
#include <SPI.h>

/* For Matlab communication */
const byte    numInts = 19;  // size of byte message
unsigned long receivedInt[numInts]; //variable holding the byte message

int mode =1;
int freq;
//int e1;
//int e2;
int amp;
char key;
boolean button = false;

/* Flag for organizing start and end of transmission of byte data from Matlab to Arduino A0 */
boolean newData = true;
int prevStim =0;


/* INIT  KEYBOARD  */

const byte ROWS = 1; // Four rows
const byte COLS = 2; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 2 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 12, 13 }; 

const byte interruptPin = 2;

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); 


// FOR I2C COM 
const byte MASTER_ADDRESS = 8;            
//const byte MS_DAC_ADDRESS = 9;  
//const byte MS_ADC_ADDRESS = 10;
const byte ES_ADC1_ADDRESS = 11;
const byte ES_ADC2_ADDRESS = 12;
const byte ES_ADC3_ADDRESS = 13;
const byte ES_ADC4_ADDRESS = 14;

/* Message received by ArDue2 */
const int message_size = 8; // every 4 bytes for one long, meaning 8 bytes -> 2 longs
unsigned long i2c_received_data[message_size]; // byte array holding the message from Arduino Due 1 transmitted via I2C
boolean hold;

byte x = 0;
int stim =1;
int a;
int b;
int c;
int d;


void sendSlaves(int value) {
  Wire.beginTransmission(ES_ADC1_ADDRESS);
  Wire.write(value);
  Wire.endTransmission(); 
  
  Wire.beginTransmission(ES_ADC2_ADDRESS);
  Wire.write(value);
  Wire.endTransmission(); 

  Wire.beginTransmission(ES_ADC3_ADDRESS);
  Wire.write(value);
  Wire.endTransmission(); 
  
  Wire.beginTransmission(ES_ADC4_ADDRESS);
  Wire.write(value);
  Wire.endTransmission(); 

  Serial.print("sent value = ");
  Serial.println(value);
}

void setup() {
  Wire.begin();
  Serial.begin(9600);   // start serial com for arduino monitor
  SerialUSB.begin(57600);// Start native UART Serial transmission connection to Matlab
  Serial.println("Test G");
  delay(2000);
  amp = 1; 
  //pinMode(stim_1, INPUT);        // Output pin to send stim1 start and end to master
  //pinMode(stim_2, INPUT);
 // hold = false;
  
}

void loop() {

 // recvWithStartEndMarkers();  // receive byte message from matlab
 //   if(mode == 1&& newData == true){
      char key = kpd.getKey();
      if (key){
          if (amp>1 && amp<7){
            if (key == 50){
              amp++;
            }
            else if(key == 49){
              amp--;
            }
            //sendSlaves(amp);
          }
          else if(amp == 7){
            if(key == 49){
              amp--;
             // sendSlaves(amp);
            } 
          }
          else if(amp == 1){
            if(key == 50){
              amp++;
             // sendSlaves(amp);
            }
          }
          sendSlaves(amp);
      }

  Wire.requestFrom(11, 1);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    int a = Wire.read(); // receive a byte as character
    Serial.print(a);         // print the character
  }
  
  Wire.requestFrom(12, 1);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    int b = Wire.read(); // receive a byte as character
    Serial.print(b);         // print the character
  }

   Wire.requestFrom(13, 1);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    int c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
  }

   Wire.requestFrom(14, 1);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    int d = Wire.read(); // receive a byte as character
    Serial.println(d);         // print the character
      if (d > stim){
        amp = 1;
        sendSlaves(amp);
        stim = d;
        Serial.println("*********************");
      }
  }


 
}

/* Receive Matlab message through SerialUSB using start and end markers */
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;  // flag
    static byte ndx = 0;  // counting index within the message
    int startMarker = 60; // Ascii character code for "<"
    int endMarker = 62;   // Ascii character code for ">"
    byte rc;              // received byte through SerialUSB
 
    while (SerialUSB.available() > 0) {
        rc = SerialUSB.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedInt[ndx] = rc;
                ndx++;
                if (ndx >= numInts) {
                    ndx = numInts - 1;
                }
            }
            else {
                //receivedInt[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                Serial.println("Matlab comm ended");
                mode = byteconv(receivedInt,0);
                if (mode == 1){
                  Serial.println("STARTING CALIBRATION MODE");
                  sendSlaves(amp);
                }
            }
        }
        else if (rc == startMarker) {
            recvInProgress = true; 
            Serial.println("Matlab comm initiated");
            newData = true;
            }
    }
}

/* This function returns the long value (32bit) of an array of 4 bytes */
unsigned long byteconv(unsigned long receivedInt[],int r){
unsigned long value = receivedInt[r+3];
value = value * 256 + receivedInt[r+2];  // effectively shift the first byte 8 bit positions
value = value * 256 + receivedInt[r+1];
value = value * 256 + receivedInt[r]; 
return value;}

/*Reset newData flag*/
void ResetNewData() {
    if (newData == true) {
        newData = false;
    }
}
