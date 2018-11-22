int currentValue = 5;
int sign = 1;

const int magnetsNumber = 2;

typedef struct {
  int forwardPin;
  int backwardPin;
  int powerPin;
  int fieldDirection;
} Magnet;

Magnet magnets[magnetsNumber];

const int checkReadPin = 32;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(38400);

  Magnet m1;
  Magnet m2;

  m1.forwardPin = 28;
    pinMode(m1.forwardPin,OUTPUT);
  m1.backwardPin = 27;
    pinMode(m1.backwardPin,OUTPUT);
  m1.powerPin = 29;
    pinMode(m1.powerPin,OUTPUT);
  m1.fieldDirection = 0.5;

  m2.forwardPin = 18;
    pinMode(m2.forwardPin,OUTPUT);
  m2.backwardPin = 19;
    pinMode(m2.backwardPin,OUTPUT);
  m2.powerPin = 23;
    pinMode(m2.powerPin,OUTPUT);
  m2.fieldDirection = 0.5;
  
  magnets[0] = m1;
  magnets[1] = m2;
  
  pinMode(checkReadPin,INPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  digitalWrite(magnets[0].forwardPin,HIGH);
  digitalWrite(magnets[0].backwardPin,LOW);
  setValue();
  analogWrite(magnets[0].powerPin,255);
  int sensorValue = analogRead(checkReadPin);
  // print out the value you read:

  Serial.println(sensorValue);
  
  delay(10);        // delay in between reads for stability
}


void setValue(){
  if(currentValue >= 255 || currentValue <= 0){
    sign = -sign;
  }

  currentValue += (5*sign);
}

// ================================================
//   Control fucntions
// ================================================

// Sending power value to given magnet
//
void powerMagnet (int magnet, int value){
  if (magnet >= magnetsNumber || value < 0 || value > 255){
    return;
  }
  analogWrite(magnets[magnet].powerPin, value);
}

void setDirection (int magnet) {
  if (magnet >= magnetsNumber){
    return;
  }
  Magnet m = magnets[magnet];
  digitalWrite(m.forwardPin, m.fieldDirection + 0.5 );
  digitalWrite(m.backwardPin, -m.fieldDirection + 0.5);
}

// Direction should be -0.5 OR 0.5 
//
void changePolrisation(int magnet, int fieldDirection){
  if (magnet >= magnetsNumber){
    return;
  }
  magnets[magnet].fieldDirection = fieldDirection;
}

void revertPolrisation(int magnet){
  if (magnet >= magnetsNumber){
    return;
  }
  magnets[magnet].fieldDirection *= -1;
}
















































