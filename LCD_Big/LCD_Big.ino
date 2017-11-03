#include <Time.h>
#include <TimeLib.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <BH1750.h>

/*
** Example Arduino sketch for SainSmart I2C LCD Screen 16x2
** based on https://bitbucket.org/celem/sainsmart-i2c-lcd/src/3adf8e0d2443/sainlcdtest.ino
** by
** Edward Comer
** LICENSE: GNU General Public License, version 3 (GPL-3.0)

** This example uses F Malpartida's NewLiquidCrystal library. Obtain from:
** https://bitbucket.org/fmalpartida/new-liquidcrystal 

** Modified â€“ Ian Brennan ianbren at hotmail.com 23-10-2012 to support Tutorial posted to Arduino.cc

** Written for and tested with Arduino 1.0
**
** NOTE: Tested on Arduino Uno whose I2C pins are A4==SDA, A5==SCL

*/
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

#define RELAY_1 7
#define RELAY_2 6
#define RELAY_3 5
#define RELAY_4 4

BH1750 lightMeter;


int relays[] = { RELAY_1, RELAY_2, RELAY_3, RELAY_4 };

int n = 1;

byte newChar[8] = {
        B10000,
        B01000,
        B00100,
        B11011,
        B11011,
        B00100,
        B01000,
        B10000
};

byte fullFlower[8] = {
        B01110,
        B01110,
        B00100,
        B10101,
        B10101,
        B01110,
        B00100,
        B11111
};
uint8_t newCharIndex = 0;
unsigned long t_hour = 0, t_min = 0, t_sec = 0;

LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  Serial.println("IoT GreenHouse V0.1a");
  initPage();
  
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  // On esp8266 devices you can select SCL and SDA pins using Wire.begin(D4, D3);
  Wire.begin();
  lightMeter.begin();
  Serial.println(F("BH1750 Test"));

  // On ferme tous les relais
  initRelays();
  Serial.println(F("Relays Test"));  
  allClosed();
  delay(1000);
  allOpen();
  delay(1000);
  allClosed();

  lcd.begin (20, 4);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  Serial.println(F("LCD Test"));

  // Moisture sensor
  pinMode(A0, INPUT);
  
  // End
  delay(500);
  lcd.clear();
  Serial.println("Ready.");
}

void loop()
{
  statusPage();
  delay(1000);
}

void initPage() {
  lcd.home();
  lcd.print("-[ IoT GreenHouse ]-");
  lcd.setCursor (0, 1);
  lcd.print("V 0.1a"); 
  lcd.setCursor (0, 4);  
  lcd.print("Initializing "); 
  lcd.createChar(newCharIndex, fullFlower);
  lcd.setCursor (15, 4);
  lcd.write(newCharIndex);  
}

void homePage() {  
  displayClock();
}

void displayClock() {
  char lcd_buffer[20];
  lcd.setCursor(0, 0);
  lcd.print("Time since reset:");
  lcd.setCursor(0, 1);
  //sprintf(lcd_buffer,"%02d:%02d:%02d    ", t_hour, t_min, t_sec);
  sprintf(lcd_buffer, "%02d:%02d:%02d", hour(), minute(), second());
  lcd.print(lcd_buffer);
}

void clock() {
  unsigned long time = millis() / 1000;
  Serial.print("t_sec: "); Serial.print(t_sec); Serial.print(", t_min: "); Serial.print(t_min); Serial.print(", t_hour: "); Serial.println(t_hour);
  Serial.print("time: "); Serial.println(time);
  
  if (time != t_sec) {
    t_sec = time;
    if (t_sec >= 60) {    
      t_sec=0;
      t_min++;      
      if (t_min >= 60) {
        t_min=0;
        t_hour++;        
      }
    }
  }
  
  Serial.print("AFTER -- t_sec: "); Serial.print(t_sec); Serial.print(", t_min: "); Serial.print(t_min); Serial.print(", t_hour: "); Serial.println(t_hour);
}

uint16_t readLux(char status[]) {
  char lcd_buffer[20];
  uint16_t lux = lightMeter.readLightLevel();
  sprintf(lcd_buffer, "Light: %4d", lux);
  Serial.println(lcd_buffer);
  if (lux <= 20) {
    digitalWrite(RELAY_1, LOW);
  }
  else {
    digitalWrite(RELAY_1, HIGH);
  }
  sprintf(status, "Light : %4d R%1d%c", lux, 1, checkRelay(1));
  return lux;
}

uint16_t readMoisture(char status[]) {
  char lcd_buffer[20];
  char* state = "";
  uint16_t moist = analogRead(A0);
  int s = analogRead(A0);
  sprintf(lcd_buffer, "Moisture: %4d", s);
  Serial.println(lcd_buffer);
  if (moist < 370) {
    digitalWrite(RELAY_2, HIGH);
    state = "WET";
    Serial.println("Wet");
  }
  else if (moist >=370 && moist < 600) {
    digitalWrite(RELAY_2, HIGH);
    state = "HUM";
    Serial.println("Humid");
  }
  else if (moist >= 600) {
    digitalWrite(RELAY_2, LOW);
    state = "DRY";
    Serial.println("Dry");
  }
  sprintf(status, "Moist.: %4d R%1d%c %s", moist, 2, checkRelay(2), state);
  return s;
}

void initRelays() {
  for (int i = 0; i < sizeof(relays); i++) {    
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);
  }    
}

void allClosed() {
  for (int i = 0; i < sizeof(relays); i++) {    
    digitalWrite(i, HIGH);
  }  
}

void allOpen() {
  for (int i = 0; i < sizeof(relays); i++) {
    digitalWrite(i, LOW);
  }
}

void statusPage() {
  char lcd_buffer[20];
  lcd.home();
  lcd.print("[Status]");
  // Light
  lcd.setCursor(0, 1);
  readLux(lcd_buffer);  
  lcd.print(lcd_buffer);
  // Moisture
  lcd.setCursor(0, 2);
  readMoisture(lcd_buffer);
  lcd.print(lcd_buffer);
  // Time
  lcd.setCursor(0, 3);
  sprintf(lcd_buffer, "Time  : %02d:%02d:%02d", hour(), minute(), second());  
  lcd.print(lcd_buffer);
}

char checkRelay(int index) {
  int value = digitalRead(relays[index - 1]);
  return value == LOW ? '+' : '-';
}

