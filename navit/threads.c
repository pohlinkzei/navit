/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2017 Navit Team
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

#include "threads.h"

/* Warpper functions for pthread lib to be used on systems which provide
 * pthread. For systems that don't it will work as standard sequemtial
 * code
 */ 

#ifdef CMAKE_USE_PTHREADS_INIT
int thread_create (thread_t *__restrict __newthread,
			   const thread_attr_t *__restrict __attr,
			   void *(*__start_routine) (void *),
			   void *__restrict __arg){
   return pthread_create(__newthread, __attr, __start_routine, __arg);
}

void thread_exit (void *__retval){
	pthread_exit(__retval);
}

int thread_join (thread_t __th, void **__thread_return);
	return pthread_join (__th, __thread_return);
}

int thread_detach (thread_t __th){
	return pthread_detach(__th);
}

thread_t thread_self (void){
	return pthread_self();
}

int thread_equal (thread_t __thread1, thread_t __thread2){
	return pthread_equal(__thread1, __thread2);
}

int thread_once (thread_once_t *__once_control,
			 void (*__init_routine) (void)){
	 return pthread_once(__once_control, __init_routine);
 }

int thread_setcancelstate (int __state, int *__oldstate){
	return pthread_setcancelstate (__state,__oldstate);
}

int thread_setcanceltype (int __type, int *__oldtype){
	return thread_setcanceltype (__type, *__oldtype);
}

int thread_cancel (thread_t __th){
	return pthread_cancel(__th);
}

void thread_testcancel (void){
	pthread_testcancel();
}

int thread_mutex_init (thread_mutex_t *__mutex,
			       const thread_mutexattr_t *__mutexattr){
	return pthread_mutex_init(__mutex, __mutexattr);
}

int thread_mutex_destroy (thread_mutex_t *__mutex){
	return pthread_mutex_destroy(__mutex);
}

int thread_mutex_trylock (thread_mutex_t *__mutex){
	return pthread_mutex_trylock(__mutex);
}

int thread_mutex_lock (thread_mutex_t *__mutex){
	return pthread_mutex_lock(__mutex);
}

int thread_mutex_unlock (thread_mutex_t *__mutex){
	return pthread_mutex_unlock(__mutex);
}

int thread_mutex_getprioceiling (const thread_mutex_t *__mutex,
										int * __prioceiling){
   return pthread_mutex_getprioceiling(__mutex, __prioceiling);
}

int thread_mutex_setprioceiling (thread_mutex_t * __mutex,
					 int __prioceiling,
					 int * __old_ceiling){
	return pthread_mutex_setprioceiling(__mutex, __prioceiling, __old_ceiling);
}

#else
// we do not have threads
int thread_create (thread_t *__restrict __newthread,
			   const thread_attr_t *__restrict __attr,
			   void *(*__start_routine) (void *),
			   void *__restrict __arg){
   return __start_routine;
}

void thread_exit (void *__retval){
	return;
}

int thread_join (thread_t __th, void **__thread_return){
	return 0;
}
int thread_detach (thread_t __th){
	return 0;
}

thread_t thread_self (void){
	return 0;
}

int thread_equal (thread_t __thread1, thread_t __thread2){
	return 0;
}

int thread_once (thread_once_t *__once_control,
			 void (*__init_routine) (void)){
	return  __init_routine;
}

int thread_setcancelstate (int __state, int *__oldstate){
	return 0;
}

int thread_setcanceltype (int __type, int *__oldtype){
	return 0;
}

int thread_cancel (thread_t __th){
	return 0;
}

void thread_testcancel (void){
	return;
}

int thread_mutex_init (thread_mutex_t *__mutex,
			       const thread_mutexattr_t *__mutexattr){
	return 0;
}

int thread_mutex_destroy (thread_mutex_t *__mutex){
	return 0;
}

int thread_mutex_trylock (thread_mutex_t *__mutex){
	return 0;
}

int thread_mutex_lock (thread_mutex_t *__mutex){
	return 0;
}

int thread_mutex_unlock (thread_mutex_t *__mutex){
	return 0;
}

int thread_mutex_getprioceiling (const thread_mutex_t *__mutex,
										int * __prioceiling){
   return 0;
}

int thread_mutex_setprioceiling (thread_mutex_t * __mutex,
					 int __prioceiling,
					 int * __old_ceiling){
	return 0;
}
#endif
