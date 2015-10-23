//
// Created by Victor Ignatenkov on 10/20/15.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <assert.h>

#define MAX_SIZE 100000000
#define MAX_THREADS 16

void *ThreadSort(void *param); // Thread start function
void Sort(int data[], int p, int r); // Main sorting function
void MultiCombine(int data[], int divs, int indices[MAX_THREADS][3]); // Combine multiple divisions
void Combine(int data[], int p, int q, int r); // Combine two divisions at a time
void InsertionSort(int data[], int p, int r);
void QuickSort(int data[], int p, int r);
int Partition(int data[], int p, int r);
void Swap(int& x, int& y);
bool IsSorted(int data[], int size);
int Rand(int x, int y);
void GenerateRandomData(int data[], int size);
long GetMilliSecondTime(struct timeb timeBuf);
void SetTime(void);
long GetTime(void);
void PrintData(int data[], int size, char* tilte);

long gRefTime = 0;
int data[MAX_SIZE];
int L[MAX_SIZE];
int R[MAX_SIZE];
char sortAlg;



int main(int argc, char *argv[])
{
    pthread_t tid[MAX_THREADS]; // the thread identifier
    pthread_attr_t attr[MAX_THREADS]; // set of attributes for the thread
    int arraySize, subarraySize, thrdCnt;
    int i, startIndx, endIndx;
    int indices[MAX_THREADS][3];

    if (argc != 4) {
        fprintf(stderr,"Usage: tsort <array size> <thread count> <sort alg: I for Insert, Q for Quick >\n");
        return -1;
    }

    arraySize = atoi(argv[1]);
    if (arraySize < 1 || arraySize > MAX_SIZE) {
        fprintf(stderr,"Array size must be between 1 and %d \n",MAX_SIZE);
        return -1;
    }

    thrdCnt = atoi(argv[2]);
    if (thrdCnt < 2 || thrdCnt > MAX_THREADS) {
        fprintf(stderr,"Number of threads must be between 2 and %d\n",MAX_THREADS);
        return -1;
    }

    sortAlg = argv[3][0];
    if(sortAlg == 'I')
        printf("Doing InsertionSort\n");
    else if(sortAlg == 'Q')
        printf("Doing QuickSort\n");
    else {printf("Invalid sorting algorithm %c\n", sortAlg); exit(1);}

    GenerateRandomData(data, arraySize);

    subarraySize = arraySize/thrdCnt;

    // Compute indices
    startIndx=0;
    endIndx = subarraySize-1;
    for(i=0; i<thrdCnt; i++)
    {
        indices[i][0] = i; // the thread number
        indices[i][1] = startIndx;
        if (i == thrdCnt-1) endIndx = arraySize-1;
        indices[i][2] = endIndx;

        // Set index values for the next division
        startIndx = endIndx + 1; // start index for next division is right after the end index of the current division
        endIndx += subarraySize;
    }

    // Sequential Part
    SetTime();
    for(i=0; i<thrdCnt; i++)
        Sort(data, indices[i][1], indices[i][2]);
    MultiCombine(data, thrdCnt, indices);

    if(IsSorted(data, arraySize) == true)
        printf("Correct sequential sorting\n");
    else
        printf("Incorrect sequential sorting ????\n");
    printf("Sequential sorting completed in %ld ms\n", GetTime());

    // Threaded Part
    GenerateRandomData(data, arraySize); // Refill array with random numbers

    SetTime();


    /**
     * Get default attributes
     *
     * pthread_attr_init is used to initialise a thread attributes structure, which can then
     * be passed to pthread_create.
     *
     * If you are creating threads with default attributes, you pass a NULL
     * pointer for the thread attributes argument
     * to pthread_init and there is no need to initialise
     * an attribute structure.

     * However, if you want to configure specific thread attributes, such as scheduling
     * policy, priority, concurrency level, then you must use pthread_attr_init to initialise
     * the attribute structure before manipulating it using the attribute accessors
     * functions (pthread_set... and pthread_get...) and passing it to the pthread_init
     * function.
     */
    for(i=0; i<thrdCnt; i++)
        pthread_attr_init(attr+i);

    for(i=0; i<thrdCnt; i++)
    {
//        	printf("Creating thread %d with id %d start %d and end %d\n",i, tid[i], indices[i][1], indices[i][2]);
        pthread_create(tid+i,attr+i,ThreadSort,indices[i]);
    }

    //Now wait for all threads to complete
    for(i=0; i<thrdCnt; i++)
        pthread_join(tid[i],NULL);

    MultiCombine(data, thrdCnt, indices);

    if(IsSorted(data, arraySize) == true)
        printf("Correct threaded sorting\n");
    else
        printf("Incorrect threaded sorting ????\n");
    printf("Threaded sorting completed in %ld ms\n", GetTime());
}
/***************************************************************/

// Thread start function
void *ThreadSort(void *param)
{
    // Unpack parameters
    int thrdNum = ((int*)param)[0];
    int start = ((int*)param)[1];
    int end = ((int*)param)[2];

    //printf("In thread %d: start = %d, end = %d\n", thrdNum, start, end);
    Sort(data, start, end);
    pthread_exit(0);
}
/***************************************************************/

void Sort(int data[], int start, int end)
{
    if(sortAlg == 'I')
        InsertionSort(data, start, end);
    else if(sortAlg == 'Q')
        QuickSort(data, start, end);
    else {printf("Invalid sorting algorithm %c\n", sortAlg); exit(1);}
}
/***************************************************************/

void MultiCombine(int data[], int divs, int indices_[MAX_THREADS][3])
{
    int indices[MAX_THREADS][2], newIndices[MAX_THREADS][2];
    int i, j, start, end, mid;

    for(i=0; i<divs; i++)
    {
        indices[i][0]=indices_[i][1];
        indices[i][1]=indices_[i][2];
    }

    while(divs > 1)
    {
        // Combine each division i with next division i+1
        // j is the new division number
        for(i=0, j=0; i<divs; i+=2, j++)
        {
            if (i+1 < divs) // if there is a division next to it
            {
                start = indices[i][0];
                mid = indices[i][1];
                end = indices[i+1][1];
                assert(indices[i+1][0] == indices[i][1]+1);
                Combine(data, start, mid, end);
                newIndices[j][0] = start;
                newIndices[j][1] = end;
            }
            else // an odd number of divsions
            {
                newIndices[j][0] = indices[i][0];
                newIndices[j][1] = indices[i][1];
            }
        }
        divs = j; //set number of divs to the new number after combining
        for (i=0; i<divs; i++)
        {
            indices[i][0] = newIndices[i][0];
            indices[i][1] = newIndices[i][1];
        }
    }
}
/***************************************************************/

void Combine(int data[], int p, int q, int r)
{
    int nl = q - p + 1;
    int nr = r - q;
    int i, j, k;
    int max;

    for(i=0; i<nl; i++)
        L[i] = data[p+i];
    for(j=0; j<nr; j++)
        R[j] = data[q+1+j];
    max = L[nl-1];
    if(R[nr-1] > max)
        max = R[nr-1];
    L[nl] = max + 1;
    R[nr] = max + 1;

    i = 0;
    j = 0;

    for(k=p; k <= r; k++)
    {
        if(L[i] < R[j])
        {
            data[k] = L[i];
            i++;
        }
        else
        {
            data[k] = R[j];
            j++;
        }
    }
}
/***************************************************************/

bool IsSorted(int data[], int size)
{
    int i;

    for (i = 0; i<(size - 1); i++)
        if (data[i] > data[i + 1])
            return false;
    return true;
}
/***************************************************************/

void GenerateRandomData(int data[], int size)
{
    int i;

    for (i = 0; i < size; i++)
        data[i] = rand();
}
/***************************************************************/

void InsertionSort(int data[], int p, int r)
{
    int i, j, temp;

    for (i = p + 1; i <= r; i++)
    {
        temp = data[i];
        for (j = i - 1; j >= p && data[j]>temp; j--)
            data[j + 1] = data[j];
        data[j + 1] = temp;
    }
}
/***************************************************************/

void QuickSort(int data[], int p, int r)
{
    int q;
    if(p >= r) return;
    q = Partition(data, p, r);
    QuickSort(data, p, q-1);
    QuickSort(data, q+1, r);
}
/***************************************************************/

int Partition(int data[], int p, int r)
{
    int i, j, x, pi;

    pi = Rand(p, r);
    Swap(data[r], data[pi]);

    x = data[r];
    i = p-1;
    for(j=p; j<r; j++)
    {
        if(data[j] < x)
        {
            i++;
            Swap(data[i], data[j]);
        }
    }
    Swap(data[i+1], data[r]);
    return i+1;
}
/***************************************************************/

void Swap(int& x, int& y)
{
    int temp = x;
    x = y;
    y = temp;
}
/***************************************************************/

int Rand(int x, int y)
{
    int range = y - x + 1;
    int r = rand() % range;
    r += x;
    return r;
}
/**************************************************************/

void PrintData(int data[], int size, char* title)
{
    int i;

    printf("\n %s: \n",title);
    for(i=0; i<size; i++)
    {
        printf(" %d", data[i]);
        if(i%10 == 9 && size > 10)
            printf("\n");
    }
}
/***************************************************************/

long GetCurrentTime(void)
{
    long crntTime=0;

    struct timeb timeBuf;
    ftime(&timeBuf);
    crntTime = GetMilliSecondTime(timeBuf);

    return crntTime;
}
/********************************************************************/

void SetTime(void)
{
    gRefTime = GetCurrentTime();
}
/********************************************************************/

long GetTime(void)
{
    long crntTime = GetCurrentTime();

    return (crntTime - gRefTime);
}
/********************************************************************/

long GetMilliSecondTime(struct timeb timeBuf)
{
    long mliScndTime;

    mliScndTime = timeBuf.time;
    mliScndTime *= 1000;
    mliScndTime += timeBuf.millitm;
    return mliScndTime;
}
/*****************************************************************************/

