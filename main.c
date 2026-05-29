/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Snake Game using STM32, SSD1306 OLED and 3x4 Keypad
  ******************************************************************************
  * @attention
  * Snake game with:
  * - 5 speed levels (higher = faster)
  * - No top/left/right wall collision (wrap-around behavior)
  * - Thin bottom border as a ground
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "ssd1306.h"
#include "fonts.h"
#include "stdlib.h"

I2C_HandleTypeDef hi2c1;

#define BLOCK_SIZE 2
#define MAX_SNAKE_LENGTH 100

int snake_x[MAX_SNAKE_LENGTH];
int snake_y[MAX_SNAKE_LENGTH];
int snake_length = 5;
int direction = 3; // 0=up, 1=down, 2=left, 3=right
int food_x, food_y;
int level = 1;

typedef enum {
  MENU,
  PLAYING,
  GAME_OVER
} GameState;

GameState gameState = MENU;
uint8_t key = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

void DrawPixelSnake(void);
void GenerateFood(void);
void MoveSnake(void);
void CheckCollision(void);
void Keypad_Scan(void);
void DrawGroundLine(void);
void ShowMainMenu(void);
void ShowGameOver(void);

int main(void)
{
             HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();

  SSD1306_Init();
  ShowMainMenu();

  while (1)
  {
    Keypad_Scan();

    if (gameState == MENU) {
      ShowMainMenu();
    } else if (gameState == PLAYING) {
      switch(key) {
        case 2: if(direction != 1) direction = 0; break;
        case 8: if(direction != 0) direction = 1; break;
        case 4: if(direction != 3) direction = 2; break;
        case 6: if(direction != 2) direction = 3; break;
        case 5: if(level < 5) level++; break;
      }

      MoveSnake();
      CheckCollision();

      SSD1306_Clear();
      DrawPixelSnake();

      for (int dx = 0; dx < BLOCK_SIZE; dx++) {
        for (int dy = 0; dy < BLOCK_SIZE; dy++) {
          SSD1306_DrawPixel(food_x + dx, food_y + dy, 1);
        }
      }

      DrawGroundLine();
      SSD1306_UpdateScreen();
      HAL_Delay(300 - (level * 50));
    } else if (gameState == GAME_OVER) {
      ShowGameOver();
    }
  }
}

void DrawPixelSnake(void) {
  for (int i = 0; i < snake_length; i++) {
    for (int dx = 0; dx < BLOCK_SIZE; dx++) {
      for (int dy = 0; dy < BLOCK_SIZE; dy++) {
        SSD1306_DrawPixel(snake_x[i] + dx, snake_y[i] + dy, 1);
      }
    }
  }
}

void GenerateFood(void) {
  food_x = (rand() % ((128 - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;
  food_y = (rand() % ((60 - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;
}

void MoveSnake(void) {
  for (int i = snake_length - 1; i > 0; i--) {
    snake_x[i] = snake_x[i - 1];
    snake_y[i] = snake_y[i - 1];
  }

  switch(direction) {
    case 0: snake_y[0] -= BLOCK_SIZE; break;
    case 1: snake_y[0] += BLOCK_SIZE; break;
    case 2: snake_x[0] -= BLOCK_SIZE; break;
    case 3: snake_x[0] += BLOCK_SIZE; break;
  }

  // Wrap around screen
  if (snake_x[0] >= 128) snake_x[0] = 0;
  if (snake_x[0] < 0) snake_x[0] = 126;
  if (snake_y[0] < 0) snake_y[0] = 62;
  if (snake_y[0] >= 62) snake_y[0] = 0;
}

void CheckCollision(void) {
  // Food collision
  if (abs(snake_x[0] - food_x) < BLOCK_SIZE && abs(snake_y[0] - food_y) < BLOCK_SIZE) {
    if (snake_length < MAX_SNAKE_LENGTH) snake_length++;
    GenerateFood();
  }

  // Self collision
  for (int i = 1; i < snake_length; i++) {
    if (snake_x[0] == snake_x[i] && snake_y[0] == snake_y[i]) {
      gameState = GAME_OVER;
      return;
    }
  }
}

void DrawGroundLine(void) {
  for (int x = 0; x < 128; x++) {
    SSD1306_DrawPixel(x, 63, 1);
  }
}

void Keypad_Scan(void) {
  key = 255;

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) key = 1;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) key = 2;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) key = 3;

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) key = 4;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) key = 5;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) key = 6;

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3, GPIO_PIN_SET);
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) key = 7;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) key = 8;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) key = 9;

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_SET);
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) key = 10;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) key = 0;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) key = 11;
}

void ShowMainMenu(void) {
  SSD1306_Clear();
  SSD1306_GotoXY(30, 10);
  SSD1306_Puts("SNAKE GAME", &Font_7x10, 1);
  SSD1306_GotoXY(20, 30);
  SSD1306_Puts("5: Start Game", &Font_7x10, 1);
  SSD1306_GotoXY(20, 45);
  SSD1306_Puts("1: Exit", &Font_7x10, 1);
  SSD1306_UpdateScreen();

  while (1) {
    Keypad_Scan();
    if (key == 5) {
      snake_length = 5;
      direction = 3;
      level = 1;
      for (int i = 0; i < snake_length; i++) {
        snake_x[i] = 20 - i * BLOCK_SIZE;
        snake_y[i] = 20;
      }
      GenerateFood();
      gameState = PLAYING;
      break;
    }
    if (key == 1) {
      SSD1306_Clear();
      SSD1306_GotoXY(35, 25);
      SSD1306_Puts("Goodbye!", &Font_7x10, 1);
      SSD1306_UpdateScreen();
      HAL_Delay(1000);
      while (1);
    }
  }
}

void ShowGameOver(void) {
  SSD1306_Clear();
  SSD1306_GotoXY(30, 10);
  SSD1306_Puts("GAME OVER", &Font_7x10, 1);
  SSD1306_GotoXY(20, 30);
  SSD1306_Puts("Press 5 to Restart", &Font_7x10, 1);
  SSD1306_GotoXY(20, 45);
   SSD1306_Puts("1: Exit", &Font_7x10, 1);
  SSD1306_UpdateScreen();

  while (1) {
    Keypad_Scan();
    if (key == 5) {
      gameState = MENU;
      break;
    }
    if (key == 1) {
         SSD1306_Clear();
         SSD1306_GotoXY(35, 25);
         SSD1306_Puts("Goodbye!", &Font_7x10, 1);
         SSD1306_UpdateScreen();
         HAL_Delay(1000);
         while (1);
       }
  }
}


// SystemClock_Config, MX_GPIO_Init and MX_I2C1_Init functions as previously defined

// Add your I2C, GPIO, and clock initialization functions here as usual


// Add your I2C, GPIO, and clock initialization functions here as usual

// Clock, I2C, and GPIO init functions go here (use CubeMX generated or existing ones) as per your setup


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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
