#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

pthread_mutex_t lock;
pthread_mutex_t locks[100];

bool rescued_random_global[100];
bool rescued_random_drawer[100];
bool rescued_strategy_global[100];
bool rescued_strategy_drawer[100];

typedef struct prisoner {
    int number;
    int *drawers;
    bool found;
} prisoner_t;

// Function for initializing drawers with 
// random unique number from 1 to 100
void init_drawers(int *drawers) {
    
    int dr_list[100];

    for(int i = 1; i <= 100; i++) {
        dr_list[i - 1] = i;
    }

    for (int i = 0; i < 100; i++) {
        int dr_iterator = i + rand() % (100 - i);

        int dr_temp = dr_list[i];
        dr_list[i] = dr_list[dr_iterator];
        dr_list[dr_iterator] = dr_temp;

        drawers[i] = dr_list[i];
    }
}

// pthread mutex lock/unlock with error handling
void mutex_lock(pthread_mutex_t *mutex) {
    int errc = pthread_mutex_lock(mutex);
    if (errc) {
        fprintf(stderr, "pthread_mutex_lock(): %s\n", strerror(errc));
        exit(EXIT_FAILURE);
    }
}

void mutex_unlock(pthread_mutex_t *mutex) {
    int errc = pthread_mutex_unlock(mutex);
    if (errc) {
        fprintf(stderr, "pthread_mutex_unlock(): %s\n", strerror(errc));
        exit(EXIT_FAILURE);
    }
}

void* random_global(void *data) {
    prisoner_t *prisoner = (prisoner_t *) data;
    mutex_lock(&lock);

    for(int i = 0; i < 50; i++) {
        if(prisoner->drawers[i] == prisoner->number) {
            // If prisoner finds the key, record it in global array
            prisoner->found = true;
            rescued_random_global[prisoner->number - 1] = true;
            break;
        }
    }
    
    mutex_unlock(&lock);

    return NULL;
}

void* strategy_global(void *data) {
    prisoner_t *prisoner = (prisoner_t *) data;
    mutex_lock(&lock);

    int i = 0;
    int num = prisoner->number;
    int temp_num = prisoner->number;
    // Logic for specified searching method in problem
    while(i < 50) {
        if(prisoner->drawers[temp_num - 1] != num) {
            temp_num = prisoner->drawers[temp_num - 1];
        } else {    
            // If prisoner finds the key, record it in global array
            prisoner->found = true;
            rescued_strategy_global[num - 1] = true;
            break;
        }
        i++;
    }
    
    mutex_unlock(&lock);

    return NULL;
}

void* random_drawer(void *data) {
    prisoner_t *prisoner = (prisoner_t *) data;

    int i = 0;
    while(i < 50 && prisoner->found == false) {
        mutex_lock(&locks[prisoner->number - 1]);
        if(prisoner->drawers[i] == prisoner->number) {
            // If prisoner finds the key, record it in global array
            prisoner->found = true;
            rescued_random_drawer[prisoner->number - 1] = true;
        }
        i++;
        mutex_unlock(&locks[prisoner->number - 1]);
    }

    return NULL;
}

void* strategy_drawer(void *data) {
    prisoner_t *prisoner = (prisoner_t *) data;

    int i = 0;
    int num = prisoner->number;
    int temp_num = prisoner->number;
    // Logic for specified searching method in problem
    while(i < 50 && prisoner->found == false) {
        mutex_lock(&locks[num - 1]);
        if(prisoner->drawers[temp_num - 1] != num) {
            temp_num = prisoner->drawers[temp_num - 1];
        } else {    
            // If prisoner finds the key, record it in global array
            prisoner->found = true;
            rescued_strategy_drawer[num - 1] = true;
        }
        i++;
        mutex_unlock(&locks[num - 1]);
    }
    
    return NULL;
}

// Function for running 100 threads for each method
void run_threads(void* func, int *drawers) {
    prisoner_t prisoners[100];
    int errc;
    pthread_t tids[100];

    for(int i = 0; i < 100; i++) {
        prisoner_t prisoner = {
            .number = i + 1,
            .drawers = drawers,
            .found = false,
        };
        // This line makes sure all the prisoners don't point to same memory address
        prisoners[i] = prisoner;

        errc = pthread_create(&tids[i], NULL, func, &prisoners[i]);
        if(errc) {
            fprintf(stderr, "pthread_create(): %s\n", strerror(errc));
        }
    }
    
    for(int i = 0; i < 100; i++) {
        if(tids[i]) {
            errc = pthread_join(tids[i], NULL);
            if(errc) {
                fprintf(stderr, "pthread_join(): %s\n", strerror(errc));
            }
        }
    }
}

// Function runs specified number of games, counts and returns number of wins
int run_games(bool *result_arr, int *drawers, void *func, int games_count) {

    int counter = 0;
    bool final_res = true;

    for(int i = 0; i < games_count; i++) {
        // resets global array of results to zeros
        memset(result_arr, false, sizeof(*result_arr) * 100);
        init_drawers(drawers);
        run_threads(func, drawers);
        // resets final result to true
        final_res = true;
        for(int j = 0; j < 100; j++) {
            if(result_arr[j] == false) {
                final_res = false;
            }
        }
        if(final_res) {
            counter++;
        }
    }

    return counter;
}

int main(int argc, char *argv[]) {
    //Initialize locks
    int errc;
    errc = pthread_mutex_init(&lock, NULL);
    if (errc) {
        fprintf(stderr, "pthread_mutex_init(lock): %s\n", strerror(errc));
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < 100; i++) {
        errc = pthread_mutex_init(&locks[i], NULL);
        if (errc) {
            fprintf(stderr, "pthread_mutex_init(locks[%d]): %s\n", i, strerror(errc));
            exit(EXIT_FAILURE);
        }
    }


    int opt;
    int games_count = 100;
    bool static_seed = true;

    // Parse command line arguments
    while ((opt = getopt(argc, argv, "n:s")) != -1) {
        switch (opt) {
        case 'n':
            games_count = atoi(optarg);
            break;
        case 's':
            static_seed = false;
            break;
        }
    }

    // Static or non-static seed for rand()
    if(static_seed) {
        srand(0);
    } else {
        srand(time(NULL));
    }

    int drawers[100];

    // Run specified number of games with random global method and measure time
    clock_t t1 = clock();
    int rnd_gl_win_count = run_games(rescued_random_global, drawers, random_global, games_count); 
    clock_t t2 = clock();

    // Run specified number of games with random drawer method and measure time
    clock_t t3 = clock();
    int rnd_dr_win_count = run_games(rescued_random_drawer, drawers, random_drawer, games_count);
    clock_t t4 = clock();
    
    // Run specified number of games with strategy global method and measure time
    clock_t t5 = clock();
    int str_gl_win_count = run_games(rescued_strategy_global, drawers, strategy_global, games_count);
    clock_t t6 = clock();
    
    // Run specified number of games with strategy drawer method and measure time
    clock_t t7 = clock();
    int str_dr_win_count = run_games(rescued_strategy_drawer, drawers, strategy_drawer, games_count);
    clock_t t8 = clock();

    // Calculate times and print everything
    double measure1 = ((double) t2 - (double) t1) / CLOCKS_PER_SEC * 1000;
    double measure2 = ((double) t4 - (double) t3) / CLOCKS_PER_SEC * 1000;
    double measure3 = ((double) t6 - (double) t5) / CLOCKS_PER_SEC * 1000;
    double measure4 = ((double) t8 - (double) t7) / CLOCKS_PER_SEC * 1000;

    printf("Random global: %d/%d Wins, Time: %lf ms\n", rnd_gl_win_count, games_count, measure1);
    printf("Random drawer: %d/%d Wins, Time: %lf ms\n", rnd_dr_win_count, games_count, measure2);
    printf("Strategy global: %d/%d Wins, Time: %lf ms\n", str_gl_win_count, games_count, measure3);
    printf("Strategy drawer: %d/%d Wins, Time: %lf ms\n", str_dr_win_count, games_count, measure4);

    // Destroying locks
    errc = pthread_mutex_destroy(&lock);
    if (errc) {
        fprintf(stderr, "pthread_mutex_destroy(lock): %s\n", strerror(errc));
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < 100; i++) {
        errc = pthread_mutex_destroy(&locks[i]);
        if (errc) {
            fprintf(stderr, "pthread_mutex_destroy(locks[%d]): %s\n", i, strerror(errc));
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}