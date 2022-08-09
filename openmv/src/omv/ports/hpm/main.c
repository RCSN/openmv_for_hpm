#include "board.h"

int main(void)
{
  board_init();
  board_init_led_pins();
  board_init_usb_pins();
}