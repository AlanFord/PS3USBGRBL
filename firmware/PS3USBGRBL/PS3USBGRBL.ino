/*
 Test of a joystick controller for GRBL gcode interpreter attached to 6-axis robotic arm.
 Sends gcode to a GRBL controller based on PS3 joystick controls.
 
 GRBL axes are mapped as follows:
 LeftHatX  -> wrist joint rotation -> B
 LeftHatY  -> elbow                -> Y
 RightHatX -> base                 -> X
 RightHatY -> shoulder             -> Z
 Button2   -> wrist joint pivot    -> A
 Button1   -> gripper              -> C

 Arduino Configuration
 GRBL must be running on an Arduino Mega.
 The joystick may be attached to either an Arduino Uno or Mega using a USB shield.
 The Uno has only one hardware serial port.  The Mega has multiple serial ports, hence
 the connection options are different.

 Using an Uno for the joystick, the Arduino serial monitor will not be operational:

 Mega(grbl)            Uno(joystick)
 D1(TX0)   <---------->  D0(RX)
 D0(RX0)   <---------->  D1(RX)
 GND       <---------->  GND
 VIN       <---------->  VIN [optional, allows powering both Arduinos from one power supply]

Using a Mega for the joystick, the Arduino serial monitor will be operational and can
be used to monitor status:

 Mega(grbl)            Mega(joystick)
 D1(TX0)   <---------->  D15(RX3)
 D0(RX0)   <---------->  D14(RX3)
 GND       <---------->  GND
 VIN       <---------->  VIN [optional, allows powering both Arduinos from one power supply]

 TODO:  remove delay(5000) and wait for 'OK' in response to a '$J' command
 TODO:  constantly send commands as Button 1 and Button 2 are held down
 TODO:  see if grbl needs line feeds or whether Serial.write will work
 */

#include <PS3USB.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif


USB Usb;
PS3USB PS3(&Usb); 

String grblOut;   // used to build the output string that will be sent to grbl
bool grblFlag;    // true if move data needs to be sent to grbl; false otherwise
bool grblJogFlag = 0;  //true if a Jog command was last issued; false if Jog Cancel was last issued
String returnCode;
  
#define JOG_DIST 5  // define a nominal jog distance in mm
#define MFR 25  // define a max feed rate in mm/min

void setup() 
{
  // initialize the serial connection to the grbl controller
  Serial3.begin(115200);

  // initialize the serial connection to the Arduino Serial Monitor
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // verify the USB port (i.e. joystick) has initialized correctly
  if (Usb.Init() == -1) 
  {
    Serial.println(F("\r\n(OSC did not start)"));
    while (1); //halt
  }
  Serial.println(F("\r\n(PS3 USB Library Started)"));

  // Clear the serial buffer if grbl has written data
  if (Serial3.available()){      
    Serial3.readStringUntil('\n');
  }
  if (Serial3.available()){      
    Serial3.readStringUntil('\n');
  }
}


void loop() {
  Usb.Task();

  grblOut = "$J= G21 G91 ";  // basic preamble for a jog command
  grblFlag = 0;              // clear flag until move data is detected
  if (PS3.PS3Connected) {
    if (PS3.getAnalogHat(LeftHatX) > 137) {
      grblOut += "B "+ String(JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(LeftHatX) < 117) {
      grblOut += "B "+ String(-JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(LeftHatY) > 137) {
      grblOut += "Y "+ String(-JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(LeftHatY) < 117) {
      grblOut += "Y "+ String(JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(RightHatX) > 137) {
      grblOut += "X "+ String(JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(RightHatX) < 117) {
      grblOut += "X "+ String(-JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(RightHatY) > 137) {
      grblOut += "Z "+ String(-JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(RightHatY) < 117) {
      grblOut += "Z "+ String(JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getButtonPress(L1)) {
      grblOut += "C "+ String(-JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getButtonPress(R1)) {
      grblOut += "C "+ String(JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getButtonPress(L2)) {
      grblOut += "A "+ String(-JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (PS3.getButtonPress(R2)) {
      grblOut += "A "+ String(JOG_DIST) + " ";
      grblFlag = 1;
      }
    if (grblFlag == 1) {
      // a grbl string is ready to be sent
      // send the movement data to grbl and set the grblJogFlag to indicate jog command sent
      Serial.println("Sending: " + grblOut+ "F "+ String(MFR));
      Serial3.println(grblOut+ "F "+ String(MFR));
      waitForResponse();
      Serial.println("Received: " + returnCode);
      grblJogFlag = 1;
      }
    else {
      // all joystick controls have returned to neutral position, so cancel jog commands
      // if last command sent was a jog command (i.e. not a cancel command)
      if (grblJogFlag == 1) {
        Serial.println("Sending: Jog Cancel");
        Serial3.write(0x85);  // "Jog Cancel" command
        Serial.println("Received: " + returnCode);
        grblJogFlag = 0;
        }
      } 
    /*delay(500);
    if (Serial3.available()){      
    returnCode = Serial3.readStringUntil('\n');
    Serial.println("Received: " + returnCode);
      }
    else {
      Serial.println("Nothing received");
    }*/
  }
}

void waitForResponse() {
  while (Serial3.available() <= 0) {
    delay(50);
  }  
  returnCode = Serial3.readStringUntil('\n');
}

