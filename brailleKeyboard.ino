#include <math.h>  
int idleReading = 536;
int farReading = 5;
int mediumReading = 10;
int strongReading = 25;
int touchReading = 100;
int consecutiveReadings = 5;
int restTime = 5000;
int maxWorkTime = 3000;
int minTimeBetweenRuns = 300;
int vibrationSetLength = 500;

int singleVibrationLength = 4;
int totalVibrationRepetitions = 3;

int singlePulseLength = 400;
int totalPulseRepetitions = 1;

int confirmationSinglePulseLength = 30;
int confirmationPulseRepetitions = 3;



int signalPulses[3] = {1, 3, 4};
int signalPulsesLength[3] = {400, 4, 25};


int letters[26][6] = {
  {1,0,0,0,0,0},
  {1,1,0,0,0,0},
  {1,0,0,1,0,0},
  {1,0,0,1,1,0},
  {1,0,0,0,1,0},
  {1,1,0,1,0,0},
  {1,1,0,1,1,0},
  {1,1,0,0,1,0},
  {0,1,0,1,0,0},
  {0,1,0,1,1,0},
  {1,0,1,0,0,0},
  {1,1,1,0,0,0},
  {1,0,1,1,0,0},
  {1,0,1,1,1,0},
  {1,0,1,0,1,0},
  {1,1,1,1,0,0},
  {1,1,1,1,1,0},
  {1,1,1,0,1,0},
  {0,1,1,1,0,0},
  {0,1,1,1,1,0},
  {1,0,1,0,0,1},
  {1,1,1,0,0,1},
  {0,1,0,1,1,1},
  {1,0,1,1,0,1},
  {1,0,1,1,1,1},
  {1,0,1,0,1,1}
};


const int ESCSignal = 6;
const int ConfirmationSignal = 5;
const int PreReadingMode = 1;
const int ReadingMode = 2;
const int WritingMode = 0;
const int SetUpMode = 9;



int mode = SetUpMode;
int signalType = 0;

const int shift = 10;
const int strongVibrate = 255;
const int magnetsNumber = 6;

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
  unsigned long lastRun;
  unsigned long firstRun;
  bool isRunning;
  bool overheated;
  bool reseting;
  float repellingDirection;
} Magnet;

Magnet magnets[magnetsNumber];

int forwardPins[] = {1,2,3,4,5,6};
int backwardPins[] = {1,2,3,4,5,6};
int powerPins[] = {1,2,3,4,5,6};
int hallInputPins[] = {1,2,3,4,5,6};



// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  for (int i = 0; i < magnetsNumber; i++){
    Magnet m;
    Hall h;

    h.inputPin = hallInputPins[i];
    pinMode(h.inputPin, INPUT);
    h.idleVal = analogRead(h.inputPin);

    m.forwardPin = forwardPins[i];
    m.backwardPin = backwardPins[i];
    m.powerPin = powerPins[i];

    pinMode(m.forwardPin, OUTPUT);
    pinMode(m.backwardPin, OUTPUT);
    pinMode(m.powerPin, OUTPUT);

    m.fieldDirection = 0.5;
    m.repellingDirection = 0.5;
    m.reseting = false;
    m.lastRun = millis();
    m.firstRun = millis();
    m.isRunning = false;
    m.overheated = false;

    magnets[i] = m;
  }
}





void loop() {
   int val = -1;
   if (Serial.available()){ // If data is available to read,
      val = Serial.read(); // read it and store it in val
      Serial.printf("Received: %d\n", val);
   }
   if(mode == SetUpMode){
      if (val == PreReadingMode){ // If 1 was receidved
        mode = PreReadingMode;
        Serial.write(ConfirmationSignal);
      }
      else if (val == WritingMode){
        mode = WritingMode;
      }
      else {
        return;
      }
      Serial.print("waiting in mode: ");
      Serial.println(mode);
      delay(10); // Wait 10 milliseconds for next reading
   }
   else if(mode == PreReadingMode){
    if (val > 0 && val < 10){
      mode = ReadingMode;
      signalType = val; 
    }
   }
   else if (mode == ReadingMode){
      if (val != -1){ // If data is available to read,
        handleReading(val);
      }
      delay(10);
   }
   else if (mode == WritingMode){
    handleWriting();
   }
}






// ================================================
//   Control fucntions
// ================================================

void handleReading(int letter){
  letter -= shift;
  Serial.printf("Printing letter: %c: {", (letter + 'a') );
  for (int i = 0; i < 6; i++){
    Serial.printf("%d,", letters[letter-shift][i]);
    if(letters[letter-shift][i] == 1){
      pulseMagnet(i, 255, magnets[i].repellingDirection, signalPulsesLength[signalType], signalPulses[signalType]);
    }
  }
  Serial.println("}");
}

void handleWriting(){
  handleInput(1, readHall(1));
}

//void handleVibration(int magnet, int value){
//  int total = 0;
//  while(total < vibrationSetLength){
//    unsigned long StartTime = millis();
//    vibrateMagnet(magnet, value, singleVibrationLength, totalVibrationRepetitions);
//    unsigned long CurrentTime = millis();
//    total += (CurrentTime - StartTime);
//  }
//}



// Deciding whether to apply feedback
//
void handleInput(int magnet, int reading){
  if (magnet >= magnetsNumber){
    error("index out of bound", "stopMagnet");
    return;
  }
  int base = magnets[magnet].sensor.idleVal;
  int diff = base - reading;
  diff = abs(diff);
//  Serial.printf("Diff: %d\n", diff);
      
  if(diff < farReading){
    if(certain(magnet, 0, farReading)){
      if(magnets[magnet].reseting){
        magnets[magnet].reseting = false;
      }
    }
    return;
  }
  else  if(diff < touchReading && !magnets[magnet].reseting){
    if(certain(magnet, strongReading, touchReading)){
      int power = getPower(diff, 0, 100, true);
      pulseMagnet(magnet, power, magnets[magnet].repellingDirection * -1, singlePulseLength, totalPulseRepetitions);
    }
  }
  else {
    if (!magnets[magnet].reseting){
      Serial.printf("%d 1 %lu\n", magnet, millis());
      magnets[magnet].reseting = true;
      pulseMagnet(magnet, strongVibrate, magnets[magnet].repellingDirection, confirmationSinglePulseLength, confirmationPulseRepetitions);
    }
  }
}





void vibrateMagnet(int magnet, int value, int vibrationLength, int repetitions){
  Magnet *m = &magnets[magnet];
  for (int i = 0; i < repetitions; i++){
    int pulse = 0;
    while (pulse < vibrationLength){
      unsigned long StartTime = millis();
      driveMagnet(magnet, value);
      unsigned long CurrentTime = millis();
      pulse += CurrentTime - StartTime;
    }
    revertDirection(magnet);
  }
  stopMagnet(magnet);
  delay(2);
}

void pulseMagnet(int magnet, int value, float dir, int pulseLength, int repetitions){
  setDirection(magnet, dir);
  unsigned long total = 0;
  int runPulse = 1;
  int alreadyStopped = false;
  for(int i = 0; i < repetitions; i++){
    int pulse = 0;
    while (pulse < pulseLength){
      unsigned long StartTime = millis();
      if(runPulse == 1){
        driveMagnet(magnet, value);
        alreadyStopped = false;
      }
      else if (!alreadyStopped){
        stopMagnet(magnet);
        alreadyStopped = true;
      }
      unsigned long CurrentTime = millis();
      pulse += CurrentTime - StartTime;
    }
    runPulse *= -1;
  }
  stopMagnet(magnet);
  delay(2);
}



void driveMagnet(int magnet, int value){
  Magnet* m = &magnets[magnet];
  if(canRun(magnet)){
    if(m->isRunning == false){
      m->isRunning = true;
      if( (millis() - m->lastRun) > minTimeBetweenRuns){
        m->firstRun = millis();
      }
    }
    digitalWrite(m->forwardPin, m->fieldDirection + 0.5 );
    digitalWrite(m->backwardPin, -m->fieldDirection + 0.5);
    analogWrite(m->powerPin, value);
  }
}

bool canRun(int magnet){
  Magnet *m = &magnets[magnet];
  if(m->overheated){
    if ( millis() - m->lastRun > restTime){
      m->overheated = false;
      return true;
    }
    return false;
  }
  if (m->isRunning && millis() - m->firstRun > maxWorkTime){
    m->overheated = true;
    stopMagnet(magnet);
    return false;
  }
  return true;  
}

void stopMagnet(int magnet){
  Magnet *m = &magnets[magnet];
  if(m->isRunning){
    digitalWrite(m->forwardPin, 0 );
    digitalWrite(m->backwardPin, 0);
    m->lastRun = millis();
    m->isRunning = false;
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


void updateIdleVal(int magnet){
  Magnet *m = &magnets[magnet];
  m->sensor.idleVal = analogRead(m->sensor.inputPin);
}



void revertDirection(int magnet){
  if (magnet >= magnetsNumber){
    return;
  }
  Magnet *m = &magnets[magnet];
  m->fieldDirection *= -1;
}

void setDirection(int magnet, float dir){
  if (magnet >= magnetsNumber){
    return;
  }
  Magnet *m = &magnets[magnet];
  m->fieldDirection = dir;
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

int getPower(int val,int minVal, int maxVal, bool revert){
  double normalised = (val - minVal)/(double)maxVal;
  double x = 0.0;
  if(revert){
    x = -(normalised*normalised) + 1;
  } 
  else {
    x = normalised * normalised;  
  }

  
//  Serial.printf("Received power: %f, %f\n", normalised, x);
  
  if (x > 0.9){
    return 255;
  }
  else if(x < 0.1){
    return 100;
  }
  else {
    return 155*x + 100;
  }
}












































