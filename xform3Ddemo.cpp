//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
//	OpenGL/ GLUT 'starter' code to demonstrate 3D transforms;
//					play with this to learn how GL_PROJECTION and GL_MODELVIEW
//					matrices affect how 3D vertices appear on-screen.
//	OVERVIEW:
//		OpenGL transforms all vertices first by GL_MODELVIEW matrix and then by
//		the GL_PROJECTION matrix before display on screen. These matrices act as
//		converters from 'model' space to 'world' space to 'eye' space:
//
//		model space --GL_MODELVIEW ---> world space --GL_PROJECTION--> eye space
//		(vertices)  --GL_MODELVIEW --->  (ground)   --GL_PROJECTION--> (film)
//		
//		Note that we're used to thinking of *everything* in world space, both
//		the models and the camera that views them. The behavior of MODELVIEW
//		matrix makes sense to most people--it transforms basic shapes to their 
//		world-space positions.  But the GL_PROJECTION is usually confusing; this
//		matrix transforms away from world-space and into eye-space coords. The 
//		origin of eye-space is the cameras position, the camera's direction of 
//		view is the -Z direction, and camera image x,y define eye-space x,y.  
//		The GL_PROJECTION matrix changes all world-space coordinates of each 
//		point so they are measured using those camera coords.
//			It is *dangerous* to think of 'GL_PROJECTION as the 'camera-
//		position-setting' matrix, because you are probably thinking of the 
//		*INVERSE* of GL_PROJECTION.  Remember, the camera is at the origin of
//		eye space; if we want to find the camera position in world space, we
//		must transform that eye-space origin BACKWARDS through GL_PROJECTION
//		to get back to world-space coordinates.
//
//	OPERATION:
//	  Draws 'world-space' axes as 3 colored lines: +x=RED, +y=GREEN, +z = BLUE
//	  Draws 'model-space' axes as 3 lines: +x = YELLOW, +y = CYAN, +z = PURPLE
//			(and a gray wire-frame teapot in model-space axes)
//		--MOUSE left-click/drag applies x,y glRotation() to GL_PROJECTION 
//		--MOUSE right-click/drag applies x,y glRotation() to GL_MODELVIEW
//		--ARROW keys applies x,y glTranslate() to GL_MODELVIEW
//		-- 'R' key to reset GL_MODELVIEW matrix to initial values.
//		-- 'r' key to reset GL_PROJECTION matrix to initial values.
//		-- 'm' key to enlarge the object.
//		-- 'n' key to shrink the object.
//		-- 'Q' key to quit.
//
//	To compile this under Microsoft Visual Studio (VC++ 6.0 or later) create
//		a new Project, Win32 Console Application, and make an 'empty' project.
//		Then add this file as 'source file', be sure you have the GLUT files
//		in the directory with this source code or installed on your machine.
//
//  If you don't have it, search the web for 'GLUT' --Marc Kilgard's
//		elegant little library that makes OpenGL platform-independent.  GLUT
//		uses 'callback' functions--you tell GLUT the names of your functions,
//		then it calls them whenever users do something that requires a response
//		from your program; for example, they moved the mouse, they resized a
//		window or uncovered it so that it must be re-drawn.
//
//  for CS 351, Northwestern University, Jack Tumblin, jet@cs.northwestern.edu
//
//	12/11/2004 - J. Tumblin--Created.
//	01/01/2005 - J. Tumblin--Added teapot; changed camera position to (0,3,5),
//								and field-of-view to 20 degrees.
//	10/20/2005 - J. Tumblin--Made GLUT local, updated comments for CS351 2005.
//  11/08/2005 - Modified by Seunghoon Kim for CS351 Project C.
//==============================================================================
#include <windows.h>						// Header File For Windows
#include <math.h>							// for sin(), cos(), tan(), etc.
//#include <stdlib.h>							// for all non-core C routines.
#include <stdio.h>							// for printf(), scanf(), etc.
#include <iostream>							// for cout, cin, etc.
//#include <assert.h>							// for error checking by ASSERT().
#include "glut.h"							// Mark Kilgard's GLUT library.
											// (Error here? be sure you have
											// glut.h, glut.dll, glut.lib
											// in your project directory.
											// What's GLUT? ask google...
#include <gl\GL.h>			// Header File For The OpenGL32 Library
#include <gl\GLU.h>			// Header File For The GLu32 Library
#include <gl\GLAux.h>		// Header File For The Glaux Library
//#include "glm.h"
#pragma comment(lib,"glut32.lib") 
#pragma comment(lib,"opengl32.lib")			// link OpenGL libraries for corresponding header files above
#pragma comment(lib,"glu32.lib") 
#pragma comment(lib,"glaux.lib") 

using namespace std;

#define JT_TITLE	"CS351 Project D by Seunghoon Kim - Use arrow keys and +/- to change the view"	// Display window's title bar:
#define JT_WIDTH	800						// window size in pixels
#define JT_HEIGHT	450
#define JT_XPOS		  0						// initial window position
#define JT_YPOS		256
#define JT_ZNEAR	1.0						// near, far clipping planes for
#define JT_ZFAR		50.0					// a 3D camera.

char* TEXTURE_FNAMES[] = {"data/0ball.bmp", "data/1ball.bmp", "data/2ball.bmp", "data/3ball.bmp", 
						  "data/4ball.bmp", "data/5ball.bmp", "data/6ball.bmp", "data/7ball.bmp", 
						  "data/8ball.bmp", "Data/9ball.bmp", "Data/pool.bmp", "Data/8ball.bmp"};
GLuint texture[12];			// Storage For One Texture ( NEW )

//====================
//
//	Function Prototypes  (these belong in a '.h' file if we have one)
//	
//====================

void glut_init(int *argc, char **argv);	// GLUT initialization
void ogl_init();						// OpenGL initialization

					// GLUT callback functions. Called when:
void display(void);						// GLUT says re-draw the display window.
void reshape(int w, int h);				// GLUT says window was re-sized to w,h
void keyboard(unsigned char key, int x, int y);	//GLUT says user pressed a key
void keySpecial(int key, int x, int y);	// GLUT says user pressed a 'special'key
//void mouseMove(int xpos,int ypos);		// GLUT says user moved the mouse to..
//void mouseClik(int,int,int,int);		// GLUT says user click/dragged mouse to
void DrawSphere(double xc, double yc, double zc, double r);
//void idle(void);
AUX_RGBImageRec *LoadBMP(char *Filename);
int LoadGLTextures();

class CTransRot
//==============================================================================
// Declares a new class that holds how much rotation and translation we want
// for a matrix (such as a GL_PROJECTION or GL_MODELVIEW matrix).  We'll make
// two instances of this class, one for each matrix we'll want to change with
// the mouse.
// PURPOSE:
// Record/accumulate offset amounts and rotation amounts from mouse & keyboard
{
public:
double	x_pos, y_pos, z_pos;	// cumulative position offset
double	x_rot, y_rot, z_rot;	// cumulative rotation on x,y,z axes
double  x_scale, y_scale, z_scale;	// scale for the object
double  x_trans, y_trans, z_trans;	// translation for the object

int		isDragging;				// TRUE if user is holding down the mouse button
								// that affects our value(s); else FALSE.
int m_x,m_y;					// last mouse-dragging position.

~CTransRot(void);				// default destructor
 CTransRot(void);				// default constructor
void reset(void);				// reset everything to zero.
void applyMatrix(void);			// apply translations, rotations to openGL.
};


//===================
//
// GLOBAL VARIABLES (bad idea!)
//
//====================
CTransRot setModel;			// Changes to initial GL_MODELVIEW matrix
CTransRot setProj;			// Changes to initial GL_PROJECTION matrix
unsigned char* image = NULL;
int iheight, iwidth = 256;

AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;										// If Load Failed Return NULL
}

int LoadGLTextures()						// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;									// Status Indicator

	AUX_RGBImageRec *TextureImage[12];					// Create Storage Space For The Texture

	memset(TextureImage,0,sizeof(void *)*12);           	// Set The Pointer To NULL

	glGenTextures(12, &texture[0]);					// Create The Texture

	for (int i=0; i<12; i++) {

		// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
		if (TextureImage[i]=LoadBMP(TEXTURE_FNAMES[i]))
		{
			Status=TRUE;									// Set The Status To TRUE

			// Typical Texture Generation Using Data From The Bitmap
			glBindTexture(GL_TEXTURE_2D, texture[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage[i]->sizeX, TextureImage[i]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[i]->data);
		}

		if (TextureImage[i])									// If Texture Exists
		{
			if (TextureImage[i]->data)							// If Texture Image Exists
			{
				free(TextureImage[i]->data);					// Free The Texture Image Memory
			}

			free(TextureImage[i]);								// Free The Image Structure
		}
	}

	return Status;										// Return The Status
}

int main(int argc, char** argv)
//------------------------------------------------------------------------------
{
	glut_init(&argc, argv);				// First initialize GLUT,
	ogl_init();							// Then initialize any non-default 
										// states we want in openGL,

	glutMainLoop();
	// Then give program control to GLUT.  This is an infinite loop, and from
	// within it GLUT will call the 'callback' functions below as needed.
	return 0;							// orderly exit.
}

//=====================
//
//  Other Function Bodies
//
//=====================

void glut_init(int *argc, char **argv)
//------------------------------------------------------------------------------
// A handy place to put all the GLUT library initial settings; note that we
// 'registered' all the function names for the callbacks we want GLUT to use.
{
	
	glutInit(argc, argv);				// GLUT's own internal initializations.

							// single buffered display, 
							//  RGB color model, use Z-buffering (depth buffer)
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(JT_WIDTH, JT_HEIGHT);	// set display-window size
	glutInitWindowPosition(JT_XPOS, JT_XPOS);	// and position,
	glutCreateWindow(JT_TITLE);					// then create it.

	// Register GLUT Callback function names. (these names aren't very creative)
	glutDisplayFunc(display);			// 'display'  callback:  display();
	glutKeyboardFunc(keyboard);			// 'keyboard' callback:  keyboard(); 
	glutSpecialFunc(keySpecial);		// 'special'keys callback: keyspecial()
	glutReshapeFunc(reshape);			// 'reshape'  callback:  reshape();
//	glutIdleFunc(idle);					// 'idle'	  callback:  idle(); 
// CAREFUL! WE DON'T NEED IDLE()!
//	glutMouseFunc(mouseClik);			// callbacks for mouse click, move
//	glutMotionFunc(mouseMove);		
}


void ogl_init()
//------------------------------------------------------------------------------
// A handy place to put all the OpenGL initial settings-- remember, you only 
// have to change things if you don't like openGL's default settings.
{
	LoadGLTextures();
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	
	glClearColor(0.0, 0.0, 0.0, 0.0);	// Display-screen-clearing color;
										// acts as 'background color'
	glColor3f(1.0, 1.0, 1.0);			// Select current color  for drawing
//	glShadeModel(GL_FLAT);				// Choose 'flat shading' model  
	glDisable(GL_LIGHTING);				// No lighting needed 

	GLfloat light_ambient[]  = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_shininess = 100;
    GLfloat light_position[] = {0.0, 10.0, 0.0, 0.0 };
	
//	glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv (GL_LIGHT0, GL_POSITION, light_position);
    glLightf (GL_FRONT, GL_SHININESS, light_shininess);

    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT0);

}


void reshape(int w, int h)
//------------------------------------------------------------------------------
// GLUT 'reshape' Callback. Called when user resizes the window from its current
// shape to one with width w, height h.
// We usually initialize (or re-initialize)openGL's 'GL_PROJECTION' matrix here.
{
	// set size of viewport to window size
	glViewport(0, 0, (GLsizei) w, (GLsizei) h); 

	// Set the projection matrix  
	glMatrixMode(GL_PROJECTION);		// Select the Projection matrix,

//***CHOOSE A CAMERA:***

/*
//----------------------------------------------------
	glLoadIdentity();		// (Clear out any previous camera settings)
	gluOrtho2D(0, w, 0, h);	// orthographic projection(left,right,bottom,top): 
							// using (0,w,0,h) maps x,y world space to screen
							// space in pixel units.
*/

//**OR**----------------------------------------------


	glLoadIdentity();			// (Clear out any previous camera settings)
	gluPerspective(				// Set camera's internal parameters: 
		45.0,					// vertical (y-axis) field-of-view in degrees,
		(double)w/(double)h,	// display image aspect ratio (width/height),
		JT_ZNEAR,JT_ZFAR);		// near,far clipping planes for camera depth.
 
//**OR**----------------------------------------------
/*
	glLoadIdentity();		// (Clear out any previous camera settings)
	gluPerspective(20.0,	// Set camera's vertical field-of-view  (y-axis)
							// measured in degrees, and set the display image
		(double)w/(double)h,// (width/height) and finally
		JT_ZNEAR,JT_ZFAR);	// set near, far clipping planes.
							// if GL_PERSPECTIVE matrix was identity before we
							// called gluPerspective, then we're at the world
							// space origin, but looking in the (world space)
							// -Z direction. 
							// (if current matrix is NOT identity, then the
							// current matrix M is pre-multiplied by the matrix
							// 'T' spec'd by gluPerspective: new matrix is MT).
							//
	// REMEMBER, all vertices are first multipled by the GL_MODELVIEW matrix,
	// and then by the GL_PROJECTION matrix before the 'viewport' maps them
	// to the display window.
	// REMEMBER when you call glTranslate() or glRotate() in openGL, existing 
	// GL_PROJECTION or GL_MODELVIEW is ***PRE_MULTIPLIED*** by the specified
	// translate or rotate matrix to make the GL_PROJECTION or GL_MODELVIEW
	// matrix!  This is *NOT* intuitive!
	//		THUS, if we call glTranslatef(0,-3,-5), then the world-space origin
	// is transformed to (0,-3,-5) *BEFORE* we apply the camera 
	// matrix that turns it into a picture:

		glTranslatef(0.0f, -3.0f, -5.0f);

	//		This gives you the same picture you'd get if you'd translated the
	// camera to the world-space location (0,+3,+5). Confusing, isn't it?!
	// Here's a good way to think of it; 
	//		1) the camera is at the origin of 'eye' space, and looking in the 
	//			-Z direction in 'eye' space.  
	//		2) The glTranslate(0,-3,-5) above converts world space coords to 
	//			eye-space coordinates. 
	// The INVERSE transform (e.g. glTranslate(0,+3,+5) converts eye-space
	// coords to world space.  The camera is always the origin of eye-space; 
	// if we transform the eye-space origin to world space, we find the camera's
	// world-space position is 0,+3,+5.

		glRotatef(30.0f, 0.0f, 1.0f, 0.0f);

	//		Similarly, if we next call glRotationf(30.0,0,1,0) (e.g. rotate by
	// 30 degrees around the y axis) the current contents of the GL_PROJECTION
	// matrix is again pre-multiplied by the new rotation matrix we made.  Any 
	// point in world space is rotated  (about the world-space origin) first, 
	// then translated to make eye-space coordinates (where the camera is at
	// the origin and looking down the -Z axis).  
	// Just as before, the INVERSE transform (eye-space-to-world space) tells
	// us the camera position in world space. Take the origin of eye space
	// (e.g. the camera position) translate(0,+3,+5) so now camera is at 0,3,5
	// and still looking in -Z direction towards origin. Next, rotate about
	// the Y axis by -30 degrees, causing the camera to swing around from the
	// Z axis towards the -X axis. 
*/

//**OR**-----------------------------------------------

	glLoadIdentity();			// (Clear out any previous camera settings)
	gluPerspective(				// Set camera's internal parameters: 
		20.0,					// vertical (y-axis) field-of-view in degrees,
		(double)w/(double)h,	// display image aspect ratio (width/height),
		JT_ZNEAR,JT_ZFAR);		// near,far clipping planes for camera depth.

	GLfloat pos[4] = {1.50, 1.00, 1.00, 0.00};

	gluLookAt(0.0, 5.0, 7.0,	// VRP: eyepoint x,y,z position in world space.
			   0.0, 0.0, 0.0,	// 'look-at' point--we're looking at origin.
								// (VPN = look-at-point - VRP)
			   0.0, 1.0, 0.0);	// VUP: view 'up' vector; set 'y' as up...
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	//*** SURPRISE****
	// the matrix made by gluLookAt() *POST-MULTIPLIES* the current matrix,
	// unlike the glRotate() and glTranslate() functions.

	// Puzzle: What would happen now if you called 'glTranslate(0,0,-10)?
	// can you explain what happens if you then call 'glRotate(30f,0,1,0)?

	// Initialize the modelview matrix to do nothing.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();					// Set it to 'do nothing'.
}



void display(void)
//------------------------------------------------------------------------------
// GLUT 'display' Callback.  GLUT calls this fcn when it needs you to redraw 
// the dislay window's contents.  Your program should never call 'display()',
// because it will confuse GLUT--instead, call glutPostRedisplay() if you need
// to trigger a redrawing of the screen.
{
	// Clear the frame-buffer  
	glClear(GL_COLOR_BUFFER_BIT);

// =============================================================================
// START DRAWING CODE HERE 
// =============================================================================
	glMatrixMode(GL_PROJECTION);	// select projection matrix,
	glPushMatrix();					// save current version, then
	setProj.applyMatrix();			// apply results of mouse, keyboard

	// Draw model-space axes:
	glMatrixMode(GL_MODELVIEW);		// select the modelview matrix,
	glPushMatrix();					// save current version, then
	setModel.applyMatrix();			// apply results of mouse, keyboard

	GLfloat material_Jd[] = {0.80, 0.80, 0.80, 1.00};
	GLfloat material_Je[] = {0.60, 0.60, 0.60, 0.60};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_Jd);
    glMaterialfv(GL_FRONT, GL_EMISSION, material_Je);

	glBindTexture(GL_TEXTURE_2D, texture[0]);		// draw the white ball
	DrawSphere(-2.0, 0.1, 0.0, 0.10);

	glBindTexture(GL_TEXTURE_2D, texture[9]);		// draw ball #9
	DrawSphere(0.6, 0.1, 0.0, 0.10);
	glBindTexture(GL_TEXTURE_2D, texture[1]);		// draw ball #1
	DrawSphere(0.2, 0.1, 0.0, 0.10);
	glBindTexture(GL_TEXTURE_2D, texture[2]);		// draw ball #2
	DrawSphere(0.4, 0.1, -0.1, 0.10);
	glBindTexture(GL_TEXTURE_2D, texture[3]);		// draw ball #3
	DrawSphere(0.4, 0.1, 0.1, 0.10);
	glBindTexture(GL_TEXTURE_2D, texture[4]);		// draw ball #4
	DrawSphere(0.6, 0.1, -0.2, 0.10);
	glBindTexture(GL_TEXTURE_2D, texture[5]);		// draw ball #5
	DrawSphere(0.6, 0.1, 0.2, 0.10);
	glBindTexture(GL_TEXTURE_2D, texture[6]);		// draw ball #6
	DrawSphere(0.8, 0.1, -0.1, 0.10);
	glBindTexture(GL_TEXTURE_2D, texture[7]);		// draw ball #7
	DrawSphere(0.8, 0.1, 0.1, 0.10);
	glBindTexture(GL_TEXTURE_2D, texture[8]);		// draw ball #8
	DrawSphere(1.0, 0.1, 0.0, 0.10);

	glPopMatrix();				// restore original MODELVIEW matrix.

	// Draw axes in world-space:	
	glLoadIdentity();			// wipe out current GL_MODELVIEW matrix so that
								// model-space vertices become world-space
								// vertices without change.


	GLfloat material_Ka[] = {0.30, 0.27, 0.12, 1.00};
	GLfloat material_Kd[] = {0.4, 0.4, 0.4, 0.60};
	GLfloat material_Ks[] = {0.98, 0.63, 0.19, 1.00};
	GLfloat material_Ke[] = {0.40, 0.40, 0.40, 0.40};
	GLfloat material_Se = 10;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_Kd);
    glMaterialfv(GL_FRONT, GL_EMISSION, material_Ke);

    glEnable (GL_BLEND);
    glDepthMask (GL_FALSE);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, texture[10]);
	glBegin(GL_QUADS);			// start drawing the table
		glTexCoord2f(0.0f, 0.0f); glVertex3f(3.0f, 0.0f, 2.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-3.0f, 0.0f, 2.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-3.0f, 0.0f, -2.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(3.0f, 0.0f, -2.0f);
	glEnd();					// end drawing lines

    glDepthMask (GL_TRUE);
    glDisable (GL_BLEND);

	glBegin(GL_LINES);			// start drawing lines:
		glColor3f ( 1.0, 0.0, 0.0);	// Red X axis
		glVertex3f( 0.0, 0.0, 0.0);	
		glVertex3f( 1.0, 0.0, 0.0);

		glColor3f ( 0.0, 1.0, 0.0);	// Green Y axis
		glVertex3f( 0.0, 0.0, 0.0);	
		glVertex3f( 0.0, 1.0, 0.0);

		glColor3f ( 0.0, 0.0, 1.0);	// Blue Z axis
		glVertex3f( 0.0, 0.0, 0.0);
		glVertex3f( 0.0, 0.0, 1.0);

	glEnd();					// end drawing lines
	glMatrixMode(GL_PROJECTION);// Restore the original GL_PROJECTION matrix
	glPopMatrix();
	// ================================================================================
	// END DRAWING CODE HERE 
	// ================================================================================
	
	cout << "Screen ReDrawn" << endl;
	glFlush();
}                                                                                                                                                                                 

/*
   Create a sphere centered at c, with radius r, and precision n
   Draw a point for zero radius spheres
   Use CCW facet ordering
   "method" is 0 for quads, 1 for triangles
      (quads look nicer in wireframe mode)
   Partial spheres can be created using theta1->theta2, phi1->phi2
   in radians 0 < theta < 2pi, -pi/2 < phi < pi/2
   Slight modified from the source code taken from http://astronomy.swin.edu.au/~pbourke/opengl/sphere/
*/

void DrawSphere(double xc, double yc, double zc, double r)
{
   int i,j,n;
   double t1,t2,t3;
   float e[3],p[3];
   n=100;
   double PI = 3.14260;

   for (j=0;j<n/2;j++) {
      t1 = (j*PI / (n/2)) - (PI/2);
      t2 = ((j+1)*PI / (n/2)) - (PI/2);

      glBegin(GL_TRIANGLE_STRIP);

      for (i=0;i<=n;i++) {
         t3 = 2*i*PI / n - (PI/2);

         e[0] = cos(t1) * cos(t3);
         e[1] = sin(t1);
         e[2] = cos(t1) * sin(t3);
         p[0] = xc + r * e[0];
         p[1] = yc + r * e[1];
         p[2] = zc + r * e[2];
         glNormal3f(e[0],e[1],e[2]);
         glTexCoord2f(i/(double)n,2*j/(double)n);
         glVertex3f(p[0],p[1],p[2]);

         e[0] = cos(t2) * cos(t3);
         e[1] = sin(t2);
         e[2] = cos(t2) * sin(t3);
         p[0] = xc + r * e[0];
         p[1] = yc + r * e[1];
         p[2] = zc + r * e[2];
         glNormal3f(e[0],e[1],e[2]);
         glTexCoord2f(i/(double)n,2*(j+1)/(double)n);
         glVertex3f(p[0],p[1],p[2]);
      }
      glEnd();
   }
}

void keyboard(unsigned char key, int x, int y)
//------------------------------------------------------------------------------
// GLUT 'keyboard' Callback.  User pressed an alphanumeric keyboard key.
// ('special' keys such as return, function keys, arrow keys? keyboardspecial)
{
	switch(key) {
		case 27: // Esc  
		case 'Q':
		case 'q':
			exit(0);		// Quit application  
			break;
		case '+':				// enlarge the object
			setProj.x_scale *= 1.1;
			setProj.y_scale *= 1.1;
			setProj.z_scale *= 1.1;
			break;
		case '-':					// shrink the object
			setProj.x_scale *= 0.9;
			setProj.y_scale *= 0.9;
			setProj.z_scale *= 0.9;
			break;
		case 'r':
			setProj.reset();
			break;
		case 'R':
			setModel.reset();
			break;
		default:
			printf("unknown key.  Try arrow keys, +, -, r, R, or q");
			break;
	}
	// We might have changed something. Force a re-display  
	glutPostRedisplay();
}

void keySpecial(int key, int x, int y)
//------------------------------------------------------------------------------
// GLUT 'special' Callback.  User pressed an non-alphanumeric keyboard key, such
// as function keys, arrow keys, etc.
{
static double x_pos, y_pos;

	switch(key)	
	{
		case GLUT_KEY_UP:		// up arrow key
			setProj.x_rot += 0.5;
			break;
		case GLUT_KEY_DOWN:		// dn arrow key
			setProj.x_rot -= 0.5;
			break;
		case GLUT_KEY_LEFT:		// left arrow key
			setProj.y_rot += 1.0;
			break;
		case GLUT_KEY_RIGHT:	// right arrow key
			setProj.y_rot -= 1.0;
			break;
		default:
			break;
	}
	printf("key=%d, setModel.x_pos=%f, setModel.y_pos=%f\n",
							key,setModel.x_pos,setModel.y_pos);
	// We might have changed something. Force a re-display  
	glutPostRedisplay();
}


/*
void idle(void)
//------------------------------------------------------------------------------
// GLUT 'idle' Callback. Called when OS has nothing else to do; a 'clock tick'.  
// Use 'idle' *ONLY IF* your program does anything that needs continual updates, even 
// when users are not pressing keys, then put code to do the updates here.
// If you need to redraw the screen after your update, don't forget to call
// glutPostRedisplay() too.
//
//			*** A COMMON MISTAKE TO AVOID: ***
// 'idle()' gets called VERY OFTEN.  If you register 'idle()' and leave the idle
// function empty, GLUT will waste most/all CPU time not otherwise used on
// useless calls to idle().  If idle() contains only glutPostRedisplay(), you
// will force GLUT to redraw the screen as often as possible--even if the 
// contents of the screen has not changed.  If your program ONLY changes screen 
// contents when user moves,clicks, or drags the mouse, presses a key, etc.,
// then you don't need idle() at all! Instead, call glutPostRedisplay() at the 
// end of each of the GLUT callbacks that change the screen contents.  
// Then you'll update the screen only when there is something new to show on it.
{

}
*/

//==============================================================================
// jt_transRot function bodies:

CTransRot::~CTransRot(void)
//------------------------------------------------------------------------------
// Default destructor
{
}

CTransRot::CTransRot(void)
//------------------------------------------------------------------------------
// Default constructor
{
	reset();						// set all values to zero.
}

void CTransRot::reset(void)
//------------------------------------------------------------------------------
// Set all values to zero.
{
	x_pos = 0.0; y_pos = 0.0; z_pos = 0.0;
	x_rot = 0.0; y_rot = 0.0; z_rot = 0.0;
	x_trans = 0.0; y_trans = 0.0; z_trans = 0.0;
	x_scale = 1.0; y_scale = 1.0; z_scale = 1.0;
}

void CTransRot::applyMatrix(void)
//------------------------------------------------------------------------------
// Apply rotations, then translations to the coordinate axes.  
// (Note OpenGL pre-multiplies matrices,
//   so commands appear to be in reverse order!)
{

	glTranslated(x_pos, y_pos, z_pos);	// OpenGL call to make a translate
									// matrix (see Blue Book) Uses 'd'
									// suffix to specify 'doubles' as arguments.

	glRotated(z_rot, 0.0, 0.0, 1.0);	// OpenGL call to make & apply a rotate
	glRotated(y_rot, 0.0, 1.0, 0.0);	// matrix; we want to rotate the coord
	glRotated(x_rot, 1.0, 0.0, 0.0);	// system FIRST in x, then y, then
										// z, BUT have to call them in
										// !!!REVERSE ORDER!!! because OpenGL
										//  glRotate, glTranslate each make
										// a matrix and then PRE_MULTIPLY the
										// existing 'active' matrix in storage
										// (e.g. the GL_MODELVIEW or 
										// GL_PROJECTION, depending on which
										// one you selected with 
										// 'glMatrixMode()'.
										// Note how we do all this in our
										// 'reshape()' callback function.

	glScaled(x_scale, y_scale, z_scale);	

}
