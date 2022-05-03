#ifndef __LEDS_H
#define __LEDS_H

#include "common.h"

typedef enum
{
    LED1 = 0,
    LED2 = 1,
    LED3 = 2,
    LED4 = 3
} led_enum_t;

#define LEDn                4

#define LED1_PIN            GPIO_PIN_10
#define LED1_GPIO_PORT      GPIOC
#define LED1_GPIO_CLK       RCU_GPIOC
  
#define LED2_PIN            GPIO_PIN_11
#define LED2_GPIO_PORT      GPIOC
#define LED2_GPIO_CLK       RCU_GPIOC
  
#define LED3_PIN            GPIO_PIN_12
#define LED3_GPIO_PORT      GPIOC
#define LED3_GPIO_CLK       RCU_GPIOC
  
#define LED4_PIN            GPIO_PIN_2
#define LED4_GPIO_PORT      GPIOD
#define LED4_GPIO_CLK       RCU_GPIOD

/*** LEDS DRIVER API ***/
void ledInit(led_enum_t lednum);
void turnLedOn(led_enum_t lednum);
void turnLedOff(led_enum_t lednum);
void toggleLed(led_enum_t lednum);

#endif //__LEDS_h
