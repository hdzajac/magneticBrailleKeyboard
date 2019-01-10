import processing.serial.*;
Serial myPort;
BufferedReader reader;
String line;
String val;
String file1 = "file1.txt";
String file2 = "file2.txt";
String file3 = "file3.txt";
int mode = 9; // 0 = reading letters from keyboard
              // 1 = prep - sending letters to keyboard
              // 9 = setup
              
final int ESCSignal = 6;
final int ConfirmationSignal = 5;
final int PrepSendingMode = 1;
final int SendingMode = 2;
final int AcceptingLettersMode = 0;
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
int ESC = 27;
//
//

              
int shift = 10;

void setup(){
  String portName = Serial.list()[0]; //change the 0 to a 1 or 2 etc. to match your port
  myPort = new Serial(this, portName, 9600);
  
  println("Choose to send letters to keyboard [r] or accept keyboard's input [w], [Esc] to exit any mode and start again.");  
}

void draw()
{

    if ( myPort.available() > 0) {  // If data is available,
      val = myPort.readStringUntil('\n');         // read it and store it in val
      if(val != null){
        switch (mode){
         case PrepSendingMode: {
           int intVal = Integer.parseInt(val);
           if (intVal == ConfirmationSignal){
             myPort.write(ConfirmationSignal);
             mode = SendingMode;
             sendingLoop();
           }
         }
          
          
          
        }
        println(val); //print it out in the console
      }
    }
}


void keyPressed() {
  println("Pressed: " + key);
  int keyIndex = -1;
  
  // reset key [ESC]
  if (key == ESC){
      mode = SetUpMode;
      myPort.write(SetUpMode);
  }
  
  // Selecting mode
  if(mode == 9){
    if (key == 'r' || key == 'R'){
      mode = 1;
      myPort.write(AcceptingLettersMode);
    }
    else if (key == 'w' || key == 'W'){
      mode = PrepSendingMode;
      myPort.write(PrepSendingMode);
    }
    else return;
  }
}

void seningLoop(){
  reader = createReader(file1); 
  
  try {
    line = reader.readLine();
  } catch (IOException e) {
    e.printStackTrace();
    line = null;
  }
  if (line == null) {
    // Stop reading because of an error or file is empty
    mode = SetUpMode;
  } else {
    char characters [] = line.toCharArray();
    for (char c : characters){
      int toSend = 0;
      if (c >= 'a' && c <= 'z'){
        toSend = c - 'a' + shift;
      }
      else if (c >= 'A' && c <= 'Z'){
        toSend = c - 'A' + shift;
      }
      if (toSend != 0){
        myPort.write(toSend);
      }
    } 
  }
}
