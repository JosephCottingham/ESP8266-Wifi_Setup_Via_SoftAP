/****************************************************************
'*  Name    : ESP8266-Wifi_Setup_Via_SoftAP.INO                 *
'*  Author  : Joseph Cottingham and Ethan Reiland               *
'*  Notice  : Copyright (c) 2019                                *
'*          : All Rights Reserved                               *
'*  Date    : 6/27/2019                                         *
'*  Version : 1.0                                               *
'*  Notes   :                                                   *
'*          :                                                   *
'****************PROGRAM DESCRIPTION*********************************************************  
'*          : This firmware allows the ESP8266 to take in network configurations and setup a*
'*          : IOT heartbeat return system for environment data factors such as water level  *
'****************CHANGES FROM PREVIOUS REVISION**********************************************
'*          :                                                                               *
'* **************IMPORTANT CONSIDERATIONS FOR THE ESP8266************************************
'*          :                                                                               *
'*********************************************************************************************************************************** 
'*          :  Arduino Programmer Settings for ESP8266 Plugin                                                                      *
'*          :  Oscillator Frequency: 26 MHz                                                                                        *
'*          :  CPU Frequency: 80 MHz                                                                                               *
'*          :  Flash Size: 2M (128K SPIFFS)                                                                                        *
'*          :  Upload Baud: 115200                                                                                                 *
'*          :  Restart Meathod: ck                                                                                                 *
'*          :  Flash Mode: DOUT                                                                                                    *
'*          :  Flash Frequency: 40MHz                                                                                              *
'*          :  VTables: Flash                                                                                                      *
'*          :  Board: Generic ESP8266  Module                                                                                      *
'***********************************************************************************************************************************                                                                                                    
'*                                          *     
'******************************************************************************************************************
'*          :PROGRAMMING HARDWARE SETUP (ESP8266 must be powered by Adapter or Battery Power)                     *
'* GPIO16   :Pin 0: N/A                                                                                           *
'* GPIO14   :Pin 1: N/A                                                                                           *
'* GPIO12   :Pin 2: N/A                                                                                           *
'* GPIO13   :Pin 3: Mem Rest Momentary Switch                                                                     *
'* GPIO15   :Pin 4: Pull Down For Programing and normal operation                                                 *
'* GPIO2    :Pin 5: N/A                                                                                           *
'* GPIO0    :Pin 6: Pull Down for normal operation and high for programing                                        *
'* GPIO4    :Pin 7: NS7000 TX                                                                                     *
'* GPIO5    :Pin 8: NS7000 RX                                                                                     *
'* RXD      :Pin 9:  Device RX Programing                                                                         *
'* TXD      :Pin 10: Device TX Programing                                                                         *
'*          :Rest  : Momentary Switch that brings it low                                                          *
'*          :ADC   : N/A                                                                                          *
'*          :CH_PD : Pulled High for programing and operation                                                     *
'*          :GND   : Pull Down to Negative                                                                        *
'*          :VCC   : Pull Up to Postive                                                                           *
******************************************************************************************************************/

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <ESPSoftwareSerial.h>

#ifndef APSSID
#define APSSID "H2O-WiFi-Unit"
#define APPSK  ""
#endif


ESP8266WebServer server(80);
WiFiClient tcpClient;
SoftwareSerial ESPserial(5, 4); RX, TX

const int port = 65035;
const char ip[] = "10.1.10.253";
const char url[] = "ioth2o.com";
const char *APssid = APSSID;
const char *APpassword = APPSK;
String UnHtmlForm = "";
int MemResetPin = 13;
String form = "";
String ssidWifi;
String passwordWifi;


/*****************************************************************************************
 ********** The following functions are based on memory storage and retrevial*************
 ****************************************************************************************/
 
String memRead(int l, int p) {
  String temp;
  for (int n = p; n < l + p; ++n)
  {
    if (char(EEPROM.read(n)) != ';') {
      if (isWhitespace(char(EEPROM.read(n)))) {

      } else temp += String(char(EEPROM.read(n)));

    } else n = l + p;

  }
  return temp;
}

void memWrite(String s, String p) {
  s += ";";
  writeEEPROM(s, 10);
  p += ";";
  writeEEPROM(p, 110);
  EEPROM.commit();
}

void writeEEPROM(String x, int pos) {
  for (int n = pos; n < x.length() + pos; n++) {
    EEPROM.write(n, x[n - pos]);
  }
}

boolean validSSID(String S) {
  if (S.charAt(0) > 32 && S.charAt(0) < 122) {
    return true;
  } else {
    return false;
  }
}

void memClear(String s, String p) {
  String clearS = "";
  String clearP = "";
  Serial.println(s.length());
  for (int i = 0; i < s.length(); i++) {
    clearS += (char)0;
  }
  for (int i = 0; i < p.length(); i++) {
    clearP += (char)0;
  }
  memWrite(clearS, clearP);
}

/*****************************************************************************************
 *************** The following functions are based on network function *******************
 ****************************************************************************************/

void networkSearchPrint() {
  Serial.print("Scan start ... ");
  int n = WiFi.scanNetworks();
  Serial.println(n);
  Serial.println(" network(s) found");
  for (int i = 0; i < n; i++) {
    Serial.println(i);
    Serial.println(WiFi.SSID(i));
  }
  Serial.println();
}

String htmlForm() {
  String htmlForm = "<!DOCTYPE HTML>";
  htmlForm += "<html>";
  htmlForm += "<head>";
  htmlForm += "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">";
  htmlForm += "<title>ESP8266 Web Form Demo</title>";
  htmlForm += "<style>";
  htmlForm += "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\"";
  htmlForm += "</style>";
  htmlForm += "</head>";
  htmlForm += "<body>";
  htmlForm += "<h1>Enter Data Here</h1>";
  htmlForm += "<FORM action=\"/\" method=\"post\">";
  htmlForm += "<P>";
  htmlForm += "<select required name=\"ssid\">";
  htmlForm += "<option value=\"\">None</option>";
  int c = 4;
  String ssidList[45];
  for (int i = 0; i < 3; i++) {
    c = WiFi.scanNetworks();
    for (int j = 0; j < c; j++) {
      ssidList[j] = WiFi.SSID(j);
    }
  }
  for (int i = 0; i < 45; i++) {
    for (int j = i + 1; j < 45; j++) {
      if (ssidList[j] == ssidList[i]) {
        ssidList[j] = "";
      }
    }
  }
  for (int i = 0; i < 45; i++) {
    Serial.println(ssidList[i]);
  }
  for (int i = 0; i < 45; i++) {
    if (ssidList[i] == "") continue;
    htmlForm += "<option value=\"";
    htmlForm += ssidList[i];
    htmlForm += "\">";
    htmlForm += ssidList[i];
    htmlForm += "</option>";
  }
  htmlForm += "</select><BR>";
  htmlForm += "<INPUT type=\"text\" name=\"password\" placeholder=\"password\"><BR>";
  htmlForm += "<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"button\" name=\"reset\" value=\"reset\">";
  Serial.println("Reformat");
  htmlForm += "</P>";
  htmlForm += "</FORM>";
  htmlForm += "</body>";
  htmlForm += "</html>";
  return htmlForm;
}

boolean handleSubmit() {
  String ssidWifi = server.arg("ssid");
  String passwordWifi = server.arg("password");
  if (validSSID(ssidWifi)) {
    if (wifiConnect(ssidWifi, passwordWifi)) {
      WiFi.softAPdisconnect(true);
      memWrite(ssidWifi, passwordWifi);
      Serial.print("ssid:");
      Serial.println(ssidWifi);
      Serial.print("password:");
      Serial.println(passwordWifi);
      return false;
    }
  }
  else {
    return true;
  }
}

//Refreshes the http server if data has not been input and sent
void handleRoot() {
  if (server.hasArg("ssid")) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", form);
  }
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void softAPConnect() {
  WiFi.softAP(APssid, APpassword);
  Serial.println("");
  Serial.print("Hosting: ");
  Serial.println(APssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

boolean wifiConnect(String s, String p) {
  WiFi.persistent(false);
  WiFi.begin(s, p);
  Serial.println("");
  int i = 0;
  delay(10);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    i++;
    if (i == 300) {
      WiFi.disconnect();
      Serial.println("Wifi Connection Failed, Starting AP");
      return false;
    }
  }
  return true;
}

/*****************************************************************************************
 ****** The following functions are based on data retrieval and RF communcations *********
 ****************************************************************************************/
 
String ICRequestData() {
  ESPserial.begin(9600);
  ESPserial.write(65);
  Serial.println("\nWrite");
  byte bytesRead = 0;
  byte data[16];
  delay(100);
  noInterrupts();
  while (ESPserial.available() > 0) {
    data[bytesRead] = ESPserial.read();
    bytesRead++;
  }
  interrupts();

  for (int a = 0; a < 16; a++) {
    Serial.print(data[a], DEC);
    if (a != 15) {
      Serial.print("~");
    }
  }
  Serial.println("\n");
  ESPserial.end();
  return String((char *)data);
}

void dataSend(String d) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& rxpk = root.createNestedArray("rxpk");
  JsonObject& data = rxpk.createNestedObject();
  data["data"] = d;

  WiFi.printDiag(Serial);

  String f1 = "";
  root.printTo(f1);
  char f2[f1.length()];
  f1.toCharArray(f2, f1.length() + 1);
  Serial.print(f1);
  
  
  
  
  if (tcpClient.connect("10.1.10.142", 21)) {
    tcpClient.println(f1);
    Serial.println("\nSENT");
    tcpClient.stop(); //fix this
  }
  else {
    Serial.println("\nFailed Data Send Attempt");
  }
}

/*****************************************************************************************
 ****************** Device setup and operation management functions **********************
 ****************************************************************************************/


void deviceConfig() {
  WiFi.forceSleepWake();
  delay(1);
  WiFi.mode( WIFI_STA );
  form = htmlForm();
  networkSearchPrint();
  softAPConnect();
  while (handleSubmit()) {
    delay(25);
    server.handleClient();
  }
  Serial.println("Setup Complete");
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay(3);
}

void setup(void) {
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay(1);
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(MemResetPin, INPUT);
  ssidWifi = memRead(30, 10);
  passwordWifi = memRead(30, 110);
  if (validSSID(ssidWifi) == false) {
    deviceConfig();
  }
}

void loop(void) {
  if (digitalRead(MemResetPin) == 0) {
    memClear(ssidWifi, passwordWifi);
    ESP.restart();
  }
  String data = ICRequestData();
  Serial.println(data);
  Serial.println(ssidWifi);
  Serial.println(passwordWifi);
  if (data.charAt(15) == 255) {
    WiFi.forceSleepWake();
    delay(1);
    WiFi.mode( WIFI_STA );
    if (wifiConnect(ssidWifi, passwordWifi)) {
      dataSend(data);
      WiFi.disconnect();
    }
    WiFi.mode( WIFI_OFF );
    WiFi.forceSleepBegin();
    delay(1);
  }
  else {
    Serial.println("Data Collect Fail or Network Connection Failure");
  }
  delay(100);
}
