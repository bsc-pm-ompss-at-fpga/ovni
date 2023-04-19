/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#include "xtasks_priv.h"
#include <stddef.h>
#include "common.h"
#include "emu.h"
#include "emu_prv.h"
#include "model.h"
#include "model_chan.h"
#include "model_cpu.h"
#include "model_pvt.h"
#include "model_thread.h"
#include "pv/pcf.h"
#include "pv/prv.h"
#include "system.h"
#include "track.h"

static const char model_name[] = "xtasks";
enum { model_id = 'X' };

struct model_spec model_xtasks = {
	.name = model_name,
	.model = model_id,
	.create  = model_xtasks_create,
//	.connect = model_xtasks_connect,
	.event   = model_xtasks_event,
	.probe   = model_xtasks_probe,
};

/* ----------------- channels ------------------ */

static const char *chan_name[CH_MAX] = {
	[CH_SUBSYSTEM] = "subsystem",
};

static const int chan_stack[CH_MAX] = {
	[CH_SUBSYSTEM] = 1,
};

/* ----------------- pvt ------------------ */

static const int pvt_type[] = {
	[CH_SUBSYSTEM] = PRV_XTASKS_SUBSYSTEM,
};

static const char *pcf_prefix[CH_MAX] = {
	[CH_SUBSYSTEM] = "FPGA xtasks subsystem",
};

static const struct pcf_value_label xtasks_ss_values[] = {
	{ -1, NULL },
};

static const struct pcf_value_label *pcf_labels[CH_MAX] = {
	[CH_SUBSYSTEM] = xtasks_ss_values,
};

static const long prv_flags[CH_MAX] = {
	[CH_SUBSYSTEM] = 0,
};

static const struct model_pvt_spec pvt_spec = {
	.type = pvt_type,
	.prefix = pcf_prefix,
	.label = pcf_labels,
	.flags = prv_flags,
};

/* ----------------- tracking ------------------ */

static const int th_track[CH_MAX] = {
	[CH_SUBSYSTEM] = TRACK_TH_ANY,
};

/* ----------------- chan_spec ------------------ */

static const struct model_chan_spec th_chan = {
	.nch = CH_MAX,
	.prefix = model_name,
	.ch_names = chan_name,
	.ch_stack = chan_stack,
	.pvt = &pvt_spec,
	.track = th_track,
};

/* ----------------- models ------------------ */

static const struct model_thread_spec th_spec = {
	.size = sizeof(struct xtasks_thread),
	.chan = &th_chan,
	.model = &model_xtasks,
};

/* ----------------------------------------------------- */

int
model_xtasks_probe(struct emu *emu)
{
	if (emu->system.nthreads == 0)
		return 1;

	return 0;
}

int
model_xtasks_create(struct emu *emu)
{
	if (model_thread_create(emu, &th_spec) != 0) {
		err("model_thread_init failed");
		return -1;
	}

	return 0;
}

int
model_xtasks_connect(struct emu *emu)
{
	if (model_thread_connect(emu, &th_spec) != 0) {
		err("model_thread_connect failed");
		return -1;
	}

	return 0;
}
