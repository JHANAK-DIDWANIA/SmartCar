// LIBRARIES

#include <Wire.h>
#include <Smartcar.h>
#include <Smartcar_sensors.h>
#include <SoftwareSerial.h>


// VARIABLES

// Components
//Odometer encoder;
//Smartcar alice;
Sonar sonar;

// Pins
const int trig_pin = 43;
const int echo_pin = 42;
const int odo_pin = 19;

// Safe stop
int counter = 0;
static int i;
boolean frontIsClear = true;

// Mode
// Values should be "Idle", "Auto" or "Manual"
String mode = "Idle";



void setup() 
{
 // alice.begin();
  //Serial2.begin(9600);
  Serial.begin(4800);
  sonar.attach(trig_pin, echo_pin);
  //encoder.attach(odo_pin);
}


// AUTO MODE methods

// Returns the number of instructions expected
int numOfInstr() {
  return Serial.readStringUntil('!').toInt();
}

// Reads instructions and stores them in the queue
void storeInstr(String queue[], int queueLen) {
  int indx = 0;
  while (indx < queueLen) {
    if (Serial.available() > 0) {
      queue[indx] = Serial.readStringUntil('*');
      indx++;
    }
    else
    {
      mode = "Idle";
      break;
    }
  }
}

// Executes instructions in the queue
void executeInstr(String queue[], int queueLen) {
  
  // Loops through the queue as long as the mode is Auto
  i = 0;
  while (mode.equals("Auto")) {
    
    String instr = "";
    String parameter = "";
    
    // Splits instruction and parameter
    boolean spaceFound = false;  
    for (int j = 0; j<queue[i].length(); j++) {
      if (queue[i].charAt(j)==' ') {
        spaceFound = true;
      } else if (spaceFound) {
        parameter += queue[i].charAt(j);
      } else {
        instr += queue[i].charAt(j);
      }
    }
    
    // Prints instruction
    Serial.println(instr);
      
    // Executes instruction
    int value = parameter.toInt();
    if (instr.equals("goForward")) {
      goForwardSafe(value);
    } else if (instr=="rotateClockwise") {
      //alice.rotateClockwise(value);
      Serial.println(instr + value);
    } else if (instr=="rotateCounterClockwise") {
      Serial.println(instr + value);
    } else {
      // Prints an error message if the instruction doesn't match any of the possible cases
      String error = "Error with instruction: ";
      Serial.println(error + i);
      mode = "Idle";
    }
    // Moves to next instruction
    i ++;
    
    // Switches back to Idle mode if it reached the end of the queue
    if (i >= queueLen) {
      mode = "Idle";
    }
  }
}

// GO FORWARD SAFE methods

void goForwardSafe(int desiredDistance)
{
  // Resets the odometer
  //encoder.begin(); 
  int distance = 0;
  
  //while(encoder.getDistance() < desiredDistance)
  while (distance < desiredDistance)
  {
    // Check for obstacles
    scan();
    
    // If obstacle is detected: breaks, switches back to Idle mode, sends message to app, resets counter for the scan
    if(!frontIsClear)
    {
      brake();
      //alice.stop();
      Serial.print("Stop");
      mode = "Idle";
      Serial.print("Obstacle Detected ");
      Serial.println(++i);
      counter = 0;
      break;
    }
    
    //alice.goForward();
    Serial.print("Going forward");
    distance ++;
    delay(100);
  }
}

// Scan using sonar
void scan()
{
  int distance = sonar.getDistance();
  
  if(distance < 25 && distance != 0)
    counter++;
  
  if(counter >= 3)
  {
    frontIsClear = false;
  }
}

// Brake
void brake()
{
  //alice.stop();
  Serial.print("Stop");
  delay(50);
  //alice.goBackward();
  delay(100);
}

//AUTO mode
void autoMode()
{
  //Initialize a queue as accommodation
    int arraylength = numOfInstr();
    String queue[arraylength];
    
    //Store instructions in queue
    storeInstr(queue, arraylength);
   
    //Interpret and excute instructions
    executeInstr(queue, arraylength);
}

//MANUAL mode
void manualMode()
{
  String manualIn = Serial.readStringUntil('*');
  if (manualIn.equals("@"))
  {
    Serial.read();
    mode = "Idle";
  } else
  {
    if (manualIn.equals("goForward")){
      //alice.goForward();
      Serial.println(manualIn);
    }else if (manualIn.equals("goBackward")){
      Serial.println(manualIn);
    }else if (manualIn.equals("rotateClockwise")){
      Serial.println(manualIn);
    }else if(manualIn.equals("rotateCounterClockwise")){
      Serial.println(manualIn);
    } else if (manualIn.equals("stop")) {
      Serial.println("Stop");
    }
  }
}



void loop() 
{
  if (mode.equals("Idle"))
  {
    //Serial.println(mode);
    if ((Serial.available() > 0 && Serial.peek() != '$'))
    {
      mode = "Auto";
      Serial.print(mode);
    } else if (Serial.available() > 0 && Serial.peek() == '$')
    {
      mode = "Manual";
      Serial.print(mode);
      String trash = Serial.readStringUntil('$');
    }
  } else if (mode.equals("Auto"))
  {
   autoMode(); 
  } else if (mode.equals("Manual"))
  {
    manualMode();
  }
}





