/* Copyright (c) 2021 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#include "uthash.h"

#include "chan.h"
#include "emu.h"
#include "ovni.h"
#include "prv.h"
#include "trace.h"

/* --------------------------- init ------------------------------- */

void
hook_init_kernel(struct ovni_emu *emu)
{
	struct ovni_ethread *th;
	struct ovni_cpu *cpu;
	size_t i;
	int row;
	FILE *prv_th, *prv_cpu;
	int64_t *clock;
	struct ovni_chan **uth, **ucpu;

	clock = &emu->delta_time;
	prv_th = emu->prv_thread;
	prv_cpu = emu->prv_cpu;

	/* Init the channels in all threads */
	for (i = 0; i < emu->total_nthreads; i++) {
		th = emu->global_thread[i];
		row = th->gindex + 1;
		uth = &emu->th_chan;

		chan_th_init(th, uth, CHAN_KERNEL_CS, CHAN_TRACK_NONE, 0, 1, 1, row, prv_th, clock);
	}

	/* Init the channels in all cpus */
	for (i = 0; i < emu->total_ncpus; i++) {
		cpu = emu->global_cpu[i];
		row = cpu->gindex + 1;
		ucpu = &emu->cpu_chan;

		chan_cpu_init(cpu, ucpu, CHAN_KERNEL_CS, CHAN_TRACK_TH_ACTIVE, 0, 0, 1, row, prv_cpu, clock);
	}
}

/* --------------------------- pre ------------------------------- */

static void
context_switch(struct ovni_emu *emu)
{
	struct ovni_ethread *th;
	struct ovni_chan *chan;

	th = emu->cur_thread;
	chan = &th->chan[CHAN_KERNEL_CS];

	switch (emu->cur_ev->header.value) {
		case 'O':
			chan_push(chan, ST_KERNEL_CSOUT);
			break;
		case 'I':
			chan_pop(chan, ST_KERNEL_CSOUT);
			break;
		default:
			edie(emu, "unexpected value '%c' (expecting 'O' or 'I')\n",
				emu->cur_ev->header.value);
	}
}

void
hook_pre_kernel(struct ovni_emu *emu)
{
	if (emu->cur_ev->header.model != 'K')
		edie(emu, "hook_pre_kernel: unexpected event with model %c\n",
			emu->cur_ev->header.model);

	switch (emu->cur_ev->header.category) {
		case 'C':
			context_switch(emu);
			break;
		default:
			edie(emu, "hook_pre_kernel: unexpected event with category %c\n",
				emu->cur_ev->header.category);
	}
}
