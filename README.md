### GarageConnect

#### Goals:
* Garage iOT device knowledge of the state of your garage door and push notification if you leave your garage open
* iOS app with integration with Foscam camera to see video inside your garage

##### Fritzing wiring diagram & screenshots of the iOS experience in /documents; example pushover json in /pushover; firmware in /photon

#### Parts List:
* Photon starter kit: https://store.particle.io/?product=particle-photon&utm_source=Proto&utm_medium=Button&utm_content=Photon&utm_campaign=Buy
* ~~Ultrasonic Motion Sensor: http://www.amazon.com/SainSmart-HC-SR04-Ranging-Detector-Distance/dp/B004U8TOE6~~
* I found the ultrasonic sensor to not be reliable enough, so I changed to a reed switch on the garage door: https://www.amazon.com/gp/product/B01BBH325A/ref=oh_aui_detailpage_o03_s01?ie=UTF8&psc=1
* Breadboard: http://www.amazon.com/Phantom-YoYo-Points-Breadboard-Arduino/dp/B016Q6T7Q4
* Relay: http://www.amazon.com/Uxcell-a13071200ux0770-Relay-Shield-Module/dp/B00EDMIWRE
* Jumper Wires: http://www.amazon.com/Honbay®-120pcs-Multicolored-Female-Breadboard/dp/B017NEGTXC
* Wifi antenna (if you need additional range): http://www.amazon.com/Wi-fi-Antenna-RP-SMA-Antennas-Cables/dp/B00A4I3AGE
* Others:  An Enclosure for Logic board, a USB power supply, Adhesive tape, 22 AWG wire for Garage connection, enclosure for ultrasonic motion sensor, alarm/doorbell wire between the device and the sensor/relay

#### Homekit Support via HomeBridge
* Added support for the variables needed in the Homebridge plug-in https://www.npmjs.com/package/homebridge-photon-garagedoor.
     Sample config.json config
     ```
            {
          "accessory": "PhotonGarageDoor",
          "name": "Garage Door",
          "access_token": "<token>",
          "url": "https://api.particle.io/v1/devices/",
          "deviceid": "<device id>",
          "doorOpensInSeconds": 10,
          "doorOpenSensorVariableName": "DoorState",
          "doorStateChangedEventName": "DoorPublish",
          "doorOpenCloseFunctionName": "TellGarage"
      }
     ```
     
* Here are some great resources for getting up and running with Homebridge on a raspberry pi that can control your garage door:
   * https://github.com/nfarina/homebridge
   * https://github.com/nfarina/homebridge/wiki/Running-HomeBridge-on-a-Raspberry-Pi
   * https://www.npmjs.com/package/homebridge-photon-garagedoor
   * https://gist.github.com/johannrichard/0ad0de1feb6adb9eb61a/


