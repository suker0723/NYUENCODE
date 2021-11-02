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
    new_task->next_task=NULL;
    new_task->encoded=malloc((end-start+1)*sizeof (char));
    return new_task;
}

thread_pool* start_thread_pool(thread_pool *new_thread_pool,int threads_count,task* head, int queue_length){
    if(threads_count==0){
        threads_count=1; // sequentially;
    }else if(head==NULL||queue_length==0){
        return NULL;
    }
    new_thread_pool= malloc(sizeof(thread_pool));
    new_thread_pool->queue_length=queue_length;
    new_thread_pool->head=head;
    new_thread_pool->all_threads= malloc(sizeof(pthread_t)*threads_count);
    new_thread_pool->pending_task=queue_length;
    new_thread_pool->threads_count=threads_count;
    new_thread_pool->status=0;
    pthread_mutex_init(&(new_thread_pool->mutex),NULL);
    pthread_cond_init(&(new_thread_pool->cond),NULL);
    for(int i=0;i<threads_count;i++){
        pthread_create(&(new_thread_pool->all_threads[0]),NULL,work_thread,new_thread_pool);
        new_thread_pool->threads_count--;
    }
    new_thread_pool->status=1;
    return new_thread_pool;
}


static void *work_thread(void *new_thread_pool){
    thread_pool *wrapper=(thread_pool*) new_thread_pool;
    while(true){
        pthread_mutex_lock(&(wrapper->mutex));
        if(wrapper->threads_count==0){
            break;
        }
        if(wrapper->head==NULL){
            break;
        }
     //   encode(wrapper->head);
        wrapper->head->completed=true;

        wrapper->head=wrapper->head->next_task;

        wrapper->pending_task--;
        pthread_mutex_unlock(&(wrapper->mutex));
    }

    pthread_mutex_unlock(&(wrapper->mutex));
    pthread_exit(NULL);
}
