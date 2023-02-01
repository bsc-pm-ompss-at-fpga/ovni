/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef MODEL_H
#define MODEL_H

#include "emu_hook.h"

struct model_spec {
	char *name;
	int model;
	char *depends;
	emu_hook_t *probe;
	emu_hook_t *create;
	emu_hook_t *connect;
	emu_hook_t *event;
};

#define MAX_MODELS 256

struct model {
	struct model_spec *spec[MAX_MODELS];
	int registered[MAX_MODELS];
};

void model_init(struct model *model);
void model_register(struct model *model, struct model_spec *spec);

int model_probe(struct model *model, struct emu *emu);
int model_create(struct model *model, struct emu *emu);
int model_connect(struct model *model, struct emu *emu);
int model_event(struct model *model, struct emu *emu, int index);

#endif /* MODEL_H */
