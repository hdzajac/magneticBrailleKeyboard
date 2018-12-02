#include <QMC5883L.h>
#include <Wire.h>

int currentValue = 5;
int sign = 1;
QMC5883L compass;
int xAverageReading = -500;
int yAverageReading = -3000;
int zAverageReading = 4000;
int sensitivity = 2000;
int pulseLenght = 35;
int totalPulseLenght = 1000;

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

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} CompassReading;

Magnet magnets[magnetsNumber];
Dot dots[6];

const int checkReadPin = 32;

// the setup routine runs once when you press reset:
void setup() {
  Wire.begin();
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  compass.init();
  compass.setRange(8);
  
  Magnet m1;
  Magnet m2;

  m1.forwardPin = 28;
    pinMode(m1.forwardPin,OUTPUT);
  m1.backwardPin = 27;
    pinMode(m1.backwardPin,OUTPUT);
  m1.powerPin = 29;
    pinMode(m1.powerPin,OUTPUT);
  m1.fieldDirection = 0.5;

  m2.forwardPin = 39;
    pinMode(m2.forwardPin,OUTPUT);
  m2.backwardPin = 38;
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

  //revertDirection(1);
}

void loop() {
  CompassReading reading;
  reading = readCompass();
  handleReading(reading);
 
  
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

// Deciding whether to apply feedback
//
void handleReading(CompassReading reading){
  if(reading.x < (xAverageReading + sensitivity) && reading.x > (xAverageReading - sensitivity)){
      if(reading.y < (yAverageReading + sensitivity) && reading.y > (yAverageReading - sensitivity)){
            if(reading.z < (zAverageReading + sensitivity) && reading.z > (zAverageReading - sensitivity)){
              vibrateMagnet(0);
            }
      }
  } 
}

void vibrateMagnet(int magnet){
  unsigned long total = 0;
  while(total < totalPulseLenght){
    int pulse = 0;
    while (pulse < pulseLenght){
      unsigned long StartTime = millis();
      sendDirection(magnet);
      sendPower(magnet,255);
      unsigned long CurrentTime = millis();
      pulse += CurrentTime - StartTime;
    }
    total += pulse;
    revertDirection(magnet);
  }
  
}

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

CompassReading readCompass(){
  CompassReading reading;
  int16_t x,y,z,t;
  compass.readRaw(&x,&y,&z,&t);
  reading.x = x;
  reading.y = y;
  reading.z = z;
  Serial.print("x: ");
  Serial.print(x);
  Serial.print("    y: ");
  Serial.print(y);
  Serial.print("    z: ");
  Serial.print(z);
  Serial.println();  
  return reading;
}

















































