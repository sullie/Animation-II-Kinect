#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"

// uncomment this to read from two kinects simultaneously
//#define USE_TWO_KINECTS

class testApp : public ofBaseApp {
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void drawPointCloud();
	
	void keyPressed (int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	
	void audioIn(float * input, int bufferSize, int nChannels); 
	
	
	vector <float> left;
	vector <float> right;
	vector <float> volHistory;
	
	int 	bufferCounter;
	int 	drawCounter;
	
	float smoothedVol;
	float scaledVol;
	
	ofSoundStream soundStream;

	ofxKinect kinect;
	
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image

	ofxCvContourFinder contourFinder;
	
	bool bThreshWithOpenCV;
	bool bDrawPointCloud;
	
	int nearThreshold;
	int farThreshold;
	
	int angle;
	
    // used for viewing the point cloud
	ofEasyCam easyCam;
	ofShader shader;
	bool reloadShaders;

private:
	ofImage ocean;
	ofImage desert;
	ofImage mask;

	unsigned char * frames;
	int numPixels; 
};
