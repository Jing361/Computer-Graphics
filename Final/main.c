#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include<math.h>
#include<stdio.h>
#include<stdbool.h>

// The robot arm is specified by (1) the angle that the upper arm makes
// relative to the x-axis, called shoulderAngle, and (2) the angle that the
// lower arm makes relative to the upper arm, called elbowAngle.  These angles
// are adjusted in 5 degree increments by a keyboard callback.

static int shoulderAngle = 0, elbowAngle = 0;
const float shoulderDelta = .5, elbowDelta = .5;
static float cameraAngleX = 0, cameraAngleZ = 0;
static float cameraPosX = 0, cameraPosY = 0, cameraPosZ = 0;
const GLfloat orthoSize = 150.0;
static bool isAnimating = false;
enum AnimState {ARM_UP, ARM_DOWN};
static enum AnimState armState = ARM_UP;
GLdouble camLensSize[] = {1.4, 1.4, 1};
GLdouble camBodySize[] = {3, 4, 6};
GLdouble camHingeSize[] = {1, 1, 1};

GLdouble shaftBodySize[] = {2, 2, 20};
GLdouble shaftHingeSize[] = {2, 1, 1};

GLdouble baseSize[] = {12, 20, 2};
//GLdouble baseHingeSize[] = {3, 3, 2};

GLdouble deskTopSize[] = {30, 48, 2};
GLdouble deskLegSize[] = {4, 4, 28};

GLfloat qaBlack[] = {0.0, 0.0, 0.0, 1.0};
GLfloat qaWhite[] = {1.0, 1.0, 1.0, 1.0};
GLfloat qaRed[]   = {1.0, 0.0, 0.0, 1.0};
GLfloat qaGreen[] = {0.0, 1.0, 0.0, 1.0};
GLfloat qaBlue[]  = {0.0, 0.0, 1.0, 1.0};
GLfloat qaColor[]	= {.5, .5, .5, 1.0};

GLfloat qaLowAmbient[]    = {0.2, 0.2, 0.2, 1.0};
GLfloat qaFullAmbient[]   = {1.0, 1.0, 1.0, 1.0};

// Handles the keyboard event: the left and right arrows bend the docuCam, the
// up and down keys bend the shaft.
void special(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_LEFT:
    	elbowAngle += 5;
    	elbowAngle %= 360;
    	break;
    case GLUT_KEY_RIGHT:
    	elbowAngle -= 5;
    	elbowAngle %= 360;
    	break;
    case GLUT_KEY_UP:
    	shoulderAngle += 5;
    	shoulderAngle %= 360;
    	break;
    case GLUT_KEY_DOWN:
    	shoulderAngle -= 5;
    	shoulderAngle %= 360;
    	break;
    default:
    	return;
  }
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
    //up
    case ',':
  	//case 'w':
    	cameraAngleZ += 5;
    	if(cameraAngleZ > 360)
    		cameraAngleZ -= 360;
    	break;
    //down
    case 'o':
  	//case 's':
    	cameraAngleZ -= 5;
    	if(cameraAngleZ < 0)
    		cameraAngleZ += 360;
    	break;
    //left
    case 'a':
  	//case 'a':
    	cameraAngleX -= 5;
    	if(cameraAngleZ < 0)
    		cameraAngleZ += 360;
    	break;
    //right
    case 'e':
  	//case 'd':
    	cameraAngleX += 5;
    	if(cameraAngleZ > 360)
    		cameraAngleZ -= 360;
    	break;
    case 'r':
  	//case 'o':
    	cameraPosX += 1;
    	break;
    case 'n':
  	//case 'l':
    	cameraPosX -= 1;
    	break;
    case 't':
  	//case 'k':
    	cameraPosY += 1;
    	break;
    case 's':
  	//case ';':
    	cameraPosY -= 1;
    	break;
    case ';':
  	//case 'q':
    	cameraPosZ += 1;
    	break;
    case '.':
  	//case 'e':
    	cameraPosZ -= 1;
    	break;
    case 'c':
    //case 'i':
    	isAnimating = true;
    	if(armState == ARM_UP){
    		armState = ARM_DOWN;
    	} else if(armState == ARM_DOWN){
    		armState = ARM_UP;
    	}
    	break;
    case 'l':
    	break;
    default:
    	return;
  }
}

// wireBox(w, h, d) makes a wireframe box with width w, height h and
// depth d centered at the origin.  It uses the GLUT wire cube function.
// The calls to glPushMatrix and glPopMatrix are essential here; they enable
// this function to be called from just about anywhere and guarantee that
// the glScalef call does not pollute code that follows a call to myWireBox.
void wireBox(GLdouble width, GLdouble height, GLdouble depth) {
  glPushMatrix();
  glScalef(width, height, depth);
  //glutWireCube(1.0);
  glutSolidCube(1.0);
  glPopMatrix();
}

/*void wireTriangles(GLdouble width, GLdoubl, height, GLdouble depth){
  glPushMatrix();
  glScalef(width, height, depth);
  glBegin(GL_TRIANGLES);
  glVertex3f();
  glEnd();
  glPopMatrix();
}*/

// Displays the arm in its current position and orientation.  The whole
// function is bracketed by glPushMatrix and glPopMatrix calls because every
// time we call it we are in an "environment" in which a gluLookAt is in
// effect.  (Note that in particular, replacing glPushMatrix with
// glLoadIdentity makes you lose the camera setting from gluLookAt).
void display() {
  const int x = 0;
  const int y = 1;
  const int z = 2;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);

  //setup desk material properties
  glMaterialfv(GL_FRONT, GL_AMBIENT, qaRed);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, qaGreen);
  glMaterialfv(GL_FRONT, GL_SPECULAR, qaBlue);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, qaLowAmbient);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  //start with desk, base, shaft, camera, lens
  //add desk legs
  //color all legs red
  glColor3f(1.0, 0.0, 0.0);
  //Place legs in position
  glTranslatef(deskTopSize[x]/2 - deskLegSize[x]/2, (deskTopSize[y]/2 - deskLegSize[y]/2), -(deskLegSize[z]/2 + deskTopSize[z]/2));
  wireBox(deskLegSize[x], deskLegSize[y], deskLegSize[z]);
  //shift legs in x
  glTranslatef(-(deskTopSize[x] - deskLegSize[x]), 0, 0);
  wireBox(deskLegSize[x], deskLegSize[y], deskLegSize[z]);
  //shift legs in y
  glTranslatef(0, -(deskTopSize[y] - deskLegSize[y]), 0);
  wireBox(deskLegSize[x], deskLegSize[y], deskLegSize[z]);
  //shift legs back in x
  glTranslatef((deskTopSize[x] - deskLegSize[x]), 0, 0);
  wireBox(deskLegSize[x], deskLegSize[y], deskLegSize[z]);

  //add desk top
  //color desktop yellow
  glColor3f(1.0, 1.0, 0.0);
  //position the desktop on top of legs
  glTranslatef(-deskTopSize[x]/2 + deskLegSize[x]/2, deskTopSize[y]/2 - deskLegSize[y]/2, deskLegSize[z]/2 + deskTopSize[z]/2);
  wireBox(deskTopSize[x], deskTopSize[y], deskTopSize[z]);

  //add base board
  //color baseboard green
  glColor3f(0.0, 1.0, 0.0);
  //move the baseboard on top of the desk, accounting for desktop and base sizes
  glTranslatef(0, 0, deskTopSize[z]/2 + baseSize[z]/2);
  glRotatef(90, 0,0,1);
  wireBox(baseSize[x], baseSize[y], baseSize[z]);

  // Draw the shaft, rotated shoulder degrees about the x-axis.  Note that
  // the thing about glutWireBox is that normally its origin is in the middle
  // of the box, but we want the "origin" of our box to be at the left end of
  // the box, so it needs to first be shifted half the size of the base units
  // in the x direction, then rotated.

  //add shaft and shaft hinge
  glColor3f(0.0, 0.0, 1.0);
  //position the shaft above the base board, and at the 'head' of the baseboard
  glTranslatef(0.0, 0.6 * baseSize[x], baseSize[z]/2);
  glRotatef((GLfloat)shoulderAngle, 1.0, 0.0, 0);
  glTranslatef(0.0, 0.0, shaftBodySize[z]/2);
  wireBox(shaftBodySize[x], shaftBodySize[y], shaftBodySize[z]);

  //position shaft hinge up most of the shaft
  glTranslatef(0, -shaftBodySize[y]/2, (0.8 * shaftBodySize[z])/2);
  wireBox(shaftHingeSize[x], shaftHingeSize[y], shaftHingeSize[z]);

  // Since the camera is attached to the shaft we put the code here so that
  // all rotations we do are relative to the rotation that we already made
  // above to orient shaft.  So, we want to rotate elbow degrees about
  // the x-axis.  But, like before, the anchor point for the rotation is at the end of the box, so
  // we translate before rotating.
  //add camera
  glColor3f(0.0, 1.0, 1.0);
  glRotatef((GLfloat)elbowAngle, 1.0, 0.0, 0.0);
  //align shaft hinge to camera hinge
  glTranslatef(-1, 0, 0);
  glTranslatef(shaftHingeSize[x]/2 + camHingeSize[x]/2, 0, 0);
  wireBox(camHingeSize[x], camHingeSize[y], camHingeSize[z]);
  glTranslatef(camBodySize[x]/2 + camHingeSize[x]/2, 0, 0);
  wireBox(camBodySize[x], camBodySize[y], camBodySize[z]);
  glTranslatef(0, 0, -(camBodySize[z]/2 + camLensSize[z]/2));
  wireBox(camLensSize[x], camLensSize[y], camLensSize[z]);

  glPopMatrix();
  glLoadIdentity();
  gluLookAt(10 * cos(cameraAngleX * (M_PI/180)) + cameraPosX,\
		    10 * sin(cameraAngleX * (M_PI/180)) + cameraPosY,\
		    10 * sin(cameraAngleZ * (M_PI/180)) + cameraPosZ,\
		    cameraPosX,cameraPosY,cameraPosZ,\
		    0,0,1);

  glutSwapBuffers();

  //Process Animation, resetting the docuCam in the upright position
  if(isAnimating == true){
	if((shoulderAngle == 0 || elbowAngle == 360)
	&& (elbowAngle == 360 || elbowAngle == 0)){
	  isAnimating = false;
	}else{
	  shoulderAngle -= shoulderDelta;
	  shoulderAngle %= 360;
	  elbowAngle += elbowDelta;
	  elbowAngle %= 360;
	}
  }
}

// Handles the reshape event by setting the viewport so that it takes up the
// whole visible region, then sets the projection matrix to something reason-
// able that maintains proper aspect ratio.
void reshape(GLint w, GLint h) {
  const GLfloat size = 150.0;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-size, size, -size, size, -size, size);
}

// Performs application specific initialization: turn off smooth shading,
// sets the viewing transformation once and for all.  In this application we
// won't be moving the camera at all, so it makes sense to do this.
void init() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-orthoSize, orthoSize, -orthoSize, orthoSize, -orthoSize, orthoSize);

  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  GLfloat qaAmbientLight[]  = {0.2, 0.2, 0.2, 1.0};
  GLfloat qaDiffuseLight[]  = {0.8, 0.8, 0.8, 1.0};
  GLfloat qaSpecularLight[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat qaLightPosition[] = {0.0, 0.0, 30.0, 1.0};

  //set-up the light colors and properties
  glLightfv(GL_LIGHT0, GL_AMBIENT, qaAmbientLight);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, qaDiffuseLight);
  glLightfv(GL_LIGHT0, GL_SPECULAR, qaSpecularLight);
  glLightfv(GL_LIGHT0, GL_POSITION, qaLightPosition);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(10 * cos(cameraAngleX * (M_PI/180)),\
		    10 * sin(cameraAngleX * (M_PI/180)),\
		    10 * sin(cameraAngleZ * (M_PI/180)),\
		    0,0,0,\
		    0,0,1);
}

//render loop
void Timer(int unused){
	glutPostRedisplay();
	glutTimerFunc(30, Timer, 0);
}

// Initializes GLUT, the display mode, and main window; registers callbacks;
// does application initialization; enters the main event loop.
int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowPosition(80, 80);
  glutInitWindowSize(800, 600);
  glutCreateWindow("CS 6060");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutSpecialFunc(special);
  glutKeyboardFunc(keyboard);
  init();
  Timer(0);
  glutMainLoop();
  return 0;
}
