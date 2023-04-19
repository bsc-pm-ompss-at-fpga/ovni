/* Copyright (c) 2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#include <stddef.h>
#include <stdint.h>
#include "compat.h"
#include "instr_xtasks.h"

int
main(void)
{
	instr_start(0, 1);

	/* Example trace:
	 * 69539385   +69539385  Fse :00:00:00:00:00:00:00:00:4e:00:00:00:b0:ae:19:15
	 * 69539810        +425  Fse :01:00:00:00:00:00:00:00:4e:00:00:00:b0:ae:19:15
	 * 69540010        +200  Fse :00:00:00:00:00:00:00:00:50:00:00:00:b0:ae:19:15
	 * 69540215        +205  Fse :01:00:00:00:00:00:00:00:50:00:00:00:b0:ae:19:15
	 * 69540415        +200  Fse :00:00:00:00:00:00:00:00:4f:00:00:00:b0:ae:19:15
	 * 69540615        +200  Fse :01:00:00:00:00:00:00:00:4f:00:00:00:b0:ae:19:15
	 */

	instr_xtasks_ev(0, 0x4e, 123);
	sleep_us(100);
	instr_xtasks_ev(1, 0x4e, 123);

	instr_end();

	return 0;
}
