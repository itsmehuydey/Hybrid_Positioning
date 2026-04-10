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
	.section	.text.tof_2d_localize,"ax",%progbits
	.align	1
	.global	tof_2d_localize
	.syntax unified
	.thumb
	.thumb_func
	.type	tof_2d_localize, %function
tof_2d_localize:
.LFB13:
	.loc 1 13 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 320
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
	sub	sp, sp, #324
	.cfi_def_cfa_offset 360
	add	r4, sp, #320
	sub	r4, r4, #276
	str	r0, [r4]
	add	r0, sp, #320
	sub	r0, r0, #280
	str	r1, [r0]
	add	r1, sp, #320
	sub	r1, r1, #284
	str	r2, [r1]
	add	r2, sp, #320
	sub	r2, r2, #288
	str	r3, [r2]
	.loc 1 15 8
	add	r3, sp, #320
	sub	r3, r3, #280
	ldr	r3, [r3]
	cmp	r3, #2
	ble	.L2
	.loc 1 15 25 discriminator 1
	add	r3, sp, #320
	sub	r3, r3, #276
	ldr	r3, [r3]
	cmp	r3, #0
	beq	.L2
	.loc 1 15 33 discriminator 2
	add	r3, sp, #320
	sub	r3, r3, #284
	ldr	r3, [r3]
	cmp	r3, #0
	beq	.L2
	.loc 1 15 47 discriminator 3
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r3, [r3]
	cmp	r3, #0
	bne	.L3
.L2:
	.loc 1 16 16
	mov	r3, #-1
	b	.L4
.L3:
	.loc 1 19 9
	add	r3, sp, #320
	sub	r3, r3, #288
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
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r3, [r3]
	vldr.64	d7, [r3, #8]
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	__SEGGER_RTL_float64_isnan
	mov	r3, r0
	.loc 1 19 27 discriminator 1
	cmp	r3, #0
	bne	.L5
	.loc 1 20 9
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r3, [r3]
	vldr.64	d7, [r3]
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	__SEGGER_RTL_float64_isinf
	mov	r3, r0
	.loc 1 19 48 discriminator 2
	cmp	r3, #0
	bne	.L5
	.loc 1 20 30
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r3, [r3]
	vldr.64	d7, [r3, #8]
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	__SEGGER_RTL_float64_isinf
	mov	r3, r0
	.loc 1 20 27 discriminator 1
	cmp	r3, #0
	bne	.L5
	.loc 1 21 21
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r3, [r3]
	ldrd	r2, [r3]
	.loc 1 21 9
	mov	r6, r2
	bic	r7, r3, #-2147483648
	.loc 1 20 48 discriminator 1
	mov	r2, #0
	ldr	r3, .L58+8
	mov	r0, r6
	mov	r1, r7
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	bne	.L5
	.loc 1 21 51
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r3, [r3]
	ldrd	r2, [r3, #8]
	.loc 1 21 39
	mov	r8, r2
	bic	r9, r3, #-2147483648
	.loc 1 21 36
	mov	r2, #0
	ldr	r3, .L58+8
	mov	r0, r8
	mov	r1, r9
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	beq	.L6
.L5:
	.loc 1 23 20
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r1, [r3]
	mov	r2, #0
	mov	r3, #0
	strd	r2, [r1]
	.loc 1 24 20
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r1, [r3]
	mov	r2, #0
	mov	r3, #0
	strd	r2, [r1, #8]
.L6:
	.loc 1 30 9
	movs	r3, #0
	str	r3, [sp, #300]
.LBB2:
	.loc 1 31 14
	movs	r3, #1
	str	r3, [sp, #296]
	.loc 1 31 5
	b	.L8
.L11:
	.loc 1 32 22
	ldr	r3, [sp, #296]
	lsls	r3, r3, #3
	add	r2, sp, #320
	sub	r2, r2, #284
	ldr	r2, [r2]
	add	r3, r3, r2
	ldrd	r0, [r3]
	.loc 1 32 37
	ldr	r3, [sp, #300]
	lsls	r3, r3, #3
	add	r2, sp, #320
	sub	r2, r2, #284
	ldr	r2, [r2]
	add	r3, r3, r2
	ldrd	r2, [r3]
	.loc 1 32 12
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L9
	.loc 1 33 19
	ldr	r3, [sp, #296]
	str	r3, [sp, #300]
.L9:
	.loc 1 31 39 discriminator 2
	ldr	r3, [sp, #296]
	adds	r3, r3, #1
	str	r3, [sp, #296]
.L8:
	.loc 1 31 23 discriminator 1
	add	r3, sp, #320
	sub	r3, r3, #280
	ldr	r2, [sp, #296]
	ldr	r3, [r3]
	cmp	r2, r3
	blt	.L11
.LBE2:
	.loc 1 35 17
	ldr	r3, [sp, #300]
	lsls	r3, r3, #4
	add	r2, sp, #320
	sub	r2, r2, #276
	ldr	r2, [r2]
	add	r3, r3, r2
	.loc 1 35 12
	ldrd	r2, [r3]
	strd	r2, [sp, #312]
	.loc 1 36 17
	ldr	r3, [sp, #300]
	lsls	r3, r3, #4
	add	r2, sp, #320
	sub	r2, r2, #276
	ldr	r2, [r2]
	add	r3, r3, r2
	.loc 1 36 12
	ldrd	r2, [r3, #8]
	strd	r2, [sp, #304]
	.loc 1 38 12
	adr	r3, .L58
	ldrd	r2, [r3]
	strd	r2, [sp, #288]
	.loc 1 39 15
	movs	r3, #50
	str	r3, [sp, #224]
.LBB3:
	.loc 1 41 14
	movs	r3, #0
	str	r3, [sp, #284]
	.loc 1 41 5
	b	.L12
.L43:
.LBB4:
	.loc 1 43 16
	add	r3, sp, #320
	sub	r3, r3, #272
	mov	r0, r3
	movs	r3, #32
	mov	r2, r3
	movs	r1, #0
	bl	memset
	.loc 1 44 16
	add	r3, sp, #112
	movs	r2, #0
	str	r2, [r3]
	str	r2, [r3, #4]
	str	r2, [r3, #8]
	str	r2, [r3, #12]
	.loc 1 45 16
	mov	r2, #0
	mov	r3, #0
	strd	r2, [sp, #272]
.LBB5:
	.loc 1 48 18
	movs	r3, #0
	str	r3, [sp, #268]
	.loc 1 48 9
	b	.L13
.L59:
	.align	3
.L58:
	.word	-1717986918
	.word	1069128089
	.word	1079574528
.L16:
.LBB6:
	.loc 1 50 37
	ldr	r3, [sp, #268]
	lsls	r3, r3, #4
	add	r2, sp, #320
	sub	r2, r2, #276
	ldr	r2, [r2]
	add	r3, r3, r2
	.loc 1 50 40
	ldrd	r2, [r3]
	.loc 1 50 20
	ldrd	r0, [sp, #312]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #160]
	.loc 1 51 37
	ldr	r3, [sp, #268]
	lsls	r3, r3, #4
	add	r2, sp, #320
	sub	r2, r2, #276
	ldr	r2, [r2]
	add	r3, r3, r2
	.loc 1 51 40
	ldrd	r2, [r3, #8]
	.loc 1 51 20
	ldrd	r0, [sp, #304]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #152]
	.loc 1 53 31
	ldrd	r2, [sp, #160]
	ldrd	r0, [sp, #160]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 53 39
	ldrd	r2, [sp, #152]
	ldrd	r0, [sp, #152]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 53 24
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	vmov	d7, r2, r3
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	sqrt
	vstr.64	d0, [sp, #256]
	.loc 1 54 16
	adr	r3, .L60+24
	ldrd	r2, [r3]
	ldrd	r0, [sp, #256]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L14
	.loc 1 54 29 discriminator 1
	adr	r3, .L60+24
	ldrd	r2, [r3]
	strd	r2, [sp, #256]
.L14:
	.loc 1 56 39
	ldr	r3, [sp, #268]
	lsls	r3, r3, #3
	add	r2, sp, #320
	sub	r2, r2, #284
	ldr	r2, [r2]
	add	r3, r3, r2
	ldrd	r2, [r3]
	.loc 1 56 20
	ldrd	r0, [sp, #256]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #144]
	.loc 1 57 30
	ldrd	r2, [sp, #144]
	ldrd	r0, [sp, #144]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 57 23
	ldrd	r0, [sp, #272]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #272]
	.loc 1 59 20
	ldrd	r2, [sp, #256]
	ldrd	r0, [sp, #160]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #136]
	.loc 1 60 20
	ldrd	r2, [sp, #256]
	ldrd	r0, [sp, #152]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #128]
	.loc 1 62 19
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r4, [r3]
	.loc 1 62 29
	ldrd	r2, [sp, #136]
	ldrd	r0, [sp, #136]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 62 23
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	add	r1, sp, #320
	sub	r1, r1, #272
	strd	r2, [r1]
	.loc 1 62 41
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r4, [r3, #8]
	.loc 1 62 51
	ldrd	r2, [sp, #128]
	ldrd	r0, [sp, #136]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 62 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	add	r1, sp, #320
	sub	r1, r1, #272
	strd	r2, [r1, #8]
	.loc 1 63 19
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r4, [r3, #16]
	.loc 1 63 29
	ldrd	r2, [sp, #136]
	ldrd	r0, [sp, #128]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 63 23
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	add	r1, sp, #320
	sub	r1, r1, #272
	strd	r2, [r1, #16]
	.loc 1 63 41
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r4, [r3, #24]
	.loc 1 63 51
	ldrd	r2, [sp, #128]
	ldrd	r0, [sp, #128]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 63 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	add	r1, sp, #320
	sub	r1, r1, #272
	strd	r2, [r1, #24]
	.loc 1 65 16
	ldrd	r4, [sp, #112]
	.loc 1 65 26
	ldrd	r2, [sp, #144]
	ldrd	r0, [sp, #136]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 65 20
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #112]
	.loc 1 66 16
	ldrd	r4, [sp, #120]
	.loc 1 66 26
	ldrd	r2, [sp, #144]
	ldrd	r0, [sp, #128]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 66 20
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #120]
.LBE6:
	.loc 1 48 43 discriminator 2
	ldr	r3, [sp, #268]
	adds	r3, r3, #1
	str	r3, [sp, #268]
.L13:
	.loc 1 48 27 discriminator 1
	add	r3, sp, #320
	sub	r3, r3, #280
	ldr	r2, [sp, #268]
	ldr	r3, [r3]
	cmp	r2, r3
	blt	.L16
.LBE5:
	.loc 1 70 15
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r0, [r3]
	.loc 1 70 19
	ldrd	r2, [sp, #288]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	add	r1, sp, #320
	sub	r1, r1, #272
	strd	r2, [r1]
	.loc 1 71 15
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r0, [r3, #24]
	.loc 1 71 19
	ldrd	r2, [sp, #288]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	add	r1, sp, #320
	sub	r1, r1, #272
	strd	r2, [r1, #24]
	.loc 1 74 28
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r0, [r3]
	.loc 1 74 38
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r2, [r3, #24]
	.loc 1 74 31
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 74 50
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r0, [r3, #8]
	.loc 1 74 60
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r2, [r3, #16]
	.loc 1 74 53
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 74 16
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #216]
	.loc 1 76 13
	ldr	r3, [sp, #216]
	str	r3, [sp]
	ldr	r3, [sp, #220]
	bic	r3, r3, #-2147483648
	str	r3, [sp, #4]
	.loc 1 76 12
	adr	r3, .L60+24
	ldrd	r2, [r3]
	ldrd	r0, [sp]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L53
	.loc 1 77 20
	mov	r2, #0
	ldr	r3, .L60+16
	ldrd	r0, [sp, #288]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #288]
	.loc 1 78 13
	b	.L41
.L53:
	.loc 1 83 28
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r0, [r3, #24]
	.loc 1 83 32
	ldrd	r2, [sp, #216]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 83 19
	strd	r2, [sp, #80]
	.loc 1 84 28
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r2, [r3, #8]
	.loc 1 84 21
	mov	r10, r2
	eor	fp, r3, #-2147483648
	.loc 1 84 32
	ldrd	r2, [sp, #216]
	mov	r0, r10
	mov	r1, fp
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 84 19
	strd	r2, [sp, #88]
	.loc 1 85 28
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r2, [r3, #16]
	.loc 1 85 21
	str	r2, [sp, #8]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #12]
	.loc 1 85 32
	ldrd	r2, [sp, #216]
	ldrd	r0, [sp, #8]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 85 19
	strd	r2, [sp, #96]
	.loc 1 86 28
	add	r3, sp, #320
	sub	r3, r3, #272
	ldrd	r0, [r3]
	.loc 1 86 32
	ldrd	r2, [sp, #216]
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 86 19
	strd	r2, [sp, #104]
	.loc 1 88 34
	ldrd	r0, [sp, #80]
	.loc 1 88 41
	ldrd	r2, [sp, #112]
	.loc 1 88 37
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 88 53
	ldrd	r0, [sp, #88]
	.loc 1 88 60
	ldrd	r2, [sp, #120]
	.loc 1 88 56
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 88 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 88 16
	mov	r1, r2
	str	r1, [sp, #248]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #252]
	.loc 1 89 34
	ldrd	r0, [sp, #96]
	.loc 1 89 41
	ldrd	r2, [sp, #112]
	.loc 1 89 37
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 89 53
	ldrd	r0, [sp, #104]
	.loc 1 89 60
	ldrd	r2, [sp, #120]
	.loc 1 89 56
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 89 45
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 89 16
	mov	r1, r2
	str	r1, [sp, #240]
	eor	r3, r3, #-2147483648
	str	r3, [sp, #244]
	.loc 1 92 12
	adr	r3, .L60
	ldrd	r2, [r3]
	ldrd	r0, [sp, #248]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	beq	.L20
	.loc 1 92 43 discriminator 1
	adr	r3, .L60
	ldrd	r2, [r3]
	strd	r2, [sp, #248]
.L20:
	.loc 1 93 12
	adr	r3, .L60+8
	ldrd	r2, [r3]
	ldrd	r0, [sp, #248]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L22
	.loc 1 93 44 discriminator 1
	adr	r3, .L60+8
	ldrd	r2, [r3]
	strd	r2, [sp, #248]
.L22:
	.loc 1 94 12
	adr	r3, .L60
	ldrd	r2, [r3]
	ldrd	r0, [sp, #240]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	beq	.L24
	.loc 1 94 43 discriminator 1
	adr	r3, .L60
	ldrd	r2, [r3]
	strd	r2, [sp, #240]
.L24:
	.loc 1 95 12
	adr	r3, .L60+8
	ldrd	r2, [r3]
	ldrd	r0, [sp, #240]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L26
	.loc 1 95 44 discriminator 1
	adr	r3, .L60+8
	ldrd	r2, [r3]
	strd	r2, [sp, #240]
.L26:
	.loc 1 97 16
	ldrd	r2, [sp, #248]
	ldrd	r0, [sp, #312]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #208]
	.loc 1 98 16
	ldrd	r2, [sp, #240]
	ldrd	r0, [sp, #304]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #200]
	.loc 1 101 13
	vldr.64	d0, [sp, #208]
	bl	__SEGGER_RTL_float64_isnan
	mov	r3, r0
	.loc 1 101 12 discriminator 1
	cmp	r3, #0
	bne	.L28
	.loc 1 101 30 discriminator 2
	vldr.64	d0, [sp, #200]
	bl	__SEGGER_RTL_float64_isnan
	mov	r3, r0
	.loc 1 101 27 discriminator 1
	cmp	r3, #0
	bne	.L28
	.loc 1 102 13
	ldr	r3, [sp, #208]
	str	r3, [sp, #16]
	ldr	r3, [sp, #212]
	bic	r3, r3, #-2147483648
	str	r3, [sp, #20]
	.loc 1 101 44 discriminator 4
	mov	r2, #0
	ldr	r3, .L60+20
	ldrd	r0, [sp, #16]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	bne	.L28
	.loc 1 102 39
	ldr	r3, [sp, #200]
	str	r3, [sp, #24]
	ldr	r3, [sp, #204]
	bic	r3, r3, #-2147483648
	str	r3, [sp, #28]
	.loc 1 102 36
	mov	r2, #0
	ldr	r3, .L60+20
	ldrd	r0, [sp, #24]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	beq	.L54
.L28:
	.loc 1 104 20
	mov	r2, #0
	ldr	r3, .L60+16
	ldrd	r0, [sp, #288]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #288]
	.loc 1 105 13
	b	.L41
.L54:
	.loc 1 109 16
	mov	r2, #0
	mov	r3, #0
	strd	r2, [sp, #232]
.LBB7:
	.loc 1 110 18
	movs	r3, #0
	str	r3, [sp, #228]
	.loc 1 110 9
	b	.L31
.L61:
	.align	3
.L60:
	.word	858993459
	.word	1069757235
	.word	858993459
	.word	-1077726413
	.word	1075052544
	.word	1079574528
	.word	-1598689907
	.word	1051772663
.L32:
.LBB8:
	.loc 1 111 37
	ldr	r3, [sp, #228]
	lsls	r3, r3, #4
	add	r2, sp, #320
	sub	r2, r2, #276
	ldr	r2, [r2]
	add	r3, r3, r2
	.loc 1 111 40
	ldrd	r2, [r3]
	.loc 1 111 20
	ldrd	r0, [sp, #208]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #192]
	.loc 1 112 37
	ldr	r3, [sp, #228]
	lsls	r3, r3, #4
	add	r2, sp, #320
	sub	r2, r2, #276
	ldr	r2, [r2]
	add	r3, r3, r2
	.loc 1 112 40
	ldrd	r2, [r3, #8]
	.loc 1 112 20
	ldrd	r0, [sp, #200]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #184]
	.loc 1 113 32
	ldrd	r2, [sp, #192]
	ldrd	r0, [sp, #192]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 113 40
	ldrd	r2, [sp, #184]
	ldrd	r0, [sp, #184]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 113 25
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	vmov	d7, r2, r3
	vmov.f32	s0, s14
	vmov.f32	s1, s15
	bl	sqrt
	vstr.64	d0, [sp, #176]
	.loc 1 114 40
	ldr	r3, [sp, #228]
	lsls	r3, r3, #3
	add	r2, sp, #320
	sub	r2, r2, #284
	ldr	r2, [r2]
	add	r3, r3, r2
	ldrd	r2, [r3]
	.loc 1 114 20
	ldrd	r0, [sp, #176]
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #168]
	.loc 1 115 30
	ldrd	r2, [sp, #168]
	ldrd	r0, [sp, #168]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 115 23
	ldrd	r0, [sp, #232]
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #232]
.LBE8:
	.loc 1 110 43 discriminator 3
	ldr	r3, [sp, #228]
	adds	r3, r3, #1
	str	r3, [sp, #228]
.L31:
	.loc 1 110 27 discriminator 1
	add	r3, sp, #320
	sub	r3, r3, #280
	ldr	r2, [sp, #228]
	ldr	r3, [r3]
	cmp	r2, r3
	blt	.L32
.LBE7:
	.loc 1 119 12
	ldrd	r2, [sp, #272]
	ldrd	r0, [sp, #232]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L55
	.loc 1 120 20
	ldrd	r2, [sp, #208]
	strd	r2, [sp, #312]
	.loc 1 121 20
	ldrd	r2, [sp, #200]
	strd	r2, [sp, #304]
	.loc 1 123 20
	adr	r3, .L62
	ldrd	r2, [r3]
	ldrd	r0, [sp, #288]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #288]
	.loc 1 124 16
	adr	r3, .L62+8
	ldrd	r2, [r3]
	ldrd	r0, [sp, #288]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L35
	.loc 1 124 39 discriminator 1
	adr	r3, .L62+8
	ldrd	r2, [r3]
	strd	r2, [sp, #288]
.L35:
	.loc 1 126 29
	ldrd	r2, [sp, #248]
	ldrd	r0, [sp, #248]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 126 47
	ldrd	r2, [sp, #240]
	ldrd	r0, [sp, #240]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 126 17
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
	.loc 1 126 16 discriminator 1
	adr	r3, .L62+16
	ldrd	r2, [r3]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	bne	.L56
	b	.L41
.L55:
	.loc 1 129 20
	ldrd	r0, [sp, #288]
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #288]
	.loc 1 130 16
	adr	r3, .L62+24
	ldrd	r2, [r3]
	ldrd	r0, [sp, #288]
	bl	__aeabi_dcmpgt
	mov	r3, r0
	cmp	r3, #0
	bne	.L57
.L41:
.LBE4:
	.loc 1 41 43 discriminator 2
	ldr	r3, [sp, #284]
	adds	r3, r3, #1
	str	r3, [sp, #284]
.L12:
	.loc 1 41 29 discriminator 1
	ldr	r2, [sp, #284]
	ldr	r3, [sp, #224]
	cmp	r2, r3
	blt	.L43
	b	.L42
.L56:
.LBB9:
	.loc 1 127 17
	nop
	b	.L42
.L57:
	.loc 1 131 17
	nop
.L42:
.LBE9:
.LBE3:
	.loc 1 135 16
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r1, [r3]
	ldrd	r2, [sp, #312]
	strd	r2, [r1]
	.loc 1 136 16
	add	r3, sp, #320
	sub	r3, r3, #288
	ldr	r1, [r3]
	ldrd	r2, [sp, #304]
	strd	r2, [r1, #8]
	.loc 1 138 12
	movs	r3, #1
.L4:
	.loc 1 139 1
	mov	r0, r3
	add	sp, sp, #324
	.cfi_def_cfa_offset 36
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L63:
	.align	3
.L62:
	.word	1717986918
	.word	1072064102
	.word	-1998362383
	.word	1055193269
	.word	-350469331
	.word	1058682594
	.word	0
	.word	1093567616
	.cfi_endproc
.LFE13:
	.size	tof_2d_localize, .-tof_2d_localize
	.section	.text.calculate_anchor_geometry,"ax",%progbits
	.align	1
	.global	calculate_anchor_geometry
	.syntax unified
	.thumb
	.thumb_func
	.type	calculate_anchor_geometry, %function
calculate_anchor_geometry:
.LFB14:
	.loc 1 144 1
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 72
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, lr}
	.cfi_def_cfa_offset 12
	.cfi_offset 4, -12
	.cfi_offset 5, -8
	.cfi_offset 14, -4
	sub	sp, sp, #76
	.cfi_def_cfa_offset 88
	vstr.64	d0, [sp, #56]
	vstr.64	d1, [sp, #48]
	vstr.64	d2, [sp, #40]
	vstr.64	d3, [sp, #32]
	vstr.64	d4, [sp, #24]
	vstr.64	d5, [sp, #16]
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	str	r2, [sp, #4]
	.loc 1 145 8
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #56]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L65
	.loc 1 145 18 discriminator 2
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #48]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L65
	.loc 1 145 30 discriminator 4
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #40]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L65
	.loc 1 145 42 discriminator 6
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #32]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L65
	.loc 1 145 54 discriminator 8
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #24]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	bne	.L65
	.loc 1 145 66 discriminator 10
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #16]
	bl	__aeabi_dcmple
	mov	r3, r0
	cmp	r3, #0
	beq	.L75
.L65:
	.loc 1 145 86 discriminator 11
	movs	r3, #0
	.loc 1 145 86 is_stmt 0
	b	.L68
.L75:
	.loc 1 148 11 is_stmt 1
	ldr	r1, [sp, #12]
	ldrd	r2, [sp, #56]
	strd	r2, [r1]
	.loc 1 149 11
	ldr	r1, [sp, #12]
	mov	r2, #0
	mov	r3, #0
	strd	r2, [r1, #8]
	.loc 1 153 18
	ldrd	r2, [sp, #48]
	ldrd	r0, [sp, #48]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 153 30
	ldrd	r2, [sp, #56]
	ldrd	r0, [sp, #56]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 153 24
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 153 42
	ldrd	r2, [sp, #32]
	ldrd	r0, [sp, #32]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 153 36
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 153 56
	ldrd	r0, [sp, #56]
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 153 49
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 153 11
	ldr	r1, [sp, #8]
	strd	r2, [r1]
	.loc 1 155 24
	ldrd	r2, [sp, #48]
	ldrd	r0, [sp, #48]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 155 35
	ldr	r3, [sp, #8]
	ldrd	r0, [r3]
	.loc 1 155 43
	ldr	r3, [sp, #8]
	ldrd	r2, [r3]
	.loc 1 155 39
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 155 12
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	strd	r2, [sp, #64]
	.loc 1 156 8
	mov	r2, #0
	mov	r3, #0
	ldrd	r0, [sp, #64]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L69
	.loc 1 156 26 discriminator 1
	mov	r2, #0
	mov	r3, #0
	strd	r2, [sp, #64]
.L69:
	.loc 1 157 13
	vldr.64	d0, [sp, #64]
	bl	sqrt
	vmov.f32	s14, s0
	vmov.f32	s15, s1
	.loc 1 157 11 discriminator 1
	ldr	r3, [sp, #8]
	vstr.64	d7, [r3, #8]
	.loc 1 160 18
	ldrd	r2, [sp, #40]
	ldrd	r0, [sp, #40]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 160 30
	ldrd	r2, [sp, #56]
	ldrd	r0, [sp, #56]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 160 24
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 160 42
	ldrd	r2, [sp, #24]
	ldrd	r0, [sp, #24]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 160 36
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 160 56
	ldrd	r0, [sp, #56]
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 160 49
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 160 11
	ldr	r1, [sp, #4]
	strd	r2, [r1]
	.loc 1 162 11
	ldr	r3, [sp, #8]
	ldrd	r0, [r3, #8]
	.loc 1 162 8
	adr	r3, .L77
	ldrd	r2, [r3]
	bl	__aeabi_dcmplt
	mov	r3, r0
	cmp	r3, #0
	beq	.L76
	.loc 1 162 30 discriminator 1
	movs	r3, #0
	.loc 1 162 30 is_stmt 0
	b	.L68
.L76:
	.loc 1 164 18 is_stmt 1
	ldrd	r2, [sp, #48]
	ldrd	r0, [sp, #48]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 164 30
	ldrd	r2, [sp, #40]
	ldrd	r0, [sp, #40]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 164 24
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 164 42
	ldrd	r2, [sp, #16]
	ldrd	r0, [sp, #16]
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 164 36
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 164 58
	ldr	r3, [sp, #4]
	ldrd	r0, [r3]
	.loc 1 164 54
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	mov	r0, r2
	mov	r1, r3
	.loc 1 164 66
	ldr	r3, [sp, #8]
	ldrd	r2, [r3]
	.loc 1 164 62
	bl	__aeabi_dmul
	mov	r2, r0
	mov	r3, r1
	.loc 1 164 48
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_dsub
	mov	r2, r0
	mov	r3, r1
	mov	r4, r2
	mov	r5, r3
	.loc 1 164 82
	ldr	r3, [sp, #8]
	ldrd	r0, [r3, #8]
	.loc 1 164 78
	mov	r2, r0
	mov	r3, r1
	bl	__aeabi_dadd
	mov	r2, r0
	mov	r3, r1
	.loc 1 164 71
	mov	r0, r4
	mov	r1, r5
	bl	__aeabi_ddiv
	mov	r2, r0
	mov	r3, r1
	.loc 1 164 11
	ldr	r1, [sp, #4]
	strd	r2, [r1, #8]
	.loc 1 166 12
	movs	r3, #1
.L68:
	.loc 1 167 1
	mov	r0, r3
	add	sp, sp, #76
	.cfi_def_cfa_offset 12
	@ sp needed
	pop	{r4, r5, pc}
.L78:
	.align	3
.L77:
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
	.4byte	0x435
	.2byte	0x4
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF31
	.byte	0xc
	.4byte	.LASF32
	.4byte	.LASF33
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
	.byte	0x10
	.byte	0x2
	.byte	0xe
	.byte	0x9
	.4byte	0x95
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
	.byte	0
	.uleb128 0x7
	.4byte	.LASF34
	.byte	0x2
	.byte	0x11
	.byte	0x3
	.4byte	0x75
	.uleb128 0x3
	.4byte	0x95
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
	.4byte	0xc4
	.uleb128 0x9
	.4byte	0x54
	.byte	0
	.uleb128 0xa
	.4byte	.LASF11
	.byte	0x3
	.byte	0x63
	.byte	0xd
	.4byte	0x25
	.4byte	0xda
	.uleb128 0x9
	.4byte	0x54
	.byte	0
	.uleb128 0xa
	.4byte	.LASF12
	.byte	0x3
	.byte	0x65
	.byte	0xd
	.4byte	0x25
	.4byte	0xf0
	.uleb128 0x9
	.4byte	0x54
	.byte	0
	.uleb128 0xb
	.4byte	.LASF13
	.byte	0x1
	.byte	0x8d
	.byte	0x5
	.4byte	0x25
	.4byte	.LFB14
	.4byte	.LFE14-.LFB14
	.uleb128 0x1
	.byte	0x9c
	.4byte	0x1a2
	.uleb128 0xc
	.ascii	"d01\000"
	.byte	0x1
	.byte	0x8d
	.byte	0x26
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0xc
	.ascii	"d02\000"
	.byte	0x1
	.byte	0x8d
	.byte	0x32
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0xc
	.ascii	"d03\000"
	.byte	0x1
	.byte	0x8d
	.byte	0x3e
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0xc
	.ascii	"d12\000"
	.byte	0x1
	.byte	0x8e
	.byte	0x26
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0xc
	.ascii	"d13\000"
	.byte	0x1
	.byte	0x8e
	.byte	0x32
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0xc
	.ascii	"d23\000"
	.byte	0x1
	.byte	0x8e
	.byte	0x3e
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0xc
	.ascii	"a1\000"
	.byte	0x1
	.byte	0x8f
	.byte	0x25
	.4byte	0x1a2
	.uleb128 0x3
	.byte	0x91
	.sleb128 -76
	.uleb128 0xc
	.ascii	"a2\000"
	.byte	0x1
	.byte	0x8f
	.byte	0x2f
	.4byte	0x1a2
	.uleb128 0x3
	.byte	0x91
	.sleb128 -80
	.uleb128 0xc
	.ascii	"a3\000"
	.byte	0x1
	.byte	0x8f
	.byte	0x39
	.4byte	0x1a2
	.uleb128 0x3
	.byte	0x91
	.sleb128 -84
	.uleb128 0xd
	.4byte	.LASF18
	.byte	0x1
	.byte	0x9b
	.byte	0xc
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0xe
	.byte	0x4
	.4byte	0x95
	.uleb128 0xb
	.4byte	.LASF14
	.byte	0x1
	.byte	0xa
	.byte	0x5
	.4byte	0x25
	.4byte	.LFB13
	.4byte	.LFE13-.LFB13
	.uleb128 0x1
	.byte	0x9c
	.4byte	0x40a
	.uleb128 0xc
	.ascii	"anc\000"
	.byte	0x1
	.byte	0xa
	.byte	0x20
	.4byte	0x40a
	.uleb128 0x3
	.byte	0x91
	.sleb128 -316
	.uleb128 0xf
	.4byte	.LASF15
	.byte	0x1
	.byte	0xa
	.byte	0x2b
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -320
	.uleb128 0xf
	.4byte	.LASF16
	.byte	0x1
	.byte	0xb
	.byte	0x22
	.4byte	0x410
	.uleb128 0x3
	.byte	0x91
	.sleb128 -324
	.uleb128 0xf
	.4byte	.LASF17
	.byte	0x1
	.byte	0xc
	.byte	0x1b
	.4byte	0x1a2
	.uleb128 0x3
	.byte	0x91
	.sleb128 -328
	.uleb128 0xd
	.4byte	.LASF19
	.byte	0x1
	.byte	0x1b
	.byte	0xc
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0xd
	.4byte	.LASF20
	.byte	0x1
	.byte	0x1b
	.byte	0x14
	.4byte	0x54
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0xd
	.4byte	.LASF21
	.byte	0x1
	.byte	0x1e
	.byte	0x9
	.4byte	0x25
	.uleb128 0x2
	.byte	0x91
	.sleb128 -60
	.uleb128 0xd
	.4byte	.LASF22
	.byte	0x1
	.byte	0x26
	.byte	0xc
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0xd
	.4byte	.LASF23
	.byte	0x1
	.byte	0x27
	.byte	0xf
	.4byte	0x2c
	.uleb128 0x3
	.byte	0x91
	.sleb128 -136
	.uleb128 0x10
	.4byte	.LBB2
	.4byte	.LBE2-.LBB2
	.4byte	0x26a
	.uleb128 0x11
	.ascii	"i\000"
	.byte	0x1
	.byte	0x1f
	.byte	0xe
	.4byte	0x25
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.byte	0
	.uleb128 0x12
	.4byte	.LBB3
	.4byte	.LBE3-.LBB3
	.uleb128 0xd
	.4byte	.LASF24
	.byte	0x1
	.byte	0x29
	.byte	0xe
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -76
	.uleb128 0x13
	.4byte	.Ldebug_ranges0+0
	.uleb128 0x11
	.ascii	"jtj\000"
	.byte	0x1
	.byte	0x2b
	.byte	0x10
	.4byte	0x416
	.uleb128 0x3
	.byte	0x91
	.sleb128 -312
	.uleb128 0x11
	.ascii	"jtf\000"
	.byte	0x1
	.byte	0x2c
	.byte	0x10
	.4byte	0x42c
	.uleb128 0x3
	.byte	0x91
	.sleb128 -248
	.uleb128 0xd
	.4byte	.LASF25
	.byte	0x1
	.byte	0x2d
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -88
	.uleb128 0x11
	.ascii	"det\000"
	.byte	0x1
	.byte	0x4a
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -144
	.uleb128 0x11
	.ascii	"inv\000"
	.byte	0x1
	.byte	0x52
	.byte	0x10
	.4byte	0x416
	.uleb128 0x3
	.byte	0x91
	.sleb128 -280
	.uleb128 0xd
	.4byte	.LASF26
	.byte	0x1
	.byte	0x58
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -112
	.uleb128 0xd
	.4byte	.LASF27
	.byte	0x1
	.byte	0x59
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -120
	.uleb128 0xd
	.4byte	.LASF28
	.byte	0x1
	.byte	0x61
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -152
	.uleb128 0xd
	.4byte	.LASF29
	.byte	0x1
	.byte	0x62
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -160
	.uleb128 0xd
	.4byte	.LASF30
	.byte	0x1
	.byte	0x6d
	.byte	0x10
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -128
	.uleb128 0x10
	.4byte	.LBB5
	.4byte	.LBE5-.LBB5
	.4byte	0x3a8
	.uleb128 0x11
	.ascii	"i\000"
	.byte	0x1
	.byte	0x30
	.byte	0x12
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -92
	.uleb128 0x12
	.4byte	.LBB6
	.4byte	.LBE6-.LBB6
	.uleb128 0x11
	.ascii	"dx\000"
	.byte	0x1
	.byte	0x32
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -200
	.uleb128 0x11
	.ascii	"dy\000"
	.byte	0x1
	.byte	0x33
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -208
	.uleb128 0x11
	.ascii	"r\000"
	.byte	0x1
	.byte	0x35
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -104
	.uleb128 0x11
	.ascii	"res\000"
	.byte	0x1
	.byte	0x38
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -216
	.uleb128 0x11
	.ascii	"jx\000"
	.byte	0x1
	.byte	0x3b
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -224
	.uleb128 0x11
	.ascii	"jy\000"
	.byte	0x1
	.byte	0x3c
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -232
	.byte	0
	.byte	0
	.uleb128 0x12
	.4byte	.LBB7
	.4byte	.LBE7-.LBB7
	.uleb128 0x11
	.ascii	"i\000"
	.byte	0x1
	.byte	0x6e
	.byte	0x12
	.4byte	0x25
	.uleb128 0x3
	.byte	0x91
	.sleb128 -132
	.uleb128 0x12
	.4byte	.LBB8
	.4byte	.LBE8-.LBB8
	.uleb128 0x11
	.ascii	"tx\000"
	.byte	0x1
	.byte	0x6f
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -168
	.uleb128 0x11
	.ascii	"ty\000"
	.byte	0x1
	.byte	0x70
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -176
	.uleb128 0x11
	.ascii	"tr\000"
	.byte	0x1
	.byte	0x71
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -184
	.uleb128 0x11
	.ascii	"res\000"
	.byte	0x1
	.byte	0x72
	.byte	0x14
	.4byte	0x54
	.uleb128 0x3
	.byte	0x91
	.sleb128 -192
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xe
	.byte	0x4
	.4byte	0xa1
	.uleb128 0xe
	.byte	0x4
	.4byte	0x5b
	.uleb128 0x14
	.4byte	0x54
	.4byte	0x42c
	.uleb128 0x15
	.4byte	0x38
	.byte	0x1
	.uleb128 0x15
	.4byte	0x38
	.byte	0x1
	.byte	0
	.uleb128 0x16
	.4byte	0x54
	.uleb128 0x15
	.4byte	0x38
	.byte	0x1
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
	.4byte	0x439
	.4byte	0xad
	.ascii	"sqrt\000"
	.4byte	0xc4
	.ascii	"__SEGGER_RTL_float64_isinf\000"
	.4byte	0xda
	.ascii	"__SEGGER_RTL_float64_isnan\000"
	.4byte	0xf0
	.ascii	"calculate_anchor_geometry\000"
	.4byte	0x1a8
	.ascii	"tof_2d_localize\000"
	.4byte	0
	.section	.debug_pubtypes,"",%progbits
	.4byte	0xba
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x439
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
	.4byte	0x95
	.ascii	"vec2\000"
	.4byte	0xa6
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
.LASF34:
	.ascii	"vec2\000"
.LASF22:
	.ascii	"lambda\000"
.LASF11:
	.ascii	"__SEGGER_RTL_float64_isinf\000"
.LASF24:
	.ascii	"iter\000"
.LASF17:
	.ascii	"pos_est\000"
.LASF27:
	.ascii	"dy_step\000"
.LASF28:
	.ascii	"next_x\000"
.LASF29:
	.ascii	"next_y\000"
.LASF3:
	.ascii	"float\000"
.LASF18:
	.ascii	"y2_sq\000"
.LASF26:
	.ascii	"dx_step\000"
.LASF5:
	.ascii	"double\000"
.LASF15:
	.ascii	"num_anchors\000"
.LASF32:
	.ascii	"D:\\TN\\Hybrid_Positioning\\Hybrid_Positioning\\ss_"
	.ascii	"twr_init\\utils.c\000"
.LASF12:
	.ascii	"__SEGGER_RTL_float64_isnan\000"
.LASF8:
	.ascii	"complex long double\000"
.LASF13:
	.ascii	"calculate_anchor_geometry\000"
.LASF1:
	.ascii	"unsigned int\000"
.LASF14:
	.ascii	"tof_2d_localize\000"
.LASF0:
	.ascii	"char\000"
.LASF4:
	.ascii	"complex float\000"
.LASF9:
	.ascii	"long long int\000"
.LASF20:
	.ascii	"curr_y\000"
.LASF30:
	.ascii	"next_cost\000"
.LASF6:
	.ascii	"complex double\000"
.LASF21:
	.ascii	"min_i\000"
.LASF33:
	.ascii	"D:\\TN\\Hybrid_Positioning\\Hybrid_Positioning\\ss_"
	.ascii	"twr_init\\SES\000"
.LASF2:
	.ascii	"long int\000"
.LASF31:
	.ascii	"GNU C99 14.2.1 20241119 -fmessage-length=0 -std=gnu"
	.ascii	"99 -mcpu=cortex-m4 -mlittle-endian -mfloat-abi=hard"
	.ascii	" -mabi=aapcs -mfpu=fpv4-sp-d16 -mthumb -munaligned-"
	.ascii	"access -mtp=soft -mfp16-format=ieee -gdwarf-4 -g2 -"
	.ascii	"gpubnames -fomit-frame-pointer -ffunction-sections "
	.ascii	"-fdata-sections -fshort-enums -fno-common\000"
.LASF7:
	.ascii	"long double\000"
.LASF25:
	.ascii	"curr_cost\000"
.LASF16:
	.ascii	"distances\000"
.LASF19:
	.ascii	"curr_x\000"
.LASF10:
	.ascii	"sqrt\000"
.LASF23:
	.ascii	"max_it\000"
	.global	__aeabi_dcmple
	.global	__aeabi_ddiv
	.global	__aeabi_dadd
	.global	__aeabi_dmul
	.global	__aeabi_dsub
	.global	__aeabi_dcmplt
	.global	__aeabi_dcmpgt
	.ident	"GCC: (based on arm-14.2.Rel1 source release) 14.2.1 20241119"
