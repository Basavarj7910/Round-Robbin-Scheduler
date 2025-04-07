

#include <stdint.h>
#include <stdio.h>
#include "main.h"

void user_task1(void);
void user_task2(void);
void user_task3(void);
void user_task4(void);
void idle_task(void);
void task_delay(uint32_t tick);
uint8_t current_task = 1;

#define enable_interrupt() do { __asm volatile("mov r0,#0"); __asm volatile("msr PRIMASK,r0");}while(0);
#define disable_interrupt() do { __asm volatile("mov r0,#1"); __asm volatile("msr PRIMASK,r0");}while(0);

typedef struct {
	uint32_t psp;
	uint32_t block_count;
	uint8_t run_state;
	void (*task_handler)(void);
}TCB_t;

TCB_t user_tasks[5];
uint32_t global_count = 0;

void init_systick(uint32_t period);
__attribute__((naked))void init_msp(uint32_t);
void psp_init(void);
__attribute__((naked))void switch_to_psp(void);
void enable_faults(void);
void led_init(void);
int main(void)
{
	enable_faults();
	init_msp(SCHED_START);
	psp_init();
	init_systick(SYS_TIME);
	led_init();
    switch_to_psp();
	//check_systick_exception();
    /* Loop forever */
    user_task1();
    while(1);

}
void enable_faults()
{
	uint32_t *pSHCSR = (uint32_t *)0xe000ed24;
    *pSHCSR |= 0x00070000;
}
__attribute__((naked))void init_msp(uint32_t msp_value){
	__asm volatile("msr msp,r0");
	__asm volatile("bx lr");
}

void psp_init()
{
    user_tasks[0].psp = T1_START;
    user_tasks[1].psp = T2_START;
    user_tasks[2].psp = T3_START;
    user_tasks[3].psp = T4_START;
    user_tasks[4].psp = T5_START;

    user_tasks[0].run_state = TASK_READY_STATE;
    user_tasks[1].run_state = TASK_READY_STATE;
    user_tasks[2].run_state = TASK_READY_STATE;
    user_tasks[3].run_state = TASK_READY_STATE;
    user_tasks[4].run_state = TASK_READY_STATE;

    user_tasks[0].task_handler = idle_task;
    user_tasks[1].task_handler = user_task1;
    user_tasks[2].task_handler = user_task2;
    user_tasks[3].task_handler = user_task3;
    user_tasks[4].task_handler = user_task4;
	uint32_t *psp;
	for(int i=0;i<MAX_TASK;i++)
	{
		psp = (uint32_t *)user_tasks[i].psp;
		psp--;
		*psp = 0x01000000;
		psp--;
		*psp = (uint32_t)user_tasks[i].task_handler;
		psp--;
		*psp = 0xfffffffd;
		for (int j=0;j<13;j++)
		{
			psp--;
			*psp = 0x00;
		}
		user_tasks[i].psp = (uint32_t)psp;
	}

}
void init_systick(uint32_t period)
{
	uint32_t *SYST_RVR = (uint32_t *)0xE000E014;
	//uint32_t *SYST_CVR = (uint32_t *)0xE000E018;
	uint32_t *SYST_CSR = (uint32_t *)0xE000E010;
	uint32_t value = (SYS_CLOCK/period)-1;
	*SYST_RVR &= ~(0x00ffffffff);
	//*SYST_CVR = 0x00;
	*SYST_RVR |= value;
	//*SYST_RVR |= 10;
	*SYST_CSR |= (1<<1);
	*SYST_CSR |= (1<<2);
	*SYST_CSR |= (1<<0);
	//check_systick_exception();

}

void TaskYeild()
{
	uint32_t *pendsv = (uint32_t *)0xE000ED04;
	*pendsv |= (1 << 28);
}

void task_delay(uint32_t tick)
{
	disable_interrupt();
	if (current_task) {
	user_tasks[current_task].block_count = global_count + tick;
	user_tasks[current_task].run_state = TASK_BLOCKED_STATE;
	TaskYeild();
	}
	enable_interrupt();
}
uint32_t get_psp()
{
	return user_tasks[current_task].psp;
}
void update_current_task()
{
	int state = TASK_BLOCKED_STATE;
	for (int i=0;i<MAX_TASK;i++)
	{
		current_task++;
	    current_task %= MAX_TASK;
		state = user_tasks[current_task].run_state;
		if ((state != TASK_BLOCKED_STATE) && (current_task != 0)) {
			break;
		}
	}
	if (state != TASK_READY_STATE)
	{
		current_task = 0;
	}
	}

void push_psp(uint32_t psp_val)
{
	user_tasks[current_task].psp = psp_val;
}
__attribute__((naked))void switch_to_psp(void)
{
	__asm volatile("push {lr}");
	__asm volatile("bl get_psp");
	__asm volatile("msr psp,r0");
	__asm volatile("pop {lr}");
	__asm volatile("mov r0,#0x02");
	__asm volatile("msr control,r0");
	__asm volatile("bx lr");
}

__attribute__((naked))void PendSV_Handler(void) {
	  __asm volatile("push {lr}");
		__asm volatile("mrs r0,psp");
		__asm volatile("stmdb r0!,{r4-r11}");
		__asm volatile("bl push_psp");
		__asm volatile("bl update_current_task");
		__asm volatile("bl get_psp");
		__asm volatile("ldm r0!,{r4-r11}");
		__asm volatile("msr psp,r0");
		//__asm volatile("bl push_psp");
	    __asm volatile("pop {lr}");
	    __asm volatile("bx lr");


}

void update_global_count()
{
	global_count++;
}

void unblock_task()
{
	for (int i=1;i<MAX_TASK;i++){
		if (user_tasks[i].run_state == TASK_BLOCKED_STATE) {
			if (user_tasks[i].block_count == global_count){
				user_tasks[i].run_state = TASK_READY_STATE;
			}
		}
	}
}

void SysTick_Handler(void) {
	uint32_t *pendsv = (uint32_t *)0xE000ED04;
      update_global_count();
      unblock_task();
      *pendsv |= (1 << 28);
	}

void led_init()
{
    RCC_AHB1ENR |= (1 << 3);  // Enable GPIOD peripheral clock

    // 2. Configure GPIOD pin 12 as output
    GPIOD_MODER &= ~(3 << 24);  // Clear mode bits for pin 12 (24th and 25th bit)
    GPIOD_MODER |= (1 << 24);   // Set pin 12 to output (01)
    // 2. Configure GPIOD pin 13 as output
    GPIOD_MODER &= ~(3 << 26);  // Clear mode bits for pin 13 (26th and 27th bit)
    GPIOD_MODER |= (1 << 26);   // Set pin 13 to output (01)
    // 2. Configure GPIOD pin 14 as output
    GPIOD_MODER &= ~(3 << 28);  // Clear mode bits for pin 14 (28th and 29th bit)
    GPIOD_MODER |= (1 << 28);   // Set pin 14 to output (01)
    // 2. Configure GPIOD pin 15 as output
    GPIOD_MODER &= ~(3 << 30);  // Clear mode bits for pin 15 (30th and 31th bit)
    GPIOD_MODER |= (1 << 30);   // Set pin 15 to output (01)
				//
    // 3. Set output type to push-pull for pin 12
    GPIOD_OTYPER &= ~(1 << 12); // Set pin 12 as push-pull (0)
    // 3. Set output type to push-pull for pin 13
    GPIOD_OTYPER &= ~(1 << 13); // Set pin 13 as push-pull (0)
    // 3. Set output type to push-pull for pin 14
    GPIOD_OTYPER &= ~(1 << 14); // Set pin 14 as push-pull (0)
    // 3. Set output type to push-pull for pin 15
    GPIOD_OTYPER &= ~(1 << 15); // Set pin 15 as push-pull (0)
				//
    // 4. Set output speed for pin 12 to low speed (optional)
    GPIOD_OSPEEDR &= ~(3 << 24); // Clear speed bits for pin 12 (low speed)
    // 4. Set output speed for pin 13 to low speed (optional)
    GPIOD_OSPEEDR &= ~(3 << 26); // Clear speed bits for pin 13 (low speed)
    // 4. Set output speed for pin 14 to low speed (optional)
    GPIOD_OSPEEDR &= ~(3 << 28); // Clear speed bits for pin 14 (low speed)
    // 4. Set output speed for pin 15 to low speed (optional)
    GPIOD_OSPEEDR &= ~(3 << 30); // Clear speed bits for pin 15 (low speed)
				 //
    // 5. Disable pull-up/pull-down resistors for pin 12
    GPIOD_PUPDR &= ~(3 << 24);  // No pull-up, no pull-down
    // 5. Disable pull-up/pull-down resistors for pin 13
    GPIOD_PUPDR &= ~(3 << 26);  // No pull-up, no pull-down
    // 5. Disable pull-up/pull-down resistors for pin 14
    GPIOD_PUPDR &= ~(3 << 28);  // No pull-up, no pull-down
    // 5. Disable pull-up/pull-down resistors for pin 15
    GPIOD_PUPDR &= ~(3 << 30);  // No pull-up, no pull-down

}


void user_task1(void)
{
	while(1)
	{
		 GPIOD_ODR |= (1 << 12);   // Set pin 12 high (LED ON)
		 task_delay(250);
	     GPIOD_ODR &= ~(1 << 12);  // Set pin 12 low (LED OFF)
	     task_delay(250);               // Delay
	}
}
void user_task2(void)
{
	while(1)
	{
		GPIOD_ODR |= (1 << 13);   // Set pin 12 high (LED ON)
		task_delay(250);
		GPIOD_ODR &= ~(1 << 13);  // Set pin 12 low (LED OFF)
		task_delay(250);
	}
}

void user_task3(void)
{
	while(1)
	{
        // Delay
	        GPIOD_ODR |= (1 << 14);   // Set pin 12 high (LED ON)
			task_delay(250);
	        GPIOD_ODR &= ~(1 << 14);  // Set pin 12 low (LED OFF)
			task_delay(250);

	}
}

void idle_task()
{
	while(1);
}

void user_task4(void)
{
	while(1)
	{
		 GPIOD_ODR |= (1 << 15);   // Set pin 12 high (LED ON)
		 task_delay(250);
		 GPIOD_ODR &= ~(1 << 15);  // Set pin 12 low (LED OFF)
	     task_delay(250);
	}
}
void HardFault_Handler()
{
	while(1){}
}
void MemManage_Handler()
{
	while(1){}
}
void BusFault_Handler()
{
	while(1){}
}
void UsageFault_Handler()
{
	while(1){}
}



