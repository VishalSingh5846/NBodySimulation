#include <assert.h>
#include <iostream>
#include <string.h>
#include <omp.h>
#include<math.h>
#include <time.h>
#include<chrono>
using namespace std;

#define BENCHMARK true
#define TIME_STEP 0.0005

#define DEVICE_MEM true

#define MASS_MAX 20
#define MASS_MIN 10
#define RADIUS_MAX 0.01
#define RADIUS_MIN 0.01
#define X_MAX 2
#define X_MIN -2
#define Y_MAX 2
#define Y_MIN -2
#define Z_MAX -5
#define Z_MIN -5
#define VEL_MAX 0
#define VEL_MIN 0
#define ACC_MAX 0
#define ACC_MIN 0
#define CUDA_CALL(x) do {						\
    cudaError_t ____rc = (x);					\
    assert(____rc == cudaSuccess);					\
  } while (0)

int ITERATIONS;
int N_BODY;

struct Vec3{
    double x;
    double y;
    double z;
};
__device__ void magnitude(Vec3* vec, double* mag){
    *mag = sqrt(vec->x*vec->x + vec->y*vec->y + vec->z*vec->z);
}
__device__ void normalize(Vec3* vec){
    double mag;
    magnitude(vec, &mag);
    vec->x /= mag;
    vec->y /= mag;
    vec->z /= mag;
}
__device__ void multiply(Vec3* vec, double k){
    vec->x *= k;
    vec->y *= k;
    vec->z *= k;
}
__device__ void add(Vec3* vec1, Vec3* vec2){
    vec1->x += vec2->x;
    vec1->y += vec2->y;
    vec1->z += vec2->z;
}
void print(const char* str, Vec3* vec){
    printf("%s: (%f,%f,%f)\n",str,vec->x,vec->y,vec->z);
}

struct Body{
    double mass;
    double radius;
    Vec3 color;
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Body() {};
    Body(double mass, double radius, Vec3 color, Vec3 position, Vec3 velocity, Vec3 acceleration): color(color), velocity(velocity), position(position), acceleration(acceleration)  {
        this->mass = mass;
        this->radius = radius;
    };
    void print(const char* id){
        printf("%s -> Mass: %f, radius:%f, Color:(%f,%f,%f), Pos:(%f,%f,%f), Vel:(%f,%f,%f)\n", id,mass, radius, color.x, color.y, color.z, position.x, position.y, position.z, velocity.x, velocity.y, velocity.z);
    }
    void printPos(const char* id){
        printf("%s -> Pos:(%f,%f,%f), Vel:(%f,%f,%f)\n", id, position.x, position.y, position.z, velocity.x, velocity.y, velocity.z);
    }
};

int numThreads;
Body* bodyListH;
Body* bodyList;

__device__ double TimeSpecToSeconds(struct timespec* ts){
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

double generateRandomNumber(double mi,double ma){
    return  mi + (rand() / (1.0 * RAND_MAX)) * (ma - mi);
}

__host__ void generateBodies(Body* bodyList){
    for(unsigned int i=0; i<N_BODY; i++){
        Vec3 posVec;
        Vec3 velVec;
        Vec3 accVec;
        Vec3 colVec;

        colVec.x = rand() * 1.0 / RAND_MAX;
        colVec.y = rand() * 1.0 / RAND_MAX;
        colVec.z = rand() * 1.0 / RAND_MAX;

        posVec.x = generateRandomNumber(X_MIN, X_MAX);
        posVec.y = generateRandomNumber(Y_MIN, Y_MAX);
        posVec.z = generateRandomNumber(Z_MIN, Z_MAX);

        velVec.x = generateRandomNumber(VEL_MIN, VEL_MAX);
        velVec.y = generateRandomNumber(VEL_MIN, VEL_MAX);
        velVec.z = 0;

        accVec.x = generateRandomNumber(ACC_MIN, ACC_MAX);
        accVec.y = generateRandomNumber(ACC_MIN, ACC_MAX);
        accVec.z = 0;


        bodyList[i] = Body(
            generateRandomNumber(MASS_MIN, MASS_MAX),
            generateRandomNumber(RADIUS_MIN, RADIUS_MAX) , 
            colVec, 
            posVec, 
            velVec,  
            accVec
        );
        // bodyList[i].print("Generating Body");
    }
    
    // Body b2(100,0.3,Vec3(0,0,1),Vec3(2,0,-10),Vec3(0,0,0), Vec3(0,0,0));
    // bodyList[0] = b1;
    // bodyList[1] = b2;   
    // bodyList[0].print("Here");
}

__device__ double distance(Vec3 &p1, Vec3 &p2){
    return sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y) + (p1.z-p2.z)*(p1.z-p2.z));
}

__device__ void computeForce(Body &b1, Body &b2, Vec3* force){
    double K = 1;
    double dist = distance(b1.position,b2.position);
    double mag = K * b1.mass * b2.mass / (dist * dist + 0.000001);
    force->x = b2.position.x-b1.position.x;
    force->y = b2.position.y-b1.position.y;
    force->z = b2.position.z-b1.position.z;    
    normalize(force);
    // mag = 0;
    multiply(force,mag);
}

__device__ void modifyBodyPositionAndVelocity(Body &b1){
    b1.position.x += ( b1.velocity.x + 0.5 * b1.acceleration.x * TIME_STEP ) * TIME_STEP;
    b1.position.y += ( b1.velocity.y + 0.5 * b1.acceleration.y * TIME_STEP ) * TIME_STEP;
    b1.position.z += ( b1.velocity.z + 0.5 * b1.acceleration.z * TIME_STEP ) * TIME_STEP;

    b1.velocity.x += b1.acceleration.x * TIME_STEP;
    b1.velocity.y += b1.acceleration.y * TIME_STEP;
    b1.velocity.z += b1.acceleration.z * TIME_STEP;
}

__device__ void computeAcceleration(Body &b1, Vec3 force){
    b1.acceleration.x = force.x / b1.mass;
    // printf("Force:%f, Mass:%d, Force")
    b1.acceleration.y = force.y / b1.mass;
    b1.acceleration.z = force.z / b1.mass;
}

__global__ void computeInteractionShared(Body* bodyListGM, int N_BODY, int ITERATIONS){

    // printf(" Thread %d %d %d %d \n",threadIdx.x,blockIdx.x,blockDim.x,gridDim.x);
    int bodyId = threadIdx.x;
    int jump = blockDim.x;
    extern __shared__ Body bodyListS[];

    // for(int i= bodyId; i<N_BODY; i += jump ){
    //     bodyListS[i] = bodyListGM[i];
    // }
    memcpy(bodyListS, bodyListGM, sizeof(Body) * N_BODY);
    
    // __syncthreads();
    // return;

    for(int iter = 0 ; iter<ITERATIONS; iter++){
        
        for(int i = bodyId; i<N_BODY; i += jump ){
            Vec3 netForce;
            netForce.x = 0;
            netForce.y = 0;
            netForce.z = 0;
            Vec3 force;
            for(int j=0; j<N_BODY; j++){
                if(i==j) continue;
                computeForce(bodyListS[i],bodyListS[j], &force);
                add(&netForce, &force);
            }
            // bodyList[i].acceleration.print("ACC BEFORE");
            // netForce.print("Net Force");
            computeAcceleration(bodyListS[i], netForce);
            // bodyList[i].acceleration.print("ACC AFTER");

        }
        
        // __syncthreads();

        // for(int i= bodyId; i<N_BODY; i += jump ){
        //     modifyBodyPositionAndVelocity(bodyListS[i]);
        // }

    }
    for(int i= bodyId; i<N_BODY; i += jump ){
        bodyListGM[i] = bodyListS[i];
    }

}

__global__ void computeInteraction(Body* bodyList, int N_BODY, int ITERATIONS){

    // printf(" Thread %d %d %d %d \n",threadIdx.x,blockIdx.x,blockDim.x,gridDim.x);
    int bodyId = blockDim.x * blockIdx.x + threadIdx.x;
    int jump = blockDim.x * gridDim.x;

    for(int iter = 0 ; iter<ITERATIONS; iter++){
        
        for(int i = bodyId; i<N_BODY; i += jump ){
            Vec3 netForce;
            netForce.x = 0;
            netForce.y = 0;
            netForce.z = 0;
            Vec3 force;
            for(int j=0; j<N_BODY; j++){
                if(i==j) continue;
                computeForce(bodyList[i],bodyList[j], &force);
                add(&netForce, &force);
            }
            // bodyList[i].acceleration.print("ACC BEFORE");
            // netForce.print("Net Force");
            computeAcceleration(bodyList[i], netForce);
            // bodyList[i].acceleration.print("ACC AFTER");

        }
        
        __syncthreads();

        for(int i= bodyId; i<N_BODY; i += jump ){
            modifyBodyPositionAndVelocity(bodyList[i]);
        }
    }
}

void printDevProp(cudaDeviceProp devProp)
{
    printf("Major revision number:         %d\n",  devProp.major);
    printf("Minor revision number:         %d\n",  devProp.minor);
    printf("Name:                          %s\n",  devProp.name);
    printf("Total global memory:           %u\n", (unsigned int) devProp.totalGlobalMem);
    printf("Total shared memory per block: %u\n", (unsigned int) devProp.sharedMemPerBlock);
    printf("Total registers per block:     %d\n",  devProp.regsPerBlock);
    printf("Warp size:                     %d\n",  devProp.warpSize);
    printf("Maximum memory pitch:          %u\n", (unsigned int)  devProp.memPitch);
    printf("Maximum threads per block:     %d\n",  devProp.maxThreadsPerBlock);
    for (int i = 0; i < 3; ++i)
    printf("Maximum dimension %d of block:  %d\n", i, devProp.maxThreadsDim[i]);
    for (int i = 0; i < 3; ++i)
    printf("Maximum dimension %d of grid:   %d\n", i, devProp.maxGridSize[i]);
    printf("Clock rate:                    %d\n",  devProp.clockRate);
    printf("Total constant memory:         %u\n", (unsigned int) devProp.totalConstMem);
    printf("Texture alignment:             %u\n", (unsigned int) devProp.textureAlignment);
    printf("Concurrent copy and execution: %s\n",  (devProp.deviceOverlap ? "Yes" : "No"));
    printf("Number of multiprocessors:     %d\n",  devProp.multiProcessorCount);
    printf("Kernel execution timeout:      %s\n",  (devProp.kernelExecTimeoutEnabled ? "Yes" : "No"));
    return;
}

void setupCUDA(){
    CUDA_CALL(cudaSetDevice(0));

    int devCount;

    
    CUDA_CALL(cudaGetDeviceCount(&devCount));
    // CUDA_CALL(cudaMalloc(&bodyList, sizeof(Body) * N_BODY));
    
    if(DEVICE_MEM){
        bodyListH = new Body[N_BODY];
        generateBodies(bodyListH);
        CUDA_CALL(cudaMalloc(&bodyList, N_BODY * sizeof(Body)));
        CUDA_CALL(cudaMemcpy(bodyList, bodyListH, sizeof(Body) * N_BODY, cudaMemcpyHostToDevice));

    } else {
        CUDA_CALL(cudaMallocManaged(&bodyList, N_BODY * sizeof(Body)));
        generateBodies(bodyList);

    }
    

    
    // generateBodies(bodyListH);



    // Iterate through devices
    // for (int i = 0; i < devCount; ++i)
    // {
    //     // Get device properties
    //     printf("\nCUDA Device #%d\n", i);
    //     cudaDeviceProp devProp;
    //     cudaGetDeviceProperties(&devProp, i);
    //     printDevProp(devProp);
    // }

}

void printBodies(Body* lst){
    for(int i=0;i<N_BODY;i++){
        lst[i].print("");
    }
}


 
int main(int argc, char **argv){
    N_BODY = atoi(argv[1]);
    ITERATIONS = atoi(argv[2]);
    numThreads = atoi(argv[3]);

    int numThreadPerBlock = 128;
    int numBlocks = (numThreads % numThreadPerBlock == 0 ? 0 : 1) +  numThreads / numThreadPerBlock;
    numThreads = numThreads > numThreadPerBlock ? numThreadPerBlock : numThreads;
    printf("%d x %d = %d\n", numBlocks, numThreads, numBlocks*numThreads); 
    setupCUDA();
    // printBodies(bodyListH);
    printf("Total Memory: %.3f MB\n",sizeof(Body) * N_BODY / (1024.0 * 1024.0));
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    // for(int i=0;i<ITERATIONS;i++){
        // printf("-----\nIteration: %d\n",i);
        // for(int j=0;j<N_BODY;j++){
        //     bodyList[j].printPos("");
        // }
        computeInteraction<<<numBlocks,numThreads>>>(bodyList, N_BODY, ITERATIONS);
        // computeInteractionShared<<<1,atoi(argv[3]), sizeof(Body) * N_BODY>>>(bodyList, N_BODY, ITERATIONS);
        if(!DEVICE_MEM) cudaDeviceSynchronize();
        if(DEVICE_MEM) CUDA_CALL(cudaMemcpy(bodyListH, bodyList, sizeof(Body) * N_BODY, cudaMemcpyDeviceToHost));
        // printBodies(bodyListH);
        
    // }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    
    double elapsed =  std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
    printf("Iteration: %d\nBodies: %d\nTime: %.5f secs\n", ITERATIONS,N_BODY, elapsed);
    return 0;

}

