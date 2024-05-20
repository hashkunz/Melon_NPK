// #include "DFRobot_SHT20.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include "DHT.h"
#include <WiFiManager.h> 
#include "ThingSpeak.h" 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)

#define RE 2 
#define DE 0
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
WiFiClient  client;
WiFiUDP ntpUDP;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// หน่วยเป็นวินาที จะได้ 7*60*60 = 25200
const long offsetTime = 25200; 
//const byte code[]= {0x01, 0x03, 0x00, 0x1e, 0x00, 0x03, 0x65, 0xCD};
const byte nitro[] = {0x01,0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01,0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01,0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};
NTPClient timeClient(ntpUDP, "pool.ntp.org", offsetTime);

byte values[11];
SoftwareSerial mod(14,12);

unsigned long myChannelNumber = xxxxxxx;
const char * myWriteAPIKey = "writeapikey";

int S1 = 34, H1; //

String myStatus = "";
int hour;
int minute;
int second;
int count =0;
bool set = false;



void setup() {
  Serial.begin(115200);
  setupWiFi();
  Serial.println("Serial begin");
  Serial.println("SHT20 Example!");
  dht.begin();
  delay(100);


// Set delay between sensor readings based on sensor details.
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  
  timeClient.begin();
  timeClient.update();

  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  second = timeClient.getSeconds();
  Serial.println(F("Time Begin : "));
  Serial.println(timeClient.getFormattedTime());

  mod.begin(4800);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  delay(500);
  display.clearDisplay();
  display.setCursor(25, 15);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(" NPK Sensor");
  display.setCursor(25, 35);
  display.setTextSize(1);
  display.print("Initializing");
  display.display();
  delay(3000);
}

void loop() {
  timeClient.update();

  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  second = timeClient.getSeconds();
  Serial.println(F("Time Begin : "));
  Serial.println(timeClient.getFormattedTime());
  Serial.printf("%d : %d : %d \n", hour, minute, second);


  // float  = sht20.readHumidity(); // Read Humidity
  // float  = sht20.readTemperature(); // Read Temperature
  float Hum_DHT = dht.readHumidity();
  float Tem_DHT = dht.readTemperature();
  float f = dht.readTemperature(true);
  int Hum_Soil;

  // if (isnan(Hum_DHT) || isnan(Tem_DHT) || isnan(f)) {
  //   Serial.println(F("Failed to read from DHT sensor!"));
  //   return;
  // }

  H1 = analogRead(S1);
  Serial.println(String("Default Humi1 : ") + H1);
  Hum_Soil = map(H1, 3000, 1200, 0,100);
  Serial.printf("Hum_Soil : %d", Hum_Soil);
  Serial.println("%");
  Serial.printf("Hum_DHT : %.2f", Hum_DHT);
  Serial.println("%");
  Serial.printf("Tem_DHT : %.2f", Tem_DHT);
  Serial.println("C");
  Serial.println();
  npksensor(); //เรียกใช้ฟังชั้น npksensor

  ThingSpeak.setField(1, Hum_Soil);
  ThingSpeak.setField(2, Hum_DHT);
  ThingSpeak.setField(3, Tem_DHT);

  ThingSpeak.setStatus(myStatus);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  delay(20000);
}
  
void npksensor() {
  byte val1,val2,val3;
  val1 = nitrogen();
  delay(250);
  val2 = phosphorous();
  delay(250);
  val3 = potassium();
  delay(250);
  
  
  Serial.print("Nitrogen: ");
  Serial.print(val1);
  Serial.println(" mg/kg");
  Serial.print("Phosphorous: ");
  Serial.print(val2);
  Serial.println(" mg/kg");
  Serial.print("Potassium: ");
  Serial.print(val3);
  Serial.println(" mg/kg");
  delay(2000);
 
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print("N: ");
  display.print(val1);
  display.setTextSize(1);
  display.print(" mg/kg");
 
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print("P: ");
  display.print(val2);
  display.setTextSize(1);
  display.print(" mg/kg");
 
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print("K: ");
  display.print(val3);
  display.setTextSize(1);
  display.print(" mg/kg");
 
  display.display();
}

void setupWiFi() {
       WiFiManager wm;
    bool res;
      res = wm.autoConnect("Melon Box ", "12345678"); // password protected ap
    if(!res) {
        Serial.println("Failed to connect");     
    } 
    else {
        Serial.println("connected...yahooooooooooooooooooooooooooooo!");
    }
}

 
byte nitrogen(){
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mod.write(nitro,sizeof(nitro))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mod.read();
    //Serial.print(values[i],HEX);
    }
    //Serial.println();
  }
  return values[4];
}
 
byte phosphorous(){
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mod.write(phos,sizeof(phos))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mod.read();
    //Serial.print(values[i],HEX);
    }
    //Serial.println();
  }
  return values[4];
}
 
byte potassium(){
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mod.write(pota,sizeof(pota))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mod.read();
    //Serial.print(values[i],HEX);
    }
    Serial.println();
  }
  return values[4];
}
