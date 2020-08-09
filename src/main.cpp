#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "menu.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


char topRow[21];

char time_fmt[] = "%02d:%02d";

int menu_len;

int selected = 0;
// timestamp like time keeping, incremeted by timer interrupt every second
volatile uint16_t elapsed = 0; 

bool BB_P = false;
bool BD_P = false;
bool BU_P = false;
bool BO_P = false;
int val = 0;
volatile bool started = false;
uint16_t duration = 0;
int running_idx = 0;

uint16_t sec = 0;
void setup() {
   Serial.begin(9600);
   pinMode(BTN_BACK_PIN, INPUT_PULLUP);
   pinMode(BTN_UP_PIN, INPUT_PULLUP);
   pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
   pinMode(BTN_OK_PIN, INPUT_PULLUP);
   menu_len = sizeof(Menu)/sizeof(MenuEntry);
   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
   Serial.println(F("SSD1306 allocation failed"));
   for(;;); // Don't proceed, loop forever
   }
   display.clearDisplay();
   display.setTextSize(1);      // Normal 1:1 pixel scale
   display.setTextColor(WHITE); // Draw white text
   display.setCursor(0,0);             // Start at top-left corner
   display.println(F("Time to steam some potatoes!"));
   display.display();
   // clear interrupts
   cli();
   TCCR1A = 0;
   TCCR1B = 0;
   // Timer/Counter Control Register
   TCCR1B |= (1 << CS10)|(1 << CS12);    // 1024 prescaler 
   TCCR1B |= 1<<WGM12;     //Put Timer/Counter1 in CTC mode(Clear Timer on Compare Match)
   OCR1A = 15624; // Output compare register
   TIMSK1 |= 1<<OCIE1A;        //enable timer compare interrupt
   // set interrupts
   sei();
}

ISR(TIMER1_COMPA_vect)      //Interrupt Service Routine
{
   if (started) {
      elapsed++;
   }
}  

void display_menu_entry(int index) {
   display.clearDisplay();
   display.setTextSize(1); // 1 = 6x8
   display.setCursor(0,0);      // Start at top-left corner
   display.print(F("        "));
   display.print(Menu[index].duration);
   display.print(F(":00"));
   display.setCursor(0,10);
   display.setTextSize(2);      // Normal 1:1 pixel scale
   display.println(Menu[index].name);
}

void display_running_stats() {
   char t[6];
   display.clearDisplay();
   int min = elapsed / 60;
   int sec = elapsed % 60;
   display.setTextSize(1); // 1 = 6x8
   display.setCursor(0,0);      // Start at top-left corner
   display.print(Menu[running_idx].name);
   display.setCursor(15*6,0);
   sprintf(t,time_fmt, min, sec);
   display.print(t);
   display.setCursor(0,10);
   display.setTextSize(2);      // Normal 1:1 pixel scale
   int16_t remaining = duration - elapsed;
   min = remaining / 60;
   sec = remaining % 60;
   sprintf(t,time_fmt, min, sec);
   display.print(F("   "));
   display.print(t);
}

void start(int idx) {
   running_idx = idx;
   duration = Menu[idx].duration * 60;
   started = true;
}

void stop() {
   started = false;
   elapsed = 0;
}

void loop() {
   if (!digitalRead(BTN_BACK_PIN)) {
         if (!BB_P) {
            Serial.print(F("Back pressed\n"));
            BB_P = true;
         }
   } else {
      if (BB_P) {
            Serial.print(F("Back released\n"));
            BB_P = false;
            if (started) {
               stop();
            }
      }
   }
   if (!digitalRead(BTN_DOWN_PIN)) {
         if (!BD_P) {
            Serial.print(F("Down pressed\n"));
            BD_P = true;
         }
   } else {
      if (BD_P) {
            Serial.print(F("Down released\n"));
            BD_P = false;
            if (!started){
               selected--;
               if (selected < 0){
                  selected = menu_len - 1;
               }
               Serial.print(F("selected:"));
               Serial.println(selected);
            } else {
               duration -= 60;
               Serial.print(F("Duration:"));
               Serial.println(duration);
            }
      }
   }
   if (!digitalRead(BTN_UP_PIN)) {
         if (!BU_P) {
            Serial.print(F("Up pressed\n"));
            BU_P = true;
         }
   } else {
      if (BU_P) {
         BU_P = false;
         if (!started){
            Serial.print(F("Up released\n"));
            selected++;
            if (selected >= menu_len) {
               selected = 0;
            }
            Serial.print(F("selected:"));
            Serial.println(selected);
         } else {
            duration += 60;
            Serial.print(F("Duration:"));
            Serial.println(duration);
         }
      }
   }
   if (!digitalRead(BTN_OK_PIN)) {
         if (!BO_P) {
            Serial.print(F("OK pressed\n"));
            BO_P = true;
         }
   } else {
      if (!started) {
         if (BO_P) {
               Serial.print(F("OK released\n"));
               BO_P = false;
               if (!started) {
                  start(selected);
               }
         }
      }
   }
   if (elapsed > duration) {
      stop();
   }
   if (!started) {
      display_menu_entry(selected);
   } else {
      display_running_stats();
   }
  display.display();
}