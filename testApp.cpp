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
    videoGrayscaleCvImageForPlayer.allocate(width, height);
    playerMask.allocate(width, height);
    grayscaleDiff.allocate(width, height);
    grayscaleBg.allocate(width, height);
	grayPixelsForBall = new unsigned char [width * height];
    grayPixelsForPlayer = new unsigned char [width * height];
    
    ofImage image;
    ofLoadImage(image,"mask.jpg");
    image.update();
    playerMaskColor.setFromPixels(image.getPixels(), width, height);
    playerMask = playerMaskColor;

    // Panel Setup
    ofBackground(255, 255, 255);
    panel.setup("cv settings", 1000, 0, 300, 748);
	panel.addPanel("control", 1, false);

	panel.setWhichPanel("control");
	panel.setWhichColumn(0);

    panel.addSlider("Mouse press control", "CALIBRATION_TOGGLE", 0, 0, 255, true);
	panel.addSlider("hue low ", "HUELOW", 5, 0, 255, true);
    panel.addSlider("hue high ", "HUEHIGH", 20, 0, 255, true);
	panel.addSlider("sat low ", "SATLOW", 30, 0, 255, true);
    panel.addSlider("sat high ", "SATHIGH", 30, 0, 255, true);
    panel.addSlider("val low ", "VALLOW", 25, 0, 255, true);
	panel.addSlider("val high ", "VALHIGH", 25, 0, 255, true);
    panel.addSlider("hue low ", "HUELOWPLAYER", 5, 0, 255, true);
    panel.addSlider("hue high ", "HUEHIGHPLAYER", 20, 0, 255, true);
    panel.addSlider("sat low ", "SATLOWPLAYER", 30, 0, 255, true);
    panel.addSlider("sat high ", "SATHIGHPLAYER", 30, 0, 255, true);
    panel.addSlider("val low ", "VALLOWPLAYER", 25, 0, 255, true);
    panel.addSlider("val high ", "VALHIGHPLAYER", 25, 0, 255, true);
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
	ballSat = 158;
	ballVal = 255;
    playerHue = 25;
    playerSat = 158;
    playerVal = 255;
    thresh = 85;
    
    // setup serial communcation
    serial.listDevices();
    vector<ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    
    int baud = 9600;
    serial.setup(0, baud);

    originX = 0;
    originY = 0;
    
    controlSelection = "ball";
    //playerMask
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
        unsigned char * playerMaskPixels = playerMask.getPixels();
        cv::Mat hsv;
        cvtColor(ofxCv::toCv(videoColorCvImage), hsv, CV_BGR2HSV);
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
                       playerMaskPixels[i] == 255){
                    grayPixelsForPlayer[i] = 255;
            } else {
                grayPixelsForBall[i] = 0;
                grayPixelsForPlayer[i] = 0;
            }
        }
			
        videoGrayscaleCvImage.setFromPixels(grayPixelsForBall, width, height);
        videoGrayscaleCvImageForPlayer.setFromPixels(grayPixelsForPlayer, width, height);

        videoGrayscaleCvImage.blurGaussian(blur);
        videoGrayscaleCvImageForPlayer.blurGaussian(blur);
        
        videoGrayscaleCvImage.threshold(thresh);
        videoGrayscaleCvImageForPlayer.threshold(thresh);
        contourFinder.findContours(videoGrayscaleCvImage, 5, (width*height)/4, 5, true);
        contourFinder2.findContours(videoGrayscaleCvImageForPlayer, 2, (width*height)/4, 6, false);
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    ofSetColor(255, 255, 255);
    videoColorCvImage.draw(0,0, 320,240);
    videoColorHSVCvImage.draw(320+20, 0, 320, 240);
    playerMask.draw(640+40, 0);
    videoGrayscaleHueImage.draw(0,240+20);
    videoGrayscaleSatImage.draw(320+20,240+20);
    videoGrayscaleBriImage.draw(640+40,240+20);
    videoGrayscaleCvImage.draw(0,480+40,320,240);
    videoGrayscaleCvImageForPlayer.draw(320+20,480+40,320,240);

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
    contourFinder2.draw();

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
    if(max_index != -1)
      ss << "Ball centroid: x:" << contourFinder.blobs.at(max_index).centroid.x - originX
         << " y:" <<  contourFinder.blobs.at(max_index).centroid.y - originY << endl;
    ofDrawBitmapStringHighlight(ss.str(), ofPoint(640+40, 600));
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

