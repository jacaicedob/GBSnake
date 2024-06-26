#include <gb/gb.h>
#include <gbdk/font.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <rand.h>

#include "Sprite.h"
#include "food_spritesheet_tiles_py.h"
#include "snake_spritesheet_py.h"
#include "progressbar_tiles_tiles_py.h"
#include "windowmap.h"

#include "data.h"

#define SNAKE_MAX_SIZE 40
#define FONT_TILE_OFFSET 37

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

uint8_t wait(uint8_t n){
  // Interrupt-based delay.
  // Returns after n Vertical Blanking interrupts (screen refreshes)
  uint8_t input = 0;
  for (uint8_t x = 0; x < n; x++){
    input |= joypad();
    wait_vbl_done();
  };

  return input;
}

void play_eating_sound(void){
  // Stop Channel before playing FX
  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play FX
  NR10_REG = 0x37;
  NR11_REG = 0X85;
  NR12_REG = 0XF1;
  NR13_REG = 0X75;
  NR14_REG = 0X86;
}

void play_leveltitle_sound(void){
  // Stop Channel before playing FX
  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play FX
  NR10_REG = 0x75;
  NR11_REG = 0X86;
  NR12_REG = 0XF7;
  NR13_REG = 0X75;
  NR14_REG = 0X86;
}

void play_key_sound(void){
  // Stop Channel before playing FX
  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play FX
  NR10_REG = 0x35;
  NR11_REG = 0X85;
  NR12_REG = 0XF7;
  NR13_REG = 0X75;
  NR14_REG = 0X86;
}

void play_gameover_sound(void){
  // Stop Channel before playing FX
  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play FX
  NR10_REG = 0x1C;
  NR11_REG = 0X89;
  NR12_REG = 0XF7;
  NR13_REG = 0X75;
  NR14_REG = 0X86;
}

void play_wining_sound(void){
  // Stop Channel before playing FX
  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play FX
  NR10_REG = 0x37;
  NR11_REG = 0X85;
  NR12_REG = 0X5F;
  NR13_REG = 0X75;
  NR14_REG = 0X86;
}

void play_dying_sound(void){
  // Stop Channel before playing FX
  NR42_REG = 0x0;
  NR44_REG = 0x0;

  // Play FX
  NR41_REG = 0X00;
  NR42_REG = 0XFB;
  NR43_REG = 0X80;
  NR44_REG = 0XC0;
}

void play_moving_sound(void){
  // Stop Channel before playing FX
  NR42_REG = 0x0;
  NR44_REG = 0x0;

  // Play FX
  NR41_REG = 0X3A;
  NR42_REG = 0XA1;
  NR43_REG = 0X00;
  NR44_REG = 0XC0;
}

uint8_t calculate_tempo(uint8_t snake_len){
  /* 
    After some experimentation, i realized that using some form 
    of equation to calculate the tempo as a ratio of the speed and the
    desired bpm was causing the music to get too fast too quickly and 
    also getting out of sync with the snake movement sounds.

    I decided to use set values and using the tracker, i found that
    if each eighth note is played every 10-15 frames, the music is not
    too fast. 
  */
  if (snake_len < 8){
    return 15;
  }
  else if (snake_len < 16){
    return 14;
  }
  else if (snake_len < 24){
    return 13;
  }
  else if (snake_len < 32){
    return 12;
  }
  else if (snake_len < 36){
    return 11;
  }
  else {
    return 10;
  }
}

void play_music(uint8_t *music_ind, uint8_t *frame_counter, uint8_t *bass_music, uint8_t *lead_music){
  // CH1 plays lead, CH2 plays bass
  if ((frequencies[lead_music[*music_ind]] != 0) && (*frame_counter == 0)) {
    NR10_REG = 0x0;
    NR11_REG = 0x41;
    NR12_REG = 0xA7;
    NR13_REG = (frequencies[lead_music[*music_ind]] & 0xFF);
    NR14_REG = (frequencies[lead_music[*music_ind]] >> 8) | (1 << 7); // Upper nibble
  }
    
  if ((frequencies[bass_music[*music_ind]] != 0) && (*frame_counter == 0)) {
    NR21_REG = 0x40;
    NR22_REG = 0x94;
    NR23_REG = (frequencies[bass_music[*music_ind]] & 0xFF);
    NR24_REG = (frequencies[bass_music[*music_ind]] >> 8) | (1 << 7); // Upper nibble
  }
}

void stop_music(void){
  NR12_REG = 0x0;
  NR14_REG = 0x0;

  NR22_REG = 0x0;
  NR24_REG = 0x0;

  NR42_REG = 0x0;
  NR44_REG = 0x0;
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

  waitpad(J_START | J_A | J_B);
  waitpadup();
  HIDE_BKG;
}

void show_game_titlescreen(unsigned char* titlescreen){
  // This plays the music at 120 bpm (change notes every 15 frames)
  uint16_t music_ind = 0;
  uint8_t frame_counter = 0;
  uint8_t input;

  HIDE_SPRITES;
  HIDE_WIN;
  
  set_bkg_tiles(0, 0, 20, 18, titlescreen);

  SHOW_BKG;
  
  // Scroll background so it looks it comes down from the top of the screen
  // move_bkg(0, 72);
  fadein();

  SWITCH_ROM(2);
  while (frequencies[bass_intro_music[music_ind]] != 65535){
    play_music(&music_ind, &frame_counter, bass_intro_music, lead_intro_music);
    input = joypad();
    input = joypad();
    input = joypad();

    if (input & J_START) {
      HIDE_BKG;
      stop_music();
      wait_vbl_done();
      return;
    }
    frame_counter++;
    if (frame_counter == 15){
      frame_counter = 0;
      music_ind++;
    }
    wait_vbl_done();
  }

  music_ind = 0;
  frame_counter = 0;

  while (1){
    if (frequencies[bass_main_music[music_ind]] != 65535){ 
      play_music(&music_ind, &frame_counter, bass_main_music, lead_main_music);
      input = joypad();
      input = joypad();
      input = joypad();

      if (input & J_START) {
        HIDE_BKG;
        stop_music();
        wait_vbl_done();
        return;
      }
      frame_counter++;
      if (frame_counter == 15){
        frame_counter = 0;
        music_ind++;
      }
      wait_vbl_done();
    }
    else {
      music_ind = 0;
      frame_counter = 0;
    }
  }
}

void show_titlescreen(unsigned char* titlescreen){
  HIDE_SPRITES;
  HIDE_WIN;
  
  set_bkg_tiles(0, 0, 20, 18, titlescreen);
  move_bkg(0,0);

  SHOW_BKG;

  fadein();
  
  // // Scroll background so it looks it comes down from the top of the screen
  // move_bkg(0, 72);
  // fadein();

  // play_leveltitle_sound();
  // for (int i = 72; i >= 0; i--){
    // move_bkg(0, i);
    // wait_vbl_done();
  // }

  waitpad(J_START | J_A | J_B);
  waitpadup();
  stop_music();
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
  uint16_t tileind;
  uint8_t stride = 20;

  uint8_t x_tile;
  uint8_t y_tile;

  x_tile = (x-8) / 8;
  y_tile = (y-16) / 8;

  tileind = y_tile*stride + x_tile;
  debug_tiles[1] = bkg_colliders[tileind] + 0x1;
  // set_win_tiles(14, 0, 3, 1, debug_tiles);

  return bkg_colliders[tileind];

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

void placefood(uint8_t spriteid, uint8_t x, uint8_t y, unsigned char *map, unsigned char *background_colliders, uint8_t food_tilemap_offset){
  uint16_t tileind;
  uint8_t stride = 20;
  uint8_t x_tile;
  uint8_t y_tile;

  x_tile = (x-8) / 8;
  y_tile = (y-16) / 8;
  tileind = y_tile*stride + x_tile;

  map[tileind] = food_tilemap_offset + spriteid - FONT_TILE_OFFSET;
  background_colliders[tileind] = 2;
  set_bkg_based_tiles(0, 0, 20, 18, map, FONT_TILE_OFFSET);
}

void replacefood(uint8_t spriteid, uint8_t old_x, uint8_t old_y, uint8_t x, uint8_t y, unsigned char *map, unsigned char *background_colliders, uint8_t food_tilemap_offset){
  uint16_t tileind;
  uint8_t stride = 20;
  uint8_t x_tile;
  uint8_t y_tile;

  // New location
  x_tile = (x-8) / 8;
  y_tile = (y-16) / 8;
  tileind = y_tile*stride + x_tile;

  map[tileind] = food_tilemap_offset + spriteid - FONT_TILE_OFFSET;
  background_colliders[tileind] = 2;
  
  // Old location
  x_tile = (old_x-8) / 8;
  y_tile = (old_y-16) / 8;
  tileind = y_tile*stride + x_tile;

  map[tileind] = 256 - FONT_TILE_OFFSET;  // When adding FONT_TILE_OFFSET later, this should overflow to 0
  background_colliders[tileind] = 0;
  
  // Update map
  set_bkg_based_tiles(0, 0, 20, 18, map, FONT_TILE_OFFSET);
}

void removefood(uint8_t x, uint8_t y, unsigned char *map, unsigned char *background_colliders){
  uint16_t tileind;
  uint8_t stride = 20;
  uint8_t x_tile;
  uint8_t y_tile;

  x_tile = (x-8) / 8;
  y_tile = (y-16) / 8;
  tileind = y_tile*stride + x_tile;

  map[tileind] = 256 - FONT_TILE_OFFSET;  // When adding FONT_TILE_OFFSET later, this should overflow to 0
  background_colliders[tileind] = 0;
  set_bkg_based_tiles(0, 0, 20, 18, map, FONT_TILE_OFFSET);
}

void movefood(struct Sprite* food, struct SnakePart* head, unsigned char* bkg_colliders, unsigned char* map, unsigned char lbound, unsigned char rbound, unsigned char tbound, unsigned char bbound, uint8_t* debug_tiles, unsigned char key, uint8_t food_tilemap_offset, char food_timer){
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
      new_spriteid = 3;
    }
    else{
      new_spriteid = food->spriteid+1;
      if (new_spriteid > 2){
        new_spriteid = 0;
      }
    }
    food->spriteid = new_spriteid;

    replacefood(food->spriteid, food->x, food->y, randx, randy, map, bkg_colliders, food_tilemap_offset);
    food->x = randx;
    food->y = randy; 

    food->timer = food_timer;
    food->animation_frame = 20;

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
  else if ((((*input & J_UP) && !(*old_input & J_UP)) || \
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
    uint8_t pause_tiles[6]; 
    pause_tiles[0] = 0x1A;
    pause_tiles[1] = 0x0B;
    pause_tiles[2] = 0x1F;
    pause_tiles[3] = 0x1D;
    pause_tiles[4] = 0x0F;
    pause_tiles[5] = 0x0E;
    set_win_tiles(10, 0, 6, 1, pause_tiles);

    waitpadup();
    waitpad(J_START);
    waitpadup();

    pause_tiles[0] = 0x0;
    pause_tiles[1] = 0x0;
    pause_tiles[2] = 0x0;
    pause_tiles[3] = 0x0;
    pause_tiles[4] = 0x0;
    pause_tiles[5] = 0x0;
    set_win_tiles(10, 0, 6, 1, pause_tiles);
  }           
  return valid_input;
}

void level_animation_up(unsigned char * next_level_tiles, unsigned char * next_level_map, unsigned char tile_ind, unsigned char ntiles){
  uint8_t input;
  HIDE_SPRITES;

  set_bkg_data(tile_ind, ntiles, next_level_tiles);
  set_bkg_based_tiles(0, 18, 20, 14, &next_level_map[4*20], tile_ind);

  for (char i=0; i < 14; i++){
    scroll_bkg(0, -8);
    play_moving_sound();
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }
  set_bkg_based_tiles(0, 14, 20, 4, next_level_map, tile_ind);
  for (char i=0; i < 4; i++){
    scroll_bkg(0, -8);
    play_moving_sound();
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }
}

void level_animation_down(unsigned char * next_level_tiles, unsigned char * next_level_map, unsigned char tile_ind, unsigned char ntiles){
  uint8_t input;
  HIDE_SPRITES;

  set_bkg_data(tile_ind, ntiles, next_level_tiles);
  set_bkg_based_tiles(0, 18, 20, 14, next_level_map, tile_ind);

  for (char i=0; i < 14; i++){
    scroll_bkg(0, 8);
    play_moving_sound();
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }

  set_bkg_based_tiles(0, 0, 20, 4, &next_level_map[14*20], tile_ind);
  for (char i=0; i < 4; i++){
    scroll_bkg(0, 8);
    play_moving_sound();
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }
}

void level_animation_right(unsigned char * next_level_tiles, unsigned char * next_level_map, unsigned char tile_ind, unsigned char ntiles){
  unsigned char column_buffer[18];
  char row, col;
  uint8_t input;
  
  HIDE_SPRITES;
  
  set_bkg_data(tile_ind, ntiles, next_level_tiles);

  // Load the first 12 columns into VRAM
  for (col = 0; col < 12; col++){
    for (row = 0; row < 18; row++){
      column_buffer[row] = next_level_map[(20*row) + col];
    }
    set_bkg_based_tiles(20 + col, 0, 1, 18, column_buffer, tile_ind);
  }

  // Move window to the right 20 tiles while loading the remaining 8 column at
  // the beginning of VRAM
  for (col = 0; col < 20; col++){
    play_moving_sound();
    scroll_bkg(8, 0);
    if (col < 8){
      for (row = 0; row < 18; row++){
        column_buffer[row] = next_level_map[12 + (20*row) + col];
      }
      set_bkg_based_tiles(col, 0, 1, 18, column_buffer, tile_ind);
    }
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }
}

void level_animation_left(unsigned char * next_level_tiles, unsigned char * next_level_map, unsigned char tile_ind, unsigned char ntiles){
  unsigned char column_buffer[18];
  char row, col;
  uint8_t input;
  
  HIDE_SPRITES;
  
  set_bkg_data(tile_ind, ntiles, next_level_tiles);

  // Load the last 12 columns into VRAM
  for (col = 0; col < 12; col++){
    for (row = 0; row < 18; row++){
      column_buffer[row] = next_level_map[8 + (20*row) + col];
    }
    set_bkg_based_tiles(20 + col, 0, 1, 18, column_buffer, tile_ind);
  }

  // Move window to the left 12 tiles
  for (col = 0; col < 12; col++){
    play_moving_sound();
    scroll_bkg(-8, 0);
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }

  // Load the remaining 8 columns into the 12th column of VRAM
  for (col = 0; col < 8; col++){
    for (row = 0; row < 18; row++){
      column_buffer[row] = next_level_map[(20*row) + col];
    }
    set_bkg_based_tiles(12 + col, 0, 1, 18, column_buffer, tile_ind);
  }
  
  // Move window to the left 8 tiles
  for (col = 0; col < 8; col++){
    play_moving_sound();
    scroll_bkg(-8, 0);
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }
}

void world_pan(struct LevelData * level_data){
  char current_level;
  uint8_t offset;
  uint8_t input;

  HIDE_SPRITES;
  HIDE_WIN;
  SHOW_BKG;

  // Load Level 4 tiles and map  
  current_level = 3;
  set_bkg_data(FONT_TILE_OFFSET, level_data[current_level].ntiles, level_data[current_level].tiles);
  set_bkg_based_tiles(0, 0, 20, 18, level_data[current_level].map, FONT_TILE_OFFSET);
  move_bkg(0,0);
  offset = FONT_TILE_OFFSET + level_data[current_level].ntiles;
  wait(20);

  // Move up to Level 3 (up)
  current_level--;
  set_bkg_data(offset, level_data[current_level].ntiles, level_data[current_level].tiles);
  set_bkg_based_tiles(0, 18, 20, 14, &level_data[current_level].map[4*20], offset);

  for (char i=0; i < 14; i++){
    scroll_bkg(0, -8);
    play_moving_sound();
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }
  set_bkg_based_tiles(0, 14, 20, 4, level_data[current_level].map, offset);
  for (char i=0; i < 4; i++){
    scroll_bkg(0, -8);
    play_moving_sound();
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }
  offset += level_data[current_level].ntiles;

  // Move up to Level 2 (left)
  current_level--;
  unsigned char column_buffer[18];
  char row, col;
  
  set_bkg_data(offset, level_data[current_level].ntiles, level_data[current_level].tiles);

  // Load the last 12 columns into VRAM
  for (col = 0; col < 12; col++){
    for (row = 0; row < 18; row++){
      column_buffer[row] = level_data[current_level].map[8 + (20*row) + col];
    }
    set_bkg_based_tiles(20 + col, 14, 1, 18, column_buffer, offset);
  }

  // Move window to the left 12 tiles
  for (col = 0; col < 12; col++){
    play_moving_sound();
    scroll_bkg(-8, 0);
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }

  // Load the remaining 8 columns into the 12th column of VRAM
  for (col = 0; col < 8; col++){
    for (row = 0; row < 18; row++){
      column_buffer[row] = level_data[current_level].map[(20*row) + col];
    }
    set_bkg_based_tiles(12 + col, 14, 1, 18, column_buffer, offset);
  }
  
  // Move window to the left 8 tiles
  for (col = 0; col < 8; col++){
    play_moving_sound();
    scroll_bkg(-8, 0);
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }

  offset += level_data[current_level].ntiles;

  // Move up to Level 1 (down)
  current_level--;
  set_bkg_data(offset, level_data[current_level].ntiles, level_data[current_level].tiles);
  set_bkg_based_tiles(12, 0, 20, 14, level_data[current_level].map, offset);

  for (char i=0; i < 14; i++){
    scroll_bkg(0, 8);
    play_moving_sound();
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }

  set_bkg_based_tiles(12, 14, 20, 4, &level_data[current_level].map[14*20], offset);
  for (char i=0; i < 4; i++){
    scroll_bkg(0, 8);
    play_moving_sound();
    input = wait(20);
    if (input & (J_START | J_A | J_B)){
      return;
    }
  }

  move_win(7, 120);
  unsigned char first_row[] = {0x0,0xC,0xF,0xB,0x1E,0x0,0xB,0x16,0x16,0x0,0x16,0xF,0x20,0xF,0x16,0x1D,0x0,0x1E,0x19,0x0};
  unsigned char second_row[] = {0x0,0x11,0xF,0x1E,0x0,0xC,0xB,0xD,0x15,0x0,0x12,0x19,0x17,0xF,0x0,0x0,0x0,0x0,0x0,0x0};
  unsigned char third_row[] = {0x0,0x0,0x0,0x0,0x1A,0x1C,0xF,0x1D,0x1D,0x0,0xB,0x18,0x23,0x0,0x15,0xF,0x23,0x0,0x0,0x0};
  set_win_tiles(0, 0, 20, 1, first_row);
  set_win_tiles(0, 1, 20, 1, second_row);
  set_win_tiles(0, 2, 20, 1, third_row);
  SHOW_WIN;

  waitpad(J_START | J_A | J_B);
  waitpadup();

  HIDE_BKG;
  HIDE_WIN;
  
  // Clear the window
  for (char i = 0; i < 20; i++){
    first_row[i] = 0x0;
    second_row[i] = 0x0;
    third_row[i] = 0x0;
  }
  set_win_tiles(0, 0, 20, 1, first_row);
  set_win_tiles(0, 1, 20, 1, second_row);
  set_win_tiles(0, 2, 20, 1, third_row);

  move_win(7, 128);
}

void main(void){
  struct SnakePart* tmphead;
  font_t min_font;
  uint8_t lives_tilemap_offset;
  uint8_t progressbar_tilemap_offset;
  uint8_t food_tilemap_offset;
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

  uint8_t food_sprite_id = 0; 
  uint8_t next_snaketail_sprite_id = 0;
  uint8_t snake_tail_ind = 0;
  uint8_t stop_play;
  uint8_t game_over;

  uint8_t score_tiles[3];
  uint8_t lives_tiles[3];
  uint8_t progressbar_tiles[11];
  uint8_t lives;
  
  struct LevelData level_data[] = {
    {
      .tiles = level1_tiles,
      .map = level1_map,
      .background_colliders = level1_background_colliders,
      .ntiles = level1_ntiles, 
      .head_startx = 80,
      .head_starty = 80,
      .head_dir = 1, // Up
      .food_startx = 72,
      .food_starty = 50,
      .left_boundary = 4,
      .right_boundary = 15,
      .top_boundary = 3,
      .bottom_boundary = 14,
      .next_level_len = 20,
      .start_speed = 35,
      .speedup = 1,
      .speed_increase_len = 1,
      .titlescreen = level1_titlescreen,
      .food_timer = 20
    },
    {
      .tiles = level2_tiles,
      .map = level2_map,
      .background_colliders = level2_background_colliders,
      .ntiles = level2_ntiles,
      .head_startx = 104,
      .head_starty = 120,
      .head_dir = 1, // Up
      .food_startx = 48,
      .food_starty = 88,
      .left_boundary = 3,
      .right_boundary = 13,
      .top_boundary = 5,
      .bottom_boundary = 14,
      .next_level_len = 25,
      .start_speed = 35,
      .speedup = 2,
      .speed_increase_len = 3,
      .titlescreen = level2_titlescreen,
      .food_timer = 20
    },
    {
      .tiles = level3_tiles,
      .map = level3_map,
      .background_colliders = level3_background_colliders,
      .ntiles = level3_ntiles,
      .head_startx = 72,
      .head_starty = 80,
      .head_dir = 4, // Right
      .food_startx = 72,
      .food_starty = 96,
      .left_boundary = 7,
      .right_boundary = 16,
      .top_boundary = 3,
      .bottom_boundary = 12,
      .next_level_len = 30,
      .start_speed = 30,
      .speedup = 1,
      .speed_increase_len = 2,
      .titlescreen = level3_titlescreen,
      .food_timer = 20
    },
    {
      .tiles = level4_tiles,
      .map = level4_map,
      .background_colliders = level4_background_colliders,
      .ntiles = level4_ntiles,
      .head_startx = 104,
      .head_starty = 32,
      .head_dir = 2, // Down
      .food_startx = 120,
      .food_starty = 120,
      .left_boundary = 7,
      .right_boundary = 16,
      .top_boundary = 1,
      .bottom_boundary = 14,
      .next_level_len = 40,
      .start_speed = 30,
      .speedup = 1,
      .speed_increase_len = 3,
      .titlescreen = level4_titlescreen,
      .food_timer = 20
    }
  };

  char current_level;

  char tail1_xoffset;
  char tail1_yoffset;
  char tail2_xoffset;
  char tail2_yoffset;

  unsigned char current_map[360];
  unsigned char current_bkg_colliders[360];
  
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

  /* Load window */
  move_win(7,128);

  // Turn on Sound
  NR52_REG = 0x80;  // Enable sound chip
  NR50_REG = 0x77;  // Max volume on both speakers
  NR51_REG = 0xBB;  // Enable bass,2,4 on both speakers

  uint16_t music_ind;
  uint8_t frame_counter;
  uint8_t old_bank;

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

    food_sprite_id = 1;
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
    SWITCH_ROM(2);
    show_game_titlescreen(game_titlescreen);
    SWITCH_ROM(1);
    world_pan(level_data);
    SWITCH_ROM(2);
    show_titlescreen(level_data[current_level].titlescreen);

    /* Start game */

    while (!game_over){
      /* Load Level */
      // SWITCH_ROM(current_level+1);
      SWITCH_ROM(1);

      // Copy background map and background colliders to Bank0 RAM
      for (int16_t i = 0; i < 360; i++){
        current_map[i] = level_data[current_level].map[i];
        current_bkg_colliders[i] = level_data[current_level].background_colliders[i];
      }

      /* Load background */
      lives_tilemap_offset = FONT_TILE_OFFSET + level_data[current_level].ntiles;
      progressbar_tilemap_offset = lives_tilemap_offset + 1;
      food_tilemap_offset = progressbar_tilemap_offset + 7;
      set_bkg_data(FONT_TILE_OFFSET, level_data[current_level].ntiles, level_data[current_level].tiles);
      set_bkg_data(lives_tilemap_offset, 1, snake_spritesheet_data);
      set_bkg_data(progressbar_tilemap_offset, 7, progressbar_tiles_tiles);
      set_bkg_data(food_tilemap_offset, 4, food_spritesheet_tiles);

      set_bkg_based_tiles(0, 0, 20, 18, current_map, FONT_TILE_OFFSET);
      move_bkg(0,0);

      lives_tiles[0] = FONT_TILE_OFFSET + level_data[current_level].ntiles;
      lives_tiles[1] = 0x22;
      lives_tiles[2] = 0x1 + lives;
      set_win_tiles(5, 0, 3, 1, lives_tiles);

      progressbar_tiles[0] = progressbar_tilemap_offset; // timer tile
      progressbar_tiles[1] = progressbar_tilemap_offset + 1; // left edge of bar
      for (uint8_t i = 2; i < 10; i++){
        progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
      }
      progressbar_tiles[10] = progressbar_tilemap_offset + 3; // right edge of bar
      set_win_tiles(5, 1, 11, 1, progressbar_tiles);

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
      food_sprite.animation_frame = 20;
      food_sprite.animation_state = 0;
      placefood(food_sprite.spriteid, food_sprite.x, food_sprite.y, current_map, current_bkg_colliders, food_tilemap_offset);
      
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

      SHOW_SPRITES;
      SHOW_BKG;
      SHOW_WIN;

      music_ind = 0;
      frame_counter = 0;
      
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
        if (background_collision(snake_tail[0].sprite.x+dx_coll, snake_tail[0].sprite.y+dy_coll, current_bkg_colliders, debug_tiles) == 1){
          stop_play = 1;
          play_dying_sound();
          if (dx > 0) {
            // Face right
            set_sprite_tile(snake_tail[0].sprite.spriteid, SNAKE_HEAD_L);
            set_sprite_prop(snake_tail[0].sprite.spriteid, S_FLIPX);
          }
          else if (dx < 0) {
            // Face left
            set_sprite_tile(snake_tail[0].sprite.spriteid, SNAKE_HEAD_L);
            set_sprite_prop(snake_tail[0].sprite.spriteid, get_sprite_prop(0) & ~S_FLIPX);
          }

          if (dy > 0){
            // Face down
            set_sprite_tile(snake_tail[0].sprite.spriteid, SNAKE_HEAD_UP);
            set_sprite_prop(snake_tail[0].sprite.spriteid, S_FLIPY);
          }
          else if (dy < 0) {
            // Face up
            set_sprite_tile(snake_tail[0].sprite.spriteid, SNAKE_HEAD_UP);
            set_sprite_prop(snake_tail[0].sprite.spriteid, get_sprite_prop(0) & ~S_FLIPY);
          }

          move_tail(&snake_tail[0], snake_tail[0].sprite.x, snake_tail[0].sprite.y);
        }
        else {
          new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 
          // play_moving_sound();
          move_snake(&snake_tail[0], dx, dy);
          if (background_collision(snake_tail[0].sprite.x, snake_tail[0].sprite.y, current_bkg_colliders, debug_tiles) == 2) {
            new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 
            if (food_sprite.spriteid == 3){
              // Key
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
              movefood(&food_sprite, &snake_tail[0], current_bkg_colliders, current_map, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 0, food_tilemap_offset, level_data[current_level].food_timer);
              progressbar_tiles[0] = progressbar_tilemap_offset; // timer tile
              progressbar_tiles[1] = progressbar_tilemap_offset + 1; // left edge of bar
              for (uint8_t i = 2; i < 10; i++){
                progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
              }
              progressbar_tiles[10] = progressbar_tilemap_offset + 3; // right edge of bar
              set_win_tiles(5, 1, 11, 1, progressbar_tiles);
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
        
        switch (food_sprite.timer)
        {
          case 18:
            progressbar_tiles[10] = progressbar_tilemap_offset + 6; // First decrement
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          case 16:
            progressbar_tiles[10] = progressbar_tilemap_offset + 6; // First decrement
            progressbar_tiles[9] = progressbar_tilemap_offset + 5;
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          case 14:
            progressbar_tiles[10] = progressbar_tilemap_offset + 6; // First decrement
            progressbar_tiles[9] = progressbar_tilemap_offset + 5;
            progressbar_tiles[8] = progressbar_tilemap_offset + 5;
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          case 12:
            progressbar_tiles[10] = progressbar_tilemap_offset + 6; // First decrement
            progressbar_tiles[9] = progressbar_tilemap_offset + 5;
            progressbar_tiles[8] = progressbar_tilemap_offset + 5;
            progressbar_tiles[7] = progressbar_tilemap_offset + 5;
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          case 10:
            progressbar_tiles[6] = progressbar_tilemap_offset + 5;
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          case 8:
            progressbar_tiles[5] = progressbar_tilemap_offset + 5;
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          case 6:
            progressbar_tiles[4] = progressbar_tilemap_offset + 5;
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          case 4:
            progressbar_tiles[3] = progressbar_tilemap_offset + 5;
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          case 2:
            progressbar_tiles[2] = progressbar_tilemap_offset + 5;
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          case 0:
            progressbar_tiles[1] = progressbar_tilemap_offset + 4;
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
            break;
          
          default:
            break;
        }        

        new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 
        if (((snake_tail_ind) == level_data[current_level].next_level_len) && (food_sprite.spriteid != 3)){
          // Deploy key
          play_key_sound();
          movefood(&food_sprite, &snake_tail[0], current_bkg_colliders, current_map, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 1, food_tilemap_offset, level_data[current_level].food_timer);
          progressbar_tiles[0] = progressbar_tilemap_offset; // timer tile
          progressbar_tiles[1] = progressbar_tilemap_offset + 1; // left edge of bar
          for (uint8_t i = 2; i < 10; i++){
            progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
          }
          progressbar_tiles[10] = progressbar_tilemap_offset + 3; // right edge of bar
          set_win_tiles(5, 1, 11, 1, progressbar_tiles);
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
          old_bank = _current_bank;
          SWITCH_ROM(2);
          if (frequencies[bass_main_music[music_ind]] != 65535){ 
            play_music(&music_ind, &frame_counter, bass_main_music, lead_main_music);
            frame_counter++;
            if (frame_counter == calculate_tempo(snake_tail_ind)){
              frame_counter = 0;
              music_ind++;
            }
          }
          else {
            music_ind = 0;
            frame_counter = 0;
          }
          SWITCH_ROM(old_bank);

          new_input |= get_input(&input, &old_input, move_dir_buff, &start_ind, &old_direction); 

          if (food_sprite.timer == 0) {
            movefood(&food_sprite, &snake_tail[0], current_bkg_colliders, current_map, level_data[current_level].left_boundary, level_data[current_level].right_boundary, level_data[current_level].top_boundary, level_data[current_level].bottom_boundary, debug_tiles, 0, food_tilemap_offset, level_data[current_level].food_timer);
            progressbar_tiles[0] = progressbar_tilemap_offset; // timer tile
            progressbar_tiles[1] = progressbar_tilemap_offset + 1; // left edge of bar
            for (uint8_t i = 2; i < 10; i++){
              progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
            }
            progressbar_tiles[10] = progressbar_tilemap_offset + 3; // right edge of bar
            set_win_tiles(5, 1, 11, 1, progressbar_tiles);
          }
          else {
            // Wait until the Vertical Blanking is done
            wait_vbl_done();
          }
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
        // lives -= 1;  // Removed for v0.3.1
        stop_play = 0;
      }
      else if (stop_play == 2){
        stop_play = 0;
        current_level++;
        if ((current_level > 1) && (lives < 3)){
          // Add a life after beating the level 2 and 3
          // and the player has less than 3 lives.
          lives++;
        }

        /* Reset all variables */
        tmphead = snake_tail;
        while(tmphead != NULL) {
          move_sprite(tmphead->sprite.spriteid, 0, 0);
          tmphead = tmphead->next;
        }

        fadeout();
        HIDE_WIN;
        removefood(food_sprite.x, food_sprite.y, current_map, current_bkg_colliders);
        if (current_level == 1) {
          fadein();
          level_animation_up(level_data[current_level].tiles, level_data[current_level].map, food_tilemap_offset + 4, level_data[current_level].ntiles);
        }
        else if (current_level == 2){
          fadein();
          level_animation_right(level_data[current_level].tiles, level_data[current_level].map, food_tilemap_offset + 4, level_data[current_level].ntiles);
        }
        else if (current_level == 3){
          fadein();
          level_animation_down(level_data[current_level].tiles, level_data[current_level].map, food_tilemap_offset + 4, level_data[current_level].ntiles);
        }
        SWITCH_ROM(2);
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
        lives_tiles[0] = FONT_TILE_OFFSET + level_data[current_level].ntiles;
        lives_tiles[2] = 0x1 + lives;
        set_win_tiles(5, 0, 3, 1, lives_tiles);
    
        /* Reset all variables */
        tmphead = snake_tail;
        while(tmphead != NULL) {
          move_sprite(tmphead->sprite.spriteid, 0, 0);
          tmphead = tmphead->next;
        }

        // Reset background_colliders and background sprites
        removefood(food_sprite.x, food_sprite.y, current_map, current_bkg_colliders);
      }
    // End Game loop (while (!game_over){})
    }

    if (game_over) {
      fadeout();
      SWITCH_ROM(2);
      show_finalscreen(gameover_titlescreen, 0);
    }
    else if (stop_play == 3){
      fadeout();
      SWITCH_ROM(2);
      show_finalscreen(win_titlescreen, 1);
    }
    HIDE_BKG;
    HIDE_WIN;

    /* Reset all variables */
    tmphead = snake_tail;
    while(tmphead != NULL) {
      move_sprite(tmphead->sprite.spriteid, 0, 0);
      tmphead = tmphead->next;
    }

    // // Reset background_colliders and background sprites
    // for (uint16_t i; i < (20*18); i++){
      // if (level_data[current_level].background_colliders[i] == 2){
        // level_data[current_level].background_colliders[i] == 0;
      // }
      // if (level_data[current_level].map[i] >= food_tilemap_offset){
        // level_data[current_level].map[i] == 0;
      // }
    // }

    // /* 
    // End the program: 
    // REMOVE TO ALLOW FOR THE GAME TO RESTART
    // ONCE THE MAP CORRUPTION IS DEBUGGED.
    // */
    // show_finalscreen(restart_titlescreen, 2);
    // SHOW_BKG;
    // break;

  // End system loop (while (1){})
  }
}
