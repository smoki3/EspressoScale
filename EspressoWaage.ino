#include <Arduino.h>
#include <Wire.h>
#include <HX711_ADC.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define OLED_RESET -1 // we don't have a reset, but the constructor expects it
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

const char* ssid = "Espresso";
const char* password = "CoffeeLove";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.4.1:70/post";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
HX711_ADC LoadCell(0, 2);
byte ModePin = 14;
byte PowerPin = 16;
byte TaraPin = 12;
byte TimerPin = 13;
float weight = 0;
boolean actMode, oldMode, actPower, oldPower, actTara, oldTara, actTimer, oldTimer, Mode, Timer, TimerStopped, Tara, Power, WeightTimer;
float stopTime = 0, startWeight = 0, checkWeight = 0, diffWeight = 0;
int minute = 0, sekunde = 0, stoppedTime = 0, startTime = 0;
long powerTimer = 0;
byte changeMode = 0;


// 'Bildschirmfoto 2020-08-23 um 12', 128x32px
const unsigned char startlogo [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfc, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x7f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x7f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x9f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x9f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x9f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x8f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x9f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x9f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x9f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x9f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// 'Bildschirmfoto 2020-08-23 um 12', 16x16px
const unsigned char coffe [] PROGMEM = {
  0x00, 0x40, 0x06, 0xe0, 0x0e, 0xf0, 0x1e, 0xf8, 0x1e, 0xf8, 0x1f, 0x7c, 0x3f, 0x7c, 0x3f, 0x3c,
  0x3f, 0x3c, 0x3f, 0x3c, 0x1f, 0x3c, 0x1f, 0x78, 0x1f, 0x78, 0x0f, 0x70, 0x06, 0x60, 0x00, 0x00
};


// 'Bildschirmfoto 2020-08-23 um 13', 16x16px
const unsigned char clocklogo [] PROGMEM = {
  0x00, 0x00, 0x03, 0xc0, 0x00, 0x80, 0x13, 0xe4, 0x2c, 0x10, 0x18, 0xc8, 0x10, 0xe4, 0x20, 0xf4,
  0x20, 0xf2, 0x20, 0x72, 0x20, 0x12, 0x20, 0x04, 0x10, 0x04, 0x08, 0x08, 0x06, 0x30, 0x00, 0x80
};


void setup()   {
  Serial.begin(9600);
  pinMode(ModePin, INPUT_PULLUP);
  pinMode(PowerPin, INPUT_PULLUP);
  pinMode(TimerPin, INPUT_PULLUP);
  pinMode(TaraPin, INPUT_PULLUP);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C
  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.drawBitmap(0, 0, startlogo, 128, 32, WHITE);
  display.display();
  LoadCell.begin();
  LoadCell.start(2000);
  LoadCell.setCalFactor(1065.52);
  Timer = false;
  Mode = false;
  Tara = false;
  Power = false;
  TimerStopped = false;
  WeightTimer = false;
  powerTimer = millis();
  }


void loop()
{
  buttoncheck();

  // Gewichtsmessung
  LoadCell.update();
  weight= LoadCell.getData();
  displayWeight(weight);
  display.drawBitmap(0, 16, clocklogo, 16, 16, WHITE);
  display.setCursor(18,18);

  if (Timer == true)    // Stoppuhr
  {
    powerTimer = millis();
    if (startTime == 0 || TimerStopped == true)
    {
      startTime = millis();
      TimerStopped = false;
    }
    sekunde = ((stoppedTime + (millis() - startTime)) / 1000) % 60;
    minute = ((stoppedTime + (millis() - startTime)) / 60000) % 60;
    display.print(minute);
    display.setTextSize(1);
    display.print("m ");
    display.setTextSize(2);
    if (sekunde<10) display.print("0");
    display.print(sekunde);
    stopTime = ((stoppedTime + (millis() - startTime)));
  }
  else if (Timer == false)
  {
      TimerStopped = true;
      stoppedTime = stopTime;
      sekunde = (stoppedTime / 1000) % 60;
      minute = (stoppedTime / 60000) % 60;
      display.print(minute);
      display.setTextSize(1);
      display.print("m ");
      display.setTextSize(2);
      if (sekunde<10) display.print("0");
      display.print(sekunde);
  }

  if (changeMode == 0) //Mode Switch
  {
    // Code für Mode 1 (Manual Mode)
  }

  else if (changeMode == 1)
  { // Auto Start Stop Mode
    if (weight > 0.2 && WeightTimer == false)
    {
      Timer = true;
      WeightTimer = true;
    }
    if ((millis() - startWeight) > 500)
    {
      diffWeight = checkWeight - weight;
      Serial.println(fabs(diffWeight));
      if (fabs(diffWeight) <= 0.2 && weight > 5) // 0.12 zu klein 
      {
        Timer = false;
      }
      checkWeight = weight;
      startWeight = millis();
    }
  }

  else if (changeMode == 2)
  {
    if (weight > 15 && WeightTimer == false)
    {
      if ((millis() - startWeight) > 500)
      {
      diffWeight = checkWeight - weight;
      Serial.println(fabs(diffWeight));
      if (fabs(diffWeight) <= 0.05 && weight > 3)
      {
        doTara();
        display.clearDisplay();
        display.display();
        delay(100);
        changeMode = 1;
      }
      checkWeight = weight;
      startWeight = millis();
      }
    }
  }

  else if (changeMode == 3)
  {
    // Code für Mode 2 Auto Tara
    if (weight > 15 && WeightTimer == false)
    {
      if ((millis() - startWeight) > 500)
      {
      diffWeight = checkWeight - weight;
      Serial.println(fabs(diffWeight));
      if (fabs(diffWeight) <= 0.05 && weight > 3)
      {
        doTara();
        display.clearDisplay();
        display.display();
        delay(100);
        Timer = true;
        WeightTimer = true;
      }
      checkWeight = weight;
      startWeight = millis();
      }
    }

  }

  else if (changeMode == 4)
  {
    // Code für Mode 2 Auto Tara
    if (weight > 15 && WeightTimer == false)
    {
      if ((millis() - startWeight) > 500)
      {
      diffWeight = checkWeight - weight;
      Serial.println(fabs(diffWeight));
      if (fabs(diffWeight) <= 0.05 && weight > 3)
      {
        doTara();
        display.clearDisplay();
        display.display();
        delay(100);
      }
      checkWeight = weight;
      startWeight = millis();
      }
    }
  }

  if (Mode == true) // Call Modes
  {
    changeModi();
    Mode = false;
  }

  if (Tara == true) // Request Tara
  {
    display.clearDisplay();
    display.setCursor(10,4);
    display.setTextSize(4);
    display.println("TARE");
    display.display();
    doTara();
    Tara = false;
  }
  display.setTextSize(1);
  display.println("s");
  display.setTextSize(1);
  display.setCursor(100,16);
  display.println("MODE");
  display.setCursor(110,24);
  display.println(changeMode);
  display.display();

  if ((millis() - powerTimer) > 300000)
  {
    display.clearDisplay();
    display.setCursor(0,4);
    display.setTextSize(4);
    display.print("SLEEP");
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    delay(50);
    ESP.deepSleep(0);
  }
  if (stopTime > 360000)
  {
    Timer = false;
  }
}

void changeModi()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("MODE ");

  if (changeMode == 0)
  {
    changeMode = 1;
  }
  else if (changeMode == 1)
  {
    changeMode = 2;
  }
  else if (changeMode == 2)
  {
    changeMode = 3;
  }
  else if (changeMode == 3)
  {
    changeMode = 4;
  }
  else if (changeMode == 4)
  {
    changeMode = 0;
  }

  display.println(changeMode);
  display.setTextSize(1);
    if (changeMode == 0)
  {
    display.println("Manual Mode");
  }
  else if (changeMode == 1)
  {
    display.println("Auto Timer");
  }
  else if (changeMode == 2)
  {
    display.println("Auto Tara & Timer");
  }
  else if (changeMode == 3)
  {
    display.println("Auto Tara,Start Timer");
  }
  else if (changeMode == 4)
  {
    display.println("Auto Tara");
  }
  display.display();
  doTara();
}

void displayWeight(float weight){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.drawBitmap(0, 0, coffe, 16, 16, WHITE);
  display.setCursor(18,0);
  if (abs(weight)>0.1) display.print(weight,1);
  if (abs(weight)<=0.1) display.print("0");
  display.setTextSize(1);
  display.println("g");
  display.setTextSize(2);
}

void buttoncheck()
{
  oldMode = actMode;
  oldPower = actPower;
  oldTimer = actTimer;
  oldTara = actTara;
  actMode = digitalRead(ModePin);
  actPower = digitalRead(PowerPin);
  actTara = digitalRead(TaraPin);
  actTimer = digitalRead(TimerPin);

  if (oldMode && !actMode)   // wenn der Taster gedrückt wurde...
  {
    //Mode = !Mode;
    wifiConnect();
    sendData();
    delay(50);
    delay(30);  // einfaches Entprellen
    powerTimer = millis();
  }

  if (!oldMode && actMode)   // wenn der Taster losgelassen wurde...
  { // was passieren soll, wenn der Taster losgelassen wird könnte hier stehen
    delay(30);  // einfaches Entprellen
  }

  if (oldPower && !actPower)   // wenn der Taster gedrückt wurde...
  {
    Power = !Power;
    delay(30);  // einfaches Entprellen
    powerTimer = millis();
  }

  if (!oldPower && actPower)   // wenn der Taster losgelassen wurde...
  { // was passieren soll, wenn der Taster losgelassen wird könnte hier stehen
    delay(30);  // einfaches Entprellen
  }

  if (oldTimer && !actTimer)   // wenn der Taster gedrückt wurde...
  {
    Timer = !Timer;
    delay(30);  // einfaches Entprellen
    powerTimer = millis();
  }

  if (!oldTimer && actTimer)   // wenn der Taster losgelassen wurde...
  { // was passieren soll, wenn der Taster losgelassen wird könnte hier stehen
    delay(30);  // einfaches Entprellen
  }

    if (oldTara && !actTara)   // wenn der Taster gedrückt wurde...
  {
    Tara = !Tara;
    delay(30);  // einfaches Entprellen
    powerTimer = millis();
  }

  if (!oldTara && actTara)   // wenn der Taster losgelassen wurde...
  { // was passieren soll, wenn der Taster losgelassen wird könnte hier stehen
    delay(30);  // einfaches Entprellen
  }
}

void doTara()
{
  Timer = false;
  LoadCell.tareNoDelay();
  delay(50);
  startTime = 0;
  stopTime = 0;
  TimerStopped = false;
  stoppedTime = 0;
  checkWeight = 0;
  WeightTimer = false;
  diffWeight= 0;
  powerTimer = millis();
  while(LoadCell.getTareStatus()== false)
  {
    LoadCell.update();
    delay(150);
  }
}

void wifiConnect(){
	  WiFi.mode(WIFI_STA);
	  WiFi.begin(ssid, password);
	  Serial.println("Connecting");
    int startconnect = millis();
	  while(WiFi.status() != WL_CONNECTED && (millis() - startconnect < 4000 )) {
	    delay(500);
	    Serial.print(".");
	  }
	  Serial.println("");
	  Serial.print("Connected to WiFi network with IP Address: ");
	  Serial.println(WiFi.localIP());
}

void sendData(){
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      String daten = String(weight,1);
      // Your Domain name with URL path or IP address with path
      http.begin(serverName);

      // If you need an HTTP request with a content type: text/plain
      http.addHeader("Content-Type", "text/plain");
      int httpResponseCode = http.POST(daten);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    WiFi.mode(WIFI_OFF);
    delay(50);
}
