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

#include <algorithm> 

#include <pspgu.h>
#include <glib2d.h>

// maps
#include "dm1.h"



//teeworlds psp
PSP_MODULE_INFO("tee", 0, 1, 0);
PSP_HEAP_SIZE_KB(-256);

int zoom = 1;
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

int x = -576;
int y = 0;

int teeWep = 0;
int gravity = 0;
int doublejump = 0;
int jump_disable;

int teeEyeSX = 35;
int teeEyeSY = 51;

int nX = x;
int nY = y;

int camX;
int camY;

int airCounter = 2;

// level tile variables!
int levelCountX = 20;
int levelCountY = 30;


int cloneCountY=30;
int cloneCountX=20;
int cloneNumberX=0; // for loops are janky

// collision (useful!)
bool collision(int x1,int y1,int x2,int y2,int w1,int h1,int w2, int h2,bool debug,int sx){
	if (debug==true) {
		if (sx>2) {
			g2dBeginRects(NULL);
			g2dSetColor(BLUE);	
			g2dSetScaleWH(w1,h1);
			g2dSetCoordXY(x1,y1); 
			g2dAdd();
			g2dEnd(); 
			g2dBeginRects(NULL);
			g2dSetColor(RED);	
			g2dSetScaleWH(w2/2,h2/2);
			g2dSetCoordXY(x2,y2); 
			g2dAdd();
			g2dEnd(); 
		}
	}
	return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && h1 + y1 > y2);
}

bool inRange(unsigned low, unsigned high, unsigned x){        
 return (low <= x && x <= high);         
} 

int tilePos(int x){
	return(x * 32);
}

int tileX(int x, int y){
	if (x == 2) {
		return(0);
	}
	if (x == 3) {
		return(0);
	}
	if (x == 4) {
		return(0);
	}
	if (x == 5) {
		return(1);
	}
	if (x == 6) {
		return(1);
	}
	if (x == 7) {
		return(3);
	}
	if (x == 8) {
		return(0);
	}
	if (x == 9) {
		return(1);
	}
	if (x == 10) {
		return(1);
	}
	if (x == 11) {
		return(3);
	}
	if (x == 12) {
		return(2);
	}
	if (x == 13) {
		return(2);
	}
	if (x == 14) {
		return(2);
	}
	if (x == 15) {
		return(3);
	}
	if (x == 16) {
		y = 2;
		return(5);
	}
	if (x == 17) {
		y = 2;
		return(6);
	}
}

int tileY(int x, int y){
	if (x <= 2 || x > 17) {
		y = 2;
		return(0);
	}
	if (x == 3) {
		return(1);
	}
	if (x == 4) {
		return(3);
	}
	if (x == 5) {
		return(3);
	}
	if (x == 6) {
		return(1);
	}
	if (x == 7) {
		return(1);
	}
	if (x == 8) {
		return(2);
	}
	if (x == 9) {
		return(2);
	}
	if (x == 10) {
		return(0);
	}
	if (x == 11) {
		return(2);
	}
	if (x == 12) {
		return(2);
	}
	if (x == 13) {
		return(1);
	}
	if (x == 14) {
		return(3);
	}
	if (x == 15) {
		return(3);
	}
	if (x == 16) {
		y = 2;
		return(1);
	}
	if (x == 17) {
		y = 2;
		return(1);
	}
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
	int index = 0;
	int indexOrig = 0;
};

auto main() -> int {
	
	//Get from map data
	int lvlData[600];
	std::copy(mapData,mapData+600,lvlData);
	for (int i = 0; i < cloneCountX * cloneCountY; i++) {
		if(lvlData[i] < 2){
			lvlData[i]=2;
		}
	}
	
	Tile lvl[600];
	// build lvl
	for (int i = 0; i < cloneCountX * cloneCountY; i++) {
		lvl[i].en=true;
		lvl[i].tX = tilePos(cloneNumberX * -1);
		lvl[i].tY = tilePos(i - cloneNumberX * cloneCountX);
		lvl[i].index=i;
		lvl[i].indexOrig=lvl[i].index;
		lvl[i].scX = 32;
		lvl[i].scY = 32;
		lvl[i].type=1;
		if (lvlData[lvl[i].index] >= 16) {
			lvl[i].sheet = 2;
			lvl[i].type = 2;
		}else{
			lvl[i].sheet = 1;
			lvl[i].type = 1;
		}
		lvl[i].sX = tilePos(tileX(lvlData[lvl[i].index],lvl[i].type));
		lvl[i].sY = tilePos(tileY(lvlData[lvl[i].index],lvl[i].type));
			
		cloneNumberX = round(i/cloneCountX);
	}

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
			if (ctrlData.Buttons & PSP_CTRL_UP) {
				teeBodyOff = rand() % 2 - 1;
				teeFootMove+=15;
				dir = -1;
				y+=5;
			}
			if (ctrlData.Buttons & PSP_CTRL_DOWN) {
				teeBodyOff = rand() % 2 - 1;
				teeFootMove+=15;
				dir = 1;
				y-=5;
			}
			if (!(ctrlData.Buttons & PSP_CTRL_RIGHT) && !(ctrlData.Buttons & PSP_CTRL_LEFT)) {
				teeFootMove=0;
				teeBodyOff=0;
			}
			
			camX = x + 219;
			camY = y + 121;
			
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
			
			
			
			gravity+=1;
			y-=gravity;
			
			// level collisions
			for (int i = 0; i < cloneCountX * cloneCountY; i++) {
				// collision
				if(lvlData[lvl[i].index] > 2){
					if(collision((camX-x),(camY-y),(camX-lvl[i].tX)+219,(camY-lvl[i].tY+121),43,38,32,32,false,lvlData[lvl[i].index])){
						if(lvl[i].en == true){
							if(lvl[i].type == 2){
								if (lvlData[lvl[i].index] == 16){
									if (lvl[i].hidden == false) {
										if (health<9){
											lvl[i].hidden=true;
											health+=1;
										}
									}
								}
								if (lvlData[lvl[i].index] == 17){
									if (lvl[i].hidden == false) {
										if (armor<9){
											lvl[i].hidden=true;
											armor+=1;
										}
									}
								}
							}
							if(lvl[i].type == 1){
									jump_disable=false;
									if(gravity < -1){
										gravity = -5;
									}else{
										gravity = 1;
										doublejump=0;
										// controls part 2 :D
										if (ctrlData.Buttons & PSP_CTRL_CROSS) {
											doublejump+=1;
											gravity=-15;
											y-=gravity;
										}
									}
									y = nY;
										
									if(collision((camX-x),(camY-y),(camX-lvl[i].tX)+219,(camY-lvl[i].tY)+121,43,38,32,32,false,lvl[i].index)){
										x=nX;
									}
									

									
									teeFeetAllRot = 0;
							}
						}
					}
				}
			}
			// level showing
			for (int i = 0; i < cloneCountX * cloneCountY; i++) {
				
				if (lvl[i].hidden == false) {
					lvl[i].sX=tilePos(tileX(lvlData[lvl[i].index],lvl[i].type));
					lvl[i].sY=tilePos(tileY(lvlData[lvl[i].index],lvl[i].type));
					
					if (lvl[i].sheet == 2) {
						g2dBeginRects(game); 
					}else{
						g2dBeginRects(grass); 
					}
					g2dSetScaleWH(lvl[i].scX,lvl[i].scY);
					g2dSetCoordXY((camX-lvl[i].tX)+219,(camY-lvl[i].tY)+121); 
					g2dSetCropXY(lvl[i].sX,lvl[i].sY); 
					g2dSetCropWH(lvl[i].scX,lvl[i].scY);
					g2dSetCropWH(32,32);
					g2dAdd();
					g2dEnd(); 
				}else{
					lvl[i].timer +=1;
					if(lvl[i].timer >= 200){
						lvl[i].timer = 0;
						lvl[i].hidden = false;
					}
				}
			}
			
			nX = x;
			nY = y;
			
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
			
			sceCtrlReadLatch(&latchData);
			if (latchData.uiBreak & PSP_CTRL_TRIANGLE){
				teeWep+=1;
				if (teeWep>5) {
					teeWep=0;
				}
			}
			
			if(y<-256){
				health = 0;
				armor = 0;
			}
			
			if (health<1){
				tick+=1;
				if(tick>99){
					// reset tee vars
					x = 512;
					y = 0;
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