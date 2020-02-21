.section ".text"
.global calc_add
.global calc_sub
.global calc_mul
.global calc_div

calc_add:
	add r0, r0, r1
	bx lr

calc_sub:
	sub r0, r0, r1
	bx lr

calc_mul: @long long calc_div(int l, int r)
	smull r0, r1, r0, r1
	bx lr

calc_div: @int calc_div(int* rem, int n, int d)
	cmp r2, #0
	bge calc_div_after_divisor_check
	neg r2, r2
	push {lr}
	bl calc_div_after_divisor_check
	pop {lr}
	neg r0, r0
	bx lr
calc_div_after_divisor_check:
	cmp r1, #0
	bge calc_div_after_num_check
	neg r1, r1
	push {lr}
	bl calc_div_after_num_check
	pop {lr}
	neg r0, r0
	bx lr
calc_div_after_num_check:
	mov r3, #0
	mov r4, r1
	b calc_div_loop_cond
calc_div_loop:
	add r3, r3, #1
	sub r4, r4, r2
calc_div_loop_cond:
	cmp r4, r2
	bge calc_div_loop
	stm r0, {r4}
	mov r0, r3
	bx lr
