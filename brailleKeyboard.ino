int sign = 1;
int pulseLenght = 4;
int totalPulseLenght = 12;
int idleReading = 536;
int farReading = 5;
int mediumReading = 10;
int strongReading = 25;
int touchReading = 100;
int consecutiveReadings = 5;
int maxConsecutivePulses = 200000;

int farVibrate = 150;
int mediumVibrate = 200;
int strongVibrate = 255;

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
  int consecutivePulses;
  unsigned long lastRun;
  bool reseting;
} Magnet;

typedef struct {
  Magnet m1;
  Magnet m2;
} Dot;

Magnet magnets[magnetsNumber];
Dot dots[6];

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
  m2.sensor = h1;
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
  m2.reseting = false;
  m2.lastRun = millis();
  m2.consecutivePulses = 0;
  
  magnets[0] = m1;
  magnets[1] = m2;

  Dot d1;
  d1.m1 = m1;
  d1.m2 = m2;
  dots[0] = d1;

}

void loop() {
    handleReading(1,readHall(1));
//driveMagnet(1,255);
//Magnet m = magnets[1];
//  digitalWrite(m.forwardPin, m.fieldDirection + 0.5 );
//  digitalWrite(m.backwardPin, -m.fieldDirection + 0.5);
//  analogWrite(m.powerPin, 255);
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
  int diff = base - reading;
  diff = abs(diff);
      
  if(diff < farReading){
    if(certain(magnet, 0, farReading)){
      Serial.println("OUT");
      if(magnets[magnet].reseting){
        magnets[magnet].reseting = false;
      }
    }
    return;
  }
  else if (diff < mediumReading && !magnets[magnet].reseting){
    if(certain(magnet, farReading, mediumReading)){
      Serial.println("FAR");
      vibrateMagnet(magnet, strongVibrate);
    }
  }
  else if(diff < strongReading && !magnets[magnet].reseting){
    if(certain(magnet, mediumReading, strongReading)){
      Serial.println("MED");
      vibrateMagnet(magnet, mediumVibrate);
    }
  }
  else  if(diff < touchReading && !magnets[magnet].reseting){
    if(certain(magnet, strongReading, touchReading)){
      Serial.println("STRONG");
      vibrateMagnet(magnet, strongVibrate);
    }
  }
  else {
    Serial.println("TOUCH");
    if (!magnets[magnet].reseting){
      magnets[magnet].reseting = true;
    }
  }
}

bool certain (int magnet, int lower, int higher){
  bool result = true;
  for (int i = 0; i < consecutiveReadings; i++){
    int base = magnets[magnet].sensor.idleVal;
    int diff = base - analogRead(magnets[magnet].sensor.inputPin);
    diff = abs(diff);
    if (diff < lower || diff > higher){
      result = false;    
    }
  }
 return result;
}

void vibrateMagnet(int magnet, int value){
  unsigned long total = 0;
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
  delay(2);
}

void driveMagnet(int magnet, int value){
  Magnet* m = &magnets[magnet];
  digitalWrite(m->forwardPin, m->fieldDirection + 0.5 );
  digitalWrite(m->backwardPin, -m->fieldDirection + 0.5);
  analogWrite(m->powerPin, value);
}



// running Magnet with given power
//
void pulseMagnet(int magnet, int value){
  if (magnet >= magnetsNumber){
    error("index out of bound", "stopMagnet");
    return;
  }
  Magnet* m = &magnets[magnet];
  unsigned long currentTime = millis();
  unsigned long diff = currentTime - m->lastRun;
  if(diff < 50){
    if(m->consecutivePulses < maxConsecutivePulses){
      m->consecutivePulses++;
      digitalWrite(m->forwardPin, m->fieldDirection + 0.5 );
      digitalWrite(m->backwardPin, -m->fieldDirection + 0.5);
      analogWrite(m->powerPin, value);
      stopMagnet(magnet);
      m->lastRun = millis();
    }
    else{
      stopMagnet(magnet);
      return;
    }
  }
  else if(diff > 4000){
    m->consecutivePulses = 0;
    m->lastRun = millis();
  }
  else {
     if(m->consecutivePulses < maxConsecutivePulses){
        digitalWrite(m->forwardPin, m->fieldDirection + 0.5 );
        digitalWrite(m->backwardPin, -m->fieldDirection + 0.5);
        analogWrite(m->powerPin, value);
        stopMagnet(magnet);
        if(m->consecutivePulses > 0){
          m->consecutivePulses -= 1;
        }
        m->lastRun = millis();
     }
  }

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
  Magnet m = magnets[magnet];
  digitalWrite(m.forwardPin, 0 );
  digitalWrite(m.backwardPin, 0);
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
//  Serial.print("Hall effect reading ");
//  Serial.println(reading);
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














































