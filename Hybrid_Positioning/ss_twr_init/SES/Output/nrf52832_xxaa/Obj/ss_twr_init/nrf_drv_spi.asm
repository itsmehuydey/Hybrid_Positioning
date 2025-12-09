	.cpu cortex-m4
	.arch armv7e-m
	.fpu fpv4-sp-d16
	.eabi_attribute 27, 1
	.eabi_attribute 28, 1
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 6
	.eabi_attribute 34, 1
	.eabi_attribute 38, 1
	.eabi_attribute 18, 4
	.file	"nrf_drv_spi.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.file 1 "D:\\TN\\Hybrid_Positioning\\nRF5_SDK_14.2.0\\components\\drivers_nrf\\spi_master\\nrf_drv_spi.c"
	.section	.text.__NVIC_DisableIRQ,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	__NVIC_DisableIRQ, %function
__NVIC_DisableIRQ:
.LFB121:
	.file 2 "D:/OLD_D/nRF5_SDK_17.1.0_ddde560/components/toolchain/cmsis/include/core_cm4.h"
	.loc 2 1718 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	mov	r3, r0
	strb	r3, [sp, #7]
	.loc 2 1719 6
	ldrsb	r3, [sp, #7]
	cmp	r3, #0
	blt	.L3
	.loc 2 1721 81
	ldrb	r3, [sp, #7]	@ zero_extendqisi2
	and	r2, r3, #31
	.loc 2 1721 9
	ldr	r1, .L4
	.loc 2 1721 18
	ldrsb	r3, [sp, #7]
	.loc 2 1721 34
	lsrs	r3, r3, #5
	.loc 2 1721 45
	movs	r0, #1
	lsl	r2, r0, r2
	.loc 2 1721 43
	adds	r3, r3, #32
	str	r2, [r1, r3, lsl #2]
.LBB6:
.LBB7:
	.file 3 "D:/OLD_D/nRF5_SDK_17.1.0_ddde560/components/toolchain/cmsis/include/cmsis_gcc.h"
	.loc 3 946 3
	.syntax unified
@ 946 "D:/OLD_D/nRF5_SDK_17.1.0_ddde560/components/toolchain/cmsis/include/cmsis_gcc.h" 1
	dsb 0xF
@ 0 "" 2
	.loc 3 947 1
	.thumb
	.syntax unified
	nop
.LBE7:
.LBE6:
.LBB8:
.LBB9:
	.loc 3 935 3
	.syntax unified
@ 935 "D:/OLD_D/nRF5_SDK_17.1.0_ddde560/components/toolchain/cmsis/include/cmsis_gcc.h" 1
	isb 0xF
@ 0 "" 2
	.loc 3 936 1
	.thumb
	.syntax unified
	nop
.L3:
.LBE9:
.LBE8:
	.loc 2 1725 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L5:
	.align	2
.L4:
	.word	-536813312
	.cfi_endproc
.LFE121:
	.size	__NVIC_DisableIRQ, .-__NVIC_DisableIRQ
	.section	.text.nrf_spi_event_clear,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_event_clear, %function
nrf_spi_event_clear:
.LFB161:
	.file 4 "../../../nRF5_SDK_14.2.0/components/drivers_nrf/hal/nrf_spi.h"
	.loc 4 260 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	r0, [sp, #4]
	mov	r3, r1
	strh	r3, [sp, #2]	@ movhi
	.loc 4 261 48
	ldrh	r3, [sp, #2]
	.loc 4 261 7
	ldr	r2, [sp, #4]
	add	r3, r3, r2
	.loc 4 261 70
	movs	r2, #0
	str	r2, [r3]
	.loc 4 263 74
	ldrh	r3, [sp, #2]
	.loc 4 263 33
	ldr	r2, [sp, #4]
	add	r3, r3, r2
	.loc 4 263 31
	ldr	r3, [r3]
	.loc 4 263 23
	str	r3, [sp, #12]
	.loc 4 264 5
	ldr	r3, [sp, #12]
	.loc 4 266 1
	nop
	add	sp, sp, #16
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE161:
	.size	nrf_spi_event_clear, .-nrf_spi_event_clear
	.section	.text.nrf_spi_event_check,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_event_check, %function
nrf_spi_event_check:
.LFB162:
	.loc 4 270 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	mov	r3, r1
	strh	r3, [sp, #2]	@ movhi
	.loc 4 271 60
	ldrh	r3, [sp, #2]
	.loc 4 271 19
	ldr	r2, [sp, #4]
	add	r3, r3, r2
	.loc 4 271 18
	ldr	r3, [r3]
	.loc 4 271 12
	cmp	r3, #0
	ite	ne
	movne	r3, #1
	moveq	r3, #0
	uxtb	r3, r3
	.loc 4 272 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE162:
	.size	nrf_spi_event_check, .-nrf_spi_event_check
	.section	.text.nrf_spi_int_enable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_int_enable, %function
nrf_spi_int_enable:
.LFB164:
	.loc 4 282 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	str	r1, [sp]
	.loc 4 283 21
	ldr	r3, [sp, #4]
	ldr	r2, [sp]
	str	r2, [r3, #772]
	.loc 4 284 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE164:
	.size	nrf_spi_int_enable, .-nrf_spi_int_enable
	.section	.text.nrf_spi_int_disable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_int_disable, %function
nrf_spi_int_disable:
.LFB165:
	.loc 4 288 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	str	r1, [sp]
	.loc 4 289 21
	ldr	r3, [sp, #4]
	ldr	r2, [sp]
	str	r2, [r3, #776]
	.loc 4 290 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE165:
	.size	nrf_spi_int_disable, .-nrf_spi_int_disable
	.section	.text.nrf_spi_enable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_enable, %function
nrf_spi_enable:
.LFB167:
	.loc 4 299 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 4 300 19
	ldr	r3, [sp, #4]
	movs	r2, #1
	str	r2, [r3, #1280]
	.loc 4 301 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE167:
	.size	nrf_spi_enable, .-nrf_spi_enable
	.section	.text.nrf_spi_disable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_disable, %function
nrf_spi_disable:
.LFB168:
	.loc 4 304 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 4 305 19
	ldr	r3, [sp, #4]
	movs	r2, #0
	str	r2, [r3, #1280]
	.loc 4 306 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE168:
	.size	nrf_spi_disable, .-nrf_spi_disable
	.section	.text.nrf_spi_pins_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_pins_set, %function
nrf_spi_pins_set:
.LFB169:
	.loc 4 312 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	str	r2, [sp, #4]
	str	r3, [sp]
	.loc 4 313 21
	ldr	r3, [sp, #12]
	ldr	r2, [sp, #8]
	str	r2, [r3, #1288]
	.loc 4 314 21
	ldr	r3, [sp, #12]
	ldr	r2, [sp, #4]
	str	r2, [r3, #1292]
	.loc 4 315 21
	ldr	r3, [sp, #12]
	ldr	r2, [sp]
	str	r2, [r3, #1296]
	.loc 4 316 1
	nop
	add	sp, sp, #16
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE169:
	.size	nrf_spi_pins_set, .-nrf_spi_pins_set
	.section	.text.nrf_spi_txd_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_txd_set, %function
nrf_spi_txd_set:
.LFB170:
	.loc 4 319 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	mov	r3, r1
	strb	r3, [sp, #3]
	.loc 4 320 16
	ldrb	r2, [sp, #3]	@ zero_extendqisi2
	ldr	r3, [sp, #4]
	str	r2, [r3, #1308]
	.loc 4 321 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE170:
	.size	nrf_spi_txd_set, .-nrf_spi_txd_set
	.section	.text.nrf_spi_rxd_get,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_rxd_get, %function
nrf_spi_rxd_get:
.LFB171:
	.loc 4 324 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 4 325 17
	ldr	r3, [sp, #4]
	ldr	r3, [r3, #1304]
	uxtb	r3, r3
	.loc 4 326 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE171:
	.size	nrf_spi_rxd_get, .-nrf_spi_rxd_get
	.section	.text.nrf_spi_frequency_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_frequency_set, %function
nrf_spi_frequency_set:
.LFB172:
	.loc 4 330 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	str	r1, [sp]
	.loc 4 331 22
	ldr	r2, [sp]
	ldr	r3, [sp, #4]
	str	r2, [r3, #1316]
	.loc 4 332 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE172:
	.size	nrf_spi_frequency_set, .-nrf_spi_frequency_set
	.section	.text.nrf_spi_configure,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_spi_configure, %function
nrf_spi_configure:
.LFB173:
	.loc 4 337 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	r0, [sp, #4]
	mov	r3, r1
	strb	r3, [sp, #3]
	mov	r3, r2
	strb	r3, [sp, #2]
	.loc 4 339 35
	ldrb	r3, [sp, #2]	@ zero_extendqisi2
	cmp	r3, #0
	ite	ne
	movne	r3, #1
	moveq	r3, #0
	uxtb	r3, r3
	.loc 4 338 14
	str	r3, [sp, #12]
	.loc 4 340 5
	ldrb	r3, [sp, #3]	@ zero_extendqisi2
	cmp	r3, #3
	beq	.L19
	cmp	r3, #3
	bgt	.L24
	cmp	r3, #1
	beq	.L21
	cmp	r3, #2
	beq	.L22
	.loc 4 346 9
	b	.L24
.L21:
	.loc 4 349 16
	ldr	r3, [sp, #12]
	orr	r3, r3, #2
	str	r3, [sp, #12]
	.loc 4 351 9
	b	.L23
.L22:
	.loc 4 354 16
	ldr	r3, [sp, #12]
	orr	r3, r3, #4
	str	r3, [sp, #12]
	.loc 4 356 9
	b	.L23
.L19:
	.loc 4 359 16
	ldr	r3, [sp, #12]
	orr	r3, r3, #6
	str	r3, [sp, #12]
	.loc 4 361 9
	b	.L23
.L24:
	.loc 4 346 9
	nop
.L23:
	.loc 4 363 19
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #12]
	str	r2, [r3, #1364]
	.loc 4 364 1
	nop
	add	sp, sp, #16
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE173:
	.size	nrf_spi_configure, .-nrf_spi_configure
	.section	.text.nrf_drv_common_irq_disable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_drv_common_irq_disable, %function
nrf_drv_common_irq_disable:
.LFB198:
	.file 5 "../../../nRF5_SDK_14.2.0/components/drivers_nrf/common/nrf_drv_common.h"
	.loc 5 302 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #12
	.cfi_def_cfa_offset 16
	mov	r3, r0
	strb	r3, [sp, #7]
	.loc 5 303 5
	ldrsb	r3, [sp, #7]
	mov	r0, r3
	bl	__NVIC_DisableIRQ
	.loc 5 304 1
	nop
	add	sp, sp, #12
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE198:
	.size	nrf_drv_common_irq_disable, .-nrf_drv_common_irq_disable
	.section	.text.nrf_gpio_pin_port_decode,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_pin_port_decode, %function
nrf_gpio_pin_port_decode:
.LFB204:
	.file 6 "../../../nRF5_SDK_14.2.0/components/drivers_nrf/hal/nrf_gpio.h"
	.loc 6 463 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 6 467 12
	mov	r3, #1342177280
	.loc 6 479 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE204:
	.size	nrf_gpio_pin_port_decode, .-nrf_gpio_pin_port_decode
	.section	.text.nrf_gpio_cfg,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_cfg, %function
nrf_gpio_cfg:
.LFB207:
	.loc 6 511 1
	.cfi_startproc
	@ args = 8, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	mov	r3, r0
	strb	r3, [sp, #3]
	mov	r3, r1
	strb	r3, [sp, #2]
	mov	r3, r2
	strb	r3, [sp, #1]
	.loc 6 512 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 6 514 47
	ldrb	r2, [sp, #3]	@ zero_extendqisi2
	.loc 6 515 35
	ldrb	r3, [sp, #2]	@ zero_extendqisi2
	.loc 6 515 51
	lsls	r3, r3, #1
	.loc 6 515 32
	orrs	r2, r2, r3
	.loc 6 516 35
	ldrb	r3, [sp, #1]	@ zero_extendqisi2
	.loc 6 516 50
	lsls	r3, r3, #2
	.loc 6 516 32
	orrs	r2, r2, r3
	.loc 6 517 35
	ldrb	r3, [sp, #24]	@ zero_extendqisi2
	.loc 6 517 51
	lsls	r3, r3, #8
	.loc 6 517 32
	orr	r1, r2, r3
	.loc 6 518 35
	ldrb	r3, [sp, #28]	@ zero_extendqisi2
	.loc 6 518 51
	lsls	r3, r3, #16
	.loc 6 514 17
	ldr	r2, [sp, #4]
	.loc 6 518 32
	orrs	r1, r1, r3
	.loc 6 514 30
	ldr	r3, [sp, #12]
	add	r2, r2, #448
	str	r1, [r3, r2, lsl #2]
	.loc 6 519 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE207:
	.size	nrf_gpio_cfg, .-nrf_gpio_cfg
	.section	.text.nrf_gpio_cfg_output,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_cfg_output, %function
nrf_gpio_cfg_output:
.LFB208:
	.loc 6 523 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #12]
	.loc 6 524 5
	movs	r3, #0
	str	r3, [sp, #4]
	movs	r3, #0
	str	r3, [sp]
	movs	r3, #0
	movs	r2, #1
	movs	r1, #1
	ldr	r0, [sp, #12]
	bl	nrf_gpio_cfg
	.loc 6 531 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE208:
	.size	nrf_gpio_cfg_output, .-nrf_gpio_cfg_output
	.section	.text.nrf_gpio_cfg_input,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_cfg_input, %function
nrf_gpio_cfg_input:
.LFB209:
	.loc 6 535 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #12]
	mov	r3, r1
	strb	r3, [sp, #11]
	.loc 6 536 5
	ldrb	r3, [sp, #11]	@ zero_extendqisi2
	movs	r2, #0
	str	r2, [sp, #4]
	movs	r2, #0
	str	r2, [sp]
	movs	r2, #0
	movs	r1, #0
	ldr	r0, [sp, #12]
	bl	nrf_gpio_cfg
	.loc 6 543 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE209:
	.size	nrf_gpio_cfg_input, .-nrf_gpio_cfg_input
	.section	.text.nrf_gpio_pin_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_pin_set, %function
nrf_gpio_pin_set:
.LFB216:
	.loc 6 623 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	.loc 6 624 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 6 626 5
	ldr	r3, [sp, #4]
	movs	r2, #1
	lsl	r3, r2, r3
	mov	r1, r3
	ldr	r0, [sp, #12]
	bl	nrf_gpio_port_out_set
	.loc 6 627 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE216:
	.size	nrf_gpio_pin_set, .-nrf_gpio_pin_set
	.section	.text.nrf_gpio_pin_clear,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_pin_clear, %function
nrf_gpio_pin_clear:
.LFB217:
	.loc 6 631 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	.loc 6 632 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 6 634 5
	ldr	r3, [sp, #4]
	movs	r2, #1
	lsl	r3, r2, r3
	mov	r1, r3
	ldr	r0, [sp, #12]
	bl	nrf_gpio_port_out_clear
	.loc 6 635 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE217:
	.size	nrf_gpio_pin_clear, .-nrf_gpio_pin_clear
	.section	.text.nrf_gpio_port_out_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_port_out_set, %function
nrf_gpio_port_out_set:
.LFB230:
	.loc 6 729 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	str	r1, [sp]
	.loc 6 730 19
	ldr	r3, [sp, #4]
	ldr	r2, [sp]
	str	r2, [r3, #1288]
	.loc 6 731 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE230:
	.size	nrf_gpio_port_out_set, .-nrf_gpio_port_out_set
	.section	.text.nrf_gpio_port_out_clear,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_port_out_clear, %function
nrf_gpio_port_out_clear:
.LFB231:
	.loc 6 735 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	str	r1, [sp]
	.loc 6 736 19
	ldr	r3, [sp, #4]
	ldr	r2, [sp]
	str	r2, [r3, #1292]
	.loc 6 737 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE231:
	.size	nrf_gpio_port_out_clear, .-nrf_gpio_port_out_clear
	.section	.bss.m_cb,"aw",%nobits
	.align	2
	.type	m_cb, %object
	.size	m_cb, 36
m_cb:
	.space	36
	.section	.text.nrf_drv_spi_init,"ax",%progbits
	.align	1
	.global	nrf_drv_spi_init
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_drv_spi_init, %function
nrf_drv_spi_init:
.LFB236:
	.loc 1 167 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 40
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #52
	.cfi_def_cfa_offset 56
	str	r0, [sp, #20]
	str	r1, [sp, #16]
	str	r2, [sp, #12]
	str	r3, [sp, #8]
	.loc 1 169 51
	ldr	r3, [sp, #20]
	ldrb	r3, [r3, #5]	@ zero_extendqisi2
	mov	r2, r3
	.loc 1 169 27
	mov	r3, r2
	lsls	r3, r3, #3
	add	r3, r3, r2
	lsls	r3, r3, #2
	ldr	r2, .L47
	add	r3, r3, r2
	str	r3, [sp, #36]
	.loc 1 172 13
	ldr	r3, [sp, #36]
	ldrb	r3, [r3, #28]	@ zero_extendqisi2
	.loc 1 172 8
	cmp	r3, #0
	beq	.L36
	.loc 1 174 18
	movs	r3, #8
	str	r3, [sp, #28]
	.loc 1 178 16
	ldr	r3, [sp, #28]
	b	.L37
.L36:
	.loc 1 193 19
	ldr	r3, [sp, #36]
	ldr	r2, [sp, #12]
	str	r2, [r3]
	.loc 1 194 21
	ldr	r3, [sp, #36]
	ldr	r2, [sp, #8]
	str	r2, [r3, #4]
	.loc 1 203 17
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #12]	@ zero_extendqisi2
	.loc 1 203 8
	cmp	r3, #1
	bhi	.L38
	.loc 1 205 36
	ldr	r3, [sp, #16]
	ldrb	r3, [r3]	@ zero_extendqisi2
	.loc 1 205 9
	mov	r0, r3
	bl	nrf_gpio_pin_clear
	b	.L39
.L38:
	.loc 1 209 34
	ldr	r3, [sp, #16]
	ldrb	r3, [r3]	@ zero_extendqisi2
	.loc 1 209 9
	mov	r0, r3
	bl	nrf_gpio_pin_set
.L39:
	.loc 1 211 26
	ldr	r3, [sp, #16]
	ldrb	r3, [r3]	@ zero_extendqisi2
	.loc 1 211 5
	mov	r0, r3
	movs	r3, #0
	str	r3, [sp, #4]
	movs	r3, #0
	str	r3, [sp]
	movs	r3, #0
	movs	r2, #0
	movs	r1, #1
	bl	nrf_gpio_cfg
	.loc 1 218 17
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #1]	@ zero_extendqisi2
	.loc 1 218 8
	cmp	r3, #255
	beq	.L40
	.loc 1 220 28
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #1]	@ zero_extendqisi2
	.loc 1 220 18
	str	r3, [sp, #44]
	.loc 1 221 9
	ldr	r0, [sp, #44]
	bl	nrf_gpio_pin_clear
	.loc 1 222 9
	ldr	r0, [sp, #44]
	bl	nrf_gpio_cfg_output
	b	.L41
.L40:
	.loc 1 226 18
	mov	r3, #-1
	str	r3, [sp, #44]
.L41:
	.loc 1 229 17
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #2]	@ zero_extendqisi2
	.loc 1 229 8
	cmp	r3, #255
	beq	.L42
	.loc 1 231 28
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #2]	@ zero_extendqisi2
	.loc 1 231 18
	str	r3, [sp, #40]
	.loc 1 232 9
	movs	r1, #1
	ldr	r0, [sp, #40]
	bl	nrf_gpio_cfg_input
	b	.L43
.L42:
	.loc 1 236 18
	mov	r3, #-1
	str	r3, [sp, #40]
.L43:
	.loc 1 239 17
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #3]	@ zero_extendqisi2
	.loc 1 239 8
	cmp	r3, #255
	beq	.L44
	.loc 1 241 34
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #3]	@ zero_extendqisi2
	.loc 1 241 9
	mov	r0, r3
	bl	nrf_gpio_pin_set
	.loc 1 242 37
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #3]	@ zero_extendqisi2
	.loc 1 242 9
	mov	r0, r3
	bl	nrf_gpio_cfg_output
.L44:
	.loc 1 244 20
	ldr	r3, [sp, #20]
	ldrb	r3, [r3, #5]	@ zero_extendqisi2
	mov	r1, r3
	.loc 1 244 53
	ldr	r3, [sp, #16]
	ldrb	r0, [r3, #3]	@ zero_extendqisi2
	.loc 1 244 43
	ldr	r2, .L47
	mov	r3, r1
	lsls	r3, r3, #3
	add	r3, r3, r1
	lsls	r3, r3, #2
	add	r3, r3, r2
	adds	r3, r3, #30
	mov	r2, r0
	strb	r2, [r3]
.LBB10:
	.loc 1 265 5
	ldr	r3, [sp, #20]
	ldr	r3, [r3]
	str	r3, [sp, #32]
	ldr	r3, [sp, #16]
	ldrb	r3, [r3]	@ zero_extendqisi2
	mov	r1, r3
	ldr	r3, [sp, #40]
	ldr	r2, [sp, #44]
	ldr	r0, [sp, #32]
	bl	nrf_spi_pins_set
	.loc 1 265 5 is_stmt 0 discriminator 1
	ldr	r3, [sp, #16]
	ldr	r3, [r3, #8]
	mov	r1, r3
	ldr	r0, [sp, #32]
	bl	nrf_spi_frequency_set
	.loc 1 265 5 discriminator 2
	ldr	r3, [sp, #16]
	ldrb	r1, [r3, #12]	@ zero_extendqisi2
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #13]	@ zero_extendqisi2
	mov	r2, r3
	ldr	r0, [sp, #32]
	bl	nrf_spi_configure
	.loc 1 265 5 discriminator 3
	ldr	r3, [sp, #20]
	ldrb	r3, [r3, #5]	@ zero_extendqisi2
	mov	r1, r3
	ldr	r3, [sp, #16]
	ldrb	r0, [r3, #5]	@ zero_extendqisi2
	ldr	r2, .L47
	mov	r3, r1
	lsls	r3, r3, #3
	add	r3, r3, r1
	lsls	r3, r3, #2
	add	r3, r3, r2
	adds	r3, r3, #31
	mov	r2, r0
	strb	r2, [r3]
	ldr	r3, [sp, #36]
	ldr	r3, [r3]
	cmp	r3, #0
	beq	.L45
	.loc 1 265 5 discriminator 1
	movs	r1, #4
	ldr	r0, [sp, #32]
	bl	nrf_spi_int_enable
.L45:
	.loc 1 265 5 discriminator 3
	ldr	r0, [sp, #32]
	bl	nrf_spi_enable
.LBE10:
	.loc 1 285 13 is_stmt 1
	ldr	r3, [sp, #36]
	ldr	r3, [r3]
	.loc 1 285 8
	cmp	r3, #0
	beq	.L46
	.loc 1 287 45
	ldr	r3, [sp, #20]
	ldrsb	r2, [r3, #4]
	.loc 1 287 60
	ldr	r3, [sp, #16]
	ldrb	r3, [r3, #4]	@ zero_extendqisi2
	.loc 1 287 9
	mov	r1, r3
	mov	r0, r2
	bl	nrf_drv_common_irq_enable
.L46:
	.loc 1 290 32
	ldr	r3, [sp, #36]
	movs	r2, #0
	strb	r2, [r3, #29]
	.loc 1 291 17
	ldr	r3, [sp, #36]
	movs	r2, #1
	strb	r2, [r3, #28]
	.loc 1 295 14
	movs	r3, #0
	str	r3, [sp, #28]
	.loc 1 299 12
	ldr	r3, [sp, #28]
.L37:
	.loc 1 300 1
	mov	r0, r3
	add	sp, sp, #52
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
.L48:
	.align	2
.L47:
	.word	m_cb
	.cfi_endproc
.LFE236:
	.size	nrf_drv_spi_init, .-nrf_drv_spi_init
	.section	.text.nrf_drv_spi_uninit,"ax",%progbits
	.align	1
	.global	nrf_drv_spi_uninit
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_drv_spi_uninit, %function
nrf_drv_spi_uninit:
.LFB237:
	.loc 1 303 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	.loc 1 304 50
	ldr	r3, [sp, #4]
	ldrb	r3, [r3, #5]	@ zero_extendqisi2
	mov	r2, r3
	.loc 1 304 27
	mov	r3, r2
	lsls	r3, r3, #3
	add	r3, r3, r2
	lsls	r3, r3, #2
	ldr	r2, .L52
	add	r3, r3, r2
	str	r3, [sp, #12]
	.loc 1 307 13
	ldr	r3, [sp, #12]
	ldr	r3, [r3]
	.loc 1 307 8
	cmp	r3, #0
	beq	.L50
	.loc 1 309 46
	ldr	r3, [sp, #4]
	ldrsb	r3, [r3, #4]
	.loc 1 309 9
	mov	r0, r3
	bl	nrf_drv_common_irq_disable
.L50:
.LBB11:
	.loc 1 331 5
	ldr	r3, [sp, #4]
	ldr	r3, [r3]
	str	r3, [sp, #8]
	ldr	r3, [sp, #12]
	ldr	r3, [r3]
	cmp	r3, #0
	beq	.L51
	.loc 1 331 5 is_stmt 0 discriminator 1
	mov	r1, #-1
	ldr	r0, [sp, #8]
	bl	nrf_spi_int_disable
.L51:
	.loc 1 331 5 discriminator 3
	ldr	r0, [sp, #8]
	bl	nrf_spi_disable
.LBE11:
	.loc 1 346 17 is_stmt 1
	ldr	r3, [sp, #12]
	movs	r2, #0
	strb	r2, [r3, #28]
	.loc 1 347 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
.L53:
	.align	2
.L52:
	.word	m_cb
	.cfi_endproc
.LFE237:
	.size	nrf_drv_spi_uninit, .-nrf_drv_spi_uninit
	.section	.text.nrf_drv_spi_transfer,"ax",%progbits
	.align	1
	.global	nrf_drv_spi_transfer
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_drv_spi_transfer, %function
nrf_drv_spi_transfer:
.LFB238:
	.loc 1 354 1
	.cfi_startproc
	@ args = 4, pretend = 0, frame = 32
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #36
	.cfi_def_cfa_offset 40
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	str	r3, [sp]
	mov	r3, r2
	strb	r3, [sp, #7]
	.loc 1 356 27
	ldr	r3, [sp, #8]
	str	r3, [sp, #16]
	.loc 1 357 27
	ldr	r3, [sp]
	str	r3, [sp, #24]
	.loc 1 358 27
	ldrb	r3, [sp, #7]
	strb	r3, [sp, #20]
	.loc 1 359 27
	ldrb	r3, [sp, #40]
	strb	r3, [sp, #28]
	.loc 1 364 12
	add	r3, sp, #16
	movs	r2, #0
	mov	r1, r3
	ldr	r0, [sp, #12]
	bl	nrf_drv_spi_xfer
	mov	r3, r0
	.loc 1 365 1
	mov	r0, r3
	add	sp, sp, #36
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE238:
	.size	nrf_drv_spi_transfer, .-nrf_drv_spi_transfer
	.section	.text.finish_transfer,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	finish_transfer, %function
finish_transfer:
.LFB239:
	.loc 1 368 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #12
	.cfi_def_cfa_offset 16
	str	r0, [sp, #4]
	.loc 1 370 13
	ldr	r3, [sp, #4]
	ldrb	r3, [r3, #30]	@ zero_extendqi