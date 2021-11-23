/**
 * Practical work: Developing RTOS programs
 * Exercise 3 : Task and Mailbox
 * File : main.c
 * Description :
 *  - Main function which initializes binary semaphore + mailbox + interrupt + LED and start scheduler
 *  - Task 1 : pend a semaphore and blink the red LED + post mailbox (state red LED)
 *  - Task 2 : pend mailbox (state red led) and set the state to the green LED
 *  - ISR    : post a binary semaphore
 * @Author : Tiziano NARDONE - Undergraduate student in Computer Science - University of Reims Champagne Ardenne France
 * */

/* Includes */
#include <xdc/runtime/System.h>             /* For System abort */
#include <ti/drivers/GPIO.h>                /* For GPIO header files */
#include <ti/drivers/Board.h>               /* For Example/Board Header files */
#include <ti/sysbios/knl/Task.h>            /* For Module task manager*/
#include <ti/sysbios/knl/Clock.h>           /* For Clock tick period */
#include <ti/sysbios/BIOS.h>                /* For RTOS header files */
#include <ti/sysbios/knl/semaphore.h>       /* For semaphore header files*/
#include <ti/sysbios/knl/Mailbox.h>         /* For mailbox header files*/
#include "board.h"                          /* For the board configuration */

/* Defines */
#define STACK_SIZE 1024                     /* Size of the stack */
#define LED_RED Board_GPIO_LED0             /* Led 0 */
#define LED_GREEN Board_GPIO_LED1           /* Led 1 */
#define LED_ON Board_GPIO_LED_ON            /* Switch Led on */
#define LED_OFF Board_GPIO_LED_OFF          /* Switch Led off */
#define BUTTON_0 Board_GPIO_BUTTON0         /* Button 0 */

/* Global */
uint_fast8_t state_LED_RED=0;
Semaphore_Handle S1_h;
Mailbox_Handle mailHandle;

/* Task 1 */
void task1_fxn(){
    uint_fast8_t state_LED_RED=0;
    while(1){
        Semaphore_pend(S1_h, BIOS_WAIT_FOREVER);
        GPIO_toggle(LED_RED);
        state_LED_RED = GPIO_read(LED_RED);
        Task_sleep(1000000 / Clock_tickPeriod);
        if((Mailbox_post(mailHandle, &state_LED_RED, BIOS_NO_WAIT)) == FALSE ){
            System_abort("Error posting mailbox 1\n");
        }
    }
}

/* Task 2 */
void task2_fxn(){
    uint_fast8_t state_LED_RED=0;
    while(1){
        if((Mailbox_pend(mailHandle, &state_LED_RED, BIOS_WAIT_FOREVER)) == FALSE ){
            System_abort("Error posting mailbox 1\n");
        }
        GPIO_write(LED_GREEN,state_LED_RED);
    }
}


/* ISR */
void InterruptServiceRoutine(uint_least8_t index){
    Semaphore_post(S1_h);
}

/* main */
int main(void){
    Task_Params taskParams;
    Semaphore_Params semParams;
    enum Semaphore_Mode semMode = Semaphore_Mode_BINARY;

    /* Initialization drivers */
    Board_init();
    GPIO_init();

    /* Initialization LEDs */
    GPIO_setConfig(LED_RED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(LED_GREEN, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Initialization Interrupt */
    GPIO_setConfig(BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setCallback(BUTTON_0, InterruptServiceRoutine);
    GPIO_enableInt(BUTTON_0);

    /* Initialization Semaphore */
    Semaphore_Params_init(&semParams);
    semParams.mode = semMode;
    if( (S1_h = Semaphore_create(0, &semParams, NULL)) == NULL ){
        System_abort("Error creating semaphore 1\n");
    }

    /* Initialization Mailbox */
    if( (mailHandle = Mailbox_create(sizeof(state_LED_RED), sizeof(state_LED_RED), NULL, NULL)) == NULL ){
        System_abort("Error creating Mailbox 1\n");
    }

    /* Initialization Task 1 */
    Task_Params_init(&taskParams);
    taskParams.stackSize = STACK_SIZE;
    taskParams.priority = 1;
    if( (Task_create((Task_FuncPtr) task1_fxn, &taskParams, NULL)) == NULL ){
        System_abort("Error creating task1\n");
    }

    /* Initialization Task 2 */
    Task_Params_init(&taskParams);
    taskParams.stackSize = STACK_SIZE;
    taskParams.priority = 1;
    if( (Task_create((Task_FuncPtr) task2_fxn, &taskParams, NULL)) == NULL ){
        System_abort("Error creating task2\n");
    }

    /* Start the TI-RTOS scheduler */
    BIOS_start();

    return (0);
}
