#include "myLib.h"
#include "PALETTE.h"
#include "StartText.h"
#include "GIBS.h"
#include "MODE.h"
#include "Sin_Lut.h"
#include "TileMaps.h"
#include "GAMETILES.h"
#include "GAMESPRITES.h"
#include <debugging.h>
#include <stdlib.h>

typedef struct particle{
	int x;
	int y;
	int velX;
	int velY;
	int accelY;
	int accelX;
}Particle;

typedef struct Bot{
	int x;
	int y;
	int velX;
	int velY;
	int accelY;
	int accelX;
	int angle;
	int health;
	int shotTimer;
	int basicTimer;
}Bot;

typedef struct Spawner{
	int x;
	int y;
	int health;
}Spawner;

int lu_sin(u16 theta){
	return (short)sin_lut[(theta>>7)&0x1FF];
}

int lu_cos(u16 theta){   
	return (short)sin_lut[((theta>>7)+128)&0x1FF];     
}

/*
int atan2(int x, int y){
	//X and Y are in 16.16 format
	//Pi in 3.29 fixed point.
	int c1 = 3141592653>>2; //Math.PI / 4;
	int c2 = c1+c1+c1; //3*c1;
	int ay = y<0? -y : y;
	int angle;
	
	//c1 and c2 are in 3.29 format.
	if(x >= 0){
		int r = 0; // (x - abs_y) / (x + abs_y)
		angle = c2 - (c1*r);
	}else{
		int r = 0; // (x + abs_y) / (abs_y - x)
		angle = c2 - (c1*r);
	}
	return y < 0 ? -angle : angle;
}*/

int main(){
	REG_DISPCNT = MODE0 | BG0_ENABLE | OBJ_ENABLE | SPR_MAP_1D;
	dma_cpy(background_palette, PALETTE_palette, 5, 3);
	background_palette[0] = PALETTE_palette[1];
	dma_cpy(CHAR_BLOCK0+16, StartText, 192, 3);
	dma_cpy((u16*)0x6000800, titleTileMap, 1024, 3);
	dma_cpy((u16*)0x6000800+0x400, title2TileMap, 1024, 3);
	dma_cpy((u16*)0x6000800+0x400+0x400, title3TileMap, 1024, 3);
	dma_cpy(sprite_palette, PALETTE_palette, 5, 3);
	REG_BG0_CNT = SBB(1) | CBB(0);
	
	int fading = 0;
	int fadeTimer = 0;

	//These values are in 24.8 fixed point format.
	int current_r_values[5] = {(PALETTE_palette[4]&0x1f)<<8, (PALETTE_palette[4]&0x1f)<<8, (PALETTE_palette[4]&0x1f)<<8, (PALETTE_palette[4]&0x1f)<<8, (PALETTE_palette[4]&0x1f)<<8};
	int current_g_values[5] = {((PALETTE_palette[4]>>5)&0x1f)<<8, ((PALETTE_palette[4]>>5)&0x1f)<<8, ((PALETTE_palette[4]>>5)&0x1f)<<8, ((PALETTE_palette[4]>>5)&0x1f)<<8, ((PALETTE_palette[4]>>5)&0x1f)<<8};
	int current_b_values[5] = {((PALETTE_palette[4]>>10)&0x1f)<<8, ((PALETTE_palette[4]>>10)&0x1f)<<8, ((PALETTE_palette[4]>>10)&0x1f)<<8, ((PALETTE_palette[4]>>10)&0x1f)<<8, ((PALETTE_palette[4]>>10)&0x1f)<<8};

	int step_r[5] = {0,0,0,0,0};
	int step_g[5] = {0,0,0,0,0};
	int step_b[5] = {0,0,0,0,0};

	int target_r[5] = {0,0,0,0,0};
	int target_g[5] = {0,0,0,0,0};
	int target_b[5] = {0,0,0,0,0};

	int numSteps = 200;
	for(int i = 0; i < 5; i++){
		target_r[i] = (((PALETTE_palette[i]>>0)&0x1f)<<8);
		target_g[i] = (((PALETTE_palette[i]>>5)&0x1f)<<8);
		target_b[i] = (((PALETTE_palette[i]>>10)&0x1f)<<8);
		step_r[i] = ((((PALETTE_palette[i]>>0)&0x1f)<<8) - current_r_values[i])/(numSteps);
		step_g[i] = ((((PALETTE_palette[i]>>5)&0x1f)<<8) - current_g_values[i])/(numSteps);
		step_b[i] = ((((PALETTE_palette[i]>>10)&0x1f)<<8) - current_b_values[i])/(numSteps);
	}
	fading = 1;
	fadeTimer = 0;
	
	int state = 0;	
	int subState = 0;

	REG_BG0_HOFF = -4;
	REG_BG0_VOFF = -4;

	int count = 0;

	while(state == 0){
		//Intro sequence
		//Start high color, fade to PRESS START
		//Slowly fade the colors to and from the base color

		//The initial fade-in is done, now tally up 60 frames and then start fading the text.
		if(fading == 0 && subState == 0){
			fadeTimer++;
			if(fadeTimer >= 60){
				subState = 1;
			}
		}

		if(fading == 0 && subState == 3){
			subState = 2;
		}

		if(subState == 1){
			numSteps = 30;
			for(int i = 0; i < 5; i++){
				target_r[i] = (((PALETTE_palette[1]>>0)&0x1f)<<8);
				target_g[i] = (((PALETTE_palette[1]>>5)&0x1f)<<8);
				target_b[i] = (((PALETTE_palette[1]>>10)&0x1f)<<8);
				step_r[i] = ((((PALETTE_palette[1]>>0)&0x1f)<<8) - current_r_values[i])/(numSteps);
				step_g[i] = ((((PALETTE_palette[1]>>5)&0x1f)<<8) - current_g_values[i])/(numSteps);
				step_b[i] = ((((PALETTE_palette[1]>>10)&0x1f)<<8) - current_b_values[i])/(numSteps);
			}
			fading = 1;
			fadeTimer = 0;
			subState = 3;
		}
		if(subState == 2){
			numSteps = 30;
			for(int i = 0; i < 5; i++){
				target_r[i] = (((PALETTE_palette[i]>>0)&0x1f)<<8);
				target_g[i] = (((PALETTE_palette[i]>>5)&0x1f)<<8);
				target_b[i] = (((PALETTE_palette[i]>>10)&0x1f)<<8);
				step_r[i] = ((((PALETTE_palette[i]>>0)&0x1f)<<8) - current_r_values[i])/(numSteps);
				step_g[i] = ((((PALETTE_palette[i]>>5)&0x1f)<<8) - current_g_values[i])/(numSteps);
				step_b[i] = ((((PALETTE_palette[i]>>10)&0x1f)<<8) - current_b_values[i])/(numSteps);
			}
			fading = 1;
			fadeTimer = 0;
			subState = 0;
		}
		vid_vsync();
		count++;
		if(fading){
			for(int i = 0; i < 5; i++){
				current_r_values[i] += step_r[i];
				current_g_values[i] += step_g[i];
				current_b_values[i] += step_b[i];
			}
			fadeTimer++;
			if(fadeTimer == numSteps){
				fading = 0;
				fadeTimer = 0;
				for(int i = 0; i < 5; i++){
					current_r_values[i] = target_r[i];
					current_g_values[i] = target_g[i];
					current_b_values[i] = target_b[i];
				}
			}
		}
		for(int i = 0; i < 5; i++){
			background_palette[i] = RGB(current_r_values[i]>>8, current_g_values[i]>>8, current_b_values[i]>>8);
		}
		if((~REG_BUTTONS) & BTN_STRT){
			state = 1;
		}
	}
	srand(count);
	subState = 0;
	fading = 0;
	
	//Fix the palettes
	dma_cpy(background_palette, PALETTE_palette, 5, 3);
	dma_cpy(sprite_palette, PALETTE_palette, 5, 3);
	REG_BG0_CNT = SBB(6) | CBB(0);

	//Set up the rotation matricies
	int alpha1 = 0;
	int alpha2 = 0;
	int alpha3 = 0;
	int alpha4 = 0;
	obj_attr_mem[3] = lu_cos(0)>>4;
	obj_attr_mem[7] = -(lu_sin(0)>>4);
	obj_attr_mem[11] = lu_sin(0)>>4;
	obj_attr_mem[15] = lu_cos(0)>>4;

	obj_attr_mem[19] = lu_cos(0)>>4;
	obj_attr_mem[23] = -(lu_sin(0)>>4);
	obj_attr_mem[27] = lu_sin(0)>>4;
	obj_attr_mem[31] = lu_cos(0)>>4;

	obj_attr_mem[35] = lu_cos(0)>>4;
	obj_attr_mem[39] = -(lu_sin(0)>>4);
	obj_attr_mem[43] = lu_sin(0)>>4;
	obj_attr_mem[47] = lu_cos(0)>>4;

	obj_attr_mem[51] = lu_cos(0)>>4;
	obj_attr_mem[55] = -(lu_sin(0)>>4);
	obj_attr_mem[59] = lu_sin(0)>>4;
	obj_attr_mem[63] = lu_cos(0)>>4;

	obj_attr_mem[67] = lu_cos(0)>>4;
	obj_attr_mem[71] = -(lu_sin(0)>>4);
	obj_attr_mem[75] = lu_sin(0)>>4;
	obj_attr_mem[79] = lu_cos(0)>>4;

	obj_attr_mem[83] = lu_cos(0)>>4;
	obj_attr_mem[87] = -(lu_sin(0)>>4);
	obj_attr_mem[91] = lu_sin(0)>>4;
	obj_attr_mem[95] = lu_cos(0)>>4;
	//Set up the images
	dma_cpy(OVRAM_BNK1, GIBS, 160, 3);
	dma_cpy(OVRAM_BNK1+160, MODE, 512, 3);
	dma_cpy(OVRAM_BNK1+160+512, GAMESPRITES, 4416, 3);
	
	for(int i = 0; i < 128; i++){
		obj_attr_mem[(i)*4] = SPRITE_RENDER_HIDDEN;
	}

	int MOx = 240;
	int DEx = -64;
	
	obj_attr_mem[0] = 40 | SPRITE_RENDER_DUBAFFINE | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_WIDE;
	obj_attr_mem[1] = 240 | SPRITE_RENDER_PALETTE0 | SPRITE_SIZE_MEDIUM | (4<<9);
	obj_attr_mem[2] = 10;
	obj_attr_mem[4] = 40 | SPRITE_RENDER_DUBAFFINE | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_WIDE;
	obj_attr_mem[5] = (-64 & 0x1FF) | SPRITE_RENDER_PALETTE0 | SPRITE_SIZE_MEDIUM | (5<<9);
	obj_attr_mem[6] = 26;

	Particle particles[120];

	for(int i = 0; i < 120; i++){
		obj_attr_mem[(i+2)*4+0] = 0 | SPRITE_RENDER_HIDDEN | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
		obj_attr_mem[(i+2)*4+1] = 0 | SPRITE_RENDER_PALETTE0 | SPRITE_SIZE_SMALL | (i/30)<<9;
		obj_attr_mem[(i+2)*4+2] = (i/24)*2;
	}
	
	while(state == 1){
		if(subState == 0){
			MOx-=3;
			DEx+=3;
			if(MOx < 70){
				subState = 1;
				int nAlpha1 = ((rand()%40)-20)*300;
				int nAlpha2 = ((rand()%40)-20)*300;
				obj_attr_mem[67] = lu_cos(nAlpha1)>>4;
				obj_attr_mem[71] = -(lu_sin(nAlpha1)>>4);
				obj_attr_mem[75] = lu_sin(nAlpha1)>>4;
				obj_attr_mem[79] = lu_cos(nAlpha1)>>4;

				obj_attr_mem[83] = lu_cos(nAlpha2)>>4;
				obj_attr_mem[87] = -(lu_sin(nAlpha2)>>4);
				obj_attr_mem[91] = lu_sin(nAlpha2)>>4;
				obj_attr_mem[95] = lu_cos(nAlpha2)>>4;
			}			
			obj_attr_mem[1] = (MOx & 0x1FF) | SPRITE_RENDER_PALETTE0 | SPRITE_SIZE_MEDIUM | (4<<9);
			obj_attr_mem[5] = (DEx & 0x1FF) | SPRITE_RENDER_PALETTE0 | SPRITE_SIZE_MEDIUM | (5<<9);
		}

		if(subState == 2){
			int x;
			int y;
			for(int i = 0; i < 120; i++){
				x = ((rand()%64)+80)<<16;
				y = ((rand()%32)+40)<<16;
				obj_attr_mem[(i+2)*4+0] = y>>16 | SPRITE_RENDER_AFFINE | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
				obj_attr_mem[(i+2)*4+1] = (obj_attr_mem[(i+2)*4+1]&0xFE00) | x>>16;
				particles[i].x = x;
				particles[i].y = y;
				particles[i].velX = ((rand()%80)-40)<<12;
				particles[i].velY = ((rand()%80)-100)<<12;
				particles[i].accelY = 25<<8;
			}

			//Position the '0'
			obj_attr_mem[488] = 65 | SPRITE_RENDER_NORMAL | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_WIDE;
			obj_attr_mem[489] = (240/2-8) | SPRITE_SIZE_SMALL;
			obj_attr_mem[490] = 14;
			obj_attr_mem[492] = (65+8) | SPRITE_RENDER_NORMAL | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_WIDE;
			obj_attr_mem[493] = (240/2-8) | SPRITE_SIZE_SMALL;
			obj_attr_mem[494] = 22;
			//And the Press A+B
			REG_BG0_CNT = SBB(2) | CBB(0);
			REG_BG0_HOFF = -12;
			REG_BG0_VOFF = -50;
			subState = 3;
		}

		if(subState == 3){
			for(int i = 0; i < 120; i++){
				particles[i].x += particles[i].velX;
				particles[i].y += particles[i].velY;
				particles[i].velY += particles[i].accelY;
				//is OnScreen?
				if(particles[i].x > (-8)<<16 && particles[i].x < 248<<16 && particles[i].y > (-8)<<16 && particles[i].y < 168<<16){
					obj_attr_mem[(i+2)*4+0] = ((particles[i].y>>16)&0xFF) | SPRITE_RENDER_AFFINE | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
					obj_attr_mem[(i+2)*4+1] = (obj_attr_mem[(i+2)*4+1]&0xFE00) | ((particles[i].x>>16)&0x1FF);
				}else{
					obj_attr_mem[(i+2)*4+0] = SPRITE_RENDER_HIDDEN;
				}
			}
		}
		
		if(subState == 1){
			numSteps = 30;
			for(int i = 0; i < 5; i++){
				current_r_values[i] = ((PALETTE_palette[4]>>0)&0x1f)<<8;
				current_g_values[i] = ((PALETTE_palette[4]>>5)&0x1f)<<8;
				current_b_values[i] = ((PALETTE_palette[4]>>10)&0x1f)<<8;
				target_r[i] = (((PALETTE_palette[i]>>0)&0x1f)<<8);
				target_g[i] = (((PALETTE_palette[i]>>5)&0x1f)<<8);
				target_b[i] = (((PALETTE_palette[i]>>10)&0x1f)<<8);
				step_r[i] = ((((PALETTE_palette[i]>>0)&0x1f)<<8) - current_r_values[i])/(numSteps);
				step_g[i] = ((((PALETTE_palette[i]>>5)&0x1f)<<8) - current_g_values[i])/(numSteps);
				step_b[i] = ((((PALETTE_palette[i]>>10)&0x1f)<<8) - current_b_values[i])/(numSteps);
			}
			fading = 1;
			fadeTimer = 0;
			subState = 2;
		}

		if(fading){
			for(int i = 0; i < 5; i++){
				current_r_values[i] += step_r[i];
				current_g_values[i] += step_g[i];
				current_b_values[i] += step_b[i];
			}
			fadeTimer++;
			if(fadeTimer == numSteps){
				fading = 0;
				fadeTimer = 0;
				for(int i = 0; i < 5; i++){
					current_r_values[i] = target_r[i];
					current_g_values[i] = target_g[i];
					current_b_values[i] = target_b[i];
				}
			}
			for(int i = 0; i < 5; i++){
				background_palette[i] = RGB(current_r_values[i]>>8, current_g_values[i]>>8, current_b_values[i]>>8);
			}
		}
		if(~REG_BUTTONS & BTN_A && ~REG_BUTTONS & BTN_B){
			state = 2;
		}
		alpha1 += 1;
		alpha2 += 2;
		alpha3 -= 1;
		alpha4 -= 2;
		vid_vsync();
		obj_attr_mem[3] = lu_cos(200*alpha1)>>4;
		obj_attr_mem[7] = -(lu_sin(200*alpha1)>>4);
		obj_attr_mem[11] = lu_sin(200*alpha1)>>4;
		obj_attr_mem[15] = lu_cos(200*alpha1)>>4;

		obj_attr_mem[19] = lu_cos(200*alpha2)>>4;
		obj_attr_mem[23] = -(lu_sin(200*alpha2)>>4);
		obj_attr_mem[27] = lu_sin(200*alpha2)>>4;
		obj_attr_mem[31] = lu_cos(200*alpha2)>>4;

		obj_attr_mem[35] = lu_cos(200*alpha3)>>4;
		obj_attr_mem[39] = -(lu_sin(200*alpha3)>>4);
		obj_attr_mem[43] = lu_sin(200*alpha3)>>4;
		obj_attr_mem[47] = lu_cos(200*alpha3)>>4;

		obj_attr_mem[51] = lu_cos(200*alpha4)>>4;
		obj_attr_mem[55] = -(lu_sin(200*alpha4)>>4);
		obj_attr_mem[59] = lu_sin(200*alpha4)>>4;
		obj_attr_mem[63] = lu_cos(200*alpha4)>>4;
	}
	
	for(int i = 0; i < 128; i++){
		obj_attr_mem[(i)*4] = SPRITE_RENDER_HIDDEN;
	}
	//Set up the game
	//The tileblocks are going to be large
	//0 1
	//2 3 blah
	//Each tilemap block is a 2x2 of rooms.
	//Generate those here
	//Start with the walls and cieling.
	//Build Screenblock 0-3
	int spawnerNumber = 0;
	Spawner spawners[6];
	for(int i = 0; i < 4; i++){
		int maxW = 9;
		int maxH = 7;
		int minW = 2;
		int minH = 1;
		//Buildrooms in each screenblock
		for(int j = 0; j < 4; j++){
			int spawner = 0;
			//Does this room have a spawner in it?
			int sx = 0;
			int sy = 0;
			if( (i==0 && j==0) || (i==0 && j==2) || (i==1 && j==1) || (i==1 && j==3) || (i==2 && j==2) || (i==3 && j==3) ){
				sx = 2+rand()%10;
				sy = 2+rand()%10;
				spawner = 1;
			}
			int numBlocks = (rand()%4)+3;
			int bw = 0;
			int bh = 0;
			int bx = 0;
			int by = 0;
			for(int k = 0; k < numBlocks; k++){
				int check = 0;
				do{
					bw = minW + rand()%(maxW-minW);
					bh = minH + rand()%(maxH-minH);
					bx = (rand()%(17-bw))-1;
					by = (rand()%(17-bh))-1;
					if(spawner){
						if(sx>bx+bw || sx+3<bx || sy>by+bh || sy+3<by){
							check = 1;
						}
					}else{
						check = 1;
					}
				}while(!check);
				//Block is good, construct it.
				//Double for loop for rows/cols
				for(int q = 0; q < bh; q++){
					for(int w = 0; w < bw; w++){
						SCREENBLOCK[i+4].SCREEN_ENTRY[(32*((by+q)+((j/2)*16)))+((bx+w) + ((j%2)*16))] = (rand()%6)+1;
					}
				}
				//If it's got space, fill it with dirt
				if(bw >=4 && bh >= 4){
					for(int q = 0; q < bh-2; q++){
						for(int w = 1; w < bw-1; w++){
							if(q==0) SCREENBLOCK[i+4].SCREEN_ENTRY[(32*((by+q)+((j/2)*16)))+((bx+w) + ((j%2)*16))] = (rand()%2)+7;
							else SCREENBLOCK[i+4].SCREEN_ENTRY[(32*((by+q)+((j/2)*16)))+((bx+w) + ((j%2)*16))] = (rand()%12)+9;
						}
					}
				}
			}
			if(spawner){
				spawners[spawnerNumber].x = ((sx*8)+((i%2)*256)+((j%2)*128))<<16;
				spawners[spawnerNumber].y = ((sy*8)+((i/2)*256)+((j/2)*128))<<16;
				spawners[spawnerNumber].health = 8;
				spawnerNumber++;
			}
		}
		
		//Put the edges of this screenblock in.
		for(int a = 0; a < 32; a++){
			for(int s = 0; s < 32; s++){
				if(i == 0){
					if(a == 0 || s == 0) SCREENBLOCK[i+4].SCREEN_ENTRY[a*32 + s] = (rand()%6)+1;
				}else if(i==1){
					if(a == 0 || s == 31) SCREENBLOCK[i+4].SCREEN_ENTRY[a*32 + s] = (rand()%6)+1;
				}else if(i==2){
					if(a == 30) SCREENBLOCK[i+4].SCREEN_ENTRY[a*32 + s] = (rand()%2)+7;
					if(a == 31 && s!=0) SCREENBLOCK[i+4].SCREEN_ENTRY[a*32 + s] = (rand()%15)+9;
					if(s == 0) SCREENBLOCK[i+4].SCREEN_ENTRY[a*32 + s] = (rand()%6)+1;
				}else if(i==3){
					if(a == 30) SCREENBLOCK[i+4].SCREEN_ENTRY[a*32 + s] = (rand()%2)+7;
					if(a == 31 && s!=31) SCREENBLOCK[i+4].SCREEN_ENTRY[a*32 + s] = (rand()%15)+9;
					if(s == 31) SCREENBLOCK[i+4].SCREEN_ENTRY[a*32 + s] = (rand()%6)+1;
				}
			}
		}
	}
	
	dma_cpy(CHAR_BLOCK1+16, GAMETILES, 384, 3);
	
	REG_BG0_CNT = SBB(4) | CBB(1) | (3<<0xE);
	int scrollX = 0;
	int scrollY = 0;
	int frameCount = 0;
	int animation[24] = {0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0};
	int poofAnim[4] = {0, 1, 2, 3};
	
	int generalAnimationCounter = 0;
	int walk[4] = {1, 2, 3, 0};
	
	int playerBulletAnimCounters[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	Particle player = {300<<16, 300<<16, 0, 0, 25<<8};
	Particle playerBullets[10];
	Bot bots[16];
	obj_attr_mem[0] = ((player.x>>16)&0xFF) | SPRITE_RENDER_NORMAL | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
	obj_attr_mem[1] = ((player.y>>16)&0x1FF) | SPRITE_SIZE_SMALL;
	obj_attr_mem[2] = 72;
	
	//Set up the 16 bot affine matricies and set the sprites to invisible
	int matrixID = 67;
	for(int i = 0; i < 16; i++){
		obj_attr_mem[4*(i+11)] = SPRITE_RENDER_HIDDEN;
		
		obj_attr_mem[matrixID] = 1<<8;
		matrixID+=4;
		obj_attr_mem[matrixID] = 0;
		matrixID+=4;
		obj_attr_mem[matrixID] = 0;
		matrixID+=4;
		obj_attr_mem[matrixID] = 1<<8;
		matrixID+=4;
	}
	short prevREG_BUTTONS = 0;
	int playerBulletIndex = 0;
	int numActiveBots = 0;
	int numActiveSpawners = 6;
	
	//get the 60 giblets ready
	for(int i = 0; i < 60; i++){
		obj_attr_mem[(i+28)*4+0] = 0 | SPRITE_RENDER_HIDDEN | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
		obj_attr_mem[(i+28)*4+1] = 0 | SPRITE_SIZE_SMALL | (i/20)<<9;
		obj_attr_mem[(i+28)*4+2] = (rand()%5)*2;
	}
	int gibletPosition = 0;
	
	dma_cpy(background_palette, PALETTE_palette, 5, 3);
	dma_cpy(sprite_palette, PALETTE_palette, 5, 3);
	
	while(state == 2){
		if(~REG_BUTTONS & BTN_LEFT){
			player.velX = (-3)<<15;
			obj_attr_mem[1] = obj_attr_mem[1] | 1<<0xC;
		}else if(~REG_BUTTONS & BTN_RGHT){
			player.velX = 3<<15;
			obj_attr_mem[1] = obj_attr_mem[1] & 0xEFFF;
		}else{
			player.velX = 0;
		}
		if(~REG_BUTTONS & BTN_UP){
			//player.velY = (-3)<<15;
		}else if(~REG_BUTTONS & BTN_DOWN && (~REG_BUTTONS & BTN_B && !(~prevREG_BUTTONS & BTN_B))){
			player.velY -= 6<<12;
		}else{
			//player.velY = 0;
		}
		
		if(~REG_BUTTONS & BTN_B && !(~prevREG_BUTTONS & BTN_B)){
			//Fire a bullet
			playerBullets[playerBulletIndex].x = player.x;
			playerBullets[playerBulletIndex].y = player.y;
			playerBullets[playerBulletIndex].velY = (~REG_BUTTONS & BTN_UP) ? (-(75<<12)) : ((~REG_BUTTONS & BTN_DOWN)? (75<<12) : 0);
			if(playerBullets[playerBulletIndex].velY == 0)
				playerBullets[playerBulletIndex].velX = (obj_attr_mem[1] & 0x1000) > 0 ? (-(75<<12)) : (75<<12);
			else
				playerBullets[playerBulletIndex].velX = 0;
			playerBullets[playerBulletIndex].accelY = 1;
			playerBulletAnimCounters[playerBulletIndex] = 0;
			playerBulletIndex++;
			if(playerBulletIndex > 9){
				playerBulletIndex = 0;
			}
		}
		
		player.velY += player.accelY;
		if(player.velY > 100<<12)
			player.velY = 100<<12;
		player.x += player.velX;
		player.y += player.velY;
		
		for(int i = 9; i >= 0; i--){
			playerBullets[i].x += playerBullets[i].velX;
			playerBullets[i].y += playerBullets[i].velY;
		}
		
		//Bot AI goes here
		for(int i = 0; i < 16; i++){
			if(player.x < bots[i].x) bots[i].accelX = -(25<<8);
			if(player.x > bots[i].x) bots[i].accelX = 25<<8;
			if(player.y < bots[i].y) bots[i].accelY = -(25<<8);
			if(player.y > bots[i].y) bots[i].accelY = 25<<8;
			
			bots[i].x += bots[i].velX;
			bots[i].y += bots[i].velY;
			bots[i].velX += bots[i].accelX;
			if(bots[i].velX > 30<<12){
				bots[i].velX = 30<<12;
			}else if(bots[i].velX < -(30<<12)){
				bots[i].velX = -(30<<12);
			}
			bots[i].velY += bots[i].accelY;
			if(bots[i].velY > 30<<12){
				bots[i].velY = 30<<12;
			}else if(bots[i].velY < -(30<<12)){
				bots[i].velY = -(30<<12);
			}
			
			if(bots[i].velX > (30<<10) && bots[i].velY > (30<<10)){
				bots[i].angle = 225;
			}else if(bots[i].velX < -(30<<10) && bots[i].velY > (30<<10)){
				bots[i].angle = 135;
			}else if(bots[i].velX < -(30<<10) && bots[i].velY < -(30<<10)){
				bots[i].angle = 45;
			}else if(bots[i].velX > (30<<10) && bots[i].velY < -(30<<10)){
				bots[i].angle = 315;
			}/*else if(bots[i].velX < (30<<10) && bots[i].velX > -(30<<10) && bots[i].velY > 0){
				bots[i].angle = 180;
			}else if(bots[i].velX < (30<<10) && bots[i].velX > -(30<<10) && bots[i].velY < 0){
				bots[i].angle = 0;
			}else if(bots[i].velY < (30<<10) && bots[i].velY > -(30<<10) && bots[i].velX > 0){
				bots[i].angle = 270;
			}else if(bots[i].velY < (30<<10) && bots[i].velY > -(30<<10) && bots[i].velX < 0){
				bots[i].angle = 90;
			}*/
		}
		
		for(int i = 0; i < 60; i++){
			particles[i].x += particles[i].velX;
			particles[i].y += particles[i].velY;
			particles[i].velY += particles[i].accelY;
			if(particles[i].velX == 0 || particles[i].velX == -1 ){
				particles[i].accelX = 0;
			}
		}
		
		//////////////////////////////
		//Collision with the tilemap//
		//////////////////////////////
		//Check all four points?
		int quad = (player.x>>16 > 255 ? 1 : 0) | (player.y>>16 > 255 ? 2 : 0);
		int p1 = SCREENBLOCK[quad+4].SCREEN_ENTRY[(((player.x>>16)-(256*(quad&0x1)))>>3)+((((player.y>>16)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
		quad = ((player.x>>16)+8 > 255 ? 1 : 0) | ((player.y>>16) > 255 ? 2 : 0);
		int p2 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((player.x>>16)+8)-(256*(quad&0x1)))>>3)+((((player.y>>16)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
		quad = ((player.x>>16) > 255 ? 1 : 0) | ((player.y>>16)+8 > 255 ? 2 : 0);
		int p3 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((player.x>>16))-(256*(quad&0x1)))>>3)+(((((player.y>>16)+8)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
		quad = ((player.x>>16)+8 > 255 ? 1 : 0) | ((player.y>>16)+8 > 255 ? 2 : 0);
		int p4 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((player.x>>16)+8)-(256*(quad&0x1)))>>3)+(((((player.y>>16)+8)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
		
		//if the bottom two are touching then separate Y
		if(p3 && p4){
			player.y = player.y & (~(0x7<<16));
			player.velY = 0;
		}else if(p1 && p2){
			player.y = (player.y & (~(0x7<<16))) + (8<<16);
			player.velY = 0;
		}
		if(p1 && p3){
			player.x = (player.x & (~(0x7<<16))) + (8<<16);
		}else if(p2 && p4){
			player.x = player.x & (~(0x7<<16));
		}
		//Hard edges taken care of.
		//Now for solitary points.
		int penX = 0;
		int penY = 0;
		if(p1 && !(p3 || p2 || p4)){
			penX = 8 - ((player.x>>16) & 0x7);
			penY = 8 - ((player.y>>16) & 0x7);
			if(penY > penX){
				player.x = (player.x & (~(0x7<<16))) + (8<<16);
			}else{
				player.y = (player.y & (~(0x7<<16))) + (8<<16);
				if(player.velY < 0) player.velY = 0;
			}
		}
		if(p2 && !(p3 || p1 || p4)){
			penX = ((player.x>>16) & 0x7);
			penY = 8 - ((player.y>>16) & 0x7);
			if(penY > penX){
				player.x = (player.x & (~(0x7<<16)));
			}else{
				player.y = (player.y & (~(0x7<<16))) + (8<<16);
				if(player.velY < 0) player.velY = 0;
			}
		}
		if(p3 && !(p2 || p1 || p4)){
			penX = 8 - ((player.x>>16) & 0x7);
			penY = ((player.y>>16) & 0x7);
			if(penY > penX){
				player.x = (player.x & (~(0x7<<16))) + (8<<16);
			}else{
				player.y = (player.y & (~(0x7<<16)));
				if(player.velY > 0) player.velY = 0;
			}
		}
		if(p4 && !(p2 || p1 || p3)){
			penX = ((player.x>>16) & 0x7);
			penY = ((player.y>>16) & 0x7);
			if(penY > penX){
				player.x = (player.x & (~(0x7<<16)));
			}else{
				player.y = (player.y & (~(0x7<<16)));
				if(player.velY > 0) player.velY = 0;
			}
		}
		//SUPER EDGE CASE: Opposite points.
		if(p2 && p3 && !p1 && !p4){
			//This could either be coming from underneath it, or from above it
			if(((player.y>>16)&0x7) < 5){
				player.x = (player.x & (~(0x7<<16)));
				player.y = (player.y & (~(0x7<<16)));
				if(player.velY > 0) player.velY = 0;
			}else{
				player.x = (player.x & (~(0x7<<16))) + (8<<16);
				player.y = (player.y & (~(0x7<<16))) + (8<<16);
				if(player.velY < 0) player.velY = 0;
			}
		}
		if(p1 && p4 && !p2 && !p3){
			if(((player.y>>16)&0x7) < 5){
				player.x = (player.x & (~(0x7<<16))) + (8<<16);
				player.y = (player.y & (~(0x7<<16)));
				if(player.velY > 0) player.velY = 0;
			}else{
				player.x = (player.x & (~(0x7<<16)));
				player.y = (player.y & (~(0x7<<16))) + (8<<16);
				if(player.velY < 0) player.velY = 0;
			}
		}
		
		//The player needs these points for jumping.
		if(~REG_BUTTONS & BTN_A && ((p3 || p4) && !(p1 && p2) && player.velY == 0)){
			player.velY = -40<<12;
		}
		
		//bot collisions
		for(int i = 0; i < 16; i++){
			quad = (bots[i].x>>16 > 255 ? 1 : 0) | (bots[i].y>>16 > 255 ? 2 : 0);
			p1 = SCREENBLOCK[quad+4].SCREEN_ENTRY[(((bots[i].x>>16)-(256*(quad&0x1)))>>3)+((((bots[i].y>>16)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
			
			quad = ((bots[i].x>>16)+15 > 255 ? 1 : 0) | ((bots[i].y>>16) > 255 ? 2 : 0);
			p2 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((bots[i].x>>16)+15)-(256*(quad&0x1)))>>3)+((((bots[i].y>>16)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
			
			quad = ((bots[i].x>>16) > 255 ? 1 : 0) | ((bots[i].y>>16)+15 > 255 ? 2 : 0);
			p3 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((bots[i].x>>16))-(256*(quad&0x1)))>>3)+(((((bots[i].y>>16)+15)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
			
			quad = ((bots[i].x>>16)+15 > 255 ? 1 : 0) | ((bots[i].y>>16)+15 > 255 ? 2 : 0);
			p4 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((bots[i].x>>16)+15)-(256*(quad&0x1)))>>3)+(((((bots[i].y>>16)+15)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
			
			//if the bottom two are touching then separate Y
			if(p3 && p4){
				bots[i].y = bots[i].y & (~(0x7<<16));
				bots[i].velY = 0;
			}else if(p1 && p2){
				bots[i].y = (bots[i].y & (~(0x7<<16))) + (8<<16);
				bots[i].velY = 0;
			}
			if(p1 && p3){
				bots[i].x = (bots[i].x & (~(0x7<<16))) + (8<<16);
			}else if(p2 && p4){
				bots[i].x = bots[i].x & (~(0x7<<16));
			}
			//Hard edges taken care of.
			//Now for solitary points.
			int penX = 0;
			int penY = 0;
			if(p1 && !(p3 || p2 || p4)){
				penX = 8 - ((bots[i].x>>16) & 0x7);
				penY = 8 - ((bots[i].y>>16) & 0x7);
				if(penY > penX){
					bots[i].x = (bots[i].x & (~(0x7<<16))) + (8<<16);
				}else{
					bots[i].y = (bots[i].y & (~(0x7<<16))) + (8<<16);
					if(bots[i].velY < 0) bots[i].velY = 0;
				}
			}
			if(p2 && !(p3 || p1 || p4)){
				penX = ((bots[i].x>>16) & 0x7);
				penY = 8 - ((bots[i].y>>16) & 0x7);
				if(penY > penX){
					bots[i].x = (bots[i].x & (~(0x7<<16)));
				}else{
					bots[i].y = (bots[i].y & (~(0x7<<16))) + (8<<16);
					if(bots[i].velY < 0) bots[i].velY = 0;
				}
			}
			if(p3 && !(p2 || p1 || p4)){
				penX = 8 - ((bots[i].x>>16) & 0x7);
				penY = ((bots[i].y>>16) & 0x7);
				if(penY > penX){
					bots[i].x = (bots[i].x & (~(0x7<<16))) + (8<<16);
				}else{
					bots[i].y = (bots[i].y & (~(0x7<<16)));
					if(bots[i].velY > 0) bots[i].velY = 0;
				}
			}
			if(p4 && !(p2 || p1 || p3)){
				penX = ((bots[i].x>>16) & 0x7);
				penY = ((bots[i].y>>16) & 0x7);
				if(penY > penX){
					bots[i].x = (bots[i].x & (~(0x7<<16)));
				}else{
					bots[i].y = (bots[i].y & (~(0x7<<16)));
					if(bots[i].velY > 0) bots[i].velY = 0;
				}
			}
			//SUPER EDGE CASE: Opposite points.
			if(p2 && p3 && !p1 && !p4){
				//This could either be coming from underneath it, or from above it
				if(((bots[i].y>>16)&0x7) < 5){
					bots[i].x = (bots[i].x & (~(0x7<<16)));
					bots[i].y = (bots[i].y & (~(0x7<<16)));
					if(bots[i].velY > 0) bots[i].velY = 0;
				}else{
					bots[i].x = (bots[i].x & (~(0x7<<16))) + (8<<16);
					bots[i].y = (bots[i].y & (~(0x7<<16))) + (8<<16);
					if(bots[i].velY < 0) bots[i].velY = 0;
				}
			}
			if(p1 && p4 && !p2 && !p3){
				if(((bots[i].y>>16)&0x7) < 5){
					bots[i].x = (bots[i].x & (~(0x7<<16))) + (8<<16);
					bots[i].y = (bots[i].y & (~(0x7<<16)));
					if(bots[i].velY > 0) bots[i].velY = 0;
				}else{
					bots[i].x = (bots[i].x & (~(0x7<<16)));
					bots[i].y = (bots[i].y & (~(0x7<<16))) + (8<<16);
					if(bots[i].velY < 0) bots[i].velY = 0;
				}
			}
		}
		
		//TIME TO SLOW THE UNIVERSE DOWN!
		//CHECKING FOR 60 PARTICLES! WOOOOOOO
		for(int i = 0; i < 60; i++){
			if(particles[i].accelX != 0){
				quad = (particles[i].x>>16 > 255 ? 1 : 0) | (particles[i].y>>16 > 255 ? 2 : 0);
				p1 = SCREENBLOCK[quad+4].SCREEN_ENTRY[(((particles[i].x>>16)-(256*(quad&0x1)))>>3)+((((particles[i].y>>16)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
				quad = ((particles[i].x>>16)+8 > 255 ? 1 : 0) | ((particles[i].y>>16) > 255 ? 2 : 0);
				p2 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((particles[i].x>>16)+8)-(256*(quad&0x1)))>>3)+((((particles[i].y>>16)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
				quad = ((particles[i].x>>16) > 255 ? 1 : 0) | ((particles[i].y>>16)+8 > 255 ? 2 : 0);
				p3 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((particles[i].x>>16))-(256*(quad&0x1)))>>3)+(((((particles[i].y>>16)+8)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
				quad = ((particles[i].x>>16)+8 > 255 ? 1 : 0) | ((particles[i].y>>16)+8 > 255 ? 2 : 0);
				p4 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((particles[i].x>>16)+8)-(256*(quad&0x1)))>>3)+(((((particles[i].y>>16)+8)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
				
				//if the bottom two are touching then separate Y
				if(p3 && p4){
					particles[i].y = particles[i].y & (~(0x7<<16));
					particles[i].velY = -particles[i].velY>>1;
					particles[i].velX = particles[i].velX>>1;
				}else if(p1 && p2){
					particles[i].y = (particles[i].y & (~(0x7<<16))) + (8<<16);
					particles[i].velY = 0;
				}
				if(p1 && p3){
					particles[i].x = (particles[i].x & (~(0x7<<16))) + (8<<16);
				}else if(p2 && p4){
					particles[i].x = particles[i].x & (~(0x7<<16));
				}
				//Hard edges taken care of.
				//Now for solitary points.
				int penX = 0;
				int penY = 0;
				if(p1 && !(p3 || p2 || p4)){
					penX = 8 - ((particles[i].x>>16) & 0x7);
					penY = 8 - ((particles[i].y>>16) & 0x7);
					if(penY > penX){
						particles[i].x = (particles[i].x & (~(0x7<<16))) + (8<<16);
					}else{
						particles[i].y = (particles[i].y & (~(0x7<<16))) + (8<<16);
						particles[i].velY = -particles[i].velY>>1;
						particles[i].velX = particles[i].velX>>1;
					}
				}
				if(p2 && !(p3 || p1 || p4)){
					penX = ((particles[i].x>>16) & 0x7);
					penY = 8 - ((particles[i].y>>16) & 0x7);
					if(penY > penX){
						particles[i].x = (particles[i].x & (~(0x7<<16)));
					}else{
						particles[i].y = (particles[i].y & (~(0x7<<16))) + (8<<16);
						particles[i].velY = -particles[i].velY>>1;
						particles[i].velX = particles[i].velX>>1;
					}
				}
				if(p3 && !(p2 || p1 || p4)){
					penX = 8 - ((particles[i].x>>16) & 0x7);
					penY = ((particles[i].y>>16) & 0x7);
					if(penY > penX){
						particles[i].x = (particles[i].x & (~(0x7<<16))) + (8<<16);
					}else{
						particles[i].y = (particles[i].y & (~(0x7<<16)));
						particles[i].velY = -particles[i].velY>>1;
						particles[i].velX = particles[i].velX>>1;
					}
				}
				if(p4 && !(p2 || p1 || p3)){
					penX = ((particles[i].x>>16) & 0x7);
					penY = ((particles[i].y>>16) & 0x7);
					if(penY > penX){
						particles[i].x = (particles[i].x & (~(0x7<<16)));
					}else{
						particles[i].y = (particles[i].y & (~(0x7<<16)));
						particles[i].velY = -particles[i].velY>>1;
						particles[i].velX = particles[i].velX>>1;
					}
				}
				//SUPER EDGE CASE: Opposite points.
				if(p2 && p3 && !p1 && !p4){
					//This could either be coming from underneath it, or from above it
					if(((particles[i].y>>16)&0x7) < 5){
						particles[i].x = (particles[i].x & (~(0x7<<16)));
						particles[i].y = (particles[i].y & (~(0x7<<16)));
						particles[i].velY = -particles[i].velY>>1;
						particles[i].velX = particles[i].velX>>1;
					}else{
						particles[i].x = (particles[i].x & (~(0x7<<16))) + (8<<16);
						particles[i].y = (particles[i].y & (~(0x7<<16))) + (8<<16);
						particles[i].velY = -particles[i].velY>>1;
						particles[i].velX = particles[i].velX>>1;
					}
				}
				if(p1 && p4 && !p2 && !p3){
					if(((particles[i].y>>16)&0x7) < 5){
						particles[i].x = (particles[i].x & (~(0x7<<16))) + (8<<16);
						particles[i].y = (particles[i].y & (~(0x7<<16)));
						particles[i].velY = -particles[i].velY>>1;
						particles[i].velX = particles[i].velX>>1;
					}else{
						particles[i].x = (particles[i].x & (~(0x7<<16)));
						particles[i].y = (particles[i].y & (~(0x7<<16))) + (8<<16);
						particles[i].velY = -particles[i].velY>>1;
						particles[i].velX = particles[i].velX>>1;
					}
				}
			}
		}
		
		//Collide bullets with the tilemap
		for(int i = 9; i>= 0; i--){
			quad = ((playerBullets[i].x>>16)+4 > 255 ? 1 : 0) | ((playerBullets[i].y>>16)+4 > 255 ? 2 : 0);
			p1 = SCREENBLOCK[quad+4].SCREEN_ENTRY[((((playerBullets[i].x>>16)+4)-(256*(quad&0x1)))>>3)+(((((playerBullets[i].y>>16)+4)-(256*(quad>>1)))>>3)*32)] > 0 ? 1 : 0;
			if(p1){
				playerBullets[i].velX = 0;
				playerBullets[i].velY = 0;
				playerBullets[i].accelY = 0;
			}
		}
		
		//Collide bullets with the spawners, collide bullets with the bots.
		for(int i = 0; i < 6; i++){
			for(int j = 0; j < 10; j++){
				if(playerBullets[j].accelY && spawners[i].health > 0){
					if(playerBullets[j].x+(4<<16) < spawners[i].x+(24<<16) && playerBullets[j].y+(4<<16) < spawners[i].y+(24<<16) && playerBullets[j].x+(4<<16) > spawners[i].x && playerBullets[j].y+(4<<16) > spawners[i].y){
						playerBullets[j].velX = 0;
						playerBullets[j].velY = 0;
						playerBullets[j].accelY = 0;
						spawners[i].health--;
						if(spawners[i].health <= 0){
							numSteps = 20;
							for(int i = 0; i < 5; i++){
								current_r_values[i] = ((PALETTE_palette[4]>>0)&0x1f)<<8;
								current_g_values[i] = ((PALETTE_palette[4]>>5)&0x1f)<<8;
								current_b_values[i] = ((PALETTE_palette[4]>>10)&0x1f)<<8;
								target_r[i] = (((PALETTE_palette[i]>>0)&0x1f)<<8);
								target_g[i] = (((PALETTE_palette[i]>>5)&0x1f)<<8);
								target_b[i] = (((PALETTE_palette[i]>>10)&0x1f)<<8);
								step_r[i] = ((((PALETTE_palette[i]>>0)&0x1f)<<8) - current_r_values[i])/(numSteps);
								step_g[i] = ((((PALETTE_palette[i]>>5)&0x1f)<<8) - current_g_values[i])/(numSteps);
								step_b[i] = ((((PALETTE_palette[i]>>10)&0x1f)<<8) - current_b_values[i])/(numSteps);
							}
							fading = 1;
							fadeTimer = 0;
							numActiveSpawners--;
							for(int k = 0; k < 15; k++){
								particles[15*gibletPosition + k].x = spawners[i].x;
								particles[15*gibletPosition + k].y = spawners[i].y;
								particles[15*gibletPosition + k].accelY = 25<<8;
								particles[15*gibletPosition + k].accelX = 1;
								particles[15*gibletPosition + k].velX = (rand()%60-30)<<12;
								particles[15*gibletPosition + k].velY = (rand()%60-45)<<12;
							}
							gibletPosition++;
							if(gibletPosition > 3){
								gibletPosition = 0;
							}
							for(int k = 0; k < 15; k++){
								particles[15*gibletPosition + k].x = spawners[i].x;
								particles[15*gibletPosition + k].y = spawners[i].y;
								particles[15*gibletPosition + k].accelY = 25<<8;
								particles[15*gibletPosition + k].accelX = 1;
								particles[15*gibletPosition + k].velX = (rand()%60-30)<<12;
								particles[15*gibletPosition + k].velY = (rand()%60-45)<<12;
							}
							gibletPosition++;
							if(gibletPosition > 3){
								gibletPosition = 0;
							}
						}
					}
				}
			}
		}
		for(int i = 0; i < 16; i++){
			for(int j = 0; j < 10; j++){
				if(playerBullets[j].accelY && bots[i].health > 0){
					if(playerBullets[j].x+(4<<16) < bots[i].x+(16<<16) && playerBullets[j].y+(4<<16) < bots[i].y+(16<<16) && playerBullets[j].x+(4<<16) > bots[i].x && playerBullets[j].y+(4<<16) > bots[i].y){
						playerBullets[j].velX = 0;
						playerBullets[j].velY = 0;
						playerBullets[j].accelY = 0;
						bots[i].health--;
						if(bots[i].health <=0){
							numActiveBots--;
							for(int k = 0; k < 15; k++){
								particles[15*gibletPosition + k].x = bots[i].x;
								particles[15*gibletPosition + k].y = bots[i].y;
								particles[15*gibletPosition + k].accelY = 25<<8;
								particles[15*gibletPosition + k].accelX = 1;
								particles[15*gibletPosition + k].velX = (rand()%60-30)<<12;
								particles[15*gibletPosition + k].velY = (rand()%60-45)<<12;
							}
							gibletPosition++;
							if(gibletPosition > 3){
								gibletPosition = 0;
							}
						}
					}
				}
			}
		}
		
		prevREG_BUTTONS = REG_BUTTONS;
		
		frameCount++;
		if(frameCount > 249){
			frameCount = 0;
		}
		if(player.x>>16 < scrollX + 100)
			scrollX = (player.x>>16) - 100;
		if(player.x>>16 > scrollX + 140)
			scrollX = (player.x>>16) - 140;
		if(player.y>>16 < scrollY + 60)
			scrollY = (player.y>>16) - 60;
		if(player.y>>16 > scrollY + 100)
			scrollY = (player.y>>16) - 100;
			
		if(scrollX < 0){
			scrollX = 0;
		}
		if(scrollY < 0){
			scrollY = 0;
		}
		if(scrollY > 512-160){
			scrollY = 512-160;
		}
		if(scrollX > 512-240){
			scrollX = 512-240;
		}
		
		if(numActiveSpawners <= 0 && !fading){
			numSteps = 120;
			for(int i = 0; i < 5; i++){
				target_r[i] = (((PALETTE_palette[1]>>0)&0x1f)<<8);
				target_g[i] = (((PALETTE_palette[1]>>5)&0x1f)<<8);
				target_b[i] = (((PALETTE_palette[1]>>10)&0x1f)<<8);
				step_r[i] = ((((PALETTE_palette[1]>>0)&0x1f)<<8) - current_r_values[i])/(numSteps);
				step_g[i] = ((((PALETTE_palette[1]>>5)&0x1f)<<8) - current_g_values[i])/(numSteps);
				step_b[i] = ((((PALETTE_palette[1]>>10)&0x1f)<<8) - current_b_values[i])/(numSteps);
			}
			fading = 1;
			fadeTimer = 0;
		}
		
		vid_vsync();
		
		matrixID = 67;
		for(int i = 0; i < 16; i++){
			obj_attr_mem[matrixID] = lu_cos(183*bots[i].angle)>>4;
			matrixID+=4;
			obj_attr_mem[matrixID] = -(lu_sin(183*bots[i].angle)>>4);
			matrixID+=4;
			obj_attr_mem[matrixID] = lu_sin(183*bots[i].angle)>>4;
			matrixID+=4;
			obj_attr_mem[matrixID] = lu_cos(183*bots[i].angle)>>4;
			matrixID+=4;
		}
		
		if(fading){
			for(int i = 0; i < 5; i++){
				current_r_values[i] += step_r[i];
				current_g_values[i] += step_g[i];
				current_b_values[i] += step_b[i];
			}
			fadeTimer++;
			if(fadeTimer == numSteps){
				fading = 0;
				fadeTimer = 0;
				for(int i = 0; i < 5; i++){
					current_r_values[i] = target_r[i];
					current_g_values[i] = target_g[i];
					current_b_values[i] = target_b[i];
				}
				if(numActiveSpawners <=0 && target_r[4] == (((PALETTE_palette[1]>>0)&0x1f)<<8)){
					state = 3;
				}
			}
			for(int i = 0; i < 5; i++){
				background_palette[i] = RGB(current_r_values[i]>>8, current_g_values[i]>>8, current_b_values[i]>>8);
				sprite_palette[i] = RGB(current_r_values[i]>>8, current_g_values[i]>>8, current_b_values[i]>>8);
			}
		}
		
		for(int i = 0; i < 6; i++){
			if((spawners[i].x>>16)-scrollX > 240 || ((spawners[i].x>>16)+24) < scrollX || (spawners[i].y>>16)+24 < scrollY || (spawners[i].y>>16)-scrollY > 160){
				obj_attr_mem[4*(122+i)] = SPRITE_RENDER_HIDDEN;
			}else{
				obj_attr_mem[4*(122+i)] = (((spawners[i].y>>16)-scrollY)&0xFF) | SPRITE_RENDER_NORMAL | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
				obj_attr_mem[4*(122+i)+1] = (((spawners[i].x>>16)-scrollX)&0x1FF) | SPRITE_SIZE_MEDIUM;
				obj_attr_mem[4*(122+i)+2] = (spawners[i].health > 0 ? (94+(animation[frameCount/10]*32)) : 286);
			}
			if(frameCount == 90 && spawners[i].health > 0){
				//Spawn up to 6 bots, to a total of 16
				if(numActiveBots < 16){
					int botIndex = 0;
					while(bots[botIndex].health > 0){
						botIndex++;
					}
					bots[botIndex].x = spawners[i].x+(4<<16);
					bots[botIndex].y = spawners[i].y+(4<<16);
					numActiveBots++;
					bots[botIndex].health = 2;
				}
			}
		}
		if((player.x>>16)-scrollX > 240 || ((player.x>>16)+24) < scrollX || (player.y>>16)+24 < scrollY || (player.y>>16)-scrollY > 160){
			obj_attr_mem[0] = SPRITE_RENDER_HIDDEN;
		}else{
			obj_attr_mem[0] = (((player.y>>16)-scrollY) & 0xFF) | SPRITE_RENDER_NORMAL | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
			obj_attr_mem[1] = (((player.x>>16)-scrollX) & 0x1FF) | (obj_attr_mem[1] & 0xFE00);
			//PLAYER ANIMATIONS TIME, FUCK YEAH!
			generalAnimationCounter++;
			if(generalAnimationCounter > 19){
				generalAnimationCounter = 0;
			}
			if(player.velY != 0){
				if(~REG_BUTTONS & BTN_DOWN){
					obj_attr_mem[2] = 92;
				}else if(~REG_BUTTONS & BTN_UP){
					obj_attr_mem[2] = 90;
				}else{
					obj_attr_mem[2] = 80;
				}
			}else if(player.velX != 0){
				if(~REG_BUTTONS & BTN_UP){
					obj_attr_mem[2] = 82+(walk[generalAnimationCounter/5]*2);
				}else{
					obj_attr_mem[2] = 72+(walk[generalAnimationCounter/5]*2);
				}
			}else{
				if(~REG_BUTTONS & BTN_UP){
					obj_attr_mem[2] = 82;
				}else{
					obj_attr_mem[2] = 72;
				}
			}
		}
		for(int i = 0; i < 10; i++){
			if(playerBullets[i].accelY != 0){
				if((playerBullets[i].x>>16)-scrollX > 240 || ((playerBullets[i].x>>16)+24) < scrollX || (playerBullets[i].y>>16)+24 < scrollY || (playerBullets[i].y>>16)-scrollY > 160){
					obj_attr_mem[4*(2+i)] = SPRITE_RENDER_HIDDEN;
				}else{
					obj_attr_mem[4*(2+i)] = (((playerBullets[i].y>>16)-scrollY)&0xFF) | SPRITE_RENDER_NORMAL | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
					obj_attr_mem[4*(2+i)+1] = (((playerBullets[i].x>>16)-scrollX)&0x1FF) | SPRITE_SIZE_SMALL | (playerBullets[i].velX < 0 ? (1<<0xC) : 0) | (playerBullets[i].velY > 0 ? 1<<0xD : 0);
					obj_attr_mem[4*(2+i)+2] = playerBullets[i].velY != 0 ? 60 : 62;
				}
			}else{
				obj_attr_mem[4*(2+i)+2] = 64 + (poofAnim[playerBulletAnimCounters[i]]*2);
				playerBulletAnimCounters[i]++;
				if(playerBulletAnimCounters[i] > 3){
					obj_attr_mem[4*(2+i)] = SPRITE_RENDER_HIDDEN;
				}
			}
		}
		for(int i = 0; i < 16; i++){
			if((bots[i].x>>16)-scrollX > 240 || ((bots[i].x>>16)+24) < scrollX || (bots[i].y>>16)+24 < scrollY || (bots[i].y>>16)-scrollY > 160 || bots[i].health <= 0){
				obj_attr_mem[4*(12+i)] = SPRITE_RENDER_HIDDEN;
			}else{
				obj_attr_mem[4*(12+i)] = (((bots[i].y>>16)-scrollY)&0xFF) | SPRITE_RENDER_AFFINE | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
				obj_attr_mem[4*(12+i)+1] = (((bots[i].x>>16)-scrollX)&0x1FF) | SPRITE_SIZE_EXTEND | ((i+4)<<9);
				obj_attr_mem[4*(12+i)+2] = 42;
			}
		}
		for(int i = 0; i < 60; i++){
			if((particles[i].x>>16)-scrollX > 240 || ((particles[i].x>>16)+8) < scrollX || (particles[i].y>>16)+8 < scrollY || (particles[i].y>>16)-scrollY > 160 || particles[i].accelX == 0){
				obj_attr_mem[4*(28+i)] = SPRITE_RENDER_HIDDEN;
			}else{
				obj_attr_mem[4*(28+i)] = (((particles[i].y>>16)-scrollY)&0xFF) | SPRITE_RENDER_AFFINE | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
				obj_attr_mem[4*(28+i)+1] = (obj_attr_mem[(i+28)*4+1]&0xFE00) | (((particles[i].x>>16)-scrollX)&0x1FF);
			}
		}
		REG_BG0_HOFF = scrollX;
		REG_BG0_VOFF = scrollY;
	}
	
	//Set up victory screen
	REG_BG0_HOFF = 0;
	REG_BG0_VOFF = 0;
	REG_BG0_CNT = SBB(2) | CBB(0);
	
	dma_cpy(background_palette, PALETTE_palette, 5, 3);
	dma_cpy(sprite_palette, PALETTE_palette, 5, 3);
	
	for(int i = 0; i < 128; i++){
		obj_attr_mem[(i)*4] = SPRITE_RENDER_HIDDEN;
	}
	
	for(int i = 0; i < 120; i++){
		particles[i].x = (rand()%232)<<16;
		particles[i].y = (rand()%400-400)<<16;
		particles[i].velX = 0;
		particles[i].velY = 0;
		particles[i].accelY = (25<<8);
		obj_attr_mem[(i+2)*4+0] = 0 | SPRITE_RENDER_HIDDEN | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
		obj_attr_mem[(i+2)*4+1] = 0 | SPRITE_RENDER_PALETTE0 | SPRITE_SIZE_SMALL | (i/30)<<9;
		obj_attr_mem[(i+2)*4+2] = (i/24)*2;
	}
	
	REG_BG0_CNT = SBB(3) | CBB(0);
	REG_BG0_HOFF = -12;
	REG_BG0_VOFF = -50;
	
	//Count a bunch of frames then go back into the game.
	while(state == 3){
		vid_vsync();
		for(int i = 0; i < 120; i++){
			particles[i].y += particles[i].velY;
			particles[i].velY += particles[i].accelY;
			if(particles[i].y>(160<<16)){
				particles[i].y = (rand()%50-60)<<16;
				particles[i].x = (rand()%232)<<16;
				particles[i].velY = 0;
			}
			//is OnScreen?
			if(particles[i].x > (-8)<<16 && particles[i].x < 248<<16 && particles[i].y > (-8)<<16 && particles[i].y < 168<<16){
				obj_attr_mem[(i+2)*4+0] = ((particles[i].y>>16)&0xFF) | SPRITE_RENDER_AFFINE | SPRITE_COLORMODE_8BPP | SPRITE_SHAPE_SQUARE;
				obj_attr_mem[(i+2)*4+1] = (obj_attr_mem[(i+2)*4+1]&0xFE00) | ((particles[i].x>>16)&0x1FF);
			}else{
				obj_attr_mem[(i+2)*4+0] = SPRITE_RENDER_HIDDEN;
			}
		}
	}
}
