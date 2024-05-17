/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef EMU_SYSTEM_H
#define EMU_SYSTEM_H

#include <stddef.h>
#include "clkoff.h"
#include "common.h"
struct bay;
struct emu_args;
struct recorder;
struct stream;
struct trace;

#define MAX_ACCELERATORS  128

/* Map from stream to lpt */
struct lpt {
	struct stream *stream; /* Redundancy */
	struct loom *loom;
	struct proc *proc;
	struct thread *thread;
};

struct acc {
	char name[128];
	unsigned int id;
};

struct info_acc{
	struct acc accel[MAX_ACCELERATORS];
	unsigned int n_accelerators;
};


struct system {
	/* Total counters */
	size_t nlooms;
	size_t nthreads;
	size_t nprocs;
	size_t ncpus; /* Including virtual cpus */
	size_t nphycpus;

	int sort_by_rank;

	struct loom *looms;
	struct proc *procs;
	struct thread *threads;
	struct cpu *cpus;

	struct clkoff clkoff;
	struct emu_args *args;

	struct lpt *lpt;
	struct info_acc *acc_info;
};

USE_RET int system_init(struct system *sys, struct emu_args *args, struct trace *trace);
USE_RET int system_connect(struct system *sys, struct bay *bay, struct recorder *rec);
USE_RET struct lpt *system_get_lpt(struct stream *stream);

#endif /* EMU_SYSTEM_H */
