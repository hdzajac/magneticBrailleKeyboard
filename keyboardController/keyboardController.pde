import processing.serial.*;
Serial myPort;
String val;
int mode = 9; // 0 = reading
              // 1 = writing
              // 9 = setup

void setup(){
  String portName = Serial.list()[0]; //change the 0 to a 1 or 2 etc. to match your port
  myPort = new Serial(this, portName, 9600);
  
  println("Choose reading or writing mode [r/w]");  
  
}

void draw()
{

    if ( myPort.available() > 0) {  // If data is available,
      val = myPort.readStringUntil('\n');         // read it and store it in val
      if(val != null){
      println(val); //print it out in the console
      }
    }
  

}

void keyPressed() {
  println("Pressed: " + key);
  int keyIndex = -1;
  if(mode == 9){
    if (key == 'r' || key == 'R'){
      mode = 1;
      myPort.write(0);
    }
    else if (key == 'w' || key == 'W'){
      mode = 0;
      myPort.write(1);
    }
    else return;
  }
  if(mode == 1){
     if (key >= 'A' && key <= 'Z') {
         keyIndex = key - 'A';
     }
     else if (key >= 'a' && key <= 'z') {
       keyIndex = key - 'a';
     }
     if(keyIndex != -1){
       myPort.write(keyIndex);  
     }
  }
}
