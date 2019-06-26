#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ESPSoftwareSerial.h>
#include <FS.h>

#ifndef APSSID
#define APSSID "H2O-WiFi-Unit"
#define APPSK  ""
#endif

ESP8266WebServer server(80);
WiFiClientSecure tcpClient;
PubSubClient client(tcpClient);
SoftwareSerial ESPserial(5, 4);

//const char *c = "-----BEGIN CERTIFICATE-----\nMIIC7jCCAdagAwIBAgIJAPxjsOZYFWheMA0GCSqGSIb3DQEBDQUAMAwxCjAIBgNV\nBAMMAS4wHhcNMTkwNjI2MTUzNTMzWhcNMzIwNjIyMTUzNTMzWjAMMQowCAYDVQQD\nDAEuMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAncIz5QZ9n5c0Wbo/\nTGX8GSejBzghP/UCpBAeeFXd/WjnNOcibtZXXS/0SSjknRgKT4C/EF3PneE7C5kQ\nYps5HDTScK1DKRTc9fbuaIjHz2PlNF+t+Hl+kjxZqgIzKQv8LgYYdMeQ1fhh3zG1\n+zy5DOJSh0E2d5vU6GoboAs6FqIQq6s1Jp47jdCZhajXiSKWxL615ipIizNuNEw2\nqqk5JiA9AAOGqRNh33F1bw2Y7YaN2py9U/nKhE9drKK76xE/NElKRK3DRnR44vJQ\ns1s+s4WOhTO6+s/yrJ5kK0GTqlluENwUSePpin2VL1KFfOKK2I5l6pF3QjVRgz9L\nc9e74QIDAQABo1MwUTAdBgNVHQ4EFgQUiSKXEQua1Mv5nDdS24wkAl/hJjQwHwYD\nVR0jBBgwFoAUiSKXEQua1Mv5nDdS24wkAl/hJjQwDwYDVR0TAQH/BAUwAwEB/zAN\nBgkqhkiG9w0BAQ0FAAOCAQEAO3MUq1BJJbsto4yxny+1VWgrDjglv5DVfRtRI7vX\noLdLWWNJJzQpKfTK+yaV9wSWCXxL4kMRkqFjyMZIIGOUyu2slGX8HiTVzMLvYy+A\nePjlZmH+0OqfLlpng8DFumSq7YHxOQmD9D5KVIzulemD6R4Sw2KoQfmcDSfRwPSb\n1iPSFKAs7gXh4lubtBuxzL8adODK1Usz9YQ/4mUP8abj7h5wu+fm7NGpCqj1qAxk\nbsKtQaVsIXh9kTNTMwhlIfA3Uqaa72UW6e5c1cHMuChVzEsqdpWBEPcNj69bkra6\n2IZ+kNjQGD4OXqUSGSV8HVnPqeouhW9uRZmCGBq8rEcGhA==\n-----END CERTIFICATE-----";
BearSSL::X509List cert("-----BEGIN CERTIFICATE-----\nMIIC7jCCAdagAwIBAgIJAPxjsOZYFWheMA0GCSqGSIb3DQEBDQUAMAwxCjAIBgNV\nBAMMAS4wHhcNMTkwNjI2MTUzNTMzWhcNMzIwNjIyMTUzNTMzWjAMMQowCAYDVQQD\nDAEuMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAncIz5QZ9n5c0Wbo/\nTGX8GSejBzghP/UCpBAeeFXd/WjnNOcibtZXXS/0SSjknRgKT4C/EF3PneE7C5kQ\nYps5HDTScK1DKRTc9fbuaIjHz2PlNF+t+Hl+kjxZqgIzKQv8LgYYdMeQ1fhh3zG1\n+zy5DOJSh0E2d5vU6GoboAs6FqIQq6s1Jp47jdCZhajXiSKWxL615ipIizNuNEw2\nqqk5JiA9AAOGqRNh33F1bw2Y7YaN2py9U/nKhE9drKK76xE/NElKRK3DRnR44vJQ\ns1s+s4WOhTO6+s/yrJ5kK0GTqlluENwUSePpin2VL1KFfOKK2I5l6pF3QjVRgz9L\nc9e74QIDAQABo1MwUTAdBgNVHQ4EFgQUiSKXEQua1Mv5nDdS24wkAl/hJjQwHwYD\nVR0jBBgwFoAUiSKXEQua1Mv5nDdS24wkAl/hJjQwDwYDVR0TAQH/BAUwAwEB/zAN\nBgkqhkiG9w0BAQ0FAAOCAQEAO3MUq1BJJbsto4yxny+1VWgrDjglv5DVfRtRI7vX\noLdLWWNJJzQpKfTK+yaV9wSWCXxL4kMRkqFjyMZIIGOUyu2slGX8HiTVzMLvYy+A\nePjlZmH+0OqfLlpng8DFumSq7YHxOQmD9D5KVIzulemD6R4Sw2KoQfmcDSfRwPSb\n1iPSFKAs7gXh4lubtBuxzL8adODK1Usz9YQ/4mUP8abj7h5wu+fm7NGpCqj1qAxk\nbsKtQaVsIXh9kTNTMwhlIfA3Uqaa72UW6e5c1cHMuChVzEsqdpWBEPcNj69bkra6\n2IZ+kNjQGD4OXqUSGSV8HVnPqeouhW9uRZmCGBq8rEcGhA==\n-----END CERTIFICATE-----");

const char *APssid = APSSID;
const char *APpassword = APPSK;
int MemResetPin = 13;
String form = "";
String ssidWifi;
String passwordWifi;
const char* mqttServer = "10.1.10.53";
const int mqttPort = 8883;
const char* mqttUser = "";
const char* mqttPassword = "";

String MemRead(int l, int p) {
  String temp;
  for (int n = p; n < l + p; ++n) {
    if (char(EEPROM.read(n)) != ';') {
      if (isWhitespace(char(EEPROM.read(n)))) {}
      else temp += String(char(EEPROM.read(n)));
    } else n = l + p;
  }
  return temp;
}

void loadCerts() {
   if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  Serial.println("spiffs val haskdjhfjksadfhkjsadfjksahdfkjwsdjkfhdasjkdfkasdhfkashdkfjhsakdjfhskadhfksadhfksahdfkshdkfjhsakdfha");
  Serial.println(SPIFFS.exists("/CA.crt"));
//  // Load client certificate file from SPIFFS
//  File cert = SPIFFS.open("/esp.der", "r"); //replace esp.der with your uploaded file name
//  if (!cert) {
//    Serial.println("Failed to open cert file");
//  }
//  else {
//    Serial.println("Success to open cert file");
//  }
//
//  delay(1000);
//
//  // Set client certificate
//  if (wifiClient.loadCertificate(cert)) {
//    Serial.println("cert loaded");
//  }
//  else {
//    Serial.println("cert not loaded");
//  }
//
//  // Load client private key file from SPIFFS
//  File private_key = SPIFFS.open("/espkey.der", "r"); //replace espkey.der with your uploaded file name
//  if (!private_key) {
//    Serial.println("Failed to open private cert file");
//  }
//  else {
//    Serial.println("Success to open private cert file");
//  }
//
//  delay(1000);
//
//  // Set client private key
//  if (wifiClient.loadPrivateKey(private_key)) {
//    Serial.println("private key loaded");
//  }
//  else {
//    Serial.println("private key not loaded");
//  }

  // Load CA file from SPIFFS
  File ca = SPIFFS.open("/CA.crt", "r");
  if (!ca) {
    Serial.println("Failed to open ca ");
  }
  else {
    Serial.println("Success to open ca");
  }
  
  delay(1000);
  
  // Set server CA file
  if(tcpClient.loadCACert(ca)) {
    Serial.println("ca loaded");
  }
  else {
    Serial.println("ca failed");
  }
  SPIFFS.end();
}

//Checks the first char of a String to see if its valid ASCII
boolean ValidSSID(String S) {
  if (S.charAt(0) > 32 && S.charAt(0) < 122) {
    return true;
  } else {
    return false;
  }
}

void DeviceConfig() {
  WiFi.forceSleepWake();
  delay(1);
  WiFi.mode( WIFI_STA );
  form = htmlForm();
  networkSearchPrint();
  SoftAPConnect();
  while (handleSubmit()) {
    delay(25);
    server.handleClient();
  }
  Serial.println("Setup Complete");
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay(1);
}

//Searches for available networks and then prints them out
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

void handleNotFound() {
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
  if (ValidSSID(ssidWifi)) {
    if (wifiConnect(ssidWifi, passwordWifi)) {
      WiFi.softAPdisconnect(true);
      writeMemory(ssidWifi, passwordWifi);
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
  WiFi.persistent( false );
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

String ICRequestData() {
  ESPserial.begin(9600);
  ESPserial.write(65);
  Serial.println("\nWrite");
  byte bytes_read = 0;
  byte data[16];
  delay(100);
  noInterrupts();
  while (ESPserial.available() > 0) {
    data[bytes_read] = ESPserial.read();
    bytes_read++;
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
  Serial.print(f1);
  f1.toCharArray(f2, f1.length() + 1);
  Serial.print(f1);

  client.setServer(mqttServer, mqttPort);
  int l = 0; // l is the value that will timeout the MQTT connection after a certain number of tries
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.println(client.state());
    }
    if(l >= 5) break;
    l++;
    delay(500);
  }
  client.publish("Heartbeat", f2);

  //  client.publish("Heartbeat", f1);
//  if (tcpClient.connect("10.1.10.142", 21)) {
//    tcpClient.println(f1);
//    Serial.println("\nSENT");
//    tcpClient.stop(); //fix this
//  }
//  else {
//    Serial.println("\nFailed Data Send Attempt");
//  }
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
  writeMemory(clearS, clearP);
}

//Turns WiFi off and reads ssid and password from EEPROM, then calls DeviceConfig if the ssid is valid
void setup(void) {
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay(1);
  Serial.begin(115200);
  EEPROM.begin(512);
  writeMemory("RedSparrow", "solutions");
  pinMode(MemResetPin, INPUT);
  ssidWifi = MemRead(30, 10);
  passwordWifi = MemRead(30, 110);
  if (ValidSSID(ssidWifi) == false) {
    DeviceConfig();
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
//      loadCerts();
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
