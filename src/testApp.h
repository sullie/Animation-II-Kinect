#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"

#include "ofxFft.h"

#include "ofxBox2d.h"
#include "..\..\Box2D.h"
//#include "contourSimplify\contourSimplify.h"

#include "Data.h"
#include "MyListener.h"

#define B2SCALE  30.0f

#define MIC 0
#define NOISE 1
#define SINE 2


//testApp class
class testApp : public ofBaseApp {
public:
	
	void setup();
	void update();
	void draw();
	void exit();
		
	void keyPressed (int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	 
	// Kinect Function
	void drawPointCloud();

	// Box2D Functions
	void initBox2D();
	void makeOutlines();
	void spawnParticles( float brickX, float brickY );
	void doDestroy();
	void createCircle();	

	// Audio Functions
	void audioReceived(float* input, int bufferSize, int nChannels);
	void plot(vector<float>& buffer, float scale, float offset);

//--------------------------------------------------------------------------------------------------------------	
	// Kinect Variables
	ofxKinect kinect;
	bool bDrawKinect;
	
	// Image variables
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage	grayThresh;
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image

	ofxCvContourFinder contourFinder;
	
	bool bDrawPointCloud;
	int nearThreshold;
	int farThreshold;
	
    // used for viewing the point cloud
	int angle;
	ofEasyCam easyCam;

	//shader
	ofShader shader;
	bool reloadShaders;
//--------------------------------------------------------------------------------------------------------------
	//Audio Variables
	int plotHeight, bufferSize;   
    bool bDrawSound;     
	ofxFft* fft;
    int mode;
	
	ofMutex soundMutex;
    vector<float> drawBuffer, middleBuffer, audioBuffer;
	vector<float> drawBins, middleBins, audioBins;

//--------------------------------------------------------------------------------------------------------------	
	//BOX 2D Variables
	ofxBox2d box2dRed;			                //the box2d world
	ofxBox2d box2dBlue;			                //the box2d world
	vector <ofxBox2dCircle>	redCircles;	   //default box2d circles
	vector <ofxBox2dCircle>	blueCircles;	   //default box2d circles
	vector <ofxBox2dPolygon> polygons;	  //defalut box2d polgons
	vector <ofPolyline>	lines;		 //default box2d lines (hacked)
	
	b2World *myWorld;           //our work space
	b2Body*	ground;

	vector<ofxBox2dRect> worldBlobs;   //vextor of blocks in our work space
	vector <ofPolyline> lineStrips;
	
	vector <b2Body*> toDestroy;
	vector <b2Body*> objects;
	vector <ofPolyline> outlines;
	
	MyListener*	listener;

	// FILTERING VARIABLES
	const static uint16 CATEGORY_GROUND = 0x0010;

    const static uint16 CATEGORY_RED_BALLS = 0x0001; // 0000000000000010 in binary
	const static uint16 CATEGORY_BLUE_BALLS = 0x0002;
	const static uint16  CATEGORY_YELLOW_BALLS= 0x0004;
	const static uint16 CATEGORY_GREEN_BALLS = 0x0008;

	const static uint16 MASK_RED_BALLS = CATEGORY_RED_BALLS | CATEGORY_GROUND ;
	const static uint16 MASK_BLUE_BALLS = CATEGORY_BLUE_BALLS | CATEGORY_GROUND ;
	const static uint16 MASK_YELLOW_BALLS = CATEGORY_YELLOW_BALLS | CATEGORY_GROUND ;
	const static uint16 MASK_GREEN_BALLS = CATEGORY_GREEN_BALLS | CATEGORY_GROUND ;
	const static int MASK_GROUND= -1;

	const static int RED_GROUP=0;
	const static int BLUE_GROUP=1;
	const static int YELLOW_GROUP=2;
	const static int GREEN_GROUP=3;


//--------------------------------------------------------------------------------------------------------------
	/*
	//Simple contour
	contourSimplify contourSimp;
	vector<vector <ofxPoint2f> > simpleContours;
	vector <ofxPoint2f> contourReg;
	vector <ofxPoint2f> contourSmooth;
	vector <ofxPoint2f> contourSimple;*/
	
//--------------------------------------------------------------------------------------------------------------
private:
	ofImage ocean;
	ofImage desert;
	ofImage mask;
	

};
