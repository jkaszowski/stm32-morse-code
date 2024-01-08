#pragma once
#include "stm32f4xx_hal.h"

#define LOG_INFO(msg) ;
#define LOG_DEBUG(msg) ;

void sleep_for_ms(int n);
uint32_t time_get_ms();

uint32_t adc_get();
