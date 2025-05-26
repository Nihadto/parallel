# parallel
Description
This project implements and compares both sequential and parallel versions of the Merge Sort algorithm in C++, utilizing POSIX threads (pthreads) for concurrency. The program measures the execution time of each version and verifies the correctness of the sorting.

Key Features
Sequential Merge Sort: Standard recursive implementation.
Parallel Merge Sort: Utilizes pthreads to sort subarrays concurrently up to a specified thread depth
( THREAD_LEVEL ).
Thread-Level Control: The THREAD LEVEL argument allows you to control how deep the threading goes.
Performance Measurement: Measures the time taken for each sorting method.
Validation: Verifies if the final array is sorted correctly.

How It Works
The array is recursively split into halves (like standard Merge Sort).
If thread_level > 0 , the algorithm spawns threads to sort each half in parallel.
If thread level =0, it falls back to sequential sorting.
After sorting, the halves are merged.
