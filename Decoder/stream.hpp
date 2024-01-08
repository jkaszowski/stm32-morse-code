#pragma once

#include <cstdint>

class Stream {
public:
  Stream(uint8_t *place, uint16_t length) : ptr(place), max_length(length) {}
  bool append(uint8_t);
  uint8_t get_next();
  bool is_ok() { return length < max_length; }
  uint16_t len() { return length; }

private:
  uint8_t *ptr;
  uint16_t length = 0;
  uint16_t max_length;
};
