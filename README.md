### GarageConnect

#### Goals:
* Garage iOT device knowledge of the state of your garage door and push notification if you leave your garage open
* Homekit support for garage door via Homebridge on a Raspberry Pi
* iOS app with integration with Foscam camera to see video inside your garage (iOS code not published to github)

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
   * https://gist.github.com/johannrichard/0ad0de1feb6adb9eb61a/
   * https://www.npmjs.com/package/homebridge-photon-garagedoor

    
   * Commands (Raspberry Basic Setup):
      ```
ssh pi@raspberrypi.local
change password
set up .ssh/authorized_keys to make future logins easier
set date/time (sudo cp /usr/share/zoneinfo/America/Los_Angeles /etc/localtime)
sudo apt-get update
sudo apt-get upgrade
sudo reboot
sudo cp /boot/config.txt /boot/config.txt.backup
sudo vi /boot/config.txt (set hdmi_group=1 and hdmi_mode=16, remove overscan)
rename raspberrypi
sudo reboot
```
   * Homebridge setup:
      ```
curl -sL https://deb.nodesource.com/setup_6.x | sudo -E bash -
sudo apt-get install -y nodejs

sudo apt-get install libavahi-compat-libdnssd-dev

sudo npm install -g --unsafe-perm homebridge hap-nodejs node-gyp
cd /usr/lib/node_modules/homebridge/
sudo npm install --unsafe-perm bignum
cd /usr/lib/node_modules/hap-nodejs/node_modules/mdns
sudo node-gyp BUILDTYPE=Release rebuild

sudo npm install -g homebridge-photon-garagedoor

create homebridge user, set the password
place homebridge under /etc/default and homebridge.service under /etc/systemd/system
create /var/homebridge, set up config.json under /var/homebridge as homebridge user
sudo systemctl daemon-reload
sudo systemctl enable homebridge
sudo systemctl start homebridge
```
