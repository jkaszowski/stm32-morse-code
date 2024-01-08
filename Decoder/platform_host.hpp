#pragma once
// #include <boost/log/trivial.hpp>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <thread>
// #define BOOST_LOG_DYN_LINK 1

#define LOG_DEBUG(msg) std::cout << msg << std::endl;
#define LOG_INFO(msg) std::cout << msg << std::endl;

uint64_t time_get_ms() {
  using namespace std::chrono;
  static auto beg = high_resolution_clock::now();
  return duration_cast<milliseconds>(high_resolution_clock::now() - beg)
      .count();
};

void HAL_Delay(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void sleep_for_ms(int ms) { HAL_Delay(ms); }
void CDC_Transmit_FS(char *ptr, int) { std::cout << *ptr; }

uint32_t adc_get() {
  static int cnt = 0;
  uint32_t arr[]{1, 2, 3, 4, 5, 6, 7, 9, 10};

  return cnt < (sizeof(arr) / sizeof(arr[0])) ? arr[cnt++] : 0;
}
