#include"simulation.h"


int main(int argc, char **argv){
        struct timespec start;
        struct timespec end;
        if(clock_gettime(CLOCK_MONOTONIC, &start)){ printf("CLOCK ERROR"); }
        for(int i=0;i<ITERATIONS;i++)
            computeInteraction();
        if(clock_gettime(CLOCK_MONOTONIC, &end)){ printf("CLOCK ERROR"); }
        printf("Total Time: %.5f secs\n\n",TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start));
        return 0;
}
 

 
