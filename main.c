#include "keyboard_test.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void usage() {
    printf("Usage: [led]  \n");
    printf("Usage: [lcd] \n");
    printf("Usage: [version] \n");
    printf("Usage: [status] \n");
    printf("Usage: keyboard_freeze_blue_control [operate](ON OFF)\n");
    exit(-1);
}
int main(int argc, char *argv[]) {
   
    //change_keyled_stat();
    if (argc < 2) {
        usage();
        return 1;
    }
    
    //根据参数执行不同的选项
    if (!strcmp(argv[1], "version")) {
        keyboard_init();
        get_kb_version();
        return keyboard_close();
    }

    if (!strcmp(argv[1], "status")) {
        keyboard_init();
        printf("status:%d\n", get_kb_status());
        return keyboard_close();
    }

    if(argc > 2) {
        uint8_t level = atoi(argv[2]);
        if (!strcmp(argv[1], "led")) {
            keyboard_init();
            set_led_white_level(level);
            return keyboard_close();
        }
        if (!strcmp(argv[1], "lcd")) {
            keyboard_init();
            set_lcd_level(level);
            return keyboard_close();
        }
    }
    
    //set_led_white_level(level);
    //set_lcd_level(level);
    //set_led_blue_level(level);
    //set_single_led_color(23, 0x0000,0x0019*5);
    //set_single_led_color(21, 0x0000,0x0019*5);
    return 0;
}
