/* Copyright (c) 2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

/* Spawn a task from an external thread that calls some nanos6
 * functions. The external thread must be paused when the task pauses
 * the execution. This emulates the same behavior as TAMPI when using a
 * polling task. */

#define _DEFAULT_SOURCE

#include <nanos6.h>
#include <nanos6/library-mode.h>
#include <time.h>
#include <pthread.h>

#include "common.h"

static double
get_time_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double) ts.tv_sec + (double) ts.tv_nsec * 1.0e-9;
}

static void
dummy_work(double ms)
{
	double end = get_time_ms() + ms * 1e-3;
	while (get_time_ms() < end);
}

static void
polling_func(void *arg)
{
	double ms = *((double *) arg);
	double end = get_time_ms() + ms * 1e-3;
	while (get_time_ms() < end) {
		dummy_work(1.0); /* 1 ms */
		nanos6_wait_for(1000UL); /* 1 ms */
	}
}

/* Call the nanos6_spawn_function from an external thread */
static void *
spawn(void *arg)
{
	double ms = *((double *) arg);
	nanos6_spawn_function(polling_func, &ms, NULL, NULL, "polling_task");
	return NULL;
}

int
main(void)
{
	pthread_t th;
	double T = 100.0;

	if (pthread_create(&th, NULL, spawn, &T) != 0)
		die("pthread_create failed:");

	if (pthread_join(th, NULL) != 0)
		die("pthread_join failed:");

	#pragma oss task label("dummy_task")
	dummy_work(T);

	#pragma oss taskwait

	return 0;
}