// rapid ball
// font by zee-3.com, spiderman

#include <stdint.h>
#include <limits.h>
#include <stdlib.h> // rand

#include "bitbox.h"

#include "lib/blitter.h"
#include "lib/chiptune.h"

#include "bg.h"
extern const unsigned char ball_spr[];
extern const unsigned char platform_spr[];
extern const unsigned char heart_spr[];

const int player_hspeed=3;
const int player_vspeed=2;

#define top_y 20
#define bottom_y 300
#define right_x 290
#define left_x 80
#define pos_score   19+ 9*25
#define pos_lives   20+11*25
#define pos_hiscore 18+11*25

extern struct ChipSong rapid1_chipsong, rapid2_chipsong;

extern uint8_t songdata[], songdata2[];

enum Game_state {
	state_welcome, state_ready, state_play, state_dead
};


int score, vspeed, in_menu, lives;
int best_score;
int pause; // tempo for pause before next state
enum Game_state state; // game state

#define NB_LOGS 7

struct object *ob_player, *ob_background, *platform[NB_LOGS], *ob_heart;
/* sprites are evenly spaced in the playground. They have one frame per type of log */
/* ob_heart */

uint8_t vram[25*19]; 

void hide_all() {
	ob_player->y=2000;
	ob_heart->y=2000;
	for (int i=0;i<NB_LOGS;i++)
		platform[i]->y=2000;
}

int collide(int obj_id)
// check if platform i or heart (if i==100) collides with ball.
{
	// collision between an object and the player : check AABB collision between player & obstacle
	// ball collision is smaller than sprite of 2 pix in every direction
	struct object *pl = ob_player;
	struct object *oth =  obj_id==100  ? ob_heart : platform[obj_id];
	return !(pl->x+pl->w-2<oth->x || oth->x+oth->w<pl->x+2 || pl->y+pl->h-2<oth->y || oth->y+oth->h<pl->y+2);
}

void setup_bg(int i) 
{
	tmap_blit(ob_background, 0,0, bg_header,bg_tmap[i]);
}

void setup_welcome()
{
	setup_bg(bg_intro); 
	// best score
	int x=best_score;
	for (int i=0;i<5;i++) {
		vram[pos_hiscore+4-i] = bg_zero+x%10;
		x/=10;
	}
	lives=3;
	state = state_welcome;
	score=0;
	hide_all();
	chip_play(&rapid1_chipsong);
}


void display()
{
	// display score (5 digits)
	int x=score;
	for (int i=0;i<5;i++) {
		vram[pos_score+4-i] = bg_zero+x%10;
		x/=10;
	}
	// lives
	for (int i=0;i<5;i++)
		vram[pos_lives+i] = lives>i ? bg_heart : bg_empty;
}


void setup_ready(void)
{
	state=state_ready;
	setup_bg(bg_ready);
	pause=210; 
	display();
	chip_play(&rapid2_chipsong);
}

void setup_play(void) {
	chip_play(0);

	setup_bg(bg_game); 
	state = state_play;
	ob_player->y = 44;
	for (int i=0;i<NB_LOGS;i++) {
		platform[i]->x = left_x+(right_x-left_x-platform[0]->w)*i/NB_LOGS;
		platform[i]->y = top_y +(bottom_y-top_y)*i/NB_LOGS + 128; // keep a little bit of space below
		platform[i]->fr = 0; // standard log.
	}
	display();
}

void setup_dead() {
	hide_all();
	setup_bg(bg_over); // do it each frame ?			
	pause=200;
	state=state_dead;
	// play dead song ?
}

void loose() 
{
	if (lives) {
		lives--; // lose a life
		setup_ready();			
	} else {
		if (best_score<score) 
			best_score = score;
		setup_dead();
	}
}

void play_frame() 
{
	// update bg
	if (vga_frame%16==0) {
		score++;
		// update bg with score
	}

	vspeed = (score/64)+2;

	// handle player input / movement / animation
	if (GAMEPAD_PRESSED(0,left) && ob_player->x > left_x) 
		ob_player->x -= player_hspeed;
	if (GAMEPAD_PRESSED(0,right) && ob_player->x < right_x - ob_player->w) 
		ob_player->x += player_hspeed;
	ob_player->y += player_vspeed;

	// move/update logs
	// 30 fps only for objects (xxx should use fractional positions)
	if (vga_frame%2==0) {
		// move heart if on sreen
		if (ob_heart->y!=2000) 
			ob_heart->y-=vspeed;
		if (ob_heart->y+(int)ob_heart->h <= top_y)
			ob_heart->y=2000;
		// move logs
		for (int i=0; i<NB_LOGS;i++) {
			platform[i]->y -= vspeed;
			if (platform[i]->y+(int)platform[i]->h <= top_y)	{
				/* if logs have reached the top of the screen
				 push enemies on bottom of screen as needed 
				 no need to create/delete them, they're just 
				 too high to be seen / collide.
				 Also, randomly add hearts/deadly platforms.
				 */
				platform[i]->x = left_x+rand()%(right_x-platform[i]->w-left_x);
				platform[i]->y += bottom_y-top_y;
				platform[i]->fr = rand()%4; 
				// should we place an heart ? 
				// do so if heart is avail, on a blue platform, 1/8 odds
				if (platform[i]->fr == 3 && ob_heart->y>1000 && rand()%8==0) {
					ob_heart->x = platform[i]->x+20;
					ob_heart->y = platform[i]->y-15;
				}
			}
		}
	}

	// -- test collisions
	// borders
	if (ob_player->y <= top_y || ob_player->y >= bottom_y - ob_player->h)
	{
		loose();
		return;
	}
	
	// logs
	for (int i=0;i<NB_LOGS;i++)
		if (collide(i)) // check frame !
			switch(platform[i]->fr) {
				case 0 : 
				case 3 : 
					// put on top of platform
					ob_player->y = platform[i]->y-ob_player->h;
					break;
				case 1 : // deadly : 
					loose();
					return;
				case 2 : // transparent 
					break;				
			}

	if (collide(100)) // heart collision 
	{
		if (lives<4)
			lives +=1;
		ob_heart->y=2000;
	}

	display();
}

void game_init() {
	blitter_init();

	// load resources, sprites, bg
	ob_background = tilemap_new (bg_tset,0,0,bg_header,vram);
	ob_player = sprite_new(ball_spr,0,2000,0);
	ob_heart = sprite_new(heart_spr,0,2000,0);
	for (int i=0;i<NB_LOGS;i++)
		platform[i] = sprite_new(platform_spr,0,2000,0);

	best_score = 0;

	setup_welcome();
}

void game_frame() {
	kbd_emulate_gamepad();
	switch (state) 
	{
		case state_welcome : 
			if (GAMEPAD_PRESSED(0,A) || GAMEPAD_PRESSED(0,start)) 
				setup_ready();
			break;

		case state_ready : 
			if (!pause--) 
				setup_play();
			else if (pause%30==0 && pause<180) {
				hide_all();
				ob_player->x = 173; 
				ob_player->y = 44 + (pause%60*1000);
			}
			break;

		case state_dead : 
			if (!pause--) 
				setup_welcome();
			break;

		case state_play : 
			play_frame();
			break;
	}
}
