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
	.file	"nrf_drv_gpiote.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.file 1 "D:\\TN\\Hybrid_Positioning\\nRF5_SDK_14.2.0\\components\\drivers_nrf\\gpiote\\nrf_drv_gpiote.c"
	.section	.text.nrf_gpiote_task_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_task_set, %function
nrf_gpiote_task_set:
.LFB161:
	.file 2 "../../../nRF5_SDK_14.2.0/components/drivers_nrf/hal/nrf_gpiote.h"
	.loc 2 308 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	mov	r3, r0
	strb	r3, [sp, #7]
	.loc 2 309 45
	ldrb	r3, [sp, #7]	@ zero_extendqisi2
	add	r3, r3, #1073741824
	add	r3, r3, #24576
	.loc 2 309 6
	mov	r2, r3
	.loc 2 309 53
	movs	r3, #1
	str	r3, [r2]
	.loc 2 310 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE161:
	.size	nrf_gpiote_task_set, .-nrf_gpiote_task_set
	.section	.text.nrf_gpiote_task_addr_get,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_task_addr_get, %function
nrf_gpiote_task_addr_get:
.LFB162:
	.loc 2 313 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	mov	r3, r0
	strb	r3, [sp, #7]
	.loc 2 314 34
	ldrb	r3, [sp, #7]	@ zero_extendqisi2
	add	r3, r3, #1073741824
	add	r3, r3, #24576
	.loc 2 315 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE162:
	.size	nrf_gpiote_task_addr_get, .-nrf_gpiote_task_addr_get
	.section	.text.nrf_gpiote_event_is_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_event_is_set, %function
nrf_gpiote_event_is_set:
.LFB163:
	.loc 2 318 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #12
	.cfi_def_cfa_offset 16
	mov	r3, r0
	strh	r3, [sp, #6]	@ movhi
	.loc 2 319 26
	ldrh	r3, [sp, #6]
	mov	r0, r3
	bl	nrf_gpiote_event_addr_get
	mov	r3, r0
	.loc 2 319 13 discriminator 1
	ldr	r3, [r3]
	.loc 2 319 76 discriminator 1
	cmp	r3, #1
	ite	eq
	moveq	r3, #1
	movne	r3, #0
	uxtb	r3, r3
	.loc 2 320 1
	mov	r0, r3
	add	sp, sp, #12
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE163:
	.size	nrf_gpiote_event_is_set, .-nrf_gpiote_event_is_set
	.section	.text.nrf_gpiote_event_clear,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_event_clear, %function
nrf_gpiote_event_clear:
.LFB164:
	.loc 2 323 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	mov	r3, r0
	strh	r3, [sp, #6]	@ movhi
	.loc 2 324 18
	ldrh	r3, [sp, #6]
	mov	r0, r3
	bl	nrf_gpiote_event_addr_get
	mov	r3, r0
	.loc 2 324 6 discriminator 1
	mov	r2, r3
	.loc 2 324 51 discriminator 1
	movs	r3, #0
	str	r3, [r2]
	.loc 2 326 54
	ldrh	r3, [sp, #6]
	mov	r0, r3
	bl	nrf_gpiote_event_addr_get
	mov	r3, r0
	.loc 2 326 31 discriminator 1
	ldr	r3, [r3]
	.loc 2 326 23 discriminator 1
	str	r3, [sp, #12]
	.loc 2 327 5
	ldr	r3, [sp, #12]
	.loc 2 329 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE164:
	.size	nrf_gpiote_event_clear, .-nrf_gpiote_event_clear
	.section	.text.nrf_gpiote_event_addr_get,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_event_addr_get, %function
nrf_gpiote_event_addr_get:
.LFB165:
	.loc 2 332 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	mov	r3, r0
	strh	r3, [sp, #6]	@ movhi
	.loc 2 333 34
	ldrh	r3, [sp, #6]
	add	r3, r3, #1073741824
	add	r3, r3, #24576
	.loc 2 334 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE165:
	.size	nrf_gpiote_event_addr_get, .-nrf_gpiote_event_addr_get
	.section	.text.nrf_gpiote_int_enable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_int_enable, %function
nrf_gpiote_int_enable:
.LFB166:
	.loc 2 337 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 2 338 15
	ldr	r2, .L10
	.loc 2 338 26
	ldr	r3, [sp, #4]
	str	r3, [r2, #772]
	.loc 2 339 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L11:
	.align	2
.L10:
	.word	1073766400
	.cfi_endproc
.LFE166:
	.size	nrf_gpiote_int_enable, .-nrf_gpiote_int_enable
	.section	.text.nrf_gpiote_int_disable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_int_disable, %function
nrf_gpiote_int_disable:
.LFB167:
	.loc 2 342 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 2 343 15
	ldr	r2, .L13
	.loc 2 343 26
	ldr	r3, [sp, #4]
	str	r3, [r2, #776]
	.loc 2 344 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L14:
	.align	2
.L13:
	.word	1073766400
	.cfi_endproc
.LFE167:
	.size	nrf_gpiote_int_disable, .-nrf_gpiote_int_disable
	.section	.text.nrf_gpiote_int_is_enabled,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_int_is_enabled, %function
nrf_gpiote_int_is_enabled:
.LFB168:
	.loc 2 347 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 2 348 23
	ldr	r3, .L17
	ldr	r2, [r3, #772]
	.loc 2 348 34
	ldr	r3, [sp, #4]
	ands	r3, r3, r2
	.loc 2 349 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L18:
	.align	2
.L17:
	.word	1073766400
	.cfi_endproc
.LFE168:
	.size	nrf_gpiote_int_is_enabled, .-nrf_gpiote_int_is_enabled
	.section	.text.nrf_gpiote_event_enable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_event_enable, %function
nrf_gpiote_event_enable:
.LFB169:
	.loc 2 352 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 2 353 14
	ldr	r2, .L20
	.loc 2 353 22
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	ldr	r3, [r2, r3, lsl #2]
	.loc 2 353 14
	ldr	r1, .L20
	.loc 2 353 28
	orr	r2, r3, #1
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	str	r2, [r1, r3, lsl #2]
	.loc 2 354 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L21:
	.align	2
.L20:
	.word	1073766400
	.cfi_endproc
.LFE169:
	.size	nrf_gpiote_event_enable, .-nrf_gpiote_event_enable
	.section	.text.nrf_gpiote_event_disable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_event_disable, %function
nrf_gpiote_event_disable:
.LFB170:
	.loc 2 357 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 2 358 14
	ldr	r2, .L23
	.loc 2 358 22
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	ldr	r3, [r2, r3, lsl #2]
	.loc 2 358 14
	ldr	r1, .L23
	.loc 2 358 28
	bic	r2, r3, #1
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	str	r2, [r1, r3, lsl #2]
	.loc 2 359 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L24:
	.align	2
.L23:
	.word	1073766400
	.cfi_endproc
.LFE170:
	.size	nrf_gpiote_event_disable, .-nrf_gpiote_event_disable
	.section	.text.nrf_gpiote_event_configure,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_event_configure, %function
nrf_gpiote_event_configure:
.LFB171:
	.loc 2 362 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	mov	r3, r2
	strb	r3, [sp, #7]
	.loc 2 363 13
	ldr	r2, .L26
	.loc 2 363 21
	ldr	r3, [sp, #12]
	add	r3, r3, #324
	ldr	r3, [r2, r3, lsl #2]
	.loc 2 363 13
	ldr	r1, .L26
	.loc 2 363 27
	bic	r3, r3, #203776
	bic	r3, r3, #768
	ldr	r2, [sp, #12]
	add	r2, r2, #324
	str	r3, [r1, r2, lsl #2]
	.loc 2 364 13
	ldr	r2, .L26
	.loc 2 364 21
	ldr	r3, [sp, #12]
	add	r3, r3, #324
	ldr	r2, [r2, r3, lsl #2]
	.loc 2 364 36
	ldr	r3, [sp, #8]
	lsls	r3, r3, #8
	.loc 2 364 63
	and	r1, r3, #7936
	.loc 2 365 42
	ldrb	r3, [sp, #7]	@ zero_extendqisi2
	lsls	r3, r3, #16
	.loc 2 365 73
	and	r3, r3, #196608
	.loc 2 364 93
	orrs	r3, r3, r1
	.loc 2 364 13
	ldr	r1, .L26
	.loc 2 364 27
	orrs	r2, r2, r3
	ldr	r3, [sp, #12]
	add	r3, r3, #324
	str	r2, [r1, r3, lsl #2]
	.loc 2 366 1
	nop
	add	sp, sp, #16
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L27:
	.align	2
.L26:
	.word	1073766400
	.cfi_endproc
.LFE171:
	.size	nrf_gpiote_event_configure, .-nrf_gpiote_event_configure
	.section	.text.nrf_gpiote_event_pin_get,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_event_pin_get, %function
nrf_gpiote_event_pin_get:
.LFB172:
	.loc 2 369 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 2 370 24
	ldr	r2, .L30
	.loc 2 370 32
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	ldr	r3, [r2, r3, lsl #2]
	.loc 2 370 68
	lsrs	r3, r3, #8
	and	r3, r3, #31
	.loc 2 371 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L31:
	.align	2
.L30:
	.word	1073766400
	.cfi_endproc
.LFE172:
	.size	nrf_gpiote_event_pin_get, .-nrf_gpiote_event_pin_get
	.section	.text.nrf_gpiote_event_polarity_get,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_event_polarity_get, %function
nrf_gpiote_event_polarity_get:
.LFB173:
	.loc 2 374 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 2 375 47
	ldr	r2, .L34
	.loc 2 375 55
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	ldr	r3, [r2, r3, lsl #2]
	.loc 2 375 91
	lsrs	r3, r3, #16
	and	r3, r3, #3
	.loc 2 375 12
	uxtb	r3, r3
	.loc 2 376 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L35:
	.align	2
.L34:
	.word	1073766400
	.cfi_endproc
.LFE173:
	.size	nrf_gpiote_event_polarity_get, .-nrf_gpiote_event_polarity_get
	.section	.text.nrf_gpiote_task_enable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_task_enable, %function
nrf_gpiote_task_enable:
.LFB174:
	.loc 2 379 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	r0, [sp, #4]
	.loc 2 380 39
	ldr	r2, .L37
	.loc 2 380 47
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	ldr	r3, [r2, r3, lsl #2]
	.loc 2 380 14
	orr	r3, r3, #3
	str	r3, [sp, #12]
	.loc 2 391 15
	ldr	r1, .L37
	.loc 2 391 29
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	ldr	r2, [sp, #12]
	str	r2, [r1, r3, lsl #2]
	.loc 2 392 1
	nop
	add	sp, sp, #16
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L38:
	.align	2
.L37:
	.word	1073766400
	.cfi_endproc
.LFE174:
	.size	nrf_gpiote_task_enable, .-nrf_gpiote_task_enable
	.section	.text.nrf_gpiote_task_disable,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_task_disable, %function
nrf_gpiote_task_disable:
.LFB175:
	.loc 2 395 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 2 396 15
	ldr	r2, .L40
	.loc 2 396 23
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	ldr	r3, [r2, r3, lsl #2]
	.loc 2 396 15
	ldr	r1, .L40
	.loc 2 396 29
	bic	r2, r3, #3
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	str	r2, [r1, r3, lsl #2]
	.loc 2 397 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L41:
	.align	2
.L40:
	.word	1073766400
	.cfi_endproc
.LFE175:
	.size	nrf_gpiote_task_disable, .-nrf_gpiote_task_disable
	.section	.text.nrf_gpiote_task_configure,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_task_configure, %function
nrf_gpiote_task_configure:
.LFB176:
	.loc 2 402 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	mov	r1, r2
	mov	r2, r3
	mov	r3, r1
	strb	r3, [sp, #7]
	mov	r3, r2
	strb	r3, [sp, #6]
	.loc 2 403 13
	ldr	r2, .L43
	.loc 2 403 21
	ldr	r3, [sp, #12]
	add	r3, r3, #324
	ldr	r3, [r2, r3, lsl #2]
	.loc 2 403 13
	ldr	r1, .L43
	.loc 2 403 27
	bic	r3, r3, #1245184
	bic	r3, r3, #7936
	ldr	r2, [sp, #12]
	add	r2, r2, #324
	str	r3, [r1, r2, lsl #2]
	.loc 2 407 13
	ldr	r2, .L43
	.loc 2 407 21
	ldr	r3, [sp, #12]
	add	r3, r3, #324
	ldr	r2, [r2, r3, lsl #2]
	.loc 2 407 36
	ldr	r3, [sp, #8]
	lsls	r3, r3, #8
	.loc 2 407 63
	and	r1, r3, #7936
	.loc 2 408 41
	ldrb	r3, [sp, #7]	@ zero_extendqisi2
	lsls	r3, r3, #16
	.loc 2 408 72
	and	r3, r3, #196608
	.loc 2 407 93
	orrs	r1, r1, r3
	.loc 2 409 41
	ldrb	r3, [sp, #6]	@ zero_extendqisi2
	lsls	r3, r3, #20
	.loc 2 409 71
	and	r3, r3, #1048576
	.loc 2 408 102
	orrs	r3, r3, r1
	.loc 2 407 13
	ldr	r1, .L43
	.loc 2 407 27
	orrs	r2, r2, r3
	ldr	r3, [sp, #12]
	add	r3, r3, #324
	str	r2, [r1, r3, lsl #2]
	.loc 2 410 1
	nop
	add	sp, sp, #16
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L44:
	.align	2
.L43:
	.word	1073766400
	.cfi_endproc
.LFE176:
	.size	nrf_gpiote_task_configure, .-nrf_gpiote_task_configure
	.section	.text.nrf_gpiote_task_force,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_task_force, %function
nrf_gpiote_task_force:
.LFB177:
	.loc 2 413 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	mov	r3, r1
	strb	r3, [sp, #3]
	.loc 2 414 42
	ldr	r2, .L46
	.loc 2 414 50
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	ldr	r3, [r2, r3, lsl #2]
	.loc 2 414 56
	bic	r2, r3, #1048576
	.loc 2 415 44
	ldrb	r3, [sp, #3]	@ zero_extendqisi2
	lsls	r3, r3, #20
	.loc 2 415 74
	and	r3, r3, #1048576
	.loc 2 414 15
	ldr	r1, .L46
	.loc 2 415 31
	orrs	r2, r2, r3
	.loc 2 414 29
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	str	r2, [r1, r3, lsl #2]
	.loc 2 416 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L47:
	.align	2
.L46:
	.word	1073766400
	.cfi_endproc
.LFE177:
	.size	nrf_gpiote_task_force, .-nrf_gpiote_task_force
	.section	.text.nrf_gpiote_te_default,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpiote_te_default, %function
nrf_gpiote_te_default:
.LFB178:
	.loc 2 419 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 2 420 15
	ldr	r2, .L49
	.loc 2 420 29
	ldr	r3, [sp, #4]
	add	r3, r3, #324
	movs	r1, #0
	str	r1, [r2, r3, lsl #2]
	.loc 2 421 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L50:
	.align	2
.L49:
	.word	1073766400
	.cfi_endproc
.LFE178:
	.size	nrf_gpiote_te_default, .-nrf_gpiote_te_default
	.section	.text.nrf_gpio_pin_port_decode,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_pin_port_decode, %function
nrf_gpio_pin_port_decode:
.LFB179:
	.file 3 "../../../nRF5_SDK_14.2.0/components/drivers_nrf/hal/nrf_gpio.h"
	.loc 3 463 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 3 467 12
	mov	r3, #1342177280
	.loc 3 479 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE179:
	.size	nrf_gpio_pin_port_decode, .-nrf_gpio_pin_port_decode
	.section	.text.nrf_gpio_cfg,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_cfg, %function
nrf_gpio_cfg:
.LFB182:
	.loc 3 511 1
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
	.loc 3 512 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 3 514 47
	ldrb	r2, [sp, #3]	@ zero_extendqisi2
	.loc 3 515 35
	ldrb	r3, [sp, #2]	@ zero_extendqisi2
	.loc 3 515 51
	lsls	r3, r3, #1
	.loc 3 515 32
	orrs	r2, r2, r3
	.loc 3 516 35
	ldrb	r3, [sp, #1]	@ zero_extendqisi2
	.loc 3 516 50
	lsls	r3, r3, #2
	.loc 3 516 32
	orrs	r2, r2, r3
	.loc 3 517 35
	ldrb	r3, [sp, #24]	@ zero_extendqisi2
	.loc 3 517 51
	lsls	r3, r3, #8
	.loc 3 517 32
	orr	r1, r2, r3
	.loc 3 518 35
	ldrb	r3, [sp, #28]	@ zero_extendqisi2
	.loc 3 518 51
	lsls	r3, r3, #16
	.loc 3 514 17
	ldr	r2, [sp, #4]
	.loc 3 518 32
	orrs	r1, r1, r3
	.loc 3 514 30
	ldr	r3, [sp, #12]
	add	r2, r2, #448
	str	r1, [r3, r2, lsl #2]
	.loc 3 519 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE182:
	.size	nrf_gpio_cfg, .-nrf_gpio_cfg
	.section	.text.nrf_gpio_cfg_output,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_cfg_output, %function
nrf_gpio_cfg_output:
.LFB183:
	.loc 3 523 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #12]
	.loc 3 524 5
	movs	r3, #0
	str	r3, [sp, #4]
	movs	r3, #0
	str	r3, [sp]
	movs	r3, #0
	movs	r2, #1
	movs	r1, #1
	ldr	r0, [sp, #12]
	bl	nrf_gpio_cfg
	.loc 3 531 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE183:
	.size	nrf_gpio_cfg_output, .-nrf_gpio_cfg_output
	.section	.text.nrf_gpio_cfg_input,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_cfg_input, %function
nrf_gpio_cfg_input:
.LFB184:
	.loc 3 535 1
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
	.loc 3 536 5
	ldrb	r3, [sp, #11]	@ zero_extendqisi2
	movs	r2, #0
	str	r2, [sp, #4]
	movs	r2, #0
	str	r2, [sp]
	movs	r2, #0
	movs	r1, #0
	ldr	r0, [sp, #12]
	bl	nrf_gpio_cfg
	.loc 3 543 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE184:
	.size	nrf_gpio_cfg_input, .-nrf_gpio_cfg_input
	.section	.text.nrf_gpio_cfg_default,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_cfg_default, %function
nrf_gpio_cfg_default:
.LFB185:
	.loc 3 547 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #12]
	.loc 3 548 5
	movs	r3, #0
	str	r3, [sp, #4]
	movs	r3, #0
	str	r3, [sp]
	movs	r3, #0
	movs	r2, #1
	movs	r1, #0
	ldr	r0, [sp, #12]
	bl	nrf_gpio_cfg
	.loc 3 555 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE185:
	.size	nrf_gpio_cfg_default, .-nrf_gpio_cfg_default
	.section	.text.nrf_gpio_cfg_watcher,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_cfg_watcher, %function
nrf_gpio_cfg_watcher:
.LFB186:
	.loc 3 559 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	.loc 3 560 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 3 562 32
	ldr	r2, [sp, #4]
	ldr	r3, [sp, #12]
	add	r2, r2, #448
	ldr	r3, [r3, r2, lsl #2]
	.loc 3 562 14
	bic	r3, r3, #2
	str	r3, [sp, #8]
	.loc 3 564 17
	ldr	r2, [sp, #4]
	.loc 3 564 30
	ldr	r3, [sp, #12]
	add	r2, r2, #448
	ldr	r1, [sp, #8]
	str	r1, [r3, r2, lsl #2]
	.loc 3 565 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE186:
	.size	nrf_gpio_cfg_watcher, .-nrf_gpio_cfg_watcher
	.section	.text.nrf_gpio_cfg_sense_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_cfg_sense_set, %function
nrf_gpio_cfg_sense_set:
.LFB189:
	.loc 3 593 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	mov	r3, r1
	strb	r3, [sp, #3]
	.loc 3 594 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 3 597 17
	ldr	r2, [sp, #4]
	ldr	r3, [sp, #12]
	add	r2, r2, #448
	ldr	r3, [r3, r2, lsl #2]
	ldr	r2, [sp, #4]
	.loc 3 597 30
	bic	r1, r3, #196608
	ldr	r3, [sp, #12]
	add	r2, r2, #448
	str	r1, [r3, r2, lsl #2]
	.loc 3 598 17
	ldr	r2, [sp, #4]
	ldr	r3, [sp, #12]
	add	r2, r2, #448
	ldr	r1, [r3, r2, lsl #2]
	.loc 3 598 47
	ldrb	r3, [sp, #3]	@ zero_extendqisi2
	lsls	r3, r3, #16
	.loc 3 598 17
	ldr	r2, [sp, #4]
	.loc 3 598 30
	orrs	r1, r1, r3
	ldr	r3, [sp, #12]
	add	r2, r2, #448
	str	r1, [r3, r2, lsl #2]
	.loc 3 599 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE189:
	.size	nrf_gpio_cfg_sense_set, .-nrf_gpio_cfg_sense_set
	.section	.text.nrf_gpio_pin_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_pin_set, %function
nrf_gpio_pin_set:
.LFB191:
	.loc 3 623 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	.loc 3 624 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 3 626 5
	ldr	r3, [sp, #4]
	movs	r2, #1
	lsl	r3, r2, r3
	mov	r1, r3
	ldr	r0, [sp, #12]
	bl	nrf_gpio_port_out_set
	.loc 3 627 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE191:
	.size	nrf_gpio_pin_set, .-nrf_gpio_pin_set
	.section	.text.nrf_gpio_pin_clear,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_pin_clear, %function
nrf_gpio_pin_clear:
.LFB192:
	.loc 3 631 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	.loc 3 632 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 3 634 5
	ldr	r3, [sp, #4]
	movs	r2, #1
	lsl	r3, r2, r3
	mov	r1, r3
	ldr	r0, [sp, #12]
	bl	nrf_gpio_port_out_clear
	.loc 3 635 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE192:
	.size	nrf_gpio_pin_clear, .-nrf_gpio_pin_clear
	.section	.text.nrf_gpio_pin_toggle,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_pin_toggle, %function
nrf_gpio_pin_toggle:
.LFB193:
	.loc 3 639 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	.loc 3 640 34
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 3 641 21
	ldr	r3, [sp, #12]
	ldr	r3, [r3, #1284]
	str	r3, [sp, #8]
	.loc 3 643 20
	ldr	r3, [sp, #8]
	mvns	r2, r3
	.loc 3 643 39
	ldr	r3, [sp, #4]
	movs	r1, #1
	lsl	r3, r1, r3
	.loc 3 643 32
	ands	r2, r2, r3
	.loc 3 643 17
	ldr	r3, [sp, #12]
	str	r2, [r3, #1288]
	.loc 3 644 38
	ldr	r3, [sp, #4]
	movs	r2, #1
	lsls	r2, r2, r3
	.loc 3 644 31
	ldr	r3, [sp, #8]
	ands	r2, r2, r3
	.loc 3 644 17
	ldr	r3, [sp, #12]
	str	r2, [r3, #1292]
	.loc 3 645 1
	nop
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE193:
	.size	nrf_gpio_pin_toggle, .-nrf_gpio_pin_toggle
	.section	.text.nrf_gpio_pin_read,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_pin_read, %function
nrf_gpio_pin_read:
.LFB195:
	.loc 3 662 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	.loc 3 663 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 3 665 14
	ldr	r0, [sp, #12]
	bl	nrf_gpio_port_in_read
	mov	r2, r0
	.loc 3 665 41 discriminator 1
	ldr	r3, [sp, #4]
	lsr	r3, r2, r3
	.loc 3 665 56 discriminator 1
	and	r3, r3, #1
	.loc 3 666 1
	mov	r0, r3
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE195:
	.size	nrf_gpio_pin_read, .-nrf_gpio_pin_read
	.section	.text.nrf_gpio_pin_sense_get,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_pin_sense_get, %function
nrf_gpio_pin_sense_get:
.LFB197:
	.loc 3 678 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #20
	.cfi_def_cfa_offset 24
	str	r0, [sp, #4]
	.loc 3 679 27
	add	r3, sp, #4
	mov	r0, r3
	bl	nrf_gpio_pin_port_decode
	str	r0, [sp, #12]
	.loc 3 681 48
	ldr	r2, [sp, #4]
	ldr	r3, [sp, #12]
	add	r2, r2, #448
	ldr	r3, [r3, r2, lsl #2]
	.loc 3 682 60
	lsrs	r3, r3, #16
	and	r3, r3, #3
	.loc 3 681 12
	uxtb	r3, r3
	.loc 3 683 1
	mov	r0, r3
	add	sp, sp, #20
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE197:
	.size	nrf_gpio_pin_sense_get, .-nrf_gpio_pin_sense_get
	.section	.text.nrf_gpio_port_in_read,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_port_in_read, %function
nrf_gpio_port_in_read:
.LFB202:
	.loc 3 711 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 3 712 17
	ldr	r3, [sp, #4]
	ldr	r3, [r3, #1296]
	.loc 3 713 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE202:
	.size	nrf_gpio_port_in_read, .-nrf_gpio_port_in_read
	.section	.text.nrf_gpio_port_out_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_port_out_set, %function
nrf_gpio_port_out_set:
.LFB205:
	.loc 3 729 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	str	r1, [sp]
	.loc 3 730 19
	ldr	r3, [sp, #4]
	ldr	r2, [sp]
	str	r2, [r3, #1288]
	.loc 3 731 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE205:
	.size	nrf_gpio_port_out_set, .-nrf_gpio_port_out_set
	.section	.text.nrf_gpio_port_out_clear,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_port_out_clear, %function
nrf_gpio_port_out_clear:
.LFB206:
	.loc 3 735 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	str	r1, [sp]
	.loc 3 736 19
	ldr	r3, [sp, #4]
	ldr	r2, [sp]
	str	r2, [r3, #1292]
	.loc 3 737 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE206:
	.size	nrf_gpio_port_out_clear, .-nrf_gpio_port_out_clear
	.section	.text.nrf_gpio_ports_read,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_gpio_ports_read, %function
nrf_gpio_ports_read:
.LFB207:
	.loc 3 741 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #28
	.cfi_def_cfa_offset 32
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	str	r2, [sp, #4]
	.loc 3 742 21
	mov	r3, #1342177280
	str	r3, [sp, #16]
	.loc 3 747 12
	ldr	r3, [sp, #12]
	str	r3, [sp, #20]
	.loc 3 747 5
	b	.L71
.L72:
	.loc 3 749 51
	ldr	r3, [sp, #20]
	lsls	r3, r3, #2
	adds	r3, r3, #24
	add	r3, sp, r3
	ldr	r3, [r3, #-8]
	.loc 3 749 20
	mov	r0, r3
	bl	nrf_gpio_port_in_read
	mov	r2, r0
	.loc 3 749 18 discriminator 1
	ldr	r3, [sp, #4]
	str	r2, [r3]
	.loc 3 750 16
	ldr	r3, [sp, #4]
	adds	r3, r3, #4
	str	r3, [sp, #4]
	.loc 3 747 54 discriminator 3
	ldr	r3, [sp, #20]
	adds	r3, r3, #1
	str	r3, [sp, #20]
.L71:
	.loc 3 747 42 discriminator 1
	ldr	r2, [sp, #12]
	ldr	r3, [sp, #8]
	add	r3, r3, r2
	.loc 3 747 28 discriminator 1
	ldr	r2, [sp, #20]
	cmp	r2, r3
	bcc	.L72
	.loc 3 752 1
	nop
	nop
	add	sp, sp, #28
	.cfi_def_cfa_offset 4
	@ sp needed
	ldr	pc, [sp], #4
	.cfi_endproc
.LFE207:
	.size	nrf_gpio_ports_read, .-nrf_gpio_ports_read
	.section	.text.nrf_bitmask_bit_is_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_bitmask_bit_is_set, %function
nrf_bitmask_bit_is_set:
.LFB218:
	.file 4 "../../../nRF5_SDK_14.2.0/components/libraries/util/nrf_bitmask.h"
	.loc 4 62 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	r0, [sp, #4]
	str	r1, [sp]
	.loc 4 63 21
	ldr	r3, [sp]
	str	r3, [sp, #12]
	.loc 4 64 14
	ldr	r3, [sp, #4]
	lsrs	r3, r3, #3
	str	r3, [sp, #8]
	.loc 4 65 9
	ldr	r3, [sp, #4]
	and	r3, r3, #7
	str	r3, [sp, #4]
	.loc 4 66 15
	movs	r2, #1
	ldr	r3, [sp, #4]
	lsl	r3, r2, r3
	.loc 4 66 32
	ldr	r1, [sp, #12]
	ldr	r2, [sp, #8]
	add	r2, r2, r1
	ldrb	r2, [r2]	@ zero_extendqisi2
	.loc 4 66 23
	ands	r3, r3, r2
	.loc 4 67 1
	mov	r0, r3
	add	sp, sp, #16
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE218:
	.size	nrf_bitmask_bit_is_set, .-nrf_bitmask_bit_is_set
	.section	.text.nrf_bitmask_bit_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	nrf_bitmask_bit_set, %function
nrf_bitmask_bit_set:
.LFB219:
	.loc 4 76 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	r0, [sp, #4]
	str	r1, [sp]
	.loc 4 77 15
	ldr	r3, [sp]
	str	r3, [sp, #12]
	.loc 4 78 14
	ldr	r3, [sp, #4]
	lsrs	r3, r3, #3
	str	r3, [sp, #8]
	.loc 4 79 9
	ldr	r3, [sp, #4]
	and	r3, r3, #7
	str	r3, [sp, #4]
	.loc 4 80 12
	ldr	r2, [sp, #12]
	ldr	r3, [sp, #8]
	add	r3, r3, r2
	ldrb	r3, [r3]	@ zero_extendqisi2
	sxtb	r2, r3
	.loc 4 80 29
	movs	r1, #1
	ldr	r3, [sp, #4]
	lsl	r3, r1, r3
	.loc 4 80 23
	sxtb	r3, r3
	orrs	r3, r3, r2
	sxtb	r1, r3
	.loc 4 80 12
	ldr	r2, [sp, #12]
	ldr	r3, [sp, #8]
	add	r3, r3, r2
	.loc 4 80 23
	uxtb	r2, r1
	strb	r2, [r3]
	.loc 4 81 1
	nop
	add	sp, sp, #16
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE219:
	.size	nrf_bitmask_bit_set, .-nrf_bitmask_bit_set
	.section	.bss.m_cb,"aw",%nobits
	.align	2
	.type	m_cb, %object
	.size	m_cb, 88
m_cb:
	.space	88
	.section	.text.pin_in_use,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	pin_in_use, %function
pin_in_use:
.LFB223:
	.loc 1 121 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 1 122 33
	ldr	r2, .L78
	ldr	r3, [sp, #4]
	add	r3, r3, r2
	adds	r3, r3, #48
	ldrsb	r3, [r3]
	.loc 1 122 39
	cmp	r3, #-1
	ite	ne
	movne	r3, #1
	moveq	r3, #0
	uxtb	r3, r3
	.loc 1 123 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L79:
	.align	2
.L78:
	.word	m_cb
	.cfi_endproc
.LFE223:
	.size	pin_in_use, .-pin_in_use
	.section	.text.pin_in_use_as_non_task_out,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	pin_in_use_as_non_task_out, %function
pin_in_use_as_non_task_out:
.LFB224:
	.loc 1 127 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 1 128 33
	ldr	r2, .L82
	ldr	r3, [sp, #4]
	add	r3, r3, r2
	adds	r3, r3, #48
	ldrsb	r3, [r3]
	.loc 1 128 39
	cmn	r3, #2
	ite	eq
	moveq	r3, #1
	movne	r3, #0
	uxtb	r3, r3
	.loc 1 129 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L83:
	.align	2
.L82:
	.word	m_cb
	.cfi_endproc
.LFE224:
	.size	pin_in_use_as_non_task_out, .-pin_in_use_as_non_task_out
	.section	.text.pin_in_use_by_te,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	pin_in_use_by_te, %function
pin_in_use_by_te:
.LFB225:
	.loc 1 133 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 1 134 33
	ldr	r2, .L88
	ldr	r3, [sp, #4]
	add	r3, r3, r2
	adds	r3, r3, #48
	ldrsb	r3, [r3]
	.loc 1 135 35
	cmp	r3, #0
	blt	.L85
	.loc 1 134 67
	ldr	r2, .L88
	ldr	r3, [sp, #4]
	add	r3, r3, r2
	adds	r3, r3, #48
	ldrsb	r3, [r3]
	.loc 1 135 35 discriminator 1
	cmp	r3, #7
	bgt	.L85
	.loc 1 135 35 is_stmt 0 discriminator 3
	movs	r3, #1
	.loc 1 135 35
	b	.L86
.L85:
	.loc 1 135 35 discriminator 4
	movs	r3, #0
.L86:
	.loc 1 135 35 discriminator 6
	and	r3, r3, #1
	uxtb	r3, r3
	.loc 1 136 1 is_stmt 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L89:
	.align	2
.L88:
	.word	m_cb
	.cfi_endproc
.LFE225:
	.size	pin_in_use_by_te, .-pin_in_use_by_te
	.section	.text.pin_in_use_by_port,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	pin_in_use_by_port, %function
pin_in_use_by_port:
.LFB226:
	.loc 1 140 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 1 141 33
	ldr	r2, .L92
	ldr	r3, [sp, #4]
	add	r3, r3, r2
	adds	r3, r3, #48
	ldrsb	r3, [r3]
	.loc 1 141 39
	cmp	r3, #7
	ite	gt
	movgt	r3, #1
	movle	r3, #0
	uxtb	r3, r3
	.loc 1 142 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L93:
	.align	2
.L92:
	.word	m_cb
	.cfi_endproc
.LFE226:
	.size	pin_in_use_by_port, .-pin_in_use_by_port
	.section	.text.pin_in_use_by_gpiote,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	pin_in_use_by_gpiote, %function
pin_in_use_by_gpiote:
.LFB227:
	.loc 1 146 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 1 147 33
	ldr	r2, .L96
	ldr	r3, [sp, #4]
	add	r3, r3, r2
	adds	r3, r3, #48
	ldrsb	r3, [r3]
	.loc 1 147 39
	mvns	r3, r3
	uxtb	r3, r3
	lsrs	r3, r3, #7
	uxtb	r3, r3
	.loc 1 148 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L97:
	.align	2
.L96:
	.word	m_cb
	.cfi_endproc
.LFE227:
	.size	pin_in_use_by_gpiote, .-pin_in_use_by_gpiote
	.section	.text.pin_in_use_by_te_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	pin_in_use_by_te_set, %function
pin_in_use_by_te_set:
.LFB228:
	.loc 1 155 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	str	r2, [sp, #4]
	strb	r3, [sp, #3]
	.loc 1 156 31
	ldr	r3, [sp, #8]
	sxtb	r1, r3
	ldr	r2, .L101
	ldr	r3, [sp, #12]
	add	r3, r3, r2
	adds	r3, r3, #48
	mov	r2, r1
	strb	r2, [r3]
	.loc 1 157 31
	ldr	r1, .L101
	ldr	r3, [sp, #8]
	ldr	r2, [sp, #4]
	str	r2, [r1, r3, lsl #2]
	.loc 1 158 9
	ldrb	r3, [sp, #3]
	eor	r3, r3, #1
	uxtb	r3, r3
	.loc 1 158 8
	cmp	r3, #0
	beq	.L100
	.loc 1 160 44
	ldr	r3, [sp, #8]
	subs	r3, r3, #8
	.loc 1 160 63
	ldr	r2, [sp, #12]
	sxtb	r1, r2
	.loc 1 160 61
	ldr	r2, .L101
	add	r3, r3, r2
	mov	r2, r1
	strb	r2, [r3, #80]
.L100:
	.loc 1 162 1
	nop
	add	sp, sp, #16
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L102:
	.align	2
.L101:
	.word	m_cb
	.cfi_endproc
.LFE228:
	.size	pin_in_use_by_te_set, .-pin_in_use_by_te_set
	.section	.text.pin_in_use_set,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	pin_in_use_set, %function
pin_in_use_set:
.LFB229:
	.loc 1 166 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 1 167 31
	ldr	r2, .L104
	ldr	r3, [sp, #4]
	add	r3, r3, r2
	adds	r3, r3, #48
	movs	r2, #254
	strb	r2, [r3]
	.loc 1 168 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L105:
	.align	2
.L104:
	.word	m_cb
	.cfi_endproc
.LFE229:
	.size	pin_in_use_set, .-pin_in_use_set
	.section	.text.pin_in_use_clear,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	pin_in_use_clear, %function
pin_in_use_clear:
.LFB230:
	.loc 1 172 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 1 173 31
	ldr	r2, .L107
	ldr	r3, [sp, #4]
	add	r3, r3, r2
	adds	r3, r3, #48
	movs	r2, #255
	strb	r2, [r3]
	.loc 1 174 1
	nop
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L108:
	.align	2
.L107:
	.word	m_cb
	.cfi_endproc
.LFE230:
	.size	pin_in_use_clear, .-pin_in_use_clear
	.section	.text.channel_port_get,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	channel_port_get, %function
channel_port_get:
.LFB231:
	.loc 1 178 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 1 179 32
	ldr	r2, .L111
	ldr	r3, [sp, #4]
	add	r3, r3, r2
	adds	r3, r3, #48
	ldrsb	r3, [r3]
	.loc 1 180 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L112:
	.align	2
.L111:
	.word	m_cb
	.cfi_endproc
.LFE231:
	.size	channel_port_get, .-channel_port_get
	.section	.text.channel_handler_get,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	channel_handler_get, %function
channel_handler_get:
.LFB232:
	.loc 1 184 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	str	r0, [sp, #4]
	.loc 1 185 25
	ldr	r2, .L115
	ldr	r3, [sp, #4]
	ldr	r3, [r2, r3, lsl #2]
	.loc 1 186 1
	mov	r0, r3
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
.L116:
	.align	2
.L115:
	.word	m_cb
	.cfi_endproc
.LFE232:
	.size	channel_handler_get, .-channel_handler_get
	.section	.text.channel_port_alloc,"ax",%progbits
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	channel_port_alloc, %function
channel_port_alloc:
.LFB233:
	.loc 1 190 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{lr}
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -4
	sub	sp, sp, #36
	.cfi_def_cfa_offset 40
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	mov	r3, r2
	strb	r3, [sp, #7]
	.loc 1 191 14
	movs	r3, #255
	strb	r3, [sp, #31]
	.loc 1 194 38
	ldrb	r3, [sp, #7]	@ zero_extendqisi2
	cmp	r3, #0
	beq	.L118
	.loc 1 194 38 is_stmt 0 discriminator 1
	movs	r3, #0
	b	.L119
.L118:
	.loc 1 194 38 discriminator 2
	movs	r3, #8
.L119:
	.loc 1 194 14 is_stmt 1 discriminator 4
	str	r3, [sp, #20]
	.loc 1 196 33
	ldrb	r3, [sp, #7]	@ zero_extendqisi2
	cmp	r3, #0
	beq	.L120
	.loc 1 196 33 is_stmt 0 discriminator 1
	movs	r3, #8
	b	.L121
.L120:
	.loc 1 196 33 discriminator 2
	movs	r3, #12
.L121:
	.loc 1 195 14 is_stmt 1
	str	r3, [sp, #16]
	.loc 1 200 12
	ldr	r3, [sp, #20]
	str	r3, [sp, #24]
	.loc 1 200 5
	b	.L122
.L125:
	.loc 1 202 26
	ldr	r2, .L127
	ldr	r3, [sp, #24]
	ldr	r3, [r2, r3, lsl #2]
	.loc 1 202 12
	cmp	r3, #-1
	bne	.L123
	.loc 1 204 13
	ldrb	r3, [sp, #7]	@ zero_extendqisi2
	ldr	r2, [sp, #8]
	ldr	r1, [sp, #24]
	ldr	r0, [sp, #12]
	bl	pin_in_use_by_te_set
	.loc 1 205 24
	ldr	r3, [sp, #24]
	strb	r3, [sp, #31]
	.loc 1 206 13
	b	.L124
.L123:
	.loc 1 200 39 discriminator 2
	ldr	r3, [sp, #24]
	adds	r3, r3, #1
	str	r3, [sp, #24]
.L122:
	.loc 1 200 27 discriminator 1
	ldr	r2, [sp, #24]
	ldr	r3, [sp, #16]
	cmp	r2, r3
	bcc	.L125
.L124:
	