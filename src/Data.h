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
            lifetime = ofRandom(0, 3);
			bDelete = false;
		}
		
		void setupCustomData(int firstId) {
			tag = firstId;
			r  = ofRandom(0, 255);
			g  = ofRandom(0, 255);
			b  = ofRandom(0, 255);
			
		}
		

};