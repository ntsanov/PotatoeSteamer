
#include <stdint.h>

int BTN_BACK_PIN = 15;
int BTN_DOWN_PIN = 14;
int BTN_UP_PIN = 16;
int BTN_OK_PIN = 10;

struct MenuEntry{
   char name[10];
   uint16_t duration;
};

MenuEntry Menu[]={
   {
      "  Potatoe",
      30 
   },
   {
      "  Brokoli",
      17
   },
   {
      "  Carrots",
      22
   }
};