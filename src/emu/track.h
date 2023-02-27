/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef TRACK_H
#define TRACK_H

#include "mux.h"

enum track_type {
	TRACK_TYPE_TH = 0,
	TRACK_TYPE_MAX
};

enum track_th {
	TRACK_TH_ANY = 0,
	TRACK_TH_RUN,
	TRACK_TH_ACT,
	TRACK_TH_MAX,
};

struct track {
	enum track_type type;
	int mode;
	char name[MAX_CHAN_NAME];
	struct bay *bay;
	struct chan ch;
	struct chan *out;
	struct mux mux;
};

USE_RET int track_init(struct track *track, struct bay *bay, enum track_type type, int mode, const char *fmt, ...);
USE_RET int track_set_select(struct track *track, struct chan *sel, mux_select_func_t fsel, int64_t ninputs);
USE_RET int track_set_input(struct track *track, int64_t index, struct chan *inp);
USE_RET struct chan *track_get_output(struct track *track);
USE_RET int track_connect_thread(struct track *tracks, struct chan *chans, struct chan *sel, int n);

#endif /* TRACK_H */
