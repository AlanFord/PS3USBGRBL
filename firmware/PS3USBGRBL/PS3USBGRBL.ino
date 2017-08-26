/*
 Basic joystick controller for GRBL gcode interpreter attached to 6-axis robotic arm.
 Sends gcode to a GRBL controller based on PS3 joystick controls.
 
 GRBL axes are mapped as follows:
 LeftHatX  -> wrist joint rotation -> B
 LeftHatY  -> elbow                -> Y
 RightHatX -> base                 -> X
 RightHatY -> shoulder             -> Z
 Button2   -> wrist joint pivot    -> A
 Button1   -> gripper              -> C

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
/* You can create the instance of the class in two ways */
PS3USB PS3(&Usb); // This will just create the instance
//PS3USB PS3(&Usb,0x00,0x15,0x83,0x3D,0x0A,0x57); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch

String grblOut;   // used to build the output string that will be sent to grbl
bool grblFlag;    // true if move data needs to be sent to grbl; false otherwise
bool grblJogFlag = 0;  //true if a Jog command was last issued; false if Jog Cancel was last issued
String returnCode;
  
#define CAFJOG 5  // define a nominal jog distance in mm
#define CAFMFR 25  // define a max feed rate in mm/min

void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.println(F("\r\n(OSC did not start)"));
    while (1); //halt
  }
  Serial.println(F("\r\n(PS3 USB Library Started)"));
}
void loop() {
  Usb.Task();

  grblOut = "$J= G21 G91 ";  // basic preamble for a jog command
  grblFlag = 0;              // clear flag until move data is detected
  if (PS3.PS3Connected) {
    if (PS3.getAnalogHat(LeftHatX) > 137) {
      grblOut += "B "+ String(CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(LeftHatX) < 117) {
      grblOut += "B "+ String(-CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(LeftHatY) > 137) {
      grblOut += "Y "+ String(-CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(LeftHatY) < 117) {
      grblOut += "Y "+ String(CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(RightHatX) > 137) {
      grblOut += "X "+ String(CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(RightHatX) < 117) {
      grblOut += "X "+ String(-CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(RightHatY) > 137) {
      grblOut += "Z "+ String(-CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getAnalogHat(RightHatY) < 117) {
      grblOut += "Z "+ String(CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getButtonPress(L1)) {
      grblOut += "C "+ String(-CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getButtonPress(R1)) {
      grblOut += "C "+ String(CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getButtonPress(L2)) {
      grblOut += "A "+ String(-CAFJOG) + " ";
      grblFlag = 1;
      }
    if (PS3.getButtonPress(R2)) {
      grblOut += "A "+ String(CAFJOG) + " ";
      grblFlag = 1;
      }
    if (grblFlag == 1) {
      // send the movement data to grbl and set the grblJogFlag to indicate jog command sent
      Serial.println(grblOut+ "F "+ String(CAFMFR));
      grblJogFlag = 1;
      }
    else {
      // all joystick controls have returned to neutral position, so cancel jog commands
      // if last command sent was a jog command (i.e. not a cancel command)
      if (grblJogFlag == 1) {
        Serial.write(0x85);  // "Jog Cancel" command
        grblJogFlag = 0;
        }
      }
    //while (!Serial.available()){
      
    //}
    //returnCode = Serial.readStringUntil('\n');
    //if (returnCode.equals("OK")){
    //  Serial.println("OK received");
    //} else {
    //  Serial.println("not OK");
    //}
    delay(500);
  }
}
