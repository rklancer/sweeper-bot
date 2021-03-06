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


const float CURRENT_SENSOR_RESISTANCE = 0.05;

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

void logSPI(int pin) {
  byte rawByte[6];
  int i;

  int msb;
  int lsb;
  unsigned int lsbReversed;
  unsigned int lsbBit;

  unsigned int msbSignBit;
  unsigned int msbNullBit;
  unsigned int lsbSignBit;
  unsigned int lsbNullBit;

  float reading;
  float voltage;
  float current;

  digitalWrite(pin, LOW);
  for (i = 0; i < 6; i++) {
    rawByte[i] = SPI.transfer(0x00);
  }
  digitalWrite(pin, HIGH);

  Serial.print("----\n");
  for (i = 0; i < 6; i++) {
    Serial.print(rawByte[i], BIN);
    Serial.print(" ");
  }
  Serial.print("\n");

  msbNullBit = (rawByte[0] & 0x20) >> 5;
  msbSignBit = (rawByte[0] & 0x10) >> 4;
  lsbSignBit = (rawByte[3] & 0x10) >> 4;
  lsbNullBit = (rawByte[3] & 0x08) >> 3;

  if (msbNullBit|| lsbNullBit) {
    Serial.print("null bits aren't null!\n");
    Serial.print("----\n");
    return;
  }

  msb = ((int)(rawByte[0] & 0x0F) << 8) | (int)rawByte[1];

  lsbReversed = ((unsigned int)rawByte[2] << 8) | (unsigned int)(rawByte[3] & 0xE0);

  // the zero bit of the LSB reading is not repeated
  lsb = ((unsigned int)rawByte[1] & 0x01);

  for (i = 1; i <= 11; i++) {
    lsbBit = (lsbReversed >> (16 - i));
    lsbBit &= 0x01;
    lsb |= (lsbBit << i);
  }

  if (msbSignBit) {
    msb |= 0xF000;
  }
  if (lsbSignBit) {
    lsb |= 0xF000;
  }

  if (msb == 4095 || msb == -4096) {
    Serial.print("OVERFLOW\n");
  }
  else if (msb == lsb) {
    reading = (float)msb;

    voltage = reading / 4096.0 * 2.0;
    current = voltage / CURRENT_SENSOR_RESISTANCE;

    Serial.print("voltage (V): ");
    Serial.print(voltage, 4);
    Serial.print("\n");
    Serial.print("current (A): ");
    Serial.print(current, 2);
    Serial.print("\n\n");
  }
  else {
    Serial.print("msb reading doesn't match lsb reading\n");
    Serial.print(msbNullBit, DEC);
    Serial.print(lsbNullBit, DEC);
    Serial.print(msbSignBit, DEC);
    Serial.print(lsbSignBit, DEC);

    Serial.print("\n");
    Serial.print(msb, DEC);
    Serial.print(" ?= ");
    Serial.print(lsb, DEC);
    Serial.print("\n\n");
  }

  Serial.print("----\n");
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
  Serial.print("Brush motor on\n");
  digitalWrite(brushRelayPin, HIGH);
  delay(2000);
  logSPI(SPISelectCPin);
  delay(2000);

  Serial.print("Brush motor off\n");
  digitalWrite(brushRelayPin, LOW);
  delay(2000);
  logSPI(SPISelectCPin);
  delay(2000);
}
