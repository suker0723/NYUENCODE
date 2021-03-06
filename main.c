#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <ctype.h>

#include "file.h"

#define TRUNK_SIZE 10

int merge_tail(task* next,char** result ){

    int cur_position=0;
    unsigned char j,k;
    j=(*result)[(int)strlen(*result)-1];
    k=(next->encoded[1]);
    cur_position=(int)strlen(*result)-1;
    if((*result)[(int)strlen(*result)-2]!=next->encoded[0]){
        strcat(*result,next->encoded);
        return 0;
    }else{
        number_handler(*result,(int)(j+k),&cur_position);
        (*result)[cur_position]='\0';
        strcat(*result,next->encoded+2);
        return 2;
    }

}

void make_task_queue(my_file *input_head, thread_pool* cur_pool){
    my_file *cur=input_head;
    while(cur!=NULL){
        int start=0;
        while(start<strlen(input_head->ptr)){
            task* new_task= malloc(sizeof(task));
            if((start+TRUNK_SIZE)<strlen(input_head->ptr)){
                new_task=start_task(new_task,start,start+TRUNK_SIZE,cur);
                start+=TRUNK_SIZE;
            }else{
                new_task= start_task(new_task,start,strlen(input_head->ptr),cur);
                start+=TRUNK_SIZE;
            }

            add_task_to_thread(cur_pool, new_task);
        }
        cur=cur->next_file;
        close(input_head->fd);
    }

}

void number_handler(char* after_encode, int count,int* offset){

    after_encode[(*offset)++]=(unsigned char)count;
}
int encode(task* cur_task){
    int count=0;

    int offset=0;
    for(int i=cur_task->start;i<=cur_task->end;i++){
        if(cur_task->this_file->ptr[i]=='\0'|| i==cur_task->end){
            cur_task->encoded[offset++]=cur_task->this_file->ptr[i-1];
            number_handler(cur_task->encoded,count,&offset);
        }else if(i==cur_task->start){
            count+=1;
        }else{
          if(cur_task->this_file->ptr[i]==cur_task->this_file->ptr[i-1]){
              count+=1;
          }else{
              cur_task->encoded[offset++]=cur_task->this_file->ptr[i-1];
              number_handler(cur_task->encoded,count,&offset);
              count=1;
          }
        }
    }
    cur_task->encoded[offset]='\0';
    return offset;
}

int main(int argc, char *const argv[]) {
    int opt;
    bool input=true;
    bool threads_jobs=false;
    int threads_count=0;
    int start=0;
    ssize_t input_size=0;
    my_file *input_head=NULL;
    my_file *input_tail=NULL;
    my_file *output=NULL;
    while((opt= getopt(argc,argv,"j>"))!=-1){
        if(opt=='j'){
            threads_jobs=true;
            break;
        }
    }
    if(threads_jobs==true){
        threads_count=atoi(argv[2]);
        start=2;
    }
    while(++start<argc){
        if(strcmp(argv[start],">")==0){
            input=false;
            continue;
        }

        if(input){
            int fd = open(argv[start], O_RDONLY);
            if(fd < 0){
                fprintf(stderr,"\n\"%s \" could not open\n",
                        argv[start]);
                exit(1);
            }
            struct stat statbuf;
            int err = fstat(fd, &statbuf);
            if(err < 0){
                fprintf(stderr,"\n\"%s \" could not open\n",
                        argv[start]);
                exit(1);
            }
            if(input_head==NULL){
                input_head=start_file(input_head,fd,statbuf.st_size);
                input_tail=input_head;
            }else{
                my_file *new_file=NULL;
                new_file=start_file(new_file,fd,statbuf.st_size);
                input_tail->next_file=new_file;
                input_tail=input_tail->next_file;
            }
            input_size+=input_tail->size;
        }else{
            int fd = open(argv[start], O_WRONLY|O_CREAT);
            if(fd < 0){
                fprintf(stderr,"\n\"%s \" could not open\n",
                        argv[start]);
                exit(1);
            }
            struct stat statbuf;
            int err = fstat(fd, &statbuf);
            if(err < 0){
                fprintf(stderr,"\n\"%s \" could not open\n",
                        argv[start]);
                exit(1);
            }
            output= start_file(output,fd,statbuf.st_size);
            break;
        }
    }
    if(input_head==NULL||strlen(input_head->ptr)==0){
        fprintf(stderr,"\n\" \" empty file\n"
                );
        exit(1);
    }
    thread_pool *cur_pool=NULL;

    cur_pool=start_thread_pool(cur_pool,threads_count,NULL);
    make_task_queue(input_head, cur_pool);
    task* queue_head=cur_pool->head;
//    destroy_thread_pool(cur_pool);
    char* result= malloc((strlen(queue_head->encoded)+TRUNK_SIZE)*sizeof (char));
    strcpy(result,queue_head->encoded);
    while(queue_head->next_task!=NULL){
        merge_tail(queue_head->next_task,&result);
        queue_head=queue_head->next_task;
    }
    write(output->fd,result, strlen(result));


    return 0;
}

