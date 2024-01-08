#include "stream.hpp"

bool Stream::append(uint8_t value) {
  if (length < max_length) {
    ptr[length++] = value;
    return true;
  } else {
    return false;
  }
}
