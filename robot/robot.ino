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

// PWM pins
const int motorControllerS1Pin = 2;
const int motorControllerS2Pin = 3;

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
    eStopPin,
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

void setup() {
  setupOutputPins();
  digitalWrite(eStopPin, HIGH);
}

void loop() {
  int i;
  int pin = motorControllerS2Pin;

  for (i = 0; i <= 5; i++){
    analogWrite(pin, i*50);
    analogWrite(LEDPin, i*25);
    delay(1000);
  }
  blink(3000);

  for (i = 5; i >= 0; i--){
    analogWrite(pin, i*50);
    analogWrite(LEDPin, i*25);
    delay(1000);
  }
  blink(3000);
}
