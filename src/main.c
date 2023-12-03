#include <gb/gb.h>
#include <gbdk/font.h>
#include <stdint.h>
#include <stdio.h>
#include <rand.h>

#include "Sprite.h"
#include "food_spritesheet_py.h"
#include "snake_spritesheet_py.h"
#include "progressbar_tiles_tiles_py.h"
#include "windowmap.h"

#include "level_title_screens.h"
#include "level1_tiles_py.h"
#include "level1_map_py.h"
#include "level1_colliders.h"
#include "level2_tiles_py.h"
#include "level2_map_py.h"
#include "level2_colliders.h"
#include "level3_tiles_py.h"
#include "level3_map_py.h"
#include "level3_colliders.h"
#include "level4_tiles_py.h"
#include "level4_map_py.h"
#include "level4_colliders.h"

#define SNAKE_MAX_SIZE 36

// uint8_t SCREEN_LEFT = 40;
// uint8_t SCREEN_WIDTH = 80;
// uint8_t SCREEN_TOP = 40;
// uint8_t SCREEN_HEIGHT = 80;

uint8_t SCREEN_LEFT = 0;
uint8_t SCREEN_WIDTH = 160;
uint8_t SCREEN_TOP = 8;
uint8_t SCREEN_HEIGHT = 144;

/*
The snake sprite has5 8x8 tiles:
    index 0: Head Up
    index 1: Head Left
    index 2: Head Dead Up
    index 3: Head Dead Left
    index 4: Body 
*/
/* Load snake sprite on the first 5 locations of sprite memory */
uint8_t SNAKE_MEMIND = 0;
uint8_t SNAKE_NTILES = 5;

uint8_t SNAKE_HEAD_UP = 0 + 0;  //SNAKE_MEMIND
uint8_t SNAKE_HEAD_L = 0 + 1;  //SNAKE_MEMIND + 1
uint8_t SNAKE_DEAD_HEAD_UP = 0 + 2;  //SNAKE_MEMIND + 2
uint8_t SNAKE_DEAD_HEAD_L = 0 + 3;  //SNAKE_MEMIND + 3
uint8_t SNAKE_BODY = 0 + 4;  //SNAKE_MEMIND + 4
uint8_t SNAKE_SPRITE_SIZE = 8;


/* The food sprite has 3 8x8 tiles:
    index 0: biscuit
    index 1: carrot
    index 2: turnip
*/
/* Load the food sprite right after snake */
uint8_t FOOD_MEMIND = 5;  
uint8_t FOOD_NTILES = 4;

uint8_t FOOD_BISCUIT = 5 + 0; //FOOD_MEMIND
uint8_t FOOD_CARROT = 5 + 1; //FOOD_MEMIND + 1
uint8_t FOOD_TURNIP = 5 + 2; //FOOD_MEMIND + 2
uint8_t KEY = 5 + 3; //FOOD_MEMIND + 3
uint8_t FOOD_SPRITE_SIZE = 8;

/*
Level start locations
*/
uint8_t LEVEL_STARTX = 80;
uint8_t LEVEL_STARTY = 80;
uint8_t LEVEL_FOOD_STARTX = 72;
uint8_t LEVEL_FOOD_STARTY = 50;

void move_tail(struct SnakePart* head, uint8_t headx, uint8_t heady);

struct LevelData{
  unsigned char* tiles;
  unsigned char* map;
  unsigned char* background_colliders;
  short ntiles;
  short head_startx;
  short head_starty;
  unsigned char head_dir;
  short food_startx;
  short food_starty;
  short left_boundary;
  short right_boundary;
  short top_boundary;
  short bottom_boundary;
  unsigned char next_level_len;
  short start_speed;
  unsigned char speedup;
  short speed_increase_len;
  unsigned char* titlescreen;
  short food_timer;
};

void wait(uint8_t n){
  // Interrupt-based delay.
  // Returns after n Vertical Blanking interrupts (screen refreshes)
  uint8_t x;
  for (x = 0; x < n; x++){
    wait_vbl_done();
  }
}

void fadeout(void){
  uint8_t i;

  for (i=0; i<4; i++){
    switch (i)
    {
    case 0:
      BGP_REG = 0xE4;
      break;

    case 1:
      BGP_REG = 0xF9;
      break;
    
    case 2:
      BGP_REG = 0xFE;
      break;
    
    case 3:
      BGP_REG = 0xFF;
      break;
    }
    wait(10);
  }
}

void fadein(void){
  uint8_t i;

  for (i=0; i<3; i++){
    switch (i)
    {
    case 0:
      BGP_REG = 0xFE;
      break;

    case 1:
      BGP_REG = 0xF9;
      break;
    
    case 2:
      BGP_REG = 0xE4;
      break;
    }
    wait(10);
  }
}

void flash_sprites(void){
    uint8_t y;

    for (y=0; y<3; y++){
      HIDE_SPRITES;
      wait(10);
      SHOW_SPRITES;
      wait(10);
    }
}

void flash_sprite(struct Sprite* sprite){
  char y;

  for (y=0; y<3; y++){
    move_sprite(sprite->spriteid, 0, 0);
    wait(10);
    move_sprite(sprite->spriteid, sprite->x, sprite->y);
    wait(10);
  }
}

void show_finalscreen(unsigned char* finalscreen, char type){
  HIDE_SPRITES;
  HIDE_WIN;
  
  set_bkg_tiles(0, 0, 20, 18, finalscreen);

  SHOW_BKG;
  fadein();
  if (type == 0){
    play_gameover_sound();
  }
  else if (type == 1){
    play_wining_sound();
  }

  waitpad(J_START);
  waitpadup();
  HIDE_BKG;
}

void show_titlescreen(unsigned char* titlescreen){
  HIDE_SPRITES;
  HIDE_WIN;
  
  set_bkg_tiles(0, 0, 20, 18, titlescreen);

  SHOW_BKG;
  
  // Scroll background so it looks it comes down from the top of the screen
  move_bkg(0, 72);
  fadein();

  play_leveltitle_sound();
  for (int i = 72; i >= 0; i--){
    move_bkg(0, i);
    wait_vbl_done();
  }

  waitpad(J_START);
  waitpadup();
  HIDE_BKG;
}

UBYTE sprite_collision_coord(struct Sprite* sp1, uint8_t x, uint8_t y){
  /* 
    Check for collision between sp1 and sp2. Using sp1 as reference,
    define a 2-pixel boundary around its center and check if the 
    passed coordinates are inside that boundary.

    The passed coordinates denote the top-left corner of a sprite
    sp2, so find the equivalent center of this sprite.
  */
  uint8_t sp1_left_bound = sp1->x + (sp1->size >> 1) - 2;
  uint8_t sp1_right_bound = sp1->x + (sp1->size >> 1) + 2;
  uint8_t sp1_top_bound = sp1->y + (sp1->size >> 1) - 2;
  uint8_t sp1_bottom_bound = sp1->y + (sp1->size >> 1) + 2;

  uint8_t sp2_xcenter = x + (sp1->size >> 1);
  uint8_t sp2_ycenter = y + (sp1->size >> 1);

  return (sp2_xcenter >= sp1_left_bound) && \
         (sp2_xcenter <= sp1_right_bound) && \
         (sp2_ycenter >= sp1_top_bound) && \
         (sp2_ycenter <= sp1_bottom_bound);
}

UBYTE sprite_collision(struct Sprite* sp1, struct Sprite* sp2){
  /* 
    Check for collision between sp1 and sp2. Using sp1 as reference,
    define a 2-pixel boundary around its center and check if the center
    of sp2 is inside that boundary.
  */

  uint8_t sp1_left_bound = sp1->x + (sp1->size >> 1) - 2;
  uint8_t sp1_right_bound = sp1->x + (sp1->size >> 1) + 2;
  uint8_t sp1_top_bound = sp1->y + (sp1->size >> 1) - 2;
  uint8_t sp1_bottom_bound = sp1->y + (sp1->size >> 1) + 2;

  uint8_t sp2_xcenter = sp2->x + (sp2->size >> 1);
  uint8_t sp2_ycenter = sp2->y + (sp2->size >> 1);

  return (sp2_xcenter >= sp1_left_bound) && \
         (sp2_xcenter <= sp1_right_bound) && \
         (sp2_ycenter >= sp1_top_bound) && \
         (sp2_ycenter <= sp1_bottom_bound);
}

uint8_t head_tail_collision(struct SnakePart* head){
  struct SnakePart* tail = head->next;

  while (tail != NULL){
    if (sprite_collision(&head->sprite, &tail->sprite)) {
      return 1;
      break;
    }
    tail = tail->next;
  }
  return 0;
}

UBYTE head_tail_collision_coord(struct SnakePart* head, uint8_t x, uint8_t y){
  while (head != NULL){
    if (sprite_collision_coord(&head->sprite, x, y)) {
      return 1;
      break;
    }
    head = head->next;
  }
  return 0;
}

UBYTE background_collision(uint8_t x, uint8_t y, char* bkg_colliders, uint8_t* debug_tiles){
  /* 
    Convert x, y into tile indices (20 wide, 18 tall) and check the
    bkg_colliders array at that index. If there is a 0x1 at that 
    location, then a collision with a background collider has occurred.

    The sprite x coordinate in OAM is 8 pixels higher than bkg_colliders coordinates.
    The sprite y coordinate in OAM is 16 pixels higher than the bkg_colliders coordinates.
    */
  uint8_t collision;
  uint16_t tileind;
  uint8_t stride = 20;

  uint8_t x_tile;
  uint8_t y_tile;

  x_tile = (x-8) / 8;
  y_tile = (y-16) / 8;

  tileind = y_tile*stride + x_tile;
  debug_tiles[1] = bkg_colliders[tileind] + 0x1;
  // set_win_tiles(14, 0, 3, 1, debug_tiles);

  if (bkg_colliders[tileind] == 1) {
    collision = 1;
  }
  else{
    collision = 0;
  }

  return collision;
}

void move_snake(struct SnakePart* head, int8_t x, int8_t y){
  /*
  For the snake movement, the head moves in the direction of the joypad 
  and the tail follows the head.
  */

  // Process head movement first
  uint8_t headx;
  uint8_t heady;
  uint8_t newx;
  uint8_t newy;

  headx = head->sprite.x;
  heady = head->sprite.y;
  newx = head->sprite.x + x;
  newy = head->sprite.y + y;

  // Load appropriate head sprites based on movement
  if (newx <  head->sprite.x){
    set_sprite_tile(head->sprite.spriteid, SNAKE_HEAD_L);
    set_sprite_prop(head->sprite.spriteid, get_sprite_prop(0) & ~S_FLIPX);
  }
  else if (newx > head->sprite.x){
    set_sprite_tile(head->sprite.spriteid, SNAKE_HEAD_L);
    set_sprite_prop(head->sprite.spriteid, S_FLIPX);
  }
  else if (newy < head->sprite.y){
    set_sprite_tile(head->sprite.spriteid, SNAKE_HEAD_UP);
    set_sprite_prop(head->sprite.spriteid, get_sprite_prop(0) & ~S_FLIPY);
  }
  else if (newy > head->sprite.y){
    set_sprite_tile(head->sprite.spriteid, SNAKE_HEAD_UP);
    set_sprite_prop(head->sprite.spriteid, S_FLIPY);
  }

  // Move head
  head->sprite.x = newx;
  head->sprite.y = newy;
  move_sprite(head->sprite.spriteid, head->sprite.x, head->sprite.y);
  move_tail(head, headx, heady);
}

void move_tail(struct SnakePart* head, uint8_t headx, uint8_t heady){
  struct SnakePart* tail;
  uint8_t newx;
  uint8_t newy;

  //Process tail
  tail = head->next;

  while (1){
    newx = headx;
    newy = heady;
    headx = tail->sprite.x;
    heady = tail->sprite.y;
    tail->sprite.x = newx;
    tail->sprite.y = newy;

    move_sprite(tail->sprite.spriteid, newx, newy);
    
    if (tail->next == NULL){
      break;
    }
    else{
      head = tail;
      tail = head->next;
    }
  }
}

void movefood(struct Sprite* food, struct SnakePart* head, char* bkg_colliders, short lbound, short rbound, short tbound, short bbound, uint8_t* debug_tiles, unsigned char key, struct LevelData* level_data){
    initrand(DIV_REG);
    /* This function places a new food item at the location of the snake's tail tip */
    uint8_t collision;
    uint8_t new_spriteid;

    uint8_t randx;
    uint8_t randy;

    do {
      randx = rand();
      randy = rand();

      // Add the upper and lower nibbles but restrict answer to 4 bits.
      randx = ((randx >> 4) + (randx & 0xF)) & 0xF;
      randy = ((randy >> 4) + (randy & 0xF)) & 0xF;

      // Check for world boundaries
      if (randx <= lbound){
        randx = randx + (lbound + 1);
      }
      else if (randx >= rbound){
        randx = randx - rbound + (lbound + 1);
      }

      if (randy <= tbound){
        randy = randy + (tbound + 1);
      }
      else if (randy >= bbound){
        randy = randy - bbound + (tbound+1);
      }

      // Convert to display x,y values
      randx = randx*8 + 8;
      randy = randy*8 + 16;
      
      collision = background_collision(randx, randy, bkg_colliders, debug_tiles);
      collision = collision || head_tail_collision_coord(head, randx, randy);
      
    } while (collision != 0);

    if (key){
      new_spriteid = 39;
    }
    else{
      new_spriteid = food->spriteid+1;
      if (new_spriteid > 38){
        new_spriteid = 36;
      }
    }
    move_sprite(food->spriteid, 0, 0);
    food->spriteid = new_spriteid;

    food->x = randx;
    food->y = randy; 
    move_sprite(food->spriteid, food->x, food->y);

    food->timer = level_data->food_timer;
    food->animation_frame = 40;

    if (key){
      // Disable the timer
      food->timer = 255;
    }
}

void score2tile(uint8_t score, uint8_t* score_tiles){
    if (score < 255){
      score_tiles[0] = (score / 100) + 0x01;
      score_tiles[1] = (score - (score/100)*100)/10 + 0x01;
      score_tiles[2] = (score - (score/10)*10) + 0x01;
    }
    else {
      score_tiles[0] = 0x01;
      score_tiles[1] = 0x01;
      score_tiles[2] = 0x01;
    }
}

void play_eating_sound(void){
    NR10_REG = 0x37;
    NR11_REG = 0X85;
    NR12_REG = 0X43;
    NR13_REG = 0X75;
    NR14_REG = 0X86;
}

void play_leveltitle_sound(void){
    NR10_REG = 0x75;
    NR11_REG = 0X86;
    NR12_REG = 0X87;
    NR13_REG = 0X75;
    NR14_REG = 0X86;
}

void play_key_sound(void){
    NR10_REG = 0x35;
    NR11_REG = 0X85;
    NR12_REG = 0X47;
    NR13_REG = 0X75;
    NR14_REG = 0X86;
}

void play_gameover_sound(void){
    NR10_REG = 0x1C;
    NR11_REG = 0X89;
    NR12_REG = 0XF7;
    NR13_REG = 0X75;
    NR14_REG = 0X86;
}

void play_wining_sound(void){
    NR10_REG = 0x37;
    NR11_REG = 0X85;
    NR12_REG = 0X1F;
    NR13_REG = 0X75;
    NR14_REG = 0X86;
}

void play_dying_sound(void){
    NR41_REG = 0X00;
    NR42_REG = 0XFB;
    NR43_REG = 0X80;
    NR44_REG = 0XC0;
}

void play_moving_sound(void){
    NR41_REG = 0X3A;
    NR42_REG = 0XA1;
    NR43_REG = 0X00;
    NR44_REG = 0XC0;
}

char get_input(uint8_t *input, uint8_t *old_input, uint8_t *move_dir_buff, char *start_ind, uint8_t *old_direction){
  char valid_input = 0;

  *old_input = *input;
  *input = joypad();
  
  /*
    J_RIGHT = 0x1
    J_LEFT  = 0x2
    J_UP    = 0x4
    J_DOWN  = 0x8
  */
  
  /*
  We only want to process the first time the button is pressed.

  To do this, we check that the old_input was not the key we want,
  and only then process the new input.
  
  I saw this concept on the Black Castle GB game github.
    https://github.com/untoxa/BlackCastle/
    include/global.h  --> KEY_TICKED(K)
  */
  if ((((*input & J_LEFT) && !(*old_input & J_LEFT)) || \
       ((*input & J_RIGHT) && !(*old_input & J_RIGHT))) && \
      ((*old_direction & 0x3) == 0)){
    // case where J_RIGHT or J_LEFT were pressed and
    // snake is not already moving RIGHT or LEFT.
    // This is valid user input.
    *start_ind = *start_ind + 1;
    *start_ind = *start_ind & 0xF;
    move_dir_buff[*start_ind] = *input & 0x3;
    *old_direction = *input & 0x3;
    valid_input |= 1;
  }
  if ((((*input & J_UP) && !(*old_input & J_UP)) || \
       ((*input & J_DOWN) && !(*old_input & J_DOWN))) && \
      ((*old_direction & 0xC) == 0)){
    // case where J_UP or J_DOWN were pressed and
    // snake is not already moving UP or DOWN.
    // This is valid user input.
    *start_ind = *start_ind + 1;
    *start_ind = *start_ind & 0xF;
    move_dir_buff[*start_ind] = *input & 0xC;
    *old_direction = *input & 0xC;
    valid_input |= 1;
  }  
  if ((*input & J_START) > 0){
    // PAUSE GAME. Resume with the old direction.
    waitpadup();
    waitpad(J_START);
    waitpadup();
  }           
  return valid_input;
}

void main(void){
  struct SnakePart* tmphead;
  font_t min_font;
  uint8_t font_tilemap_offset = 37;
  uint8_t lives_tilemap_offset;
  uint8_t progressbar_tilemap_offset;
  struct SnakePart snake_tail[SNAKE_MAX_SIZE];
  struct Sprite food_sprite;
  
  uint8_t move_direction;
  uint8_t old_direction;
  uint8_t move_dir_buff[16]; // Rolling input buffer
  char move_dir_buff_ind;
  char new_input;
  char start_ind;
  char latest_ind;

  for (char i = 0; i <= 15; i++){
    move_dir_buff[i] = 0x00;
  }

  uint8_t input;
  uint8_t old_input;
  uint8_t wait_loop_ind;
  uint8_t speed;
  uint8_t speed_factor;
  uint8_t old_speed_factor;

  uint8_t food_sprite_id = 36;
  uint8_t next_snaketail_sprite_id = 0;
  uint8_t snake_tail_ind = 0;
  uint8_t stop_play;
  uint8_t game_over;

  uint8_t score_tiles[3];
  uint8_t lives_tiles[3];
  uint8_t progressbar_tiles[10];
  uint8_t lives;
  
  struct LevelData level_data[4];
  level_data[0].tiles = level1_tiles;
  level_data[0].map = level1_map;
  level_data[0].background_colliders = level1_background_colliders;
  level_data[0].ntiles = level1_ntiles;
  level_data[0].head_startx = 80;
  level_data[0].head_starty = 80;
  level_data[0].head_dir = 1; // Up
  level_data[0].food_startx = 72;
  level_data[0].food_starty = 50;
  level_data[0].left_boundary = 4;
  level_data[0].right_boundary = 15;
  level_data[0].top_boundary = 3;
  level_data[0].bottom_boundary = 14;
  level_data[0].next_level_len = 36;
  level_data[0].start_speed = 40;
  level_data[0].speedup = 5;
  level_data[0].speed_increase_len = 10;
  level_data[0].titlescreen = level1_titlescreen;
  level_data[0].food_timer = 20;

  level_data[1].tiles = level2_tiles;
  level_data[1].map = level2_map;
  level_data[1].background_colliders = level2_background_colliders;
  level_data[1].ntiles = level2_ntiles;
  level_data[1].head_startx = 104;
  level_data[1].head_starty = 120;
  level_data[1].head_dir = 1; // Up
  level_data[1].food_startx = 48;
  level_data[1].food_starty = 88;
  level_data[1].left_boundary = 3;
  level_data[1].right_boundary = 13;
  level_data[1].top_boundary = 5;
  level_data[1].bottom_boundary = 14;
  level_data[1].next_level_len = 36;
  level_data[1].start_speed = 35;
  level_data[1].speedup = 5;
  level_data[1].speed_increase_len = 10;
  level_data[1].titlescreen = level2_titlescreen;
  level_data[1].food_timer = 20;

  level_data[2].tiles = level3_tiles;
  level_data[2].map = level3_map;
  level_data[2].background_colliders = level3_background_colliders;
  level_data[2].ntiles = level3_ntiles;
  level_data[2].head_startx = 72;
  level_data[2].head_starty = 80;
  level_data[2].head_dir = 4; // Right
  level_data[2].food_startx = 72;
  level_data[2].food_starty = 96;
  level_data[2].left_boundary = 7;
  level_data[2].right_boundary = 16;
  level_data[2].top_boundary = 3;
  level_data[2].bottom_boundary = 12;
  level_data[2].next_level_len = 36;
  level_data[2].start_speed = 30;
  level_data[2].speedup = 5;
  level_data[2].speed_increase_len = 10;
  level_data[2].titlescreen = level3_titlescreen;
  level_data[2].food_timer = 15;

  level_data[3].tiles = level4_tiles;
  level_data[3].map = level4_map;
  level_data[3].background_colliders = level4_background_colliders;
  level_data[3].ntiles = level4_ntiles;
  level_data[3].head_startx = 112;
  level_data[3].head_starty = 32;
  level_data[3].head_dir = 2; // Down
  level_data[3].food_startx = 128;
  level_data[3].food_starty = 120;
  level_data[3].left_boundary = 7;
  level_data[3].right_boundary = 16;
  level_data[3].top_boundary = 1;
  level_data[3].bottom_boundary = 14;
  level_data[3].next_level_len = 36;
  level_data[3].start_speed = 30;
  level_data[3].speedup = 5;
  level_data[3].speed_increase_len = 10;
  level_data[3].titlescreen = level4_titlescreen;
  level_data[3].food_timer = 15;

  unsigned char* level_tiles;
  unsigned char* level_map;
  unsigned char* background_colliders;
  short level_ntiles;
  char current_level;

  char tail1_xoffset;
  char tail1_yoffset;
  char tail2_xoffset;
  char tail2_yoffset;
  
  int8_t dx;
  int8_t dy;
  int8_t dx_coll;
  int8_t dy_coll;

  uint8_t debug_tiles [3] = {0x0, 0x0, 0x0};

  /* Initialize font */
  font_init();
  min_font = font_load(font_min); // 36 tiles
  font_set(min_font);

  /* Load sprite data */
  set_sprite_data(SNAKE_MEMIND, SNAKE_NTILES, snake_spritesheet_data);
  set_sprite_data(FOOD_MEMIND, FOOD_NTILES, food_spritesheet_data);
  set_sprite_tile(food_sprite_id, FOOD_BISCUIT);
  set_sprite_tile(food_sprite_id+1, FOOD_CARROT);
  set_sprite_tile(food_sprite_id+2, FOOD_TURNIP);
  set_sprite_tile(food_sprite_id+3, KEY);

  /* Load window */
  // set_win_tiles(0, 0, 6, 1, lives_label);
  // set_win_tiles(10, 0, 3, 1, score_tiles);
  // set_win_tiles(14, 0, 3, 1, debug_tiles);
  // move_win(7,136);
  move_win(7,136);

  // Turn on Sound
  NR52_REG = 0x80;  // Enable sound chip
  NR50_REG = 0x77;  // Max volume on both speakers
  NR51_REG = 0xBB;  // Enable CH1,2,4 on both speakers

  DISPLAY_ON;

  uint8_t x;

  while (1) {
    /* GAME RESET */
    initrand(DIV_REG);

    input = 0x0;
    old_input = input;
    speed_factor = 0;
    old_speed_factor = 0;
    lives = 3;

    food_sprite_id = 37;
    next_snaketail_sprite_id = 0;
    snake_tail_ind = 0;
    stop_play = 0;
    game_over = 0;
    current_level = 0;

    dx, dy = 0;
    dx_coll, dy_coll = 0;
    tail1_xoffset = tail1_yoffset = 0;
    tail2_xoffset = tail2_yoffset = 0;

    /*
      Show level title screen
    */
    fadeout();
    show_titlescreen(level_data[current_level].titlescreen);

    /* Start game */

    while (!game_over){
      /* Load Level */
      level_tiles = level_data[current_level].tiles;
      level_map = level_data[current_level].map;
      background_colliders = level_data[current_level].background_colliders;
      level_ntiles = level_data[current_level].ntiles;

      /* Load background */
      lives_tilemap_offset = font_tilemap_offset + level_ntiles;
      progressbar_tilemap_offset = lives_tilemap_offset + 1;
      set_bkg_data(font_tilemap_offset, level_ntiles, level_tiles);
      set_bkg_data(lives_tilemap_offset, 1, snake_spritesheet_data);
      set_bkg_data(progressbar_tilemap_offset, 6, progressbar_tiles_tiles);

      for (uint8_t i = 0; i < lives; i++){
        lives_tiles[i] = font_tilemap_offset+level_ntiles;
      }
      set_win_tiles(16, 0, 3, 1, lives_tiles);

      progressbar_tiles[0] = progressbar_tilemap_offset;
      for (uint8_t i = 1; i < 9; i++){
        progressbar_tiles[i] = progressbar_tilemap_offset + 1;
      }
      progressbar_tiles[9] = progressbar_tilemap_offset + 2;
      set_win_tiles(1, 0, 10, 1, progressbar_tiles);

      /* Main code */
      set_bkg_tiles(0, 0, 20, 18, level_map);

      // fadeout();
      SHOW_SPRITES;
      SHOW_BKG;
      SHOW_WIN;
      // fadein();

      next_snaketail_sprite_id = 0;
      snake_tail_ind = 0;
      move_dir_buff_ind = 0;
      new_input = 0;
      start_ind = move_dir_buff_ind;
      latest_ind = start_ind;

      /* Create food sprite */
      // Add code to randomly generate the x,y coordinates of the food. Copy to other places where we spawn new food.
      food_sprite.x = level_data[current_level].food_startx; 
      food_sprite.y = level_data[current_level].food_starty;
      food_sprite.size = FOOD_SPRITE_SIZE;
      food_sprite.spriteid = food_sprite_id;
      food_sprite.timer = level_data[current_level].food_timer;
      food_sprite.animation_frame = 40;
      food_sprite.animation_state = 0;
      move_sprite(food_sprite.spriteid, food_sprite.x, food_sprite.y);
      
      /* Create Snake head at index 0 of snake_tail array*/
      snake_tail[snake_tail_ind].sprite.x = level_data[current_level].head_startx;
      snake_tail[snake_tail_ind].sprite.y = level_data[current_level].head_starty;
      snake_tail[snake_tail_ind].sprite.size = SNAKE_SPRITE_SIZE;
      snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
      switch (level_data[current_level].head_dir) {
        case 1:
          // UP
          set_sprite_tile(next_snaketail_sprite_id, SNAKE_HEAD_UP);
          set_sprite_prop(next_snaketail_sprite_id, get_sprite_prop(0) & ~S_FLIPY);
          tail1_yoffset = SNAKE_SPRITE_SIZE;
          tail1_xoffset = 0;
          tail2_yoffset = 2 * SNAKE_SPRITE_SIZE;
          tail2_xoffset = 0;
          move_dir_buff[move_dir_buff_ind] = J_UP;
          break;

        case 2:
          // DOWN
          set_sprite_tile(next_snaketail_sprite_id, SNAKE_HEAD_UP);
          set_sprite_prop(next_snaketail_sprite_id, S_FLIPY);
          tail1_yoffset = -SNAKE_SPRITE_SIZE;
          tail1_xoffset = 0;
          tail2_yoffset = -2 * SNAKE_SPRITE_SIZE;
          tail2_xoffset = 0;
          move_dir_buff[move_dir_buff_ind] = J_DOWN;
          break;

        case 3:
          // LEFT
          set_sprite_tile(next_snaketail_sprite_id, SNAKE_HEAD_L);
          set_sprite_prop(next_snaketail_sprite_id, get_sprite_prop(0) & ~S_FLIPX);
          tail1_yoffset = 0;
          tail1_xoffset = SNAKE_SPRITE_SIZE;
          tail2_yoffset = 0;
          tail2_xoffset = 2 * SNAKE_SPRITE_SIZE;
          move_dir_buff[move_dir_buff_ind] = J_LEFT;
          break;

        case 4:
          // RIGHT
          set_sprite_tile(next_snaketail_sprite_id, SNAKE_HEAD_L);
          set_sprite_prop(next_snaketail_sprite_id, S_FLIPX);
          tail1_yoffset = 0;
          tail1_xoffset = -SNAKE_SPRITE_SIZE;
          tail2_yoffset = 0;
          tail2_xoffset = -2 * SNAKE_SPRITE_SIZE;
          move_dir_buff[move_dir_buff_ind] = J_RIGHT;
          break;

        default:
          break;
      }
      move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
      next_snaketail_sprite_id++;
      snake_tail_ind++;

      /* Create snake tail */
      snake_tail[snake_tail_ind].sprite.x = level_data[current_level].head_startx + tail1_xoffset;
      snake_tail[snake_tail_ind].sprite.y = level_data[current_level].head_starty + tail1_yoffset;
      snake_tail[snake_tail_ind].sprite.size = SNAKE_SPRITE_SIZE;
      snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
      set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
      move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
      snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
      next_snaketail_sprite_id++;
      snake_tail_ind++;   
      
      snake_tail[snake_tail_ind].sprite.x = level_data[current_level].head_startx + tail2_xoffset;
      snake_tail[snake_tail_ind].sprite.y = level_data[current_level].head_starty + tail2_yoffset;
      snake_tail[snake_tail_ind].sprite.size = SNAKE_SPRITE_SIZE;
      snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
      set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
      move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
      snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
      snake_tail[snake_tail_ind].next = NULL;
      next_snaketail_sprite_id++;
      snake_tail_ind++;

      speed = level_data[current_level].start_speed;
      speed_factor = 0;
      old_speed_factor = 0;

      while(!stop_play){
        new_input = 0;
        move_direction = move_dir_buff[move_dir_buff_ind];
        old_direction = move_direction;
        start_ind = move_dir_buff_ind;

        if ((move_direction & J_UP) == J_UP){
          debug_tiles[0] = 0x1 + 0x1; 
          dx = 0;
          dy = -8;
          dx_coll = 0;
          dy_coll = dy + 4;
        }                     
        else if ((move_direction & J_DOWN) == J_DOWN){
          debug_tiles[0] = 0x2 + 0x1; 
          dx = 0;
          dy = 8;
          dx_coll = 0;
          dy_coll = dy + 4;
        }
        else if (((move_direction & J_LEFT) == J_LEFT)){
          debug_tiles[0] = 0x3 + 0x1; 
          dx = -8;
          dy = 0;
          dx_coll = dx + 4;
          dy_coll = 0;
        }
        else if (((move_direction & J_RIGHT) == J_RIGHT)){
          debug_tiles[0] = 0x4 + 0x1; 
          dx = 8;
          dy = 0;
          dx_coll = dx + 4;
          dy_coll = 0;
        }
        else {
          dx = 0;
          dy = 0;
          dx_coll = 0;
          dy_coll = 0;
        }

        new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 
        
        debug_tiles[2] = move_dir_buff_ind + 1;
        // set_win_tiles(14, 0, 3, 1, debug_tiles);
        // Check collision with background obstacles    
        if (background_collision(snake_tail[0].sprite.x+dx_coll, snake_tail[0].sprite.y+dy_coll, background_colliders, debug_tiles)){
          stop_play = 1;
          play_dying_sound();
          move_tail(&snake_tail[0], snake_tail[0].sprite.x, snake_tail[0].sprite.y);
        }
        else {
          new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 
          play_moving_sound();
          move_snake(&snake_tail[0], dx, dy);
          if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
            new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 
            if (food_sprite.spriteid == 39){
              // play_key_sound();
              play_eating_sound();
            }
            else {
              play_eating_sound();
            }
            new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 
            if (snake_tail_ind < level_data[current_level].next_level_len) {
              snake_tail[snake_tail_ind].sprite.x = snake_tail[snake_tail_ind-1].sprite.x;
              snake_tail[snake_tail_ind].sprite.y = snake_tail[snake_tail_ind-1].sprite.y;
              snake_tail[snake_tail_ind].sprite.size = snake_tail[snake_tail_ind-1].sprite.size;
              snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
              set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
              move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
              snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
              snake_tail[snake_tail_ind].next = NULL;
              snake_tail_ind++;
              next_snaketail_sprite_id++;
              movefood(&food_sprite, &snake_tail[0], background_colliders, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 0, &level_data[current_level]);
            }                    
            else {
              snake_tail_ind++;
            }
          } 
          else if (head_tail_collision(snake_tail)){
            // Collided head with tail. End Game
            play_dying_sound();
            stop_play = 1;
          }  
        }

        // score2tile(speed, score_tiles);  // DEBUG ONLY, REMOVE!
        // set_win_tiles(10,0, 3, 1, score_tiles);
        
        switch (snake_tail_ind)
        {
          case 4:
            progressbar_tiles[0] = progressbar_tilemap_offset + 3;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          case 8:
            progressbar_tiles[1] = progressbar_tilemap_offset + 4;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          case 12:
            progressbar_tiles[2] = progressbar_tilemap_offset + 4;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          case 16:
            progressbar_tiles[3] = progressbar_tilemap_offset + 4;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          case 20:
            progressbar_tiles[4] = progressbar_tilemap_offset + 4;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          case 24:
            progressbar_tiles[5] = progressbar_tilemap_offset + 4;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          case 28:
            progressbar_tiles[6] = progressbar_tilemap_offset + 4;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          case 32:
            progressbar_tiles[7] = progressbar_tilemap_offset + 4;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          case 36:
            progressbar_tiles[8] = progressbar_tilemap_offset + 4;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          case 40:
            progressbar_tiles[9] = progressbar_tilemap_offset + 5;
            set_win_tiles(1, 0, 10, 1, progressbar_tiles);
            break;
          
          default:
            break;
        }        

        new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 
        if (((snake_tail_ind) == level_data[current_level].next_level_len) && (food_sprite.spriteid != 39)){
          // Deploy key
          play_key_sound();
          movefood(&food_sprite, &snake_tail[0], background_colliders, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 1, &level_data[current_level]);
        }
        else if ((snake_tail_ind) > level_data[current_level].next_level_len){
          // Grabbed key 
          if (current_level == 3){
            // WIN THE GAME
            stop_play = 3;
          }
          else {
            // WIN THE LEVEL
            stop_play = 2;
          }
        }
        else {
          // Increase speed when the snake length reaches a multiple of LevelData.speedup
          speed_factor = ((int) (snake_tail_ind) / level_data[current_level].speed_increase_len);
          if (speed_factor != old_speed_factor) {
            speed = speed - level_data[current_level].speedup;
            old_speed_factor = speed_factor;
          }
        }
        
        // Decrement food timer
        if (food_sprite.timer > 0){
          food_sprite.timer--;
        }

        // old_direction = move_direction;
        // start_ind = move_dir_buff_ind;
        for (wait_loop_ind = 0; wait_loop_ind < speed; wait_loop_ind++){
          new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 

          // When timer expires, start food blinking animation
          if (food_sprite.timer == 0) {
            switch (food_sprite.animation_state){
              case 0:
                // Hide
                move_sprite(food_sprite.spriteid, 0, 0);
                food_sprite.animation_state = 1;
                break;
              case 1:
                // Show
                move_sprite(food_sprite.spriteid, food_sprite.x, food_sprite.y);
                food_sprite.animation_state = 0;
                break;
            }
            food_sprite.animation_frame--;
          }
          // When animation frames are done, respawn food
          if (food_sprite.animation_frame == 0){
            movefood(&food_sprite, &snake_tail[0], background_colliders, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 0, &level_data[current_level]);
          }
          
          // Wait until the Vertical Blanking is done
          wait_vbl_done();
        }
        if (new_input == 1){
          latest_ind = start_ind;
          move_dir_buff_ind++;
          move_dir_buff_ind = move_dir_buff_ind & 0xF;
        }
        else if (latest_ind != move_dir_buff_ind){
          move_dir_buff_ind++;
          move_dir_buff_ind = move_dir_buff_ind & 0xF;
        }
      // End level loop (while (!stop_play){})
      }

      if (stop_play == 1) {
        flash_sprites();
        lives -= 1;
        stop_play = 0;
      }
      else if (stop_play == 2){
        stop_play = 0;
        current_level++;
        fadeout();
        show_titlescreen(level_data[current_level].titlescreen);
      }

      if (lives == 0){
        game_over = 1;
      }
      else if (stop_play == 3){
        // Should not be necessary but forcing the loop exit when the game is won
        game_over = 0;
        break;
      }
      else {
        speed = level_data[current_level].start_speed;
        move_dir_buff[move_dir_buff_ind] = J_UP;
        lives_tiles[lives] = 0x0;
        set_win_tiles(16, 0, 3, 1, lives_tiles);
    
        /* Reset all variables */
        move_sprite(food_sprite.spriteid, 0, 0);
        tmphead = snake_tail;
        while(tmphead != NULL) {
          move_sprite(tmphead->sprite.spriteid, 0, 0);
          tmphead = tmphead->next;
        }
      }
    // End Game loop (while (!game_over){})
    }

    if (game_over) {
      fadeout();
      show_finalscreen(gameover_titlescreen, 0);
    }
    else if (stop_play == 3){
      fadeout();
      show_finalscreen(win_titlescreen, 1);
    }
    HIDE_BKG;
    HIDE_WIN;

    /* Reset all variables */
    move_sprite(food_sprite.spriteid, 0, 0);
    tmphead = snake_tail;
    while(tmphead != NULL) {
      move_sprite(tmphead->sprite.spriteid, 0, 0);
      tmphead = tmphead->next;
    }

    /* 
    End the program: 
    REMOVE TO ALLOW FOR THE GAME TO RESTART
    ONCE THE MAP CORRUPTION IS DEBUGGED.
    */
    show_finalscreen(restart_titlescreen, 2);
    SHOW_BKG;
    break;

  // End system loop (while (1){})
  }
}
