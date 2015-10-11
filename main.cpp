#include <iostream>
#include <sys/timeb.h>
#include <random>

using namespace std;

#define DEBUG_MODE 0

int *arr;
long gRefTime;
int TYPE_OF_SORT;

typedef struct _thread_data_t {
    int tid;
    int low;
    int high;
} thread_data_t;


enum SORT_TYPES{
    INSERTION,
    QUICK,
    MERGE,
};




//TODO: Add ability to use only the main thread
//TODO: test your program in 2003 or 2009 lab
//TODO: add an array with indices
//TODO: 20 threads for 20 elements is not good - RESOLVE IT!!!

/**
 * Quick Sort
 */
int partition( int a[], int l, int r) {
    int pivot, i, j, t;
    pivot = a[l];
    i = l; j = r+1;

    while( 1)
    {
        do ++i; while( a[i] <= pivot && i <= r );
        do --j; while( a[j] > pivot );
        if( i >= j ) break;
        t = a[i]; a[i] = a[j]; a[j] = t;
    }
    t = a[l]; a[l] = a[j]; a[j] = t;
    return j;
}

void quickSort( int a[], int l, int r)
{
    int j;

    if( l < r )
    {
        // divide and conquer
        j = partition( a, l, r);
        quickSort( a, l, j-1);
        quickSort( a, j+1, r);
    }

}
/**
 * END of Quick Sort
 */



/**
 * Insertion Sort
 */
void insertionSort(int arr[], int length) {
    int i, j, tmp;
    for (i = 1; i < length; i++) {
        j = i;
        while (j > 0 && arr[j - 1] > arr[j]) {
            tmp = arr[j];
            arr[j] = arr[j - 1];
            arr[j - 1] = tmp;
            j--;
        }
    }
}
/**
 * END Insertion Sort
 */







long GetMilliSecondTime(struct timeb timeBuf)
{
    long mliScndTime;

    mliScndTime = timeBuf.time;
    mliScndTime *= 1000;
    mliScndTime += timeBuf.millitm;
    return mliScndTime;
}

long GetCurrentTime(void)
{
    long crntTime=0;

    struct timeb timeBuf;
    ftime(&timeBuf);
    crntTime = GetMilliSecondTime(timeBuf);

    return crntTime;
}

void setTime(void)
{
    gRefTime = GetCurrentTime();
}

long getTime(void)
{
    long crntTime = GetCurrentTime();

    return (crntTime - gRefTime);
}


void printAllElements(int numOfElements, int arr[]) {
    for (int j = 0; j < numOfElements; ++j) {
        printf("The value of element: %d - %d\n", j, arr[j]);
    }
    printf("\n");
}

void merge(int arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;

    /* create temp arrays */
    //int *L[n1], R[n2];
    int *L = (int*)calloc(n1, sizeof(int));
    int *R = (int*)calloc(n2, sizeof(int));

    /* Copy data to temp arrays L[] and R[] */
    for(i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for(j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];

    /* Merge the temp arrays back into arr[l..r]*/
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    /* Copy the remaining elements of L[], if there are any */
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    /* Copy the remaining elements of R[], if there are any */
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}


/* l is for left index and r is right index of the sub-array
  of arr to be sorted */
void mergeSort(int arr[], int l, int r)
{
    if (l < r)
    {
        int m = l+(r-l)/2; //Same as (l+r)/2, but avoids overflow for large l and h
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
        merge(arr, l, m, r);
    }
}





void* thr_func(void *arg){
    thread_data_t *data = (thread_data_t *)arg;

    if (TYPE_OF_SORT == SORT_TYPES::MERGE)
        mergeSort(arr, data->low, data->high);
    else if (TYPE_OF_SORT == SORT_TYPES::QUICK)
        quickSort(arr, data->low, data->high);
    else if (TYPE_OF_SORT == SORT_TYPES::INSERTION)
        insertionSort(&arr[data->low], data->high);


    printf("\nhello from thr_func, thread id: %d\n", data->tid);
    pthread_exit(NULL);
}


/**
 * Return 1 for success
 * Return -1 for failure
 *
 * Compare only 4 times.
 * [ 2   4    6    4   5]
 *    +    +    +    +
 *
 */
int assertSuccessSort(const int const *arr, const int length){

    int i = 0;
    while(i != length-1 ){
        if(arr[i] > arr[i+1]){
            return -1;
        }
        i++;
    }
    return 1;
}

/**
 * low inclusive [
 * high exclusive )
 */
void populateArrayWithRandomInt( int *&data,  const int len, int const low, int const high){

    data = (int*)calloc(len, sizeof(int));
    int temp = 0;
    while( temp < len){
        *(data+temp) = (rand()%(high-low)+low);
      //  *(data+temp) = rand();
        temp++;
    }
}


int main(int argc, char **argv)
{

    //Run QuickSort using two threads with array sizes 1M, 10M and 100M.

    int **indices;
    indices = (int**)(calloc(12, sizeof(int*)));
    for(int i = 0; i<12; i++){
        indices[i] = (int*)calloc(2, sizeof(int));
    }

#if DEBUG_MODE == 1
    for(int i = 0; i < 12; i++){
        for(int j = 0; j < 2; j++){
            indices[i][j] = j;
        }
    }
    for(int i = 0; i < 12; i++){
        cout << "index is " << i << " value are: " << indices[i][0] << " and " << indices[i][1] << endl;
    }

    if(argc < 3){
        cerr << "Please enter | array size [int between 1 and 100 000 000 | number of threads [int between 1 - 16 |\nsorting algorithm [l - insertion sort or q - quick sort] ";
        return -1;
    }
#endif


    const int NUM_THREADS = atoi(argv[1]);
    const int NUM_ELEMENTS = atoi(argv[2]);
    char SORTING_ALGORITHM = argv[3][0]; //[][0] in order to get the first character instead of a "c-string"

    if(SORTING_ALGORITHM == 'i' | SORTING_ALGORITHM == 'I')
        TYPE_OF_SORT = SORT_TYPES::INSERTION ;
    else if (SORTING_ALGORITHM == 'q' | SORTING_ALGORITHM == 'Q')
        TYPE_OF_SORT = SORT_TYPES::QUICK;
    else if (SORTING_ALGORITHM == 'm' | SORTING_ALGORITHM == 'M')
        TYPE_OF_SORT = SORT_TYPES::MERGE;



    int low;
    int pivot = NUM_ELEMENTS / NUM_THREADS;
    for (int i = 0, j = 1; i < NUM_THREADS; i++, j++) {
        low = i * pivot;

        /*
         * This case is only for the cases when division
         * of elements in the array by the number of threads
         * doesn't produce equal sections
         *
         * 11 elements and 4 threads
         * 11/4 = 3
         * 0-2 3-5 6-8 9-the rest of the array
         *
         */
        if (i == NUM_THREADS - 1) {
            indices[i][0] = low;
            indices[i][1] = NUM_ELEMENTS - 1;

        } else {
            /*
             * In case of an array with 20 elements and 4 threads
             * 0-4 5-9 10-14 15-19
             *
             * pivot = 20 / 4 = 5
             */
            indices[i][0] = low;
            indices[i][1] = j * pivot - 1;

        }
    }

    populateArrayWithRandomInt(arr, NUM_ELEMENTS, 0, NUM_ELEMENTS);

#if DEBUG_MODE == 1
    printAllElements(NUM_ELEMENTS, arr);
#endif

    setTime();

            for (int i = 0; i < NUM_THREADS; i++) {
                if (TYPE_OF_SORT == SORT_TYPES::MERGE)
                    mergeSort(arr, indices[i][0], indices[i][1]);
                else if (TYPE_OF_SORT == SORT_TYPES::QUICK)
                    quickSort(arr, indices[i][0], indices[i][1]);
                else if (TYPE_OF_SORT == SORT_TYPES::INSERTION)
                    insertionSort(&arr[indices[i][0]], indices[i][1]);
            }


#if DEBUG_MODE == 1
    printAllElements(NUM_ELEMENTS, arr);
#endif

        pivot = NUM_ELEMENTS / NUM_THREADS;
    for(int i = 0 ,j= 1; i < NUM_THREADS-1; i++, j++){
        int low;
        low = i * pivot;
        if((NUM_THREADS-2) == i){
            merge(arr, 0, (j*pivot)-1, NUM_ELEMENTS - 1);
        }else {
            merge(arr, 0, (j*pivot)-1, (j+1)*pivot-1);
        }
    }

#if DEBUG_MODE == 1
    printAllElements(NUM_ELEMENTS, arr);
#endif
    cout << "The time spent with one thread is: " << getTime() << endl;

    if(assertSuccessSort(arr, NUM_ELEMENTS) == 1){
        printf("Sort is accurate\n");
    } else {
        printf("Sort is inacurate\n");
    }


    //printAllElements(NUM_ELEMENTS, arr);



    /**
     * n[] and *n = (int*)calloc(len, sizeof(int));
     * It is dangerous to use "fixed arrays" due their
     * allocation on the stack. For example, Visual Studio
     * defaults the stack size only to 1MB
     * +
     * the size must be known in advance [at compile time]
     */
    populateArrayWithRandomInt(arr, NUM_ELEMENTS, 0, NUM_ELEMENTS);
    pthread_t thr[NUM_THREADS];
    int i, rc;
    /* create a thread_data_t argument array */
    thread_data_t thr_data[NUM_THREADS];

    /* create threads */

    setTime();

    if(NUM_THREADS <= NUM_ELEMENTS){

        int pivot = NUM_ELEMENTS / NUM_THREADS;

        for(int i = 0 ,j= 1; i < NUM_THREADS; i++, j++){

            thr_data[i].low = i * pivot;
            if(i == NUM_THREADS -1){
                thr_data[i].high = NUM_ELEMENTS - 1;
            }else {
                thr_data[i].high = j * pivot - 1;
            }
            thr_data[i].tid = i;
            /** --Signature of the function--
             *
             * int pthread_create(pthread_t *thread, pthread_attr_t *attr,
                       void *(*start_routine)(void *), void *arg);
             */
            rc = pthread_create(&thr[i], NULL, thr_func, &thr_data[i]);
            if(rc > 0){
                fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
                return EXIT_FAILURE;
            }
        }
    }else {
        cout << "Too many threads for too few elements";
    }

    /* block until all threads complete */
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(thr[i], NULL);
    }



   //printAllElements(NUM_ELEMENTS, arr);


    pivot = NUM_ELEMENTS / NUM_THREADS;
    for(int i = 0 ,j= 1; i < NUM_THREADS-1; i++, j++){
        int low;
        low = i * pivot;
        if((NUM_THREADS-2) == i){
            merge(arr, 0, (j*pivot)-1, NUM_ELEMENTS - 1);
        }else {
            merge(arr, 0, (j*pivot)-1, (j+1)*pivot-1);
        }
    }

    cout << "The time spent is: " << getTime() << endl;

    //printAllElements(NUM_ELEMENTS, arr);


    if(assertSuccessSort(arr, NUM_ELEMENTS) == 1){
        printf("Sort is accurate\n");
    } else {
        printf("Sort is inacurate\n");
    }


    printf("hello from the main()\n");



    return 0;
}



