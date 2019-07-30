#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include "EEPROMManager.h"
#include "DebugSerial.h"
#include "BeamRequest.cpp"
#include <ArduinoJson.h>
#include <stdlib.h>

EEPROMManager eepromManager;
DebugSerial debugSerial;
const char* www_username = "<set here>";
const char* www_password = "<set here>";
JsonObject mLastRedJsonObject;
StaticJsonDocument<200> doc;
ESP8266WebServer HTTP(1900);


void setup() {
  Serial.begin(9600);

  WiFiManager wifiManager;
  wifiManager.autoConnect("WifiVault");

  config_rest_server_routing();
}

void request_authorization() {
  if (!HTTP.authenticate(www_username, www_password)) {
    return HTTP.requestAuthentication();
  }
}

boolean checkForKey(const char* key) {
  if (!mLastRedJsonObject.containsKey(key)) {
    debugSerial.printToSerial("wrong json format for request!!!");
    HTTP.send(400, "application/json", "{\"success\" : false}");
    return false;
  }
  return true;
}

void parseRequestBody() {

  request_authorization();

  doc.clear();
  String post_body = HTTP.arg("plain");
  //Serial.println(post_body);

  DeserializationError error = deserializeJson(doc, post_body);

  if (error) {
    debugSerial.printToSerial("error in parsin json body");
    HTTP.send(400, "application/json", "{\"success\" : false}");
  }

  mLastRedJsonObject = doc.as<JsonObject>();
}

void get_psw() {

  parseRequestBody();

  if (!checkForKey("addr")) {
    return;
  }

  const int addr = mLastRedJsonObject["addr"];

  PasswordObject obj = eepromManager.readEEPROMObject(addr);
  StaticJsonDocument<100> jsonResponseDoc;

  jsonResponseDoc["addr"] = addr;
  jsonResponseDoc["key"] = obj.name;
  jsonResponseDoc["value"] = obj.password;

  char JSONmessageBuffer[100];
  serializeJsonPretty(jsonResponseDoc, JSONmessageBuffer);
  HTTP.send(200, "application/json", JSONmessageBuffer);
}

void post_psw() {

  parseRequestBody();

  if (!checkForKey("key")) {
    return;
  }

  debugSerial.printToSerial("Searching for empty address:");

  int address_value = eepromManager.findEmptyAddress();

  if (address_value != eepromManager.NULL_ADDRESS) {

    PasswordObject customVar;

    strlcpy(customVar.name, mLastRedJsonObject["key"] | "", sizeof(customVar.name));
    strlcpy(customVar.password, mLastRedJsonObject["value"] | "", sizeof(customVar.password));

    eepromManager.writeEEPROMObject(address_value, customVar);

    HTTP.send(200, "application/json", "{\"success\" : true}");
  } else {
    debugSerial.printToSerial("No more space in EEPROM!!!");
    HTTP.send(400, "application/json", "{\"success\" : false}");
  }
}

void sendDataSerial(const char* text) {
  Serial.println(text);
  delay(200);
  Serial.flush();
}

void beam_details() {
  parseRequestBody();

  if (!checkForKey("user")) {
    return;
  }

  BeamRequest beam;
  beam.username = mLastRedJsonObject["user"];
  beam.password = mLastRedJsonObject["pass"];

  sendDataSerial(beam.username);
  sendDataSerial(beam.password);

  HTTP.send(200, "application/json", "{\"success\" : true}");
}

void delete_psw() {

  parseRequestBody();

  if (!checkForKey("addr")) {
    return;
  }

  //if everything is fine we will process the request here
  eepromManager.clearEEPROMAddr(mLastRedJsonObject["addr"]);
  HTTP.send(200, "application/json", "{\"success\" : true}");
}

void config_rest_server_routing() {
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {

    debugSerial.printToSerial("Starting HTTP...\n");

    HTTP.on("/index.html", HTTP_GET, []() {
      HTTP.send(200, "text/plain", "{\"deviceID\":\"<set here>\"}");
    });
    HTTP.on("/description.xml", HTTP_GET, []() {
      SSDP.schema(HTTP.client());
    });

    HTTP.on("/get_psw", HTTP_POST, get_psw);
    HTTP.on("/psw", HTTP_POST, post_psw);
    HTTP.on("/del_psw", HTTP_POST, delete_psw);
    HTTP.on("/beam", HTTP_POST, beam_details);

    HTTP.begin();

    debugSerial.printToSerial("Starting SSDP...\n");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(1900);
    SSDP.setName("WiFi Vault");
    SSDP.setSerialNumber("<set here>");
    SSDP.setURL("index.html");
    SSDP.setModelName("WiFi Vault");
    SSDP.setModelNumber("<set here>");
    SSDP.setModelURL("http://www.google.bg");
    SSDP.setManufacturer("Dimitar Ivanov");
    SSDP.setManufacturerURL("http://www.google.bg");
    //SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setDeviceType("urn:schemas-upnp-org:device:WifiVault:1");
    SSDP.begin();
    debugSerial.printToSerial("Ready!\n");
  } else {
    debugSerial.printToSerial("WiFi Failed\n");
    while (1) {
      delay(100);
    }
  }
}

void loop() {
  HTTP.handleClient();
  delay(1);
}