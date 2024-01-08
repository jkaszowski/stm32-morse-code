#include "platform_stm32.h"
#include "main.h"

void sleep_for_ms(int n) {
	HAL_Delay(n);
}
uint32_t time_get_ms() {
	return HAL_GetTick();
}

uint32_t adc_get() {
	getAdc();
}
