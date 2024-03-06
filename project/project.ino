#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

// Șiruri de caractere cu mesaje predefinite
const String SETUP_INIT = "SETUP: Initializing ESP32 dev board";
const String SETUP_ERROR = "!!ERROR!! SETUP: Unable to start SoftAP mode";
const String SETUP_SERVER_START = "SETUP: HTTP server started --> IP addr: ";
const String SETUP_SERVER_PORT = " on port: ";
const String INFO_NEW_CLIENT = "New client connected";
const String INFO_DISCONNECT_CLIENT = "Client disconnected";

int red = 0;    
int green = 255; 
int blue = 0; 

const int historySize = 10;  
struct SoundData {
  String timestamp;
  float soundLevel;
};
SoundData history[historySize];
int historyIndex = 0;
int buzzer = 0;
int BUZZER_PIN = 32;

const String HTTP_HEADER = "HTTP/1.1 200 OK\r\nContent-type:text/html\r\n\r\n";
const String HTML_WELCOME = "<h1>Welcome to my Sound Meter Project!</h1>";

const String square = "<div id='greenSquare' style='width: 50px; height: 50px; background-color: rgb(%RED%, %GREEN%, %BLUE%);; margin-top: 10px;'></div>"
                   "<meta http-equiv='refresh' content='1'>";
                         
String HTML_CODE = "";

const char *SSID = "mySSID";

const char *PASS = "myPASS123!";

const int HTTP_PORT_NO = 80;
unsigned long lastUpdateTime = 0;  
const unsigned long updateInterval = 1000; 


WiFiServer HttpServer(HTTP_PORT_NO);

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  if (!WiFi.softAP(SSID)) {
    Serial.println(SETUP_ERROR);
    
    while (1)
      ;
  }

  const IPAddress accessPointIP = WiFi.softAPIP();
  const String webServerInfoMessage = SETUP_SERVER_START + accessPointIP.toString()
                                      + SETUP_SERVER_PORT + HTTP_PORT_NO;

  HttpServer.begin();
  Serial.println(webServerInfoMessage);
}

void loop() {
  WiFiClient client = HttpServer.available();  
  if (client) {                                
    Serial.println(INFO_NEW_CLIENT);           
    String currentLine = "";                  
    while (client.connected()) {               
      if (client.available()) {               
        const char c = client.read();          
        Serial.write(c);                       
        if (c == '\n') {                       
      
          if (currentLine.length() == 0) {
            unsigned long currentTime = millis();
            if (currentTime - lastUpdateTime >= updateInterval) {
              lastUpdateTime = currentTime;
              printWelcomePage(client);
            }
            break;
          } else currentLine = "";
        } else if (c != '\r') {  
          currentLine += c;      
        }
      }
     
    }
    client.stop();
    Serial.println(INFO_DISCONNECT_CLIENT);
    Serial.println();
  }
}


void printWelcomePage(WiFiClient client) {

  client.println(HTTP_HEADER);
  client.print(HTML_WELCOME);
  
  String htmlSquare = square;
  setColorSqaure();
  htmlSquare.replace("%RED%", String(red));
  htmlSquare.replace("%GREEN%", String(green));
  htmlSquare.replace("%BLUE%", String(blue));

  client.print(htmlSquare);

  addSoundToHistory(getSoundLevel());
  client.print(getSoundTable());

  client.println();
}

void playTone(long duration, int freq) {
  duration *= 1000;
  int period = (1.0 / freq) * 1000000;
  long elapsed_time = 0;
  while (elapsed_time < duration) {
    digitalWrite(BUZZER_PIN,HIGH);
    delayMicroseconds(period / 2);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(period / 2);
    elapsed_time += (period);
}
}

float getSoundLevel() {
  int num_Measure = 128;
  int pinSignal = 34;
  long Sound_signal;
  long sum = 0;
  long level = 0;

  for (int i = 0; i < num_Measure; i++) {
    Sound_signal = analogRead(pinSignal);
    sum = sum + Sound_signal;
  }

  level = sum / num_Measure;

  float voltage = level * (3.3 / 1023.0);
  float decibels = 20 * log10(voltage / 0.001);
  
  return decibels;
}

String getSoundTable() {
   String table = "<style>"
                 "table {"
                 "  width: 80%;"
                 "  border-collapse: collapse;"
                 "}"
                 "table, th, td {"
                 "  border: 1px solid black;"
                 "}"
                 "th, td {"
                 "  padding: 8px;"
                 "  text-align: left;"
                 "}"
                 "th {"
                 "  background-color: #f2f2f2;"
                 "}"
                 "</style>";
  table += "<table border='1'>"
                           "<tr><th>Timestamp</th><th>Sound Level (dB)</th></tr>"
                           "<tbody id='soundTable'></tbody>";

  for (int i = 0; i < historySize; i++) {
    table += "<tr><td>" + history[i].timestamp + "</td><td>" + String(history[i].soundLevel) + "</td></tr>";
  }

  table += "</table>";
  table += "<meta http-equiv='refresh' content='1'>";


  return table;
}

void addSoundToHistory(float soundLevel) {
  
  String timestamp = String(millis()/1000);

  history[historyIndex].timestamp = timestamp;
  history[historyIndex].soundLevel = soundLevel;
  historyIndex = (historyIndex + 1) % historySize;
  delay(1000);
}

void setColorSqaure(){
  if(getSoundLevel() <= 35){
      green = 255;
      red = 0;
      buzzer = 0;
  } 
  else if(getSoundLevel() <= 40){
      green = 255;
      red = 120;
      buzzer = 0;
  } 
  else if(getSoundLevel() <= 50){
      green = 255;
      red = 180;
      buzzer = 0;
  } 
  else if(getSoundLevel() <= 60){
      green = 255;
      red = 255;
      buzzer = 0;
  } 
  else if(getSoundLevel() <= 70){
      green = 255;
      red = 200;
      buzzer = 1;
  } 
  else if(getSoundLevel() <= 70){
      green = 200;
      red = 255;
      buzzer = 1;
  } 
   else if(getSoundLevel() <= 80){
      green = 100;
      red = 255;
      buzzer = 1;
  } 
  else if(getSoundLevel() <= 90){
      green = 255;
      red = 255;
      buzzer = 1;
  } 
  
}
