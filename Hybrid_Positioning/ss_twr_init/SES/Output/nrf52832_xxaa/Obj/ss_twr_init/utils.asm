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
	.file	"utils.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.file 1 "D:\\TN\\Hybrid_Positioning\\Hybrid_Positioning\\ss_twr_init\\utils.c"
	.section	.text.tof_3d_localize,"ax",%progbits
	.align	1
	.global	tof_3d_localize
	.syntax unified
	.thumb
	.thumb_func
	.type	tof_3d_localize, %function
tof_3d_localize:
.LFB13:
	.loc 1 13 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 488
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.cfi_def_cfa_offset 36
	.cfi_offset 4, -36
	.cfi_offset 5, -32
	.cfi_offset 6, -28
	.cfi_offset 7, -24
	.cfi_offset 8, -20
	.cfi_offset 9, -16
	.cfi_offset 10, -12
	.cfi_offset 11, -8
	.cfi_offset 14, -4
	sub	sp, sp, #492
	.cfi_def_cfa_offset 528
	add	r4, sp, #488
	sub	r4, r4, #412
	str	r0, [r4]
	add	r0, sp, #488
	sub	r0, r0, #416
	str	r1, [r0]
	add	r1, sp, #488
	sub	r1, r1, #420
	str	r2, [r1]
	add	r2, sp, #488
	sub	r2, r2, #424
	str	r3, [r2]
	.loc 1 15 8
	add	r3, sp, #488
	sub	r3, r3, #416
	ldr	r3, [r3]
	cmp	r3, #3
	ble	.L2
	.loc 1 15 25 discriminator 1
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	cmp	r3, #0
	beq	.L2
	.loc 1 15 33 discriminator 2
	add	r3, sp, #488
	sub	r3, r3, #420
	ldr	r3, [r3]
	cmp	r3, #0
	beq	.L2
	.loc 1 15 47 discriminator 3
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	cmp	r3, #0
	bne	.L3
.L2:
	.loc 1 16 16
	mov	r3, #-1
	b	.L4
.L3:
	.loc 1 19 9
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	vldr.64	d7, [r3]
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	__SEGGER_RTL_float64_isnan
	mov	r3, r0
	.loc 1 19 8 discriminator 1
	cmp	r3, #0
	bne	.L5
	.loc 1 19 30 discriminator 1
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	vldr.64	d7, [r3, #8]
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	__SEGGER_RTL_float64_isnan
	mov	r3, r0
	.loc 1 19 27 discriminator 1
	cmp	r3, #0
	bne	.L5
	.loc 1 19 51 discriminator 2
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	vldr.64	d7, [r3, #16]
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	__SEGGER_RTL_float64_isnan
	mov	r3, r0
	.loc 1 19 48 discriminator 1
	cmp	r3, #0
	bne	.L5
	.loc 1 20 9
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	vldr.64	d7, [r3]
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	__SEGGER_RTL_float64_isinf
	mov	r3, r0
	.loc 1 19 69 discriminator 3
	cmp	r3, #0
	bne	.L5
	.loc 1 20 30
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	vldr.64	d7, [r3, #8]
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	__SEGGER_RTL_float64_isinf
	mov	r3, r0
	.loc 1 20 27 discriminator 1
	cmp	r3, #0
	bne	.L5
	.loc 1 20 51 discriminator 1
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	vldr.64	d7, [r3, #16]
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	__SEGGER_RTL_float64_isinf
	mov	r3, r0
	.loc 1 20 48 discriminator 1
	cmp	r3, #0
	bne	.L5
	.loc 1 21 21
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	ldrd	r2, [r3]
	.loc 1 21 9
	mov	r6, r2
	bic	r7, r3, #-2147483648
	.loc 1 20 69 discriminator 2
	mov	r2, #0
	ldr	r3, .L64+8
	mov	r0, r6
	mov	r1, r7
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	bne	.L5
	.loc 1 21 51
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	ldrd	r2, [r3, #8]
	.loc 1 21 39
	mov	r8, r2
	bic	r9, r3, #-2147483648
	.loc 1 21 36
	mov	r2, #0
	ldr	r3, .L64+8
	mov	r0, r8
	mov	r1, r9
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	bne	.L5
	.loc 1 21 81 discriminator 1
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r3, [r3]
	ldrd	r2, [r3, #16]
	.loc 1 21 69 discriminator 1
	mov	r10, r2
	bic	fp, r3, #-2147483648
	.loc 1 21 66 discriminator 1
	mov	r2, #0
	ldr	r3, .L64+8
	mov	r0, r10
	mov	r1, fp
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	beq	.L6
.L5:
	.loc 1 23 20
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r1, [r3]
	mov	r2, #0
	mov	r3, #0
	strd	r2, [r1]
	.loc 1 24 20
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r1, [r3]
	mov	r2, #0
	mov	r3, #0
	strd	r2, [r1, #8]
	.loc 1 25 20
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r1, [r3]
	mov	r2, #0
	mov	r3, #0
	strd	r2, [r1, #16]
.L6:
	.loc 1 31 9
	movs	r3, #0
	str	r3, [sp, #460]
.LBB2:
	.loc 1 32 14
	movs	r3, #1
	str	r3, [sp, #456]
	.loc 1 32 5
	b	.L8
.L11:
	.loc 1 33 22
	ldr	r3, [sp, #456]
	lsls	r2, r3, #3
	add	r3, sp, #488
	sub	r3, r3, #420
	ldr	r3, [r3]
	add	r3, r3, r2
	ldrd	r0, [r3]
	.loc 1 33 37
	ldr	r3, [sp, #460]
	lsls	r2, r3, #3
	add	r3, sp, #488
	sub	r3, r3, #420
	ldr	r3, [r3]
	add	r3, r3, r2
	ldrd	r2, [r3]
	.loc 1 33 12
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L9
	.loc 1 34 19
	ldr	r3, [sp, #456]
	str	r3, [sp, #460]
.L9:
	.loc 1 32 39 discriminator 2
	ldr	r3, [sp, #456]
	adds	r3, r3, #1
	str	r3, [sp, #456]
.L8:
	.loc 1 32 23 discriminator 1
	add	r3, sp, #488
	sub	r3, r3, #416
	ldr	r2, [sp, #456]
	ldr	r3, [r3]
	cmp	r2, r3
	blt	.L11
.LBE2:
	.loc 1 36 17
	ldr	r2, [sp, #460]
	mov	r3, r2
	lsls	r3, r3, #1
	add	r3, r3, r2
	lsls	r3, r3, #3
	mov	r2, r3
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	add	r3, r3, r2
	.loc 1 36 12
	ldrd	r2, [r3]
	strd	r2, [sp, #480]
	.loc 1 37 17
	ldr	r2, [sp, #460]
	mov	r3, r2
	lsls	r3, r3, #1
	add	r3, r3, r2
	lsls	r3, r3, #3
	mov	r2, r3
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	add	r3, r3, r2
	.loc 1 37 12
	ldrd	r2, [r3, #8]
	strd	r2, [sp, #472]
	.loc 1 38 17
	ldr	r2, [sp, #460]
	mov	r3, r2
	lsls	r3, r3, #1
	add	r3, r3, r2
	lsls	r3, r3, #3
	mov	r2, r3
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	add	r3, r3, r2
	.loc 1 38 12
	ldrd	r2, [r3, #16]
	strd	r2, [sp, #464]
	.loc 1 40 12
	adr	r3, .L64
	ldrd	r2, [r3]
	strd	r2, [sp, #448]
	.loc 1 41 15
	movs	r3, #50
	str	r3, [sp, #376]
.LBB3:
	.loc 1 43 14
	movs	r3, #0
	str	r3, [sp, #444]
	.loc 1 43 5
	b	.L12
.L47:
.LBB4:
	.loc 1 45 16
	add	r3, sp, #488
	sub	r3, r3, #408
	mov	r0, r3
	movs	r3, #72
	mov	r2, r3
	movs	r1, #0
	bl	memset
	.loc 1 46 16
	add	r3, sp, #488
	sub	r3, r3, #264
	mov	r2, r3
	movs	r3, #0
	str	r3, [r2]
	str	r3, [r2, #4]
	str	r3, [r2, #8]
	str	r3, [r2, #12]
	str	r3, [r2, #16]
	str	r3, [r2, #20]
	.loc 1 47 16
	mov	r2, #0
	mov	r3, #0
	strd	r2, [sp, #432]
.LBB5:
	.loc 1 50 18
	movs	r3, #0
	str	r3, [sp, #428]
	.loc 1 50 9
	b	.L13
.L65:
	.align	3
.L64:
	.word	-1717986918
	.word	1069128089
	.word	1079574528
.L16:
.LBB6:
	.loc 1 52 37
	ldr	r2, [sp, #428]
	mov	r3, r2
	lsls	r3, r3, #1
	add	r3, r3, r2
	lsls	r3, r3, #3
	mov	r2, r3
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	add	r3, r3, r2
	.loc 1 52 40
	ldrd	r2, [r3]
	.loc 1 52 20
	ldrd	r0, [sp, #480]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #296]
	.loc 1 53 37
	ldr	r2, [sp, #428]
	mov	r3, r2
	lsls	r3, r3, #1
	add	r3, r3, r2
	lsls	r3, r3, #3
	mov	r2, r3
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	add	r3, r3, r2
	.loc 1 53 40
	ldrd	r2, [r3, #8]
	.loc 1 53 20
	ldrd	r0, [sp, #472]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #288]
	.loc 1 54 37
	ldr	r2, [sp, #428]
	mov	r3, r2
	lsls	r3, r3, #1
	add	r3, r3, r2
	lsls	r3, r3, #3
	mov	r2, r3
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	add	r3, r3, r2
	.loc 1 54 40
	ldrd	r2, [r3, #16]
	.loc 1 54 20
	ldrd	r0, [sp, #464]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #280]
	.loc 1 56 31
	ldrd	r2, [sp, #296]
	ldrd	r0, [sp, #296]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 56 39
	ldrd	r2, [sp, #288]
	ldrd	r0, [sp, #288]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 56 35
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 56 47
	ldrd	r2, [sp, #280]
	ldrd	r0, [sp, #280]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 56 24
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	vmov	d7, r2, r3
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	sqrt
	vstr.64	d0, [sp, #416]
	.loc 1 57 16
	adr	r3, .L66+4
	ldrd	r2, [r3]
	ldrd	r0, [sp, #416]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L14
	.loc 1 57 29 discriminator 1
	adr	r3, .L66+4
	ldrd	r2, [r3]
	strd	r2, [sp, #416]
.L14:
	.loc 1 59 39
	ldr	r3, [sp, #428]
	lsls	r2, r3, #3
	add	r3, sp, #488
	sub	r3, r3, #420
	ldr	r3, [r3]
	add	r3, r3, r2
	ldrd	r2, [r3]
	.loc 1 59 20
	ldrd	r0, [sp, #416]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #272]
	.loc 1 60 30
	ldrd	r2, [sp, #272]
	ldrd	r0, [sp, #272]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 60 23
	ldrd	r0, [sp, #432]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #432]
	.loc 1 62 20
	ldrd	r2, [sp, #416]
	ldrd	r0, [sp, #296]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #264]
	.loc 1 63 20
	ldrd	r2, [sp, #416]
	ldrd	r0, [sp, #288]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #256]
	.loc 1 64 20
	ldrd	r2, [sp, #416]
	ldrd	r0, [sp, #280]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #248]
	.loc 1 66 19
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r4, [r3]
	.loc 1 66 29
	ldrd	r2, [sp, #264]
	ldrd	r0, [sp, #264]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 66 23
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3]
	.loc 1 66 41
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r4, [r3, #8]
	.loc 1 66 51
	ldrd	r2, [sp, #256]
	ldrd	r0, [sp, #264]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 66 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #8]
	.loc 1 66 63
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r4, [r3, #16]
	.loc 1 66 73
	ldrd	r2, [sp, #248]
	ldrd	r0, [sp, #264]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 66 67
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #16]
	.loc 1 67 19
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r4, [r3, #24]
	.loc 1 67 29
	ldrd	r2, [sp, #264]
	ldrd	r0, [sp, #256]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 67 23
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #24]
	.loc 1 67 41
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r4, [r3, #32]
	.loc 1 67 51
	ldrd	r2, [sp, #256]
	ldrd	r0, [sp, #256]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 67 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #32]
	.loc 1 67 63
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r4, [r3, #40]
	.loc 1 67 73
	ldrd	r2, [sp, #248]
	ldrd	r0, [sp, #256]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 67 67
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #40]
	.loc 1 68 19
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r4, [r3, #48]
	.loc 1 68 29
	ldrd	r2, [sp, #264]
	ldrd	r0, [sp, #248]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 68 23
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #48]
	.loc 1 68 41
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r4, [r3, #56]
	.loc 1 68 51
	ldrd	r2, [sp, #256]
	ldrd	r0, [sp, #248]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 68 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #56]
	.loc 1 68 63
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r4, [r3, #64]
	.loc 1 68 73
	ldrd	r2, [sp, #248]
	ldrd	r0, [sp, #248]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 68 67
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #64]
	.loc 1 70 16
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r4, [r3]
	.loc 1 70 26
	ldrd	r2, [sp, #272]
	ldrd	r0, [sp, #264]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 70 20
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #264
	strd	r0, [r3]
	.loc 1 71 16
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r4, [r3, #8]
	.loc 1 71 26
	ldrd	r2, [sp, #272]
	ldrd	r0, [sp, #256]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 71 20
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #264
	strd	r0, [r3, #8]
	.loc 1 72 16
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r4, [r3, #16]
	.loc 1 72 26
	ldrd	r2, [sp, #272]
	ldrd	r0, [sp, #248]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 72 20
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #264
	strd	r0, [r3, #16]
.LBE6:
	.loc 1 50 43 discriminator 2
	ldr	r3, [sp, #428]
	adds	r3, r3, #1
	str	r3, [sp, #428]
.L13:
	.loc 1 50 27 discriminator 1
	add	r3, sp, #488
	sub	r3, r3, #416
	ldr	r2, [sp, #428]
	ldr	r3, [r3]
	cmp	r2, r3
	blt	.L16
.LBE5:
	.loc 1 76 15
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3]
	.loc 1 76 19
	ldrd	r2, [sp, #448]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3]
	.loc 1 77 15
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #32]
	.loc 1 77 19
	ldrd	r2, [sp, #448]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #32]
	.loc 1 78 15
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #64]
	.loc 1 78 19
	ldrd	r2, [sp, #448]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	add	r3, sp, #488
	sub	r3, r3, #408
	strd	r0, [r3, #64]
	.loc 1 81 28
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r6, [r3]
	.loc 1 81 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #32]
	.loc 1 81 49
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #64]
	.loc 1 81 42
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 81 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #40]
	.loc 1 81 71
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #56]
	.loc 1 81 64
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 81 53
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	.loc 1 81 31
	mov	r0, r6
	mov	r1, r7
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r8, r2
	mov	r9, r3
	.loc 1 82 28
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r6, [r3, #8]
	.loc 1 82 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #24]
	.loc 1 82 49
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #64]
	.loc 1 82 42
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 82 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #40]
	.loc 1 82 71
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #48]
	.loc 1 82 64
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 82 53
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	.loc 1 82 31
	mov	r0, r6
	mov	r1, r7
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 82 20
	mov	r0, r8
	mov	r1, r9
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r8, r2
	mov	r9, r3
	.loc 1 83 28
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r6, [r3, #16]
	.loc 1 83 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #24]
	.loc 1 83 49
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #56]
	.loc 1 83 42
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 83 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #32]
	.loc 1 83 71
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #48]
	.loc 1 83 64
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 83 53
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	.loc 1 83 31
	mov	r0, r6
	mov	r1, r7
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 81 16
	mov	r0, r8
	mov	r1, r9
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #368]
	.loc 1 85 13
	ldr	r3, [sp, #368]
	str	r3, [sp]
	ldr	r3, [sp, #372]
	bic	r3, r3, #-2147483648
	str	r3, [sp, #4]
	.loc 1 85 12
	adr	r3, .L66+4
	ldrd	r2, [r3]
	ldrd	r0, [sp]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L59
	.loc 1 86 20
	mov	r2, #0
	ldr	r3, .L66
	ldrd	r0, [sp, #448]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #448]
	.loc 1 87 13
	b	.L45
.L67:
	.align	3
.L66:
	.word	1075052544
	.word	-1598689907
	.word	1051772663
.L59:
	.loc 1 92 29
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #32]
	.loc 1 92 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #64]
	.loc 1 92 32
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 92 51
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #40]
	.loc 1 92 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #56]
	.loc 1 92 54
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 92 43
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 92 66
	ldrd	r2, [sp, #368]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 92 19
	add	r3, sp, #488
	sub	r3, r3, #336
	strd	r0, [r3]
	.loc 1 93 29
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #8]
	.loc 1 93 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #64]
	.loc 1 93 32
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 93 51
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #16]
	.loc 1 93 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #56]
	.loc 1 93 54
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 93 43
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	.loc 1 93 21
	str	r2, [sp, #8]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #12]
	.loc 1 93 66
	ldrd	r2, [sp, #368]
	ldrd	r0, [sp, #8]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 93 19
	add	r3, sp, #488
	sub	r3, r3, #336
	strd	r0, [r3, #8]
	.loc 1 94 29
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #8]
	.loc 1 94 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #40]
	.loc 1 94 32
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 94 51
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #16]
	.loc 1 94 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #32]
	.loc 1 94 54
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 94 43
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 94 66
	ldrd	r2, [sp, #368]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 94 19
	add	r3, sp, #488
	sub	r3, r3, #336
	strd	r0, [r3, #16]
	.loc 1 96 29
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #24]
	.loc 1 96 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #64]
	.loc 1 96 32
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 96 51
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #40]
	.loc 1 96 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #48]
	.loc 1 96 54
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 96 43
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	.loc 1 96 21
	str	r2, [sp, #16]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #20]
	.loc 1 96 66
	ldrd	r2, [sp, #368]
	ldrd	r0, [sp, #16]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 96 19
	add	r3, sp, #488
	sub	r3, r3, #336
	strd	r0, [r3, #24]
	.loc 1 97 29
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3]
	.loc 1 97 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #64]
	.loc 1 97 32
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 97 51
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #16]
	.loc 1 97 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #48]
	.loc 1 97 54
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 97 43
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 97 66
	ldrd	r2, [sp, #368]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 97 19
	add	r3, sp, #488
	sub	r3, r3, #336
	strd	r0, [r3, #32]
	.loc 1 98 29
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3]
	.loc 1 98 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #40]
	.loc 1 98 32
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 98 51
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #16]
	.loc 1 98 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #24]
	.loc 1 98 54
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 98 43
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	.loc 1 98 21
	str	r2, [sp, #24]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #28]
	.loc 1 98 66
	ldrd	r2, [sp, #368]
	ldrd	r0, [sp, #24]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 98 19
	add	r3, sp, #488
	sub	r3, r3, #336
	strd	r0, [r3, #40]
	.loc 1 100 29
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #24]
	.loc 1 100 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #56]
	.loc 1 100 32
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 100 51
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #32]
	.loc 1 100 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #48]
	.loc 1 100 54
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 100 43
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 100 66
	ldrd	r2, [sp, #368]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 100 19
	add	r3, sp, #488
	sub	r3, r3, #336
	strd	r0, [r3, #48]
	.loc 1 101 29
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3]
	.loc 1 101 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #56]
	.loc 1 101 32
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 101 51
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #8]
	.loc 1 101 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #48]
	.loc 1 101 54
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 101 43
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	.loc 1 101 21
	str	r2, [sp, #32]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #36]
	.loc 1 101 66
	ldrd	r2, [sp, #368]
	ldrd	r0, [sp, #32]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 101 19
	add	r3, sp, #488
	sub	r3, r3, #336
	strd	r0, [r3, #56]
	.loc 1 102 29
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3]
	.loc 1 102 39
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #32]
	.loc 1 102 32
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 102 51
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r0, [r3, #8]
	.loc 1 102 61
	add	r3, sp, #488
	sub	r3, r3, #408
	ldrd	r2, [r3, #24]
	.loc 1 102 54
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 102 43
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 102 66
	ldrd	r2, [sp, #368]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 102 19
	add	r3, sp, #488
	sub	r3, r3, #336
	strd	r0, [r3, #64]
	.loc 1 104 34
	add	r3, sp, #488
	sub	r3, r3, #336
	ldrd	r0, [r3]
	.loc 1 104 41
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r2, [r3]
	.loc 1 104 37
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 104 53
	add	r3, sp, #488
	sub	r3, r3, #336
	ldrd	r0, [r3, #8]
	.loc 1 104 60
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r2, [r3, #8]
	.loc 1 104 56
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 104 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 104 72
	add	r3, sp, #488
	sub	r3, r3, #336
	ldrd	r0, [r3, #16]
	.loc 1 104 79
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r2, [r3, #16]
	.loc 1 104 75
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 104 64
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 104 16
	mov	r1, r2
	str	r1, [sp, #408]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #412]
	.loc 1 105 34
	add	r3, sp, #488
	sub	r3, r3, #336
	ldrd	r0, [r3, #24]
	.loc 1 105 41
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r2, [r3]
	.loc 1 105 37
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 105 53
	add	r3, sp, #488
	sub	r3, r3, #336
	ldrd	r0, [r3, #32]
	.loc 1 105 60
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r2, [r3, #8]
	.loc 1 105 56
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 105 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 105 72
	add	r3, sp, #488
	sub	r3, r3, #336
	ldrd	r0, [r3, #40]
	.loc 1 105 79
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r2, [r3, #16]
	.loc 1 105 75
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 105 64
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 105 16
	mov	r1, r2
	str	r1, [sp, #400]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #404]
	.loc 1 106 34
	add	r3, sp, #488
	sub	r3, r3, #336
	ldrd	r0, [r3, #48]
	.loc 1 106 41
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r2, [r3]
	.loc 1 106 37
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 106 53
	add	r3, sp, #488
	sub	r3, r3, #336
	ldrd	r0, [r3, #56]
	.loc 1 106 60
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r2, [r3, #8]
	.loc 1 106 56
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 106 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 106 72
	add	r3, sp, #488
	sub	r3, r3, #336
	ldrd	r0, [r3, #64]
	.loc 1 106 79
	add	r3, sp, #488
	sub	r3, r3, #264
	ldrd	r2, [r3, #16]
	.loc 1 106 75
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 106 64
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 106 16
	mov	r1, r2
	str	r1, [sp, #392]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #396]
	.loc 1 109 12
	adr	r3, .L68+32
	ldrd	r2, [r3]
	ldrd	r0, [sp, #408]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	beq	.L20
	.loc 1 109 43 discriminator 1
	adr	r3, .L68+32
	ldrd	r2, [r3]
	strd	r2, [sp, #408]
.L20:
	.loc 1 110 12
	adr	r3, .L68+40
	ldrd	r2, [r3]
	ldrd	r0, [sp, #408]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L22
	.loc 1 110 44 discriminator 1
	adr	r3, .L68+40
	ldrd	r2, [r3]
	strd	r2, [sp, #408]
.L22:
	.loc 1 111 12
	adr	r3, .L68+32
	ldrd	r2, [r3]
	ldrd	r0, [sp, #400]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	beq	.L24
	.loc 1 111 43 discriminator 1
	adr	r3, .L68+32
	ldrd	r2, [r3]
	strd	r2, [sp, #400]
.L24:
	.loc 1 112 12
	adr	r3, .L68+40
	ldrd	r2, [r3]
	ldrd	r0, [sp, #400]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L26
	.loc 1 112 44 discriminator 1
	adr	r3, .L68+40
	ldrd	r2, [r3]
	strd	r2, [sp, #400]
.L26:
	.loc 1 113 12
	adr	r3, .L68+32
	ldrd	r2, [r3]
	ldrd	r0, [sp, #392]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	beq	.L28
	.loc 1 113 43 discriminator 1
	adr	r3, .L68+32
	ldrd	r2, [r3]
	strd	r2, [sp, #392]
.L28:
	.loc 1 114 12
	adr	r3, .L68+40
	ldrd	r2, [r3]
	ldrd	r0, [sp, #392]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L30
	.loc 1 114 44 discriminator 1
	adr	r3, .L68+40
	ldrd	r2, [r3]
	strd	r2, [sp, #392]
.L30:
	.loc 1 116 16
	ldrd	r2, [sp, #408]
	ldrd	r0, [sp, #480]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #360]
	.loc 1 117 16
	ldrd	r2, [sp, #400]
	ldrd	r0, [sp, #472]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #352]
	.loc 1 118 16
	ldrd	r2, [sp, #392]
	ldrd	r0, [sp, #464]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #344]
	.loc 1 121 13
	vldr.64	d0, [sp, #360]
	bl	__SEGGER_RTL_float64_isnan
	mov	r3, r0
	.loc 1 121 12 discriminator 1
	cmp	r3, #0
	bne	.L32
	.loc 1 121 30 discriminator 2
	vldr.64	d0, [sp, #352]
	bl	__SEGGER_RTL_float64_isnan
	mov	r3, r0
	.loc 1 121 27 discriminator 1
	cmp	r3, #0
	bne	.L32
	.loc 1 121 47 discriminator 4
	vldr.64	d0, [sp, #344]
	bl	__SEGGER_RTL_float64_isnan
	mov	r3, r0
	.loc 1 121 44 discriminator 1
	cmp	r3, #0
	bne	.L32
	.loc 1 122 13
	ldr	r3, [sp, #360]
	str	r3, [sp, #40]
	ldr	r3, [sp, #364]
	bic	r3, r3, #-2147483648
	str	r3, [sp, #44]
	.loc 1 121 61 discriminator 6
	mov	r2, #0
	ldr	r3, .L68+24
	ldrd	r0, [sp, #40]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	bne	.L32
	.loc 1 122 39
	ldr	r3, [sp, #352]
	str	r3, [sp, #48]
	ldr	r3, [sp, #356]
	bic	r3, r3, #-2147483648
	str	r3, [sp, #52]
	.loc 1 122 36
	mov	r2, #0
	ldr	r3, .L68+24
	ldrd	r0, [sp, #48]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	bne	.L32
	.loc 1 122 65 discriminator 1
	ldr	r3, [sp, #344]
	str	r3, [sp, #56]
	ldr	r3, [sp, #348]
	bic	r3, r3, #-2147483648
	str	r3, [sp, #60]
	.loc 1 122 62 discriminator 1
	mov	r2, #0
	ldr	r3, .L68+24
	ldrd	r0, [sp, #56]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	beq	.L60
.L32:
	.loc 1 124 20
	mov	r2, #0
	ldr	r3, .L68+28
	ldrd	r0, [sp, #448]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #448]
	.loc 1 125 13
	b	.L45
.L60:
	.loc 1 129 16
	mov	r2, #0
	mov	r3, #0
	strd	r2, [sp, #384]
.LBB7:
	.loc 1 130 18
	movs	r3, #0
	str	r3, [sp, #380]
	.loc 1 130 9
	b	.L35
.L36:
.LBB8:
	.loc 1 131 37
	ldr	r2, [sp, #380]
	mov	r3, r2
	lsls	r3, r3, #1
	add	r3, r3, r2
	lsls	r3, r3, #3
	mov	r2, r3
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	add	r3, r3, r2
	.loc 1 131 40
	ldrd	r2, [r3]
	.loc 1 131 20
	ldrd	r0, [sp, #360]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #336]
	.loc 1 132 37
	ldr	r2, [sp, #380]
	mov	r3, r2
	lsls	r3, r3, #1
	add	r3, r3, r2
	lsls	r3, r3, #3
	mov	r2, r3
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	add	r3, r3, r2
	.loc 1 132 40
	ldrd	r2, [r3, #8]
	.loc 1 132 20
	ldrd	r0, [sp, #352]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #328]
	.loc 1 133 37
	ldr	r2, [sp, #380]
	mov	r3, r2
	lsls	r3, r3, #1
	add	r3, r3, r2
	lsls	r3, r3, #3
	mov	r2, r3
	add	r3, sp, #488
	sub	r3, r3, #412
	ldr	r3, [r3]
	add	r3, r3, r2
	.loc 1 133 40
	ldrd	r2, [r3, #16]
	.loc 1 133 20
	ldrd	r0, [sp, #344]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #320]
	.loc 1 134 32
	ldrd	r2, [sp, #336]
	ldrd	r0, [sp, #336]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 134 40
	ldrd	r2, [sp, #328]
	ldrd	r0, [sp, #328]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 134 36
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 134 48
	ldrd	r2, [sp, #320]
	ldrd	r0, [sp, #320]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 134 25
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	vmov	d7, r2, r3
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	sqrt
	vstr.64	d0, [sp, #312]
	.loc 1 135 40
	ldr	r3, [sp, #380]
	lsls	r2, r3, #3
	add	r3, sp, #488
	sub	r3, r3, #420
	ldr	r3, [r3]
	add	r3, r3, r2
	ldrd	r2, [r3]
	.loc 1 135 20
	ldrd	r0, [sp, #312]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #304]
	.loc 1 136 30
	ldrd	r2, [sp, #304]
	ldrd	r0, [sp, #304]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 136 23
	ldrd	r0, [sp, #384]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #384]
.LBE8:
	.loc 1 130 43 discriminator 3
	ldr	r3, [sp, #380]
	adds	r3, r3, #1
	str	r3, [sp, #380]
.L35:
	.loc 1 130 27 discriminator 1
	add	r3, sp, #488
	sub	r3, r3, #416
	ldr	r2, [sp, #380]
	ldr	r3, [r3]
	cmp	r2, r3
	blt	.L36
.LBE7:
	.loc 1 140 12
	ldrd	r2, [sp, #432]
	ldrd	r0, [sp, #384]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L61
	.loc 1 141 20
	ldrd	r2, [sp, #360]
	strd	r2, [sp, #480]
	.loc 1 142 20
	ldrd	r2, [sp, #352]
	strd	r2, [sp, #472]
	.loc 1 143 20
	ldrd	r2, [sp, #344]
	strd	r2, [sp, #464]
	.loc 1 145 20
	adr	r3, .L68
	ldrd	r2, [r3]
	ldrd	r0, [sp, #448]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #448]
	.loc 1 146 16
	adr	r3, .L68+8
	ldrd	r2, [r3]
	ldrd	r0, [sp, #448]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L39
	.loc 1 146 39 discriminator 1
	adr	r3, .L68+8
	ldrd	r2, [r3]
	strd	r2, [sp, #448]
.L39:
	.loc 1 148 29
	ldrd	r2, [sp, #408]
	ldrd	r0, [sp, #408]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 148 47
	ldrd	r2, [sp, #400]
	ldrd	r0, [sp, #400]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 148 38
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 148 65
	ldrd	r2, [sp, #392]
	ldrd	r0, [sp, #392]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 148 17
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	vmov	d7, r2, r3
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	sqrt
	vmov	r0, r1, d0
	.loc 1 148 16 discriminator 1
	adr	r3, .L68+16
	ldrd	r2, [r3]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	bne	.L62
	b	.L45
.L69:
	.align	3
.L68:
	.word	1717986918
	.word	1072064102
	.word	-1998362383
	.word	1055193269
	.word	-350469331
	.word	1058682594
	.word	1079574528
	.word	1075052544
	.word	858993459
	.word	1069757235
	.word	858993459
	.word	-1077726413
.L61:
	.loc 1 151 20
	ldrd	r0, [sp, #448]
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #448]
	.loc 1 152 16
	adr	r3, .L70
	ldrd	r2, [r3]
	ldrd	r0, [sp, #448]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	bne	.L63
.L45:
.LBE4:
	.loc 1 43 43 discriminator 2
	ldr	r3, [sp, #444]
	adds	r3, r3, #1
	str	r3, [sp, #444]
.L12:
	.loc 1 43 29 discriminator 1
	ldr	r2, [sp, #444]
	ldr	r3, [sp, #376]
	cmp	r2, r3
	blt	.L47
	b	.L46
.L62:
.LBB9:
	.loc 1 149 17
	nop
	b	.L46
.L63:
	.loc 1 153 17
	nop
.L46:
.LBE9:
.LBE3:
	.loc 1 157 16
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r1, [r3]
	ldrd	r2, [sp, #480]
	strd	r2, [r1]
	.loc 1 158 16
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r1, [r3]
	ldrd	r2, [sp, #472]
	strd	r2, [r1, #8]
	.loc 1 159 16
	add	r3, sp, #488
	sub	r3, r3, #424
	ldr	r1, [r3]
	ldrd	r2, [sp, #464]
	strd	r2, [r1, #16]
	.loc 1 161 12
	movs	r3, #1
.L4:
	.loc 1 162 1
	mov	r0, r3
	add	sp, sp, #492
	.cfi_def_cfa_offset 36
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L71:
	.align	3
.L70:
	.word	0
	.word	1093567616
	.cfi_endproc
.LFE13:
	.size	tof_3d_localize, .-tof_3d_localize
	.section	.text.calculate_anchor_geometry,"ax",%progbits
	.align	1
	.global	calculate_anchor_geometry
	.syntax unified
	.thumb
	.thumb_func
	.type	calculate_anchor_geometry, %function
calculate_anchor_geometry:
.LFB14:
	.loc 1 168 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 80
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, lr}
	.cfi_def_cfa_offset 12
	.cfi_offset 4, -12
	.cfi_offset 5, -8
	.cfi_offset 14, -4
	sub	sp, sp, #84
	.cfi_def_cfa_offset 96
	vstr.64	d0, [sp, #56]
	vstr.64	d1, [sp, #48]
	vstr.64	d2, [sp, #40]
	vstr.64	d3, [sp, #32]
	vstr.64	d4, [sp, #24]
	vstr.64	d5, [sp, #16]
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	str	r2, [sp, #4]
	.loc 1 169 8
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #56]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L73
	.loc 1 169 18 discriminator 2
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #48]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L73
	.loc 1 169 30 discriminator 4
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #40]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L73
	.loc 1 169 42 discriminator 6
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #32]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L73
	.loc 1 169 54 discriminator 8
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #24]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L73
	.loc 1 169 66 discriminator 10
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #16]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	beq	.L86
.L73:
	.loc 1 169 86 discriminator 11
	movs	r3, #0
	.loc 1 169 86 is_stmt 0
	b	.L76
.L86:
	.loc 1 172 11 is_stmt 1
	ldr	r1, [sp, #12]
	ldrd	r2, [sp, #56]
	strd	r2, [r1]
	.loc 1 173 11
	ldr	r1, [sp, #12]
	mov	r2, #0
	mov	r3, #0
	strd	r2, [r1, #8]
	.loc 1 174 11
	ldr	r1, [sp, #12]
	mov	r2, #0
	mov	r3, #0
	strd	r2, [r1, #16]
	.loc 1 178 18
	ldrd	r2, [sp, #48]
	ldrd	r0, [sp, #48]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 178 30
	ldrd	r2, [sp, #56]
	ldrd	r0, [sp, #56]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 178 24
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 178 42
	ldrd	r2, [sp, #32]
	ldrd	r0, [sp, #32]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 178 36
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 178 56
	ldrd	r0, [sp, #56]
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 178 49
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 178 11
	ldr	r1, [sp, #8]
	strd	r2, [r1]
	.loc 1 180 24
	ldrd	r2, [sp, #48]
	ldrd	r0, [sp, #48]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 180 35
	ldr	r3, [sp, #8]
	ldrd	r0, [r3]
	.loc 1 180 43
	ldr	r3, [sp, #8]
	ldrd	r2, [r3]
	.loc 1 180 39
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 180 12
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #72]
	.loc 1 181 8
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #72]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L77
	.loc 1 181 26 discriminator 1
	mov	r2, #0
	mov	r3, #0
	strd	r2, [sp, #72]
.L77:
	.loc 1 182 13
	vldr.64	d0, [sp, #72]
	bl	sqrt
	vmov.f32	s14, s0
	vmov.f32	s15, s1
	.loc 1 182 11 discriminator 1
	ldr	r3, [sp, #8]
	vstr.64	d7, [r3, #8]
	.loc 1 183 11
	ldr	r1, [sp, #8]
	mov	r2, #0
	mov	r3, #0
	strd	r2, [r1, #16]
	.loc 1 186 18
	ldrd	r2, [sp, #40]
	ldrd	r0, [sp, #40]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 186 30
	ldrd	r2, [sp, #56]
	ldrd	r0, [sp, #56]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 186 24
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 186 42
	ldrd	r2, [sp, #24]
	ldrd	r0, [sp, #24]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 186 36
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 186 56
	ldrd	r0, [sp, #56]
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 186 49
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 186 11
	ldr	r1, [sp, #4]
	strd	r2, [r1]
	.loc 1 188 11
	ldr	r3, [sp, #8]
	ldrd	r0, [r3, #8]
	.loc 1 188 8
	adr	r3, .L88
	ldrd	r2, [r3]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L87
	.loc 1 188 30 discriminator 1
	movs	r3, #0
	.loc 1 188 30 is_stmt 0
	b	.L76
.L87:
	.loc 1 190 18 is_stmt 1
	ldrd	r2, [sp, #48]
	ldrd	r0, [sp, #48]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 190 30
	ldrd	r2, [sp, #40]
	ldrd	r0, [sp, #40]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 190 24
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 190 42
	ldrd	r2, [sp, #16]
	ldrd	r0, [sp, #16]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 190 36
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 190 58
	ldr	r3, [sp, #4]
	ldrd	r0, [r3]
	.loc 1 190 54
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 190 66
	ldr	r3, [sp, #8]
	ldrd	r2, [r3]
	.loc 1 190 62
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 190 48
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 190 82
	ldr	r3, [sp, #8]
	ldrd	r0, [r3, #8]
	.loc 1 190 78
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 190 71
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 190 11
	ldr	r1, [sp, #4]
	strd	r2, [r1, #8]
	.loc 1 192 24
	ldrd	r2, [sp, #40]
	ldrd	r0, [sp, #40]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 192 35
	ldr	r3, [sp, #4]
	ldrd	r0, [r3]
	.loc 1 192 43
	ldr	r3, [sp, #4]
	ldrd	r2, [r3]
	.loc 1 192 39
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 192 30
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 192 53
	ldr	r3, [sp, #4]
	ldrd	r0, [r3, #8]
	.loc 1 192 61
	ldr	r3, [sp, #4]
	ldrd	r2, [r3, #8]
	.loc 1 192 57
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 192 12
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #64]
	.loc 1 193 8
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #64]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L81
	.loc 1 193 26 discriminator 1
	mov	r2, #0
	mov	r3, #0
	strd	r2, [sp, #64]
.L81:
	.loc 1 194 13
	vldr.64	d0, [sp, #64]
	bl	sqrt
	vmov.f32	s14, s0
	vmov.f32	s15, s1
	.loc 1 194 11 discriminator 1
	ldr	r3, [sp, #4]
	vstr.64	d7, [r3, #16]
	.loc 1 196 12
	movs	r3, #1
.L76:
	.loc 1 197 1
	mov	r0, r3
	add	sp, sp, #84
	.cfi_def_cfa_offset 12
	@ sp needed
	pop	{r4, r5, pc}
.L89:
	.align	3
.L88:
	.word	-1598689907
	.word	1051772663
	.cfi_endproc
.LFE14:
	.size	calculate_anchor_geometry, .-calculate_anchor_geometry
	.text
.Letext0:
	.file 2 "D:\\TN\\Hybrid_Positioning\\Hybrid_Positioning\\ss_twr_init\\utils.h"
	.file 3 "C:/Program Files/SEGGER/SEGGER Embedded Studio 8.24/include/math.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x4ae
	.2byte	0x4
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF35
	.byte	0xc
	.4byte	.LASF36
	.4byte	.LASF37
	.4byte	.Ldebug_ranges0+0x18
	.4byte	0
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x3
	.4byte	0x25
	.uleb128 0x4
	.byte	0x1
	.byte	0x8
	.4byte	.LASF0
	.uleb128 0x4
	.byte	0x4
	.byte	0x7
	.4byte	.LASF1
	.uleb128 0x4
	.byte	0x4
	.byte	0x5
	.4byte	.LASF2
	.uleb128 0x4
	.byte	0x4
	.byte	0x4
	.4byte	.LASF3
	.uleb128 0x4
	.byte	0x8
	.byte	0x3
	.4byte	.LASF4
	.uleb128 0x4
	.byte	0x8
	.byte	0x4
	.4byte	.LASF5
	.uleb128 0x3
	.4byte	0x54
	.uleb128 0x4
	.byte	0x10
	.byte	0x3
	.4byte	.LASF6
	.uleb128 0x4
	.byte	0x8
	.byte	0x4
	.4byte	.LASF7
	.uleb128 0x4
	.byte	0x10
	.byte	0x3
	.4byte	.LASF8
	.uleb128 0x5
	.byte	0x18
	.byte	0x2
	.byte	0xe
	.byte	0x9
	.4byte	0xa0
	.uleb128 0x6
	.ascii	"x\000"
	.byte	0x2
	.byte	0xf
	.byte	0xc
	.4byte	0x54
	.byte	0
	.uleb128 0x6
	.ascii	"y\000"
	.byte	0x2
	.byte	0x10
	.byte	0xc
	.4byte	0x54
	.byte	0x8
	.uleb128 0x6
	.ascii	"z\000"
	.byte	0x2
	.byte	0x11
	.byte	0xc
	.4byte	0x54
	.byte	0x10
	.byte	0
	.uleb128 0x7
	.4byte	.LASF38
	.byte	0x2
	.byte	0x12
	.byte	0x3
	.4byte	0x75
	.uleb128 0x3
	.4byte	0xa0
	.uleb128 0x4
	.byte	0x8
	.byte	0x5
	.4byte	.LASF9
	.uleb128 0x8
	.4byte	.LASF10
	.byte	0x3
	.2byte	0x101
	.byte	0xd
	.4byte	0x54
	.4byte	0xcf
	.uleb128 0x9
	.4byte	0x54
	.byte	0
	.uleb128 0xa
	.4byte	.LASF11
	.byte	0x3
	.byte	0x63
	.byte	0xd
	.4byte	0x25
	.4byte	0xe5
	.uleb128 0x9
	.4byte	0x54
	.byte	0
	.uleb128 0xa
	.4byte	.LASF12
	.byte	0x3
	.byte	0x65
	.byte	0xd
	.4byte	0x25
	.4byte	0xfb
	.uleb128 0x9
	.4byte	0x54
	.byte	0
	.uleb128 0xb
	.4byte	.LASF15
	.byte	0x1
	.byte	0xa5
	.byte	0x5
	.4byte	0x25
	.4byte	.LFB14
	.4byte	.LFE14-.LFB14
	.uleb128 0x1
	.byte	0x9c
	.4byte	0x1bd
	.uleb128 0xc
	.ascii	"d01\000"
	.byte	0x1
	.byte	0xa5
	.byte	0x26
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0xc
	.ascii	"d02\000"
	.byte	0x1
	.byte	0xa5
	.byte	0x32
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0xc
	.ascii	"d03\000"
	.byte	0x1
	.byte	0xa5
	.byte	0x3e
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0xc
	.ascii	"d12\000"
	.byte	0x1
	.byte	0xa6
	.byte	0x26
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0xc
	.ascii	"d13\000"
	.byte	0x1
	.byte	0xa6
	.byte	0x32
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0xc
	.ascii	"d23\000"
	.byte	0x1
	.byte	0xa6
	.byte	0x3e
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -80
	.uleb128 0xc
	.ascii	"a1\000"
	.byte	0x1
	.byte	0xa7
	.byte	0x25
	.4byte	0x1bd
	.uleb128 0x3
	.byte	0x91
	.sleb128 -84
	.uleb128 0xc
	.ascii	"a2\000"
	.byte	0x1
	.byte	0xa7
	.byte	0x2f
	.4byte	0x1bd
	.uleb128 0x3
	.byte	0x91
	.sleb128 -88
	.uleb128 0xc
	.ascii	"a3\000"
	.byte	0x1
	.byte	0xa7
	.byte	0x39
	.4byte	0x1bd
	.uleb128 0x3
	.byte	0x91
	.sleb128 -92
	.uleb128 0xd
	.4byte	.LASF13
	.byte	0x1
	.byte	0xb4
	.byte	0xc
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0xd
	.4byte	.LASF14
	.byte	0x1
	.byte	0xc0
	.byte	0xc
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0xe
	.byte	0x4
	.4byte	0xa0
	.uleb128 0xb
	.4byte	.LASF16
	.byte	0x1
	.byte	0xa
	.byte	0x5
	.4byte	0x25
	.4byte	.LFB13
	.4byte	.LFE13-.LFB13
	.uleb128 0x1
	.byte	0x9c
	.4byte	0x483
	.uleb128 0xc
	.ascii	"anc\000"
	.byte	0x1
	.byte	0xa
	.byte	0x20
	.4byte	0x483
	.uleb128 0x3
	.byte	0x91
	.sleb128 -452
	.uleb128 0xf
	.4byte	.LASF17
	.byte	0x1
	.byte	0xa
	.byte	0x2b
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -456
	.uleb128 0xf
	.4byte	.LASF18
	.byte	0x1
	.byte	0xb
	.byte	0x22
	.4byte	0x489
	.uleb128 0x3
	.byte	0x91
	.sleb128 -460
	.uleb128 0xf
	.4byte	.LASF19
	.byte	0x1
	.byte	0xc
	.byte	0x1b
	.4byte	0x1bd
	.uleb128 0x3
	.byte	0x91
	.sleb128 -464
	.uleb128 0xd
	.4byte	.LASF20
	.byte	0x1
	.byte	0x1c
	.byte	0xc
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0xd
	.4byte	.LASF21
	.byte	0x1
	.byte	0x1c
	.byte	0x14
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0xd
	.4byte	.LASF22
	.byte	0x1
	.byte	0x1c
	.byte	0x1c
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0xd
	.4byte	.LASF23
	.byte	0x1
	.byte	0x1f
	.byte	0x9
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -68
	.uleb128 0xd
	.4byte	.LASF24
	.byte	0x1
	.byte	0x28
	.byte	0xc
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -80
	.uleb128 0xd
	.4byte	.LASF25
	.byte	0x1
	.byte	0x29
	.byte	0xf
	.4byte	0x2c
	.uleb128 0x3
	.byte	0x91
	.sleb128 -152
	.uleb128 0x10
	.4byte	.LBB2
	.4byte	.LBE2-.LBB2
	.4byte	0x296
	.uleb128 0x11
	.ascii	"i\000"
	.byte	0x1
	.byte	0x20
	.byte	0xe
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.byte	0
	.uleb128 0x12
	.4byte	.LBB3
	.4byte	.LBE3-.LBB3
	.uleb128 0xd
	.4byte	.LASF26
	.byte	0x1
	.byte	0x2b
	.byte	0xe
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -84
	.uleb128 0x13
	.4byte	.Ldebug_ranges0+0
	.uleb128 0x11
	.ascii	"jtj\000"
	.byte	0x1
	.byte	0x2d
	.byte	0x10
	.4byte	0x48f
	.uleb128 0x3
	.byte	0x91
	.sleb128 -448
	.uleb128 0x11
	.ascii	"jtf\000"
	.byte	0x1
	.byte	0x2e
	.byte	0x10
	.4byte	0x4a5
	.uleb128 0x3
	.byte	0x91
	.sleb128 -304
	.uleb128 0xd
	.4byte	.LASF27
	.byte	0x1
	.byte	0x2f
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -96
	.uleb128 0x11
	.ascii	"det\000"
	.byte	0x1
	.byte	0x51
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -160
	.uleb128 0x11
	.ascii	"inv\000"
	.byte	0x1
	.byte	0x5b
	.byte	0x10
	.4byte	0x48f
	.uleb128 0x3
	.byte	0x91
	.sleb128 -376
	.uleb128 0xd
	.4byte	.LASF28
	.byte	0x1
	.byte	0x68
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -120
	.uleb128 0xd
	.4byte	.LASF29
	.byte	0x1
	.byte	0x69
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -128
	.uleb128 0xd
	.4byte	.LASF30
	.byte	0x1
	.byte	0x6a
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -136
	.uleb128 0xd
	.4byte	.LASF31
	.byte	0x1
	.byte	0x74
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -168
	.uleb128 0xd
	.4byte	.LASF32
	.byte	0x1
	.byte	0x75
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -176
	.uleb128 0xd
	.4byte	.LASF33
	.byte	0x1
	.byte	0x76
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -184
	.uleb128 0xd
	.4byte	.LASF34
	.byte	0x1
	.byte	0x81
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -144
	.uleb128 0x10
	.4byte	.LBB5
	.4byte	.LBE5-.LBB5
	.4byte	0x412
	.uleb128 0x11
	.ascii	"i\000"
	.byte	0x1
	.byte	0x32
	.byte	0x12
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -100
	.uleb128 0x12
	.4byte	.LBB6
	.4byte	.LBE6-.LBB6
	.uleb128 0x11
	.ascii	"dx\000"
	.byte	0x1
	.byte	0x34
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -232
	.uleb128 0x11
	.ascii	"dy\000"
	.byte	0x1
	.byte	0x35
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -240
	.uleb128 0x11
	.ascii	"dz\000"
	.byte	0x1
	.byte	0x36
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -248
	.uleb128 0x11
	.ascii	"r\000"
	.byte	0x1
	.byte	0x38
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -112
	.uleb128 0x11
	.ascii	"res\000"
	.byte	0x1
	.byte	0x3b
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -256
	.uleb128 0x11
	.ascii	"jx\000"
	.byte	0x1
	.byte	0x3e
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -264
	.uleb128 0x11
	.ascii	"jy\000"
	.byte	0x1
	.byte	0x3f
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -272
	.uleb128 0x11
	.ascii	"jz\000"
	.byte	0x1
	.byte	0x40
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -280
	.byte	0
	.byte	0
	.uleb128 0x12
	.4byte	.LBB7
	.4byte	.LBE7-.LBB7
	.uleb128 0x11
	.ascii	"i\000"
	.byte	0x1
	.byte	0x82
	.byte	0x12
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -148
	.uleb128 0x12
	.4byte	.LBB8
	.4byte	.LBE8-.LBB8
	.uleb128 0x11
	.ascii	"tx\000"
	.byte	0x1
	.byte	0x83
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -192
	.uleb128 0x11
	.ascii	"ty\000"
	.byte	0x1
	.byte	0x84
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -200
	.uleb128 0x11
	.ascii	"tz\000"
	.byte	0x1
	.byte	0x85
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -208
	.uleb128 0x11
	.ascii	"tr\000"
	.byte	0x1
	.byte	0x86
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -216
	.uleb128 0x11
	.ascii	"res\000"
	.byte	0x1
	.byte	0x87
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -224
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xe
	.byte	0x4
	.4byte	0xac
	.uleb128 0xe
	.byte	0x4
	.4byte	0x5b
	.uleb128 0x14
	.4byte	0x54
	.4byte	0x4a5
	.uleb128 0x15
	.4byte	0x38
	.byte	0x2
	.uleb128 0x15
	.4byte	0x38
	.byte	0x2
	.byte	0
	.uleb128 0x16
	.4byte	0x54
	.uleb128 0x15
	.4byte	0x38
	.byte	0x2
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x2134
	.uleb128 0x19
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x55
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_pubnames,"",%progbits
	.4byte	0x87
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x4b2
	.4byte	0xb8
	.ascii	"sqrt\000"
	.4byte	0xcf
	.ascii	"__SEGGER_RTL_float64_isinf\000"
	.4byte	0xe5
	.ascii	"__SEGGER_RTL_float64_isnan\000"
	.4byte	0xfb
	.ascii	"calculate_anchor_geometry\000"
	.4byte	0x1c3
	.ascii	"tof_3d_localize\000"
	.4byte	0
	.section	.debug_pubtypes,"",%progbits
	.4byte	0xba
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x4b2
	.4byte	0x25
	.ascii	"int\000"
	.4byte	0x31
	.ascii	"char\000"
	.4byte	0x38
	.ascii	"unsigned int\000"
	.4byte	0x3f
	.ascii	"long int\000"
	.4byte	0x46
	.ascii	"float\000"
	.4byte	0x4d
	.ascii	"complex float\000"
	.4byte	0x54
	.ascii	"double\000"
	.4byte	0x60
	.ascii	"complex double\000"
	.4byte	0x67
	.ascii	"long double\000"
	.4byte	0x6e
	.ascii	"complex long double\000"
	.4byte	0xa0
	.ascii	"vec3\000"
	.4byte	0xb1
	.ascii	"long long int\000"
	.4byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x24
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.LFB13
	.4byte	.LFE13-.LFB13
	.4byte	.LFB14
	.4byte	.LFE14-.LFB14
	.4byte	0
	.4byte	0
	.section	.debug_ranges,"",%progbits
.Ldebug_ranges0:
	.4byte	.LBB4
	.4byte	.LBE4
	.4byte	.LBB9
	.4byte	.LBE9
	.4byte	0
	.4byte	0
	.4byte	.LFB13
	.4byte	.LFE13
	.4byte	.LFB14
	.4byte	.LFE14
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF29:
	.ascii	"dy_step\000"
.LASF4:
	.ascii	"complex float\000"
.LASF24:
	.ascii	"lambda\000"
.LASF13:
	.ascii	"y2_sq\000"
.LASF25:
	.ascii	"max_it\000"
.LASF36:
	.ascii	"D:\\TN\\Hybrid_Positioning\\Hybrid_Positioning\\ss_"
	.ascii	"twr_init\\utils.c\000"
.LASF16:
	.ascii	"tof_3d_localize\000"
.LASF3:
	.ascii	"float\000"
.LASF9:
	.ascii	"long long int\000"
.LASF20:
	.ascii	"curr_x\000"
.LASF21:
	.ascii	"curr_y\000"
.LASF2:
	.ascii	"long int\000"
.LASF34:
	.ascii	"next_cost\000"
.LASF18:
	.ascii	"distances\000"
.LASF35:
	.ascii	"GNU C99 14.2.1 20241119 -fmessage-length=0 -std=gnu"
	.ascii	"99 -mcpu=cortex-m4 -mlittle-endian -mfloat-abi=hard"
	.ascii	" -mabi=aapcs -mfpu=fpv4-sp-d16 -mthumb -munaligned-"
	.ascii	"access -mtp=soft -mfp16-format=ieee -gdwarf-4 -g2 -"
	.ascii	"gpubnames -fomit-frame-pointer -ffunction-sections "
	.ascii	"-fdata-sections -fshort-enums -fno-common\000"
.LASF7:
	.ascii	"long double\000"
.LASF12:
	.ascii	"__SEGGER_RTL_float64_isnan\000"
.LASF11:
	.ascii	"__SEGGER_RTL_float64_isinf\000"
.LASF31:
	.ascii	"next_x\000"
.LASF32:
	.ascii	"next_y\000"
.LASF33:
	.ascii	"next_z\000"
.LASF23:
	.ascii	"min_i\000"
.LASF30:
	.ascii	"dz_step\000"
.LASF1:
	.ascii	"unsigned int\000"
.LASF0:
	.ascii	"char\000"
.LASF17:
	.ascii	"num_anchors\000"
.LASF38:
	.ascii	"vec3\000"
.LASF37:
	.ascii	"D:\\TN\\Hybrid_Positioning\\Hybrid_Positioning\\ss_"
	.ascii	"twr_init\\SES\000"
.LASF14:
	.ascii	"z3_sq\000"
.LASF27:
	.ascii	"curr_cost\000"
.LASF6:
	.ascii	"complex double\000"
.LASF5:
	.ascii	"double\000"
.LASF28:
	.ascii	"dx_step\000"
.LASF10:
	.ascii	"sqrt\000"
.LASF8:
	.ascii	"complex long double\000"
.LASF15:
	.ascii	"calculate_anchor_geometry\000"
.LASF26:
	.ascii	"iter\000"
.LASF22:
	.ascii	"curr_z\000"
.LASF19:
	.ascii	"pos_est\000"
	.global	__aeabi_dcmple
	.global	__aeabi_ddiv
	.global	__aeabi_dadd
	.global	__aeabi_dmul
	.global	__aeabi_dsub
	.global	__aeabi_dcmplt
	.global	__aeabi_dcmpgt
	.ident	"GCC: (based on arm-14.2.Rel1 source release) 14.2.1 20241119"
