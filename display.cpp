#define WIDTH 1280
#define HEIGHT 720
#define POLYGON_DEGREE 20
#define PI 3.1437

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

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
	glClearColor(1.0, 1.0, 1.0, 0.0);
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
    computeInteraction();
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
    generateBodies();
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMainLoop();
	return 0;
}
 

 
