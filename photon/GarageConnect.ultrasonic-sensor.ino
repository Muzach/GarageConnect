#define DoorIsClosed 0
#define DoorIsOpen 1

#define relayPin D0 // Garage Door relay 1 pin
#define echoPin D2  // Echo pin on the HC-SR04
#define relay2Pin D3// Garage Door relay 2 pin -- We use both relays to invert the high/low logic to protect during power-on
#define ledPin D5   // D5 LED Pin
#define trigPin D6  // Trig pin on the HC-SR04

int loopDelay = 2000;           //How often to loop in milliseconds
int LastDoorStatus; //Last status of the door - used to see if the current status of the door has changed
int DoorOpenTime = 0;           //Timer (ms) for time the door was detected as open for alerting
int DoorOpenAlertTime = 120000; //Threshold (ms) for when to start publishing the door left open alarm
int DoorOpenAlertFrequency = 120000; //Frequency of publishing the door left open alarm (ms)
char DoorOpenAlertPublishString[40]; // 40 characters for the alert message
int OpenClosedThreshold = 2500;   //Ultrasonic sensor threshold by which garage is Open or Closed (above or below)
int duration;                   //Used internally to measure distance
int lastGoodDuration=0;           //Used internally to sanitize ultrasonic sensor
bool shouldNotifyOnNextOpen=false; //Used to track if we should send a push on door open
bool shouldNotifyOnNextClose=false; //Used to track if we should send a push on door close

void setup()
    {

        Particle.variable("DoorState", LastDoorStatus);
        //Particle.variable("DoorStatus", LastDoorStatus);
        Particle.variable("DoorOpnTime", DoorOpenTime);
        Particle.variable("SendOnNextO", shouldNotifyOnNextOpen);
        //Particle.variable("OpenClosedT", OpenClosedThreshold); //useful for debugging
        //Particle.variable("UltraDur", duration); //useful for debugging
        Particle.function("TellGarage", TellGarage);

        pinMode(relayPin, OUTPUT); //Garage Door relay pin
        pinMode(relay2Pin, OUTPUT); //Garage Door relay pin
        pinMode(trigPin, OUTPUT);  // hc-sr04 rangefinder Trig
        pinMode(echoPin, INPUT);   // hc-sr04 rangefinder Echo
        pinMode(ledPin, OUTPUT);   // LED

        digitalWrite (relayPin,HIGH); //Set initial state of the garage door trigger to HIGH
        digitalWrite (relay2Pin,HIGH); //Set initial state of the garage door relay2 to LOW
        digitalWrite (trigPin,LOW); //Set initial state of the rangefinder trigger to off
        digitalWrite (ledPin,LOW); //Set initial state of the led to off

        //Set initial door state so we can find out if it changes in the future
        LastDoorStatus=DoorState();

        Particle.publish("DoorStatus", "Setup Complete", PRIVATE);
    }

void loop()
    {

        delay(loopDelay); //delay to not overwhelm pins or API

        //Find the state of the door and then test to see if it has changed since the last time
        //If door state has changed, publish the "DoorStatus" particle cloud variable which can be a push notification on the iPhone
        switch(DoorState())
            {
                case DoorIsOpen:
                {
                    if (LastDoorStatus!=DoorIsOpen)
                    {
                        if (shouldNotifyOnNextOpen) {
                           delay(5000); //If we're going to alarm on this value, let's ake sure it really was opened....
                        }
                        else {
                           delay(500); //State Change! Let's take a half-second, then double-check our distance reading and bail if this was a blip.
                         }
                        if (DoorState()==DoorIsClosed)
                        {
                          break;
                        }
                        LastDoorStatus=DoorIsOpen;
                        Particle.publish("DoorStatus", "Door Changed to Open", PRIVATE);
                    }
                    if (DoorOpenTime>DoorOpenAlertTime)
                    {
                      if (DoorOpenTime%(DoorOpenAlertFrequency)==0)
                      {
                          sprintf(DoorOpenAlertPublishString,"Warning: Door Open for %u minutes",(DoorOpenTime/60000));
                          Particle.publish("DoorLeftOp", DoorOpenAlertPublishString, PRIVATE);
                          shouldNotifyOnNextClose=true; //Since we're alarming on an open door, let's notify when it's next closed.
                      }
                    }
                    if (shouldNotifyOnNextOpen) {
                      Particle.publish("SendOnNextO", "Garage Door Opening", PRIVATE);
                      shouldNotifyOnNextOpen=false;
                    }
                    DoorOpenTime=DoorOpenTime+loopDelay;
                break;
                }
                case DoorIsClosed:
                {
                    if (LastDoorStatus!=DoorIsClosed)
                    {
                      delay(500); //State Change! Let's take a half-second, then double-check our distance reading and bail if this was a blip.
                      if (DoorState()==DoorIsOpen)
                      {
                        break;
                      }
                      LastDoorStatus=DoorIsClosed;
                      Particle.publish("DoorStatus", "Door Changed to Closed", PRIVATE);
                      if (shouldNotifyOnNextClose) {
                        Particle.publish("SendOnNextO", "Garage Door Closing", PRIVATE);
                        shouldNotifyOnNextClose=false;
                      }
                    }
                    if (DoorOpenTime>0)
                    {
                      DoorOpenTime=0;
                    }
                break;
                }
            }
    }


//**********Subroutines**********

//Return the door's state (Open or Closed) by simply comparing the current value or the proximity sensor to the threshold value
int DoorState()
    {
    if (ReadUltrasonicSensor()<OpenClosedThreshold)
        return DoorIsOpen;
    else
        return DoorIsClosed;
    }

//Routine to read the Ultrasonic door sensor to measure distance
// raw data from HC-SR04: 200 to 16000 where 2000 raw = ~35cm,  4000 raw = ~80cm
int ReadUltrasonicSensor ()
    {

    	// The sensor is triggered by a HIGH pulse of 10 or more microseconds.
	    digitalWriteFast(trigPin, HIGH);
	    delayMicroseconds(10);
	    digitalWriteFast(trigPin, LOW);

	    duration = pulseIn(echoPin, HIGH); // Time in microseconds to recieve a ping back on the echo pin

        if (duration>0 && duration<20000) //basic sanitation of input
        {
          lastGoodDuration=duration;
          return duration;
        }
        if (lastGoodDuration>0) //fall back to last good measurement so we smooth out blips
        {
          return lastGoodDuration;
        }
        return duration;
    }

// Relay 2 LOW to trigger garage door contacts to close/short (button press)
// After 0.3s delay, go back HIGH to finish simulation of a button press
void MoveDoor()
    {
      Particle.publish("DoorStatus", "Move Door!", PRIVATE);
        digitalWrite (relay2Pin,LOW); //activate the relay
        //blink light while the garage is moving
          digitalWrite (ledPin,HIGH);
          delay(100);
          digitalWrite (ledPin,LOW);
          delay(100);
          digitalWrite (ledPin,HIGH);
          delay(100);
          digitalWrite (ledPin,LOW);
        digitalWrite (relay2Pin,HIGH);
    }

void CalibrateSensor()
    {
    //Determine OpenClosedThreshold value by reading proximity sensor, moving door;
	  //reading sensor again, and finally moving door back to the original position

    	digitalWrite (ledPin,HIGH); //Light up LED while calibrating
        OpenClosedThreshold = ReadUltrasonicSensor();
        MoveDoor();
        delay(15000); //Wait for Door to fully change states
        OpenClosedThreshold = (OpenClosedThreshold + ReadUltrasonicSensor())/2;
        MoveDoor();
        digitalWrite (ledPin,LOW);  //Calibration is done,  LED off
    }

// Define Cloud API Function commands - Toggle, Open, Close or Check
// Toggle moves door to opposite position, Open is open, Close is close, and Check returns the door's state (open or closed), Calibrate is calibrate the rangefinder
int TellGarage (String command)
    {
      Particle.publish("DoorStatus", "Garage Command Received", PRIVATE);

        if (command=="Toggle")
        {
            MoveDoor();
            return 1;
        }
        else if (command=="Open")
        {
            if (LastDoorStatus==DoorIsClosed)
                MoveDoor();
            return 1;
        }
        else if (command=="Close")
        {
            if (LastDoorStatus==DoorIsOpen)
                MoveDoor();
            return 1;
        }
        else if (command=="Check")
        {
            if (LastDoorStatus==DoorIsOpen)
                Particle.publish("DoorStatus", "Check: Door is Open", PRIVATE);
            else
                Particle.publish("DoorStatus", "Check: Door is Closed", PRIVATE);
            return 1;
        }
        else if (command=="Calibrate")
        {
            CalibrateSensor();
            return 1;
        }
        else if (command=="SendNotificationForNextOpen")
        {
          shouldNotifyOnNextOpen=!shouldNotifyOnNextOpen;
          return 1;
        }
        else
        {
        return -1;
        }

    }
