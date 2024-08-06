/**
 * @file pin_config.h
 * @brief Pin Configuration Header File
 * @details Raspberry Pi Pico Pin Configuration
 */

#ifndef __PIN_CONFIG__
#define __PIN_CONFIG__

/**
 * I2C Pin Configuration
 */
#define SSD1306_SDA_PIN 20
#define SSD1306_SCL_PIN 21

#define AP33772_SDA_PIN 26
#define AP33772_SCL_PIN 27

/**
 * Button Pin Configuration
 */
#define LEFT_BUTTON_PIN 18
#define RIGHT_BUTTON_PIN 19
#define ENTER_BUTTON_PIN 22

/**
 * LED Pin Configuration
 */
#define PWR_LED_PIN 23


#endif