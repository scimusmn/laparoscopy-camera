#include <opencv2/opencv.hpp>
#include <X11/Xlib.h>
#include <iostream>
#include <string>

#include <sstream>

using namespace std;
using namespace cv;

int main( int argc, char** argv ) {
  // important constants
  string window_name = "Laparascopy";
    
  // get screen size
  Display* disp = XOpenDisplay(NULL);
  Screen* screen = DefaultScreenOfDisplay(disp);
  int screen_w = screen->width;
  int screen_h = screen->height;
  cout << "Screen size: " << screen_w << "x" << screen_h << endl;

  // open camera
  VideoCapture camera(0);
  if ( !camera.isOpened() ){
    cerr << "ERROR: could not open camera!" << endl;
    return -1;
  }
  camera.set(CAP_PROP_BUFFERSIZE, 1); // buffer only one frame
  cout << "opened camera" << endl;

  // create main window
  namedWindow(window_name, WINDOW_NORMAL | WINDOW_FULLSCREEN );

  // mats for pulling from the camera and displaying
  Mat frame, display;

  while (true) {
    camera >> frame; // get frame from the camera
    // we need to resize manually, using
    // nearest-neighbor interpolation, as the default linear interpolation
    // is too intensive for the Pi to keep up with and produces noticeable lag.
    resize( frame, display, Size(screen_w, screen_h), 0, 0, INTER_NEAREST );

    // show the frame
    imshow( window_name, display );

    switch (waitKey(1)) {
    case 27:
      cout << "quitting!" << endl;
      return 0;
    case -1:
      //no key pressed, do nothing
      break;
    default:
      //ignore any key not ESC 
      break;
    }
  }

  return 0;
}
