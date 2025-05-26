#include <pthread.h>
#include<bits/stdc++.h>
using namespace std;
//size of the array
const int DEFAULT_NUM = 1000;

const int DEFAULT_THREAD_LEVEL = 3;

// Function declarations
void merge(double array[], double temp[], int low, int mid, int high);
void mergesortHelper(double array[], double temp[], int low, int high);
void mergesort(double array[], int size);
bool isSorted(const double array[], int size);
void parallelMergesort(double array[], int size, int thread_level);
void* parallelMergesortHelper(void* thread_arg);

// Thread data structure
struct ThreadData {
    double* array;
    double* temp;
    int low;
    int high;
    int level;
};

int main(int argc, char* argv[]) {
    using namespace std::chrono;
    
    // Configuration
    int NUM = DEFAULT_NUM;
    int THREAD_LEVEL = DEFAULT_THREAD_LEVEL;
    
    if (argc >= 2) NUM = atoi(argv[1]);
    if (argc >= 3) THREAD_LEVEL = atoi(argv[2]);

    // Allocate memory for the arrays
    auto original_array = make_unique<double[]>(NUM);
    auto working_array = make_unique<double[]>(NUM);

    // Initialize with random values
    srand(time(nullptr));
    for (int i = 0; i < NUM; i++) {
        original_array[i] = static_cast<double>(rand()) / RAND_MAX;
    }

    // Test sequential mergesort
    copy(original_array.get(), original_array.get() + NUM, working_array.get());
    
    auto start = high_resolution_clock::now();
    mergesort(working_array.get(), NUM);
    auto end = high_resolution_clock::now();
    
    if (!isSorted(working_array.get(), NUM)) {
        cout << "Error: Sequential mergesort failed!\n";
    }
    
    duration<double> diff = end - start;
    cout << "Sequential mergesort: " << diff.count() << " seconds\n";

    // Test parallel mergesort
    copy(original_array.get(), original_array.get() + NUM, working_array.get());
    
    start = high_resolution_clock::now();
    parallelMergesort(working_array.get(), NUM, THREAD_LEVEL);
    end = high_resolution_clock::now();
    
    if (!isSorted(working_array.get(), NUM)) {
        std::cout << "Error: Parallel mergesort failed!\n";
    }
    
    diff = end - start;
    cout << "Parallel mergesort (" << THREAD_LEVEL << " levels): " 
              << diff.count() << " seconds\n";

    return 0;
}

// Top level sequential mergesort
void mergesort(double array[], int size) {
    auto temp = make_unique<double[]>(size);
    mergesortHelper(array, temp.get(), 0, size - 1);
}

// The actual mergesort implementation
void mergesortHelper(double array[], double temp[], int low, int high) {
    if (low >= high) return;
    
    int mid = low + (high - low) / 2;
    mergesortHelper(array, temp, low, mid);
    mergesortHelper(array, temp, mid + 1, high);
    merge(array, temp, low, mid, high);
}

// Merge two sorted subarrays
void merge(double array[], double temp[], int low, int mid, int high) {
    int i = low, j = mid + 1, k = low;
    
    while (i <= mid && j <= high) {
        if (array[i] <= array[j]) {
            temp[k++] = array[i++];
        } else {
            temp[k++] = array[j++];
        }
    }
    
    // Copy remaining elements
    copy(array + i, array + mid + 1, temp + k);
    copy(array + j, array + high + 1, temp + k + (mid - i + 1));
    
    // Copy back to original array
    copy(temp + low, temp + high + 1, array + low);
}

// Check if array is sorted
bool isSorted(const double array[], int size) {
    for (int i = 1; i < size; i++) {
        if (array[i] < array[i - 1]) {
            return false;
        }
    }
    return true;
}

// Top level parallel mergesort
void parallelMergesort(double array[], int size, int thread_level) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    auto temp = make_unique<double[]>(size);
    
    ThreadData td{array, temp.get(), 0, size - 1, thread_level};
    pthread_t thread;
    
    if (pthread_create(&thread, &attr, parallelMergesortHelper, &td)) {
        cout << "Error creating thread\n";
        exit(EXIT_FAILURE);
    }
    
    pthread_attr_destroy(&attr);
    
    if (pthread_join(thread, nullptr)) {
        cout << "Error joining thread\n";
        exit(EXIT_FAILURE);
    }
}

// Parallel mergesort helper function
void* parallelMergesortHelper(void* thread_arg) {
    ThreadData* data = static_cast<ThreadData*>(thread_arg);
    
    if (data->level <= 0 || data->low >= data->high) {
        mergesortHelper(data->array, data->temp, data->low, data->high);
        return nullptr;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    const int mid = data->low + (data->high - data->low) / 2;
    ThreadData thread_data[2] = {
        {data->array, data->temp, data->low, mid, data->level - 1},
        {data->array, data->temp, mid + 1, data->high, data->level - 1}
    };
    
    pthread_t threads[2];
    
    for (int i = 0; i < 2; i++) {
        if (pthread_create(&threads[i], &attr, parallelMergesortHelper, &thread_data[i])) {
            cerr << "Error creating thread\n";
            exit(EXIT_FAILURE);
        }
    }
    
    pthread_attr_destroy(&attr);
    
    for (int i = 0; i < 2; i++) {
        if (pthread_join(threads[i], nullptr)) {
            cout << "Error joining thread\n";
            exit(EXIT_FAILURE);
        }
    }
    
    merge(data->array, data->temp, data->low, mid, data->high);
    return nullptr;
}