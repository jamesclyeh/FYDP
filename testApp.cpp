#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    // Setup video grabber:
    width = 320;
    height = 240;
    video.initGrabber(width, height);
	
    // Allocate memory for images
    updateBg = false;
    videoColorCvImage.allocate(width, height);
    videoColorHSVCvImage.allocate(width, height);
    videoGrayscaleHueImage.allocate(width, height);
    videoGrayscaleSatImage.allocate(width, height);
    videoGrayscaleBriImage.allocate(width, height);
    videoGrayscaleCvImage.allocate(width, height);
    videoGrayscaleCvImageForA.allocate(width, height);
    videoGrayscaleCvImageForB.allocate(width, height);
    playerMaskA.allocate(width, height);
    playerMaskB.allocate(width, height);
    grayscaleDiff.allocate(width, height);
    grayscaleBg.allocate(width, height);
	grayPixelsForBall = new unsigned char [width * height];
    grayPixelsForPlayerA = new unsigned char [width * height];
    grayPixelsForPlayerB = new unsigned char [width * height];
    
    ofImage image;
    ofLoadImage(image,"mask.jpg");
    image.update();
    playerMaskColor.setFromPixels(image.getPixels(), width, height);
    playerMaskA = playerMaskColor;
    
    ofLoadImage(image,"mask2.jpg");
    image.update();
    playerMaskColor.setFromPixels(image.getPixels(), width, height);
    playerMaskB = playerMaskColor;

    // Panel Setup
    ofBackground(255, 255, 255);
    panel.setup("cv settings", 1000, 0, 300, 748);
	panel.addPanel("control", 1, false);

	panel.setWhichPanel("control");
	panel.setWhichColumn(0);

    panel.addSlider("Mouse press control", "CALIBRATION_TOGGLE", 0, 0, 255, true);
	panel.addSlider("hue low ", "HUELOW", 10, 0, 255, true);
    panel.addSlider("hue high ", "HUEHIGH", 20, 0, 255, true);
	panel.addSlider("sat low ", "SATLOW", 20, 0, 255, true);
    panel.addSlider("sat high ", "SATHIGH", 600, 0, 255, true);
    panel.addSlider("val low ", "VALLOW", 35, 0, 255, true);
	panel.addSlider("val high ", "VALHIGH", 15, 0, 255, true);
    panel.addSlider("hue low ", "HUELOWPLAYER", 8, 0, 255, true);
    panel.addSlider("hue high ", "HUEHIGHPLAYER", 4, 0, 255, true);
    panel.addSlider("sat low ", "SATLOWPLAYER", 30, 0, 255, true);
    panel.addSlider("sat high ", "SATHIGHPLAYER", 70, 0, 255, true);
    panel.addSlider("val low ", "VALLOWPLAYER", 58, 0, 255, true);
    panel.addSlider("val high ", "VALHIGHPLAYER", 32, 0, 255, true);
    panel.addSlider("grey thresh ", "THRESHRANGE", 85, 0, 255, true);
    panel.addSlider("blur kernel ", "BLUR", 11, 0, 21, true);

	panel.loadSettings("cvSettings.xml");

    // Initial value setup
	hueLow = 20;
	satLow = 30;
	valLow = 25;
    hueHigh = 20;
    satHigh = 30;
    valHigh = 25;
    
    hueLowPlayer = 20;
    satLowPlayer = 30;
    valLowPlayer = 25;
    hueHighPlayer = 20;
    satHighPlayer = 30;
    valHighPlayer = 25;

	ballHue = 25;
	ballSat = 175;
	ballVal = 255;
    playerHue = 107;
    playerSat = 133;
    playerVal = 123;
    thresh = 85;
    
    // setup serial communcation
    serial.listDevices();
    vector<ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    
    int baud = 115200;
    serial.setup(0, baud);

    originX = 0;
    originY = 0;
    
    controlSelection = "ball";
    //playerMask
    
    frame = 0;
}

//--------------------------------------------------------------
void testApp::update(){
		
	panel.update();

	hueLow = panel.getValueI("HUELOW");
    hueHigh = panel.getValueI("HUEHIGH");
	satLow = panel.getValueI("SATLOW");
    satHigh = panel.getValueI("SATHIGH");
	valLow = panel.getValueI("VALLOW");
    valHigh = panel.getValueI("VALHIGH");
    hueLowPlayer = panel.getValueI("HUELOWPLAYER");
    hueHighPlayer = panel.getValueI("HUEHIGHPLAYER");
    satLowPlayer = panel.getValueI("SATLOWPLAYER");
    satHighPlayer = panel.getValueI("SATHIGHPLAYER");
    valLowPlayer = panel.getValueI("VALLOWPLAYER");
    valHighPlayer = panel.getValueI("VALHIGHPLAYER");
    thresh = panel.getValueI("THRESHRANGE");
    blur = panel.getValueI("BLUR");

    video.update();
	
    if (video.isFrameNew()){
        videoColorCvImage.setFromPixels(video.getPixels(), width, height);
        videoColorHSVCvImage = videoColorCvImage;
        // videoColorHSVCvImage.setROI()
        videoColorHSVCvImage.convertRgbToHsv();
		
        videoColorHSVCvImage.convertToGrayscalePlanarImages(
            videoGrayscaleHueImage, videoGrayscaleSatImage, videoGrayscaleBriImage);

        videoGrayscaleHueImage.flagImageChanged();
        videoGrayscaleSatImage.flagImageChanged();
        videoGrayscaleBriImage.flagImageChanged();

        unsigned char * colorHsvPixels = videoColorHSVCvImage.getPixels();
        unsigned char * playerMaskPixelsA = playerMaskA.getPixels();
        unsigned char * playerMaskPixelsB = playerMaskB.getPixels();
        // cv::Mat hsv;
        // cvtColor(ofxCv::toCv(videoColorCvImage), hsv, CV_BGR2HSV);
        //inRange(
        //    hsv,
        //    ofxCv::Scalar(ballHue+50-hueLow, ballSat-satLow, ballVal-valLow),
        //    ofxCv::Scalar(ballHue+50+hueHigh, ballSat+satHigh, ballVal+valHigh),
        //    mask);
		
        for (int i = 0; i < width*height; i++) {
            // since hue is cyclical:
            int hueDiff = colorHsvPixels[i*3] - ballHue;
            if (hueDiff < -127) hueDiff += 255;
            if (hueDiff > 127) hueDiff -= 255;

            if ((colorHsvPixels[i*3] > (ballHue - hueLow) && colorHsvPixels[i*3] < (ballHue + hueHigh))&&
                //(fabs(hueDiff) < hueHigh) &&
                (colorHsvPixels[i*3+1] > (ballSat - satLow) && colorHsvPixels[i*3+1] < (ballSat + satHigh)) &&
                (colorHsvPixels[i*3+2] > (ballVal - valLow) && colorHsvPixels[i*3+2] < (ballVal + valHigh))){
                    grayPixelsForBall[i] = 255;
            } else if ((colorHsvPixels[i*3] > (playerHue - hueLowPlayer)
                          && colorHsvPixels[i*3] < (playerHue + hueHighPlayer))&&
                       (colorHsvPixels[i*3+1] > (playerSat - satLowPlayer)
                          && colorHsvPixels[i*3+1] < (playerSat + satHighPlayer)) &&
                       (colorHsvPixels[i*3+2] > (playerVal - valLowPlayer)
                          && colorHsvPixels[i*3+2] < (playerVal + valHighPlayer)) &&
                       (playerMaskPixelsA[i] == 255 || playerMaskPixelsB[i] == 255)){
                if (playerMaskPixelsA[i] == 255)
                  grayPixelsForPlayerA[i] = 255;
                else if (playerMaskPixelsB[i] == 255)
                  grayPixelsForPlayerB[i] = 255;
            } else {
                grayPixelsForBall[i] = 0;
                grayPixelsForPlayerA[i] = 0;
                grayPixelsForPlayerB[i] = 0;
            }
        }
			
        videoGrayscaleCvImage.setFromPixels(grayPixelsForBall, width, height);
        videoGrayscaleCvImageForA.setFromPixels(grayPixelsForPlayerA, width, height);
        videoGrayscaleCvImageForB.setFromPixels(grayPixelsForPlayerB, width, height);

        videoGrayscaleCvImage.blurGaussian(blur);
        videoGrayscaleCvImageForA.blurGaussian(blur);
        videoGrayscaleCvImageForB.blurGaussian(blur);
        
        videoGrayscaleCvImage.threshold(thresh);
        // videoGrayscaleCvImageForA.threshold(thresh);
        contourFinder.findContours(videoGrayscaleCvImage, 5, (width*height)/4, 5, true);
        contourFinderForA.findContours(videoGrayscaleCvImageForA, 2, (width*height)/4, 3, false);
        contourFinderForB.findContours(videoGrayscaleCvImageForB, 2, (width*height)/4, 3, false);
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    ofSetColor(255, 255, 255);
    videoColorCvImage.draw(0,0, 320,240);
    videoColorHSVCvImage.draw(320+20, 0, 320, 240);
    playerMaskA.draw(640+40, 0);
    videoGrayscaleHueImage.draw(0,240+20);
    videoGrayscaleSatImage.draw(320+20,240+20);
    videoGrayscaleBriImage.draw(640+40,240+20);
    videoGrayscaleCvImage.draw(0,480+40,320,240);
    videoGrayscaleCvImageForA.draw(320+20,480+40,320,240);
    videoGrayscaleCvImageForB.draw(640+40,480+40,320,240);

    ofSetHexColor(0xffffff);
    //ofxCv::drawMat(mask, 640+40, 0);
    // grayscaleDiff.draw(640+40, 0, 320, 240);
    // ofRect(320, 0, 320, 240);
    // contourFinder.draw(0/*320+20*/, 0, 320, 240);
    //ofColor c(255, 255, 255);
    float max_size = -1;
    int max_index = -1;
    for(int i = 0; i < contourFinder.nBlobs; i++) {
        ofRectangle r = contourFinder.blobs.at(i).boundingRect;
        if(r.getArea() > max_size && (r.getWidth() / r.getHeight() < 2)){
            max_size = r.getArea();
            max_index = i;
        }
    }
    if(max_index != -1)
      contourFinder.blobs.at(max_index).draw();
    contourFinderForA.draw();
    contourFinderForB.draw();

    panel.draw();
    
    if (panel.getValueI("CALIBRATION_TOGGLE") < 64){
        controlSelection = "ball";
    } else if (panel.getValueI("CALIBRATION_TOGGLE") < 128){
        controlSelection = "player";
    } else if (panel.getValueI("CALIBRATION_TOGGLE") < 192){
        controlSelection = "origin";
    } else if (panel.getValueI("CALIBRATION_TOGGLE") < 255){
        controlSelection = "N/A";
    }

    std::stringstream ss;
    ss << "Ball HSV: " << ballHue << " " << ballSat << " " << ballVal << endl;
    ss << "Player HSV: " << playerHue << " " << playerSat << " " << playerVal << endl;
    ss << "Calibration selection: " << controlSelection <<endl;
    int ballY = -1;
    int ballX = -1;
    if(max_index != -1) {
        ballX = contourFinder.blobs.at(max_index).centroid.x - originX;
        ballY = contourFinder.blobs.at(max_index).centroid.y - originY;
        ss << "Ball centroid: x:" << ballX << " y:" << ballY << endl;
        float a_min = 100000;
        float b_min = 100000;
        //    for (int i=0; i < contourFinderForA.blobs.size(); i++) {
        //      float x = contourFinderForA.blobs.at(i).centroid.x - originX - ballX;
        //      float y = contourFinderForA.blobs.at(i).centroid.y - originY - ballY;
        //      float dist = pow(x, 2) + pow(y, 2);
        //      a_min = (abs(y) < a_min) ? y: a_min;
        //    }
        //    for (int i=0; i < contourFinderForB.blobs.size(); i++) {
        //        float x = contourFinderForB.blobs.at(i).centroid.x - originX - ballX;
        //        float y = contourFinderForB.blobs.at(i).centroid.y - originY - ballY;
        //        float dist = pow(x, 2) + pow(y, 2);
        //        b_min = (abs(y) < b_min) ? y: b_min;
        //    }
        //    ss << a_min << " " << b_min <<endl;
        
        std::vector<float> rodsY;
        if (ballX < 150) {
            for (int i =0; i < contourFinderForB.blobs.size(); i++) {
                rodsY.push_back(contourFinderForB.blobs.at(i).centroid.y - originY);
            }
            if (rodsY.size() == 3) {
                std::sort(rodsY.begin(), rodsY.end());
                float yPos;
                if (ballY < 90) {
                    yPos = rodsY[0];
                    if (rand()%10 > 2) {
                        ballY -= (rand()%5) + 12;
                    }
                    
                } else if (ballY > 90 && ballY < 155) {
                    yPos = rodsY[1];
                } else {
                    yPos = rodsY[2];
                    if (yPos < 210) {
                        if (rand()%10 > 2) {
                            ballY += 5*rand() + 10;
                        }
                    }
                }
                float xDiff = contourFinderForB.blobs.at(0).centroid.x - originX - ballX;
                if (frame % 3 == 0) {
                    ss << "Bar B move " << getLinearMotionDirective(yPos-ballY, xDiff, true) << endl;
                }
            }
        } else {
            for (int i =0; i < contourFinderForA.blobs.size(); i++) {
                rodsY.push_back(contourFinderForA.blobs.at(i).centroid.y);
            }
            if (rodsY.size() == 3) {
                std::sort(rodsY.begin(), rodsY.end());
                float yPos;
                if (ballY < 90) {
                    yPos = rodsY[0];
                    if (rand()%10 > 8) {
                        ballY += (rand() % 5) + 12;
                    }
                } else if (ballY > 90 && ballY < 155) {
                    yPos = rodsY[1];
                } else {
                    yPos = rodsY[2];
                    if (rand() % 10 > 8) {
                        ballY -= (rand() % 5) + 12;
                    }
                }
                float xDiff = contourFinderForA.blobs.at(0).centroid.x - ballX;
                if (frame % 3 == 0) {
                    ss << "Bar A move " << getLinearMotionDirective(yPos-ballY, xDiff, false) << endl;
                    //ss << "Bar B move " << getLinearMotionDirective(b_min) << endl;
                }
            }

        }
        
        
        ofDrawBitmapStringHighlight(ss.str(), ofPoint(640+40, 200));
        
        frame++;
    }

}

void testApp::moveLeft() {
    serial.writeByte('a');
}

string testApp::getLinearMotionDirective(float yDiff, float xDiff, bool isOffence) {
    string msg;
  if (abs(yDiff) < 10) {
    if (abs(xDiff) < 35) {
        float r = rand() % 100;
//        if (r < 90) {
//            isOffence ? serial.writeByte('i') : serial.writeByte('w');
//        } else if (r >= 90 && r < 95) {
//            isOffence ? serial.writeByte('j') : serial.writeByte('a');
//        } else {
//            isOffence ? serial.writeByte('l') : serial.writeByte('d');
//        }
      isOffence ? serial.writeByte('i') : serial.writeByte('w');
      msg = "Kick";
    } else {
        msg = "Stay";
    }
  }
  else if (yDiff < 0) {
    int r = 0;
    if (yDiff > 30) {
        r = rand() % 100;
    }
    if (r > 40) {
        isOffence ? serial.writeByte('u') : serial.writeByte('q');
    } else {
        isOffence ? serial.writeByte('j') : serial.writeByte('a');
    }
    msg = "Forward";
  }
  else {
    int r = 0;
    if (yDiff > 30) {
        r = rand() % 100;
    }
    if (r > 40) {
        isOffence ? serial.writeByte('o') : serial.writeByte('e');
    } else {
        isOffence ? serial.writeByte('l') : serial.writeByte('d');
    }
    msg = "Backward";
  }
  return msg;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if (key == 'k') {
        // serial communcation:
        serial.writeByte('k');
        
        char response = serial.readByte();
        if (response == OF_SERIAL_ERROR) {
            printf("error occured");
        } else {
            printf("response is %d", response);
        }
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	panel.mouseDragged(x,y,button);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    panel.mousePressed(x,y,button);
    printf("x = %d y = %d", x, y);

    if (x >= 0 && x < 320 && y >= 0 && y < 240 && panel.getValueI("CALIBRATION_TOGGLE") < 64){
        int pixel = y * 320 + x;
        ballHue = videoGrayscaleHueImage.getPixels()[pixel];
        ballSat = videoGrayscaleSatImage.getPixels()[pixel];
        ballVal = videoGrayscaleBriImage.getPixels()[pixel];
        // updateBg = true
    } else if (x >= 0 && x < 320 && y >= 0 && y < 240 && panel.getValueI("CALIBRATION_TOGGLE") < 128){
        int pixel = y * 320 + x;
        playerHue = videoGrayscaleHueImage.getPixels()[pixel];
        playerSat = videoGrayscaleSatImage.getPixels()[pixel];
        playerVal = videoGrayscaleBriImage.getPixels()[pixel];
    } else if (x >= 0 && x < 320 && y >= 0 && y < 240 && panel.getValueI("CALIBRATION_TOGGLE") < 192){
        originX = x;
        originY = y;
    } else if (x >= 0 && x < 320 && y >= 0 && y < 240 && panel.getValueI("CALIBRATION_TOGGLE") < 255){
    }
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    panel.mouseReleased();
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}

