#include "brightness.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "esp_err.h"
#include <gw_system.h>
#include "nvs.h"

#define BRIGHTNESS_LEVEL_MIN     1
#define BRIGHTNESS_LEVEL_MAX     10
#define BRIGHTNESS_DEFAULT       5
#define BRIGHTNESS_GPIO       GPIO_NUM_15
#define BRIGHTNESS_MODE       LEDC_LOW_SPEED_MODE
#define BRIGHTNESS_CHANNEL    LEDC_CHANNEL_0
#define BRIGHTNESS_TIMER      LEDC_TIMER_0
#define BRIGHTNESS_FREQ_HZ    5000
#define BRIGHTNESS_DUTY_RES   LEDC_TIMER_10_BIT

uint16_t brightness_display_count = 0;
static uint8_t brightness_level = 7;
static uint32_t brightness_time = 0;

static uint16_t brightness_low_color = BYTE_SWAP ? 0x0F00 : 0x000F;        // Dark blue

static uint16_t brightness_medium_color = BYTE_SWAP ? 0xE0FF : 0xFFE0;       // Yellow

static uint16_t brightness_high_color = BYTE_SWAP ? 0xFFFF : 0xFFFF;       // White



void brightness_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode       = BRIGHTNESS_MODE,
        .timer_num        = BRIGHTNESS_TIMER,
        .duty_resolution  = BRIGHTNESS_DUTY_RES,
        .freq_hz          = BRIGHTNESS_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num       = BRIGHTNESS_GPIO,
        .speed_mode     = BRIGHTNESS_MODE,
        .channel        = BRIGHTNESS_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = BRIGHTNESS_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}


void brightness_apply(void)
{
    uint32_t duty;

    if (brightness_level == 0) {
        duty = 1023;
    } else {
        duty = 1023 - ((uint32_t)brightness_level * 1023) / BRIGHTNESS_LEVEL_MAX;
    }

    ESP_ERROR_CHECK(
        ledc_set_duty(BRIGHTNESS_MODE, BRIGHTNESS_CHANNEL, duty)
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(BRIGHTNESS_MODE, BRIGHTNESS_CHANNEL)
    );
}



void brightness_read(void)
{
    brightness_level =
        nvs_get_value("br_level", BRIGHTNESS_DEFAULT);

    if (brightness_level < BRIGHTNESS_LEVEL_MIN ||
        brightness_level > BRIGHTNESS_LEVEL_MAX) {

        brightness_level = BRIGHTNESS_DEFAULT;
    }

    brightness_apply();
}


void brightness_save(void)
{
    nvs_set_value("br_level", brightness_level);
}


void brightness_set(bool is_up)
{
    uint32_t current_time = esp_timer_get_time() / 1000;

    if (current_time - brightness_time > 200) {

        if (is_up) {
            if (brightness_level < 10) {
                brightness_level++;
            }
        } else {
            if (brightness_level > 0) {
                brightness_level--;
            }
        }

        brightness_apply();

        brightness_time = current_time;
        brightness_display_count = 255;

        nvs_set_value("br_level", brightness_level);
    }
}

static uint16_t brightness_get_segment_colour(uint8_t seg_number)
{

	if (seg_number > 3 && seg_number < 8) { //4, 5, 6, 7
		return brightness_medium_color;
	}
	else if (seg_number > 7) { // 8, 9, 10
		return brightness_high_color;
	}
	else {
		return brightness_low_color;
	}

}


void brightness_display(unsigned short *framebuffer)
{
    #define S(x) ((BYTE_SWAP) ? __builtin_bswap16(x) : (x))
 
    uint8_t seg_count = 0;
	bool show_segment = false;
	uint8_t seg_number = 0;
	uint16_t pixel_colour = 0;


uint16_t sun_icon_13x20[] = {
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0x0000), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff),
  S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0x0000), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff), S(0xffff)
};


uint16_t *icon = sun_icon_13x20;
uint16_t *fb = framebuffer;


	for (uint8_t y = 25; y < 45; y++) {
		uint16_t *row = fb + (y * GW_SCREEN_WIDTH) + 2;
		for (uint8_t x = 2; x < 15; x++) {
			*row++ = *icon++;
		}
	}

	for (uint8_t y = 25; y < 45; y++) {
		seg_count = 0;
		seg_number = 0;
		show_segment = false;

		uint16_t *row = fb + (y * GW_SCREEN_WIDTH) + 15;

		for (uint8_t x = 15; x < 85; x++) {

			seg_count++;
			if ((seg_count == 5 && show_segment) || (seg_count == 2 && !show_segment)) {
				seg_count = 0;
				show_segment = !show_segment;
				if (show_segment) seg_number++;
			}

			pixel_colour = BYTE_SWAP ? 0x9ad6 : 0xd69a;

			if ((y > 25 && y < 45) && show_segment) {
				if (seg_count == 0 || seg_count == 4 || y == 26 || y == 43) {
					pixel_colour = BYTE_SWAP ? 0x2c63 : 0x632c;
				} else {
					if (brightness_level >= seg_number) {
						pixel_colour = brightness_get_segment_colour(seg_number);
					} else {
						pixel_colour = BYTE_SWAP ? 0x518c : 0x8c51;
					}
				}
			}

			*row++ = pixel_colour;
		}
	}

}