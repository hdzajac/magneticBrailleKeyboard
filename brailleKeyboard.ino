int currentValue = 5;
int sign = 1;
int pulseLenght = 20;
int totalPulseLenght = 500;
int idleReading = 536;
int farReading = 3;
int mediumReading = 8;
int touchReading = 35;

int farVibrate = 150;
int mediumVibrate = 200;
int touchVibrate = 255;

const int magnetsNumber = 2;

typedef struct {
  int inputPin;
  int idleVal;
} Hall;

typedef struct {
  int forwardPin;
  int backwardPin;
  int powerPin;
  float fieldDirection;
  Hall sensor;
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
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
 
  Magnet m1;
  Hall h1;
  Magnet m2;

  h1.inputPin = 31;
    pinMode(h1.inputPin, INPUT);
  h1.idleVal = analogRead(h1.inputPin);
  m1.sensor = h1;
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
//  startMagnet(1);
//  driveMagnet(1, 255);
    handleReading(0,readHall(0));
//  stopMagnet(1);
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
void handleReading(int magnet, int reading){
  if (magnet >= magnetsNumber){
    error("index out of bound", "stopMagnet");
    return;
  }
  int base = magnets[magnet].sensor.idleVal;
  Serial.println(reading);
  int diff = base - reading;
  diff = abs(diff);
  if(diff < farReading){
    return;
  }
  else if (diff < mediumReading){
    vibrateMagnet(magnet, farVibrate);
  }
  else if(diff < touchReading){
    vibrateMagnet(magnet, mediumVibrate);
  }
  else {
    vibrateMagnet(magnet, touchVibrate);
  }
}

void vibrateMagnet(int magnet, int value){
  unsigned long total = 0;
  startMagnet(magnet);
  while(total < totalPulseLenght){
    int pulse = 0;
    while (pulse < pulseLenght){
      unsigned long StartTime = millis();
      driveMagnet(magnet, value);
      unsigned long CurrentTime = millis();
      pulse += CurrentTime - StartTime;
    }
    total += pulse;
    revertDirection(magnet);
  }
  stopMagnet(magnet);
  delay(100);
}

// running Magnet with given power
//
void driveMagnet(int magnet, int value){
  sendDirection(magnet);
  sendPower(magnet, value);
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
//  Serial.print("Forward pin: ");
//  Serial.print(m.forwardPin);
//  Serial.print(", ");
//  Serial.print(m.fieldDirection + 0.5);
//  Serial.print("Backward pin: ");
//  Serial.print(m.backwardPin);
//  Serial.print(", ");
//  Serial.print(-m.fieldDirection + 0.5);


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

void stopMagnet(int magnet){
  if (magnet >= magnetsNumber){
    error("index out of bound", "stopMagnet");
    return;
  }
  magnets[magnet].fieldDirection = 0;
}

void startMagnet(int magnet){
  if (magnet >= magnetsNumber){
    error("index out of bound", "stopMagnet");
    return;
  }
  magnets[magnet].fieldDirection = 0.5;
}

// ================================================
//   Reading Functions
// ================================================

//int readField(int dot){
//  return analogRead(dots[dot].sensorPin);
//}


int readHall(int magnet){
  if (magnet >= magnetsNumber){
    return 0;
  }
  int reading = analogRead(magnets[magnet].sensor.inputPin);
  Serial.print("Hall effect reading ");
  Serial.println(reading);
  return reading;
}

// ================================================
//   UTILS
// ================================================

void error(char *message, char *fun){
  Serial.print("[Error] [");
  Serial.print(fun);
  Serial.print("]: ");
  Serial.println(message);
}














































