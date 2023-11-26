#include <gb/gb.h>
#include <gbdk/font.h>
#include <stdint.h>
#include <stdio.h>
#include <rand.h>

#include "Sprite.h"
#include "food_spritesheet_py.h"
#include "snake_spritesheet_py.h"
#include "windowmap.h"

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

#define SNAKE_MAX_SIZE 37

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
  unsigned char speedup;
  short speed_increase_len;
};

void wait(uint8_t n){
  // Interrupt-based delay.
  // Returns after n Vertical Blanking interrupts (screen refreshes)
  uint8_t x;
  for (x = 0; x < n; x++){
    wait_vbl_done();
  }
}

void fadeout(){
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

void fadein(){
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

void flash_sprites(){
    uint8_t y;

    for (y=0; y<3; y++){
      HIDE_SPRITES;
      wait(10);
      SHOW_SPRITES;
      wait(10);
    }
}

void game_over_screen(uint8_t score){
  uint8_t x;

  HIDE_SPRITES;
  HIDE_WIN;
  
  for (x = 0; x < 8; x++){
    printf("                    ");
  }
  printf("     GAME OVER      ");
  printf("      SCORE %d      ", score);
  for (x =0; x < 8; x++){
    printf("                    ");
  }
  waitpad(J_START);
  waitpadup();
}

void win_screen(uint8_t score){
  uint8_t x;

  HIDE_SPRITES;
  HIDE_WIN;

  for (x = 0; x < 8; x++){
    printf("                    ");
  }
  printf("     YOU WIN!       ");
  printf("     SCORE %d       ", score);
  for (x =0; x < 8; x++){
    printf("                    ");
  }
  waitpad(J_START);
  waitpadup();
}

UBYTE sprite_collision(struct Sprite* sp1, struct Sprite* sp2){
  /* This code peforms collision between the centers of sp1 with sp2 */
  uint8_t sp1_left = sp1->x + sp1->size/2;
  uint8_t sp1_right = sp1->x + sp1->size - sp1->size/2;
  uint8_t sp1_top = sp1->y + sp1->size/2;
  uint8_t sp1_bottom = sp1->y + sp1->size;
  uint8_t sp2_left = sp2->x;
  uint8_t sp2_right = sp2->x + sp2->size;
  uint8_t sp2_top = sp2->y;
  uint8_t sp2_bottom = sp2->y + sp2->size;

  return (((sp1_left >= sp2_left) && (sp1_left <= sp2_right)) && ((sp1_top >= sp2_top) && (sp1_top <= sp2_bottom))) || (((sp2_left >= sp1_left) && (sp2_left <= sp1_right)) && ((sp2_top >= sp1_top) && (sp2_top <= sp1_bottom)));
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
  set_win_tiles(10, 0, 2, 1, debug_tiles);

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

  // // Check collision with the screen borders
  // if (newx >= (SCREEN_LEFT + SCREEN_WIDTH)){
    // newx = SCREEN_LEFT + SCREEN_WIDTH;
  // }
  // else if (newx <= SCREEN_LEFT){
    // newx = SCREEN_LEFT + head->sprite.size;
  // }

  // if (newy >= (SCREEN_TOP + SCREEN_HEIGHT)){
    // newy = SCREEN_TOP + SCREEN_HEIGHT;
  // }
  // else if (newy <= SCREEN_TOP){
    // newy = SCREEN_TOP + head->sprite.size;
  // }

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

void movefood(struct Sprite* food, char* bkg_colliders, short lbound, short rbound, short tbound, short bbound, uint8_t* debug_tiles, unsigned char key){
    initrand(DIV_REG);
    /* This function places a new food item at the location of the snake's tail tip */
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
    } while (background_collision(randx, randy, bkg_colliders, debug_tiles));

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


void main(){
  struct SnakePart* tmphead;
  font_t min_font;
  uint8_t font_tilemap_offset = 37;
  struct SnakePart snake_tail[SNAKE_MAX_SIZE];
  struct Sprite food_sprite;
  
  uint8_t move_direction;
  uint8_t old_direction;
  uint8_t input;
  uint8_t wait_loop_ind;
  uint8_t speed;
  uint8_t speed_factor;
  uint8_t old_speed_factor;

  uint8_t food_sprite_id = 36;
  uint8_t next_snaketail_sprite_id = 0;
  uint8_t snake_tail_ind = 0;
  uint8_t stop_play;
  uint8_t game_over;
  uint8_t score;
  uint8_t score_increment = 1;

  uint8_t score_tiles[3];
  uint8_t lives_tiles[3];
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
  level_data[0].next_level_len = 37;
  level_data[0].speedup = 5;
  level_data[0].speed_increase_len = 10;

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
  level_data[1].next_level_len = 37;
  level_data[1].speedup = 5;
  level_data[1].speed_increase_len = 8;

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
  level_data[2].next_level_len = 37;
  level_data[2].speedup = 5;
  level_data[2].speed_increase_len = 5;

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
  level_data[3].next_level_len = 37;
  level_data[3].speedup = 5;
  level_data[3].speed_increase_len = 3;

  unsigned char* level_tiles;
  unsigned char* level_map;
  unsigned char* background_colliders;
  short level_ntiles;
  char current_level;

  char tail1_xoffset;
  char tail1_yoffset;
  char tail2_xoffset;
  char tail2_yoffset;
  
  uint8_t debug_tiles [2] = {0x0, 0x0};

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
  set_win_tiles(0, 0, 6, 1, scoremap);
  set_win_tiles(6, 0, 3, 1, score_tiles);
  set_win_tiles(10, 0, 2, 1, debug_tiles);
  move_win(7,136);

  // SHOW_BKG;
  // SHOW_WIN;
  // SHOW_SPRITES;
  // DISPLAY_ON;

  current_level = 0;

  DISPLAY_ON;

  uint8_t x;

  while (1) {
    /* GAME RESET */
    initrand(DIV_REG);

    input = 0x0;
    speed = 40;
    speed_factor = 0;
    old_speed_factor = 0;
    score = 0;
    lives = 3;

    food_sprite_id = 37;
    next_snaketail_sprite_id = 0;
    snake_tail_ind = 0;
    stop_play = 0;
    game_over = 0;
    current_level = 0;
    score_increment = 1;
    move_direction = J_UP;

    fadeout();

    HIDE_SPRITES;
    HIDE_WIN;

    for (x = 0; x < 9; x++){
      printf("                    ");
    }
    printf("     LEVEL %d        ", current_level+1);
    for (x =0; x < 8; x++){
      printf("                    ");
    }
    SHOW_BKG;
    fadein();
    waitpad(J_START);
    waitpadup();

    HIDE_BKG;

    while (!game_over){
      /* Load Level */

      level_tiles = level_data[current_level].tiles;
      level_map = level_data[current_level].map;
      background_colliders = level_data[current_level].background_colliders;
      level_ntiles = level_data[current_level].ntiles;

      /* Load background */
      set_bkg_data(font_tilemap_offset, level_ntiles, level_tiles);
      set_bkg_data(font_tilemap_offset+level_ntiles, 1, snake_spritesheet_data);

      for (char i = 0; i < lives; i++){
        lives_tiles[i] = font_tilemap_offset+level_ntiles;
      }

      set_win_tiles(16, 0, 3, 1, lives_tiles);

      /* Main code */
      set_bkg_tiles(0, 0, 20, 18, level_map);
      score2tile(score, score_tiles);

      fadeout();
      SHOW_SPRITES;
      SHOW_BKG;
      SHOW_WIN;
      fadein();

      next_snaketail_sprite_id = 0;
      snake_tail_ind = 0;

      /* Create food sprite */
      // Add code to randomly generate the x,y coordinates of the food. Copy to other places where we spawn new food.
      food_sprite.x = level_data[current_level].food_startx; 
      food_sprite.y = level_data[current_level].food_starty;
      food_sprite.size = FOOD_SPRITE_SIZE;
      food_sprite.spriteid = food_sprite_id;
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
          move_direction = J_UP;
          break;

        case 2:
          // DOWN
          set_sprite_tile(next_snaketail_sprite_id, SNAKE_HEAD_UP);
          set_sprite_prop(next_snaketail_sprite_id, S_FLIPY);
          tail1_yoffset = -SNAKE_SPRITE_SIZE;
          tail1_xoffset = 0;
          tail2_yoffset = -2 * SNAKE_SPRITE_SIZE;
          tail2_xoffset = 0;
          move_direction = J_DOWN;
          break;

        case 3:
          // LEFT
          set_sprite_tile(next_snaketail_sprite_id, SNAKE_HEAD_L);
          set_sprite_prop(next_snaketail_sprite_id, get_sprite_prop(0) & ~S_FLIPX);
          tail1_yoffset = 0;
          tail1_xoffset = SNAKE_SPRITE_SIZE;
          tail2_yoffset = 0;
          tail2_xoffset = 2 * SNAKE_SPRITE_SIZE;
          move_direction = J_LEFT;
          break;

        case 4:
          // RIGHT
          set_sprite_tile(next_snaketail_sprite_id, SNAKE_HEAD_L);
          set_sprite_prop(next_snaketail_sprite_id, S_FLIPX);
          tail1_yoffset = 0;
          tail1_xoffset = -SNAKE_SPRITE_SIZE;
          tail2_yoffset = 0;
          tail2_xoffset = -2 * SNAKE_SPRITE_SIZE;
          move_direction = J_RIGHT;
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

      speed = 40;
      speed_factor = 0;
      old_speed_factor = 0;

      while(!stop_play){
        if ((move_direction & J_UP) == J_UP){
          debug_tiles[0] = 0x1 + 0x1; 
          set_win_tiles(10, 0, 2, 1, debug_tiles);
          // Check collision with background obstacles    
          if (background_collision(snake_tail[0].sprite.x, snake_tail[0].sprite.y-4, background_colliders, debug_tiles)){
            stop_play = 1;
            move_tail(&snake_tail[0], snake_tail[0].sprite.x, snake_tail[0].sprite.y);
          }
          else {
            move_snake(&snake_tail[0], 0, -8);
            if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
              if (snake_tail_ind < level_data[current_level].next_level_len) {
                movefood(&food_sprite, background_colliders, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 0);
                score = score + score_increment;
                snake_tail[snake_tail_ind].sprite.x = snake_tail[snake_tail_ind-1].sprite.x;
                snake_tail[snake_tail_ind].sprite.y = snake_tail[snake_tail_ind-1].sprite.y;
                snake_tail[snake_tail_ind].sprite.size = snake_tail[snake_tail_ind-1].sprite.size;
                snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
                set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
                move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
                snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
                snake_tail[snake_tail_ind].next = NULL;
                next_snaketail_sprite_id++;
                snake_tail_ind++;
              }                    
              else {
                snake_tail_ind++;
              }
            } 
            // else if (head_tail_collision(&snake_tail)){
            else if (head_tail_collision(snake_tail)){
              // Collided head with tail. End Game
              stop_play = 1;
            }  
          }
        }                     
        else if ((move_direction & J_DOWN) == J_DOWN){
          debug_tiles[0] = 0x2 + 0x1; 
          set_win_tiles(10, 0, 2, 1, debug_tiles);
          if (background_collision(snake_tail[0].sprite.x, snake_tail[0].sprite.y+12, background_colliders, debug_tiles)){
            stop_play = 1;
            move_tail(&snake_tail[0], snake_tail[0].sprite.x, snake_tail[0].sprite.y);
          }
          else {
            move_snake(&snake_tail[0], 0, 8);
            if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
              if (snake_tail_ind < level_data[current_level].next_level_len) {
                movefood(&food_sprite, background_colliders, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 0); 
                score = score + score_increment;
                snake_tail[snake_tail_ind].sprite.x = snake_tail[snake_tail_ind-1].sprite.x;
                snake_tail[snake_tail_ind].sprite.y = snake_tail[snake_tail_ind-1].sprite.y;
                snake_tail[snake_tail_ind].sprite.size = snake_tail[snake_tail_ind-1].sprite.size;
                snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
                set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
                move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
                snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
                snake_tail[snake_tail_ind].next = NULL;
                next_snaketail_sprite_id++;
                snake_tail_ind++;
              }
              else {
                snake_tail_ind++;
              }
            }
            // else if (head_tail_collision(&snake_tail)){
            else if (head_tail_collision(snake_tail)){
              // Collided head with tail. End Game
              stop_play = 1;
            }
          }
        }
        else if (((move_direction & J_LEFT) == J_LEFT)){
          debug_tiles[0] = 0x3 + 0x1; 
          set_win_tiles(10, 0, 2, 1, debug_tiles);
          if (background_collision(snake_tail[0].sprite.x-4, snake_tail[0].sprite.y, background_colliders, debug_tiles)){
            stop_play = 1;
            move_tail(&snake_tail[0], snake_tail[0].sprite.x, snake_tail[0].sprite.y);
          }
          else {
            move_snake(&snake_tail[0], -8, 0);
            if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
              // Ate food. Increment tail.
              if (snake_tail_ind < level_data[current_level].next_level_len) {
                movefood(&food_sprite, background_colliders, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 0);
                score = score + score_increment;
                snake_tail[snake_tail_ind].sprite.x = snake_tail[snake_tail_ind-1].sprite.x;
                snake_tail[snake_tail_ind].sprite.y = snake_tail[snake_tail_ind-1].sprite.y;
                snake_tail[snake_tail_ind].sprite.size = snake_tail[snake_tail_ind-1].sprite.size;
                snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
                set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
                move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
                snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
                snake_tail[snake_tail_ind].next = NULL;
                next_snaketail_sprite_id++;
                snake_tail_ind++;
              }
              else {
                snake_tail_ind++;
              }
            }
            // else if (head_tail_collision(&snake_tail)){
            else if (head_tail_collision(snake_tail)){
              // Collided head with tail. End Game
              stop_play = 1;
            }
          }                  
        }
        else if (((move_direction & J_RIGHT) == J_RIGHT)){
          debug_tiles[0] = 0x4 + 0x1; 
          set_win_tiles(10, 0, 2, 1, debug_tiles);
          if (background_collision(snake_tail[0].sprite.x+12, snake_tail[0].sprite.y, background_colliders, debug_tiles)){
            stop_play = 1;
            move_tail(&snake_tail[0], snake_tail[0].sprite.x, snake_tail[0].sprite.y);
          }
          else {
            move_snake(&snake_tail[0], 8, 0);
            if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
              if (snake_tail_ind < level_data[current_level].next_level_len) {
                movefood(&food_sprite, background_colliders, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 0);
                score = score + score_increment;
                snake_tail[snake_tail_ind].sprite.x = snake_tail[snake_tail_ind-1].sprite.x;
                snake_tail[snake_tail_ind].sprite.y = snake_tail[snake_tail_ind-1].sprite.y;
                snake_tail[snake_tail_ind].sprite.size = snake_tail[snake_tail_ind-1].sprite.size;
                snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
                set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
                move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
                snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
                snake_tail[snake_tail_ind].next = NULL;
                next_snaketail_sprite_id++;
                snake_tail_ind++;
              }
              else {
                snake_tail_ind++;
              }
            }
            // else if (head_tail_collision(&snake_tail)){
            else if (head_tail_collision(snake_tail)){
              // Collided head with tail. End Game
              stop_play = 1;
            }
          }
        }
        score_increment = 1;
        //score2tile(score, score_tiles);
        score2tile(speed, score_tiles);  // DEBUG ONLY, REMOVE!
        set_win_tiles(6,0, 3, 1, score_tiles);
        
        if (((snake_tail_ind) == level_data[current_level].next_level_len) && (food_sprite.spriteid != 39)){
          // Deploy key
          movefood(&food_sprite, background_colliders, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 1);
        }
        else if ((snake_tail_ind) > level_data[current_level].next_level_len){
          // Grabbed key 
          if (current_level == 3){
            // WIN THE GAME
            stop_play = 3;
          }
          else {
            // WIN THE LEVEL
            current_level++;
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
        
        old_direction = move_direction;
        for (wait_loop_ind = 0; wait_loop_ind < speed; wait_loop_ind++){
          input = joypad();
          
          /*
            J_RIGHT = 0x1
            J_LEFT  = 0x2
            J_UP    = 0x4
            J_DOWN  = 0x8
          */
          if (((input & 0x3) > 0) && ((old_direction & 0x3) == 0)){
            // case where J_RIGHT or J_LEFT were pressed and
            // snake is not already moving RIGHT or LEFT.
            // This is valid user input.
            move_direction = input;
          }
          else if (((input & 0xC) > 0) && ((old_direction & 0xC) == 0)){
            // case where J_UP or J_DOWN were pressed and
            // snake is not already moving UP or DOWN.
            // This is valid user input.
            move_direction = input;
          }  
          else if ((input & J_START) > 0){
            // use logic to pause game
            move_direction = input;
          }           
          wait_vbl_done();
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
        fadeout();

        HIDE_SPRITES;
        HIDE_WIN;

        for (x = 0; x < 9; x++){
          printf("                    ");
        }
        printf("     LEVEL %d        ", current_level+1);
        for (x =0; x < 8; x++){
          printf("                    ");
        }
        SHOW_BKG;
        fadein();
        waitpad(J_START);
        waitpadup();

        HIDE_BKG;
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
        speed = 40;
        move_direction = J_UP;
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
      game_over_screen(score);
    }
    else if (stop_play == 3){
      win_screen(score);
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
  // End system loop (while (1){})
  }
}
