# ESP32-based Smart Energy Gateway
---
##  Introduction
In recent year,the Internet of Things (IoT)has rapidly advanced and become one of the most impactful innovations, especially in developing countries such as Cambodia, IoT technology has been widely applied across various secctors including agriculture, education,industrial and smart monitoring system. It is a revolutionary concept that refers to an interconnected network of physical devices,appliances,and other onject embedded with sensors,software, and network connectivity, allowing them to collect, exchange and analyze data.
With the growing importance of IoT, real-time monitoring system have become essential for improving efficiency, resource management and decision makinng. In this project, an ESP32- based Smart Energy Gateway is developed to monitor and transmit electrical parameters from digital power meters to a cloud database. This system demonstartes how IoT can be applied to energy monitoring and management. Moreover,it's provinding a foundation for smarter and more sustainable infrasture in Cambodia.
---
##  Objective
The main objectives of this project is:
  - Design and implement an ESP32-base gateway that reads all parameters from meters via Modbus RTU.
  - Develop a Node.js backend that exposes a REST API endpoint to receive data from the gateway and forward it to Firebase Realtime Database.
  - Store timestamped meter measurements in Firebase using an organized JSON schema.
  - Ensure robust communication by implementing retries, local buffering on the backend and clear error logging for network and Modbus issues.
  -Evaluate the system in a local Wi-Fi network environment and demonstrate real-time monitoring at pilot site in Cambodia.
---
###  Scope of Work
The projectf covers the following aspects:
   - Firmeware Development: Writing a program in the ESP-IDF framework to read all meter parameter, create JSON payload and POST data to the local Node.js backend at configurable intervals.
   - Communication Protocol: Using Modbus RTU (RS485) to connect the meters with the ESP32 gateway.
   - Local Backend: Developing a Node.js server with REST API endpoints to receive gateway data, validate payloads and write them to Firebase Realtime Database.
   - Cloud Platform: Using Firebase Realtime Database to store timestamped meter records in a structured JSON format.
---
#### System Overview
The system is designed to monitor electrical parameters from meters and store them int he cloud for real-time access. It consists of the following componet:
* ESP32 Gateway
   -  Read data from digitalpower meter(ADL400 , DTSD1352) via RS485 using the Modbus RTU protocol.
   -  Measure some parameters of power meters.
   -  Create structure JSON payloads.
   -  Send JSON data to the Node.js backend via HTTP POST over Wi-Fi.
*  Node.js Backend
   -  Receives JSON data from ESP32 gateways through REST API endpoints.
   -  Validates data and handles retries in case of failure.
   -  Forwards data to Firebase Realtime Database.
   -  Provides endpoint like "/api/data/:gatewayID" for each gateway.
*   Firebase Realtime Database
   -  Stores timestamped meter data in structured JSON format.
   -  Structure Design:
     ```json
{
  "Sensor_Data": {
    "Gateway1": {
      "2025-10-19_14:00:00": {
        "ADL400": {
          "voltage": 220.5,
          "current": 1.2,
          "active_power": 264.6,
          "reactive_power": 12.4,
          "power_factor": 0.98,
          "frequency": 50.0,
          "total_energy": 15.4
        },
        "DTSD1352": {
          "voltage": 221.0,
          "current": 1.1,
          "active_power": 243.1,
          "reactive_power": 13.2,
          "power_factor": 0.97,
          "frequency": 50.1,
          "total_energy": 14.9
        }
      }
    }
  }
}
---
####  Data Flow Diagram
* Structure
Power meter ->  ESP32 Gateway -> Node.js Backend -> Firebase Realtime Database

* Process
  -  ESP32 reads meter parameters.
  -  JSON payload is created with timestamped data.
  -  Payload is sent via HTTP POST to Node.js backend.
  -  Backend validates and stores the data in Firebase.
  -  Real-time monitoring dashboards can access and visualize data.
----
#####  Getting Started
* Prerequisites
  -  ESP32 Board
  -  Digital power meters like ADL400 or DTSD1352 etc.
  -  Node.js installed on local server
  -  Firebase account and service account JSON
    
##### Step
* flash ESP32
After you open the terminal on vsCode, You need to write command "cmd" and then enter you export the source file export.bat. After that you write command "idf.py set-target ESP32". After set the target your need to write command " idf.py build" After you build you click on the project extension at the left of your screen you go tosearch and install C/C++. You install it and enable it.After that you need to write command " mode" for check the COM of ESP that you use. Last step you need to write command "idf.py -p COM8 flash monitor".
* Configure Wi-Fi credentials in the firmware.
* Run the Node.js backend server by command: node server.js.
* Ensure Firebase Realtime Database in configured and accessible.
* ESP32 start sending meter data every minute to the backend which forwards it to Firebase.
---
#### Result
* Real time meter reading are visible in Firebase with timestamps.
* Data can be used for dashboards, analytics or energy monitoring purposes.
* The system can be scaled by adding more gateways or meters.
---
#### Conclusion
This project demonstrates a complete IoT solution for real-time energy monitoring using ESP32, Node.js, and Firebase Realtime Database. It shows how Modbus RTU communication, cloud storage, and robust backend design can work together to provide accurate and timely power meter data. This system provides a foundation for smarter energy management and future IoT expansions in Cambodia.
  


