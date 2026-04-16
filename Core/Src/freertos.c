/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "menu.h"
#include "dames.h"
#include "test_uart.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern TypeEcran ecranCourant;
extern DamesModePartie modePartieDamesCourant;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId Affichage_JeuHandle;
osThreadId Logique_jeuHandle;
osThreadId Tache_IPHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

extern void AfficherEcranDames(DamesModePartie modePartie, DamesJoueurLocal joueurLocal);
extern void AfficherEcranAccueil(void);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void Fonction_affichage_jeu(void const * argument);
void fct_logique_jeu(void const * argument);
void fct_com_IP(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of Affichage_Jeu */
  osThreadDef(Affichage_Jeu, Fonction_affichage_jeu, osPriorityNormal, 0, 1024);
  Affichage_JeuHandle = osThreadCreate(osThread(Affichage_Jeu), NULL);

  /* definition and creation of Logique_jeu */
  osThreadDef(Logique_jeu, fct_logique_jeu, osPriorityNormal, 0, 1024);
  Logique_jeuHandle = osThreadCreate(osThread(Logique_jeu), NULL);

  /* definition and creation of Tache_IP */
  osThreadDef(Tache_IP, fct_com_IP, osPriorityNormal, 0, 512);
  Tache_IPHandle = osThreadCreate(osThread(Tache_IP), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_Fonction_affichage_jeu */
/**
* @brief Function implementing the Affichage_Jeu thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Fonction_affichage_jeu */
void Fonction_affichage_jeu(void const * argument)
{
  /* USER CODE BEGIN Fonction_affichage_jeu */
  TS_StateTypeDef etatTactile = {0};
  uint8_t tactileActifPrecedent = 0U;

  for(;;)
  {
    /* Lecture de l'état de l'écran tactile */
    BSP_TS_GetState(&etatTactile);

    /* Traitement si un nouvel appui est détecté */
    if ((etatTactile.touchDetected != 0U) && (tactileActifPrecedent == 0U))
    {
      if (ecranCourant == ECRAN_ACCUEIL)
      {
        MenuAction actionMenu;
        actionMenu = Menu_GererTouch(etatTactile.touchX[0], etatTactile.touchY[0]);

        if (actionMenu == MENU_ACTION_LANCER_DAMES_LOCAL)
        {
          AfficherEcranDames(DAMES_MODE_LOCAL, DAMES_JOUEUR_LOCAL_BLANC);
        }
        else if (actionMenu == MENU_ACTION_LANCER_DAMES_UART_BLANC)
        {
          AfficherEcranDames(DAMES_MODE_UART, DAMES_JOUEUR_LOCAL_BLANC);
        }
        else if (actionMenu == MENU_ACTION_LANCER_DAMES_UART_NOIR)
        {
          AfficherEcranDames(DAMES_MODE_UART, DAMES_JOUEUR_LOCAL_NOIR);
        }
      }
      else if (ecranCourant == ECRAN_DAMES)
      {
        if (Dames_GererTouch(etatTactile.touchX[0], etatTactile.touchY[0]) == DAMES_ACTION_QUITTER)
        {
          AfficherEcranAccueil();
        }
      }
    }

    /* Sauvegarde de l'état tactile pour la détection de front montant */
    tactileActifPrecedent = (etatTactile.touchDetected != 0U) ? 1U : 0U;


    osDelay(50);
  }
  /* USER CODE END Fonction_affichage_jeu */
}

/* USER CODE BEGIN Header_fct_logique_jeu */
/**
* @brief Function implementing the Logique_jeu thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_fct_logique_jeu */
void fct_logique_jeu(void const * argument)
{
  /* USER CODE BEGIN fct_logique_jeu */
  CoupDames coupLocal;
  CoupDames coupRecu;
  char messageCoup[TAILLE_MESSAGE_COUP_MAX];
  char messageRecu[TAILLE_MESSAGE_COUP_MAX];

  for(;;)
  {
    /* La communication n'est active que sur l'écran du jeu en mode distant */
    if ((ecranCourant == ECRAN_DAMES) && (modePartieDamesCourant == DAMES_MODE_UART))
    {
      /* Traitement de l'envoi du coup local */
      if ((Dames_CoupLocalEstPret() != 0U) &&
          (Dames_RecupererDernierCoupLocal(&coupLocal) != 0U) &&
          (Dames_ConvertirCoupEnTexte(&coupLocal, messageCoup, sizeof(messageCoup)) != 0U))
      {
        if (TestUart_EnvoyerMessage(messageCoup) != 0U)
        {
          Dames_AcquitterDernierCoupLocal();
        }
      }

      /* Traitement de la réception du coup distant */
      if ((TestUart_MessageRecuEstPret() != 0U) &&
          (TestUart_RecupererDernierMessageRecu(messageRecu, sizeof(messageRecu)) != 0U))
      {
        if ((Dames_ConvertirTexteEnCoup(messageRecu, &coupRecu) != 0U) &&
            (Dames_AppliquerCoupRecu(&coupRecu) != 0U))
        {
          TestUart_AcquitterDernierMessageRecu();
        }
      }
    }


    osDelay(50);
  }
  /* USER CODE END fct_logique_jeu */
}

/* USER CODE BEGIN Header_fct_com_IP */
/**
* @brief Function implementing the Tache_IP thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_fct_com_IP */
void fct_com_IP(void const * argument)
{
  /* USER CODE BEGIN fct_com_IP */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END fct_com_IP */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

