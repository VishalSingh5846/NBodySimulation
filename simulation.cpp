#include"simulation.h"

Body bodyList[N_BODY];

double TimeSpecToSeconds(struct timespec* ts){
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

double generateRandomNumber(double mi,double ma){
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
        bodyList[i].print("Generating Body");
    }
    
    // Body b2(100,0.3,Vec3(0,0,1),Vec3(2,0,-10),Vec3(0,0,0), Vec3(0,0,0));
    // bodyList[0] = b1;
    // bodyList[1] = b2;   
    // bodyList[0].print("Here");
}

double distance(Vec3 &p1, Vec3 &p2){
    return sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y) + (p1.z-p2.z)*(p1.z-p2.z));
}

Vec3 computeForce(Body &b1, Body &b2){
    double K = 1;
    double dist = distance(b1.position,b2.position);
    double mag = K * b1.mass * b2.mass / (dist * dist + 0.000001);
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
    for(int i=0; i<N_BODY; i++){
        modifyBodyPositionAndVelocity(bodyList[i]);
    }
}