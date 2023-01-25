/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#include "emu_ev.h"

void
emu_ev(struct emu_ev *ev, const struct ovni_ev *oev,
		int64_t sclock, int64_t dclock)
{
	memset(ev, 0, sizeof(struct emu_ev));

	ev->mcv[0] = ev->m = oev->header.model;
	ev->mcv[1] = ev->c = oev->header.category;
	ev->mcv[2] = ev->v = oev->header.value;
	ev->mcv[3] = '\0';

	ev->rclock = oev->header.clock;
	ev->sclock = sclock;
	ev->dclock = dclock;

	ev->payload_size = ovni_payload_size(oev);

	if (ev->payload_size > 0) {
		ev->has_payload = 1;
		ev->payload = &oev->payload;

		if (oev->header.flags & OVNI_EV_JUMBO) {
			ev->is_jumbo = 1;
		}
	}
}
