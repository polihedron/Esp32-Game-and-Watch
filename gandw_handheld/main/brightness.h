#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <stdint.h>
#include <stdbool.h>

extern uint16_t brightness_display_count;

void brightness_init(void);
void brightness_read(void);
void brightness_set(bool is_up);
void brightness_display(unsigned short *framebuffer);

#endif