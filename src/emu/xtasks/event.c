/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#include "xtasks_priv.h"
#include "chan.h"
#include "common.h"
#include "emu.h"
#include "emu_ev.h"
#include "extend.h"
#include "model_thread.h"
#include "ovni.h"
#include "thread.h"
#include "value.h"

#define EV_APICALL 85

static int
event_s(struct emu *emu)
{
	struct xtasks_thread *th = extend_get(&emu->thread->ext, 'X');
	struct chan *ch = &th->m.ch[CH_SUBSYSTEM];

	if (emu->ev->payload_size != 16) {
		err("payload must be 16 bytes");
		return -1;
	}

	if (emu->ev->v != 'e') {
		err("unknown event value");
		return -1;
	}

	uint64_t value = emu->ev->payload->u64[0];
	uint32_t id = emu->ev->payload->u32[2];
	uint32_t type = emu->ev->payload->u32[3];

	info("got event %lX %X %X\n", value, id, type);
	uint64_t idOrVal = id;
	// When creating tasks or doing taskwait, the ID is the same (EV_APICALL),
	// and the value is used to know which one is it (5 create, 1 taskwait)
	if (id == EV_APICALL) {
		idOrVal = value;
	}
	switch (type) {
		case 0:
			return chan_push(ch, value_int64(idOrVal));
		case 1:
			return chan_pop(ch, value_int64(idOrVal));
		default:
			err("unknown value %ld", value);
			return -1;
	}

	return 0;
}

static int
process_ev(struct emu *emu)
{
	switch (emu->ev->c) {
		case 's':
			return event_s(emu);
		default:
			err("unknown xtasks event category");
			return -1;
	}

	/* Not reached */
	return 0;
}

int
model_xtasks_event(struct emu *emu)
{
	static int enabled = 0;

	if (!enabled) {
		if (model_xtasks_connect(emu) != 0) {
			err("xtasks_connect failed");
			return -1;
		}
		enabled = 1;
	}

	dbg("in xtasks_event");
	if (emu->ev->m != 'X') {
		err("unexpected event model %c", emu->ev->m);
		return -1;
	}

	dbg("got xtasks event %s", emu->ev->mcv);
	if (process_ev(emu) != 0) {
		err("error processing Kernel event");
		return -1;
	}

	return 0;
}
