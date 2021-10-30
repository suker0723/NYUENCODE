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
}task;

my_file *start_file(my_file *new_file,int fd, ssize_t size);
task* start_task(task *new_task,int start,int end, my_file *file);