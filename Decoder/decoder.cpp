#include "decoder.h"
#include "morse_code.h"
#include "stream.hpp"

#ifdef HOST_BUILD
#include "platform_host.hpp"
void log(char *msg) { LOG_INFO(msg); }
#else
#include "platform_stm32.h"
#include "usbd_cdc_if.h"
void log(char *msg) { CDC_Transmit_FS((uint8_t *)msg, strlen(msg)); }
#endif

static char stack[128];
char *stack_ptr = stack;

int calibration_counter = 0;
int calibration_buffer = 0;
const int calibration_samples = 8;
int reference_value = 0;
int threshhold = 100;

const uint8_t HIGH_STATE = 1;
const uint8_t LOW_STATE = 0;

int DOT_DURATION = 200;
int DASH_DURATION = 600;
int PAUSE_DURATION = 200;
int MIDWORDPAUSE_DURATION = 600;
int BETWEENWORDPAUSE_DURATION = 2200;
int ERR = 50;
int ERR_PAUSE = 300;
int PAUSE_DURATION_LOW = (PAUSE_DURATION - ERR_PAUSE);
int PAUSE_DURATION_HIGH = (PAUSE_DURATION + ERR_PAUSE);
int DOT_DURATION_LOW = (DOT_DURATION - ERR_PAUSE);
int DOT_DURATION_HIGH = (DOT_DURATION + ERR_PAUSE);
int DASH_DURATION_LOW = (DASH_DURATION - ERR_PAUSE);
int DASH_DURATION_HIGH = (DASH_DURATION + ERR_PAUSE);
int MIDWORDPAUSE_DURATION_LOW = (MIDWORDPAUSE_DURATION - ERR_PAUSE);
int MIDWORDPAUSE_DURATION_HIGH = (MIDWORDPAUSE_DURATION + ERR_PAUSE);
int BETWEENWORDPAUSE_DURATION_LOW = (BETWEENWORDPAUSE_DURATION - ERR_PAUSE);
int BETWEENWORDPAUSE_DURATION_HIGH = (BETWEENWORDPAUSE_DURATION + ERR_PAUSE);
#define DASH '-'
#define DOT '.'
#define SPACE ' '

char human_readable[128];
char buffer[256];
char *ptr = buffer;

typedef enum {
  NORMAL,
  MEASURING_HIGH,
  MEASURING_LOW,
  FINISH
} TRANSMISSION_STATE;

// 1 - (Mikołaj's part) calibrate what is dot, what is dash, get whole stream
// 2 - gets binary stream and decodes to stream of dot and dash
// 3 - dekodowanie do human readable form

// Measure what are the noises from other light sources
uint32_t calibrateReferenceValue() {
  constexpr const int calibration_samples = 10;
  constexpr const int calibration_delay_ms = 10;
  uint32_t measured = 0;

  for (int i = 0; i < calibration_samples; i++) {
    measured += adc_get();
    sleep_for_ms(calibration_delay_ms);
  }
  return (measured / calibration_samples);
}

Stream getStream(uint32_t threshold_high) {
  constexpr const uint32_t timeout_ms = 1000;
  static uint8_t buffer[10000];
  Stream stream(buffer, 10000);
  bool inCalibrationMode = true;
  uint32_t last_high = time_get_ms();
  uint32_t value;
  while (inCalibrationMode) {
    value = adc_get();
    if (value > threshold_high) {
      last_high = time_get_ms();
      stream.append(HIGH_STATE);
    } else {
      stream.append(LOW_STATE);
    }
    if ((time_get_ms() - last_high) > timeout_ms) {
      inCalibrationMode = false;
      break;
    }
    sleep_for_ms(10);
  }
  return stream;
}

void calibrateStream(Stream str) {
  bool isPreviousHigh = false;
  uint32_t total_length = 0, n = 0;
  uint32_t sample_length = 0;
  while (str.is_ok()) {
    uint8_t sample = str.get_next();
    if (sample == HIGH_STATE) {
      sample_length++;
    }
    if (isPreviousHigh && sample == LOW_STATE) {
    }
    sample == HIGH_STATE ? isPreviousHigh = true : isPreviousHigh = false;
  }
}
// decodes stream to
void decodeStreamToString(uint8_t *stream, uint16_t length, char *string) {
  TRANSMISSION_STATE state = NORMAL;
  int time = 0, last_timestamp = 0, duration = 0, idx = 0;
  uint8_t measured = 0;

  while (1) {
    measured = stream[idx++];
    switch (state) {

    case NORMAL:
      if (measured == HIGH_STATE) {
        last_timestamp = time;
      }
      break;

    case MEASURING_HIGH:
      if (measured == LOW_STATE) {
        duration = time - last_timestamp;
        if ((DOT_DURATION_LOW < duration) && (duration < DOT_DURATION_HIGH)) {
          *(stack_ptr++) = DOT;
        } else if ((DASH_DURATION_LOW < duration) &&
                   (duration < DASH_DURATION_HIGH)) {
          *(stack_ptr++) = DASH;
        }
        state = MEASURING_LOW;
        last_timestamp = time;
      }
      break;

    case MEASURING_LOW:
      duration = time - last_timestamp;
      if (duration > 2 * BETWEENWORDPAUSE_DURATION_HIGH) {
        state = FINISH;
      }
      if (measured == HIGH_STATE) {
        state = MEASURING_HIGH;
        last_timestamp = time;
        if ((PAUSE_DURATION_LOW < duration) &&
            (duration < PAUSE_DURATION_HIGH)) {
          log("Found pause between dot and dash");
        } else if ((BETWEENWORDPAUSE_DURATION_LOW < duration) &&
                   (duration < BETWEENWORDPAUSE_DURATION_HIGH)) {
          *(stack_ptr++) = SPACE;
          *(stack_ptr++) = 'X';
          *(stack_ptr++) = SPACE;
          log("Found pause between words");
        } else if ((MIDWORDPAUSE_DURATION_LOW < duration) &&
                   (duration < MIDWORDPAUSE_DURATION_HIGH)) {
          *(stack_ptr++) = SPACE;
          log("Found pause between letters");
        } else {
          // log_dec("ERROR, low state too long, going back to NORMAL",
          // duration);
          state = NORMAL;
        }
      }
      break;

    case FINISH:
      log("FINISH");
      *(stack_ptr++) = 0;
      //		  log(stack);
      convertBuffer(stack);
      HAL_Delay(20);
      //		  CDC_Transmit_FS((uint8_t*)human_readable,
      // strlen(human_readable));
      log(human_readable);
      state = NORMAL;
      break;
      // case CALIBRATION:
      //   if (calibration_counter < calibration_samples) {
      //     calibration_buffer += getADCvalue();
      //     calibration_counter++;
      //     log("CALIBRATION");
      //   } else {
      //     reference_value = calibration_buffer / calibration_counter;
      //     state = NORMAL;
      //     // log_dec("CALIBRATION -> NORMAL", reference_value);
      //   }
      //   break;
    }
    time++;
  }
}

void run() {

  // testing::InitGoogleTest();
  // (void)RUN_ALL_TESTS();

  // char stack[] = ".- .-";
  // run(stack);
  // CDC_Transmit_FS("\r\n", 2);
  // printAscii(stack);
  uint32_t ref = calibrateReferenceValue();
  LOG_INFO("Measured " << ref << " to be a reference value");
  LOG_INFO("Got " << str.len() << " samples");
    Stream str = getStream(ref + 200);
}
