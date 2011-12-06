/*
 *  MyListener.h
 *  breakout
 *
 *  Created by lawsos2 on 4/10/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


struct MyContact {
    b2Fixture *fixtureA;
    b2Fixture *fixtureB;
    bool operator==(const MyContact& other) const
    {
        return (fixtureA == other.fixtureA) && (fixtureB == other.fixtureB);
    }
};




class MyListener : public b2ContactListener {
	
	public:
		vector<MyContact>_contacts;
		
		MyListener() : _contacts(){
		}
		
		
		
		virtual void contactAdd(ofPoint p) {
		}
		virtual void contactRemove(ofPoint p) {
		}
		
		void BeginContact(b2Contact *contact) {
			
			b2Fixture * fA = contact->GetFixtureA();
			b2Body * bodyA = fA->GetBody();
			Data * dataA = (Data *)bodyA->GetUserData();
			if (dataA == NULL) {
				return;
			}
			int tagA = dataA->tag;

			b2Fixture * fB = contact->GetFixtureB();
			b2Body * bodyB = fB->GetBody();
			Data * dataB = (Data *)bodyB->GetUserData();
			if (dataB == NULL) {
				return;
			}
			
			int tagB = dataB->tag;
			
//Can do something here if you want check stuff, as for deletion, whatever.
//			if( tagA == 1 || tagB == 1){ //ball
//				
//				if( tagA == 2 || tagB == 2){ // hits outline
//					
//				
//					
//				}else if( tagA == 3 ){ // hits brick
//					
//					dataA->bDelete = true;
//					
//				}else if( tagB == 3 ) { // hits brick
//					
//					dataB->bDelete = true;
//					
//				}
//
//			}
				
		}
		
		void EndContact(b2Contact *contact) {
		}
		
		void PreSolve(b2Contact *contact, const b2Manifold *oldManifold) {
		}
		
		void PostSolve(b2Contact *contact, const b2ContactImpulse *impulse) {
		}
};



