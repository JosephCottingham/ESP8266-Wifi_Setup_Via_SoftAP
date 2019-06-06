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
WiFiUDP udp;
SoftwareSerial ESPserial(5, 4); 

const int port = 65035;
//const char url[] = "10.1.10.253";
const char url[] = "ioth2o.com";
const char *APssid = APSSID;
const char *APpassword = APPSK;


String MemRead(int l, int p) {
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
boolean ValidSSID(String S){
  if (S.charAt(0) > 32 && S.charAt(0) < 122) {
    return true;
  } else {
    return false;
  }
}


void DeviceConfig(){
  networkSearchPrint();
  SoftAPConnect();  
  while(handleSubmit()){
    delay(100);
    server.handleClient();
  }
  WiFi.disconnect(true);
  Serial.println("Setup Complete");
}

void networkSearchPrint() {
  Serial.print("Scan start ... ");
  int n = WiFi.scanNetworks();
  Serial.println(n);
  Serial.println(" network(s) found");
  for (int i = 0; i < n; i++){
    Serial.println(i);
    Serial.println(WiFi.SSID(i));
  }
  Serial.println();
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
void SoftAPConnect() {
  WiFi.softAP(APssid, APpassword);
  Serial.println("");
  Serial.print("Hosting: ");
  Serial.println(APssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Whats UP :>");
}

String htmlForm(){
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
  for (int i = 0; i < WiFi.scanNetworks(); i++){
    htmlForm += "<option value=\"";
    htmlForm += WiFi.SSID(i);
    htmlForm += "\">";
    htmlForm += WiFi.SSID(i);
    htmlForm += "</option>";
  }
  htmlForm += "</select><BR>";
  htmlForm += "<INPUT type=\"text\" name=\"password\" placeholder=\"password\"><BR>";
  htmlForm += "<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">";
  htmlForm += "</P>";
  htmlForm += "</FORM>";
  htmlForm += "</body>";
  htmlForm += "</html>";
  return htmlForm;
}

boolean handleSubmit(){
  String ssidWifi = server.arg("ssid");
  String passwordWifi = server.arg("password");
  if (ssidWifi.charAt(0) > 32 && ssidWifi.charAt(0) < 122 && wifiConnect(passwordWifi, passwordWifi)) {
    writeMemory(ssidWifi, passwordWifi);
    WiFi.softAPdisconnect(true);
    return false;
  }
  else{
  return true;
  }
}
void writeMemory(String s, String p) {
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
boolean wifiConnect(String s, String p) {
  WiFi.begin(s, p);
  Serial.println("");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    i++;
    if (i == 300) {
      WiFi.disconnect();
      Serial.println("Wifi Connection Failed, Starting AP");
      return false;
    }
    return true;
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(s);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
String ICRequestData(){
  ESPserial.write(65);
  Serial.println("\nWrite");
  byte bytes_read = 0;
  byte data[16];
  delay(100);  
  noInterrupts();
  while(ESPserial.available() > 0){
    data[bytes_read] = ESPserial.read();
    bytes_read++;
  }
  interrupts();
  
  for(int a = 0; a < 16; a++){
      Serial.print(data[a], DEC);
      if(a != 15){
        Serial.print("~");
      }
  }
  Serial.println("\n");
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

  if (udp.beginPacket(url, port)) {
    udp.write(f2, sizeof(f2));
    Serial.println("\nSENT");
    udp.endPacket();
  }
  else {
    Serial.println("\nFailed Data Send Attempt");
  }
}
//Refreshes the http server if data has not been input and sent
void handleRoot() {
  if (server.hasArg("ssid")) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", htmlForm());
  }
}
void setup(void) {
  Serial.begin(115200);
  ESPserial.begin(9600);
  EEPROM.begin(512);  
  String ssidWifi = MemRead(30, 10);
  if(ValidSSID(ssidWifi) == false){  
    DeviceConfig();
  }
}

void loop(void) {
  String ssidWifi = MemRead(30, 10);
  String passwordWifi = MemRead(30, 110);
  String data = ICRequestData();
  Serial.println(data);
  if(data.charAt(0) != 0 && wifiConnect(ssidWifi, passwordWifi)){
    //dataSend(data);
  }
  else{
    Serial.println("Data Collect Fail or Network Connection Failure");
  }
  
}
