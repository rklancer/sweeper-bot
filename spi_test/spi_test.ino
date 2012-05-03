/*
 SPI test code for Artisan's Asylum Robotics Intensive class (sweeper team).
 
 Shows the output of a Barometric Pressure Sensor on a
 Uses the SPI library. For details on the sensor, see:
 http://www.sparkfun.com/commerce/product_info.php?products_id=8161
 http://www.vti.fi/en/support/obsolete_products/pressure_sensors/
 
 This sketch adapted from Nathan Seidle's SCP1000 example for PIC:
 http://www.sparkfun.com/datasheets/Sensors/SCP1000-Testing.zip
 
 Circuit:
 SCP1000 sensor attached to pins 6, 7, 10 - 13:
 DRDY: pin 6
 CSB: pin 7
 MOSI: pin 11
 MISO: pin 12
 SCK: pin 13
 
 created 31 July 2010
 modified 14 August 2010
 by Tom Igoe
 */

// the sensor communicates using SPI, so include the library:
#include <SPI.h>

// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):
//const int dataReadyPin = 6;
//const int chipSelectPin = 7;

const int slaveSelectPin = 32;

void setup() {
  Serial.begin(9600);
  Serial.print("Hello, World!\n");

  // start the SPI library:
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setDataMode(SPI_MODE1);

  // initalize the  data ready and chip select pins:
  // pinMode(chipSelectPin, OUTPUT);
  pinMode (slaveSelectPin, OUTPUT);

  // Configure SCP1000 for low noise configuration:
  // writeRegister(0x02, 0x2D);
  // writeRegister(0x01, 0x03);
  // writeRegister(0x03, 0x02); 

  // give the sensor time to set up:
  delay(100);
}

void loop() {
  // Select High Resolution Mode
  // writeRegister(0x03, 0x0A);

  // don't do anything until the data ready pin is high:  
  // if (digitalRead (dataReadyPin) == HIGH) {
  if (true) {
    int value;
    int sign;
    
    // Read the temperature data
    byte rawByte1 = readSPI();
    byte rawByte2 = readSPI();

    // convert the two bytes to one int
    int rawInt = rawByte1 << 8;
    rawInt = rawInt | rawByte2;
    sign = rawInt & 4096; //0x0001000000000000

    Serial.print ("rawInt=");
    Serial.print (rawInt);
    Serial.print ("  binary = ");
    Serial.print (rawInt, BIN);
    Serial.print ("  value = ");
    
    if (sign) {
      Serial.print ("+");
      value = rawInt & 4095; //0x0000111111111111
    }
    else {
      Serial.print ("-");
      value = 4096 - rawInt;  
      // value = !rawInt & 4095; //0x0000111111111111
    }
    Serial.print("  ");
    Serial.print(sign, BIN);
    Serial.print("  ");
    Serial.print(value, BIN);
    Serial.print(value);
    Serial.print("  voltage = ");
    if (sign) {
      Serial.print("+");
    }
    else {
      Serial.print("-");
    }
      
    float voltage = 2.0 - ((value / 4096.0) * 2.0);
    Serial.print(voltage);
    Serial.print("V\n");
    
    // // Read the pressure data highest 3 bits:
    // byte pressure_data_high = readRegister(0x1F, 1);
    // pressure_data_high &= 0b00000111; //you only needs bits 2 to 0

    // // Read the pressure data lower 16 bits:
    // unsigned int pressure_data_low = readRegister(0x20, 2);
    // // combine the two parts into one 19-bit number:
    // long pressure = ((pressure_data_high << 16) | pressure_data_low)/4;

    // // display the temperature:
    // Serial.println("\tPressure [Pa]=" + String(pressure));
  }
  delay (1000);
}

// Read from or write to register from the SCP1000:
unsigned int readSPI() {
  byte inByte = 0;           // incoming byte from the SPI

  digitalWrite (slaveSelectPin, LOW);
  inByte = SPI.transfer(0x00);
  // decrement the number of bytes left to read:
  // take the chip select high to de-select:
  digitalWrite (slaveSelectPin, HIGH);
  
  // return the result:
  return(inByte);
}


//Sends a write command to SCP1000

/*void writeRegister(byte thisRegister, byte thisValue) {

  // SCP1000 expects the register address in the upper 6 bits
  // of the byte. So shift the bits left by two bits:
  thisRegister = thisRegister << 2;
  // now combine the register address and the command into one byte:
  byte dataToSend = thisRegister | WRITE;

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(dataToSend); //Send register location
  SPI.transfer(thisValue);  //Send value to record into register

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
}*/
