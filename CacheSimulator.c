#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
int get_LRU(int associativity, int* cache_set);
int get_Belady(int current_line, int associativity, int* cache_set, int* accesses, int access_size);
//MARK CHIN
int main(int argc, char *argv[]){
    if(argc != 4){
        fprintf(stderr, "Invalid command line arguments");
        return EXIT_FAILURE;
    }
    int size = 256;
    int associativity = atoi(argv[1]);
    int sets = size/associativity;
    printf("Cache size: %d\n", size);
    printf("Cache associativity: %d\n", associativity);
    printf("Cache sets: %d\n", sets);
    printf("Cache algorithm: %s\n", argv[2]);
    FILE* file = fopen(argv[3], "r");
    int* accesses = calloc(sizeof(int), 1024);
    int index = 0;
    char buff[1024];
    if (file != NULL) {
        while(fgets(buff,sizeof(buff),file)!= NULL)  {
            accesses[index] = atoi(buff);
            index++;
        }
    
        fclose(file);
    }
    else {
        perror(argv[3]); 
    }
    int i, j;
    int set;
    int hits = 0;
    int misses = 0;
    char* result;
    int local_location;
    int** cache = calloc(sizeof(int*), sets);
    for(i = 0; i < index; i++){
        local_location = 0;
        //Assume a miss until match found
        result = "miss";
        set = accesses[i] % sets;
        if(cache[set]){
            for(j = 0; j < associativity; j++){
                //If the value is found, update result to "hit", store location, and go to end.
                if(cache[set][j] == accesses[i]){
                    hits++;
                    result = "hit";
                    cache[set][j+associativity] = i;
                    goto end;
                }
            }
            //This will find the next empty spot on the indices [0, associativity)
            //local_location==associativity if the whole set is full, which means a replacement will be required.
            while(cache[set][local_location] && local_location < associativity){
                local_location++;
            }
            if(local_location == associativity){
               if(strcmp(argv[2], "LRU") == 0){
                    int lru = get_LRU(associativity,cache[set]);
                    cache[set][lru-associativity] = accesses[i];
                    cache[set][lru] = i;
               }else{
                   int belady = get_Belady(i, associativity, cache[set], accesses, index);
                    cache[set][belady] = accesses[i];
                    cache[set][belady+associativity] = i;
               }
            }else{
                cache[set][local_location] = accesses[i];
                cache[set][local_location+associativity] = i;
            }
            misses++;
        }else{
            //This is the initializer which only triggers if the set has not been created yet.
            misses++;
            cache[set] = calloc(sizeof(int), associativity*2+1);
            cache[set][local_location] = accesses[i];
            cache[set][associativity] = i;
        }
        end:
        printf("%d (%s)\n",accesses[i], result);
    }
    printf("Cache accesses: %d\n", index);
    printf("Cache hits: %d\n", hits);
    printf("Cache misses: %d\n", misses);
    float hitrate = (float)hits/index;
    printf("Overall hit rate: %0.6f\n", hitrate);
    free(accesses);
    for(i = 0; i < sets; i++){
        if(cache[i]) free(cache[i]);
    }
    free(cache);
    return EXIT_SUCCESS;
}
int get_LRU(int associativity, int* cache_set){
    //This function loops through all the locations in the given set
    //and stores the smallest location to return.
    int i;
    int lru_loc = 2000;
    int lru_val = 2000;
    for(i = associativity; i < associativity*2; i++){
        if(cache_set[i] < lru_val) {
            lru_loc = i;
            lru_val = cache_set[i];
        }
    }
    return lru_loc;
}
int get_Belady(int current_line, int associativity, int* cache_set, int* accesses, int access_size){
    int i, j;
    int furthest = 1;
    int distance = 0;
    int belady_loc = 0;
    //Check each value in set against every value that comes after to find matches
    //If the distance is greater, save it to return later.
    for(i = 0; i < associativity; i++){
        for(j = current_line+1; j < access_size; j++){
            if(cache_set[i] == accesses[j]){
                distance = j - current_line;
                break;
            }
            //If there are no future matches, use that spot as replacement.
            if(j == access_size-1){
                return i;
            }
        }
        if(distance > furthest){
             furthest = distance;
             belady_loc = i;
        }
    }
    return belady_loc;
}