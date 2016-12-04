#include <iostream>
#include <sys/timeb.h>
#include <random>

using namespace std;

#define DEBUG_MODE 0
#define TEST_MODE 0
#define DEBUG_MODE_THREADS 0

int *arr;
long gRefTime;
int TYPE_OF_SORT;

typedef struct _thread_data_t {
    int tid;
    int low;
    int high;
} THREAD_DATA;

enum SORT_TYPES {
    INSERTION,
    QUICK,
    MERGE,
    HEAP,
    BUBBLE,
    SELECTION,

};

//TODO: THINK about other ways how to do the final merge of sub-arrays
//TODO: 20 threads for 20 elements is not good - RESOLVE IT!!!
//TODO: test your program in 2003 or 2009 lab


void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}


/* * * * * * * *  Selection Sort * * * * * * * * */
void selectionSort(int *arr, int n){
    for(int i = 0; i < n - 1; i++){
        int min = i;
        for(int j = i + 1; j < n; j++){
            if(arr[j] < arr[min])
                min = j;
        }
        if(i != min)
            swap(arr + i, arr + min);
    }
}

/* * * * * * * *  Bubble Sort * * * * * * * * */
void bubbleSort(int *arr, int n){
    for(int i = 0; i < n-1; i++){
        bool swapped = false;
        for(int j = 0; j < n - 1 - i; j++){
            if(arr[j] > arr[j+1]){
                swap(arr + j, arr + j + 1);
                swapped = true;
            }
        }
        if(swapped == false)
            return;
    }
}

/* * * * * * * *  Heap Sort * * * * * * * * *
 *
 * Magic rules:
 *      left kid: (i*2)+ 1
 *      right kid: (i*2) + 2
 *      inner nodes: (n/2)-1
 */

void heapifyMax(int *arr, int i, int n){
    int max = i;
    int l = i * 2 + 1;
    int r = i * 2 + 2;
    if(l < n && arr[l] > arr[max])
        max = l;
    if(r < n && arr[r] > arr[max])
        max = r;
    if(max != i){
        swap(arr + i, arr + max);
        heapifyMax(arr, max, n);
    }
}

void heapSort(int *arr, int n){
    for(int i = n/2-1; i >=0; i--){
        heapifyMax(arr, i, n);
    }
    for(int i = 0; i < n - 1; i++){
        swap(arr, arr + n - i - 1);
        heapifyMax(arr, 0, n- i - 1);
    }
}


/* * * * * * * *  Quick Sort * * * * * * * * */

int divideIntoTwoHalves(int *arr, int l, int h) {
    int pivot = arr[h];
    int j = l - 1;
    for (int i = l; i < h; i++) {
        if (arr[i] <= pivot) {
            swap(arr + (++j), arr + i);
        }
    }
    swap(arr + (++j), arr + h);
    return j;
}

void quickSort(int *arr, int l, int h) {
    if (l < h) {
        int div = divideIntoTwoHalves(arr, l, h);
        quickSort(arr, l, div - 1);
        quickSort(arr, div + 1, h);
    }
}

/* * * * * * * *  Insertion Sort * * * * * * * * */

void insertionSort(int *arr, int n) {
    for (int i = 1; i < n; i++) {
        int curr = arr[i];
        int j = i - 1;
        while (j >= 0 && curr <= arr[j]) {
            arr[j + 1] = arr[j];
            j--;
        }
        j++;
        arr[j] = curr;
    }
}


/* * * * * * * *  Merge Sort * * * * * * * * */

void merge(int arr[], int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1; // Subtle part of the merge sort. Use your brains here!
    int n2 = r - m;

    int *L = (int *) calloc((size_t) n1, sizeof(int));
    int *R = (int *) calloc((size_t) n2, sizeof(int));

    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    /* Compare elements in the left and the right sub-arrays and form a sorted array*/
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    delete L;
    delete R;
}

void mergeSort(int arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2; //Same as (l+r)/2, but avoids overflow for large l and h
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

/* * * * * * * *  Timing related stuff * * * * * * * * */

long GetMilliSecondTime(struct timeb timeBuf) {
    long milliSecondsTime;

    milliSecondsTime = timeBuf.time;
    milliSecondsTime *= 1000;
    milliSecondsTime += timeBuf.millitm;
    return milliSecondsTime;
}

long GetCurrentTime(void) {
    long currentTime = 0;

    struct timeb timeBuf;
    ftime(&timeBuf);
    currentTime = GetMilliSecondTime(timeBuf);

    return currentTime;
}

void setTime(void) {
    gRefTime = GetCurrentTime();
}

long getTime(void) {
    long currentTime = GetCurrentTime();
    return (currentTime - gRefTime);
}


void printAllElements(int numOfElements, int arr[]) {
    for (int j = 0; j < numOfElements; ++j) {
        printf("The value of element: %d - %d\n", j, arr[j]);
    }
    printf("\n");
}


/* Return 1 for success
 * Return -1 for failure */
int assertSuccessSort(const int const *arr, const int length) {
    int i = 0;
    while (i != length - 1) {
        if (arr[i] > arr[i + 1]) {
            return -1;
        }
        i++;
    }
    return 1;
}

/* low inclusive [
 * high exclusive ) */
void populateArrayWithRandomInt(int *&data, const int len, int const low, int const high) {
    data = (int *) calloc((size_t) len, sizeof(int));
    int temp = 0;
    while (temp < len) {
        *(data + temp) = (rand() % (high - low) + low);
        temp++;
    }
}

//Run the correct function from inside the threads.
void *thr_func(void *arg) {
    THREAD_DATA *data = (THREAD_DATA *) arg;

    if (TYPE_OF_SORT == SORT_TYPES::MERGE)
        mergeSort(arr, data->low, data->high);
    else if (TYPE_OF_SORT == SORT_TYPES::QUICK)
        quickSort(arr, data->low, data->high);
    else if (TYPE_OF_SORT == SORT_TYPES::INSERTION)
        insertionSort(&arr[data->low], data->high);
    else if (TYPE_OF_SORT == SORT_TYPES::HEAP)
        heapSort(&arr[data->low], data->high);
    else if (TYPE_OF_SORT == SORT_TYPES::BUBBLE)
        bubbleSort(&arr[data->low], data->high);
    else if (TYPE_OF_SORT == SORT_TYPES::SELECTION)
        selectionSort(&arr[data->low], data->high);


#if TEST_MODE == 1
    printf("\n Hello from thr_func, thread id is: %d\n", data->tid);
#endif
    pthread_exit(NULL);
}

int assertCorrectParamsForMain(int argc) {
    if (argc < 3) {
        cerr << "ERROR"
                "\n\nWhile firing the process please enter the following parameters: "
                "\narray size [int between 1 and 100 000 000 "
                "\nnumber of threads [int between 1 - 16] "
                "\nsorting algorithm [l - insertion sort | q - quick sort]"
                " | b - bubble sort | h - heap sort ";
        return -1;
    }
    return 1;
}

int main(int argc, char **argv) {

#if TEST_MODE == 1
    for(int i = 0; i < 12; i++){
        for(int j = 0; j < 2; j++){
            indices[i][j] = j;
        }
    }
    for(int i = 0; i < 12; i++){
        cout << "index is " << i << " values are: " << indices[i][0] << " and " << indices[i][1] << endl;
    }
#endif
    if (assertCorrectParamsForMain(argc) == -1)
        return -1;

    const int NUM_OF_ELEMENTS = atoi(argv[1]);
    const int NUM_OF_THREADS = atoi(argv[2]);
    char SORTING_ALGORITHM = argv[3][0]; //[][0] in order to get the first character instead of a "c-string"

    // indices mark initial array into discrete sections, which are equal to the number of threads.
    int **indices;
    indices = (int **) (calloc((size_t) NUM_OF_THREADS, sizeof(int *)));
    for (int i = 0; i < NUM_OF_THREADS; i++) {
        indices[i] = (int *) calloc(2, sizeof(int));
    }

    if (SORTING_ALGORITHM == 'i' | SORTING_ALGORITHM == 'I')
        TYPE_OF_SORT = SORT_TYPES::INSERTION;
    else if (SORTING_ALGORITHM == 'q' | SORTING_ALGORITHM == 'Q')
        TYPE_OF_SORT = SORT_TYPES::QUICK;
    else if (SORTING_ALGORITHM == 'm' | SORTING_ALGORITHM == 'M')
        TYPE_OF_SORT = SORT_TYPES::MERGE;
    else if (SORTING_ALGORITHM == 'h' | SORTING_ALGORITHM == 'H')
        TYPE_OF_SORT = SORT_TYPES::HEAP;
    else if (SORTING_ALGORITHM == 'b' | SORTING_ALGORITHM == 'B')
        TYPE_OF_SORT = SORT_TYPES::BUBBLE;
    else if (SORTING_ALGORITHM == 's' | SORTING_ALGORITHM == 'S')
        TYPE_OF_SORT = SORT_TYPES::SELECTION;


    int low;
    int pivot = NUM_OF_ELEMENTS / NUM_OF_THREADS;
    for (int i = 0, j = 1; i < NUM_OF_THREADS; i++, j++) {
        low = i * pivot;

        // * * * * * * *  Divide array into sections. * * * * * * * * * * *

        /* This logic is applicable only for the cases when division
         * of elements in the array by the number of threads
         * doesn't produce equal sections
         *
         * 11 elements and 4 threads
         * 11/4 = 3
         * 0-2 3-5 6-8 9-the rest of the array */
        if (i == NUM_OF_THREADS - 1) {
            if (TYPE_OF_SORT != SORT_TYPES::INSERTION  && TYPE_OF_SORT != SORT_TYPES::HEAP
                    && TYPE_OF_SORT != SORT_TYPES::BUBBLE && TYPE_OF_SORT != SORT_TYPES::SELECTION) {
                indices[i][0] = low;
                indices[i][1] = NUM_OF_ELEMENTS - 1;
            } else {
                indices[i][0] = low;
                indices[i][1] = NUM_OF_ELEMENTS % (NUM_OF_THREADS) + pivot; // For selection sort we need the number of elements
            }
        } else {
            /* In case of an array with 20 elements and 4 threads
             * 0-4 5-9 10-14 15-19
             *
             * pivot = 20 / 4 = 5 */

            if (TYPE_OF_SORT != SORT_TYPES::INSERTION && TYPE_OF_SORT != SORT_TYPES::HEAP
                    && TYPE_OF_SORT != SORT_TYPES::BUBBLE && TYPE_OF_SORT != SORT_TYPES::SELECTION) {
                indices[i][0] = low;
                indices[i][1] = j * pivot - 1;
            } else {
                indices[i][0] = low;
                indices[i][1] = pivot; // For selection sort we need the number of elements
            }
        }
    }

#if TEST_MODE == 1
    cout << endl;
    for(int i = 0; i < 12; i++){
        cout << "index is " << i << " value are: " << indices[i][0] << " and " << indices[i][1] << endl;
    }
    cout << endl;
#endif

    populateArrayWithRandomInt(arr, NUM_OF_ELEMENTS, 0, NUM_OF_ELEMENTS);

#if DEBUG_MODE == 1
    printAllElements(NUM_OF_ELEMENTS, arr);
#endif

    setTime();

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        if (TYPE_OF_SORT == SORT_TYPES::MERGE)
            mergeSort(arr, indices[i][0], indices[i][1]);
        else if (TYPE_OF_SORT == SORT_TYPES::QUICK)
            quickSort(arr, indices[i][0], indices[i][1]);
        else if (TYPE_OF_SORT == SORT_TYPES::INSERTION)
            insertionSort(&arr[indices[i][0]], indices[i][1]);
        else if (TYPE_OF_SORT == SORT_TYPES::HEAP)
            heapSort(&arr[indices[i][0]], indices[i][1]);
        else if (TYPE_OF_SORT == SORT_TYPES::BUBBLE)
            bubbleSort(&arr[indices[i][0]], indices[i][1]);
        else if (TYPE_OF_SORT == SORT_TYPES::SELECTION)
            selectionSort(&arr[indices[i][0]], indices[i][1]);
    }

#if DEBUG_MODE == 1
    printAllElements(NUM_OF_ELEMENTS, arr);
#endif

    //One thread
    pivot = NUM_OF_ELEMENTS / NUM_OF_THREADS;
    for (int i = 0, j = 1; i < NUM_OF_THREADS - 1; i++, j++) {
        if ((NUM_OF_THREADS - 2) == i) {
            merge(arr, 0, (j * pivot) - 1, NUM_OF_ELEMENTS - 1);
        } else {
            merge(arr, 0, (j * pivot) - 1, (j + 1) * pivot - 1);
        }
    }
    cout << "The time spent with one thread is: " << getTime() << " ms" << endl;


#if DEBUG_MODE == 1
    printAllElements(NUM_OF_ELEMENTS, arr);
#endif

    //Make sure the sorted array is correct.
    if (assertSuccessSort(arr, NUM_OF_ELEMENTS) == 1) {
        printf("\t\tSort is accurate\n");
    } else {
        printf("\t\tSort is inaccurate\n");
    }

    /* n[] and *n = (int*)calloc(len, sizeof(int));
     * It is dangerous to use "fixed arrays" due their
     * allocation on the stack. For example, Visual Studio
     * defaults the stack size only to 1MB
     * +
     * the size must be known in advance [at compile time] */

    populateArrayWithRandomInt(arr, NUM_OF_ELEMENTS, 0, NUM_OF_ELEMENTS);
#if DEBUG_MODE_THREADS == 1
    printAllElements(NUM_OF_ELEMENTS, arr);
#endif
    pthread_t thr[NUM_OF_THREADS];  //contains info about each thread and behaves as a link to a thread.
    int rc;
    // Create a THREAD_DATA argument array
    THREAD_DATA thr_data[NUM_OF_THREADS];

    setTime();

    //Create Threads
    if (NUM_OF_THREADS <= NUM_OF_ELEMENTS) {
        for (int i = 0; i < NUM_OF_THREADS; i++) {

            thr_data[i].low = indices[i][0];
            thr_data[i].high = indices[i][1];

            thr_data[i].tid = i;
            /* int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg); */
            rc = pthread_create(&thr[i], NULL, thr_func, &thr_data[i]);
            if (rc > 0) {
                fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
                return EXIT_FAILURE;
            }
        }
    } else {
        cout << "Too many threads for too few elements";
    }

    //Block until all threads come back home...
    for (int i = 0; i < NUM_OF_THREADS; ++i) {
        pthread_join(thr[i], NULL);
    }

#if DEBUG_MODE_THREADS == 1
    printAllElements(NUM_OF_ELEMENTS, arr);
#endif

    pivot = NUM_OF_ELEMENTS / NUM_OF_THREADS;
    for (int i = 0, j = 1; i < NUM_OF_THREADS - 1; i++, j++) {
        if ((NUM_OF_THREADS - 2) == i) {
            merge(arr, 0, (j * pivot) - 1, NUM_OF_ELEMENTS - 1);
        } else {
            merge(arr, 0, (j * pivot) - 1, (j + 1) * pivot - 1);
        }
    }

    cout << "The time spent with " << NUM_OF_THREADS << " thread[s] is " << getTime() << " ms" << endl;

    if (assertSuccessSort(arr, NUM_OF_ELEMENTS) == 1) {
        printf("\t\tSort is accurate\n");
    } else {
        printf("\t\tSort is inaccurate\n");
    }


    /* * * * * * * * * * * * * * * * * * * * TEST RESULTS * * * * * * * * * * * * * * * * * * *

     1. Run InsertionSort using two threads with array sizes 10K, 100K and 300K.
                    1 thread [in ms]                                         2 threads [in ms]
     10 K           13|12|14|13|12|14                                        9|8|8|9|7|7
     100 K          1754|1600|1827|1824|1621                                 772|814|798|800|845
     300 K          11375|10962|11072|10822                                  5617|5621|5583|5944


     2. Run InsertionSort using four threads with an array size of 100K.
                     1 thread [in ms]                                         4 threads [in ms]
     100 K           981|907|1036|928|917                                     304|415|381|376383


     3. Run QuickSort using two threads with array sizes 1M, 10M and 100M.
                     1 thread [in ms]                                         2 threads [in ms]
     1M              133|263|128|133|208|130                                  73|215|81|61|73|90
     10M             1492|1494|1598|1701|1861                                 831|780|774|776|784
     100M            13606|13491|17184                                        8215|7352|8509


     4. Run QuickSort using four threads with an array size of 10M.
                     1 thread [in ms]                                         4 threads [in ms]
     10M             2003|2003|2143|1794|1511                                 634|623|629|626|673
     */

    return 0;
}