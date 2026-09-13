///The code is adapted from https://github.com/PieSquared/TTTsaboard/blob/main/Firmware/FW.ino#L47C4-L49C7


#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_SCLK 9 // labeled SCL on the screen
#define TFT_MOSI 10 // labeled SDA on the screen
#define TFT_RST 8
#define TFT_DC 4
#define TFT_CS 5
#define TFT_BL 7
#define Buzzer 20
#define BT1 0
#define BT2 1
#define BT3 2
#define BT4 3
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

const unsigned long SETTINGS_TIME = 5000; //for the time auto confirm thing

// Fix setColRowStart() by exposing it via a subclass
class MyST7789 : public Adafruit_ST7789 {
public:
  MyST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
    : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {}
  void setOffsets(uint8_t col, uint8_t row) {
    _colstart = _colstart2 = col;
    _rowstart = _rowstart2 = row;
  }
};

MyST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//Modes
enum Mode {MODE_NORMAL, MODE_SETTING};
enum Target {TARGET_TIME, TARGET_ALARM};

Mode mode = MODE_NORMAL;
Target settingTarget = TARGET_TIME;
unsigned long lastActivityTime = 0;

//Clock states, if the power turns off it will reset because no RTC module on it
int hours = 12;
int minutes = 0;
int seconds = 0;

bool timeHasBeenSet = false;

int alarmHour = 12;
int alarmMinute = 0;

bool alarmRinging = false;
bool alarmTriggeredThisMinute = false;

bool lastBT1 = HIGH;
bool lastBT2 = HIGH;
bool lastBT3 = HIGH;
bool lastBT4 = HIGH;

unsigned long lastButtonTime = 0;
const unsigned long debounceTime = 150;

void drawStatusLine() {
    tft.fillRect(0, 0, tft.width(), 10, ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 1);

    if (alarmRinging) {
    tft.setTextColor(ST77XX_RED);
    tft.print("WAKE UP!");
    return;
    } 

    if (mode == MODE_SETTING) {
    tft.setTextColor(settingTarget == TARGET_TIME ? ST77XX_CYAN : ST77XX_YELLOW);
    tft.print(settingTarget == TARGET_TIME ? "SET TIME" : "SET ALARM");
    return;
    } 

    if (!timeHasBeenSet) {
    tft.setTextColor(ST77XX_CYAN);
    tft.print("BT1: SET TIME");
    } 
    else {
    tft.setTextColor(ST77XX_WHITE);
    tft.print("ALARM ");

    if (alarmHour < 10) tft.print("0");
    tft.print(alarmHour);
    tft.print(":");

    if (alarmMinute < 10) tft.print("0");
    tft.print(alarmMinute);
    }
    }

void  drawTime() {
    tft.fillRect(0, 12, tft.width(), tft.height() - 12, ST77XX_BLACK);
    tft.setTextSize(4);
    tft.setCursor(10, 22);

    bool editingAlarm = (mode == MODE_SETTING && settingTarget == TARGET_ALARM);
    bool editingTime  = (mode == MODE_SETTING && settingTarget == TARGET_TIME);


    int h = editingAlarm ? alarmHour : hours;
    int m = editingAlarm ? alarmMinute : minutes;


    uint16_t color = ST77XX_WHITE;
    if (alarmRinging) color = ST77XX_RED;
    else if (editingAlarm) color = ST77XX_YELLOW;
    else if (editingTime) color = ST77XX_CYAN;

    tft.setTextColor(color);
    if (h < 10) tft.print("0");
    tft.print(h);
    tft.print(":");
    if (m < 10)tft.print("0");
    tft.print(m);
}



void updateClock() {
if (mode != MODE_NORMAL) return; // frozen while a setting session is active

static unsigned long lastSecond = 0;
unsigned long now = millis();

if (now - lastSecond >= 1000) {
    lastSecond += 1000;
    seconds++;

if (seconds >= 60) {
      seconds = 0;
      minutes++;
      alarmTriggeredThisMinute = false;
    }
if (minutes >= 60) {
      minutes = 0;
      hours++;
    }
if (hours >= 24) {
      hours = 0;
    }
  }
}

void checkAlarm() {
if (!timeHasBeenSet || mode != MODE_NORMAL || alarmRinging) return;

if(hours == alarmHour && minutes == alarmMinute && !alarmTriggeredThisMinute) {
    alarmRinging = true;
    alarmTriggeredThisMinute = true;
digitalWrite(Buzzer, HIGH); 
drawStatusLine();
drawTime();
  }
}

void exitSettingMode() {
if (settingTarget == TARGET_TIME) {
    timeHasBeenSet = true;
    alarmTriggeredThisMinute = false;
  }
  mode = MODE_NORMAL;
drawStatusLine();
drawTime();
}

void checkButtons() {
bool bt1 = digitalRead(BT1);
bool bt2 = digitalRead(BT2);
bool bt3 = digitalRead(BT3);
bool bt4 = digitalRead(BT4);

unsigned long now = millis();
if (now - lastButtonTime < debounceTime) {
    lastBT1 = bt1; lastBT2 = bt2; lastBT3 = bt3; lastBT4 = bt4;
    return;
  }

bool bt1Pressed = (lastBT1 == HIGH && bt1 == LOW);
bool bt2Pressed = (lastBT2 == HIGH && bt2 == LOW);
bool bt3Pressed = (lastBT3 == HIGH && bt3 == LOW);
bool bt4Pressed = (lastBT4 == HIGH && bt4 == LOW);

if (mode == MODE_NORMAL) {
if (bt4Pressed && alarmRinging) {
      lastButtonTime = now;
      alarmRinging = false;
      digitalWrite(Buzzer, LOW);
      drawStatusLine();
      drawTime();

    } else if (bt2Pressed && !alarmRinging) {

      lastButtonTime = now;
      mode = MODE_SETTING;
      settingTarget = TARGET_ALARM;
      lastActivityTime = now;
      drawStatusLine();
      drawTime();
    } else if (bt1Pressed && !alarmRinging) {

      lastButtonTime = now;
      mode = MODE_SETTING;
      settingTarget = TARGET_TIME;
      lastActivityTime = now;
      drawStatusLine();
      drawTime();
    }
  } else { // MODE_SETTING
    int *h = (settingTarget == TARGET_TIME) ? &hours : &alarmHour;
    int *m = (settingTarget == TARGET_TIME) ? &minutes : &alarmMinute;

if (bt1Pressed) {
  //hour up
  lastButtonTime = now;
  lastActivityTime = now;
  *h = (*h + 1) % 24;
  drawTime();
    } else if (bt2Pressed) { 
      // hour down
      lastButtonTime = now;
      lastActivityTime = now;
      *h = (*h == 0) ? 23 : *h - 1;
      drawTime();
    } else if (bt3Pressed) {
      //minute up
      lastButtonTime = now;
      lastActivityTime = now;
      *m = (*m + 1) % 60;
      if (settingTarget == TARGET_TIME) seconds = 0;
      drawTime();
    } else if (bt4Pressed) {
      //minute down
      lastButtonTime = now;
      lastActivityTime = now;
      *m = (*m == 0) ? 59 : *m - 1;
      if (settingTarget == TARGET_TIME) seconds = 0;
      drawTime();
    }
  }

lastBT1 = bt1; lastBT2 = bt2; lastBT3 = bt3; lastBT4 = bt4;
}

void checkSettingTimeout() {
if (mode == MODE_SETTING && millis() - lastActivityTime >= SETTINGS_TIME) {
    exitSettingMode();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BT1, INPUT_PULLUP);
  pinMode(BT2, INPUT_PULLUP);
  pinMode(BT3, INPUT_PULLUP);
  pinMode(BT4, INPUT_PULLUP);

  pinMode(Buzzer, OUTPUT);
  digitalWrite(Buzzer, LOW); 

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);

  tft.init(76, 284);
  tft.setOffsets(82, 18);
  tft.invertDisplay(false);
  tft.setRotation(1);
  Serial.println("Screen Initialized");

  tft.fillScreen(ST77XX_BLACK);
  drawStatusLine();
  drawTime();
}

void loop() {
  //workflow thing
  updateClock();
  checkButtons();
  checkSettingTimeout();
  checkAlarm();

  static unsigned long lastDisplayUpdate = 0;
  if (mode == MODE_NORMAL && millis() - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = millis();
    drawTime();
  }
}