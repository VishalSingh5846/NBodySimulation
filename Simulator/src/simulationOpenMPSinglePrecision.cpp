#define BENCHMARK true
#define TIME_STEP 0.0005

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


#include <iostream>
#include <string.h>
#include <omp.h>
#include<math.h>
#include <time.h>
#include<chrono>

using namespace std;

int ITERATIONS;
int N_BODY;
struct Vec3{
    float x;
    float y;
    float z;
    Vec3(float x, float y, float z){
        this->x = x;
        this->y = y;
        this->z = z;
    };
    float magnitude(){
        return sqrt(x*x + y*y + z*z);
    }
    void normalize(){
        float mag = this->magnitude();
        this->x /= mag;
        this->y /= mag;
        this->z /= mag;
    }
    void multiply(float k){
        this->x *= k;
        this->y *= k;
        this->z *= k;
    }
    void add(Vec3 vec){
        this->x += vec.x;
        this->y += vec.y;
        this->z += vec.z;
    }
    void print(const char* str){
        printf("%s: (%f,%f,%f)\n",str,this->x,this->y,this->z);
    }
};
struct Body{
    float mass;
    float radius;
    Vec3 color;
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Body(): mass(0), radius(0), color(Vec3(0,0,0)), position(Vec3(0,0,0)), velocity(Vec3(0,0,0)), acceleration(Vec3(0,0,0)) {};
    Body(float mass, float radius, Vec3 color, Vec3 position, Vec3 velocity, Vec3 acceleration): color(color), velocity(velocity), position(position), acceleration(acceleration)  {
        this->mass = mass;
        this->radius = radius;
    };
    void print(const char* id){
        printf("%s -> Mass: %f, radius:%f, Color:(%f,%f,%f), Pos:(%f,%f,%f), Vel:(%f,%f,%f)\n", id,mass, radius, color.x, color.y, color.z, position.x, position.y, position.z, velocity.x, velocity.y, velocity.z);
    }
};

int numThreads;
Body* bodyList;
float TimeSpecToSeconds(struct timespec* ts){
    return (float)ts->tv_sec + (float)ts->tv_nsec / 1000000000.0;
}

float generateRandomNumber(float mi,float ma){
    return  mi + (rand() / (1.0 * RAND_MAX)) * (ma - mi);
}

void generateBodies(){
    for(unsigned int i=0; i<N_BODY; i++){
        bodyList[i] = Body(
            generateRandomNumber(MASS_MIN, MASS_MAX),
            generateRandomNumber(RADIUS_MIN, RADIUS_MAX) , 
            Vec3( rand() * 1.0 / RAND_MAX, rand() * 1.0 / RAND_MAX , rand() * 1.0 / RAND_MAX), 
            Vec3( generateRandomNumber(X_MIN, X_MAX), generateRandomNumber(Y_MIN, Y_MAX), generateRandomNumber(Z_MIN, Z_MAX) ), 
            Vec3( generateRandomNumber(VEL_MIN, VEL_MAX), generateRandomNumber(VEL_MIN, VEL_MAX) ,0),  
            Vec3( generateRandomNumber(ACC_MIN, ACC_MAX), generateRandomNumber(ACC_MIN, ACC_MAX),0) 
        );
        // bodyList[i].print("Generating Body");
    }
    
    // Body b2(100,0.3,Vec3(0,0,1),Vec3(2,0,-10),Vec3(0,0,0), Vec3(0,0,0));
    // bodyList[0] = b1;
    // bodyList[1] = b2;   
    // bodyList[0].print("Here");
}

float distance(Vec3 &p1, Vec3 &p2){
    return sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y) + (p1.z-p2.z)*(p1.z-p2.z));
}

Vec3 computeForce(Body &b1, Body &b2){
    float K = 1;
    float dist = distance(b1.position,b2.position);
    float mag = K * b1.mass * b2.mass / (dist * dist + 0.000001);
    Vec3 force(b2.position.x-b1.position.x, b2.position.y-b1.position.y, b2.position.z-b1.position.z);
    force.normalize();
    // mag = 0;
    force.multiply(mag);
    return force;
}

void modifyBodyPositionAndVelocity(Body &b1){
    b1.position.x += ( b1.velocity.x + 0.5 * b1.acceleration.x * TIME_STEP ) * TIME_STEP;
    b1.position.y += ( b1.velocity.y + 0.5 * b1.acceleration.y * TIME_STEP ) * TIME_STEP;
    b1.position.z += ( b1.velocity.z + 0.5 * b1.acceleration.z * TIME_STEP ) * TIME_STEP;

    b1.velocity.x += b1.acceleration.x * TIME_STEP;
    b1.velocity.y += b1.acceleration.y * TIME_STEP;
    b1.velocity.z += b1.acceleration.z * TIME_STEP;
}

void computeAcceleration(Body &b1, Vec3 force){
    b1.acceleration.x = force.x / b1.mass;
    // printf("Force:%f, Mass:%d, Force")
    b1.acceleration.y = force.y / b1.mass;
    b1.acceleration.z = force.z / b1.mass;
}

void computeInteraction(){
    #pragma omp parallel for num_threads(numThreads)
    for(int i=0; i<N_BODY; i++){
        Vec3 netForce(0,0,0);
        for(int j=0; j<N_BODY; j++){
            if(i==j) continue;
            netForce.add(computeForce(bodyList[i],bodyList[j]));
        }
        // bodyList[i].acceleration.print("ACC BEFORE");
        // netForce.print("Net Force");
        computeAcceleration(bodyList[i], netForce);
        // bodyList[i].acceleration.print("ACC AFTER");

    }
    // exit(0);
    #pragma omp parallel for num_threads(numThreads)
    for(int i=0; i<N_BODY; i++){
        modifyBodyPositionAndVelocity(bodyList[i]);
    }
}

int main(int argc, char **argv){
        N_BODY = atoi(argv[1]);
        ITERATIONS = atoi(argv[2]);
        numThreads = atoi(argv[3]);
        bodyList = new Body[N_BODY];
        
        generateBodies();
        
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        
        for(int i=0;i<ITERATIONS;i++)
            computeInteraction();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    
        float elapsed =  std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
        
        printf("Iteration: %d\nBodies: %d\nTime: %.5f secs\n", ITERATIONS,N_BODY, elapsed);
        return 0;
}
 

 
