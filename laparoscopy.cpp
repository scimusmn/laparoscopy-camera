#include <opencv2/opencv.hpp>
#include <X11/Xlib.h>
#include <wiringPi.h>
#include <iostream>
#include <string>
#include <chrono>
#include <cstdlib>

using namespace std::chrono;
using namespace std;
using namespace cv;

milliseconds diff_check_interval(10000); // check for difference every 10 seconds

milliseconds text_move_interval(10000); // move on-screen text every 10 seconds

string window_name = "Laparascopy";
string difference_fname = "diffcheck.png";
string wait_text = "Push button to start.";

double difference_threshold = 0.03; // images less than 3% different are deemed the same

// function to check the level of difference between two images
double get_difference( Mat, Mat );

// function to build a frame of the screensaver
Mat make_waitscreen( int, int, string );

int main( int argc, char** argv ) {
  // set up wiring pi
  wiringPiSetup();
  int button_pin = 9;
  int led_pin = 7;

  pinMode(button_pin, INPUT);
  pinMode(led_pin,    OUTPUT);
    
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
  Mat frame, oldframe, display;
  camera >> oldframe;

  display = make_waitscreen(screen_w, screen_h, wait_text);
  
  bool showcamera = false;

  auto timepoint = steady_clock::now();

  digitalWrite(led_pin,1);
  
  while (true) {
    if (showcamera) {
      camera >> frame; // get frame from the camera
      // resize frame for display.
      // we need to do the resizing manually, because we need to force
      // nearest-neighbor interpolation, as the default linear interpolation
      // is too intensive for the Pi to keep up with and produces noticeable
      // lag.
      resize( frame, display, Size(screen_w, screen_h), 0, 0, INTER_NEAREST );
      
      // has the difference time been exceded?
      if (steady_clock::now() >= timepoint + diff_check_interval ) {
	// time since last timepoint is greater than diff_check_interval
	if ( get_difference( frame, oldframe ) >= difference_threshold ) {
	  // someone's still using the exhibit!
	  oldframe = frame.clone();
	  timepoint = steady_clock::now();
	}
	else {
	  // nothing's changed since last we checked
	  // turn off the camera
	  showcamera = false;
	  camera >> frame;
	  display = make_waitscreen( screen_w, screen_h, wait_text );
	  digitalWrite(led_pin,1); // turn on led
	  timepoint = steady_clock::now();
	}
      }
    }
    else {
      // showcamera is false
      // check if the button has been pressed
      if ( digitalRead(button_pin) ) {
	// it has!
        digitalWrite(led_pin,0); // turn off led
	showcamera = true;
	// get a frame for difference checking
	camera >> frame;
	oldframe = frame.clone();
	
	timepoint = steady_clock::now();
      }

      // check if the time has exceeded text_move_interval
      if (steady_clock::now() >= timepoint + text_move_interval ) {
	display = make_waitscreen( screen_w, screen_h, wait_text );
	timepoint = steady_clock::now();
      }
    }

    // display whatever we're looking at
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

double get_difference( Mat src1, Mat src2 ) {
  assert ( src1.rows == src2.rows && src1.cols == src2.cols );
  //convert mats to grayscale
  Mat gray1, gray2;
  cvtColor( src1, gray1, COLOR_BGR2GRAY );
  cvtColor( src2, gray2, COLOR_BGR2GRAY );

  //flatten input images to suppress noise
  gray1 /= 64;
  gray2 /= 64;

  //find and count different pixels
  Mat diff;
  compare( gray1, gray2, diff , CMP_NE );
  int n_different = countNonZero(diff);

  return (double) n_different / (gray1.rows * gray1.cols); // fraction different
}


Mat make_waitscreen( int screen_w, int screen_h, string text ) {
  int baseline = 0;
  Size textsize = getTextSize( text, FONT_HERSHEY_DUPLEX, 1, 1, &baseline );

  int x = rand() % ( screen_w - textsize.width );
  int y = rand() % ( screen_h - textsize.height );
  
  Mat waitscreen ( screen_h, screen_w, CV_8UC3, Scalar(64,64,64) );
  putText( waitscreen, text,
	   Point(x,y), FONT_HERSHEY_SIMPLEX, 1,
	   Scalar(255,255,255), 1, LINE_AA );

  return waitscreen;
}
