/*  
 *   For a Capacitive sensor with a metal object clipped to the sensor pin.
 *   Code by Thomas Friberg (https://github.com/tomtheswede)
 *   Updated 21/05/2016
 */
 
//INPUT VARIABLES
int switchPin=2;
int sensorPin=3; //Must be an interupt pin
int samples=1000; //10000 samples gives a value return rate of 2 per second
int baseCorrection=500; ///Set manually to get the desired output floor value
int triggerThreshold=1200;
int minRetriggerTime=80; //minimum trigger time in milliseconds

//Global variables for the capacitive function
bool newState=LOW; //default to start low
bool recordAvail=false;
long switchTime;
long sensorTime;
int averageCounter=0;
long sensorSum=0;
int sensorVal=0;
bool valAvailable=false;
int baseLine=0;
float divider=samples/1000.0;
int lastVal=0;
long pressTime=0;
long releaseTime=0;
bool triggered=false;
  
void setup() {
  setupLines();
}

void loop() {
  monitorValue();
  checkIfTriggered();
}

void setupLines() {
  pinMode(switchPin,OUTPUT);
  pinMode(sensorPin,INPUT);
  Serial.begin(9600);
  digitalWrite(switchPin,newState); //Start low
  delay(10); //To make sure the state is set before an interrupt is applied
  Serial.println("Calibrating...");
  attachInterrupt(digitalPinToInterrupt(sensorPin),recordTime,CHANGE);
  baseLine=calibrateVal();
  Serial.print("Baseline set to: ");
  Serial.println(baseLine);
  //Set pin to start interrupts
  Serial.println("Going online...");
}

void checkIfTriggered() {
  if ((lastVal<triggerThreshold) && (sensorVal>=triggerThreshold) && (millis()-releaseTime>minRetriggerTime)) {
    lastVal=sensorVal;
    pressTime=millis();
    Serial.println("sensor triggered");
    triggered=true;
  }
  if ((lastVal>=triggerThreshold) && (sensorVal<triggerThreshold) && triggered) {
    lastVal=sensorVal;
    triggered=false;
    releaseTime=millis();
    Serial.print("sensor released after ");
    Serial.print(millis()-pressTime);
    Serial.println(" milliseconds");
  }
}

void monitorValue() {
  switchTime=micros();
  digitalWrite(switchPin,newState);
  if (recordAvail) {
    recordAvail=false;
    averageCounter++;
    sensorSum=sensorSum+sensorTime;
    if (averageCounter>samples) {
      averageCounter=0;
      lastVal=sensorVal;
      sensorVal=(sensorSum/divider)-baseLine;
      sensorSum=0;
      Serial.println(sensorVal); //------------Uncomment this if you want to see the values coming out of the sensor
    }
    //Reset pin for interruption
    newState= !newState;
  }
}

void recordTime() { //What happens when the sensor pin changes value
  sensorTime=micros()-switchTime;
  recordAvail=true;
}


int calibrateVal() { //This is the same code as the 
  int calVal=0;
  newState= !newState;
  while (!valAvailable) {
    switchTime=micros();
    digitalWrite(switchPin,newState);
    if (recordAvail) {
      recordAvail=false;
      averageCounter++;
      sensorSum=sensorSum+sensorTime;
      if (averageCounter>samples) {
        averageCounter=0;
        calVal=sensorSum/divider;
        sensorSum=0;
        valAvailable=true;
      }
      newState= !newState;
    }
  }
  calVal=calVal-baseCorrection; //Just to be sure it is always smaller than the sensor values
  return calVal;
}

