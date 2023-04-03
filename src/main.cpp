#include "Arduino.h"
#include <NewPing.h>
#include <Servo.h>

#define BLYNK_TEMPLATE_ID "TEmplate_ID"
#define BLYNK_TEMPLATE_NAME "Smart Trash"
#define BLYNK_AUTH_TOKEN "Auth_Token"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = ".";
char pass[] = "12345678";

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET LED_BUILTIN // Reset pin #
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIGGER_PIN_OUT D5
#define ECHO_PIN_OUT D6
#define MAX_DISTANCE_OUT 30

#define TRIGGER_PIN_IN D7
#define ECHO_PIN_IN D8
#define MAX_DISTANCE_IN 16

NewPing sonarOut(TRIGGER_PIN_OUT, ECHO_PIN_OUT, MAX_DISTANCE_OUT);
NewPing sonarIn(TRIGGER_PIN_IN, ECHO_PIN_IN, MAX_DISTANCE_IN);
Servo myservo;
BlynkTimer timer;

bool isOpen = false;
int trashLevel = 0;

void displayTrash(int trash)
{
  String status;
  if (isOpen)
  {
    status = "Open";
  }
  else
  {
    status = "Close";
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Trash level");

  display.setCursor(70, 0);
  display.print("[" + status + "]");

  display.fillRect(0, 10, trash, 40, WHITE);

  display.setCursor(0, 55);
  display.print("Trash: ");
  display.print(trash);
  display.println("%");

  display.display();
}

void openTrash(bool open)
{
  Serial.println("openTrash:" + String(open));
  isOpen = open;
  displayTrash(trashLevel);
  Blynk.virtualWrite(V0, open);

  if (open)
  {
    myservo.write(180);
    delay(5000);
  }
  else
  {
    myservo.write(0);
    delay(1000);
  }
}
void show(String msg)
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(10, 24);
  display.println(msg);
  display.display();
}

void checkTrashLevel()
{
  long distance = sonarIn.ping_cm();
  Serial.println("IN:" + String(distance));
  int level = map(distance, MAX_DISTANCE_IN, 1, 0, 100);
  if (!isOpen)
  {
    trashLevel = level;
  }

  displayTrash(trashLevel);

  Blynk.virtualWrite(V1, trashLevel);

  Serial.println("Trash level: " + String(trashLevel));
}
void checkOpenTrash()
{
  long distance = sonarOut.ping_cm();
  Serial.println("OUT:" + String(distance));
  if (distance < MAX_DISTANCE_OUT && distance > 0)
  {
    openTrash(true);
  }
  else
  {
    openTrash(false);
  }
}

void setup()
{
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // เริ่มต้นใช้งานหน้าจอ OLED
  show("Starting...");

  myservo.attach(D0); // กำหนดขา 9 ควบคุม Servo
  myservo.write(0);   // กำหนดให้ Servo หมุนไปที่องศา 0
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(100L, checkTrashLevel);
  timer.setInterval(100L, checkOpenTrash);
}

void loop()
{

  Blynk.run();
  timer.run();
}

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable

  openTrash(pinValue);
}
