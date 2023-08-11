#include "HX711.h"
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

// HX711 circuit wiring
//const int LOADCELL_DOUT_PIN = 2;
//const int LOADCELL_SCK_PIN = 3;
// to use NodeMCU kit at D1 for SCK  and D2 for DOUT
const int LOADCELL_DOUT_PINB = 4;  // D2 nodeMCU GPIO4
const int LOADCELL_SCK_PINB = 5;   // D1 nodeMCU GPIO5
const int LOADCELL_DOUT_PIN = 14;  // D5 nodeMCU GPIO14
const int LOADCELL_SCK_PIN = 12;   // D6 nodeMCU GPIO12

// calibration data
//const long ZERO_REF = 743857;
//const long CAL_FACTOR = 50093;

HX711 scale_a;
HX711 scale_b;

#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_GIGA)
  #include <WiFi.h>
#endif
#define LED 2            // Led in NodeMCU at pin GPIO16 (D0).
const int analogInPin = A0;
int adcValue = 0;
float voltage = 0;
//float depth_cm = 0;

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

//Variables wifi_credentials
int flashPin = 0; // pushbutton connected to digital pin 0 built in flash button on nodeMCU
int statusCode;
String st;
String content;
String esid;
String epass = "";
String ezero_refa = "";
String ecal_factora = "";
String ezero_refb = "";
String ecal_factorb = "";
//Function Decalration
void ReadEPROM(void);
bool testWifi(void);
void launchWeb(void);
void setupAP(void);
void turn_on_hotspot(void);
//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = SECRET_BROKER_URL;
int        port     = SECRET_BROKER_PORT;
const char topic[]  = SECRET_BROKER_TOPIC; 


const long interval = 5000;
unsigned long previousMillis = 0;

long count = 0;
//long reading = 0;

void setup() {
  pinMode(LED, OUTPUT); 
  pinMode(flashPin,INPUT_PULLUP);  // setup flash button input
  digitalWrite(LED, HIGH); 
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  ReadEPROM();

  scale_a.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale_b.begin(LOADCELL_DOUT_PINB, LOADCELL_SCK_PINB);
   // attempt to connect to WiFi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(esid.c_str());
  Serial.println(epass.c_str());
  //while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
  //while (WiFi.begin(esid.c_str(), epass.c_str())) {
    // failed, retry
    //Serial.print(".");
   // flash_start_hotspot();
    //delay(10000);
  //}

  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
  }
  else
  {
    Serial.println("wifi.status failed will reset esp ");
    ESP.restart(); 
  }

  Serial.println("You're connected to the network");
  Serial.println();
  digitalWrite(LED, LOW);
  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword(SECRET_BROKER_USER, SECRET_BROKER_PASS);

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}

void loop() {
  float depth_cm_B = 0;
  long readingB = 0;
  float depth_cm_A = 0;
  long readingA = 0;
  float ZERO_REFA = atof(ezero_refa.c_str());
  float ZERO_REFB = atof(ezero_refb.c_str());
  float CAL_FACTORA = atof(ecal_factora.c_str());
  float CAL_FACTORB = atof(ecal_factorb.c_str());

  if (scale_a.is_ready()) {
    readingA = scale_a.read();
    //Serial.print("HX711 readingA: ");
    //Serial.println(readingA);
    depth_cm_A = (readingA + ZERO_REFA)/CAL_FACTORA;
  } else {
    //Serial.println("HX711 not found.");
  }

  if (scale_b.is_ready()) {
    readingB = scale_b.read();
    //Serial.print("HX711 readingB: ");
    //Serial.println(readingB);
    depth_cm_B = (readingB + ZERO_REFB)/CAL_FACTORB;
  } else {
    //Serial.println("HX711 not found.");
  }


  flash_start_hotspot();
  delay(1000);
  adcValue = analogRead(analogInPin);
  voltage = (adcValue *  0.069);  // calibration .06451 with 220k and 10k voltage spliter to A0
  //voltage = (adcValue * 1.00);
  //Serial.print("voltage, ");
  //Serial.println(voltage);
  //Serial.print("adcValue, ");
  //Serial.println(adcValue);
 
  // to avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;
    digitalWrite(LED,LOW); 
    // call poll() regularly to allow the library to send MQTT keep alives which
    // avoids being disconnected by the broker
    mqttClient.poll();

    Serial.println("ezero_refa");
    Serial.println(ezero_refa);
    Serial.println(ZERO_REFA);
    Serial.println(ZERO_REFB);
    Serial.println(CAL_FACTORA);
    Serial.println(CAL_FACTORB);
    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.print("sent ");
    Serial.println(count);
    Serial.println(readingA);
    Serial.print("voltage, ");
    Serial.println(voltage);
    long rssi = WiFi.RSSI();
    Serial.print("RSSI: ");
    Serial.println(rssi);
    Serial.println("depth_cm_A");
    Serial.println(depth_cm_A);
     Serial.println("depth_cm_B");
    Serial.println(depth_cm_B);
    readingA = readingA * -1;
    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print(count);
    mqttClient.print(",");
    mqttClient.print(voltage);
    mqttClient.print(",");
    mqttClient.print(rssi);
    mqttClient.print(",");
    mqttClient.print(readingA);
    mqttClient.print(",");
    mqttClient.print(depth_cm_A);
     mqttClient.print(",");
    mqttClient.print(depth_cm_B);
    mqttClient.endMessage();

    Serial.println();
    //Serial.println("wifi status: ");
    //Serial.println(WiFi.status() == WL_CONNECTED);
    if (count > 4) {
      delay(1000);
      Serial.println("count > 4");
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("wifi.status OK");
      } else {
        Serial.println("wifi.status failed will reset esp ");
        ESP.restart();
      }  
    }
    count++;
     
  }
  if (0){
  //if (voltage > 5.3) { //if voltage is bellow about 5v then we must be using usb to power it
    if (voltage < 6.1) {  // two lipo low battery voltage is 3.8 x 2 = 7.6
      Serial.print("power voltage is below 6.1 (1%), will go into mode 3 power save sleep 24 hour");
      //ESP.deepSleep(60e6);  // 60 sec    //ESP.deepSleep(600e6); //10 minutes
      //ESP.deepSleep(3600e6); // 1 hour   
      ESP.deepSleep(86400e6); //24 hours
    }

    if (voltage < 6.25) {  // two lipo low battery voltage is 3.8 x 2 = 7.6
     Serial.print("power voltage is below 6.25 (25%), will go into mode 2 power save sleep 1 hour");
     //ESP.deepSleep(600e6); //10 minutes
      ESP.deepSleep(3600e6); // 1 hour
      //ESP.deepSleep(60e6);  // 60 sec
    }
    //if (voltage < 4.31) {  // use this if hooked to usb for power
    if (voltage < 6.35) {  // two lipo low battery voltage is 3.8 x 2 = 7.6
      Serial.print("power voltage is below 6.35 (50%), will go into mode 1 power save sleep 1 minutes");
      //ESP.deepSleep(600e6); //10 minutes
      //ESP.deepSleep(3600e6); // 1 hour
      ESP.deepSleep(60e6);  // 60 sec
    }
 }

  digitalWrite(LED,HIGH);
       
}

//----------------------------------------------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
void flash_start_hotspot(void){
  int flash_val = 0;
  //int flashPin = 0;
  flash_val = digitalRead(flashPin); 
  //Serial.println(flash_val);  
  if (flash_val == 0){
    Serial.println("flash button pressed will start hotspot");
    turn_on_hotspot();
  }
}

void ReadEPROM(void){
  Serial.println("Reading EEPROM ssid");
  //String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.println("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");
  //String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.println("PASS: ");
  Serial.println(epass);

  Serial.println("Reading EEPROM zero_refa");
  //String ecode = "";
  for (int i = 96; i < 128; ++i)
  {
    ezero_refa += char(EEPROM.read(i));
  }
  Serial.println("zero_refa: ");
  Serial.println(ezero_refa);

  Serial.println("Reading EEPROM cal_factora");
  //String ecode = "";
  for (int i = 128; i < 160; ++i)
  {
    ecal_factora += char(EEPROM.read(i));
  }
  Serial.println("ecal_factora: ");
  Serial.println(ecal_factora);

  Serial.println("Reading EEPROM ezero_refb");
  //String ecode = "";
  for (int i = 160; i < 192; ++i)
  {
    ezero_refb += char(EEPROM.read(i));
  }
  Serial.println("ezero_refb: ");
  Serial.println(ezero_refb);

  Serial.println("Reading EEPROM cal_factorb");
  //String ecode = "";
  for (int i = 192; i < 224; ++i)
  {
    ecal_factorb += char(EEPROM.read(i));
  }
  Serial.println("ecal_factorb: ");
  Serial.println(ecal_factorb);
}

void turn_on_hotspot(){
  Serial.println("Turning the HotSpot On");
  WiFi.disconnect();
  setupAP();// Setup HotSpot
  Serial.println();
  Serial.println("Waiting.");
  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
}

bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    flash_start_hotspot();
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("Wifi_Credentials", "");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
}

void createWebServer()
{
  //int statusCode;
  //String st;
  //String content;
  {
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Welcome to Wifi Credentials Update page";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><br><label>PASS: </label><input name='pass' length=64> <br><label>ZERO_REFA: </label><input name='zero_refa' value='1061657.0' length=32> <br><label>CAL_FACTORA: </label><input name='cal_factora' value='50093.0' length=32> <br><label>ZERO_REFB: </label><input name='zero_refb' value='1061657.0' length=32> <br><label>CAL_FACTORB: </label><input name='cal_factorb' value='50093.0' length=32> <input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qzero_refa = server.arg("zero_refa");
      String qcal_factora = server.arg("cal_factora");
      String qzero_refb = server.arg("zero_refb");
      String qcal_factorb = server.arg("cal_factorb");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 256; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }

        Serial.println("writing eeprom qzero_refa:");
        for (int i = 0; i < qzero_refa.length(); ++i)
        {
          EEPROM.write(96 + i, qzero_refa[i]);
          Serial.print("Wrote: ");
          Serial.println(qzero_refa[i]);
        }

        Serial.println("writing eeprom qcal_factora:");
        for (int i = 0; i < qcal_factora.length(); ++i)
        {
          EEPROM.write(128 + i, qcal_factora[i]);
          Serial.print("Wrote: ");
          Serial.println(qcal_factora[i]);
        }

        Serial.println("writing eeprom qzero_refb:");
        for (int i = 0; i < qzero_refb.length(); ++i)
        {
          EEPROM.write(160 + i, qzero_refb[i]);
          Serial.print("Wrote: ");
          Serial.println(qzero_refb[i]);
        }

        Serial.println("writing eeprom qcal_factorb:");
        for (int i = 0; i < qcal_factorb.length(); ++i)
        {
          EEPROM.write(192 + i, qcal_factorb[i]);
          Serial.print("Wrote: ");
          Serial.println(qcal_factorb[i]);
        }


        EEPROM.commit();
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);
    });
  }
}
