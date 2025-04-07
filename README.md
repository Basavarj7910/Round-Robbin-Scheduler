# Building a Preemptive Round-Robin Scheduler from Scratch on STM32F407VG (Cortex-M4)

This project demonstrates how to build a preemptive round-robin task scheduler for the STM32F407VG microcontroller, based on the ARM Cortex-M4 architecture. The entire system is created **from scratch**, including the startup code, linker script, CMake build system, and context switching mechanism using **SysTick** and **PendSV**. It is designed to run **four tasks**, each blinking a different LED.

## Table of Contents

- [Project Structure](#project-structure)
- [Boot Process Explained](#boot-process-explained)
- [Memory Layout: Linker Script](#memory-layout-linker-script)
- [Startup Code & Stack Initialization](#startup-code--stack-initialization)
- [Task Scheduler Architecture](#task-scheduler-architecture)
- [Context Switching: SysTick & PendSV](#context-switching-systick--pendsv)
- [Task Lifecycle: Ready, Running, Waiting](#task-lifecycle-ready-running-waiting)
- [CMake Integration](#cmake-integration)
- [GPIO and Task Behavior](#gpio-and-task-behavior)
- [Visual Overview](#visual-overview)
- [Conclusion](#conclusion)

---

## Project Structure

```
project/
├── CMakeLists.txt
├── linker.ld
├── main.c
├── main.h
├── start_up.s
├── stm32f4discovery.cfg
├── toolchain.cmake
```

---

## Boot Process Explained

1. **Reset Handler** is fetched from the vector table, loaded into PC.
2. **Copy `.data`** section from Flash to RAM.
3. **Zero `.bss`** section.
4. Call `main()` function.

```asm
_start:
    LDR   r0, =_sidata  ; Load initialized data
    LDR   r1, =_sdata
    LDR   r2, =_edata
    ... (copy loop)

    LDR   r0, =_sbss    ; Zero BSS
    LDR   r1, =_ebss
    ... (zero loop)

    BL    main
```

---

## Memory Layout: Linker Script

```ld
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
  RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
  .text : { *(.isr_vector) *(.text*) } > FLASH
  .data : AT (ADDR(.text) + SIZEOF(.text)) { *(.data*) } > RAM
  .bss  : { *(.bss*) } > RAM
}
```

---

## Startup Code & Stack Initialization

- MSP is used initially.
- PSP (Process Stack Pointer) is configured for each task.
- Task stacks are prepared in advance.

```c
void init_task_stacks() {
    for (int i = 0; i < TASK_COUNT; i++) {
        task_stacks[i][STACK_SIZE - 1] = INITIAL_XPSR;
        ... // R0–R12, LR, PC setup
    }
}
```

---

## Task Scheduler Architecture

- Round-robin scheduler using `SysTick` interrupt.
- Each task gets CPU time slice.
- When task yields, PendSV is triggered.

```c
void SysTick_Handler() {
    trigger_context_switch();
}

void trigger_context_switch() {
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}
```

### State Management
Each task can be in:
- **Ready**: waiting to run
- **Running**: currently executing
- **Waiting**: explicitly paused (e.g., waiting for delay)

No task starves CPU due to wait-state logic.

---

## Context Switching: SysTick & PendSV

- `SysTick` triggers task switch.
- `PendSV_Handler` saves current context and loads the next.

```c
__attribute__((naked)) void PendSV_Handler() {
    SAVE_CONTEXT();
    current_task = next_ready_task();
    LOAD_CONTEXT();
    BX LR;
}
```

---

## Task Lifecycle: Ready, Running, Waiting

```txt
+------------+       SysTick         +-------------+
|   Ready    | ------------------>  |   Running   |
+------------+                      +-------------+
     ^                                      |
     |           Wait logic                \/
     | <-------------------------- +----------------+
+------------+                    |    Waiting     |
                                 +----------------+
```

This mechanism avoids CPU starvation and ensures fairness.

---

## CMake Integration

```cmake
cmake_minimum_required(VERSION 3.13)
project(SchedulerFromScratch C ASM)

set(CMAKE_C_STANDARD 11)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker.ld)
add_executable(scheduler
    main.c
    start_up.s
)
target_link_options(scheduler PRIVATE -T ${LINKER_SCRIPT})
```

Build with:
```sh
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -Bbuild .
cmake --build build
```

---

## GPIO and Task Behavior

```c
void task1() {
    while (1) {
        toggle_led(GPIO_PIN_12);
        delay(100);
        task_wait();
    }
}
```

Each task handles a unique LED. After delay, it voluntarily enters wait state.




