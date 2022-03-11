#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <iostream>
#include <string.h>
#include<math.h>
#include <time.h>
#include <omp.h>
using namespace std;


#define WIDTH 1280
#define HEIGHT 720
#define POLYGON_DEGREE 20
#define PI 3.1437

#define BENCHMARK true
#define TIME_STEP 0.000005
#define ITERATIONS 100
#define N_BODY 900

#define MASS_MAX 20
#define MASS_MIN 10
#define RADIUS_MAX 0.05
#define RADIUS_MIN 0.03
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



struct Vec3{
    double x;
    double y;
    double z;
};
void magnitude(Vec3* vec, double* mag){
    *mag = sqrt(vec->x*vec->x + vec->y*vec->y + vec->z*vec->z);
}
void normalize(Vec3* vec){
    double mag;
    magnitude(vec, &mag);
    vec->x /= mag;
    vec->y /= mag;
    vec->z /= mag;
}
void multiply(Vec3* vec, double k){
    vec->x *= k;
    vec->y *= k;
    vec->z *= k;
}
void add(Vec3* vec1, Vec3* vec2){
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
};

Body* bodyListH; 
Body* bodyList;

double TimeSpecToSeconds(struct timespec* ts){
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

double generateRandomNumber(double mi,double ma){
    return  mi + (rand() / (1.0 * RAND_MAX)) * (ma - mi);
}

void generateBodies(Body* bodyList){
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

double distance(Vec3 &p1, Vec3 &p2){
    return sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y) + (p1.z-p2.z)*(p1.z-p2.z));
}

void computeForce(Body &b1, Body &b2, Vec3* force){
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

void computeInteraction(Body* bodyList){

    for(int i=0; i<N_BODY; i++){
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
    for(int i=0; i<N_BODY; i++){
        modifyBodyPositionAndVelocity(bodyList[i]);
    }
}


void drawSingleBody(Body &body){
    // body.print("Drawing Body");
    glLoadIdentity();
    glTranslatef(body.position.x, body.position.y, body.position.z);
    glColor3f(body.color.x,body.color.y,body.color.z);  //Some type of blue
    glBegin(GL_POLYGON);
    for(double i = 0; i < 2 * PI; i += PI / POLYGON_DEGREE) //<-- Change this Value
        glVertex3f(cos(i) * body.radius, sin(i) * body.radius, 0.0);
    glEnd();
}

void drawBodies(){
    for(int i=0; i<N_BODY; i++)
        drawSingleBody(bodyList[i]);
}


void init()
{
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    float T = 0;
	glClearColor(1.0*T, 1.0*T, 1.0*T, 0.0);
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)WIDTH, (GLsizei)HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void idle(){
    computeInteraction(bodyList);
    glutPostRedisplay();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    	
	drawBodies();
	glFlush();
	glutSwapBuffers();
}

int main(int argc, char **argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("N-Body Simulation");
    init();
    bodyList = new Body[N_BODY];
    generateBodies(bodyList);
    printf("Initi\n");
    glutIdleFunc(idle);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMainLoop();
	return 0;
}

 
