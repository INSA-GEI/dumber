/**
 ******************************************************************************
 * @file      syscalls.c
 * @author    Auto-generated by STM32CubeIDE
 * @brief     STM32CubeIDE Minimal System calls file
 *
 *            For more information about which c-functions
 *            need which of these lowlevel functions
 *            please consult the Newlib libc-manual
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes */
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#include "cmsis_os.h"

#include "panic.h"

/* Variables */
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

extern uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

char *__env[1] = { 0 };
char **environ = __env;

uint32_t Counter_Malloc=0;
uint32_t Counter_Free=0;

/* Functions */
void* malloc(size_t size)
{
	void* ptr = NULL;

	if(size > 0)
	{
		// We simply wrap the FreeRTOS call into a standard form
		ptr = pvPortMalloc(size);
		Counter_Malloc++;

		if (ptr==NULL) { /* plus assez de memoire dynamique*/
			PANIC_Raise(panic_malloc);
		}
	} // else NULL if there was an error

	return ptr;
}

void free(void* ptr)
{
	if (ptr)
	{
		if ((ptr>=(void*)ucHeap) && (ptr<=(void*)ucHeap+configTOTAL_HEAP_SIZE)) {
			// We simply wrap the FreeRTOS call into a standard form
			vPortFree(ptr);
			Counter_Free++;
		}
	}
}

void initialise_monitor_handles()
{
}

int _getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

void _exit (int status)
{
	_kill(status, -1);
	while (1) {}		/* Make sure we hang here */
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		*ptr++ = __io_getchar();
	}

	return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		__io_putchar(*ptr++);
	}
	return len;
}

int _close(int file)
{
	return -1;
}


int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _open(char *path, int flags, ...)
{
	/* Pretend like we always fail */
	return -1;
}

int _wait(int *status)
{
	errno = ECHILD;
	return -1;
}

int _unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

int _times(struct tms *buf)
{
	return -1;
}

int _stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

int _fork(void)
{
	errno = EAGAIN;
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}
