//
// Created by suker on 10/27/2021.
//

#include "file.h"

my_file *start_file(my_file *new_file,int fd, ssize_t size){
    new_file=malloc(sizeof(my_file));
    new_file->fd=fd;
    new_file->size=size;
    new_file->ptr=mmap(NULL,size,
                       PROT_READ,
                       MAP_SHARED,
                       fd,0);

    new_file->next_file=NULL;
    return new_file;
}

task* start_task(task *new_task,int start,int end, my_file *file){
    new_task= malloc(sizeof(task));
    new_task->start=start;
    new_task->end=end;
    new_task->this_file=file;
    new_task->completed=false;
    return new_task;
}