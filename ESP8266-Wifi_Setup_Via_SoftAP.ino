#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>

#ifndef APSSID
#define APSSID "H2O-WiFi-Unit"
#define APPSK  ""
#endif
const int port = 65035;
//const char url[] = "10.1.10.253";
const char url[] = "ioth2o.com";
const char *APssid = APSSID;
const char *APpassword = APPSK;
String String_ssidVal = "";
String String_passwordVal;
String String_ssidVal1 = "";
String String_passwordVal1;
String attemptConnection = "1";
String valueText = "none";
boolean wifiConnectAttempt = false;
boolean APOpen = false;
boolean WifiOpen = false;
byte data[16];
int mainLoop = 0;
int ssidAddress = 10;
int passAddress = 50;
int cycle = 0;
int n;
char Char_ssidVal[20];
char Char_passVal[20];
String htmlForm;
ESP8266WebServer server(80);
WiFiUDP udp;
SoftwareSerial ESPserial(5, 4); // RX | TX

//MemRead is a string value that is pulled from the flash of the esp
//This value is going to hold both the ssid and password.
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

//This is a char array that will hold the html form code to be hosted on the AP's
//http server
void form(){
  htmlForm = "<!DOCTYPE HTML>";
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
  for (int i = 0; i < n; i++){
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
}
void networkSearch() {
  Serial.print("Scan start ... ");
  n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.println(n);
  Serial.println(" network(s) found");
  for (int i = 0; i < n; i++){
    Serial.println(i);
    Serial.println(WiFi.SSID(i));
  }
  Serial.println();
}

void dataSend() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& rxpk = root.createNestedArray("rxpk");
  JsonObject& data = rxpk.createNestedObject();
  data["time"] = cycle;
  data["datr"] = "125000";
  data["codr"] = "5";
  data["lsnr"] =  "11.5";
  data["rssi"] = "-47";
  data["freq"] = "433920000";
  data["data"] = "aG9tZSBoYXMgam9pbmVkIHRoZSBjaGF0IQ==";

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
    server.send(200, "text/html", htmlForm);
  }
}

//pulls the data from the http server, disconnects the ap, and starts connection to local wifi network
void handleSubmit()
{
  String_ssidVal = server.arg("ssid");
  String_passwordVal = server.arg("password");
  valueText = String_ssidVal + " : " + String_passwordVal;
  write_to_Memory(String_ssidVal, String_passwordVal);
  char WiFiSave = (String_ssidVal.charAt(0));
  if (WiFiSave > 33) {
    attemptConnection = "1";
    WiFi.softAPdisconnect(true);
    wifiConnect();
  }
}
// handles the attempt to connect with a network and ends attempt if fails after 5 minutes
void wifiConnect() {
  WiFi.begin(String_ssidVal, String_passwordVal);
  Serial.println("");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    i++;
    if (i == 300) {
      WiFi.disconnect();
      Serial.println("Wifi Connection Failed, Starting AP");
      SoftAPConnect();
      break;
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(String_ssidVal);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


void returnOK()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK\r\n");
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
}
void WifiAuthConfig() {
  String_ssidVal = MemRead(30, 10);
  String_passwordVal = MemRead(30, 110);
}
//Manages the writing of strings to memory
void write_to_Memory(String s, String p) {
  s += ";";
  write_EEPROM(s, 10);
  p += ";";
  write_EEPROM(p, 110);
  EEPROM.commit();
}
//write to memory
void write_EEPROM(String x, int pos) {
  for (int n = pos; n < x.length() + pos; n++) {
    EEPROM.write(n, x[n - pos]);
  }
}
void ICRequestData(){
  ESPserial.write(65);
  Serial.println("Write");
  byte bytes_read = 0;
  Serial.println(ESPserial.available());
  while(bytes_read < 16){
    Serial.println(ESPserial.available());
    if(ESPserial.available() > 0){
      data[bytes_read] = ESPserial.read();
      bytes_read++;
    }
  }
  Serial.println(ESPserial.available());
  for(int asdf = 0; asdf < 16; asdf++){
    Serial.print(data[asdf]);
    Serial.print(",");
  }
  //Serial.println("ESPserial Unavilable");
}
void setup(void) {
  Serial.begin(115200);
  ESPserial.begin(9600);
  EEPROM.begin(512);
  WifiAuthConfig();
  Serial.println(String_ssidVal);
  Serial.println(String_passwordVal);
  networkSearch();
  form();
  Serial.println(String_ssidVal);
  Serial.println(String_ssidVal.length());
  pinMode(14, OUTPUT);
  char WiFiSave = (String_ssidVal.charAt(0));
  if (WiFiSave < 33) {
    wifiConnect();
  } else {
    SoftAPConnect();
  }
  //dataSend();
}

void loop(void) {
  form();
  server.handleClient();
  if((cycle%10000) == 0){
    Serial.println("");
    Serial.print("IP Address:");
    Serial.println(WiFi.localIP());
    Serial.print("Cycle Num:");
    Serial.println(cycle);
    ICRequestData();  
  }
  //dataSend();
  cycle++;
}
