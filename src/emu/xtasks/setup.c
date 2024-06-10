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
#include "ev_spec.h"

static const char model_name[] = "xtasks";
enum { model_id = 'X' };

static struct ev_decl model_evlist[] = {
	{ "Xse", "All events in xtasks" },
	{ NULL, NULL},
};

struct model_spec model_xtasks = {
	.name = model_name,
	.version = "1.0.0",
	.evlist  = model_evlist,
	.model = model_id,
	.create  = model_xtasks_create,
//	.connect = model_xtasks_connect,
	.event   = model_xtasks_event,
	.probe   = model_xtasks_probe,
	.finish  = model_xtasks_finish,
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
#define API_CALL(val_) val_
static const struct pcf_value_label xtasks_ss_values[] = {
	{ API_CALL(1), "Waiting for tasks" },
	{ API_CALL(2), "Seting lock" },
	{ API_CALL(3), "Unseting lock" },
	{ API_CALL(5), "Creating task" },
	{ 78, "Copying in localmem" },
	{ 79, "Copying out localmem" },
	{ 80, "Running user task" },
	{ 6660, "Reverse Offload user task" },
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
	return model_version_probe(&model_xtasks, emu);
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
model_xtasks_finish(struct emu *emu)
{
	/* Skip the check if the we are stopping prematurely */
	if (!emu->finished)
		return 0;

	int ret = 0;

	// We don't check FPGA thread are dead

	return ret;
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
