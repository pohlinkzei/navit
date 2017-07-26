/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2009 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */
 
#ifndef THREADS_H
#define THREADS_H

#ifdef CMAKE_USE_PTHREADS_INIT
#include "pthread.h"
#endif

#ifdef CMAKE_USE_PTHREADS_INIT
typedef pthread_t thread_t;
typedef pthread_attr_t thread_attr_t;
typedef pthread_once_t thread_once_t;
typedef pthread_mutex_t thread_mutex_t;
typedef pthread_mutexattr_t thread_mutexattr_t;
#else
typedef void* thread_t;
typedef void* thread_attr_t;
typedef int thread_once_t;
typedef void* thread_mutex_t;
typedef void* thread_mutexattr_t;
#endif

extern int thread_create (thread_t *__restrict __newthread,
			   const thread_attr_t *__restrict __attr,
			   void *(*__start_routine) (void *),
			   void *__restrict __arg);
extern void thread_exit (void *__retval);
extern int thread_join (thread_t __th, void **__thread_return);
extern int thread_detach (thread_t __th);
extern thread_t thread_self (void);
extern int pthread_equal (thread_t __thread1, thread_t __thread2);
extern int thread_once (thread_once_t *__once_control,
			 void (*__init_routine) (void));
extern int thread_setcancelstate (int __state, int *__oldstate);
extern int thread_setcanceltype (int __type, int *__oldtype);
extern int thread_cancel (thread_t __th);
extern void thread_testcancel (void);

extern int thread_mutex_init (thread_mutex_t *__mutex,
			       const thread_mutexattr_t *__mutexattr);
extern int thread_mutex_destroy (thread_mutex_t *__mutex);
extern int thread_mutex_trylock (thread_mutex_t *__mutex);
extern int thread_mutex_lock (thread_mutex_t *__mutex);
extern int thread_mutex_unlock (thread_mutex_t *__mutex);
extern int thread_mutex_getprioceiling (const thread_mutex_t *__mutex,
										int * __prioceiling);
extern int thread_mutex_setprioceiling (thread_mutex_t * __mutex,
					 int __prioceiling,
					 int * __old_ceiling);
#endif

