#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);

	ofEnableAlphaBlending();
	
    // enable depth->rgb image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	kinect.open();
	
	// Allocatins space for images
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);

	//Allocating memory for mask 
	mask.allocate(640, 480, OF_IMAGE_GRAYSCALE);  //OF_IMAGE_COLOR
	
	numPixels= 640 * 480;
	frames = new unsigned char[256*numPixels*3];

	nearThreshold = 230;
	farThreshold = 70;
	bThreshWithOpenCV = true;
	
	ofSetFrameRate(60);
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
	
	// start from the front
	bDrawPointCloud = false;
	
	// 0 output channels, 
	// 2 input channels
	// 44100 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	
	soundStream.listDevices();
	
	//if you want to set a different device id 
	//soundStream.setDeviceID(0); //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.
	
	int bufferSize = 256;
	
	
	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	volHistory.assign(400, 0.0);
	
	bufferCounter	= 0;
	drawCounter		= 0;
	smoothedVol     = 0.0;
	scaledVol		= 0.0;
	
	soundStream.setup(this, 0, 2, 44100, bufferSize, 4);

	// Test images
	ocean.loadImage("images/ocean.jpg");
	desert.loadImage("images/desert.jpg");

	// Set up shaders for the mask
	shader.load("shaders/test.vert", "shaders/test.frag");
	shader.begin();
	shader.setUniformTexture("tex", ocean.getTextureReference(), 0);
	//shader.setUniformTexture("tex1", kinect gray image set down in draw function, since it changes per frame, 1);
	shader.setUniformTexture("tex2", desert.getTextureReference(), 2);
	shader.end();


}

//--------------------------------------------------------------
void testApp::update() {
	
	ofBackground(255);
	kinect.update();

	if (reloadShaders)
	{
		shader.load("shaders/test.vert", "shaders/test.frag");
		printf("Reloaded the shaders");
		reloadShaders = false;
	}
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		colorImg.setFromPixels(kinect.getPixels(), kinect.width, kinect.height);
	
		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
		if(bThreshWithOpenCV) {
			grayThreshNear = grayImage;
			grayThreshFar = grayImage;
			grayThreshNear.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		} else {
			
			// or we do it ourselves - show people how they can work with the pixels
			unsigned char * pix = grayImage.getPixels();
			
			int numPixels = grayImage.getWidth() * grayImage.getHeight();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold && pix[i] > farThreshold) {
					pix[i] = 255;
				} else {
					pix[i] = 0;
				}
			}
		}
		
		// update the cv images
		grayImage.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
	}
	
	//lets scale the vol up to a 0-1 range 
	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
	
	//lets record the volume into an array
	volHistory.push_back( scaledVol );
	
	//if we are bigger the the size we want to record - lets drop the oldest value
	if( volHistory.size() >= 400 ){
		volHistory.erase(volHistory.begin(), volHistory.begin()+1);
	}
	
}

//--------------------------------------------------------------
void testApp::draw()
{
	ofSetColor(255);

	// Create mask image for shader
	unsigned char * pixels = grayImage.getPixels();
	mask.setFromPixels(pixels, 640, 480, OF_IMAGE_GRAYSCALE,true);
	
	// Starting shader
	shader.begin();
    shader.setUniformTexture("tex", ocean.getTextureReference(), 0);
	shader.setUniformTexture("tex1", mask.getTextureReference(), 1);
	shader.setUniformTexture("tex2", desert.getTextureReference(), 2);
	
	glActiveTexture(GL_TEXTURE0_ARB);
	ocean.getTextureReference().bind();

	glActiveTexture(GL_TEXTURE1_ARB);
	mask.getTextureReference().bind();

	glActiveTexture(GL_TEXTURE2_ARB);
	desert.getTextureReference().bind();

	glBegin(GL_QUADS);
		
		glMultiTexCoord2d(GL_TEXTURE0_ARB, 0, 0);
		glMultiTexCoord2d(GL_TEXTURE1_ARB, 0, 0);
		glMultiTexCoord2d(GL_TEXTURE2_ARB, 0, 0);
		glVertex2f(0, 0);

		glMultiTexCoord2d(GL_TEXTURE0_ARB, ocean.getWidth(), 0);
		glMultiTexCoord2d(GL_TEXTURE1_ARB, mask.getWidth(), 0);
		glMultiTexCoord2d(GL_TEXTURE2_ARB, desert.getWidth(),0);
		glVertex2f(ofGetWidth(), 0);

		glMultiTexCoord2d(GL_TEXTURE0_ARB, ocean.getWidth(), ocean.getHeight());
		glMultiTexCoord2d(GL_TEXTURE1_ARB, mask.getWidth(), mask.getHeight());
		glMultiTexCoord2d(GL_TEXTURE2_ARB, desert.getWidth(), desert.getHeight() );
		glVertex2f(ofGetWidth(), ofGetHeight());

		glMultiTexCoord2d(GL_TEXTURE0_ARB, 0, ocean.getHeight());
		glMultiTexCoord2d(GL_TEXTURE1_ARB, 0, mask.getHeight() );
		glMultiTexCoord2d(GL_TEXTURE2_ARB, 0, desert.getHeight());
		glVertex2f(0, ofGetHeight());
	glEnd();

	glActiveTexture(GL_TEXTURE2_ARB);
	desert.getTextureReference().unbind();

	glActiveTexture(GL_TEXTURE1_ARB);
	mask.getTextureReference().unbind();

	glActiveTexture(GL_TEXTURE0_ARB);
	ocean.getTextureReference().unbind();

	shader.end();

	
	if(bDrawPointCloud) {
		easyCam.begin();
		drawPointCloud();
		easyCam.end();
	} 
	//else {
		// draw from the live kinect
		//kinect.drawDepth(10, 10, 400, 300);
		//kinect.draw(420, 10, 400, 300);
		
		//grayImage.draw(0, 0, 800, 600);
		
		//contourFinder.draw(10, 320, 400, 300);

	//}
	
	// draw instructions
	ofSetColor(255, 255, 255);
	stringstream reportStream;
	reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
	<< ofToString(kinect.getMksAccel().y, 2) << " / "
	<< ofToString(kinect.getMksAccel().z, 2) << endl
	<< "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
	<< "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
	<< "set near threshold " << nearThreshold << " (press: + -)" << endl
	<< "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
	<< ", fps: " << ofGetFrameRate() << endl
	<< "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl
	<< "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl;
	ofDrawBitmapString(reportStream.str(),20,652);
	

/*	ofSetColor(255);

	desert.draw(0, 0);

	ofEnableAlphaBlending();
	float wave = sin(ofGetElapsedTimef());
	ofSetColor(255, 255, 255, scaledVol * 255);
	ocean.draw(0, 0);
	ofDisableAlphaBlending();*/
}

void testApp::drawPointCloud() {
	int w = 640;
	int h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			if(kinect.getDistanceAt(x, y) > 0) {
				mesh.addColor(kinect.getColorAt(x,y));
				ofVec3f scaler  = ofVec3f(1.0, 1.0, scaledVol);
				ofVec3f temp = kinect.getWorldCoordinateAt(x, y);
				mesh.addVertex( temp * scaler );
//				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
			}
		}
	}
	glPointSize(3);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards' 
	ofScale(1, -1, -1);
	ofTranslate(0, 0, -1000); // center the points a bit
	glEnable(GL_DEPTH_TEST);
	mesh.drawVertices();
	glDisable(GL_DEPTH_TEST);
	ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
	
#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif
}

void testApp::audioIn(float * input, int bufferSize, int nChannels){	
	
	float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;	
	
	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2]*0.5;
		right[i]	= input[i*2+1]*0.5;
		
		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted+=2;
	}
	
	//this is how we get the mean of rms :) 
	curVol /= (float)numCounted;
	
	// this is how we get the root of rms :) 
	curVol = sqrt( curVol );
	
	smoothedVol *= 0.93;
	smoothedVol += 0.07 * curVol;
	
	bufferCounter++;
	
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch (key) {
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
			break;
			
		case'p':
			bDrawPointCloud = !bDrawPointCloud;
			break;
			
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
			
		case '<':
		case ',':
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
			
		case '-':
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
			
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
			
		case 'o':
			kinect.setCameraTiltAngle(angle); // go back to prev tilt
			kinect.open();
			break;
			
		case 'c':
			kinect.setCameraTiltAngle(0); // zero the tilt
			kinect.close();
			break;
			
		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;

		case 's':
			soundStream.start();
			break;
			
		case 'e':
			soundStream.stop();
			break;

		case 'r':
			reloadShaders = true;
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}
