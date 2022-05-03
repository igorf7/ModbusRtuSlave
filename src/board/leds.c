#include "leds.h"

static uint32_t GPIO_PORT[LEDn] =
                                {LED1_GPIO_PORT,
								LED2_GPIO_PORT,
								LED3_GPIO_PORT,
								LED4_GPIO_PORT};

static uint32_t GPIO_PIN[LEDn] =
								{LED1_PIN,
								LED2_PIN,
								LED3_PIN,
								LED4_PIN};

static rcu_periph_enum GPIO_CLK[LEDn] =
                                {LED1_GPIO_CLK,
								LED2_GPIO_CLK,
								LED3_GPIO_CLK,
								LED4_GPIO_CLK};

/*!
 \brief      configure led GPIO
 \param[in]  lednum: specify the Led to be configured
*/
void ledInit(led_enum_t lednum)
{
    /* enable the led clock */
    rcu_periph_clock_enable(GPIO_CLK[lednum]);

    /* configure led GPIO port */
    gpio_mode_set(GPIO_PORT[lednum], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN[lednum]);
    gpio_output_options_set(GPIO_PORT[lednum], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN[lednum]);

    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
 \brief      turn on selected led
 \param[in]  lednum: specify the Led to be turned on
*/
void turnLedOn(led_enum_t lednum)
{
    GPIO_BOP(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
 \brief      turn off selected led
 \param[in]  lednum: specify the Led to be turned off
*/
void turnLedOff(led_enum_t lednum)
{
    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
 \brief      toggle selected led
 \param[in]  lednum: specify the Led to be toggled
*/
void toggleLed(led_enum_t lednum)
{
    GPIO_OCTL(GPIO_PORT[lednum]) ^= GPIO_PIN[lednum];
}
