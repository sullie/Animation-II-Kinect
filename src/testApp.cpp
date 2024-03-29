#include "testApp.h"

int numBlobs = 0;
float smoothPct = 0.75f;
int numSmoothContours =0;
int tolerance = 4;
float lineFriction = 0.2f;
float lineRestitution =0.96f;
float lineDensity = 1.0f;


bool doDrawBox2DCountours=false;
//--------------------------------------------------------------------------------------------------------------
void testApp::setup() {
	//ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
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
	
	nearThreshold = 230;  //255
	farThreshold = 5;   //200
	
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
	
	plotHeight = 128;
	bufferSize = 512;
    
	fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING);
    
    drawBuffer.resize(bufferSize);
	middleBuffer.resize(bufferSize);
	audioBuffer.resize(bufferSize);
	
	drawBins.resize(fft->getBinSize());
	middleBins.resize(fft->getBinSize());
	audioBins.resize(fft->getBinSize());
    
    ofSoundStreamSetup(0, 1, this, 44100, bufferSize, 4);
    
	mode = MIC;
    bDrawSound = false;
    //end sound

	// Test images
	ocean.loadImage("images/ocean.jpg");
	desert.loadImage("images/back.jpg");

	// Set up shaders for the mask
	shader.load("shaders/test.vert", "shaders/test.frag");
	shader.begin();
	shader.setUniformTexture("tex", ocean.getTextureReference(), 0);
	//shader.setUniformTexture("tex1", kinect gray image set down in draw function, since it changes per frame, 1);
	shader.setUniformTexture("tex2", desert.getTextureReference(), 2);
	shader.end();

	// Box	2D
	initBox2D();

	// ElectricSheep video
	//sheepVideo.loadMovie("videos/Electric Sheep Fractals.mp4");
	sheepVideo.loadMovie("videos/Animation2 video.avi");
	sheepVideo.play();
	sheepVideo.setLoopState(OF_LOOP_PALINDROME);
	ofSetBackgroundAuto(false);
    	ofSetFrameRate(30);
}

//--------------------------------------------------------------------------------------------------------------
void testApp::update() {
	
	myWorld->Step((1.0f/30.0f), 10, 5);
	
	ofBackground(255);

	if (reloadShaders)
	{
		shader.load("shaders/test.vert", "shaders/test.frag");
		printf("Reloaded the shaders");
		reloadShaders = false;
	}

 //interate over objects, make any updates
    //push objects we want to keep in a new vector
    //push objects we want to delete in a new vector
    vector<b2Body*> tempObjects;
	for(int i = 0; i < objects.size(); i++) { 
		b2Body *b = objects[i];
		if (b->GetUserData() != NULL) {
            Data *d = (Data *)b->GetUserData(); 
            if( d->bDelete == true ){
				toDestroy.push_back( b );
			}else{
				tempObjects.push_back( b );
			}
		}		
	}
     //push around objects to keep
    objects.clear();
	objects = tempObjects;
	tempObjects.clear();
	
    //delete objects we don't want
	doDestroy();

	kinect.update();
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		grayImage.mirror( false, true );
		
		// or we do it ourselves - show people how they can work with the pixels
		unsigned char * pix = grayImage.getPixels();
		int numPixels = grayImage.getWidth() * grayImage.getHeight();
		
		for(int i = 0; i < numPixels; i++) 
		{
			if(pix[i] < nearThreshold && pix[i] > farThreshold) 
			{
				pix[i] = 255;
			} else {
				pix[i] = 0;
			}
		}
		
		grayThreshNear = grayImage;
        grayThreshFar = grayImage;
        grayThreshNear.threshold(nearThreshold);
        grayThreshFar.threshold(farThreshold);
        cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		
		// update the cv images
		grayImage.flagImageChanged();
		outlines.clear();
		
    	contourFinder.findContours(grayImage, 1290, (kinect.width*kinect.height)/2, 5, false); 
		makeOutlines();		

	}
	
	checkDestroy();
	sheepVideo.update();

}

//--------------------------------------------------------------------------------------------------------------
void testApp::draw()
{
	ofSetColor(255);

	// Create mask image for shader
	unsigned char * pixels = grayImage.getPixels();
	mask.setFromPixels(pixels, 640, 480, OF_IMAGE_GRAYSCALE,true);
	
	
	// Starting shader
	shader.begin();
	shader.setUniformTexture("tex", sheepVideo.getTextureReference(), 0);
	shader.setUniformTexture("tex1", mask.getTextureReference(), 1);
	shader.setUniformTexture("tex2", desert.getTextureReference(), 2);
	
	glActiveTexture(GL_TEXTURE0_ARB);
	sheepVideo.getTextureReference().bind();

	glActiveTexture(GL_TEXTURE1_ARB);
	mask.getTextureReference().bind();

	glActiveTexture(GL_TEXTURE2_ARB);
	desert.getTextureReference().bind();

	glBegin(GL_QUADS);
		
		glMultiTexCoord2d(GL_TEXTURE0_ARB, 0, 0);
		glMultiTexCoord2d(GL_TEXTURE1_ARB, 0, 0);
		glMultiTexCoord2d(GL_TEXTURE2_ARB, 0, 0);
		glVertex2f(0, 0);

		glMultiTexCoord2d(GL_TEXTURE0_ARB, sheepVideo.getWidth(), 0);
		glMultiTexCoord2d(GL_TEXTURE1_ARB, mask.getWidth(), 0);
		glMultiTexCoord2d(GL_TEXTURE2_ARB, desert.getWidth(),0);
		glVertex2f(ofGetWidth(), 0);

		glMultiTexCoord2d(GL_TEXTURE0_ARB, sheepVideo.getWidth(), sheepVideo.getHeight());
		glMultiTexCoord2d(GL_TEXTURE1_ARB, mask.getWidth(), mask.getHeight());
		glMultiTexCoord2d(GL_TEXTURE2_ARB, desert.getWidth(), desert.getHeight() );
		glVertex2f(ofGetWidth(), ofGetHeight());

		glMultiTexCoord2d(GL_TEXTURE0_ARB, 0, sheepVideo.getHeight());
		glMultiTexCoord2d(GL_TEXTURE1_ARB, 0, mask.getHeight() );
		glMultiTexCoord2d(GL_TEXTURE2_ARB, 0, desert.getHeight());
		glVertex2f(0, ofGetHeight());

		
	glEnd();

	glActiveTexture(GL_TEXTURE2_ARB);
	desert.getTextureReference().unbind();

	glActiveTexture(GL_TEXTURE1_ARB);
	mask.getTextureReference().unbind();

	glActiveTexture(GL_TEXTURE0_ARB);
	sheepVideo.getTextureReference().unbind();

	shader.end();
	
	
	
	//start draw from outlines
    /*ofSetColor(255,0,0);
    ofPushMatrix();
   // ofScale(1.25, 1.25, 0); 
	1024x768	ofScale(1.6, 1.6, 0);
	for (int i=0;i<outlines.size();i++)
	{
		outlines[i].draw();
	}
	ofPopMatrix(); */
	
	//end draw from outlines

  //start draw everything else box2d
    for(int i = 0; i < objects.size(); i++) { 
		b2Body *b = objects[i];
        if (b->GetUserData() != NULL) {
            Data *data = (Data *)b->GetUserData();  
			
			if (data->tag == 4 ){ //particles
				ofSetColor(data->r, data->g, data->b);
				ofCircle(b->GetPosition().x*B2SCALE, b->GetPosition().y*B2SCALE, data->radius);
               // cout<< b->GetPosition().x*B2SCALE << " " << b->GetPosition().y*B2SCALE << endl;
			}
            
        }        
    }	
    //end draw everything else box2d
	
	if(bDrawSound){
        soundMutex.lock();
        drawBuffer = middleBuffer;
        drawBins = middleBins;
        soundMutex.unlock();
        
        ofSetColor(255);
        ofPushMatrix();
        ofTranslate(16, 16);
        ofDrawBitmapString("Time Domain", 0, 0);
        plot(drawBuffer, plotHeight / 2, 0);
        ofTranslate(0, plotHeight + 16);
        ofDrawBitmapString("Frequency Domain", 0, 0);
        plot(drawBins, -plotHeight, plotHeight / 2);
        ofPopMatrix();
        
        ofPushMatrix();
        for (int i = 0; i<4; i++) { //each bar
            float myHeight = 0.0;
            for (int j = i*12; j < (i+1)*12; j++) {
                myHeight += drawBins[j];//fft->getAmplitudeAtBin(j+.1);
            }
            ofFill();
			ofSetColor(255, 0, 0);
           // printf("%f\n", myHeight);
            ofRect(i*10+10, 300, 10, (myHeight*300.0));
        }
        ofPopMatrix();
    }
	if(bDrawPointCloud) {
		easyCam.begin();
		drawPointCloud();
		easyCam.end();
	} 

	ofFill();
	for(int i=0; i<redCircles.size(); i++)
	{
		//ofVec2f *d  = new ofVec2f(0.0,-20.0*redCircles[i].density);  //adding force gto change gravity
		//redCircles[i].addForce(*d,5);
		Data *data = (Data*)redCircles[i].bodyDef.userData;
		ofSetColor(data->r,data->g,data->b,150);  //reds
		redCircles[i].draw();
	}

	for(int i=0; i<blueCircles.size(); i++)
	{
		Data *data = (Data*)blueCircles[i].bodyDef.userData;
		ofSetColor(data->r,data->g,data->b,150);  //blues
		blueCircles[i].draw();
	}
	

	// draw instructions
	ofSetColor(255, 255, 255);
	stringstream reportStream;
	reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
	<< ofToString(kinect.getMksAccel().y, 2) << " / "
	<< ofToString(kinect.getMksAccel().z, 2) << endl
	<< "set near threshold " << nearThreshold << " (press: + -)" << endl
	<< "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
	<< ", fps: " << ofGetFrameRate() << endl;
	ofDrawBitmapString(reportStream.str(),20,652);
	

/*	ofSetColor(255);

	desert.draw(0, 0);

	ofEnableAlphaBlending();
	float wave = sin(ofGetElapsedTimef());
	ofSetColor(255, 255, 255, scaledVol * 255);
	ocean.draw(0, 0);
	ofDisableAlphaBlending();*/
}
//--------------------------------------------------------------------------------------------------------------
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
				//ofVec3f scaler  = ofVec3f(1.0, 1.0, scaledVol);
				ofVec3f temp = kinect.getWorldCoordinateAt(x, y);
				//mesh.addVertex( temp * scaler );
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
//--------------------------------------------------------------------------------------------------------------
void testApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
	
}

//--------------------------------------------------------------------------------------------------------------
// BOX 2D
//--------------------------------------------------------------------------------------------------------------
void testApp::initBox2D()
{
//world setup
	myWorld = new b2World(b2Vec2(0.0f, 10.0f)); //(gravity);

	b2BodyDef bd;
	ground = myWorld->CreateBody(&bd);	
	b2EdgeShape shapeR, shapeL, shapeT, shapeB;
	
	//right
	b2FixtureDef rightDef;
	shapeR.Set(b2Vec2(ofGetWidth()/B2SCALE, 0), 
			   b2Vec2(ofGetWidth()/B2SCALE, ofGetHeight()/B2SCALE));
	rightDef.shape = &shapeR;
	rightDef.filter.categoryBits = CATEGORY_GROUND;
	rightDef.filter.maskBits = MASK_GROUND;
	ground->CreateFixture(&rightDef);

	//left
	b2FixtureDef leftDef;
	shapeL.Set(b2Vec2(0, 0), 
			   b2Vec2(0, ofGetHeight()/B2SCALE));
	leftDef.shape = &shapeL;
	leftDef.filter.categoryBits = CATEGORY_GROUND;
	leftDef.filter.maskBits = MASK_GROUND;
	ground->CreateFixture(&leftDef);
	
	//top
	b2FixtureDef topDef;
	shapeT.Set(b2Vec2(0, 0), 
			   b2Vec2(ofGetWidth()/B2SCALE, 0));
	topDef.shape = &shapeT;
	topDef.filter.categoryBits = CATEGORY_GROUND;
	topDef.filter.maskBits = MASK_GROUND;
	ground->CreateFixture(&topDef);
	
	//bottom
	b2FixtureDef bottomDef;
	shapeB.Set(b2Vec2(0, ofGetHeight()*1.6/B2SCALE), 
			   b2Vec2(ofGetWidth()/B2SCALE, ofGetHeight()/B2SCALE));
	bottomDef.shape = &shapeB;
	bottomDef.filter.categoryBits = CATEGORY_GROUND;
	bottomDef.filter.maskBits = MASK_GROUND;
	ground->CreateFixture(&bottomDef);
    
    //create new collision listenter
	listener = new MyListener();
	myWorld->SetContactListener(listener);
	
}

//--------------------------------------------------------------------------------------------------------------
void testApp::createCircle()
{

	ofxBox2dCircle redCircle;
	redCircle.fixture.filter.categoryBits = CATEGORY_RED_BALLS;
	redCircle.fixture.filter.maskBits = MASK_RED_BALLS;
	//redCircle.fixture.filter.groupIndex  = RED_GROUP;

	Data* ballR = new Data;
	ballR->setupCustomData(4);
	ballR->radius = ofRandom(5.0, 20.0);
	ballR->setupCustomColorR(ofRandom(0, 4));

	redCircle.bodyDef.userData = ballR;
	int bounce=ofRandom(0.5, 4.0);
	int densety = ofRandom(3, 5);
	redCircle.setPhysics(densety,bounce, 0.1);

	redCircle.setup(myWorld, mouseX, mouseY, ballR->radius);
	redCircles.push_back(redCircle);
	  
	ofxBox2dCircle blueCircle;
	blueCircle.fixture.filter.categoryBits = CATEGORY_BLUE_BALLS;
	blueCircle.fixture.filter.maskBits = MASK_BLUE_BALLS;
	//blueCircle.fixture.filter.groupIndex  = BLUE_GROUP;

	Data* ballB = new Data;
	ballB->setupCustomData(4);
	ballB->radius = ofRandom(5.0, 20.0);
	ballB->setupCustomColorB();

	blueCircle.bodyDef.userData = ballB;
	bounce=ofRandom(0.5, 2.0); 
	densety = ofRandom(3, 5);

	blueCircle.setPhysics(densety, bounce, 0.1);
	blueCircle.setup(myWorld, mouseX + 10, mouseY, ballB->radius);
	blueCircles.push_back(blueCircle);
}
void testApp::createRC()
{
	
	ofxBox2dCircle redCircle;
	redCircle.fixture.filter.categoryBits = CATEGORY_RED_BALLS;
	redCircle.fixture.filter.maskBits = MASK_RED_BALLS;
	//redCircle.fixture.filter.groupIndex  = RED_GROUP;

	Data* ballR = new Data;
	ballR->setupCustomData(4);
	ballR->radius = ofRandom(5.0, 20.0);
	ballR->setupCustomColorR(ofRandom(0, 4));

	redCircle.bodyDef.userData = ballR;
	int bounce=ofRandom(0.5, 2.0);
	int densety = ofRandom(3, 5);
	redCircle.setPhysics(densety,bounce, 0.1);
	int x = ofRandom(0,kinect.width);
	int y = 0;

	redCircle.setup(myWorld, x, y, ballR->radius);
	redCircles.push_back(redCircle);
}
void testApp::createBC()
{
	ofxBox2dCircle blueCircle;
	blueCircle.fixture.filter.categoryBits = CATEGORY_BLUE_BALLS;
	blueCircle.fixture.filter.maskBits = MASK_BLUE_BALLS;
	//blueCircle.fixture.filter.groupIndex  = BLUE_GROUP;

	Data* ballB = new Data;
	ballB->setupCustomData(4);
	ballB->radius = ofRandom(5.0, 20.0);
	ballB->setupCustomColorB();

	blueCircle.bodyDef.userData = ballB;
	int bounce=ofRandom(0.5, 2.0);
	int densety = ofRandom(3, 5);
	int x = ofRandom(0,kinect.width);
	int y = 0;
	blueCircle.setPhysics(densety, bounce, 0.1);
	blueCircle.setup(myWorld, x, y, ballB->radius);
	blueCircles.push_back(blueCircle);
}

//--------------------------------------------------------------------------------------------------------------
void testApp::makeOutlines(){	
		
	for (int i = 0; i < contourFinder.blobs.size(); i++)
	{
		ofPolyline contour;
		contour.addVertexes( contourFinder.blobs[i].pts );
		contour.setClosed(true);
        //simplify can be modified. I think 1 means no simplify, 
        //things get really square above 5
        //1.5-2.0 seems reasonable
		contour.simplify( 1.88 );
        
        vector <b2Vec2> outlinePoints;
		
		for (int j=0; j<contour.size(); j++){
/*1024x768 */           outlinePoints.push_back( b2Vec2(contour[j].x*1.6/(B2SCALE), (contour[j].y)*1.6/B2SCALE)) ;
           // outlinePoints.push_back( b2Vec2(contour[j].x*1.25/(B2SCALE), (contour[j].y)*1.25/B2SCALE)) ;
            
		}
		
        //push into drawable
		outlines.push_back( contour );
		
        //push into box2D
		b2ChainShape outlineLoopShape;
		outlineLoopShape.CreateLoop( &outlinePoints[0], contour.size() );  //Dont' forget the vector to array hack on this line!!!!!
        
		b2FixtureDef outlineShapeDef;
		outlineShapeDef.shape=&outlineLoopShape;
		outlineShapeDef.filter.categoryBits = CATEGORY_GROUND;
		outlineShapeDef.filter.maskBits = MASK_GROUND;

		Data* outlineDataP = new Data;
		outlineDataP->setupCustomData(2);
		outlineDataP->r = 128;
		outlineDataP->g = 0;
		outlineDataP->b = 0;
		outlineDataP->bDelete = true; //flag for deletion on next frame
		
		b2BodyDef outlineBodyDef;
		outlineBodyDef.userData = outlineDataP;

		
		b2Body* outlineBody = myWorld->CreateBody(&outlineBodyDef);

		outlineBody->CreateFixture(&outlineShapeDef);
		
		objects.push_back( outlineBody );
		outlineBody = NULL;
		outlineDataP = NULL;	
	}
	
	
}
//--------------------------------------------------------------------------------------------------------------
void testApp::spawnParticles( float brickX, float brickY ){
	
	int i = ofRandom(4, 20);
	
	
	for (; i>0; i--) {
		
		Data* particleP = new Data;
		particleP->setupCustomData(4);
		particleP->radius = ofRandom(5.0, 20.0);
		
		b2BodyDef particleBodyDef;
        particleBodyDef.type = b2_dynamicBody;
        particleBodyDef.position.Set(brickX/B2SCALE, brickY/B2SCALE);
		
        particleBodyDef.userData = particleP;
        b2Body *particleBody = myWorld->CreateBody(&particleBodyDef);
		
		
		b2CircleShape circle;
        circle.m_radius = particleP->radius/B2SCALE;
		
		b2FixtureDef particleShapeDef;
        particleShapeDef.shape = &circle;
        particleShapeDef.density = 1.0f;
        particleShapeDef.friction = 0.9f; 
        particleShapeDef.restitution = 0.5f;
        particleBody->CreateFixture(&particleShapeDef);
		particleBody->ApplyLinearImpulse(b2Vec2( ofRandom(-1.0, 1.0), ofRandom(-1.0, 1.0) ), 
										 particleBody->GetPosition());
		objects.push_back( particleBody );
	}
	
}

void testApp::checkDestroy()
{
	float wave =ofGetElapsedTimef();
	int i=0;
	while(i<redCircles.size())
	{
		Data *d=(Data*)redCircles[i].bodyDef.userData;
		if( (wave - d->lifetime) >=500)
		{
			redCircles.erase(redCircles.begin() + i);
		}
		else {i++;}
	}
	i=0;
	while( i<blueCircles.size())
	{
		Data *d=(Data*)blueCircles[i].bodyDef.userData;
		if( (wave - d->lifetime) >=500)
		{
			blueCircles.erase(blueCircles.begin() + i);
		}
		else {i++;}
	}

}
//--------------------------------------------------------------------------------------------------------------
void testApp::doDestroy(){
	std::vector<b2Body*>::iterator pos;
	for(pos = toDestroy.begin(); pos != toDestroy.end(); ++pos)
	{
		b2Body *b = *pos;
		Data * d = (Data *)b->GetUserData();
		if (d != NULL ) {
			delete d;
		}
		myWorld->DestroyBody( b );
	}
	
	toDestroy.clear();
}

//--------------------------------------------------------------------------------------------------------------
// Audio
//--------------------------------------------------------------------------------------------------------------
void testApp::plot(vector<float>& buffer, float scale, float offset) {
	ofNoFill();
	int n = buffer.size();
	ofRect(0, 0, n, plotHeight);
	glPushMatrix();
	glTranslatef(0, plotHeight / 2 + offset, 0);
	ofBeginShape();
	for (int i = 0; i < n; i++) {
		ofVertex(i, buffer[i] * scale);
	}
	ofEndShape();
	glPopMatrix();
}

//--------------------------------------------------------------------------------------------------------------
void testApp::audioReceived(float* input, int bufferSize, int nChannels) {
	if (mode == MIC) {
		// store input in audioInput buffer
		memcpy(&audioBuffer[0], input, sizeof(float) * bufferSize);
		
		float maxValue = 0;
		for(int i = 0; i < bufferSize; i++) {
			if(abs(audioBuffer[i]) > maxValue) {
				maxValue = abs(audioBuffer[i]);
			}
		}
		for(int i = 0; i < bufferSize; i++) {
			audioBuffer[i] /= maxValue;
			//cout<<"audio buffer "<<audioBuffer[i]<<endl;
		}
		
	} else if (mode == NOISE) {
		for (int i = 0; i < bufferSize; i++)
			audioBuffer[i] = ofRandom(-1, 1);
	} 
	
	fft->setSignal(&audioBuffer[0]);
    
	float* curFft = fft->getAmplitude();
	memcpy(&audioBins[0], curFft, sizeof(float) * fft->getBinSize());
    
	float maxValue = 0;
	for(int i = 0; i < fft->getBinSize(); i++) {
		if(abs(audioBins[i]) > maxValue) {
			maxValue = abs(audioBins[i]);
		}
	}
	for(int i = 0; i < fft->getBinSize(); i++) {
		audioBins[i] /= maxValue;
		//cout<<"audio bin "<<audioBins[i]<<endl;
	}
	
	//if(maxValue > 0.7) { createBC();}
	//else if(maxValue> 0.5){createRC();}

	soundMutex.lock();
	middleBuffer = audioBuffer;
	middleBins = audioBins;
	soundMutex.unlock();
    
}

//--------------------------------------------------------------------------------------------------------------
// kEYs
//--------------------------------------------------------------------------------------------------------------
void testApp::keyPressed (int key) {

	switch (key) {
	case 'f':
		ofSetFullscreen(true);
		break;
		case ' ':
			 spawnParticles(ofRandom(10, ofGetWidth()-10), 10);
			// spawnParticles(ofRandom(10, 200), 10);
			break;
		case 'r':
			reloadShaders = true;
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
		 case 's':
            bDrawSound = !bDrawSound;
            break;	
		case 'c':
			kinect.setCameraTiltAngle(0); // zero the tilt
			kinect.close();
			break;
		
		case 'a':
			createCircle();
			//createBC();
			//createRC();
			break;
		case 'b':
			doDrawBox2DCountours = !doDrawBox2DCountours;
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
