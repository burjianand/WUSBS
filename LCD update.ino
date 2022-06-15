// Date 15-06-2022
//By Anand Burji
#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

const int colorR = 128;
const int colorG = 0;
const int colorB = 0;


#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#define Flatno 102

#include <ThingerESP8266.h>
#include <ESP8266WiFi.h>
#define USERNAME "burjianand"
#define DEVICE_ID "NodeMCU"
#define DEVICE_CREDENTIAL "U0Ixg_l81AJLo0oH"
#define SSID "anand"
#define SSID_PASSWORD "burjiburji"
#define SENSOR  D5
ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

HTTPClient http;
WiFiClient wificlient;

String postData;
float sendval;
long sendval2;

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;

float calibrationFactor = 35;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void setup()
{
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);
  lcd.print("Welcome to WUSBS!");
  lcd.setCursor(0, 1);
  lcd.print("Final Year ECE");
  Serial.begin(115200);
  thing.add_wifi(SSID, SSID_PASSWORD);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, SSID_PASSWORD);

  pinMode(SENSOR, INPUT_PULLUP);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
  delay(2000);
  lcd.clear();
  lcd.home();

}

void loop()
{
  lcd.clear();
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    lcd.print("Flow:");
    lcd.print(flowRate);
    lcd.setCursor(0, 1);
    lcd.print("Total mL:");
    lcd.print(totalMilliLitres);


    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
    thing["data"] >> [](pson & out) {
      out["Flow Rate"] = flowRate;
      out["Total"] = totalMilliLitres;
    };
    thing.handle();
    thing.stream(thing["data"]);
  }
  sendval = flowRate;
  sendval2 = totalMilliLitres;
  postData = "sendval=" + String(sendval) + "&sendval2=" + String(sendval2) + "&Flatno=" + Flatno;

  http.begin(wificlient, "http://project-web-db.000webhostapp.com/dbwrite.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(postData);
  delay(1500);

  if (httpCode == 200) {
    Serial.println("Values uploaded successfully.");
    Serial.println(httpCode);
    String webpage = http.getString();
    Serial.println(webpage + "\n");
  }
  else {
    Serial.println(httpCode);
    Serial.println("Failed to upload values. \n");
    return;
  }

  http.end();
}