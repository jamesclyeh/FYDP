#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxControlPanel.h"
#include "ofxMacamPs3Eye.h"
#include <math.h>
#include <vector>
#include <algorithm>

class testApp : public ofBaseApp{

    public:
        void setup();
        void update();
        void draw();

        void keyPressed  (int key);
        void keyReleased(int key);
        void mouseMoved(int x, int y );
        void mouseDragged(int x, int y, int button);
        void mousePressed(int x, int y, int button);
        void mouseReleased(int x, int y, int button);
        void windowResized(int w, int h);
    
        string getLinearMotionDirective(float yDiff, float xDiff);
        void moveLeft();

        ofxControlPanel	panel;

		int	width, height;
		ofxMacamPs3Eye video;

        // create a variables for images
        ofxCvColorImage videoColorCvImage;
        ofxCvColorImage videoColorHSVCvImage;
        ofxCvGrayscaleImage	videoGrayscaleHueImage;
        ofxCvGrayscaleImage	videoGrayscaleSatImage;
        ofxCvGrayscaleImage	videoGrayscaleBriImage;
        unsigned char *	grayPixelsForBall;
        unsigned char * grayPixelsForPlayerA;
        unsigned char * grayPixelsForPlayerB;
        ofxCvGrayscaleImage	videoGrayscaleCvImage;
        ofxCvGrayscaleImage	videoGrayscaleCvImageForA;
        ofxCvGrayscaleImage	videoGrayscaleCvImageForB;
        ofxCvColorImage playerMaskColor;
        ofxCvGrayscaleImage playerMaskA;
        ofxCvGrayscaleImage playerMaskB;

        // for contours
        bool updateBg;
        ofxCvGrayscaleImage grayscaleDiff;
        ofxCvGrayscaleImage grayscaleBg;
        ofxCvContourFinder contourFinder;
        ofxCvContourFinder contourFinderForA;
        ofxCvContourFinder contourFinderForB;

        // for tracking...
        int ballHue, ballSat, ballVal;
        int playerHue, playerSat, playerVal;
        int thresh, blur;
        int hueHigh, hueLow, satHigh, satLow, valHigh, valLow;
    
        // for serial communcation
        ofSerial serial;

        int hueHighPlayer, hueLowPlayer, satHighPlayer, satLowPlayer, valHighPlayer, valLowPlayer;
        int calibration;
    
        string controlSelection;
    
        int originX;
        int originY;
    
    int frame;
};

#endif
