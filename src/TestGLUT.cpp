// TEST.cpp ///////////////////////////////////////////////////////////////////

#ifdef	WINDOWS
#include <windows.h>
#pragma	warning(disable:4267)
#endif

#include <fstream>
#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>

#include "rvrProgramObject.h"
#include"rvrVolumeRenderer.h"

#ifdef MACOSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <pthread.h>
void junk() {
  int i = pthread_getconcurrency();
};


using namespace std;



// defines ////////////////////////////////////////////////////////////////////

#define	WINDOW_WIDTH		800
#define	WINDOW_HEIGHT		600
#define	WINDOW_POSITION_X	-1
#define	WINDOW_POSITION_Y	-1

#define	IS_FULLSCREEN		0

#define	NEAR_CLIPPING_LENGTH	0.1f
#define	FAR_CLIPPING_LENGTH	500.1f

#define	FIELD_OF_VIEW		45.0f

#define	EYE_X		0.0f
#define	EYE_Y		0.0f
#define	EYE_Z		15.0f

#define	TARGET_X	0.0f
#define	TARGET_Y	0.0f
#define	TARGET_Z	0.0f

#define	UP_X		0.0f
#define	UP_Y		1.0f
#define	UP_Z		0.0f



// globals ////////////////////////////////////////////////////////////////////

int window_width;
int window_height;
int counter;

rvrVolumeRenderer *volume_renderer;



// volume renderer ////////////////////////////////////////////////////////////

#if 1

void InitVolumeRenderer()
{
  volume_renderer = new rvrVolumeRenderer(NEAR_CLIPPING_LENGTH,
					  FAR_CLIPPING_LENGTH,
					  300,     // number of layers
					  true,    // draw frame,
					  0.002f   // frame margin ratio
					  );

  // make sample volume data
  const int	sx = 128;
  const int	sy = 128;
  const int	sz = 256;

  vector<unsigned char>	data;
  data.resize( sx * sy * sz  );

  const double	kr = 0.16;
  const double	kd = 6.0;
	
  double	dx, dy, dz;
  for ( int z = 0; z < sz; z++ ) {
    cout << z << "/" << sz << "\r";
    dz = kr * ( z - ( sz / 2 ));
    for ( int y = 0; y < sy; y++ ) {
      dy = kr * ( y - ( sy / 2 ));
      for ( int x = 0; x < sx; x++ ) {
	dx = kr * ( x - ( sx / 2 ));
	double	r = sqrt( dx * dx + dy * dy + dz * dz );
	double	cos_theta = dz / r;
	double	phi = kd * ( r * r ) * exp( -r / 2 )
	  * ( 3 * cos_theta * cos_theta - 1 );
	double	c = phi * phi;
	if ( c > 255.0 ) {
	  c = 255.0;
	}
	data[ (x + sx * ( y + sy * z )) ]
	  //= data[ (x + sx * ( y + sy * z )) *2 +1 ]
	  = static_cast<unsigned char>( c );
      }
    }
  }
  
  volume_renderer->SetVolume( &data[ 0 ], sx, sy, sz );
  volume_renderer->SetLUT( rvrLUT() );

  float p0[] = {0.5, 0.5, 0.0};
  float p1[] = {1.5, 2.5, 3.0};
 // volume_renderer->SetBbox(p0, p1);
}

#endif

#if 0	// read from file

void
InitVolumeRenderer(const char *filename,
		   const int sx, const int sy, const int sz )
{
  volume_renderer = new rvrVolumeRenderer(NEAR_CLIPPING_LENGTH,
					  FAR_CLIPPING_LENGTH,
					  128,	// number of layers
					  true,	// draw frame,
					  0.02f	// frame margin ratio
					  );
  vector<unsigned char>	data;
  data.resize( sx * sy * sz );

  ifstream	f_in( filename, ios::binary );
  if ( f_in.fail()) {
    cerr << "InitVolumeRenderer(): cannot open " << filename << endl;
  }
  f_in.read( reinterpret_cast<char *>( &data[ 0 ] ), data.size());
  f_in.close();

  volume_renderer->SetVolume( &data[ 0 ], sx, sy, sz );
  volume_renderer->SetLUT( rvrLUT() );
}

#endif

// OpenGL settings ////////////////////////////////////////////////////////////

void
InitLighting( void )
{
  GLfloat light_position0[] = { 0.0, 10.0, 20.0, 0.0 };
  GLfloat light_ambient0[] = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat light_diffuse0[] = { 1.0, 1.0, 1.0, 1.0 };

  glLightfv( GL_LIGHT0, GL_POSITION, light_position0 );
  glLightfv( GL_LIGHT0, GL_AMBIENT, light_ambient0 );
  glLightfv( GL_LIGHT0, GL_DIFFUSE, light_diffuse0 );

  glDisable( GL_LIGHTING );
  glEnable( GL_LIGHT0 );
}

void
InitMiscGL( void )
{
  // clear color
  glClearColor( 0.1, 0.1, 0.1, 0.0 );

  // shading model
  glShadeModel( GL_SMOOTH );

  // depth test
  glEnable( GL_DEPTH_TEST );

  // culling
  glCullFace( GL_BACK );
  glEnable( GL_CULL_FACE );
}

// callback functions /////////////////////////////////////////////////////////

void
Display( void )
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  gluLookAt(EYE_X, EYE_Y, EYE_Z,
	    TARGET_X, TARGET_Y, TARGET_Z,
	    UP_X, UP_Y, UP_Z);

  glPushMatrix();
  glRotatef( counter*0.3f, 0.0f, 1.0f, 0.0f );
  glRotatef( counter*0.3f * 0.7f, 1.0f, 0.0f, 0.0f );
  glScalef( 5.0f, 5.0f, 8.0f );
  glTranslatef( -0.5f, -0.5f, -0.8f );
  volume_renderer->Draw();
  glPopMatrix();
  
  glutSwapBuffers();
}

void
Reshape( int w, int h )
{
  window_width = w;
  window_height = h;

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glViewport( 0, 0, window_width, window_height );

  gluPerspective( FIELD_OF_VIEW,
		  window_width / static_cast<double>( window_height ),
		  NEAR_CLIPPING_LENGTH, FAR_CLIPPING_LENGTH );

  glMatrixMode( GL_MODELVIEW );
}

void
Keyboard( unsigned char key, int x, int y )
{
  if ( key == 'q' || key == 3 || key == 27 ) {	// 3: Ctrl-C, 27: ESC
    exit( 0 );
  }
}

void
MouseButton( int button, int state, int x, int y )
{
  if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) {}
  if ( button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN ) {}
  if ( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN ) {}
}

void
Idle( void )
{
  counter++;
  glutPostRedisplay();
}

// main ///////////////////////////////////////////////////////////////////////

int
main( int argc, char **argv )
{
  // initialize variables
  window_width = WINDOW_WIDTH;
  window_height = WINDOW_HEIGHT;
  counter = 0;

  // initialize GLUT
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
  glutInitWindowSize( window_width, window_height );
  if ( WINDOW_POSITION_X >= 0 && WINDOW_POSITION_Y >= 0 ) {
    glutInitWindowPosition( WINDOW_POSITION_X, WINDOW_POSITION_Y );
  }
  glutCreateWindow( argv[ 0 ] );

  // callback functions
  glutDisplayFunc( Display );
  glutReshapeFunc( Reshape );
  glutKeyboardFunc( Keyboard );
  glutMouseFunc( MouseButton );
  glutIdleFunc( Idle );
  
  // initialize OpenGL settings
  InitLighting();
  InitMiscGL();

  // initialize volume renderer
  InitVolumeRenderer();

  // fullscreen
  if ( IS_FULLSCREEN ) {
    glutFullScreen();
  }
  
  // main loop
  glutMainLoop();

  return 0;
}
