#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>

#define API_URL "http://localhost:8000/ping"


size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    printf("%.*s", (int)(size * nmemb), (char*)contents);
    return size * nmemb;
}


int main(int argc, char **argv) {
    
    /* Setting scheduler policy and priority*/
    struct sched_param param;
    int thread_priority = argc > 1 ? atoi(argv[1]) : 0;
    param.sched_priority = thread_priority; 
    sched_setscheduler(0, SCHED_FIFO, &param);
    
    /* Set thread id */
    int thread_id = argc > 2 ? atoi(argv[2]) : -1;
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "CURL init failed\n");
        exit(EXIT_FAILURE);
    }

    while (1) {

        /* Busy waiting*/
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
        
    }

    curl_easy_cleanup(curl);

    return 0;
}
