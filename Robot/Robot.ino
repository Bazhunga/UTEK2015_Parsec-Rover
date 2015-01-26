//*********************************************************************
// Pin Assignments
//*********************************************************************

//Light pins go into the analog inputs (A0-3)
const int lsp1 = A0;
const int lsp2 = A1;
const int lsp3 = A2;
const int lsp4 = A3;

//Microswitch to detect when the car runs into an object
const int switchPin = 3;

//Pin to output HIGH when trial is done to light up LED
const int doneLightPin = 4;

//*********************************************************************
// Global variables
//*********************************************************************
// Raw input reading of the light sensors. This will be the average of the 
// 4 photoresistors so that a more consistent value can be obtained. 
// No processing was done because really, no one cares
float fLightLevel;

// Pins 5 and 6 are used to control the movements of the dc motor
int motor_left[] = {5, 6};

// Flag that gets raised when the car has finished moving and darkest patch 
// has been found
int doneFlag = 0;

// Status of the car movement
//   0: Stop
//   1: Stage 1, scan the walls moving forward
//   2: Stage 2, nudge towards the wall
//   3: Stage 3, Wall has been nudged, start moving backwards
int driveStatus = 0;
// Keeps track of how long we've been driving for
// Since a loop is continuously run, we can count the number of times 
// we've executed the loop and associate that to the darkness reading from
// the light sensor (fLightLevel)
long loopCounter = 0;

// Holds the distance value where we have found the darkest point met so far. 
// Simple comparisons will be done as the sensors poll darkness readings. The darkest
// point will then be associated with the distance traveled (kept track with loopCounter)
int darkestPoint = 0;

// Flag that gets raised when the car has hit a wall
int wallHit = 0;

// Method that gets excecuted upon arduino startup
void setup() {
  // Allows for use of Serial monitor to look at the data being collected
  // Usually used for debugging purposes
  Serial.begin(9600);
  
  // Setup motors to output mode 
  int i;
  for(i = 0; i < 2; i++){
    pinMode(motor_left[i], OUTPUT);
  }
  
  //I'm....not quite sure what this variable is for
  pinMode(2, INPUT);
  
  //Output mode to the LED pin
  pinMode(doneLightPin, OUTPUT);
  
  //Starting mode: Zero, stop.
  driveStatus = 0;
  //Start up buffer time
  delay (1000);
  //Start the driving!
  driveStatus = 1; 
  //Get the light level for now
  fLightLevel = getLightLevel();
}

void loop() {
  
  // Light detection. Compare the reading to what's known to be the darkest so far
  // Store the distance traveled if we've found a darker patch
  // Less light detected == darker
  int currentLightLevel = getLightLevel();
  if (currentLightLevel > fLightLevel){
    fLightLevel = currentLightLevel;
    darkestPoint = loopCounter;
  }
  
// Driving==================================================================================================
  //Notice how 
  
  
  //Wall nudged, move backwards now
  //Final stage of moving backward will continue running until
  //We've either gone back to our starting point (loopCounter counted
  //down to zero, or we've traveled to the marked darkestPoint)
  //A correction constant will be subtracted from the loopCounter to make up for the fact that
  //we poll for the microswitch. Once the switch is depressed, the loopCounter may still be 
  //incrementing as some time will pass before the switch is detected and thus we'll have extra "distance" 
  //when we really haven't gone anywhere.
  //Make sure to do loopCounter < darkestPoint in case some weird subtraction
  //error occurs and we skip the darkest point. Also if the darkest point is at the beginning,
  //since we subtract the loopCounter in the end by a correction constant, we might get
  //negative numbers.
  if (driveStatus == 3){
    if (doneFlag == 0) {
      drive_backward(250);
      loopCounter--;
      if (loopCounter < 0){
        motor_stop();
        doneFlag = 1;
      }
      else if (loopCounter < darkestPoint) {
        motor_stop();
        doneFlag = 1;
      }
    }
    //Check 
  }
  
  //Wall has been hit, nudge towards wall
  //This has actually been cut to be really short as the car 
  //doesn't bounce back from the wall after hitting it as we initially
  //had thought. So it only goes forward for a 25ms interval of time
  //before entering the reversal stage.
  else if (driveStatus == 2) {
    motor_stop();
    drive_forward(100);
    delay (25);
    motor_stop();
    driveStatus = 3;
  }
  
  //Regular Driving foward. nothing special here
  else if (driveStatus == 1) {
    drive_forward(250);
    loopCounter++;
  }
  
  //Polling the microswitch to check for depression
  if (digitalRead(switchPin) == 0 && wallHit == 0) {
    driveStatus = 2;
    delay(500);
    wallHit = 1;
    loopCounter = loopCounter - 5;
  }
  
  //We're done, shine the LED for done-ness
  if (doneFlag == 1){
    digitalWrite(doneLightPin, HIGH);
  }
// END DRIVING========================================================================================
  
  //Outputs data to serial monitor for debugging purposes. Check if light levels are indeed correct
  //Modulo is used so the monitor doesn't get flooded with data
  
  if (loopCounter % 10 == 0){
      Serial.print("Light level: ");
      Serial.println(currentLightLevel);
      Serial.print("Drive level: ");
      Serial.println(driveStatus);
      Serial.print("Loop Counter: ");
      Serial.println(loopCounter);
      Serial.print("Darkest point found at: ");
      Serial.println(darkestPoint);
  }
  
  //Delay so that loops do go extremely quickly
  delay(25);
  
}

//Return the average reading from light sensors 
//Higher reading means darker patch detected
float getLightLevel(){
  int l1 = analogRead(lsp1);
  int l2 = analogRead(lsp2);
  int l3 = analogRead(lsp3);
  int l4 = analogRead(lsp4);
  
  int average = (l1 + l2 + l3 + l4) / 4;
  return analogRead(average);
}

//Stop the motor by giving both motor pins a 0
void motor_stop(){
  digitalWrite(motor_left[0], LOW); 
  digitalWrite(motor_left[1], LOW); 
  delay(1000);
}

//Start the motor 
void drive_forward(int powerLevel){
  analogWrite(motor_left[0], powerLevel); 
  digitalWrite(motor_left[1], LOW); 
}

//Reverse the motor
void drive_backward(int powerLevel){
  digitalWrite(motor_left[0], LOW); 
  analogWrite(motor_left[1], powerLevel); 
}

//THESE ARE UNUSED SUBROUTINES. CAN BE USED LATER FOR TURNING
void turn_left(){
  digitalWrite(motor_left[0], LOW); 
  digitalWrite(motor_left[1], HIGH); 

}

void turn_right(){
  digitalWrite(motor_left[0], HIGH); 
  digitalWrite(motor_left[1], LOW); 
}

