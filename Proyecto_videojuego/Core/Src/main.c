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
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "Bitmaps.h"
#include "fatfs_sd.h"
#include "stdio.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
SPI_HandleTypeDef hspi1;
FATFS fs;
FATFS *pfs;
FIL  fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
char buffer[100];
UINT br;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
//Variables utilizadas para almacernar matrices de imagenes utilizadas
extern uint16_t NAVE1[];
extern uint16_t NAVE2[];
extern uint16_t GUSANO2[];
extern uint16_t BALA[];
extern uint16_t GUSANO16[];
extern uint16_t NAVE16_1[];
extern uint16_t NAVE16_2[];
extern uint16_t GANA1[];
extern uint16_t GANA2[];


char buffer[100];
volatile uint8_t ok2=0;

//VARIBALES PARA JUGADOR 1
uint8_t direccion = 0;   // 1,2,3,4
static char last_direccion = '0';
int dx1 = 0;
int dy1 = 0;
int x = 100;
int y = 100;
uint32_t frame_counter = 0;	// Cuenta los ciclos del while para mover la bala
int bx, by;          // Posición de la bala
int j1_vivo = 1; // 1 = vivo, 0 = muerto
int bala_activa = 0; // 0 = no hay bala, 1 = bala volando
int puntos_j1 = 0;


//VARIABLES PARA JUGADOR 2
uint8_t direccionJ2 = 0;   // 1,2,3,4
static char last_direccion2 = '0';
int dx2 = 0;
int dy2 = 0;
int x2 = 200;
int y2 = 150;
int j2_vivo = 1;
int bx2_b, by2_b;  //variables para control de balas
int bala_activa2 = 0;
uint32_t frame_counter2 = 0;
int puntos_j2 = 0;

volatile uint8_t comando_actual = 0;	//guarda el estado actual de los botones
volatile uint8_t fondonegro = 1;	//bandera para colocar fondo negro
volatile uint8_t comando_actual2 = 0;	//guarda el estado actual de los botones
uint8_t dato_uart;	//Guarda el estado recibido por uart
uint8_t dato_uart2;	//Guarda el estado recibido por uart
uint8_t ok_UART = 0;	//Bandera que indica que hay un dato en el uart
uint8_t controlmenu = 0;	//variables para verficar el estado del menu
uint8_t controlmenuNIVELES = 0;	//variables para verficar el estADO DEL MENU DE NIVELES
int lastmenu = -1;
int lastmenuNIVELES = -1;
uint8_t pantalla_actual;	//indica la pantalla acutal en la s

// Definimos la estructura del segmento
typedef struct {
    int x_g, y_g;    // Posición en pantalla
    int dir_g;     // Dirección horizontal: 1 (derecha), -1 (izquierda)
} Segmento;

//DEFINICIONES Y VARIABLES UTILZADAS PARA LA LOGICA DEL MOVIMIENTO DEL GUSANO
#define LARGO_GUSANO 6  // Número de segmentos iniciales
#define TAM_SEGMENTO 16
Segmento gusano[LARGO_GUSANO]; // El arreglo que contiene a todo el gusano
uint32_t last_move_time = 0;   // Para controlar la velocidad sin usar HAL_Delay
int contador_gusano = 0;
int limite_velocidad = 50000; // Ajusta este valor para cambiar la velocidad global
char msg[20];
int fin_del_juego = 0;
uint8_t miCaracter = 'A'; // O el valor numérico del carácter


#define CS_LOW()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define CS_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define BUFFER_PIXELS width
#define MENU    0
#define NIVEL1  1
#define NIVEL2   3
#define JUGADOR1  4
#define JUGADOR2  5
#define NAVE_W 16
#define NAVE_H 16

//VARIABLES PARA LA POSICIÓN DE HONGOS
#define HONGO_W 16
#define HONGO_H 16
#define NUM_HONGOS 10

int hongo_x[10] = {
    5*16, 5*16, 10*16, 15*16, 3*16,
    7*16, 12*16, 18*16, 2*16, 9*16
};

int hongo_y[10] = {
    5*16, 2*16, 4*16, 6*16, 8*16,
    10*16, 12*16, 14*16, 5*16, 9*16
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI2_Init(void);
static void MX_UART5_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
void drawImageSD (char *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void dibujar_hongo(int x, int y);
void dibujar_hongos();
int hay_colision(int x, int y);
int check_collision_ships(int ax, int ay, int bx, int by);
void matar_jugador(int num_jugador);
void manejar_bala(int *bx, int *by, int *bala_activa, uint32_t *frame_counter,
                  int nave_enemiga_x, int nave_enemiga_y, int *puntos);
void actualizar_marcador(int score1, int score2);
void transmit_uart(char *string);
void mount_SD();
void open_ReadFile(char *filename);
//void manejar_input(char comando);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void transmit_uart(char *string) {
	uint8_t len = strlen(string);
	HAL_UART_Transmit(&huart2, (uint8_t*) string, len, 200);
}

void mount_SD(){
	fres = f_mount(&fs, "/", 0);
	if (fres == FR_OK) {
		transmit_uart("Micro SD card is mounted succesfully!\r\n");
	} else if (fres != FR_OK) {
		transmit_uart("Micro SD card's mounted error!\r\n");
	}
}

void open_ReadFile(char *filename) {
	fres = f_open(&fil, filename, FA_READ);
	if (fres == FR_OK) {
		transmit_uart("File opened for reading.\r\n");
	} else if (fres != FR_OK) {
		transmit_uart("File was not opened for reading.\r\n");
	}
}

void close_File(char *filename) {
	fres = f_close(&fil);
	if (fres == FR_OK) {
		transmit_uart("The file is closed.\r\n");
	} else if (fres != FR_OK) {
		transmit_uart("The file was not closed.\r\n");
	}
}

void unmount_SD() {
	f_mount(NULL, "", 1);
	if (fres == FR_OK) {
		transmit_uart("Micro SD card is unmounted!\r\n");
	} else if (fres != FR_OK) {
		transmit_uart("Micro SD card was not unmounted!\r\n");
	}
}

void drawImageSD (char *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	uint16_t buffer[width];

	open_ReadFile(filename);

	for (uint16_t fila = 0; fila < height; fila++) {
		f_read(&fil, buffer, width * sizeof(uint16_t), &br);


		LCD_Bitmap(x, y + fila, width, 1, buffer);
	}

	close_File(filename);
}



void dibujar_hongos() {
    for (int i = 0; i < 10; i++) {
        dibujar_hongo(hongo_x[i], hongo_y[i]);
    }
}

void dibujar_hongo(int x, int y) {
	LCD_Bitmap(x, y, 16, 16, hongo);
}

int hay_colision(int x, int y) {
    for (int i = 0; i < NUM_HONGOS; i++) {

        if (x < hongo_x[i] + HONGO_W &&
            x + NAVE_W > hongo_x[i] &&
            y < hongo_y[i] + HONGO_H &&
            y + NAVE_H > hongo_y[i]) {

            return 1; // hay colisión
        }
    }
    return 0; // no hay colisión
}

int check_collision_ships(int ax, int ay, int bx, int by) {
    if (ax < bx + 16 && // 16 es el ancho de la nave
        ax + 16 > bx &&
        ay < by + 16 &&
        ay + 16 > by) {
        return 1;
    }
    return 0;
}


void init_gusano() {
    for (int i = 0; i < LARGO_GUSANO; i++) {

        gusano[i].x_g = (LARGO_GUSANO - i) * TAM_SEGMENTO;
        gusano[i].y_g = 0;
        gusano[i].dir_g = 1; // Todos empiezan moviéndose a la derecha

       //Se muestra en la pantalla
        LCD_Bitmap(gusano[i].x_g, gusano[i].y_g, 32, 32, GUSANO16);
    }
}

void actualizar_gusano() {
    // 1. Guardar posiciones anteriores (para el efecto seguidor)
    int old_x[LARGO_GUSANO];
    int old_y[LARGO_GUSANO];

    for (int i = 0; i < LARGO_GUSANO; i++) {
        old_x[i] = gusano[i].x_g;
        old_y[i] = gusano[i].y_g;
    }

    // 2. Lógica de la cabeza
    int prox_x = gusano[0].x_g + (gusano[0].dir_g * TAM_SEGMENTO);
    int prox_y = gusano[0].y_g;

    // Verificar colisión o bordes laterales
    if (prox_x < 0 || prox_x > (320 - TAM_SEGMENTO) || hay_colision(prox_x, prox_y)) {

        // Verificar si puede bajar sin desbordar la pantalla (240 - 16 = 224)
        if (gusano[0].y_g + TAM_SEGMENTO <= 224) {
            gusano[0].y_g += TAM_SEGMENTO; // Baja una fila
        } else {
            gusano[0].y_g = 0; // Reset al inicio si llega al suelo
        }
        gusano[0].dir_g *= -1; // Cambia de dirección
    } else {
        gusano[0].x_g = prox_x; // Avanza normal
    }

    // 3. Mover el cuerpo
    for (int i = 1; i < LARGO_GUSANO; i++) {
        gusano[i].x_g = old_x[i - 1];
        gusano[i].y_g = old_y[i - 1];
    }

    // 4. Dibujo en la LCD
    for (int i = 0; i < LARGO_GUSANO; i++) {
        // Borrar rastro
        FillRect(old_x[i], old_y[i], TAM_SEGMENTO + 1, TAM_SEGMENTO + 1, 0x0000);
        LCD_Bitmap(gusano[i].x_g, gusano[i].y_g, TAM_SEGMENTO, TAM_SEGMENTO, GUSANO16);
    }
}

void controlar_tiempo_gusano() {
    contador_gusano++;

    if (contador_gusano >= limite_velocidad) {
        // 1. Predecir siguiente posición de la cabeza
        int prox_x = gusano[0].x_g + (gusano[0].dir_g * 16);
        int prox_y = gusano[0].y_g;

        // 2. REVISAR SI EL GUSANO TOCA A LOS JUGADORES (Cualquier segmento)
        for (int i = 0; i < LARGO_GUSANO; i++) {
            if (j1_vivo && check_collision_ships(gusano[i].x_g, gusano[i].y_g, x, y)) {
                matar_jugador(1);
            }
            if (j2_vivo && check_collision_ships(gusano[i].x_g, gusano[i].y_g, x2, y2)) {
                matar_jugador(2);
            }
        }

        // 3. LÓGICA DE MOVIMIENTO Y REBOTE
        // El gusano rebota en bordes y hongos.
        if (prox_x < 0 || prox_x > (320 - 16) || hay_colision(prox_x, prox_y)) {
            // Si choca con borde o hongo: baja y gira
            actualizar_gusano();
            contador_gusano = 0;
        }
        else {
            // Movimiento normal hacia adelante
            actualizar_gusano();
            contador_gusano = 0;
        }

        // 4. REPARACIÓN VISUAL Y CONTROL DE ESTADO
        // Redibujar a los jugadores si siguen vivos
        if (j1_vivo) {
            LCD_Bitmap(x, y, 16, 16, NAVE16_1);
        }

        if (j2_vivo) {
            LCD_Bitmap(x2, y2, 16, 16, NAVE16_2);
        }
    }
}

void matar_jugador(int num_jugador) {
    if (num_jugador == 1) {
        j1_vivo = 0;
        FillRect(x, y, 17, 17, 0x0000); // Borrar la nave

    } else {
        j2_vivo = 0;
        FillRect(x2, y2, 17, 17, 0x0000);
    }
}

void manejar_bala(int *bx, int *by, int *bala_activa, uint32_t *frame_counter,
                  int nave_enemiga_x, int nave_enemiga_y, int *puntos) {

    if (*bala_activa == 0) return;

    if (HAL_GetTick() - *frame_counter > 30) {
        *frame_counter = HAL_GetTick();

        // 1. Borrar posición anterior
        FillRect(*bx, *by, 17, 17, 0x0000);

        // 2. Mover hacia arriba
        *by -= 8;

        // 3. Salió de pantalla
        if (*by < 0) {
            *bala_activa = 0;
            return;
        }

        // 4. Dibujar bala
        LCD_Bitmap(*bx, *by, 16, 16, BALA);

        // 5. Colisión con hongos
        if (hay_colision(*bx, *by)) {
            *bala_activa = 0;
            FillRect(*bx, *by, 17, 17, 0x0000);
            return;
        }

        // 6. Colisión con nave enemiga
        if (check_collision_ships(*bx, *by, nave_enemiga_x, nave_enemiga_y)) {
            *bala_activa = 0;
            FillRect(*bx, *by, 17, 17, 0x0000);
            return;
        }

        // 7. Colisión con gusano -> suma puntos
        for (int i = 0; i < LARGO_GUSANO; i++) {
            if (check_collision_ships(*bx, *by, gusano[i].x_g, gusano[i].y_g)) {
                *bala_activa = 0;
                FillRect(*bx, *by, 17, 17, 0x0000);
                (*puntos)++;   //acumular los puntos
            }
        }
    }
}

void actualizar_marcador(int score1, int score2) {
    char bufferJ1[20]; // Espacio para el texto de J1
    char bufferJ2[20]; // Espacio para el texto de J2

    // Convertimos los enteros a formato de texto "J1: 000"
    sprintf(bufferJ1, "J1:%d", score1);
    sprintf(bufferJ2, "J2:%d", score2);

    LCD_Print(bufferJ1, 240, 5, 1, 0xFFFF, 0x0000);
    LCD_Print(bufferJ2, 240, 20, 1, 0xFFFF, 0x0000);
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
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  MX_UART5_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  //LCD_Clear(0xE941);
  mount_SD();
  //Dubujar la primer imagen desde la SD
  drawImageSD("fondomenu.bin.bin", 0, 0, 320, 240);


  unmount_SD();

  HAL_UART_Receive_IT(&huart3, (uint8_t*)buffer, 1);
  HAL_UART_Receive_IT(&huart5, (uint8_t*)buffer, 1);


  //Implemetación de la SD
  /*************************INICIO EJEMPLO SD ***************/
  /*Waiting for the Micro SD module to initialize*/
  HAL_Delay(500);
  //Montamos el sistema de archivos
  /*fres= f_mount(&fs, "/", 0);
  if (fres == FR_OK){
	  transmit_uart("Micro SD card is mountes successfully!\r\n");
  } else if (fres != FR_OK){
	  transmit_uart("Micro SD card's mount error!\r\n");
  }

 //Abrimos el archivo en modo escritura y lectura, y append
  fres = f_open(&fil, "fondo_inicio.txt", FA_OPEN_APPEND | FA_WRITE | FA_READ);
  if (fres == FR_OK){
	  transmit_uart("File opened dor reading.\r\n");
  } else if (fres != FR_OK){
	  transmit_uart ("File was not opened for reading.! \r\n");
  }
  //Escribir en el archivo abierto
  for(uint8_t i=0; i<1; i++){
	  f_puts("",&fil);
  }

 //Cerramos el archivo
  fres = f_close(&fil);
  if(fres == FR_OK){
	  transmit_uart("The file is closed. \r\n");
  } else if (fres != FR_OK){
	  transmit_uart("The file was not closed. \r\n");
  }

  //Abrimos el modo lectura
  fres = f_open(&fil, "prueba_gusano.txt", FA_READ);
  if (fres == FR_OK){
	  transmit_uart("File opened for reading. \r\n");
  } else if (fres != FR_OK){
	  transmit_uart("File was not opened for reading. \r\n");
  }
  //Leemos el archivo
  while(f_gets(buffer, sizeof(buffer), &fil)){
	  char mRd[100];
	  sprintf(mRd, "%s", buffer);
	  transmit_uart(mRd);
  }
  //Cerramos el archivo
  fres = f_close(&fil);
  if (fres == FR_OK){
	  transmit_uart("The file is closed \r\n");
  } else if (fres != FR_OK){
	  transmit_uart("The file was not closed. \r\n");
  }
  //Desmontamos el sistema de archivos
  f_mount(NULL, "", 1);
  if (fres == FR_OK){
	  transmit_uart("The Miro SD card is unmounted! \r\n");
  } else if (fres != FR_OK){
	  transmit_uart("The Miro SD card was not unmounted!  \r\n");
  }*/
  /*************************FIN DE EJEMPLO DE LA SD ***************/
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //Verificar si los jugadores siguen con vida
	  if (!j1_vivo && !j2_vivo && !fin_del_juego) {
	      fin_del_juego = 1;
	      HAL_UART_Transmit(&huart5, (uint8_t *)"B", 1, 10);

	      // Comparar puntajes
	      if (puntos_j1 > puntos_j2) {
	          // Aquí llamamos a la lógica para el Jugador 1
	          LCD_Clear(0x0000);
	         LCD_Bitmap(100, 85, 120, 70, GANA1);
	         sprintf(msg, "P1: %2d", puntos_j1);
	         LCD_Print(msg, 175, 141, 1, 0xFFFF, 0x0000);
	         sprintf(msg, "P2: %2d", puntos_j2);
	         LCD_Print(msg, 175, 160, 1, 0xFFFF, 0x0000);



	          /*
	          // 3. Mostrar puntajes finales
	          char texto_puntos[20];
	          sprintf(texto_puntos, "J1 Gana: %d", puntos_j1);
	          LCD_Print(texto_puntos, 100, 170, 2, 0xFFFF, 0x0000); */
	      }  else if (puntos_j2 > puntos_j1) {
	          // Aquí llamamos a la lógica para el Jugador 1
	          LCD_Clear(0x0000);
	          LCD_Bitmap(100, 85, 120, 70, GANA1);
	          sprintf(msg, "P2: %2d", puntos_j2);
	          LCD_Print(msg, 175, 141, 1, 0xFFFF, 0x0000);
	          sprintf(msg, "P1: %2d", puntos_j1);
	          LCD_Print(msg, 175, 160, 1, 0xFFFF, 0x0000);

	  }
	      else if (puntos_j2 == puntos_j1) {
	    	  LCD_Clear(0x0000);
	    	  LCD_Bitmap(100, 85, 120, 70, GANA1);
	    	  sprintf(msg, "P2: %2d", puntos_j2);
			  LCD_Print(msg, 175, 141, 1, 0xFFFF, 0x0000);
			  sprintf(msg, "P1: %2d", puntos_j1);
			  LCD_Print(msg, 175, 160, 1, 0xFFFF, 0x0000);

	      }


	  }
	  if (comando_actual2 == 'P')
			pantalla_actual = MENU;


	  // Bala jugador 1 (enemigo es jugador 2: x2, y2)
	  manejar_bala(&bx, &by, &bala_activa, &frame_counter, x2, y2, &puntos_j1);

	  // Bala jugador 2 (enemigo es jugador 1: x, y)
	  manejar_bala(&bx2_b, &by2_b, &bala_activa2, &frame_counter2, x, y, &puntos_j2);


	      // =====================
	      // 1. INPUT (UART)
	      // =====================
		  //CONTROL 1

		  if (ok2 == 1){

			  // 1. Bajar la bandera inmediatamente
			 ok2 = 0;
			 // Movimiento o selección
			  if (pantalla_actual == MENU){



				  if (comando_actual2 == 'D'){
					  HAL_UART_Transmit(&huart5, (uint8_t *)"C", 1, 10);
					  controlmenu++;
				  }
				  else if (comando_actual2 == 'U'){
					  HAL_UART_Transmit(&huart5, (uint8_t *)"C", 1, 10);
					  controlmenu--;
				  }
				  else if (comando_actual2 == 'S'){
					  if (controlmenu == 1)
						  pantalla_actual = NIVEL1;


					  else if (controlmenu == 2)
						  pantalla_actual = NIVEL2;

				  }


			  }
			  else if (pantalla_actual == NIVEL1){
				  fondonegro=0;


				  if (comando_actual2 == 'U'){
					  HAL_UART_Transmit(&huart5, (uint8_t *)"C", 1, 10);
					  controlmenuNIVELES++;}

				  else if (comando_actual2 == 'D'){
					  HAL_UART_Transmit(&huart5, (uint8_t *)"C", 1, 10);
					  controlmenuNIVELES--;}

				  else if (comando_actual2 == 'S'){
					  if (controlmenuNIVELES == 1)
						  pantalla_actual = JUGADOR1;


					  else if (controlmenuNIVELES == 2){
						  pantalla_actual = JUGADOR2;
						  fondonegro=0;}

				  }

			  else if (comando_actual2 == 'P'){
				  pantalla_actual = MENU;
			  }
			  }

			  else if (pantalla_actual == 	NIVEL2){
			  }



			  else if (pantalla_actual == JUGADOR2){
				      direccion = dato_uart2;
				      if (comando_actual2 == 'U' && bala_activa == 0) {
				          bala_activa = 1;
				          bx = x+8;
				          by = y ;	//Sale desde el borde superior de la nave
				          //sprintf(msg, "P1:%.d", puntos_j1);
				          //LCD_Print(msg, 10, 10, 1, 0xFFFF, 0x0000);
				          HAL_UART_Transmit(&huart5, (uint8_t *)"A", 1, 10);
				          frame_counter = HAL_GetTick();
				      }


			  }

			}

		  //CONTROL 2
	      if (ok_UART == 1){
	          ok_UART = 0;
	          if (pantalla_actual == JUGADOR2){
	        	  direccionJ2 = dato_uart;   // 1,2,3,4

	        	  if (comando_actual == 'U' && bala_activa2 == 0) {
					  bala_activa2 = 1;
					  bx2_b = x2+8;
					  by2_b = y2 ;	//Sale desde el borde superior de la nave
					  //sprintf(msg, "P2:%.d", puntos_j2);
					  //LCD_Print(msg, 10, 25, 1, 0xFFFF, 0x0000);
					  HAL_UART_Transmit(&huart5, (uint8_t *)"A", 1, 10);
					  frame_counter2 = HAL_GetTick();
				  }
	          }


	      }

	      // =====================
	      // 2. LÓGICA
	      // =====================

	      if (pantalla_actual == MENU){

	          // control circular
	          if (controlmenu < 1) controlmenu = 2;
	          if (controlmenu > 2) controlmenu = 1;

	      }

	      else if (pantalla_actual == NIVEL2){


		  }

	      else if (pantalla_actual == NIVEL1){

	    	  // control circular
			  if (controlmenuNIVELES < 1) controlmenuNIVELES = 2;
			  if (controlmenuNIVELES > 2) controlmenuNIVELES = 1;

	      }

	      else if (pantalla_actual == JUGADOR2){

		  }

	      // =====================
	      // 3. RENDER
	      // =====================

	      if (pantalla_actual == MENU){


	    	  //  Solo redibujar si cambia
			  if (controlmenu != lastmenu){

				  switch (controlmenu){

				  case 1:
					  LCD_Print("NIVEL 1", 100, 50, 2, 0x0000, 0xFB53);
					  LCD_Print("NIVEL 2", 100, 100, 2, 0x0000, 0xFFFF);
					  break;


				  case 2:
					  LCD_Print("NIVEL 1", 100, 50, 2, 0x0000, 0xFFFF);
					  LCD_Print("NIVEL 2", 100, 100, 2, 0x0000,  0xFB53);
					  break;

				  }
				  lastmenu = controlmenu;
			  }
	      }

	      else if (pantalla_actual == NIVEL2){

	    	  LCD_Clear(0xE941);   // 👈 aquí va todo tu juego
	      }

	      else if (pantalla_actual == NIVEL1){


	    	  //  Solo redibujar si cambia
			  if (controlmenuNIVELES != lastmenuNIVELES){
				  LCD_Clear(0xBD57);

				  switch (controlmenuNIVELES){

				  case 1:
					  LCD_Print("1 JUGADOR", 90, 50, 2, 0x0000, 0xFB53);
					  LCD_Print("2 JUGADORES", 90, 100, 2, 0x0000, 0xFFFF);
					  break;

				  case 2:
					  LCD_Print("1 JUGADOR", 90, 50, 2, 0x0000, 0xFFFF);
					  LCD_Print("2 JUGADORES", 90, 100, 2, 0x0000, 0xFB53);
					  break;
				  }
				  lastmenuNIVELES = controlmenuNIVELES;
		  }}


	      else if (pantalla_actual == JUGADOR2){

	    	  if(fondonegro==0){
	    		  LCD_Clear(0x0000);
	    		  init_gusano();
	    		  dibujar_hongos();
	    		  fondonegro=1;
	    	 }

	    	  //LOGICA PARA JUGADOR 1
	    	  if (direccion == '0')
				  last_direccion = '0';


	    	  if (direccionJ2 == '0')
				  last_direccion2 = '0';
	    	  if (j1_vivo) {

			  // 1. guardar posición anterior
	    	  if (direccion != '0' && direccion != last_direccion) {

				  int x_old = x;
				  int y_old = y;

				  switch(direccion){
					  case '1': dx1 = -1; dy1 = 0; break;
					  case '2': dx1 = 1;  dy1 = 0; break;
					  case '3': dx1 = 0;  dy1 = -1; break;
					  case '4': dx1 = 0;  dy1 = 1; break;
					  default: dx1 = 0; dy1 = 0; break;
				  }

				  // 3. mover (puedes usar grid o no)
				  int delta_x = dx1 * 16;
				  int delta_y = dy1 * 16;

				  // Validar Eje X
				  if (!hay_colision(x + delta_x, y) && !check_collision_ships(x + delta_x, y, x2, y2)) {
				      x += delta_x;
				  } else if (hay_colision(x + delta_x, y)) {
					  if (puntos_j1 > 0)
						  puntos_j1 -= 1; // Restamos la cantidad que desees
					  HAL_UART_Transmit(&huart5, (uint8_t *)"D", 1, 10);
					  //sprintf(msg, "P1:%2d", puntos_j1);
					  //LCD_Print(msg, 10, 10, 1, 0xFFFF, 0x0000);

				  }

				  // Validar Eje Y
				  if (!hay_colision(x, y + delta_y) && !check_collision_ships(x, y + delta_y, x2, y2)) {
				      y += delta_y;
				  } else if (hay_colision(x, y + delta_y)) {
					  if (puntos_j1 > 0)
						  puntos_j1 -= 1;
					  HAL_UART_Transmit(&huart5, (uint8_t *)"D", 1, 10);
					  //sprintf(msg, "P1:%.d", puntos_j1);
					  //LCD_Print(msg, 10, 10, 1, 0xFFFF, 0x0000);

				  }

				  // 4. límites (IMPORTANTE ajustar a 16)
				  if (x < 0) x = 0;
				  if (x > 320 - 16) x = 320 - 16; //se regresa la el límite menos el tamaño de sprite

				  if (y < 0) y = 0;
				  if (y > 240 - 16) y = 240 - 16;

				  // 5. borrar anterior
				  FillRect(x_old, y_old, 17, 17, 0x0000); //borrar el rastro de la nave dibujando el rectangulo 	del tañano de sprite en la dirección anterior

				  // 6. dibujar nueva
				  LCD_Bitmap(x, y, 16, 16, NAVE16_1); //dibujar el sprite con el movimiento actualizado

				  last_direccion = direccion;
				  }
	    	  }

	    	  //LOGICA JUGADOR 2
	    	  if (j2_vivo) {

	    	  // 1. guardar posición anterior
			  if (direccionJ2 != '0' && direccionJ2 != last_direccion2) {
				  int x_old2 = x2;
				  int y_old2 = y2;

				  // 2. joystick
				  switch(direccionJ2){
					  case '1': dx2 = -1; dy2 = 0; break;
					  case '2': dx2 = 1;  dy2 = 0; break;
					  case '3': dx2 = 0;  dy2 = -1; break;
					  case '4': dx2 = 0;  dy2 = 1; break;
					  default: dx2 = 0; dy2 = 0; break;
				  }

				  int delta_x2 = dx2 * 16;
				  int delta_y2 = dy2 * 16;
				  // Validar Eje X
				  if (!hay_colision(x2 + delta_x2, y2) && !check_collision_ships(x2 + delta_x2, y2, x, y)) {
				      x2 += delta_x2;
				  } else if (hay_colision(x2 + delta_x2, y2)) {
					  if (puntos_j2>0)
						  puntos_j2 -= 1;
					  HAL_UART_Transmit(&huart5, (uint8_t *)"D", 1, 10);
				      //sprintf(msg, "P2:%.d", puntos_j2);
				      //LCD_Print(msg, 10, 25, 1, 0xFFFF, 0x0000);

				  }

				  // Validar Eje Y
				  if (!hay_colision(x2, y2 + delta_y2) && !check_collision_ships(x2, y2 + delta_y2, x, y)) {
				      y2 += delta_y2;
				  } else if (hay_colision(x2, y2 + delta_y2)) {
					  if (puntos_j2>0)
						  puntos_j2 -= 1;
					  HAL_UART_Transmit(&huart5, (uint8_t *)"D", 1, 10);
				     // sprintf(msg, "P2:%.d", puntos_j2);
				     // LCD_Print(msg, 10, 25, 1, 0xFFFF, 0x0000);

				  }

				  // 4. límites (IMPORTANTE ajustar a 16)
				  if (x2 < 0) x2 = 0;
				  if (x2 > 320 - 16) x2 = 320 - 16; //se regresa la el límite menos el tamaño de sprite

				  if (y2 < 0) y2 = 0;
				  if (y2 > 240 - 16) y2 = 240 - 16;

				  // 5. borrar anterior
				  FillRect(x_old2, y_old2, 17, 17,  0x0000); //borrar el rastro de la nave dibujando el rectangulo 	del tañano de sprite en la dirección anterior

				  // 6. dibujar nueva
				  LCD_Bitmap(x2, y2, 16, 16, NAVE16_2); //dibujar el sprite con el movimiento actualizado
				  //LCD_Bitmap(x2, y2, 32, 32, NAVE2); //dibujar el sprite con el movimiento actualizado
				  last_direccion2 = direccionJ2;
				  }


	    	  }
	    	  if(fin_del_juego==0)
	    	  controlar_tiempo_gusano();
	      	  }


	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin|LCD_D1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|LCD_D6_Pin|LedPrueba_Pin|LCD_D3_Pin
                          |LCD_D5_Pin|LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SPI2_NSS_Pin|SD_SS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : LCD_RST_Pin LCD_D1_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|LCD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RD_Pin LCD_WR_Pin LCD_RS_Pin LCD_D7_Pin
                           LCD_D0_Pin LCD_D2_Pin */
  GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_D6_Pin LedPrueba_Pin LCD_D3_Pin
                           LCD_D5_Pin LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_D6_Pin|LedPrueba_Pin|LCD_D3_Pin
                          |LCD_D5_Pin|LCD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI2_NSS_Pin SD_SS_Pin */
  GPIO_InitStruct.Pin = SPI2_NSS_Pin|SD_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if(huart->Instance == USART3){

    	ok_UART=1;
    	comando_actual = dato_uart;
		// Volver a activar la recepción para la próxima letra
		// Guardar directamente en &dato_uart
		HAL_UART_Receive_IT(&huart3, &dato_uart, 1);}

    else if(huart->Instance == UART5){
		ok2=1;
		comando_actual2 = dato_uart2;

		// Volver a activar la recepción para la próxima letra
		// Guardar directamente en &dato_uart
		HAL_UART_Receive_IT(&huart5, &dato_uart2, 1);}

}

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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
