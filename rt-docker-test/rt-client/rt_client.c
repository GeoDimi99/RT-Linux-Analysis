#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>

#define API_URL "http://localhost:8000/ping"

struct timespec next_activation;

void set_realtime_priority() {
    struct sched_param param;
    param.sched_priority = 80;  // High RT priority
    
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler failed");
        exit(EXIT_FAILURE);
    }
}

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    printf("%.*s", (int)(size * nmemb), (char*)contents);
    return size * nmemb;
}

void* rt_api_task(void* arg) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "CURL init failed\n");
        exit(EXIT_FAILURE);
    }

    // Set periodic execution (1Hz)
    clock_gettime(CLOCK_MONOTONIC, &next_activation);
    next_activation.tv_sec += 1;

    while (1) {
        //clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_activation, NULL);
        clock_t start = clock();
        while ((double)(clock() - start) / CLOCKS_PER_SEC < 0.5);

        // Call API
        printf("Execution curl ... ");
        curl_easy_setopt(curl, CURLOPT_URL, API_URL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        printf("Done\n");
        
        //next_activation.tv_sec += 1;
    }

    curl_easy_cleanup(curl);
    return NULL;
}

int main() {
    set_realtime_priority();
    
    pthread_t thread;
    pthread_create(&thread, NULL, rt_api_task, NULL);
    pthread_join(thread, NULL);
    
    return 0;
}
