#define BENCHMARK true
#define ITERATIONS 100
#define N_BODY 10000
#define TIME_STEP 0.000005

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
#include<math.h>
#include <time.h>
using namespace std;


struct Vec3{
    double x;
    double y;
    double z;
    Vec3(double x, double y, double z){
        this->x = x;
        this->y = y;
        this->z = z;
    };
    double magnitude(){
        return sqrt(x*x + y*y + z*z);
    }
    void normalize(){
        double mag = this->magnitude();
        this->x /= mag;
        this->y /= mag;
        this->z /= mag;
    }
    void multiply(double k){
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
    double mass;
    double radius;
    Vec3 color;
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Body(): mass(0), radius(0), color(Vec3(0,0,0)), position(Vec3(0,0,0)), velocity(Vec3(0,0,0)), acceleration(Vec3(0,0,0)) {};
    Body(double mass, double radius, Vec3 color, Vec3 position, Vec3 velocity, Vec3 acceleration): color(color), velocity(velocity), position(position), acceleration(acceleration)  {
        this->mass = mass;
        this->radius = radius;
    };
    void print(const char* id){
        printf("%s -> Mass: %f, radius:%f, Color:(%f,%f,%f), Pos:(%f,%f,%f), Vel:(%f,%f,%f)\n", id,mass, radius, color.x, color.y, color.z, position.x, position.y, position.z, velocity.x, velocity.y, velocity.z);
    }
};




void computeInteraction();
double TimeSpecToSeconds(struct timespec* ts);
