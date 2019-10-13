/*******************************************************************
		   Multi-Part Model Construction and Manipulation
********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "QuadMesh.h"

# define M_PI 3.14159265358979323846


const int meshSize = 16;    // Default Mesh Size
const int vWidth = 650;     // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

static double xPos = 0.0;
static double zPos = 0.0;
static int isON = 0;
static int direction = 1;
static int currentButton;
static unsigned char currentKey;
static double rotate_degree = 0.0;
static double prop_rotate_degree_update = 0.0;
static double sub_motion_h = 0.0;
static double sub_motion_v = 0.0;

// Lighting/shading and material properties for drone - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat drone_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat drone_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat drone_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat drone_mat_shininess[] = { 0.0F };

// A quad mesh representing the ground
static QuadMesh groundMesh;

// A cylinder for the tower of the sub
static GLUquadricObj *tower;

// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void timer(int value);
void instructions(void);
Vector3D ScreenToWorld(int x, int y);

void drawSub(void);
void drawBody(void);
void drawPropeller(void);
void drawTower(void);

int main(int argc, char** argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Assignment 1");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);
	glutTimerFunc(0, *timer, 0);


	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);   // This light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.6F, 0.6F, 0.6F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	// Set up ground quad mesh
	Vector3D origin = NewVector3D(-8.0f, 0.0f, 8.0f);
	Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh, meshSize, origin, 16.0, 16.0, dir1v, dir2v);

	Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
	Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);

	// Set up the bounding box of the scene
	// Currently unused. You could set up bounding boxes for your objects eventually.
	//Set(&BBox.min, -8.0f, 0.0, -8.0);
	//Set(&BBox.max, 8.0f, 6.0,  8.0);
}

void timer(int value) {
	if (isON)
		if(direction == 1)
		prop_rotate_degree_update += 10;
		else
		prop_rotate_degree_update -= 10;

	glutTimerFunc(16, *timer, 0);
	glutPostRedisplay();
}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set drone material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);

	//Push (copy) current CTM to matrix stack
	glPushMatrix();

	//T1: Translate the sub:
	// xPos in the X direction
	// sub_motion_v + 4.0 in the Y direction
	// zPos in the Z direction
	//CTM = T1
	glTranslatef(xPos, sub_motion_v+4.0, zPos);

	//R1: Rotate the sub upon the Y axis by "rotate_degree" degrees
	//CTM = T1 * R1
	glRotatef(rotate_degree, 0.0, 1.0, 0.0);
	
	//Draw submarine's individual parts using the default CTM
	drawSub();
	
	//Pop the original CTM from line 159 back in
	glPopMatrix();

	// Draw ground mesh
	DrawMeshQM(&groundMesh, meshSize);
	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawSub(void) {
	drawBody();
	//drawPropeller();
	//drawTower();
}

void drawBody(void) {
	// Creates body of sub

	//Copy CTM onto matrix stack
	glPushMatrix();

	//S1: Scale the body of the sub 
	//CTM: T1 * R1 * S1
	glScalef(6.0, 1.0, 1.0);
	
	//Draw the body of the sub using the CTM specified above
	glutSolidSphere(1.0, 60, 60);

	drawPropeller();

	//Pop back default matrix CTM: T1 * R1
	glPopMatrix();
}

void drawPropeller(void) {
	//Creates Propeller

	//Cope CTM onto matrix stack
	glPushMatrix();

	//T2: Translate the propeller 6 units relative to the origin of the sub
	//CTM: T1 * R1 * T2
	glTranslatef(1.0, 0.0, 0.0);
	
	//R2: Rotate the propeller so that it aligns with the rear of the sub
	//CTM: T1 * R1 * T2 * R2
	glRotatef(90, 0.0, 1.0, 0.0);

	//R3: Responsible for animation of the propeller via the "ptop_rotate_degree_update" global variable
	//CTM: T1 * R1 * T2 * R2 * R3
	glRotatef(prop_rotate_degree_update, 0.0, 0.0, 1.0);
	
	//S1: Scale the propeller so that it looks desirable
	//CTM: T1 * R1 * T2 * R2 * R3 * S1
	glScalef(0.1, 1.0, 0.1);
	
	//Draw the propeller
	glutSolidCube(2.0);
	
	//Pop back default matrix CTM: T1 * R1
	glPopMatrix();
}

void drawTower(void){
	glPushMatrix();

	glTranslatef(0.0, 0.2, 0.0);

	glRotatef(-90, 1.0, 0.0, 0.0);

	glScalef(1.0, 0.4, 1.0);
	
	tower = gluNewQuadric();
	gluQuadricDrawStyle(tower, GLU_LINE);
	gluCylinder(tower, 2.0, 1.5, 2.0, 1000, 1000);

	glPopMatrix();

}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 15.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	if (key == 's')
		isON = !isON;
	else if (key == 'f'){
		direction = 1;
		sub_motion_h -= 0.5;
		xPos -= cosf((-rotate_degree * M_PI) / 180);
		zPos -= sinf((-rotate_degree * M_PI) / 180);
	}
	else if (key == 'b') {
		direction = -1;
		sub_motion_h += 0.5;
		xPos += cosf((-rotate_degree * M_PI) / 180);
		zPos += sinf((-rotate_degree * M_PI) / 180);
	}
	glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{

	if (key == GLUT_KEY_LEFT)
		rotate_degree += 10;
	else if (key == GLUT_KEY_RIGHT)
		rotate_degree -= 10;
	else if (key == GLUT_KEY_UP)
		sub_motion_v += 0.5;
	else if (key == GLUT_KEY_DOWN)
		sub_motion_v -= 0.5;
	else if (key == GLUT_KEY_F1)
		instructions();
	glutPostRedisplay();   // Trigger a window redisplay
}

void instructions(void) {
	printf(" Controls: \n Up: Up arrow key \n Down: Down arrow key \n Rotate right: Right arrow key \n Rotate left: Left arrow key \n Forward: 'f' \n Backward: 'b' \n Toggle propeller: 's' \n");
}

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


Vector3D ScreenToWorld(int x, int y)
{
	// you will need to finish this if you use the mouse
	return NewVector3D(0, 0, 0);
}



