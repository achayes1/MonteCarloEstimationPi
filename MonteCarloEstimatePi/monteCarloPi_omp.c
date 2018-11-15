#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define DEFAULT_THROWS 500000
#define PI     3.1415926535

/***  OMP ***/

void seedThreads();
int threadct = 1;

void seedThreads(unsigned int * C) {
    int my_thread_id;
    unsigned int seed;

    #pragma omp parallel private (seed, my_thread_id)
    {
        my_thread_id = omp_get_thread_num();

        //create seed on thread using current time
        unsigned int seed = (unsigned) time(NULL);

        //munge the seed using our thread number so that each thread has its
        //own unique seed, therefore ensuring it will generate a different set of numbers
        C[my_thread_id] = (seed & 0xFFFFFFF0) | (my_thread_id + 1);

        printf("Thread %d has seed %u\n", my_thread_id, C[my_thread_id]);
    }

}

int main(int argc, char** argv) {
    long numSamples = DEFAULT_THROWS;
    long numInCircle = 0;       //number of throws in the unit circle
    double x, y;                //hold x,y position of each sample 'throw'
		unsigned int seed;

    // We will use these to time our code
    double ompStartTime, ompStopTime;
    double time_spent;

    // take in how many 'throws', or samples
    if (argc > 1) {
        numSamples = strtol(argv[1], NULL, 10);
    }

		// take in threadcount
		if (argc > 2) {
				threadct =  atoi(argv[2]); //strtol(argv[2]);
		}
    unsigned int seeds[threadct];

    // print "problem size" for debugging
    //printf("Number of throws: %ld\n", numSamples);

    int tid;       // thread id when forking threads in for loop

    omp_set_num_threads(threadct);
    seedThreads(seeds);

    ompStartTime = omp_get_wtime();   //get start time for this trial
    int n;
/***  OMP ***/
#pragma omp parallel num_threads(threadct) default(none) \
        private(tid, seed, x, y) \
        shared(numSamples, seeds) \
        reduction(+: numInCircle)
    {
        tid = omp_get_thread_num();   // my thread id
        seed = seeds[tid];            // it is much faster to keep a private copy of our seed
				srand(seed);	              //seed rand_r or rand

        #pragma omp for private(n)
        for(n=0; n<numSamples; n++) {
				// generate randome numbers between 0.0 and 1.0
					x = (double)rand_r(&seed)/RAND_MAX;
					y = (double)rand_r(&seed)/RAND_MAX;

					if ( (x*x + y*y) <= 1.0 ) {
						numInCircle++;
				}
			}
    }
    double pi = 4.0 * (double)(numInCircle) / (numSamples);
        /***  OMP ***/
        ompStopTime = omp_get_wtime();

// completion of work
    time_spent = (double)(ompStopTime - ompStartTime); // /clocks per sec?

    printf("Calculation of pi using %ld samples: %15.14lf\n", numSamples, pi);
    printf("Accuracy of pi calculation: %lf\n", pi - PI);
    printf("Time spent: %15.12lf seconds\n", time_spent);

}
