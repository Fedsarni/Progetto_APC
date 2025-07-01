/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "number_display.h"
#include <stdio.h>
#include "qrcodegen.h"
#include <stdlib.h>
#include <string.h>



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRIG_PORT GPIOA
#define ECHO_PORT GPIOA

#define SCALE 3

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
uint32_t previousMillis = 0;
uint32_t currentMillis = 0;
char keyPressed = ' ';
uint32_t echo_start_time =0;
uint32_t echo_stop_time= 0;
uint32_t distance = 0;
uint32_t distanzaVicinoCounter = 0; // conteggio in decimi di secondo
uint32_t away_counter = 0;
volatile uint16_t qr_flag = 0;
char distance_string[4];
uint32_t pwm = 0;
bool dir = 0;
uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
char qr_string[6]; // 5 cifre + terminatore '\0'
uint8_t occupato_ck = 0; //flag per indicare che il posto è occupato
uint8_t free_ck = 1; //flag per indicare che il posto è libero
uint8_t up_flag = 1; //questo flag simula un finecorsa, indica che la sbarra è alta
uint8_t down_flag = 0; // questo invece indica quando la sbarre è abbassata = 1
uint8_t sbarra = 0; //flag che indica se c'è qualcuno alla sbarra
char buffer[6];
uint8_t curr_buffer = 0;
int curr;
char key;
uint8_t row;
uint8_t col;
GPIO_TypeDef* col_ports[4] = {C1_GPIO_Port, C2_GPIO_Port, C3_GPIO_Port, C4_GPIO_Port};
uint16_t col_pins[4] = {C1_Pin, C2_Pin, C3_Pin, C4_Pin};
uint16_t row_pins[4] = {GPIO_PIN_11, GPIO_PIN_13, GPIO_PIN_15, GPIO_PIN_14};
char keypad[4][4] = {{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
uint16_t seconds_elapsed = 0;
uint16_t elapsed_secs = 0;
uint32_t elapsed_mins = 0;
uint16_t overflow = 0;
typedef struct {
	uint32_t timestamp;
	char ID[6];
}veicolo;
bool pay = 0;
bool automobile = 0;
bool gen = 1;
bool show_occupato = 0;
uint32_t start_time_2 = 0;
uint32_t stop_time_2 = 0;
uint16_t distance2 = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void ultrasound_trigger_func();
void init_posto();
void sbarra_up();
void sbarra_down();
void draw_qr_on_display2(const char *text);
void TurnOnLed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void TurnOffLed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void SetServoAngle(uint8_t anglle);
void generate_random_string(void);
void scan_colonne();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//funzione per regolare l'angolo del servo



void SetServoAngle(uint8_t angle){
	uint16_t pulse = 900+(angle*2200)/180;
	__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,pulse);
}

void sbarra_up(){
		SetServoAngle(90);
		down_flag = 0;
		up_flag = 1;
}

void sbarra_down(){
		SetServoAngle(0);
		up_flag = 0;
		down_flag = 1;
}

void init_posto(){
	TurnOnLed(GPIOE,GPIO_PIN_11);
	TurnOffLed(GPIOE,GPIO_PIN_9);
	sbarra_down();
}
void ultrasound_trigger_func(){
	//Questa funzione invia l'impulso iniziale di 10us
	HAL_GPIO_WritePin(TRIG_PORT, TRIGGER_PIN_Pin, GPIO_PIN_SET);  //Alza trigger
	HAL_GPIO_WritePin(GPIOA,TRIGGER2_PIN_Pin,GPIO_PIN_SET);
}



//ISR chiamata quando si verifica un qualsiasi evento sull'interfaccia GPIO

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == ECHO_PIN_Pin) {
		if (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN_Pin) == GPIO_PIN_SET) {
			//Se ECHO è alto allora basta inziare a contare (o salvare il conteggio iniziale)
			echo_start_time = __HAL_TIM_GET_COUNTER (&htim2);
		} else { //Quando ECHO si abbassa si smette di contare, si calcola la distanza e la si invia
			echo_stop_time = __HAL_TIM_GET_COUNTER (&htim2);
			distance = (echo_stop_time-echo_start_time)* 0.034/2;//Formula data
			if (distance >= 45) distance = 40; //questo serve per correggere l'overflow
			show_occupato = 0;
		}
		if (distance < 20) {
			      distanzaVicinoCounter++;
			      if (distanzaVicinoCounter >= 10) { // 50 * 100ms = 5 secondi
			    	  away_counter = 0;
			    	  TurnOnLed(GPIOE, GPIO_PIN_9);   // LED rosso
			          TurnOffLed(GPIOE, GPIO_PIN_11);  // LED verde
			          free_ck = 0;
			          occupato_ck  = 1;
			          if(!pay){
			        	  show_occupato = 1;
			    	  	  automobile = 1;
			       	  	  gen = 1;
			          }
			      }

		 } else {
			 	  away_counter++;
			 	  distanzaVicinoCounter = 0; // reset del timer
			 	  if(away_counter >=10){
			 		  TurnOnLed(GPIOE, GPIO_PIN_11);   // LED verde
			 		  TurnOffLed(GPIOE, GPIO_PIN_9);   // LED rosso
			 		  occupato_ck = 0;
			 		  free_ck = 1;
			 	  }

			  }
		 }

	currentMillis = HAL_GetTick();
	if (currentMillis - previousMillis > 10) {
	    /*Configure GPIO pins : PB6 PB7 PB8 PB9 to GPIO_INPUT*/
	    GPIO_InitStructPrivate.Pin = R1_Pin|R2_Pin|R3_Pin|R4_Pin;
	    GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
	    GPIO_InitStructPrivate.Pull = GPIO_NOPULL;
	    GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
	    HAL_GPIO_Init(GPIOC, &GPIO_InitStructPrivate);


	    for(int i=0;i<3;i++) {
	    	for(int j=0;j<3;j++) HAL_GPIO_WritePin(col_ports[j], col_pins[j],0);
	    	HAL_GPIO_WritePin(col_ports[i], col_pins[i],1);
	    	if(GPIO_Pin == R1_Pin && HAL_GPIO_ReadPin(R1_GPIO_Port, R1_Pin))
	    		    {
	    		      keyPressed = keypad[0][i];
	    		    }
	    		    else if(GPIO_Pin == R2_Pin && HAL_GPIO_ReadPin(R2_GPIO_Port, R2_Pin))
	    		    {
	    		      keyPressed = keypad[1][i];

	    		    }
	    		    else if(GPIO_Pin == R3_Pin && HAL_GPIO_ReadPin(R3_GPIO_Port, R3_Pin))
	    		    {
	    		      keyPressed = keypad[2][i];

	    		    }
	    		    else if(GPIO_Pin == R4_Pin && HAL_GPIO_ReadPin(R4_GPIO_Port, R4_Pin))
	    		    {
	    		      keyPressed = keypad[3][i];
	    		    }
	    }
	    HAL_GPIO_WritePin(C1_GPIO_Port, C1_Pin, 1);
	    HAL_GPIO_WritePin(C2_GPIO_Port, C2_Pin, 1);
	    HAL_GPIO_WritePin(C3_GPIO_Port, C3_Pin, 1);
	    HAL_GPIO_WritePin(C4_GPIO_Port, C4_Pin, 1);
	    /*Configure GPIO pins : PB6 PB7 PB8 PB9 back to EXTI*/
	    GPIO_InitStructPrivate.Mode = GPIO_MODE_IT_RISING;
	    GPIO_InitStructPrivate.Pull = GPIO_PULLDOWN;
	    HAL_GPIO_Init(GPIOC, &GPIO_InitStructPrivate);
	    previousMillis = currentMillis;
	  }
//============= ISR SECONDO SENSORE ===================

	  /*if(GPIO_Pin == ECHO2_PIN_Pin){
		  if(HAL_GPIO_ReadPin(GPIOA,ECHO2_PIN_Pin)==GPIO_PIN_SET){
			start_time_2 = __HAL_TIM_GET_COUNTER (&htim2);
		  }else { //Quando ECHO si abbassa si smette di contare, si calcola la distanza e la si invia
			stop_time_2 = __HAL_TIM_GET_COUNTER (&htim2);
			distance2= (stop_time_2-start_time_2)* 0.034/2;//Formula data
			if (distance2 >= 45) distance = 40;
		  }
		  if(distance2<20){
		//	  sbarra=1;
		  }else{
			  sbarra = 0;
		  }
	  }*/

}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
        if (htim->Instance == TIM3) {
        	seconds_elapsed++;
        	if ((seconds_elapsed % 10 == 0)) {
        		// 30 secondi trascorsi!
        		// Esegui azione
        		HAL_TIM_Base_Stop_IT(&htim3);
        		sbarra_down();

        	}
        }
        if(htim->Instance == TIM2){
        	overflow ++;
        		HAL_GPIO_WritePin(TRIG_PORT, TRIGGER_PIN_Pin, GPIO_PIN_RESET); //Abbassa trigger
        	    HAL_GPIO_WritePin(GPIOA,TRIGGER2_PIN_Pin,GPIO_PIN_RESET);
        	if(overflow >= 100){
        		elapsed_secs ++;
        		overflow = 0;
        		if ((elapsed_secs % 60) == 0){
        			 elapsed_mins ++;
        		}
        	}
        }

}



// Funzione per accendere un LED specifico
void TurnOnLed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

// Funzione per spegnere un LED specifico
void TurnOffLed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
}

//generazione codice univoco e conversione qrcode
void generate_random_string(void) {
    for (int i = 0; i < 5; i++) {
        qr_string[i] = '0' + (rand() % 10); // cifra casuale da '0' a '9'
    }
    qr_string[5] = '\0'; // terminatore stringa
}

void draw_qr_on_display2(const char *text) {

    bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, qrcodegen_Ecc_LOW,
                                       qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
                                       qrcodegen_Mask_AUTO, true);
        if (ok) {
            int qrsize = qrcodegen_getSize(qrcode);
            int offsetX = (128 - qrsize * SCALE) / 2;
            int offsetY = (64 - qrsize * SCALE) / 2;

            ssd1306_Fill(Black); // Pulisce lo schermo

            for (int y = 0; y < qrsize; y++) {
                for (int x = 0; x < qrsize; x++) {
                    bool pixel = qrcodegen_getModule(qrcode, x, y);
                    for (int dy = 0; dy < SCALE; dy++) {
                        for (int dx = 0; dx < SCALE; dx++) {
                            int drawX = x * SCALE + dx + offsetX;
                            int drawY = y * SCALE + dy + offsetY;

                            if (pixel)
                                ssd1306_DrawPixel(drawX, drawY, White);
                            else
                                ssd1306_DrawPixel(drawX, drawY, Black);
                        }
                    }
                }
            }

            ssd1306_UpdateScreen();  // Aggiorna il display
        }
    }


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

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  //HAL_TIM_Base_Start(&htim2);
  HAL_TIM_Base_Start_IT(&htim2); // Avvia il timer
  HAL_GPIO_WritePin(TRIG_PORT, TRIGGER_PIN_Pin, GPIO_PIN_RESET);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
   // Genera stringa casuale
  init_posto();
  ssd1306_Init();
  //questa va spostata in un altra logica
  veicolo v; //ipoteticamente per rendere il codice applicabile ad un parcheggio con più posti si potrebbe pensare di implementare una linked list di veicoli ma sono pigro :3
  int elapsed = 0;
  char* display_buffer = NULL;
  bool ok;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	//ssd1306_WriteString(buffer, Font_16x26, White);
	  if(gen) generate_random_string();
	  if(GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)){
	  		  sbarra_up();
	  		   // genera la stringa random
	  			  //seconds_elapsed = 0;
	  		  if(sbarra == 0){
	  			__HAL_TIM_SET_COUNTER(&htim3, 0);
	  		    HAL_TIM_Base_Start_IT(&htim3); // Avvia il timer
	  			gen = 0;
	  		  }
	  	  }
	  if (show_occupato){
    	  ssd1306_DisplayString("Occupato",Font_11x18);
	  }
	  if (sbarra){
			  v.timestamp = elapsed_secs; //associa il timestamp al veicolo
			  strcpy(v.ID,qr_string); //associa la stringa al veicolo
			  draw_qr_on_display2(qr_string); //mostra il qr
	  }

	  if(keyPressed == '#'&&sbarra == 0) {
		  if (automobile){

			  pay = 1;
			  curr = 0;
			  keyPressed = 'V';
			  elapsed = elapsed_secs- v.timestamp;
			  char str[30];
			  snprintf(str, sizeof(str), "Durata sosta: %d", elapsed);
			  ssd1306_DisplayString(str,Font_6x8);
		  }else{
			  ssd1306_DisplayString("non ci sono auto",Font_6x8);
	      }
	   }

		  if(pay){
			  if (keyPressed != 'V'){
				  buffer[curr]=keyPressed;
					  keyPressed = 'V';
					  curr++;
					  buffer[5]='\0';
					  ssd1306_DisplayString(buffer,Font_6x8);
					 // HAL_Delay(100);
					  if (curr>=5) {
						  if (!strcmp("11111",buffer)){
							  ssd1306_DisplayString("Arrivederci ^w^",Font_6x8);
							  pay = 0;
							  curr = 0;
							  buffer[0]=' ';
							  buffer[1]=' ';
							  buffer[2]=' ';
							  buffer[3]=' ';
							  buffer[4]=' ';
							  buffer[5]=' ';
							  automobile = 0;
							  gen = 1;
							  show_occupato = 0;
							  sbarra = 0;
							  //HAL_Delay(100);
						  }else{
							  ssd1306_DisplayString("inserisci pin",Font_6x8);
							  keyPressed = 'V';
							  curr = 0;
							  buffer[0]=' ';
							  buffer[1]=' ';
							  buffer[2]=' ';
							  buffer[3]=' ';
							  buffer[4]=' ';
							  buffer[5]=' ';
							  //HAL_Delay(100);
						  }
					  }
				  }

		  }
			  /*while(curr <= 6){
				  buffer[curr] = keyPressed;

				  ssd1306_DisplayString(buffer[curr]);
				  curr ++;
			  }*/
	  ultrasound_trigger_func();
	  HAL_Delay(500);



	  //HAL_Delay(100); // ogni 100 ms
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
  hi2c1.Init.Timing = 0x00201D2B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7200-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 19999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, C2_Pin|C3_Pin|C1_Pin|C4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TRIGGER2_PIN_Pin|TRIGGER_PIN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LED_ROSSO_Pin|LED_VERDE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : R2_Pin R4_Pin R3_Pin R1_Pin */
  GPIO_InitStruct.Pin = R2_Pin|R4_Pin|R3_Pin|R1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : C2_Pin C3_Pin C1_Pin C4_Pin */
  GPIO_InitStruct.Pin = C2_Pin|C3_Pin|C1_Pin|C4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : TRIGGER2_PIN_Pin TRIGGER_PIN_Pin */
  GPIO_InitStruct.Pin = TRIGGER2_PIN_Pin|TRIGGER_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ECHO2_PIN_Pin ECHO_PIN_Pin */
  GPIO_InitStruct.Pin = ECHO2_PIN_Pin|ECHO_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_ROSSO_Pin LED_VERDE_Pin */
  GPIO_InitStruct.Pin = LED_ROSSO_Pin|LED_VERDE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
