
/*
 * main.h
 *
 *  Created on: Dec 10, 2024
 *      Author: basavaraj
 */

#ifndef MAIN_H_
#define MAIN_H_
#define USER_STACK_SIZE 1024U
#define SCHED_STACK_SIZE 1024U
#define SRAM_START 0x20000000U
#define SRAM_SIZE ((128)*(1024))
#define SRAM_END ((SRAM_START) +(SRAM_SIZE))
#define T1_START SRAM_END
#define T2_START ((T1_START)-(USER_STACK_SIZE))
#define T3_START ((T2_START)-(USER_STACK_SIZE))
#define T4_START ((T3_START)-(USER_STACK_SIZE))
#define T5_START  ((T4_START)-(USER_STACK_SIZE))
#define SCHED_START ((T5_START)-(USER_STACK_SIZE))
#define SYS_CLOCK   16000000U
#define SYS_TIME    1000U
#define MAX_TASK     5
#define RCC_AHB1ENR    (*((volatile unsigned int*)0x40023830))
#define GPIOD_MODER    (*((volatile unsigned int*)0x40020C00))
#define GPIOD_OTYPER   (*((volatile unsigned int*)0x40020C04))
#define GPIOD_OSPEEDR  (*((volatile unsigned int*)0x40020C08))
#define GPIOD_PUPDR    (*((volatile unsigned int*)0x40020C0C))
#define GPIOD_ODR      (*((volatile unsigned int*)0x40020C14))

typedef enum
{
	TASK_READY_STATE = 0x00,
	TASK_BLOCKED_STATE = 0xff
}task_states_t;





#endif /* MAIN_H_ */
