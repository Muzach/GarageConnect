#define DoorIsClosed 0
#define DoorIsOpen 1

#define relayPin D0 // Garage Door relay 1 pin
#define reedPin D1 // Reed Switch
#define relay2Pin D3// Garage Door relay 2 pin -- We use both relays to invert the high/low logic to protect during power-on
#define ledPin D5   // D5 LED Pin

int loopDelay = 2000;           //How often to loop in milliseconds
int LastDoorStatus; //Last status of the door - used to see if the current status of the door has changed
int DoorOpenTime = 0;           //Timer (ms) for time the door was detected as open for alerting
int DoorOpenAlertTime = 120000; //Threshold (ms) for when to start publishing the door left open alarm
int DoorOpenAlertFrequency = 120000; //Frequency of publishing the door left open alarm (ms)
char DoorOpenAlertPublishString[40]; // 40 characters for the alert message
bool shouldNotifyOnNextOpen=false; //Used to track if we should send a push on door open
bool shouldNotifyOnNextClose=false; //Used to track if we should send a push on door close

void setup()
    {

        Particle.variable("DoorState", LastDoorStatus);
        Particle.variable("DoorOpnTime", DoorOpenTime);
        Particle.variable("SendOnNextO", shouldNotifyOnNextOpen);
        Particle.function("TellGarage", TellGarage);

        pinMode(relayPin, OUTPUT); //Garage Door relay pin
        pinMode(relay2Pin, OUTPUT); //Garage Door relay pin
        pinMode(reedPin, INPUT_PULLUP); //Reed Switch
        pinMode(ledPin, OUTPUT);   // LED

        digitalWrite (relayPin,HIGH); //Set initial state of the garage door trigger to HIGH
        digitalWrite (relay2Pin,HIGH); //Set initial state of the garage door relay2 to HIGH
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

//Return the door's state (Open or Closed) by checking the reed switch
int DoorState()
    {
    if(digitalRead(reedPin) == HIGH) {
        return DoorIsOpen;
      } else {
        return DoorIsClosed;
    }
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
