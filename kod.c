/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdarg.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
const char *morseCode[] = {
    ".-",   // A
    "-...", // B
    "-.-.", // C
    "-..",  // D
    ".",    // E
    "..-.", // F
    "--.",  // G
    "....", // H
    "..",   // I
    ".---", // J
    "-.-",  // K
    ".-..", // L
    "--",   // M
    "-.",   // N
    "---",  // O
    ".--.", // P
    "--.-", // Q
    ".-.",  // R
    "...",  // S
    "-",    // T
    "..-",  // U
    "...-", // V
    ".--",  // W
    "-..-", // X
    "-.--", // Y
    "--.."  // Z
};

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */
extern int usb_rx_flag;
extern char usb_rx_buffer[];

// Define the variables for Morse code durations
int DOT_DURATION;
int DASH_DURATION;
int PAUSE_DURATION;
int MIDWORDPAUSE_DURATION;
int BETWEENWORDPAUSE_DURATION;
int ERR;
int ERR_PAUSE;
int PAUSE_DURATION_LOW;
int PAUSE_DURATION_HIGH;
int DOT_DURATION_LOW;
int DOT_DURATION_HIGH;
int DASH_DURATION_LOW;
int DASH_DURATION_HIGH;
int MIDWORDPAUSE_DURATION_LOW;
int MIDWORDPAUSE_DURATION_HIGH;
int BETWEENWORDPAUSE_DURATION_LOW;
int BETWEENWORDPAUSE_DURATION_HIGH;
int THRESHOLD;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int getADCvalue() {
  int PomiarADC;
  if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
    PomiarADC = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Start(&hadc1);
    return PomiarADC;
  }
  return 0;
}

void log(const char *msg) {
  char buffer[256];
  snprintf(buffer, 256, "%s\r\n", msg);
  CDC_Transmit_FS(buffer, strlen(buffer));
}

void log_dec(const char *msg, int val) {
  char buffer[256];
  snprintf(buffer, 256, "%s, val=%d\r\n", msg, val);
  CDC_Transmit_FS(buffer, strlen(buffer));
}

char getAscii(char *str) {
  for (int i = 0; i < 26; i++) {
    if (strcmp(str, morseCode[i]) == 0) {
      return i + 'a';
    }
  }
  return ' ';
}
char human_readable[128];
char *ptr;

void convertWord(char *word) {
  char korektor[] = " ";
  char *schowek;
  schowek = strtok(word, korektor);
  *(ptr++) = getAscii(schowek);
  while ((schowek = strtok(NULL, korektor)) != NULL) {
    *(ptr++) = getAscii(schowek);
  }
  *(ptr++) = ' ';
}

void convertBuffer(char *stack) {
  char korektor[] = " ";
  char *schowek;
  ptr = human_readable;
  schowek = strtok(stack, korektor);
  *(ptr++) = getAscii(schowek);
  while ((schowek = strtok(NULL, korektor)) != NULL) {
    *(ptr++) = getAscii(schowek);
  }
  *(ptr++) = 0;
}

typedef enum {
  NORMAL,
  MEASURING_HIGH,
  MEASURING_LOW,
  FINISH,
  CALIBRATION
} TRANSMISSION_STATE;

int calibration_counter = 0;
int calibration_buffer = 0;
const int calibration_samples = 8;
int reference_value = 0;



void enterCalibrationMode() {
  unsigned long lastChangeTime = millis(); //czas ostatniej zmiany stanu
  int lastLevel = digitalRead(buttonPin);  // ostatni stan przycisku

  int highDurationTotal = 0;
  int lowDurationTotal = 0;
  int highCount = 0;
  int lowCount = 0;

  while (inCalibrationMode) {
    int currentLevel = digitalRead(buttonPin); // aktualny stan przycisku

   
    if (currentLevel != lastLevel) {
    
      int duration = millis() - lastChangeTime;

      if (lastLevel == HIGH) {
        highDurationTotal += duration;
        highCount++;
      } else {
        lowDurationTotal += duration;
        lowCount++;
      }

      // Zaktualizuj czas ostatniej zmiany i ostatni stan
      lastChangeTime = millis();
      lastLevel = currentLevel;
    }

    // Jeśli przycisk nie był naciskany przez 20 sekund, zakończ kalibrację
    if (currentLevel == LOW && millis() - lastChangeTime > 20000) {
      inCalibrationMode = false;
    }
  }

  // Oblicz średnie czasy trwania
  int averageHighDuration = highDurationTotal / highCount;
  int averageLowDuration = lowDurationTotal / lowCount;

  
  DOT_DURATION = averageHighDuration / 2; 
  DASH_DURATION = averageHighDuration;
  PAUSE_DURATION = averageLowDuration / 2; 
  MIDWORDPAUSE_DURATION = averageLowDuration;
  BETWEENWORDPAUSE_DURATION = averageLowDuration * 2; 


  PAUSE_DURATION_LOW = PAUSE_DURATION - ERR_PAUSE;
  PAUSE_DURATION_HIGH = PAUSE_DURATION + ERR_PAUSE;
  DOT_DURATION_LOW = DOT_DURATION - ERR_PAUSE;
  DOT_DURATION_HIGH = DOT_DURATION + ERR_PAUSE;
  DASH_DURATION_LOW = DASH_DURATION - ERR_PAUSE;
  DASH_DURATION_HIGH = DASH_DURATION + ERR_PAUSE;
  MIDWORDPAUSE_DURATION_LOW = MIDWORDPAUSE_DURATION - ERR_PAUSE;
  MIDWORDPAUSE_DURATION_HIGH = MIDWORDPAUSE_DURATION + ERR_PAUSE;
  BETWEENWORDPAUSE_DURATION_LOW = BETWEENWORDPAUSE_DURATION - ERR_PAUSE;
  BETWEENWORDPAUSE_DURATION_HIGH = BETWEENWORDPAUSE_DURATION + ERR_PAUSE;

 
  THRESHOLD = (averageHighDuration + averageLowDuration) / 2; 
}

/* USER CODE END 0 */
