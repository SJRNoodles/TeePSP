#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <gfx.hpp>
#include <img.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <pspaudio.h>
#include <pspaudiolib.h>
#include <psppower.h>

#include <pspgu.h>
#include <glib2d.h>



//teeworlds psp
PSP_MODULE_INFO("tee", 0, 1, 0);
PSP_HEAP_SIZE_KB(-256);

int tick;
int bulletAllocated = 81;
int particleAllocated = 0;
int lvlSideCounter = 0;
// tee animations
int teeFeetAllRot = 0;
int teeBodyOff;
int teeFootMove;
int teeWepRot = -45;
int teeEyeTargetOne;
int teeEyeTargetTwo;
int teeEyeOne;
int teeEyeTwo;
int teeTimer = 0;
// tee stats
int health = 1;
int armor = 1;

int state = 0;
int running = 1;

int dir = 1; // directions for tee's eyes

int x = 0;
int y = 128;

int teeWep = 0;
int gravity = 0;
int doublejump = 0;
int jump_disable;

int teeEyeSX = 35;
int teeEyeSY = 51;

int nX;
int nY;

int camX;
int camY;

// collision (useful!)
bool collision(int x1,int y1,int x2,int y2,int w1,int h1,int w2, int h2,bool debug){
	if (debug==true) {
		g2dBeginRects(NULL);
		g2dSetColor(RED);	
		g2dSetScaleWH(w1,h1);
		g2dSetCoordXY(x1,y1); 
		g2dAdd();
		g2dEnd(); 
		g2dBeginRects(NULL);
		g2dSetColor(RED);	
		g2dSetScaleWH(w2,h2);
		g2dSetCoordXY(x2,y2); 
		g2dAdd();
		g2dEnd(); 
	}
	return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && h1 + y1 > y2);
}

bool inRange(unsigned low, unsigned high, unsigned x){        
 return (low <= x && x <= high);         
} 

int tilePos(int x){
	return(x * 32);
}

// for levels
struct Weapon {
	public:
	int scX;
	int scY;
	int sX;
	int sY;
};
// create tee
struct Tee {
	public:
	// tee animations
		int teeTimer = 0;
		int teeFeetAllRot = 0;
		int teeBodyOff;
		int teeFootMove;
		int teeWep = 0;
		int teeWepRot = -45;
		int teeEyeTargetOne;
		int teeEyeTargetTwo;
		int teeEyeOne;
		int teeEyeTwo;
		int teeEyeSX = 35;
		int teeEyeSY = 51;
		// tee stats
		int health = 1;
		int armor = 1;

		int state = 0;
		int running = 1;
	
		int dir = 1; // directions for tee's eyes
		int teeTargetDir = 1;
		int teeTarget;

		int x = 64;
		int y = 128;

		int gravity = 0;
		int doublejump = 0;
		int jump_disable;

		int nX;
		int nY;
		
		int tick=0;
};
// tiles
struct Tile {
public:
	int tX = 10;
	int tY = 10;
	int scX = 32;
	int scY = 32;
	int sX;
	int sY;
	int type;
	int sheet;
	bool en = true;
	bool hidden = false;
	int timer=0;
	int type2;
	int dir =1; // for dynamic tiles such as projectiles
	int grav = 0;
	int dir2 =1;
};


auto main() -> int {

	SceCtrlLatch latchData;
	
	g2dTexture* bg = g2dTexLoad("assets/bg.png",G2D_SWIZZLE);
	g2dTexture* mountains = g2dTexLoad("assets/mountains.png",G2D_SWIZZLE);
	g2dTexture* title = g2dTexLoad("assets/title.png",G2D_SWIZZLE);
	
	g2dTexture* game = g2dTexLoad("assets/game.png",G2D_SWIZZLE);
	
	g2dColors color[7] = {RED,ORANGE,YELLOW,GREEN,BLUE,VIOLET,ROSE};
	
	g2dTexture* plr_skin = g2dTexLoad("assets/tee_default.png",G2D_SWIZZLE);
	g2dTexture* grass = g2dTexLoad("assets/grass_main.png",G2D_SWIZZLE);
	g2dTexture* particles = g2dTexLoad("assets/particles.png",G2D_SWIZZLE);
	
	SceCtrlData ctrlData;
	// Tee Set up
	Tee ai[1];
	// Weapon set up
	Weapon weapons[5];
	weapons[0].sX = 32;
	weapons[0].sY = 21;
	weapons[0].scX = 63;
	weapons[0].scY = 38;
	
	weapons[1].sX = 35;
	weapons[1].sY = 67;
	weapons[1].scX = 54;
	weapons[1].scY = 28;
	
	weapons[2].sX = 33;
	weapons[2].sY = 98;
	weapons[2].scX = 113;
	weapons[2].scY = 28;
	
	weapons[3].sX = 35;
	weapons[3].sY = 129;
	weapons[3].scX = 104;
	weapons[3].scY = 30;
	
	// create level
	Tile lvl[128];
	// build lvl
	for (int i = 0; i < 16; i++) {
		lvl[i].en=true;
		lvl[i].tX = tilePos(i);
		lvl[i].tY = tilePos(1);
		lvl[i].scX = 32;
		lvl[i].scY = 32;
		if(i==0){
			lvl[i].sX = 32;
			lvl[i].sY = 64; // (0, 32) is the standard grass block in grass_main
		}else{
			lvl[i].sX = 0;
			lvl[i].sY = 32; // (0, 32) is the standard grass block in grass_main
			if(i==15){
				lvl[i].sX = 0;
				lvl[i].sY = 64; // (0, 32) is the standard grass block in grass_main
			}
		}
		lvl[i].sheet = 1;
		lvl[i].type=1;
	}
	for (int i = 0; i < 16; i++) {
		lvl[i+16].en=true;
		lvl[i+16].tX = tilePos(i);
		lvl[i+16].tY = tilePos(0);
		lvl[i+16].scX = 32;
		lvl[i+16].scY = 32;
		if(i==0){
			lvl[i+16].sX = 64;
			lvl[i+16].sY = 64; // (0, 32) is the standard grass block in grass_main
		}else{
			lvl[i+16].sX = 64;
			lvl[i+16].sY = 32; // (0, 32) is the standard grass block in grass_main
			if(i==15){
				lvl[i+16].sX = 94;
				lvl[i+16].sY = 64; // (0, 32) is the standard grass block in grass_main
			}
		}
		lvl[i+16].sheet = 1;
		lvl[i+16].type=1;
	}
	
	// leaves 48 leftover!
	// reserved for projectiles (when i make a struct for projectiles its glitchy and there isnt any fix)
	for (int i = 80; i < 128; i++) {
		lvl[i].tX = tilePos(1);
		lvl[i].tY = tilePos(1);
		lvl[i].scX = 19;
		lvl[i].scY = 9;
		lvl[i].sX = 103;
		lvl[i].sY = 76;
		lvl[i].sheet = 2;
		lvl[i].type=4;
		lvl[i].en = false;
	}
	
	while(running==1){
		if (state==0){
			g2dClear(WHITE);
			g2dBeginRects(title);
			g2dSetColor(WHITE);
			g2dSetScaleWH(480,272);
			g2dSetCoordXY(0,0);
			g2dAdd();
			g2dEnd();
			
			sceCtrlReadLatch(&latchData);
			
			sceCtrlReadBufferPositive(&ctrlData, 1);
			if (ctrlData.Buttons & PSP_CTRL_START) {
				state = 1;
			}
			if (ctrlData.Buttons & PSP_CTRL_SELECT) {
				running = 0;
			}
			
		}
		if(state==1){
			// bg
			g2dClear(WHITE);
			g2dBeginRects(bg);
			g2dSetColor(WHITE);
			g2dSetScaleWH(480,272);
			g2dSetCoordXY(0,0);
			g2dAdd();
			g2dEnd();
			g2dBeginRects(mountains); 
			g2dSetScaleWH(2048,1024);
			g2dSetCoordXY((camX-2048) / 10, (camY-5048)/10); 
			g2dSetCropXY(106,21); 
			g2dAdd();
			g2dEnd(); 
			
			//controls
			sceCtrlReadBufferPositive(&ctrlData, 1);
			if (ctrlData.Buttons & PSP_CTRL_LEFT) {
				teeBodyOff = rand() % 2 - 1;
				teeFootMove+=15;
				dir = -1;
				x+=5;
			}
			if (ctrlData.Buttons & PSP_CTRL_RIGHT) {
				teeBodyOff = rand() % 2 - 1;
				teeFootMove+=15;
				dir = 1;
				x-=5;
			}
			if (!(ctrlData.Buttons & PSP_CTRL_RIGHT) && !(ctrlData.Buttons & PSP_CTRL_LEFT)) {
				teeFootMove=0;
				teeBodyOff=0;
			}
			
			// rendering
			
			// camera stuff
			camX = x + 220;
			camY = y + 116;
			
			// level
			for (int i = 0; i < sizeof(lvl); i++) {
				if (lvl[i].sheet==1) {
				g2dBeginRects(grass); 
				}else{
					if(lvl[i].sheet==2){
						g2dBeginRects(game); 
					}
					if(lvl[i].sheet==3){
						g2dBeginRects(particles); 
					}
				}
				if(lvl[i].en == false){
					if(lvl[i].type == 2 || lvl[i].type == 3){
						lvl[i].timer+=1;
						if(lvl[i].timer>200){
							lvl[i].timer=0;
							lvl[i].en=true;
						}
					}
				}
				if(lvl[i].en == true){
					if(lvl[i].hidden == false){
						g2dSetScaleWH(lvl[i].scX,lvl[i].scY);
						g2dSetCoordXY((camX-lvl[i].tX),(camY-lvl[i].tY)); 
						g2dSetCropXY(lvl[i].sX,lvl[i].sY); 
						if (lvl[i].type == 5) {
							g2dSetScaleWH(lvl[i].timer,lvl[i].timer);
						}
						if (lvl[i].type == 4) {
							g2dSetScaleWH(lvl[i].scX/1.5,lvl[i].scY/1.5);
							if(lvl[i].type2==1){
								g2dSetRotation((lvl[i].dir * 90) + 90);
							}else{
								g2dSetRotation(lvl[i].dir2);
								g2dSetCoordMode(G2D_CENTER);
								lvl[i].dir2+=15;
							}
							g2dSetCropWH(lvl[i].scX,lvl[i].scY);
						}else{
							g2dSetCropWH(32,32);
						}
						g2dAdd();
						g2dEnd(); 
					}
					
					if (lvl[i].type == 4) {
						// projectiles
						lvl[i].tX += 25 * lvl[i].dir;
						lvl[i].timer+=1;
						if(lvl[i].timer>200){
							lvl[i].timer=0;
							lvl[i].en=false;
							bulletAllocated-=1;
						}
						// collision
						if(lvl[i].type2==2){
							lvl[i].tY += lvl[i].grav;
							lvl[i].grav -=1;
							lvl[i].tX -= 21 * lvl[i].dir;
						}
						if(lvl[i].type2==3){
							lvl[i].tX -= 20 * lvl[i].dir;
						}
						for (int i2 = 0; i2 < 1; i2++) {
							if(collision((camX-ai[i2].x),(camY-ai[i2].y),(camX-lvl[i].tX),(camY-lvl[i].tY),32,32,8,8,false)){
								// ai take damage
								ai[i2].x=ai[i2].nX;
								ai[i2].y = ai[i2].nY;
								ai[i2].health -= 2 / ((ai[i2].armor + 1));
								ai[i2].armor -= 4;
								if (ai[i2].armor < 1) {
									ai[i2].armor = 0;
								}
								if(lvl[i].type2 == 2){
									ai[i2].gravity = -10;
								}
								ai[i2].teeEyeSX = 67;
								ai[i2].teeEyeSY = 51;
								lvl[i].en = false;
								bulletAllocated -=1;
							}
						}
						for (int i2 = 0; i2 < sizeof(lvl); i2++) {
							if(collision((camX-lvl[i2].tX),(camY-lvl[i2].tY),(camX-lvl[i].tX),(camY-lvl[i].tY),4,4,8,8,false)){
								if(lvl[i2].type == 1){
									if(lvl[i].type == 4){
										lvl[i].en = false;
										bulletAllocated-=1;
										
									}
								}
							}
						}
					}
				}
			}
			//lvl[i].en = false;
			sceCtrlReadLatch(&latchData);
			if (latchData.uiBreak & PSP_CTRL_TRIANGLE){
				teeWep+=1;
				if (teeWep>5) {
					teeWep=0;
				}
			}
			if (latchData.uiBreak & PSP_CTRL_CIRCLE){
				if (lvl[bulletAllocated].type == 4) {
					if (teeWep == 0){
						teeWepRot = 85 * dir;
						
						lvl[bulletAllocated].en = true;
						lvl[bulletAllocated].hidden = true;
						lvl[bulletAllocated].tX = x - 11 + (-45 * dir);
						lvl[bulletAllocated].tY = y-15;
						lvl[bulletAllocated].dir = dir * -1;
						lvl[bulletAllocated].type2=3;
						lvl[bulletAllocated].type=4;
						lvl[bulletAllocated].timer = 190;
						lvl[bulletAllocated].scX = 19;
						lvl[bulletAllocated].scY = 9;
						lvl[bulletAllocated].sX = 103;
						lvl[bulletAllocated].sY = 76;
						lvl[bulletAllocated].sheet = 2;
						bulletAllocated+=1;
					}
					if (teeWep == 1){
						lvl[bulletAllocated].en = true;
						lvl[bulletAllocated].hidden = false;
						lvl[bulletAllocated].tX = x + (-45 * dir);
						lvl[bulletAllocated].tY = y-15;
						lvl[bulletAllocated].dir = dir * -1;
						lvl[bulletAllocated].type2=1;
						lvl[bulletAllocated].type=4;
						lvl[bulletAllocated].timer = 0;
						lvl[bulletAllocated].scX = 19;
						lvl[bulletAllocated].scY = 9;
						lvl[bulletAllocated].sX = 103;
						lvl[bulletAllocated].sY = 76;
						lvl[bulletAllocated].sheet = 2;
						bulletAllocated+=1;
					}
					if (teeWep == 2){
						for (int i = 0; i < 3; i++) {
							lvl[bulletAllocated].en = true;
							lvl[bulletAllocated].hidden = false;
							lvl[bulletAllocated].timer = 0;
							lvl[bulletAllocated].type=4;
							lvl[bulletAllocated].tX = x + (-45 * dir);
							lvl[bulletAllocated].tY = y -15 + (i*2);
							lvl[bulletAllocated].dir = dir * -1;
							lvl[bulletAllocated].type2=1;
							lvl[bulletAllocated].scX = 11;
							lvl[bulletAllocated].scY = 11;
							lvl[bulletAllocated].sX = 170;
							lvl[bulletAllocated].sY = 106;
							lvl[bulletAllocated].sheet = 2;
							bulletAllocated+=1;
						}
					}
					if (teeWep == 3){
						lvl[bulletAllocated].en = true;
						lvl[bulletAllocated].hidden = false;
						lvl[bulletAllocated].timer = 0;
						lvl[bulletAllocated].type=4;
						lvl[bulletAllocated].tX = x + (-45 * dir);
						lvl[bulletAllocated].tY = y-15;
						lvl[bulletAllocated].dir = dir * -1;
						lvl[bulletAllocated].type2=2;
						lvl[bulletAllocated].scX = 29;
						lvl[bulletAllocated].scY = 17;
						lvl[bulletAllocated].sX = 160;
						lvl[bulletAllocated].sY = 135;
						lvl[bulletAllocated].sheet = 2;
						lvl[bulletAllocated].grav = 10;
						bulletAllocated+=1;
					}
				}
			}
			
			
			// tee model
			// (camX - x) (camY - y)
		    // weapons
			if (teeWep==0) {
				g2dBeginRects(game); 
				g2dSetScaleWH(weapons[teeWep].scX / 1.3,weapons[teeWep].scY / 1.3);
				g2dSetCoordMode(G2D_CENTER);
				g2dSetCoordXY((camX - x + 20),(camY - y + 20)+teeBodyOff); 
				g2dSetCropWH(weapons[teeWep].scX,weapons[teeWep].scY);
				g2dSetCropXY(weapons[teeWep].sX,weapons[teeWep].sY); 
				g2dSetRotation(teeWepRot - 90);
				g2dAdd();
				g2dEnd(); 
				teeWepRot -= (teeWepRot - (-1 * dir * 45)) / 4;
			}else{
				g2dBeginRects(game); 
				g2dSetScaleWH(weapons[teeWep].scX / 1.5,weapons[teeWep].scY / 1.5);
				g2dSetCoordMode(G2D_CENTER);
				g2dSetCoordXY((camX - x +20+ dir * 15),(camY - y + 20)+teeBodyOff); 
				g2dSetCropWH(weapons[teeWep].scX,weapons[teeWep].scY);
				g2dSetCropXY(weapons[teeWep].sX,weapons[teeWep].sY); 
				g2dSetRotation(teeWepRot - 90);
				g2dAdd();
				g2dEnd(); 
				teeWepRot -= (teeWepRot - (dir * 95)) / 3;
			}
			// foot 2
			g2dBeginRects(plr_skin); 
			g2dSetScaleWH(18,11);
			g2dSetCoordMode(G2D_CENTER);
			g2dSetCoordXY((camX - x)+25+sin(teeFootMove * -5),(camY - y)+35+cos(teeFootMove * -15)); 
			g2dSetCropWH(13,7);
			g2dSetCropXY(106,21); 
			g2dSetRotation(teeFeetAllRot + sin(teeFootMove * -90));
			g2dAdd();
			g2dEnd(); 
			// tee body
			g2dBeginRects(plr_skin); 
			g2dSetScaleWH(43,38); 
			g2dSetCoordXY((camX - x),(camY - y)+teeBodyOff); 
			g2dSetCropWH(43,38); 
			g2dSetCropXY(3,1); 
			g2dAdd(); 
			g2dEnd(); 
			// foot 1
			g2dBeginRects(plr_skin); 
			g2dSetScaleWH(18,11);
			g2dSetCoordMode(G2D_CENTER);
			g2dSetCoordXY((camX - x)+15 + sin(teeFootMove * 5),(camY - y)+35 + cos(teeFootMove * 15)); 
			g2dSetCropWH(13,7);
			g2dSetCropXY(106,21); 
			g2dSetRotation(teeFeetAllRot + sin(teeFootMove * 90));
			g2dAdd();
			g2dEnd(); 
			
			teeEyeTargetOne =(camX - x)+11+dir*6;
			teeEyeTargetTwo = (camX - x)+33+dir*6;
			teeEyeOne -= (teeEyeOne - teeEyeTargetOne) / 2;
			teeEyeTwo -= (teeEyeTwo - teeEyeTargetTwo) / 2;
			
			// eye 1
			g2dBeginRects(plr_skin); 
			g2dSetScaleWH(13,13);
			g2dSetCoordXY(teeEyeOne,(camY - y)+15); 
			g2dSetCropWH(10,10);
			g2dSetCropXY(35,51); 
			g2dAdd();
			g2dEnd(); 
			// eye 2
			g2dBeginRects(plr_skin); 
			g2dSetScaleWH(-13,13);
			g2dSetCoordXY(teeEyeTwo,(camY - y)+15); 
			g2dSetCropWH(10,10);
			g2dSetCropXY(35,51); 
			g2dAdd();
			g2dEnd();
			
			// render gui elements (hearts,armor,etc)
			for (int i = 0; i < health; i++) {
				g2dBeginRects(game); 
				g2dSetScaleWH(20,20);
				g2dSetCoordXY(i*20,0); 
				g2dSetCropWH(28,26);
				g2dSetCropXY(338,3); 
				g2dAdd();
				g2dEnd();
			}
			for (int i = 0; i < armor; i++) {
				g2dBeginRects(game); 
				g2dSetScaleWH(20,20);
				g2dSetCoordXY(i*20,20); 
				g2dSetCropWH(23,28);
				g2dSetCropXY(340,33); 
				g2dAdd();
				g2dEnd();
			}
			
			
			// level col
			y-=gravity;
			gravity+=1;
			if (gravity>2) {
				teeFeetAllRot = 45;
				if (ctrlData.Buttons & PSP_CTRL_CROSS) {
					if(doublejump=1){
						if(jump_disable==false){
						jump_disable=true;
						doublejump=2;
						gravity=-15;
						y-=gravity;
						}
					}
				}
			}	
			
			for (int i = 0; i < 1; i++) {
				
				// tee model
				// (camX - x) (camY - y)
				// weapons
				// foot 2
				g2dBeginRects(plr_skin); 
				g2dSetScaleWH(18,11);
				g2dSetCoordMode(G2D_CENTER);
				g2dSetCoordXY((camX - ai[i].x)+25+sin(ai[i].teeFootMove * -5),(camY - ai[i].y)+35+cos(ai[i].teeFootMove * -15)); 
				g2dSetCropWH(13,7);
				g2dSetCropXY(106,21); 
				g2dSetRotation(ai[i].teeFeetAllRot + sin(ai[i].teeFootMove * -90));
				g2dAdd();
				g2dEnd(); 
				// tee body
				g2dBeginRects(plr_skin); 
				g2dSetScaleWH(43,38); 
				g2dSetCoordXY((camX - ai[i].x),(camY - ai[i].y)+ai[i].teeBodyOff); 
				g2dSetCropWH(43,38); 
				g2dSetCropXY(3,1); 
				g2dAdd(); 
				g2dEnd(); 
				// foot 1
				g2dBeginRects(plr_skin); 
				g2dSetScaleWH(18,11);
				g2dSetCoordMode(G2D_CENTER);
				g2dSetCoordXY((camX - ai[i].x)+15 + sin(ai[i].teeFootMove * 5),(camY - ai[i].y)+35 + cos(ai[i].teeFootMove * 15)); 
				g2dSetCropWH(13,7);
				g2dSetCropXY(106,21); 
				g2dSetRotation(ai[i].teeFeetAllRot + sin(ai[i].teeFootMove * 90));
				g2dAdd();
				g2dEnd(); 
				
				ai[i].teeEyeTargetOne =(camX - ai[i].x)+11+ai[i].dir*6;
				ai[i].teeEyeTargetTwo = (camX - ai[i].x)+32+ai[i].dir*6;
				ai[i].teeEyeOne -= (ai[i].teeEyeOne - ai[i].teeEyeTargetOne) / 2;
				ai[i].teeEyeTwo -= (ai[i].teeEyeTwo - ai[i].teeEyeTargetTwo) / 2;
				
				// eye 1
				g2dBeginRects(plr_skin); 
				g2dSetScaleWH(13,13);
				g2dSetCoordXY(ai[i].teeEyeOne,(camY - ai[i].y)+15); 
				g2dSetCropWH(10,10);
				g2dSetCropXY(ai[i].teeEyeSX,ai[i].teeEyeSY); 
				g2dAdd();
				g2dEnd(); 
				// eye 2
				g2dBeginRects(plr_skin); 
				g2dSetScaleWH(-13,13);
				g2dSetCoordXY(ai[i].teeEyeTwo,(camY - ai[i].y)+15); 
				g2dSetCropWH(10,10);
				g2dSetCropXY(ai[i].teeEyeSX,ai[i].teeEyeSY); 
				g2dAdd();
				g2dEnd();
				
				if(ai[i].teeEyeSX != 35){
						// not default eyes
						ai[i].teeTimer+=1;
				}
				if(ai[i].teeTimer > 19){
					ai[i].teeEyeSX=35;
					ai[i].teeEyeSY=51;
					ai[i].teeTimer=0;
				}
				
				ai[i].gravity -= 1;
				ai[i].nX=ai[i].x;
				ai[i].nY=ai[i].y;
				
				ai[i].y+=ai[i].gravity;
				
				if(ai[i].y<-64){
					ai[i].health = 0;
					ai[i].armor = 0;
				}
				
				// targeting system
				ai[i].teeTarget = x; // ai target
				ai[i].teeTargetDir = (ai[i].teeTarget - ai[i].x)/abs(ai[i].teeTarget - ai[i].x); // basic targeting
				if (!inRange(ai[i].teeTarget - 100,ai[i].teeTarget+100,ai[i].x)) {
					ai[i].x+= 5 * ai[i].teeTargetDir;
				}
				
				if (ai[i].health<1){
					ai[i].tick+=1;
					if(ai[i].tick>99){
						// reset tee vars
						ai[i].x = 0;
						ai[i].y = 128;
						ai[i].gravity = 0;
						ai[i].health = 10;
						ai[i].armor = 0;
						// reset tick
						ai[i].tick=0;
					}
				}
				
			}
			
			for (int i = 0; i < sizeof(lvl); i++) {
				for (int i2 = 0; i2 < 1; i2++) {
				if(collision((camX-ai[i2].x),(camY-ai[i2].y),(camX-lvl[i].tX),(camY-lvl[i].tY),43,38,32,32,false)){
					if(lvl[i2].type == 1){
						ai[i2].jump_disable=false;
						ai[i2].gravity=1;
						ai[i2].y = ai[i2].nY;
					
						if(collision((camX-ai[i2].x),(camY-ai[i2].y),(camX-lvl[i].tX),(camY-lvl[i].tY),43,38,32,32,false)){
							ai[i2].x=ai[i2].nX;
						}
						

					
						ai[i2].teeFeetAllRot = 0;
						ai[i2].doublejump=0;
						// controls part 2 :D
						if (ctrlData.Buttons & PSP_CTRL_CROSS) {
							ai[i2].doublejump+=1;
							ai[i2].gravity=-15;
							ai[i2].y-=gravity;
						}
					}else{
						if(lvl[i].type == 4){
							ai[i2].x=ai[i2].nX;
							ai[i2].y = ai[i2].nY;
							ai[i2].health -= 2 / ((ai[i2].armor + 1));
								ai[i2].armor -= 4;
								if (ai[i2].armor < 1) {
									ai[i2].armor = 0;
								}
								if(lvl[i].type2 == 2){
									ai[i2].gravity = -10;
								}
								ai[i2].teeEyeSX = 67;
								ai[i2].teeEyeSY = 51;
								lvl[i].en = false;
								bulletAllocated -=1;
						}
						if(lvl[i].type == 5){
							ai[i2].x=ai[i2].nX;
							ai[i2].y = ai[i2].nY;
						}
					}
				}
				}
				if(collision((camX-x),(camY-y),(camX-lvl[i].tX),(camY-lvl[i].tY),43,38,32,32,false)){
					if(lvl[i].en == true){
						if(lvl[i].type == 1){
						jump_disable=false;
						gravity=1;
						y = nY;
					
						if(collision((camX-x),(camY-y),(camX-lvl[i].tX),(camY-lvl[i].tY),43,38,32,32,false)){
							x=nX;
						}
						

					
						teeFeetAllRot = 0;
						doublejump=0;
						// controls part 2 :D
						if (ctrlData.Buttons & PSP_CTRL_CROSS) {
							doublejump+=1;
							gravity=-15;
							y-=gravity;
						}
						}else{
							if(lvl[i].type == 2){
								if(health<10){
									health+=1;
									lvl[i].en = false;
								}
							}
							if(lvl[i].type == 3){
								if(armor<10){
									armor+=1;
									lvl[i].en = false;
								}
							}
							if(lvl[i].type == 4){
								health -= 2 / ((armor + 1));
								armor -= 4;
								if (armor < 1) {
									armor = 0;
								}
								lvl[i].en = false;
								bulletAllocated -=1;
							}
							if(lvl[i].type == 5){
								health -= 8 / ((armor + 1));
								armor -= 8;
								if (armor < 1) {
									armor = 0;
								}
								lvl[i].en = false;
								bulletAllocated -=1;
							}
							if(lvl[i].type == 6){
								health -= 3 / ((armor + 1));
								armor -= 3;
								if (armor < 1) {
									armor = 0;
								}
								lvl[i].en = false;
								bulletAllocated -=1;
							}
						}
					}
				}
			}
				
			nX = x;
			nY = y;
			
			if(y<-64){
				health = 0;
				armor = 0;
			}
			
			
			
			if (health<1){
				tick+=1;
				if(tick>99){
					// reset tee vars
					x = 0;
					y = 128;
					gravity = 0;
					health = 10;
					armor = 0;
					// reset tick
					tick=0;
				}
			}
		}
			g2dFlip(G2D_VSYNC);
		
	}
	sceKernelExitGame();
	return 0;
	
	
}

