/* Copyright (c) 2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#include "instr_nanos6.h"

#include "emu_prv.h"

int
main(void)
{
	/* Tests that switching to a nested task of the same type produces
	 * duplicated events in the Paraver trace with the task type and task
	 * rank. */

	instr_start(0, 1);

	uint32_t typeid = 100;
	uint32_t gid = instr_nanos6_type_create(typeid);

	/* Create two tasks of the same type */
	instr_nanos6_task_create(1, typeid);
	instr_nanos6_task_create(2, typeid);

	/* Change subsystem to prevent duplicates */
	instr_nanos6_task_body_enter();
	instr_nanos6_task_execute(1);
	instr_nanos6_submit_task_enter();
	instr_nanos6_task_body_enter();
	instr_nanos6_task_execute(2);

	/* Match the PRV line in the trace */
	FILE *f = fopen("match.sh", "w");
	if (f == NULL)
		die("fopen failed:");

	/* Check the task type */
	int prvtype = PRV_NANOS6_TYPE;
	int64_t t = get_delta();
	fprintf(f, "grep ':%ld:%d:%d$' ovni/thread.prv\n", t, prvtype, gid);
	fprintf(f, "grep ':%ld:%d:%d$' ovni/cpu.prv\n", t, prvtype, gid);

	/* Check the rank */
	prvtype = PRV_NANOS6_RANK;
	int rank = 0 + 1; /* Starts at one */
	fprintf(f, "grep ':%ld:%d:%d$' ovni/thread.prv\n", t, prvtype, rank);
	fprintf(f, "grep ':%ld:%d:%d$' ovni/cpu.prv\n", t, prvtype, rank);
	fclose(f);

	/* Exit from tasks and subsystem */
	instr_nanos6_task_end(2);
	instr_nanos6_task_body_exit();
	instr_nanos6_submit_task_exit();
	instr_nanos6_task_end(1);
	instr_nanos6_task_body_exit();

	instr_end();

	return 0;
}