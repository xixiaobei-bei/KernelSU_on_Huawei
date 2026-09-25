/*
 * Copyright (C) 2017 Huawei Technologies
 */

#include <asm/alternative.h>

	.globl	hkip_hvc3
	.globl	hkip_hvc4

	.text
	.align	3

	.type	hkip_hvc, %function
	.func	hkip_hvc
hkip_hvc:
hkip_hvc3:
hkip_hvc4:
alternative_if_not ARM64_HAS_HHEE
	b	hkip_hvc_dispatch
alternative_else
	hvc	#0
alternative_endif
	ret
	.endfunc
