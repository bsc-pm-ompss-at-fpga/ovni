/*
 * MIT License
 * 
 * Copyright (c) 2022 Barcelona Supercomputing Center (BSC)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef COMPAT_H
#define COMPAT_H

#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>

/* Define gettid for older glibc versions (below 2.30) */
#if defined(__GLIBC__)
#if !__GLIBC_PREREQ(2, 30)
static inline pid_t gettid(void)
{
	return (pid_t)syscall(SYS_gettid);
}
#endif /* !__GLIBC_PREREQ(2, 30) */
#endif /* defined(__GLIBC__) */

#endif // COMPAT_H
