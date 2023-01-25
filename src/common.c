/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: MIT */

#include "common.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

char *progname = NULL;

void
progname_set(char *name)
{
	progname = name;
}

static void
vaerr(const char *func, const char *errstr, va_list ap)
{
	if (progname != NULL)
		fprintf(stderr, "%s: ", progname);

	if (func != NULL)
		fprintf(stderr, "%s: ", func);

	vfprintf(stderr, errstr, ap);

	int len = strlen(errstr);

	if (len > 0) {
		char last = errstr[len - 1];
		if (last == ':')
			fprintf(stderr, " %s\n", strerror(errno));
		else if (last != '\n')
			fprintf(stderr, "\n");
	}
}

void
verr(const char *func, const char *errstr, ...)
{
	va_list ap;
	va_start(ap, errstr);
	vaerr(func, errstr, ap);
	va_end(ap);
}


void
vdie(const char *func, const char *errstr, ...)
{
	va_list ap;
	va_start(ap, errstr);
	vaerr(func, errstr, ap);
	va_end(ap);
	abort();
}