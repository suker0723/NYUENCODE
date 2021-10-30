#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>

#include "file.h"

#define TRUNK_SIZE 10;
char* concatenate_input(my_file *input_head, char** whole_input){
    my_file *cur=input_head;
    while(cur!=NULL){
        strcat(*whole_input,input_head->ptr);
        cur=cur->next_file;
        close(input_head->fd);
    }
    return *whole_input;
}

void number_handler(char* after_encode, int count,int* offset){
    char* reverse= malloc(sizeof (char)*10);
    int index=0;
    while(count>0){
        char digit=(char)('0'+(count%10));
        count/=10;
        reverse[index++]=digit;
    }
    while(--index>=0){
        after_encode[(*offset)++]=reverse[index];
    }
}
void encode(char* input,my_file* output){
    int count=0;
    char* after_encode= malloc(strlen(input)*sizeof(char));
    int offset=0;
    for(int i=0;i<=strlen(input);i++){
        if(input[i]=='\0'){
            after_encode[offset++]=input[i-1];
            number_handler(after_encode,count,&offset);
        }else if(i==0){
            count+=1;
        }else{
          if(input[i]==input[i-1]){
              count+=1;
          }else{
              after_encode[offset++]=input[i-1];
              number_handler(after_encode,count,&offset);
              count=1;
          }
        }
    }
    after_encode[offset]='\0';
    write(1,after_encode,offset);
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
    if(input_head==NULL){
        fprintf(stderr,"\n\" \" empty file\n"
                );
        exit(1);
    }
    char* whole_input= malloc((input_size+1)*sizeof (char));
    strcpy(whole_input,input_head->ptr);
    if(input_head->next_file!=NULL){
        whole_input= concatenate_input(input_head->next_file,&whole_input);
    }

    encode(whole_input,output);

    return 0;
}
