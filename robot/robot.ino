/* Robot test program

   Artisan's Asylum sweeper bot team
   5-2-2012

   Richard Klancer
   rpk@pobox.com
*/

#include <SPI.h>

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

const byte MOTOR1 = 0;
const byte MOTOR2 = 0x80;

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
void motor(int motor, float speed) {
  byte motorSelect;
  byte val;

  if (speed > 1.0) speed = 1.0;
  if (speed < -1.0) speed = -1.0;

  motorSetting[motor-1] = speed;

  motorSelect = motor == 1 ? MOTOR1 : MOTOR2;
  val = ((byte)(speed * RANGE) + STOP) | motorSelect;
  Serial1.write(val);
}

void setup() {
  setupOutputPins();
  Serial1.begin(19200);
  emergencyStopOff();

  Serial.begin(9600);
  Serial.write("Program starting.\n");
}

void loop() {
  Serial.write("Forward\n");
  motor(2, 1.0);
  delay(1000);

  Serial.write("Stop\n");
  motor(2, 0.0);
  delay(1000);

  Serial.write("Reverse\n");
  motor(2, -1.0);
  delay(1000);

  Serial.write("Stop\n");
  motor(2, 0.0);
  delay(1000);
}
