#include <omp.h>
#include <bits/stdc++.h>
using namespace std;

// Size of the array
const int DEFAULT_NUM = 100000000;
const int DEFAULT_THREAD_LEVEL = 0;

// Function declarations
void merge(double array[], double temp[], int low, int mid, int high);
void mergesortHelper(double array[], double temp[], int low, int high);
void mergesort(double array[], int size);
bool isSorted(const double array[], int size);
void parallelMergesort(double array[], int size, int thread_level);
void parallelMergesortHelper(double array[], double temp[], int low, int high, int level);

int main(int argc, char* argv[]) {
    using namespace std::chrono;
    
    // Configuration
    int NUM = DEFAULT_NUM;
    int THREAD_LEVEL = DEFAULT_THREAD_LEVEL;
    
    if (argc >= 2) NUM = atoi(argv[1]);
    if (argc >= 3) THREAD_LEVEL = atoi(argv[2]);

    // Set number of threads
    omp_set_num_threads(1 << THREAD_LEVEL); // 2^THREAD_LEVEL threads

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
        cout << "Error: Parallel mergesort failed!\n";
    }
    
    diff = end - start;
    cout << "Parallel mergesort (" << THREAD_LEVEL << " levels, " 
         << omp_get_max_threads() << " threads): " << diff.count() << " seconds\n";

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
    while (i <= mid) temp[k++] = array[i++];
    while (j <= high) temp[k++] = array[j++];
    
    // Copy back to original array
    for (i = low; i <= high; i++) {
        array[i] = temp[i];
    }
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
    auto temp = make_unique<double[]>(size);
    parallelMergesortHelper(array, temp.get(), 0, size - 1, thread_level);
}

// Parallel mergesort helper function using OpenMP
void parallelMergesortHelper(double array[], double temp[], int low, int high, int level) {
    if (low >= high) return;
    
    int mid = low + (high - low) / 2;
    
    // If we've reached the base level or the problem is too small, use sequential sort
    if (level <= 0 || (high - low) < 1000) {
        mergesortHelper(array, temp, low, high);
        return;
    }
    
    // Use OpenMP tasks to parallelize the recursive calls
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            parallelMergesortHelper(array, temp, low, mid, level - 1);
            
            #pragma omp task
            parallelMergesortHelper(array, temp, mid + 1, high, level - 1);
            
            #pragma omp taskwait
        }
    }
    
    // Merge the two sorted halves
    merge(array, temp, low, mid, high);
}