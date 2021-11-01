//
// Created by suker on 10/27/2021.
//

#include <sys/types.h>
#include <sys/mman.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#ifndef UNTITLED23_FILE_H
#define UNTITLED23_FILE_H

#endif //UNTITLED23_FILE_H


typedef struct my_file{
    int fd;
    char* ptr;
    ssize_t size;
    struct my_file *next_file;
}my_file;

typedef struct task{
    bool completed;
    my_file* this_file;
    int end;
    int start;
    struct task *next_task;
    char *encoded;
}task;

typedef struct thread_pool{
    task* head;
    task* tail;
    int queue_length;
    int threads_count;
    int pending_task;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t* all_threads;
}thread_pool;

my_file *start_file(my_file *new_file,int fd, ssize_t size);
task* start_task(task *new_task,int start,int end, my_file *file);
thread_pool* start_thread_pool(thread_pool *new_thread_pool,int threads_count,task* head,task* tail, int queue_length);
void number_handler(char* after_encode, int count,int* offset);
static void *work_thread(void *new_thread_pool);