/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
  BLINK_STATE_BLINKING = 0,
  BLINK_STATE_WAITING  = 1
} BlinkState_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LED_PORT        GPIOC
#define LED_PIN         GPIO_PIN_13

#define BUTTON_PORT     GPIOA
#define BUTTON_PIN      GPIO_PIN_0

#define PA1_LOW_PORT    GPIOA
#define PA1_LOW_PIN     GPIO_PIN_1

#define BUTTON_PRESSED  GPIO_PIN_RESET

#define BLINK_MIN       4
#define BLINK_MAX       7
#define DEFAULT_BLINK   4

/*
 * STM32F103C8T6 için Flash başlangıcı: 0x08000000
 * 64 KB Flash son sayfa başlangıcı: 0x0800FC00
 * Son sayfayı blink_count saklamak için kullanıyoruz.
 */
#define FLASH_BLINK_COUNT_ADDR  ((uint32_t)0x0800FC00)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile uint16_t blink_count = DEFAULT_BLINK;

volatile uint16_t completed_blinks = 0;
volatile uint8_t led_is_on = 0;
volatile uint8_t wait_seconds = 0;
volatile BlinkState_t blink_state = BLINK_STATE_BLINKING;

GPIO_PinState button_last_raw = GPIO_PIN_SET;
GPIO_PinState button_stable_state = GPIO_PIN_SET;
uint32_t button_last_change_tick = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

void LED_On(void);
void LED_Off(void);

uint16_t Flash_ReadBlinkCount(void);
void Flash_WriteBlinkCount(uint16_t value);

uint8_t Button_IsPressed(void);
uint8_t Button_HeldForMs(uint32_t hold_ms);
void Button_InitDebounceState(void);
void Button_Process(void);

void BlinkCount_Increase(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_TIM2_Init();

  /* USER CODE BEGIN 2 */

  /*
   * PA1 pini ödevde istendiği gibi sürekli lojik 0 kalacak.
   */
  HAL_GPIO_WritePin(PA1_LOW_PORT, PA1_LOW_PIN, GPIO_PIN_RESET);

  /*
   * Bluepill kartında PC13 LED genelde aktif LOW çalışır.
   * GPIO_PIN_SET   -> LED sönük
   * GPIO_PIN_RESET -> LED yanık
   */
  LED_Off();

  /*
   * Yazılım başlarken buton basılıysa ve 3 saniye boyunca basılı tutulursa
   * fabrika ayarına dönülür: blink_count = 4
   */
  if (Button_IsPressed() && Button_HeldForMs(3000))
  {
    blink_count = DEFAULT_BLINK;
    Flash_WriteBlinkCount(blink_count);
  }
  else
  {
    /*
     * Normal açılışta blink_count Flash'tan okunur.
     */
    blink_count = Flash_ReadBlinkCount();

    /*
     * Flash boşsa genelde 0xFFFF okunur.
     * 4-7 arası dışında değer varsa geçersiz kabul edilir.
     */
    if (blink_count < BLINK_MIN || blink_count > BLINK_MAX)
    {
      blink_count = DEFAULT_BLINK;
      Flash_WriteBlinkCount(blink_count);
    }
  }

  /*
   * Açılışta buton basılı kaldıysa,
   * bu basış normal buton basışı olarak sayılmasın.
   */
  Button_InitDebounceState();

  completed_blinks = 0;
  wait_seconds = 0;
  blink_state = BLINK_STATE_BLINKING;
  LED_Off();

  /*
   * TIM2 interrupt başlatılır.
   * CubeMX ayarıyla TIM2 her 1 saniyede 1 interrupt üretir.
   */
  HAL_TIM_Base_Start_IT(&htim2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /*
     * Buton kontrolü ana döngüde yapılır.
     * Buton basılı tutulsa bile sadece 1 kez artırır.
     */
    Button_Process();

    /*
     * PA1 program boyunca LOW kalsın.
     */
    HAL_GPIO_WritePin(PA1_LOW_PORT, PA1_LOW_PIN, GPIO_PIN_RESET);

    /* USER CODE END 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void LED_On(void)
{
  /*
   * PC13 LED aktif LOW.
   */
  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
  led_is_on = 1;
}

void LED_Off(void)
{
  /*
   * PC13 LED aktif LOW.
   */
  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
  led_is_on = 0;
}

uint16_t Flash_ReadBlinkCount(void)
{
  return *(__IO uint16_t *)FLASH_BLINK_COUNT_ADDR;
}

void Flash_WriteBlinkCount(uint16_t value)
{
  FLASH_EraseInitTypeDef erase_init;
  uint32_t page_error = 0;

  HAL_FLASH_Unlock();

  erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init.PageAddress = FLASH_BLINK_COUNT_ADDR;
  erase_init.NbPages = 1;

  if (HAL_FLASHEx_Erase(&erase_init, &page_error) == HAL_OK)
  {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                      FLASH_BLINK_COUNT_ADDR,
                      value);
  }

  HAL_FLASH_Lock();
}

uint8_t Button_IsPressed(void)
{
  if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == BUTTON_PRESSED)
  {
    return 1;
  }

  return 0;
}

uint8_t Button_HeldForMs(uint32_t hold_ms)
{
  uint32_t start_tick = HAL_GetTick();

  while ((HAL_GetTick() - start_tick) < hold_ms)
  {
    if (!Button_IsPressed())
    {
      return 0;
    }
  }

  return 1;
}

void Button_InitDebounceState(void)
{
  GPIO_PinState current_state;

  current_state = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);

  button_last_raw = current_state;
  button_stable_state = current_state;
  button_last_change_tick = HAL_GetTick();
}

void Button_Process(void)
{
  GPIO_PinState current_raw;

  current_raw = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);

  /*
   * Ham buton durumu değiştiyse debounce süresi başlatılır.
   */
  if (current_raw != button_last_raw)
  {
    button_last_raw = current_raw;
    button_last_change_tick = HAL_GetTick();
  }

  /*
   * 50 ms debounce.
   */
  if ((HAL_GetTick() - button_last_change_tick) >= 50)
  {
    if (current_raw != button_stable_state)
    {
      button_stable_state = current_raw;

      /*
       * Sadece butona basıldığı anda 1 kez artır.
       * Buton basılı tutulursa tekrar tekrar artırmaz.
       */
      if (button_stable_state == BUTTON_PRESSED)
      {
        BlinkCount_Increase();
      }
    }
  }
}

void BlinkCount_Increase(void)
{
  if (blink_count < BLINK_MAX)
  {
    blink_count++;
  }
  else
  {
    blink_count = BLINK_MIN;
  }

  /*
   * blink_count her değiştiğinde Flash'a yazılır.
   */
  Flash_WriteBlinkCount(blink_count);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    if (blink_state == BLINK_STATE_BLINKING)
    {
      /*
       * Blink aşaması.
       * TIM2 her 1 saniyede 1 interrupt üretir.
       */
      if (led_is_on == 0)
      {
        LED_On();
      }
      else
      {
        LED_Off();
        completed_blinks++;

        /*
         * blink_count kadar yanıp söndükten sonra
         * 5 saniye sönük bekleme aşamasına geçilir.
         */
        if (completed_blinks >= blink_count)
        {
          completed_blinks = 0;
          wait_seconds = 0;
          blink_state = BLINK_STATE_WAITING;
          LED_Off();
        }
      }
    }
    else
    {
      /*
       * 5 saniye sönük bekleme aşaması.
       */
      LED_Off();
      wait_seconds++;

      if (wait_seconds >= 5)
      {
        wait_seconds = 0;
        blink_state = BLINK_STATE_BLINKING;
      }
    }
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */

  __disable_irq();

  while (1)
  {
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */

  /* USER CODE END 6 */
}

#endif /* USE_FULL_ASSERT */
