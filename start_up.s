.extern _etext
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss


.section .bss
  array: .space 10

.section .data
  data_val: .word 1234,2345,678

.section .text
   .type Default_Handler,%function
   Default_Handler:
                    bl .

.weak PendSV_Handler
.thumb_set PendSV_Handler,Default_Handler

.weak SysTick_Handler
.thumb_set SysTick_Handler,Default_Handler


.section .vector
    vector_table:
          .word 0x20020000
          .word reset_handler
          .org 0x38
          .word PendSV_Handler
          .word SysTick_Handler
          .zero 400

.section .text
   .align 1
   .type reset_handler, %function
   reset_handler:
          ldr r1, =_etext
          ldr r2, =_sdata
          ldr r3, =_edata
       up:mov r0,#0
          ldrb r0,[r1] 
          strb r0,[r2]
          add r1,r1,#1
          add r2,r2,#1 
          cmp r2, r3
          bne up
          ldr r1,=_sbss
          ldr r2,=_ebss
          mov r3,#0
     next:strb r3,[r1]
          add r1,r1,#1
          cmp r1,r2
          bne next
          bl main
          b .

          



