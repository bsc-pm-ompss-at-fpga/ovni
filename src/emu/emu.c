/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#define _POSIX_C_SOURCE 2

#include "emu.h"

#include <unistd.h>
#include "ovni/ovni_model.h"

int
emu_init(struct emu *emu, int argc, char *argv[])
{
	memset(emu, 0, sizeof(*emu));

	emu_args_init(&emu->args, argc, argv);

	/* Load the streams into the trace */
	if (emu_trace_load(&emu->trace, emu->args.tracedir) != 0) {
		err("emu_init: cannot load trace '%s'\n",
				emu->args.tracedir);
		return -1;
	}

	/* Parse the trace and build the emu_system */
	if (emu_system_init(&emu->system, &emu->args, &emu->trace) != 0) {
		err("emu_init: cannot init system for trace '%s'\n",
				emu->args.tracedir);
		return -1;
	}

	if (emu_player_init(&emu->player, &emu->trace) != 0) {
		err("emu_init: cannot init player for trace '%s'\n",
				emu->args.tracedir);
		return -1;
	}

	/* Initialize the bay */
	bay_init(&emu->bay);

	/* Register all the models */
	emu_model_register(&emu->model, &ovni_model_spec, emu);

	if (ovni_model_spec.probe(emu) != 0) {
		err("emu_init: ovni probe failed\n");
		return -1;
	}

	if (ovni_model_spec.create(emu) != 0) {
		err("emu_init: ovni create failed\n");
		return -1;
	}

	if (ovni_model_spec.connect(emu) != 0) {
		err("emu_init: ovni connect failed\n");
		return -1;
	}

	return 0;
}

int
emu_step(struct emu *emu)
{
	int ret = emu_player_step(&emu->player);

	/* No more events */
	if (ret > 0)
		return +1;

	/* Error happened */
	if (ret < 0) {
		err("emu_step: emu_player_step failed\n");
		return -1;
	}

	emu->ev = emu_player_ev(&emu->player);
	emu->stream = emu_player_stream(&emu->player);
	emu->thread = emu_system_get_thread(emu->stream);
	emu->proc = emu->thread->proc;
	emu->loom = emu->proc->loom;

	/* Otherwise progress */
	if (ovni_model_spec.event(emu) != 0) {
		err("emu_init: ovni event failed\n");
		return -1;
	}

	return 0;
}
