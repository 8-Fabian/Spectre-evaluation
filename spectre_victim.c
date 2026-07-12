#include <x86intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>  
#include <fcntl.h>           
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sched.h>

#define TARGET_ARRAY_LENGTH 256
#define ELEMENT_DISTANCE 512

const char *shared_mem_name = "/test";
const char *pipe_name = "/tmp/spectre_pipe";

uint8_t x[150];
char my_secret[10000];
uint64_t y[16 * sizeof(uint64_t)];
uint64_t bounds[16 * sizeof(uint64_t)];
uint8_t* target_array;

void victim_func(int index){
    char trash = my_secret[index - 192];
    _mm_lfence();
    if(index < bounds[0]){ 
        y[0] = target_array[x[index] * ELEMENT_DISTANCE];
    }
    return;
}

void map_shared_array(){
    int oflags = O_RDWR;
	int fd = shm_open(shared_mem_name, oflags, 0644);
    
	assert (fd>0);
	struct stat sb;
	fstat(fd, &sb);
	off_t length = sb.st_size;

	target_array = (void*)mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	assert(target_array);
	close(fd);
}

void create_pipe(){
    mkfifo(pipe_name, 0777);
}

void read_pipe(){ // Reads from pipe and executes victim functions with given Index
    uint64_t read_buffer;
    int write_value = 42;
    int pipe_fd = open(pipe_name, O_RDONLY);
    _mm_clflush(&bounds[0]);
    read(pipe_fd, &read_buffer, sizeof(read_buffer));
    victim_func(read_buffer);
    close(pipe_fd);
    pipe_fd = open(pipe_name, O_WRONLY);
    write(pipe_fd, &write_value, sizeof(int));
    sched_yield();
    close(pipe_fd);
}

int main(int argc, char* argv[]) {
    create_pipe();
    FILE *secret_text = fopen("../secret.txt", "r");
    fgets(my_secret, 10000, secret_text);
    fclose(secret_text);
    printf("Offset of Secret is: %li\n", (void*)&my_secret[0] - (void*)&x[0]);
    bounds[0] = 150;
    x[0] = 0;
    map_shared_array();
    while(1){
        read_pipe();
    }
    return 0;
}