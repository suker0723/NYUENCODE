//
// Created by suker on 10/27/2021.
//


#include "file.h"

my_file *start_file(my_file *new_file, int fd, ssize_t size) {
    new_file = malloc(sizeof(my_file));
    new_file->fd = fd;
    new_file->size = size;
    new_file->ptr = mmap(NULL, size,
                         PROT_READ,
                         MAP_SHARED,
                         fd, 0);

    new_file->next_file = NULL;
    return new_file;
}

task *start_task(task *new_task, int start, int end, my_file *file) {
    new_task = malloc(sizeof(task));
    new_task->start = start;
    new_task->end = end;
    new_task->this_file = file;
    new_task->being_processed = false;
    new_task->next_task = NULL;
    new_task->encoded = malloc((end - start + 1) * sizeof(char));
    return new_task;
}

thread_pool *start_thread_pool(thread_pool *new_thread_pool, int threads_count, task *head) {
    if (threads_count == 0) {
        threads_count = 1; // sequentially;
    }
    new_thread_pool = malloc(sizeof(thread_pool));
    new_thread_pool->queue_length = 0;
    new_thread_pool->head = head;
    new_thread_pool->tail = NULL;
    new_thread_pool->current = NULL;
    new_thread_pool->all_threads = malloc(sizeof(pthread_t) * threads_count);
    new_thread_pool->pending_task = 0;
    new_thread_pool->threads_count = 0;
    pthread_mutex_init(&(new_thread_pool->mutex), NULL);
    pthread_cond_init(&(new_thread_pool->cond), NULL);
    for (int i = 0; i < threads_count; i++) {
        pthread_create(&(new_thread_pool->all_threads[i]), NULL, work_thread, new_thread_pool);
        new_thread_pool->threads_count++;
    }
    new_thread_pool->status = 1;
    return new_thread_pool;
}

int add_task_to_thread(thread_pool *cur_pool, task *new_task) {
    if (cur_pool == NULL || cur_pool->status != 1) {
        return -1; // not init pool
    }
    if (pthread_mutex_lock(&(cur_pool->mutex)) != 0) {
        return -1;
    }

    if (cur_pool->head == NULL) {
        cur_pool->head = new_task;
        cur_pool->tail = new_task;
        cur_pool->current = new_task;
    } else if (cur_pool->head->next_task == NULL) {
        cur_pool->head->next_task = new_task;
        cur_pool->tail = new_task;
    } else {
        cur_pool->tail->next_task = new_task;
        cur_pool->tail = cur_pool->tail->next_task;
    }
    cur_pool->pending_task += 1;
    cur_pool->queue_length += 1;


    if (cur_pool->status != 1) {
        return -1;
    }

    if (pthread_cond_signal(&(cur_pool->cond)) != 0) {
        return -1;
    }

    if (pthread_mutex_unlock(&(cur_pool->mutex)) != 0) {
        return -1;
    }

    return 0;
}


int destroy_thread_pool(thread_pool *cur_pool) {
    if (cur_pool == NULL) {
        return -1;
    }
    if (pthread_mutex_lock(&(cur_pool->mutex)) != 0) {
        return -1; // some thread still running.
    }

        if (cur_pool->status != 1) {
            return -1;
        }
        pthread_cond_broadcast(&(cur_pool->cond));
        pthread_mutex_unlock(&(cur_pool->mutex));
        for (int i = 0; i < cur_pool->threads_count; i++) {
            pthread_join(cur_pool->all_threads[i], NULL);
        }

    cur_pool->status = -1;
    return 0;
}

static void *work_thread(void *new_thread_pool) {
    thread_pool *wrapper = (thread_pool *) new_thread_pool;
    while (true) {
        pthread_mutex_lock(&(wrapper->mutex));
        while ((wrapper->pending_task == 0 || wrapper->current == NULL) && (wrapper->status == 1)) {
            pthread_cond_wait(&(wrapper->cond), &(wrapper->mutex));
        }

        if (wrapper->status != 1) {
            break;
        }
        encode(wrapper->current);

        wrapper->current->being_processed = true;
        wrapper->current = wrapper->current->next_task;

        wrapper->pending_task--;
        pthread_mutex_unlock(&(wrapper->mutex));
    }
    (wrapper->threads_count)--;
    pthread_mutex_unlock(&(wrapper->mutex));
    pthread_exit(NULL);
}