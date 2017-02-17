/*
test code for testing led dbg lib
@author: muteX023
*/

.include "ledDbgLib.h"

_start:
    mrs r0, cpsr
    bic r0, r0, #0x1F	@ clear mode bits
    orr r0, r0, #0x13	@ set supervisor mode
    orr r0, r0, #0xC0	@ disable FIQ and IRQ
    msr cpsr, r0  
    
    bl PROC_LED_INIT    
        
    mov r12, #0x0B
    mov r11, #10
    bl PROC_LED_BLINK 
	mov r7, #3
    bl PROC_DELAY    
    
    mov r12, #0x0D
    bl PROC_LED_BINARY
    mov r7, #3
    bl PROC_DELAY
    
    mov r12, #5
    bl PROC_LED_BINARY
    mov r7, #3
    bl PROC_DELAY
    
    mov r12, #9
    bl PROC_LED_BINARY
    mov r7, #3 
    bl PROC_DELAY
   
    mov r12, #2
    mov r11, #1
    bl PROC_LED_ONOFF
    mov r7, #3
    bl PROC_DELAY
    
    mov r12, #0
    mov r11, #0
    bl PROC_LED_ONOFF
    mov r7, #3
    bl PROC_DELAY
    
    mov r12, #3
    bl PROC_LED_TOGGLE
    mov r7, #3
    bl PROC_DELAY
    
    mov r12, #3
    bl PROC_LED_TOGGLE
    mov r7, #3
    bl PROC_DELAY
    
    mov r12, #1
    bl PROC_LED_TOGGLE
    mov r7, #3
    bl PROC_DELAY 
    
    mov r12, #1
    bl PROC_LED_TOGGLE
    mov r7, #3
    bl PROC_DELAY 
    
    bl PROC_LED_PANIC
     
	b _start
	
