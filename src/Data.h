/*
 *  Data.h
 *  emptyExample
 *
 *  Created by lawsos2 on 4/7/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

class Data {
	public:
		int		r, g, b;
		int		tag;
		bool	drawable;
		float	radius;
		float	width, height;
		float	angle;
        float   lifetime;
		int		startX, startY;
		bool	bDelete;
	
		Data(){ 
			r = g = b = tag = 0;
			drawable = true;
			radius = 0.0f;
			width = height = 10;
			angle = 0;
            lifetime = ofGetElapsedTimef();//ofRandom(0, 3);
			bDelete = false;
		}
		
		void setupCustomData(int firstId) {
			tag = firstId;
			r  = ofRandom(0, 255);
			g  = ofRandom(0, 255);
			b  = ofRandom(0, 255);
			
		}
		void setupCustomColorR(int i) {
			switch(i){
			case 0:
				r  = 255;
				g  = 0;
				b  = 153;	
				break;
			case 1:
				r  = 255;
				g  = 0;
				b  = 0;	
				break;
			case 2:
				r  = 255;
				g  = 102;
				b  = 204;	
				break;
			case 3:
				r  = 255;
				g  = 51;
				b  = 51;	
				break;
			case 4:
				r  = 205;
				g  = 0;
				b  = 0;	
			break;
			}
		}
		void setupCustomColorB() {
			r  = 0 ;// ofRandom(0, 200);
			g  = ofRandom(100, 255);
			b  = ofRandom(1, 255);
		}


};