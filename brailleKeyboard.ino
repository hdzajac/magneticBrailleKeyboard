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




int confirmationPulseLength = 30;
int confirmationPulseRepetitions = 3;
int confirmationRevert = false;



int signalPulses[3] = {1, 3, 4};
int signalPulsesLength[3] = {400, 4, 25};
int revertPulses[3] = {false, true, true};

int letters[26][6] = {
  {1, 0, 0, 0, 0, 0},
  {1, 1, 0, 0, 0, 0},
  {1, 0, 0, 1, 0, 0},
  {1, 0, 0, 1, 1, 0},
  {1, 0, 0, 0, 1, 0},
  {1, 1, 0, 1, 0, 0},
  {1, 1, 0, 1, 1, 0},
  {1, 1, 0, 0, 1, 0},
  {0, 1, 0, 1, 0, 0},
  {0, 1, 0, 1, 1, 0},
  {1, 0, 1, 0, 0, 0},
  {1, 1, 1, 0, 0, 0},
  {1, 0, 1, 1, 0, 0},
  {1, 0, 1, 1, 1, 0},
  {1, 0, 1, 0, 1, 0},
  {1, 1, 1, 1, 0, 0},
  {1, 1, 1, 1, 1, 0},
  {1, 1, 1, 0, 1, 0},
  {0, 1, 1, 1, 0, 0},
  {0, 1, 1, 1, 1, 0},
  {1, 0, 1, 0, 0, 1},
  {1, 1, 1, 0, 0, 1},
  {0, 1, 0, 1, 1, 1},
  {1, 0, 1, 1, 0, 1},
  {1, 0, 1, 1, 1, 1},
  {1, 0, 1, 0, 1, 1}
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

int forwardPins[] = {2, 4, 6, 8, 10, 12};
int backwardPins[] = {1, 3, 5, 7, 9, 11, 13};
int hallInputPins[] = {A0, A1, A2, A3, A4, A5};



// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  for (int i = 0; i < magnetsNumber; i++) {
    Magnet m;
    Hall h;

    h.inputPin = hallInputPins[i];
    pinMode(h.inputPin, INPUT);
    h.idleVal = analogRead(h.inputPin);

    m.forwardPin = forwardPins[i];
    m.backwardPin = backwardPins[i];

    pinMode(m.forwardPin, OUTPUT);
    pinMode(m.backwardPin, OUTPUT);

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

  digitalWrite(6, 1);

  digitalWrite(7, 0);



    
//  if (Serial.available()) { // If data is available to read,
//    val = Serial.read(); // read it and store it in val
//  }
//  if (mode == SetUpMode) {
//    if (val == PreReadingMode) { // If 1 was receidved
//      mode = PreReadingMode;
//      for (int i = 0; i < magnetsNumber; i++) {
//        setDirection(i, magnets[i].repellingDirection );
//      }
//      Serial.write(ConfirmationSignal);
//    }
//    else if (val == WritingMode) {
//      mode = WritingMode;
//    }
//    else {
//      return;
//    }
//    Serial.print("waiting in mode: ");
//    Serial.println(mode);
//    delay(10); // Wait 10 milliseconds for next reading
//  }
//  else if (mode == PreReadingMode) {
//    if (val > 0 && val < 10) {
//      Serial.print("Reading mode, received signal pulse type: ");
//      Serial.println(val);
//      mode = ReadingMode;
//      signalType = val;
//    }
//  }
//  else if (mode == ReadingMode) {
//    if (val != -1) { // If data is available to read,
//      Serial.print("Reading mode, received char: ");
//      Serial.println((char)(val + 'a' - shift));
//      handleReading(val);
//    }
//    delay(10);
//  }
//  else if (mode == WritingMode) {
//    handleWriting();
//  }
}






// ================================================
//   Control fucntions = READING
// ================================================

void handleReading(int letter) {
  letter -= shift;
  Serial.print("Printing letter: ");
  Serial.println((char) (letter + 'a'));
  pulseMagnets(signalPulsesLength[signalType], signalPulses[signalType], revertPulses[signalType], letters[letter]);
}


// ================================================
//   Control fucntions = WRITING
// ================================================

void handleWriting() {
  int activeMagnets[6] = {0, 0, 0, 0, 0, 0};
  int inputCompleteMagnets[6] = {0, 0, 0, 0, 0, 0};


  for (int magnet = 0; magnet < magnetsNumber; magnet++) {
    int reading = readHall(magnet);

    int base = magnets[magnet].sensor.idleVal;
    int diff = base - reading;
    diff = abs(diff);

    if (diff < farReading) {
      if (certain(magnet, 0, farReading)) {
        if (magnets[magnet].reseting) {
          magnets[magnet].reseting = false;
        }
      }
      continue;
    }
    else  if (diff < touchReading && !magnets[magnet].reseting) {
      if (certain(magnet, strongReading, touchReading)) {
        activeMagnets[magnet] = 1;
      }
    }
    else {
      if (!magnets[magnet].reseting) {
        //        Serial.printf("%d 1 %lu\n", magnet, millis());
        magnets[magnet].reseting = true;
        inputCompleteMagnets[magnet] = 1;
      }
    }
  }

  pulseMagnets(signalPulsesLength[signalType], signalPulses[signalType], revertPulses[signalType], activeMagnets);
  pulseMagnets(confirmationPulseLength, confirmationPulseRepetitions, confirmationRevert, inputCompleteMagnets);
}

// ================================================
//   Control fucntions = SENDING SIGNAL
// ================================================

void pulseMagnets(int pulseLength, int repetitions, int revert, int *magnetsToRun) {
  unsigned long total = 0;
  int runPulse = 1;
  int alreadyStopped = false;
  Serial.print("vibrating: ");
  Serial.println(revert);

  for (int i = 0; i < repetitions; i++) {
    int pulse = 0;

    while (pulse < pulseLength) {
      unsigned long StartTime = millis();

      for (int magnet = 0; magnet <  magnetsNumber; magnet++) {
        if (magnetsToRun[magnet] == 1)
        {
          if (runPulse == 1) {
            driveMagnet(magnet);
            alreadyStopped = false;
          }
          else if (!alreadyStopped) {
            stopMagnet(magnet);
            alreadyStopped = true;
          }
        }
      }
      unsigned long CurrentTime = millis();
      pulse += CurrentTime - StartTime;
    }
    if (revert) {
      for (int magnet = 0; magnet < magnetsNumber; magnet++) {
        revertDirection(magnet);
      }
    }
    else {
      runPulse *= -1;
    }
  }
  for (int magnet = 0; magnet < magnetsNumber; magnet++) {
    stopMagnet(magnet);
  }
  delay(2);
}



void driveMagnet(int magnet) {
  Magnet* m = &magnets[magnet];
  if (canRun(magnet)) {
    if (m->isRunning == false) {
      m->isRunning = true;
      if ( (millis() - m->lastRun) > minTimeBetweenRuns) {
        m->firstRun = millis();
      }
    }

    digitalWrite(m->forwardPin, m->fieldDirection + 0.5);
    digitalWrite(m->backwardPin, -m->fieldDirection + 0.5);
  }
}





// ================================================
//   UTILS
// ================================================

bool canRun(int magnet) {
  Magnet *m = &magnets[magnet];
  if (m->overheated) {
    if ( millis() - m->lastRun > restTime) {
      m->overheated = false;
      return true;
    }
    return false;
  }
  if (m->isRunning && millis() - m->firstRun > maxWorkTime) {
    m->overheated = true;
    stopMagnet(magnet);
    return false;
  }
  return true;
}

void stopMagnet(int magnet) {
  Magnet *m = &magnets[magnet];
  if (m->isRunning) {
    digitalWrite(m->forwardPin, 0 );
    digitalWrite(m->backwardPin, 0);
    m->lastRun = millis();
    m->isRunning = false;
  }
}

bool certain (int magnet, int lower, int higher) {
  bool result = true;
  for (int i = 0; i < consecutiveReadings; i++) {
    int base = magnets[magnet].sensor.idleVal;
    int diff = base - analogRead(magnets[magnet].sensor.inputPin);
    diff = abs(diff);
    if (diff < lower || diff > higher) {
      result = false;
    }
  }
  return result;
}


void updateIdleVal(int magnet) {
  Magnet *m = &magnets[magnet];
  m->sensor.idleVal = analogRead(m->sensor.inputPin);
}


void revertDirection(int magnet) {
  if (magnet >= magnetsNumber) {
    return;
  }
  Magnet *m = &magnets[magnet];
  m->fieldDirection *= -1;
}

void setDirection(int magnet, float dir) {
  if (magnet >= magnetsNumber) {
    return;
  }
  Magnet *m = &magnets[magnet];
  m->fieldDirection = dir;
}




// ================================================
//   Reading Functions
// ================================================

int readHall(int magnet) {
  if (magnet >= magnetsNumber) {
    return 0;
  }
  int reading = analogRead(magnets[magnet].sensor.inputPin);
  return reading;
}

// ================================================
//   WHATEV
// ================================================

void error(char *message, char *fun) {
  Serial.print("[Error] [");
  Serial.print(fun);
  Serial.print("]: ");
  Serial.println(message);
}












































