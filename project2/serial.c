#include <dirent.h> 
#include <stdio.h> 
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <time.h>
#include <pthread.h>

#define BUFFER_SIZE 1048576 // 1MB
#define MAX_THREADS 20

// Struct to hold data for each thread
typedef struct {
    char* filepath;
    int file_index;  // Added to maintain order
    unsigned char* buffer_in;
    unsigned char* buffer_out;
    int nbytes;
    int nbytes_zipped;
    int processed;   // Flag to track completion
} ThreadData;

// Global variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
FILE* global_f_out;
long global_total_in = 0;    // Changed to long to prevent overflow
long global_total_out = 0;   // Changed to long to prevent overflow
ThreadData* results;         // Array to store results in order

int cmp(const void *a, const void *b) {
    return strcmp(*(char **) a, *(char **) b);
}

void* compress_file(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    // Load file
    FILE* f_in = fopen(data->filepath, "r");
    if (f_in == NULL) {
        printf("Error opening file: %s\n", data->filepath);
        data->processed = -1;  // Mark as error
        return NULL;
    }

    // Read file
    data->buffer_in = malloc(BUFFER_SIZE * sizeof(unsigned char));
    data->buffer_out = malloc(BUFFER_SIZE * sizeof(unsigned char));
    if (!data->buffer_in || !data->buffer_out) {
        if (data->buffer_in) free(data->buffer_in);
        if (data->buffer_out) free(data->buffer_out);
        fclose(f_in);
        data->processed = -1;
        return NULL;
    }

    data->nbytes = fread(data->buffer_in, sizeof(unsigned char), BUFFER_SIZE, f_in);
    fclose(f_in);

    // Zip file
    z_stream strm = {0};  // Initialize to zero
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int ret = deflateInit(&strm, 9);
    if (ret != Z_OK) {
        free(data->buffer_in);
        free(data->buffer_out);
        data->processed = -1;
        return NULL;
    }

    strm.avail_in = data->nbytes;
    strm.next_in = data->buffer_in;
    strm.avail_out = BUFFER_SIZE;
    strm.next_out = data->buffer_out;

    ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&strm);
        free(data->buffer_in);
        free(data->buffer_out);
        data->processed = -1;
        return NULL;
    }

    data->nbytes_zipped = BUFFER_SIZE - strm.avail_out;
    deflateEnd(&strm);

    // Update global statistics thread-safely
    pthread_mutex_lock(&mutex);
    global_total_in += data->nbytes;
    global_total_out += data->nbytes_zipped;
    pthread_mutex_unlock(&mutex);

    data->processed = 1;  // Mark as successfully processed
    return NULL;
}

int main(int argc, char **argv) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    assert(argc == 2);

    DIR *d;
    struct dirent *dir;
    char **files = NULL;
    int nfiles = 0;

    d = opendir(argv[1]);
    if(d == NULL) {
        printf("An error has occurred\n");
        return 0;
    }

    // Create sorted list of PPM files
    while ((dir = readdir(d)) != NULL) {
        int len = strlen(dir->d_name);
        if(len > 4 && dir->d_name[len-4] == '.' && 
           dir->d_name[len-3] == 'p' && 
           dir->d_name[len-2] == 'p' && 
           dir->d_name[len-1] == 'm') {
            files = realloc(files, (nfiles+1)*sizeof(char *));
            assert(files != NULL);
            files[nfiles] = strdup(dir->d_name);
            assert(files[nfiles] != NULL);
            nfiles++;
        }
    }
    closedir(d);
    qsort(files, nfiles, sizeof(char *), cmp);

    // Initialize results array
    results = calloc(nfiles, sizeof(ThreadData));
    assert(results != NULL);

    // Open output file
    global_f_out = fopen("video.vzip", "w");
    assert(global_f_out != NULL);

    // Process files in batches
    pthread_t threads[MAX_THREADS];
    int batch_start = 0;

    while (batch_start < nfiles) {
        int batch_size = (nfiles - batch_start < MAX_THREADS) ? 
                        (nfiles - batch_start) : MAX_THREADS;

        // Create threads for current batch
        for (int i = 0; i < batch_size; i++) {
            int idx = batch_start + i;
            results[idx].file_index = idx;
            results[idx].processed = 0;

            // Create full path
            int path_len = strlen(argv[1]) + strlen(files[idx]) + 2;
            results[idx].filepath = malloc(path_len * sizeof(char));
            assert(results[idx].filepath != NULL);
            snprintf(results[idx].filepath, path_len, "%s/%s", argv[1], files[idx]);

            int ret = pthread_create(&threads[i], NULL, compress_file, &results[idx]);
            assert(ret == 0);
        }

        // Wait for all threads in current batch
        for (int i = 0; i < batch_size; i++) {
            pthread_join(threads[i], NULL);
        }

        // Write results in order for this batch
        for (int i = 0; i < batch_size; i++) {
            int idx = batch_start + i;
            if (results[idx].processed == 1) {  // Only write successfully processed files
                fwrite(&results[idx].nbytes_zipped, sizeof(int), 1, global_f_out);
                fwrite(results[idx].buffer_out, sizeof(unsigned char), 
                      results[idx].nbytes_zipped, global_f_out);
            }
            // Clean up resources
            free(results[idx].filepath);
            if (results[idx].buffer_in) free(results[idx].buffer_in);
            if (results[idx].buffer_out) free(results[idx].buffer_out);
        }

        batch_start += batch_size;
    }

    // Clean up and print results
    fclose(global_f_out);
    free(results);
    pthread_mutex_destroy(&mutex);

    printf("Compression rate: %.2lf%%\n", 
           100.0*(global_total_in-global_total_out)/global_total_in);

    // Release list of files
    for(int i = 0; i < nfiles; i++)
        free(files[i]);
    free(files);

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Time: %.2f seconds\n", 
           ((double)end.tv_sec+1.0e-9*end.tv_nsec)-((double)start.tv_sec+1.0e-9*start.tv_nsec));

    return 0;
}