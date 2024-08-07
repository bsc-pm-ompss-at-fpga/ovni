/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef EMU_ARGS_H
#define EMU_ARGS_H

struct emu_args {
	int linter_mode;
	int breakdown;
	int enable_all_models;
	char *clock_offset_file;
	char *tracedir;
	char *xtasks_config; // FPGA accelerator information
};

void emu_args_init(struct emu_args *args, int argc, char *argv[]);

#endif /* EMU_ARGS_H */
