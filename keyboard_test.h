#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include <stdint.h>

#define FREEZE_KEY_LED_NUM 0x15
int keyboard_init(void);
int get_kb_status(void);
void get_kb_version(void);
void set_lcd_onoff(uint16_t onoff);
void set_lcd_level(uint8_t level);
void set_led_color(uint16_t c_r,uint16_t c_gb);
void set_single_led_color(uint8_t led, uint16_t c_r,uint16_t c_gb);
void set_led_white_level(uint8_t level);
void set_led_blue_level(uint8_t level);
int keyboard_test(void);
void change_keyled_stat(void);

#endif
