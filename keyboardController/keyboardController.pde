import processing.serial.*;
Serial myPort;
BufferedReader reader;
int character;
int val;
String file1 = "file1.txt";
int mode = 9; // 0 = reading letters from keyboard
// 1 = prep - sending letters to keyboard
// 9 = setup

final int ESCSignal = -1;
final int ConfirmationSignal = 5;
final int PrepReadingMode = 1;
final int ReadingMode = 2;
final int WritingMode = 0;
final int SetUpMode = 9;

//--------------- Signals from keyboard -------------
//
//  5 - confirmation
//  6 - ESC
//
//  >10 - mapped letters
//
// -------------- Letters for controller -------------
//
int ESC = 8;
//
//


int shift = 10;

void setup() {
  String portName = Serial.list()[0]; //change the 0 to a 1 or 2 etc. to match your port
  myPort = new Serial(this, portName, 9600);

  println("Choose to send letters to keyboard [r] or accept keyboard's input [w], [Esc] to exit any mode and start again.");
}

void draw()
{

  if ( myPort.available() > 0) {  // If data is available,
    val = myPort.read();         // read it and store it in val
    switch (mode) {
    case PrepReadingMode: 
      {
        if (val == ConfirmationSignal) {
          reader = createReader(file1);
          println("Choose the signal type: signal pulse 1 -> 3 Vibration");
          mode = PrepReadingMode;
        }
      }

    case WritingMode: 
      {
        print((char)val);
      }      

    case ReadingMode: 
      {
      }

    case SetUpMode: 
      {
      }
    }
    if(mode != WritingMode){
      print(val);
    }
  }
}


void keyPressed() {
  println("Pressed: " + (int)key);

  // reset key [ESC]
  if (key == ESC) {
    mode = SetUpMode;
    println("Received break key");
    myPort.write(ESCSignal);
    return;
  }

  // Selecting mode
  switch (mode) {
    case SetUpMode:  
      {
        if (key == 'w' || key == 'W') {
          mode = WritingMode;
          myPort.write(WritingMode);
        } else if (key == 'r' || key == 'R') {
          mode = PrepReadingMode;
          myPort.write(PrepReadingMode);
        } else return;
      }
    case PrepReadingMode: 
      {
        if (key < '4' && key > '0') {
          println("sending mode: " +(key - '1'));
          myPort.write(key - '1'); 
          mode = ReadingMode;
        }
      }
    case ReadingMode: 
      {
        if (key == ' ') {
          sendNextCharachter();
        }
      }
  }
}

void sendNextCharachter() {
  try {
    character = reader.read();
  } 
  catch (IOException e) {
    e.printStackTrace();
    character = 0;
  }
  if (character == 0 || character == -1) {
    println("File finished, " + ESCSignal);
    myPort.write(ESCSignal);
    // Stop reading because of an error or file is empty
    mode = SetUpMode;
  } else {
    int toSend = 0;
    if (character >= 'a' && character <= 'z') {
      toSend = character - 'a' + shift;
    } else if (character >= 'A' && character <= 'Z') {
      toSend = character - 'A' + shift;
    }
    if (toSend != 0) {
      println("Sending: " +(char) character);
      myPort.write(toSend);
    } else {
      sendNextCharachter();
    }
  }
}
