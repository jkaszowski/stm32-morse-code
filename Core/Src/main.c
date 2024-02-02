/* USER CODE BEGIN Header */
// ############################################################################
//   Project: Optical Transmission of Text
//   Module:  Receiver
//   Author:  Jakub Kaszowski
//   Date:    09.01.2024
//   MCU:     BlackPill (STM32)
// ############################################################################
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
// Data used by USB driver
extern int usb_rx_flag;
extern char usb_rx_buffer[];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Measure current ADC value in range 0-4096
int getADCvalue() {
  int PomiarADC;
  if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
    PomiarADC = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Start(&hadc1);
    return PomiarADC;
  }
  return 0;
}

// Send message to the user via USB
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

// Convert a strign of dashes and dots to letter
char getAscii(char *str) {
  for (int i = 0; i < 26; i++) {
    if (strcmp(str, morseCode[i]) == 0) {
      return i + 'a';
    }
  }
  return ' ';
}

char human_readable[128]; // place to store human readable text after the
                          // conversion
char *ptr;                // temporary value for conversion

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

// possible states of main FSM
typedef enum {
  NORMAL,
  MEASURING_HIGH,
  MEASURING_LOW,
  FINISH,
  CALIBRATION
} TRANSMISSION_STATE;

// Helper variables for calibration
int calibration_counter = 0;
int calibration_buffer = 0;
const int calibration_samples = 8;

// Stores calibrated value of reference signal
int reference_value = 0;

// Stores the threshold over refrence value to classify signal as logic HIGH
int threshhold = 100;

// Timing constraints
const int DOT_DURATION = 200;
const int DASH_DURATION = 600;
const int PAUSE_DURATION = 200;
const int MIDWORDPAUSE_DURATION = 600;
const int BETWEENWORDPAUSE_DURATION = 2200;
const int ERR = 50;
const int ERR_PAUSE = 300;
const int PAUSE_DURATION_LOW = (PAUSE_DURATION - ERR_PAUSE);
const int PAUSE_DURATION_HIGH = (PAUSE_DURATION + ERR_PAUSE);
const int DOT_DURATION_LOW = (DOT_DURATION - ERR_PAUSE);
const int DOT_DURATION_HIGH = (DOT_DURATION + ERR_PAUSE);
const int DASH_DURATION_LOW = (DASH_DURATION - ERR_PAUSE);
const int DASH_DURATION_HIGH = (DASH_DURATION + ERR_PAUSE);
const int MIDWORDPAUSE_DURATION_LOW = (MIDWORDPAUSE_DURATION - ERR_PAUSE);
const int MIDWORDPAUSE_DURATION_HIGH = (MIDWORDPAUSE_DURATION + ERR_PAUSE);
const int BETWEENWORDPAUSE_DURATION_LOW =
    (BETWEENWORDPAUSE_DURATION - ERR_PAUSE);
const int BETWEENWORDPAUSE_DURATION_HIGH =
    (BETWEENWORDPAUSE_DURATION + ERR_PAUSE);

#define DASH '-'
#define DOT '.'
#define SPACE ' '

// Space for stroing dashes and dots
static char stack[512];
char *stack_ptr = stack;

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start(&hadc1);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  TRANSMISSION_STATE state = CALIBRATION;
  int last_timestamp = 0, duration, measured;
  while (1) {
    switch (state) {
    case NORMAL:
      measured = getADCvalue();
      if (measured > (reference_value + threshhold)) {
        last_timestamp = HAL_GetTick();
        state = MEASURING_HIGH;
        stack_ptr = stack;
        log("Transition: NORMAL -> MEASURING_HIGH");
      }
      break;
    case MEASURING_HIGH:
      if (getADCvalue() < reference_value + threshhold) {
        duration = HAL_GetTick() - last_timestamp;
        if ((DOT_DURATION_LOW < duration) && (duration < DOT_DURATION_HIGH)) {
          *(stack_ptr++) = DOT;
        } else if ((DASH_DURATION_LOW < duration) &&
                   (duration < DASH_DURATION_HIGH)) {
          *(stack_ptr++) = DASH;
        }
        state = MEASURING_LOW;
        log_dec("Measured high state ", duration);
        last_timestamp = HAL_GetTick();
      }
      break;
    case MEASURING_LOW:
      duration = HAL_GetTick() - last_timestamp;

      if (duration > 2 * BETWEENWORDPAUSE_DURATION_HIGH) {
        log("Timeout");
        state = FINISH;
      }

      if (getADCvalue() > reference_value + threshhold) {
        state = MEASURING_HIGH;
        last_timestamp = HAL_GetTick();
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
          log_dec("ERROR, low state too long, going back to NORMAL", duration);
          state = NORMAL;
        }
      }
      break;
    case FINISH:
      *(stack_ptr++) = 0;
      convertBuffer(stack);
      HAL_Delay(20);
      log(human_readable);
      state = NORMAL;
      break;
    case CALIBRATION:

      if (calibration_counter < calibration_samples) {
        calibration_buffer += getADCvalue();
        calibration_counter++;
        log("CALIBRATION");
      } else {
        reference_value = calibration_buffer / calibration_counter;
        state = NORMAL;
        log_dec("Transition: CALIBRATION -> NORMAL", reference_value);
      }
      break;
    }

    HAL_Delay(20);
    /* USER CODE END WHILE */
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 20;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data
   * Alignment and number of conversion)
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
