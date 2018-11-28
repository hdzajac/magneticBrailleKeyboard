#include <Wire.h>
#define addr 0x0D //I2C Address for The HMC5883

int currentValue = 5;
int sign = 1;

const int magnetsNumber = 2;

typedef struct {
  int forwardPin;
  int backwardPin;
  int powerPin;
  float fieldDirection;
} Magnet;

typedef struct {
  Magnet m1;
  Magnet m2;
} Dot;

Magnet magnets[magnetsNumber];
Dot dots[6];

const int checkReadPin = 32;

// the setup routine runs once when you press reset:
void setup() {
  Wire.begin();
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x0B); // Tell the HMC5883 to Continuously Measure
  Wire.write(0x01); // Set the Register
  Wire.endTransmission();
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x09); // Tell the HMC5883 to Continuously Measure
  Wire.write(0x1D); // Set the Register
  Wire.endTransmission();
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  Serial.println("Set up Begin");
  
  Magnet m1;
  Magnet m2;

  m1.forwardPin = 28;
    pinMode(m1.forwardPin,OUTPUT);
  m1.backwardPin = 27;
    pinMode(m1.backwardPin,OUTPUT);
  m1.powerPin = 29;
    pinMode(m1.powerPin,OUTPUT);
  m1.fieldDirection = 0.5;

  m2.forwardPin = A22;
    pinMode(m2.forwardPin,OUTPUT);
  m2.backwardPin = A21;
    pinMode(m2.backwardPin,OUTPUT);
  m2.powerPin = 23;
    pinMode(m2.powerPin,OUTPUT);
  m2.fieldDirection = 0.5;
  
  magnets[0] = m1;
  magnets[1] = m2;

  Dot d1;
  d1.m1 = m1;
  d1.m2 = m2;
  dots[0] = d1;
  
  pinMode(checkReadPin,INPUT);

  revertDirection(1);
}

void loop() {
  short x, y, z; //triple axis data

  setValue();

  Wire.beginTransmission(addr);
  Wire.write(0x00); //start with register 3.
  Wire.endTransmission();

  //Read the data.. 2 bytes for each axis.. 6 total bytes
  Wire.requestFrom(addr, 6);
  if (6 <= Wire.available()) {
    x = Wire.read(); //MSB  x
    x |= Wire.read() << 8; //LSB  x
    z = Wire.read(); //MSB  z
    z |= Wire.read() << 8; //LSB z
    y = Wire.read(); //MSB y
    y |= Wire.read() << 8; //LSB y
  }

  // Show Values
  Serial.print("X Value: ");
  Serial.println(x);
  Serial.print("Y Value: ");
  Serial.println(y);
  Serial.print("Z Value: ");
  Serial.println(z);
  Serial.println();
  
  sendDirection(0);
  sendDirection(1);
  sendPower(1,255);
  sendPower(0,255);

//  Serial.print("Field strength: ");
//  Serial.println(readField(0));
  
//  int sensorValue = analogRead(checkReadPin);
//  Serial.println(sensorValue);
  
  delay(10);        // delay in between reads for stability
}

// ================================================
//   Helpers
// ================================================

void setValue(){
  if(currentValue >= 255 || currentValue <= 0){
    sign = -sign;
  }
  currentValue += (5*sign);
}

void printMagnet(int magnet){
  Magnet m = magnets[magnet];
  Serial.print("Magnet: ");
  Serial.println(magnet);
  Serial.print("\tforwardPin: ");
  Serial.println(m.forwardPin);
  Serial.print("\tbackwardPin: ");
  Serial.println(m.backwardPin);
  Serial.print("\t\tvalues: ");
  Serial.println(m.fieldDirection);
}


// ================================================
//   Control fucntions
// ================================================

// Sending power value to given magnet
//
void sendPower (int magnet, int value){
  if (magnet >= magnetsNumber || value < 0 || value > 255){
    return;
  }
  analogWrite(magnets[magnet].powerPin, value);
}

void sendDirection (int magnet) {
  if (magnet >= magnetsNumber){
    return;
  }
  Magnet m = magnets[magnet];
  digitalWrite(m.forwardPin, m.fieldDirection + 0.5 );
  digitalWrite(m.backwardPin, -m.fieldDirection + 0.5);
}

// Direction should be -0.5 OR 0.5 
//
void changeDirection(int magnet, int fieldDirection){
  if (magnet >= magnetsNumber){
    return;
  }
  magnets[magnet].fieldDirection = fieldDirection;
}

void revertDirection(int magnet){
  if (magnet >= magnetsNumber){
    return;
  }
  magnets[magnet].fieldDirection *= -1;
}


// ================================================
//   Reading Functions
// ================================================

//int readField(int dot){
//  return analogRead(dots[dot].sensorPin);
//}

















































