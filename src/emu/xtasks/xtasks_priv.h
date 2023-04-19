/* Copyright (c) 2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef XTASKS_PRIV_H
#define XTASKS_PRIV_H

#include "emu.h"
#include "model_cpu.h"
#include "model_thread.h"

/* Private enums */

enum xtasks_chan {
	CH_SUBSYSTEM = 0,
	CH_MAX,
};

struct xtasks_thread {
	struct model_thread m;
};

int model_xtasks_probe(struct emu *emu);
int model_xtasks_create(struct emu *emu);
int model_xtasks_connect(struct emu *emu);
int model_xtasks_event(struct emu *emu);
int model_xtasks_finish(struct emu *emu);

#endif /* XTASKS_PRIV_H */
