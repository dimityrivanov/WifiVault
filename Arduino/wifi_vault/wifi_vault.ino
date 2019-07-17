#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

int addr = 0;
const int EEPROM_SIZE = 400;
const bool DEBUG = false;
const int EMPRY_ADDRESS = -1;
const int NULL_ADDRESS = -2;
const int EMPTY_EEPROM_CELL = 255;
// compute the required size
const size_t CAPACITY = JSON_ARRAY_SIZE(25);
const char* www_username = "<replace with BASIC USERNAME>";
const char* www_password = "<replace with BASIC PASSWORD>";
JsonObject mLastRedJsonObject;
ESP8266WebServer HTTP(1900);

struct PasswordObject {
  char name[20];
  char password[20];
};

struct BeamRequest
{
  const char* username;
  const char* psw;
  // something
};

int getEE(int addr, int def)
{
  int val = def;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(addr, val);
  EEPROM.commit();
  EEPROM.end();
  if (val == 255) return def;
  return val;
}

int findEmptyAddress() {
  for (int i = 0; i < EEPROM_SIZE; i += sizeof(PasswordObject)) {
    int status = getEE(i, EMPRY_ADDRESS);
    if (status == EMPRY_ADDRESS) {
      return i;
    }
  }

  return NULL_ADDRESS;
}

PasswordObject readEEPROMObject(int addr) {
  PasswordObject customVar; //Variable to store custom object read from EEPROM.
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(addr, customVar);
  EEPROM.commit();
  EEPROM.end();

  return customVar;
}

void clearEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 255);
  }

  // turn the LED on when we're done
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  EEPROM.end();
}

void clearEEPROMAddr(int addr) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(addr, EMPTY_EEPROM_CELL);
  EEPROM.commit();
  EEPROM.end();
}

void writeEEPROMObject(int addr, PasswordObject customVar) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(addr, customVar);
  EEPROM.commit();
  EEPROM.end();
}

void test() {
  clearEEPROM();

  PasswordObject customVar = {
    "1234567891234567891",
    "1234567891234567891"
  };

  writeEEPROMObject(0, customVar);

  for (int i = 0; i < EEPROM_SIZE; i += sizeof(PasswordObject)) {
    if (DEBUG) {
      Serial.print(i);
      Serial.print(" : ");
    }
    int value = getEE(i, -1);
    //Serial.println(value);
    if (value != -1) {
      PasswordObject obj = readEEPROMObject(i);
      if (DEBUG) {
        Serial.println("Read custom object from EEPROM: ");
        Serial.println(obj.name);
        Serial.println(obj.password);
      }
    }
  }
}


void setup() {
  Serial.begin(9600);

  WiFiManager wifiManager;
  wifiManager.autoConnect("WifiVault");

  config_rest_server_routing();

  //test();
}

void request_authorization() {
  if (!HTTP.authenticate(www_username, www_password)) {
    return HTTP.requestAuthentication();
  }
}

boolean checkForKey(const char* key) {
  if (!mLastRedJsonObject.containsKey(key)) {
    if (DEBUG)
      Serial.println("wrong json format for request!!!");
    HTTP.send(400, "application/json", "{\"success\" : false}");
    return false;
  }
  return true;
}

void parseRequestBody() {

  request_authorization();

  StaticJsonDocument<JSON_OBJECT_SIZE(10)> doc;
  String post_body = HTTP.arg("plain");

  if (DEBUG) {
    Serial.print("HTTP Method: ");
    Serial.println(HTTP.method());
    Serial.println(post_body);
  }

  DeserializationError error = deserializeJson(doc, post_body);

  if (error) {
    if (DEBUG)
      Serial.println("error in parsin json body");
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

  PasswordObject obj = readEEPROMObject(addr);
  StaticJsonDocument<JSON_OBJECT_SIZE(10)> jsonResponseDoc;

  jsonResponseDoc["addr"] = addr;
  jsonResponseDoc["key"] = obj.name;
  jsonResponseDoc["value"] = obj.password;

  char JSONmessageBuffer[200];
  serializeJsonPretty(jsonResponseDoc, JSONmessageBuffer);
  HTTP.send(200, "application/json", JSONmessageBuffer);
}

void post_psw() {

  parseRequestBody();

  if (!checkForKey("key")) {
    return;
  }

  if (DEBUG)
    Serial.print("Searching for empty address:");
  int address_value = findEmptyAddress();

  if (address_value != NULL_ADDRESS) {
    if (DEBUG)
      Serial.println(address_value);

    PasswordObject customVar;
    strcpy(customVar.name, mLastRedJsonObject["key"]);
    strcpy(customVar.password, mLastRedJsonObject["value"]);

    writeEEPROMObject(address_value, customVar);

    HTTP.send(200, "application/json", "{\"success\" : true}");
  } else {
    if (DEBUG)
      Serial.println("No more space in EEPROM!!!");
    HTTP.send(400, "application/json", "{\"success\" : false}");
  }
}

void beam_details() {
  parseRequestBody();

  if (!checkForKey("user")) {
    return;
  }

  BeamRequest beam;
  beam.username = mLastRedJsonObject["user"];
  beam.psw = mLastRedJsonObject["psw"];

  Serial.println(beam.username);
  delay(100);
  Serial.println(beam.psw);
  delay(100);
  
  HTTP.send(200, "application/json", "{\"success\" : true}");
}

void delete_psw() {

  parseRequestBody();

  if (!checkForKey("addr")) {
    return;
  }

  //if everything is fine we will process the request here
  clearEEPROMAddr(mLastRedJsonObject["addr"]);
  HTTP.send(200, "application/json", "{\"success\" : true}");
}

void config_rest_server_routing() {
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {

    if (DEBUG)
      Serial.printf("Starting HTTP...\n");
    HTTP.on("/index.html", HTTP_GET, []() {
      HTTP.send(200, "text/plain", "{\"deviceID\":\"<replace with own id>\"}");
    });
    HTTP.on("/description.xml", HTTP_GET, []() {
      SSDP.schema(HTTP.client());
    });

    HTTP.on("/get_psw", HTTP_POST, get_psw);
    HTTP.on("/psw", HTTP_POST, post_psw);
    HTTP.on("/del_psw", HTTP_POST, delete_psw);
    HTTP.on("/beam", HTTP_POST, beam_details);

    HTTP.begin();

    if (DEBUG)
      Serial.printf("Starting SSDP...\n");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(1900);
    SSDP.setName("WiFi Vault");
    SSDP.setSerialNumber("<replace with serial number>");
    SSDP.setURL("index.html");
    SSDP.setModelName("WiFi Vault");
    SSDP.setModelNumber("<replace with model number>");
    SSDP.setModelURL("http://www.google.bg");
    SSDP.setManufacturer("Dimitar Ivanov");
    SSDP.setManufacturerURL("http://www.google.bg");
    //SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setDeviceType("urn:schemas-upnp-org:device:WifiVault:1");
    SSDP.begin();
    if (DEBUG)
      Serial.printf("Ready!\n");
  } else {
    if (DEBUG)
      Serial.printf("WiFi Failed\n");
    while (1) {
      delay(100);
    }
  }
}

void loop() {
  HTTP.handleClient();
  delay(1);
}
