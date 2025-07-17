/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/


#include "soc/rtc_cntl_reg.h"
#include "soc/soc_ulp.h"

	/* ADC1 channel 4, GPIO32 */
	.set adc_channel_0, 4
	/* ADC1 channel 5, GPIO33 */
	.set adc_channel_1, 5
	/* ADC1 channel 6, GPIO34 */
	.set adc_channel_2, 6
	/* ADC1 channel 7, GPIO35 */
	.set adc_channel_3, 7
	/* Configure the number of ADC samples to average on each measurement.
	   For convenience, make it a power of 2. */
	.set adc_oversampling_factor_log, 4
	.set adc_oversampling_factor, (1 << adc_oversampling_factor_log)

.data 	/* .data section */
	/* Define variables, which go into .bss section (zero-initialized data) */
	.bss
	.global adc_data
adc_data:
	.skip 32 					/* 128bit, 8 x 4(byte) */

	/* Code goes into .text section */
	.text
	.global entry
entry:
	/* do measurements using ADC */
	/* zero accumulators */
	move r0, 0
    move r2, adc_data
    move r3, 4096
    st r0, r2, 0
    st r0, r2, 4
    st r0, r2, 8
    st r0, r2, 12
	/* initialize the loop counter */
	stage_rst
measure:
    /* measure ch 0 */
    ld r0, r2, 0
    adc r1, 0, adc_channel_0 + 1
    add r0, r0, r1
    st r0, r2, 0
    /* measure ch 1 */
    ld r0, r2, 4
    adc r1, 0, adc_channel_1 + 1
    add r0, r0, r1
    st r0, r2, 4
     /* measure ch 2 */
    ld r0, r2, 8
    adc r1, 0, adc_channel_2 + 1
    add r0, r0, r1
    st r0, r2, 8
    /* measure ch 3 */
    ld r0, r2, 12
    adc r1, 0, adc_channel_3 + 1
    add r0, r0, r1
    st r0, r2, 12
    stage_inc 1
    jumps measure, adc_oversampling_factor, lt
    /* oversampling */
    /* ch 0 */
    ld r0, r2, 0
    rsh r0, r0, adc_oversampling_factor_log
    sub r0, r3, r0
    st r0, r2, 0
    /* ch 1 */
    ld r0, r2, 4
    rsh r0, r0, adc_oversampling_factor_log
    sub r0, r3, r0
    st r0, r2, 4
    /* ch 2 */
    ld r0, r2, 8
    rsh r0, r0, adc_oversampling_factor_log
    //sub r0, r3, r0
    st r0, r2, 8
    /* ch 3 */
    ld r0, r2, 12
    rsh r0, r0, adc_oversampling_factor_log
    //sub r0, r3, r0
    st r0, r2, 12
    /* send interrupt */
    wake
    /* idle for xx us, hand tuned to approx 44100/32 Hz sampling rate */
    //wait 1500 //1518
    //jump entry
    halt