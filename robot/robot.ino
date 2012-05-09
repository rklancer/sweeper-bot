/* Robot test program

   Artisan's Asylum sweeper bot team
   5-2-2012

   Richard Klancer
   rpk@pobox.com
*/

#include <SPI.h>
#include "motors.h"

//
// Pins we define ... these can change if depending how we want to wire things
//

// digital pins
const int eStopPin = 22;
const int vacuumRelayPin = 38;
const int brushRelayPin = 40;

const int SPISelectAPin = 42;
const int SPISelectBPin = 44;
const int SPISelectCPin = 46;
const int SPISelectDPin = 48;

const int LEDPin = 13;

// motor control

const byte MOTOR1_SELECT = 0;
const byte MOTOR2_SELECT = 0x80;

const byte REVERSE = 0x01;
const byte STOP = 0x40;
const byte FORWARD = 0x7F;
const float RANGE = (float)(STOP - REVERSE);

float motorSetting[] = {0.0, 0.0};

//
// SPI stuff. See http://arduino.cc/en/Reference/SPI
// SPI library expects these pins to work as follows:
//

// Master In Slave Out
const int MISOPin = 50;
// Serial Clock
const int SCKPin = 52;
// Master Out Slave In
const int MOSIPin = 51;
// Slave Select (must be careful to set to output mode if not using)
const int SSPin = 53;

void setupOutputPins() {
  int outputPins[] = {
    SSPin,
    brushRelayPin,
    vacuumRelayPin,
    SPISelectAPin,
    SPISelectBPin,
    SPISelectCPin,
    SPISelectDPin,
    LEDPin
  };
  int i;
  int len;

  len = sizeof(outputPins) / sizeof(int);

  for (i = 0; i < len; i++) {
    pinMode(outputPins[i], OUTPUT);
  }
}

/**
  Last setup step -- turn emergency stop system off (i.e, allow system to turn on.)
  Delays 0.1s for motor controller board, etc., to power up after relay switches
*/
void emergencyStopOff() {
  pinMode(eStopPin, OUTPUT);
  digitalWrite(eStopPin, HIGH);
  delay(100);
}

void blink(int t) {
  const int HALF_PERIOD = 100;
  int n;
  int i;

  n = t / HALF_PERIOD;
  for (i = 0; i < n; i++) {
    analogWrite(LEDPin, 255 * (i % 2));
    delay(HALF_PERIOD);
  }
}

/**
  Command motor 1 or 2.

  Arguments:
    motor:  1 or 2
    speed: -1.0 for full reverse; 0 for stop; 1.0 for full forward
    Values outside this range are clamped to the range [-1.0, 1.0]
*/
void motor(Motor m, float speed) {
  byte motorSelect;
  byte val;

  if (speed > 1.0) speed = 1.0;
  if (speed < -1.0) speed = -1.0;

  motorSetting[(int)m] = speed;

  motorSelect = m == MOTOR1 ? MOTOR1_SELECT : MOTOR2_SELECT;
  val = ((byte)(speed * RANGE) + STOP) | motorSelect;
  Serial1.write(val);
}

void logSPI() {
  byte rawByte1;
  byte rawByte2;
  byte rawByte3;
  byte rawByte4;

  digitalWrite(SPISelectCPin, LOW);
  rawByte1 = SPI.transfer(0x00);
  rawByte2 = SPI.transfer(0x00);
  rawByte3 = SPI.transfer(0x00);
  rawByte4 = SPI.transfer(0x00);
  digitalWrite(SPISelectCPin, HIGH);

  rawByte1 &= 0x0F;

  Serial.print("Reading = \n");
  Serial.print(rawByte1, BIN);
  Serial.print(" ");
  Serial.print(rawByte2, BIN);
  Serial.print(" ");
  Serial.print(rawByte3, BIN);
  Serial.print(" ");
  Serial.print(rawByte4, BIN);
  Serial.print("\n\n");

  // // convert the two bytes to one int
  // int rawInt = rawByte1 << 8;
  // rawInt = rawInt | rawByte2;
  // sign = rawInt & 0x1000;

  // Serial.print("rawInt= ");
  // Serial.print(rawInt);
  // Serial.print("  binary = ");
  // Serial.print(rawInt, BIN);
  // Serial.print("  value = ");

  // if (sign) {
  //   Serial.print ("+");
  //   value = rawInt & 0x0FFF;
  // }
  // else {
  //   Serial.print ("-");
  //   value = 0x1000 - rawInt;
  //   // value = !rawInt & 4095; //0x0000111111111111
  // }
  // Serial.print("  ");
  // Serial.print(sign, BIN);
  // Serial.print("  ");
  // Serial.print(value, BIN);
  // Serial.print(value);
  // Serial.print("  voltage = ");
  // if (sign) {
  //   Serial.print("+");
  // }
  // else {
  //   Serial.print("-");
  // }

  // float voltage = 2.0 - ((value / 4096.0) * 2.0);
  // Serial.print(voltage);
  // Serial.print("V\n");
}

void setup() {
  setupOutputPins();

  // start the SPI library:
  SPI.begin();

  // Arduino clock is 16 Mhz; this corresponds to 250kHz CLK on SPI bus
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  // * data is transmitted by MCP3301 on falling edge
  // * Arduino SPI clock idles high
  // this is: CPOL=1, CPHA=1 (SPI mode 3)
  SPI.setDataMode(SPI_MODE3);

  // MCP3301 transmists most significant bit first
  // (Hwever, during a reading, if chip select is held low while 2 additional bytes are read
  // they will be the same reading with the least significant bit first. Handy for debugging!)
  SPI.setBitOrder(MSBFIRST);

  // Pull up the Chip Select pins on the MCP3301 current-sense A/D chips
  digitalWrite(SPISelectAPin, HIGH);
  digitalWrite(SPISelectBPin, HIGH);
  digitalWrite(SPISelectCPin, HIGH);
  digitalWrite(SPISelectDPin, HIGH);

  Serial1.begin(19200);

  Serial.begin(9600);
  Serial.print("Program starting.\n");

  emergencyStopOff();
}

void loop() {
  Serial.print("Forward\n");
  motor(MOTOR2, 1.0);
  delay(2000);
  logSPI();
  delay(2000);

  Serial.print("Stop\n");
  motor(MOTOR2, 0.0);
  delay(2000);
  logSPI();
  delay(2000);

  Serial.print("Reverse\n");
  motor(MOTOR2, -1.0);
  delay(2000);
  logSPI();
  delay(2000);

  Serial.print("Stop\n");
  motor(MOTOR2, 0);
  delay(2000);
  logSPI();
  delay(2000);
}
