#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxControlPanel.h"
#include "ofxMacamPs3Eye.h"

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

        ofxControlPanel	panel;

		int	width, height;
		ofxMacamPs3Eye video;

        // create a variables for images
        ofxCvColorImage videoColorCvImage;
        ofxCvColorImage videoColorHSVCvImage;
        ofxCvGrayscaleImage	videoGrayscaleHueImage;
        ofxCvGrayscaleImage	videoGrayscaleSatImage;
        ofxCvGrayscaleImage	videoGrayscaleBriImage;
        unsigned char *	grayPixels;
        ofxCvGrayscaleImage	videoGrayscaleCvImage;
        cv::Mat mask;

        // for contours
        bool updateBg;
        ofxCvGrayscaleImage grayscaleDiff;
        ofxCvGrayscaleImage grayscaleBg;
        ofxCvContourFinder contourFinder;

        // for tracking...
        int hue, sat, val, thresh, blur;
        int hueHigh, hueLow, satHigh, satLow, valHigh, valLow;
};

#endif
