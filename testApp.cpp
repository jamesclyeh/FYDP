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
    grayscaleDiff.allocate(width, height);
    grayscaleBg.allocate(width, height);
	grayPixels = new unsigned char [width * height];

    // Panel Setup
    ofBackground(255, 255, 255);
    panel.setup("cv settings", 1000, 0, 300, 748);
	panel.addPanel("control", 1, false);

	panel.setWhichPanel("control");
	panel.setWhichColumn(0);

	panel.addToggle("video settings", "VIDEO_SETTINGS", false);
	panel.addSlider("hue low ", "HUELOW", 5, 0, 255, true);
    panel.addSlider("hue high ", "HUEHIGH", 20, 0, 255, true);
	panel.addSlider("sat low ", "SATLOW", 30, 0, 255, true);
    panel.addSlider("sat high ", "SATHIGH", 30, 0, 255, true);
    panel.addSlider("val low ", "VALLOW", 25, 0, 255, true);
	panel.addSlider("val high ", "VALHIGH", 25, 0, 255, true);
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

	hue = 25;
	sat = 158;
	val = 255;
    thresh = 85;
    
    // setup serial communcation
    serial.listDevices();
    vector<ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    
    int baud = 9600;
    serial.setup(0, baud);
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
    thresh = panel.getValueI("THRESHRANGE");
    blur = panel.getValueI("BLUR");

    if (panel.getValueB("VIDEO_SETTINGS") == true){
        video.videoSettings();
        panel.setValueB("VIDEO_SETTINGS", false);
    }

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
        cv::Mat hsv;
        cvtColor(ofxCv::toCv(videoColorCvImage), hsv, CV_BGR2HSV);
        inRange(
            hsv,
            ofxCv::Scalar(hue+50-hueLow, sat-satLow, val-valLow),
            ofxCv::Scalar(hue+50+hueHigh, sat+satHigh, val+valHigh),
            mask);
		
        for (int i = 0; i < width*height; i++) {
            // since hue is cyclical:
            int hueDiff = colorHsvPixels[i*3] - hue;
            if (hueDiff < -127) hueDiff += 255;
            if (hueDiff > 127) hueDiff -= 255;

            if ((colorHsvPixels[i*3] > (hue - hueLow) && colorHsvPixels[i*3] < (hue + hueHigh))&&
                //(fabs(hueDiff) < hueHigh) &&
                (colorHsvPixels[i*3+1] > (sat - satLow) && colorHsvPixels[i*3+1] < (sat + satHigh)) &&
                (colorHsvPixels[i*3+2] > (val - valLow) && colorHsvPixels[i*3+2] < (val + valHigh))){
                    grayPixels[i] = 255;
            } else {
                    grayPixels[i] = 0;
            }
        }
			
        videoGrayscaleCvImage.setFromPixels(grayPixels, width, height);

        videoGrayscaleCvImage.blurGaussian(blur);
        
        if (updateBg == true) {
            grayscaleBg = videoGrayscaleCvImage;
            updateBg = false;
        }

        grayscaleDiff.absDiff(grayscaleBg, videoGrayscaleCvImage);
        grayscaleDiff.threshold(thresh);
        videoGrayscaleCvImage.threshold(thresh);
        contourFinder.findContours(grayscaleDiff, 5, (width*height)/4, 5, true);
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    ofSetColor(255, 255, 255);
    videoColorCvImage.draw(0,0, 320,240);
    videoColorHSVCvImage.draw(320+20, 0, 320, 240);
    videoGrayscaleHueImage.draw(0,240+20);
    videoGrayscaleSatImage.draw(320+20,240+20);
    videoGrayscaleBriImage.draw(640+40,240+20);
    videoGrayscaleCvImage.draw(0,480+40,320,240);

    ofSetHexColor(0xffffff);
    ofxCv::drawMat(mask, 640+40, 0);
    // grayscaleDiff.draw(640+40, 0, 320, 240);
    // ofRect(320, 0, 320, 240);
    contourFinder.draw(0/*320+20*/, 0, 320, 240);
    ofColor c(255, 255, 255);
    /*for(int i = 0; i < contourFinder.nBlobs; i++) {
        ofRectangle r = contourFinder.blobs.at(i).boundingRect;
        r.x += width; r.y += height;
        c.setHsb(i * 64, 255, 255);
        ofSetColor(c);
        ofRect(r);
     }*/

    panel.draw();

    std::stringstream ss;
    ss << "HSV: " << hue << " " << sat << " " << val << endl;
    ofDrawBitmapStringHighlight(ss.str(), ofPoint(10, 15));
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

    if (x >= 0 && x < 320 && y >= 0 && y < 240){
        int pixel = y * 320 + x;
        hue = videoGrayscaleHueImage.getPixels()[pixel];
        sat = videoGrayscaleSatImage.getPixels()[pixel];
        val = videoGrayscaleBriImage.getPixels()[pixel];
        // updateBg = true;
    }
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    panel.mouseReleased();
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}

