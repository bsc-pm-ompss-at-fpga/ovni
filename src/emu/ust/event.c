/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#define ENABLE_DEBUG

#include "ovni_model.h"
#include "emu.h"
#include "loom.h"
#include "common.h"

static int
pre_thread_execute(struct emu *emu, struct thread *th)
{
	/* The thread cannot be already running */
	if (th->state == TH_ST_RUNNING) {
		err("cannot execute thread %d, is already running", th->tid);
		return -1;
	}

	int cpuid = emu->ev->payload->i32[0];
	struct cpu *cpu = loom_find_cpu(emu->loom, cpuid);

	if (cpu == NULL) {
		err("cannot find CPU with phyid %d in loom %s",
				cpuid, emu->loom->id)
		return -1;
	}

	dbg("thread %d runs in %s", th->tid, cpu->name);

	/* First set the CPU in the thread */
	thread_set_cpu(th, cpu);

	/* Then set the thread to running state */
	thread_set_state(th, TH_ST_RUNNING);

	/* And then add the thread to the CPU, so tracking channels see the
	 * updated thread state */
	cpu_add_thread(cpu, th);

	return 0;
}

static int
pre_thread_end(struct thread *th)
{
	if (th->state != TH_ST_RUNNING && th->state != TH_ST_COOLING) {
		err("cannot end thread %d: state not running or cooling",
				th->tid);
		return -1;
	}

	if (thread_set_state(th, TH_ST_DEAD) != 0) {
		err("cannot set thread %d state", th->tid);
		return -1;
	}

	if (cpu_remove_thread(th->cpu, th) != 0) {
		err("cannot remove thread %d from %s",
				th->tid, th->cpu->name);
		return -1;
	}

	if (thread_unset_cpu(th) != 0) {
		err("cannot unset cpu from thread %d", th->tid);
		return -1;
	}

	return 0;
}

static int
pre_thread_pause(struct thread *th)
{
	if (th->state != TH_ST_RUNNING && th->state != TH_ST_COOLING) {
		err("cannot pause thread %d: state not running or cooling\n",
				th->tid);
		return -1;
	}

	if (thread_set_state(th, TH_ST_PAUSED) != 0) {
		err("cannot set thread %d state", th->tid);
		return -1;
	}

	if (cpu_update(th->cpu) != 0) {
		err("cpu_update failed for %s", th->cpu->name);
		return -1;
	}

	return 0;
}

static int
pre_thread_resume(struct thread *th)
{
	if (th->state != TH_ST_PAUSED && th->state != TH_ST_WARMING) {
		err("cannot resume thread %d: state not paused or warming",
				th->tid);
		return -1;
	}

	if (thread_set_state(th, TH_ST_RUNNING) != 0) {
		err("cannot set thread %d state", th->tid);
		return -1;
	}

	if (cpu_update(th->cpu) != 0) {
		err("cpu_update failed for %s", th->cpu->name);
		return -1;
	}

	return 0;
}

static int
pre_thread_cool(struct thread *th)
{
	if (th->state != TH_ST_RUNNING) {
		err("cannot cool thread %d: state not running", th->tid);
		return -1;
	}

	if (thread_set_state(th, TH_ST_COOLING) != 0) {
		err("cannot set thread %d state", th->tid);
		return -1;
	}

	if (cpu_update(th->cpu) != 0) {
		err("cpu_update failed for %s", th->cpu->name);
		return -1;
	}

	return 0;
}

static int
pre_thread_warm(struct thread *th)
{
	if (th->state != TH_ST_PAUSED) {
		err("cannot warm thread %d: state not paused\n", th->tid);
		return -1;
	}

	if (thread_set_state(th, TH_ST_WARMING) != 0) {
		err("cannot set thread %d state", th->tid);
		return -1;
	}

	if (cpu_update(th->cpu) != 0) {
		err("cpu_update failed for %s", th->cpu->name);
		return -1;
	}

	return 0;
}

static int
pre_thread(struct emu *emu)
{
	struct thread *th = emu->thread;
	struct emu_ev *ev = emu->ev;

	switch (ev->v) {
		case 'C': /* create */
			dbg("thread %d creates a new thread at cpu=%d with args=%x %x\n",
					th->tid,
					ev->payload->u32[0],
					ev->payload->u32[1],
					ev->payload->u32[2]);

			break;
		case 'x':
			return pre_thread_execute(emu, th);
		case 'e':
			return pre_thread_end(th);
		case 'p':
			return pre_thread_pause(th);
		case 'r':
			return pre_thread_resume(th);
		case 'c':
			return pre_thread_cool(th);
		case 'w':
			return pre_thread_warm(th);
		default:
			err("unknown thread event value %c\n", ev->v);
			return -1;
	}
	return 0;
}

static int
pre_affinity_set(struct emu *emu)
{
	struct thread *th = emu->thread;

	if (th->cpu == NULL) {
		err("thread %d doesn't have CPU set", th->tid);
		return -1;
	}

	if (!th->is_active) {
		err("thread %d is not active", th->tid);
		return -1;
	}

	/* Migrate current cpu to the one at phyid */
	int phyid = emu->ev->payload->i32[0];
	struct cpu *newcpu = loom_find_cpu(emu->loom, phyid);

	if (newcpu == NULL) {
		err("cannot find cpu with phyid %d", phyid);
		return -1;
	}

	/* The CPU is already properly set, return */
	if (th->cpu == newcpu)
		return 0;

	if (cpu_migrate_thread(th->cpu, th, newcpu) != 0) {
		err("cpu_migrate_thread() failed");
		return -1;
	}

	if (thread_migrate_cpu(th, newcpu) != 0) {
		err("thread_migrate_cpu() failed");
		return -1;
	}

	dbg("thread %d now runs in %s\n", th->tid, newcpu->name);

	return 0;
}

static int
pre_affinity_remote(struct emu *emu)
{
	int32_t phyid = emu->ev->payload->i32[0];
	int32_t tid = emu->ev->payload->i32[1];

	struct thread *remote_th = proc_find_thread(emu->proc, tid);

	/* Search the thread in other processes of the loom if
	 * not found in the current one */
	if (remote_th == NULL)
		remote_th = loom_find_thread(emu->loom, tid);

	if (remote_th == NULL) {
		err("thread %d not found", tid);
		return -1;
	}

	/* The remote_th cannot be in states dead or unknown */
	if (remote_th->state == TH_ST_DEAD) {
		err("thread %d is dead", tid);
		return -1;
	}

	if (remote_th->state == TH_ST_UNKNOWN) {
		err("thread %d in state unknown", tid);
		return -1;
	}

	/* It must have an assigned CPU */
	if (remote_th->cpu == NULL) {
		err("thread %d has no CPU", tid);
		return -1;
	}

	/* Migrate current cpu to the one at phyid */
	struct cpu *newcpu = loom_find_cpu(emu->loom, phyid);
	if (newcpu == NULL) {
		err("cannot find CPU with phyid %d", phyid);
		return -1;
	}

	if (cpu_migrate_thread(remote_th->cpu, remote_th, newcpu) != 0) {
		err("cpu_migrate_thread() failed");
		return -1;
	}

	if (thread_migrate_cpu(remote_th, newcpu) != 0) {
		err("thread_migrate_cpu() failed");
		return -1;
	}

	dbg("remote_th %d remotely switches to cpu %d", tid, phyid);

	return 0;
}

static int
pre_affinity(struct emu *emu)
{
	switch (emu->ev->v) {
		case 's':
			return pre_affinity_set(emu);
		case 'r':
			return pre_affinity_remote(emu);
		default:
			err("unknown affinity event value %c\n",
					emu->ev->v);
//			return -1
	}

	return 0;
}

//static int
//compare_int64(const void *a, const void *b)
//{
//	int64_t aa = *(const int64_t *) a;
//	int64_t bb = *(const int64_t *) b;
//
//	if (aa < bb)
//		return -1;
//	else if (aa > bb)
//		return +1;
//	else
//		return 0;
//}
//
//static void
//pre_burst(struct emu *emu)
//{
//	int64_t dt = 0;
//
//	UNUSED(dt);
//
//	struct emu_thread *th = emu->cur_thread;
//
//	if (th->nbursts >= MAX_BURSTS) {
//		err("too many bursts: ignored\n");
//		return;
//	}
//
//	th->burst_time[th->nbursts++] = emu->delta_time;
//	if (th->nbursts == MAX_BURSTS) {
//		int n = MAX_BURSTS - 1;
//		int64_t deltas[MAX_BURSTS - 1];
//		for (int i = 0; i < n; i++) {
//			deltas[i] = th->burst_time[i + 1] - th->burst_time[i];
//		}
//
//		qsort(deltas, n, sizeof(int64_t), compare_int64);
//
//		double avg = 0.0;
//		double maxdelta = 0;
//		for (int i = 0; i < n; i++) {
//			if (deltas[i] > maxdelta)
//				maxdelta = deltas[i];
//			avg += deltas[i];
//		}
//
//		avg /= (double) n;
//		double median = deltas[n / 2];
//
//		err("%s burst stats: median %.0f ns, avg %.1f ns, max %.0f ns\n",
//				emu->cur_loom->dname, median, avg, maxdelta);
//
//		th->nbursts = 0;
//	}
//}
//
//static void
//pre_flush(struct emu *emu)
//{
//	struct emu_thread *th = emu->cur_thread;
//	struct ovni_chan *chan_th = &th->chan[CHAN_OVNI_FLUSH];
//
//	switch (emu->ev->v) {
//		case '[':
//			chan_push(chan_th, ST_OVNI_FLUSHING);
//			break;
//		case ']':
//			chan_pop(chan_th, ST_OVNI_FLUSHING);
//			break;
//		default:
//			err("unexpected value '%c' (expecting '[' or ']')\n",
//					emu->ev->v);
//			abort();
//	}
//}

static int
hook_pre_ovni(struct emu *emu)
{
	if (emu->ev->m != 'O')
		return -1;

	switch (emu->ev->c) {
		case 'H':
			return pre_thread(emu);
		case 'A':
			return pre_affinity(emu);
//		case 'B':
//			pre_burst(emu);
//			break;
//		case 'F':
//			pre_flush(emu);
//			break;
		default:
			err("unknown ovni event category %c\n",
					emu->ev->c);
//			return -1;
	}

	return 0;
}

int
ust_model_probe(void *p)
{
	struct emu *emu = emu_get(p);

	if (emu->system.nthreads == 0)
		return -1;

	return 0;
}

int
ust_model_event(void *ptr)
{
	struct emu *emu = emu_get(ptr);
	if (emu->ev->m != 'O') {
		err("unexpected event model %c\n", emu->ev->m);
		return -1;
	}

	err("got ovni event '%s'\n", emu->ev->mcv);
	if (hook_pre_ovni(emu) != 0) {
		err("ovni_model_event: failed to process event\n");
		return -1;
	}

	return 0;
}

struct model_spec ust_model_spec = {
	.name = "ust",
	.model = 'O',
	.create  = NULL,
	.connect = NULL,
	.event   = ust_model_event,
	.probe   = ust_model_probe,
};
