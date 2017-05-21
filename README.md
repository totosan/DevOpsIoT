# DevOpsIoT
Integrate IoT device into DevOps culture with creating a Backend deployment to Azure

# Main goal of this project
This project is about bootstrapping a device (here ESP8266) from Azure cloud eith only a little initial code without any passwords.
It suits the case, when producing a piece of IoT hardware for use at innocent place an owner, it will be flashed with bootstrap code, to establish a connection to a productive environment in Azure.

# How does it works
1. The initial code contains only one endpoint. There, the ESP connects to. 
2. In Azrue a device identity will be created from transmitted MAC-Address and a new endpoint will be retransmitted together with it's new identity connection string.
3. After ESP receives that connection, it will download the up to date flash binary (firmware) and starts flashing process imediately.
4. The firmware contains all features (for now its only sending temperature data and faked battery values in intervals to *Azure IoT Hub* )
5. ESP8266 will start as soon flash is done and connection to Internet is set up.

# How to go on
This repository contains three applications.
  - Initial Code for connection to DeviceManager *InitApp*
  - The Firmware code *IoTSolution*
  - WebSite for sending commands to device *IoTWeb*
To get this working on local machine, you have to prepare as described in the following sections.

## What you need
* First you need an **ESP8266** (7/12/2e,...) - best with dev-board.
  Setting up your computer for accessing the hardware under windows is pretty easy. Connect it with USB, if you have a Developer-Board. 
  Otherwise use a search-engine you prefer or have a look to this great ["Instructable-Page"](http://www.instructables.com/id/ESP8266-Wi-fi-module-explain-and-connection/?ALLSTEPS)
* **Git** - download [here](https://git-scm.com/downloads) (as well as [Git Credential Manager](https://java.visualstudio.com/Downloads/gitcredentialmanager/Index) for Linux or Mac OS)
* **VSCode** - you can install it from [here](https://code.visualstudio.com/) or clone it from [here](https://github.com/Microsoft/vscode.git).
* **Node.JS** - Download [here](https://nodejs.org/en/)
* **NPM** 


## How to setup
1. Install [Azure PowerShell](https://docs.microsoft.com/de-de/powershell/azure/install-azurerm-ps?view=azurermps-4.0.0)
1. Clone this repository
1. Change to directory *IoTSolution*
1. **npm install**
1. **npm install -g gulp**
1. modify config.json and add the COM-port, where ESP8266 is connected to 
    1. open **config.json**
    1. open DeviceManager on Windows and have a look to the node *"Connections (COM & LPT)"*
    1. add the listed COM-port ID *(e.g. "COM3")* for your UART-Device as following `"device_port": "COM3"`
1. **gulp install-tools**
1. run *setup-localEnvironment.ps1* in folder *IoTSolution* (this prepares your local environment to build and flash ESP-code to cable connected device)
1. **gulp run** or with listener **gulp run --listen**

### Congratulations! You've completed.

## Important sites to read:
- [Release notes for Azure C SDK](https://github.com/Azure/azure-iot-sdks/releases)
- [Introduction to Azure IoT Device SDK for C](https://docs.microsoft.com/en-gb/azure/iot-hub/iot-hub-device-sdk-c-intro)
- [Arduino Commandline](https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc)

## ToDos:
- copy arduino-esp8266-nodemcu to node-modules/gulp-common
- (adding boards has do be done as defined in Boards.txt - C:\Users\XXX\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.3.0\boards.txt)
- if something goes wrong and flash is currupt: C:\Users\XXX\AppData\Local\Arduino15\packages\esp8266\tools\esptool\0.4.9/esptool.exe -cp COM3 -ce
- also a usefull tool, in case something goes wrong is the [EspExceptionDecoder](https://github.com/me-no-dev/EspExceptionDecoder)