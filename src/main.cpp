#include "ofMain.h"
#include "ofApp.h"

//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

//========================================================================
int main( int argc, char * argv[] ){
	ofGLWindowSettings s;
	s.setGLVersion( 3,2 );
	s.width = 1920;
	s.height = 1080;
	s.windowMode = ofWindowMode::OF_WINDOW;
	ofCreateWindow( s );

	//ofSetupOpenGL(1920,1080,OF_WINDOW);			// <-------- setup the GL context
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp( argc == 2 ? argv[1] : "-1" ));
}
