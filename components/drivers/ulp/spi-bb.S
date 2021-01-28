#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"
#include "stack.S"

    // Macros for pin access
    .macro read_MISO 
        READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + 11, 1) 
    .endm

    .macro clear_SCLK
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + 15, 1, 1)
    .endm
    .macro set_SCLK
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + 15, 1, 1)
    .endm
    .macro toggle_SCLK
        set_SCLK
        clear_SCLK
    .endm

    .macro clear_MOSI
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + 14, 1, 1)
    .endm
    .macro set_MOSI
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + 14, 1, 1)
    .endm

    .macro clear_CS_WM8731
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + 10, 1, 1)
    .endm
    .macro set_CS_WM8731
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + 10, 1, 1)
    .endm

    .macro clear_CS_MCP3208
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + 9, 1, 1)
    .endm
    .macro set_CS_MCP3208
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + 9, 1, 1)
    .endm

	/* Define variables, which go into .bss section (zero-initialized data) */
	.bss
	.global stack
stack:
	.skip 100
	.global stackEnd
stackEnd:
	.long 0

	.global adc_data
adc_data:
	.skip 64 					/* 128bit, 8 x 4(byte) */

    .global adc_data_offset
adc_data_offset:
    .long 0


	/* Code goes into .text section */
	.text
	.global entry
entry:
    // read channel 0
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    clear_MOSI
    toggle_SCLK
    toggle_SCLK
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop0:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop0, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2 // 4096 minus ADC value due to electronic design
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 0

    // read channel 1
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    clear_MOSI
    toggle_SCLK
    toggle_SCLK
    set_MOSI
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop1:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop1, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 4

    // read channel 2
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    clear_MOSI
    toggle_SCLK
    set_MOSI
    toggle_SCLK
    clear_MOSI
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop2:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop2, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 8

   // read channel 3
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    clear_MOSI
    toggle_SCLK
    set_MOSI
    toggle_SCLK
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop3:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop3, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 12

   // read channel 4
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    toggle_SCLK
    clear_MOSI
    toggle_SCLK
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop4:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop4, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 16

   // read channel 5
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    toggle_SCLK
    clear_MOSI
    toggle_SCLK
    set_MOSI
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop5:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop5, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 20

   // read channel 6
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    toggle_SCLK
    toggle_SCLK
    clear_MOSI
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop6:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop6, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 24

   // read channel 7
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    toggle_SCLK
    toggle_SCLK
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop7:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop7, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 28

    // set offset in storage
    move r0, adc_data_offset
    st r1, r0, 0
    /* send interrupt */
    wake
    halt



