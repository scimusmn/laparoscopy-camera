#include <opencv2/opencv.hpp>
#include <X11/Xlib.h>
#include <wiringPi.h>
#include <iostream>
#include <string>
#include <chrono>

using namespace std::chrono;
using namespace std;
using namespace cv;

milliseconds diff_check_interval(10000); // check for difference every 10 seconds

string window_name = "Laparascopy";
string difference_fname = "diffcheck.png";

double difference_threshold = 0.03; // images less than 10% different are deemed the same

double get_difference( Mat, Mat );

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

  VideoCapture camera(0);
  if ( !camera.isOpened() ){
    cerr << "ERROR: could not open camera!" << endl;
    return -1;
  }
  camera.set(CAP_PROP_BUFFERSIZE, 1); // buffer only one frame
  cout << "opened camera" << endl;

  namedWindow(window_name, WINDOW_NORMAL | WINDOW_FULLSCREEN );
  //  setWindowProperty( window_name, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN );

  Mat frame, oldframe, display;
  camera >> oldframe;

  
  
  string text = "Push button to start.";
  int baseline = 0;
  Size textsize = getTextSize( text, FONT_HERSHEY_DUPLEX, 1, 1, &baseline );

  Mat waitscreen ( screen_h, screen_w, CV_8UC3, Scalar(64,64,64) );
  putText( waitscreen, text,
	   Point( (waitscreen.cols-textsize.width)/2, (waitscreen.rows-textsize.height)/2 ),
	   FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255), 1, LINE_AA );

  cout << "waitscreen size: " << waitscreen.rows << "x" << waitscreen.cols << endl;
  
  bool showcamera = false;
  display = waitscreen.clone();

  auto timepoint = steady_clock::now();

  digitalWrite(led_pin,1);
  
  while (true) {
    if (showcamera) {
      camera >> frame;
      resize( frame, display, Size(screen_w, screen_h), 0, 0, INTER_NEAREST );
    }
    else {
      if ( digitalRead(button_pin) ) {
        digitalWrite(led_pin,0); // turn off led
	showcamera = true;
	camera >> frame;
	oldframe = frame.clone();
	timepoint = steady_clock::now();
      }
    }

    imshow( window_name, display );

    if ( showcamera &&
	 (steady_clock::now() >= timepoint + diff_check_interval ) ) {
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
	display = waitscreen.clone();
	digitalWrite(led_pin,1); // turn on led
      }
    }
    
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
