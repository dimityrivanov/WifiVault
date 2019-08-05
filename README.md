# WifiVault
This is a open source project with ESP8266 that store key value pair (key = max 20 char, value = max 20 char) secure wihtout risk if esp device is hacked to be exposed the data on the device as you will need mobile application to handle the decryption and autocompletion to the computer. It uses V-USB to emulate keyboard and type the data that is stored on the token.

# PCB Preview
![image](https://i.ibb.co/bWyPmk1/Screenshot-2019-07-22-at-10-46-30.png)

Information for schematic files and PCB can be found here: https://easyeda.com/dimitarivanovit/wifivault

# How does it work ?
  * When the device is powered on a webserver is started to setup WiFi connection.
  * After that SSDP service is started.
  * Mobile application find the device over SSDP and bind with the authentication header provided in the constants
  * From the application you can add new key,value pair data the real value is encrypted and its stored on the phone the token    contains only a small portion of the key to unlock it, other factors are your fingerprint, deviceID and encryption method.
  

# Brief story
If you are like me and got tired of remembering and entering again and again the same passwords this idea might fit perfectlly into your pocket because it will not only store it for you but it will type it after that when you request this action threw the mobile applicaiton.


