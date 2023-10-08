#include <gb/gb.h>
#include <gbdk/font.h>
#include <stdint.h>
#include <stdio.h>
#include <rand.h>

#include "Sprite.h"
#include "food.h"
#include "snake_spritesheet_py.h"
#include "background_tiles_py.h"
#include "background_map_py.h"
#include "background_colliders.h"
#include "windowmap.h"

#define SNAKE_MAX_SIZE 37

uint8_t SCREEN_L = 40;
uint8_t SCREEN_WIDTH = 80;
uint8_t SCREEN_T = 40;
uint8_t SCREEN_HEIGHT = 80;

/*
The snake sprite has5 8x8 tiles:
    index 0: Head Up
    index 1: Head Left
    index 2: Head Dead Up
    index 3: Head Dead Left
    index 4: Body 
*/
/* Load snake sprite on the first 9 locations of sprite memory */
uint8_t SNAKE_MEMIND = 0;
uint8_t SNAKE_NTILES = 5;

uint8_t SNAKE_HEAD_UP = 0 + 0;  //SNAKE_MEMIND
uint8_t SNAKE_HEAD_L = 0 + 1;  //SNAKE_MEMIND + 1
uint8_t SNAKE_DEAD_HEAD_UP = 0 + 2;  //SNAKE_MEMIND + 2
uint8_t SNAKE_DEAD_HEAD_L = 0 + 3;  //SNAKE_MEMIND + 3
uint8_t SNAKE_BODY = 0 + 4;  //SNAKE_MEMIND + 4


/* The food sprite has 3 8x8 tiles:
    index 0: biscuit
    index 1: carrot
    index 2: turnip
*/
/* Load the food sprite right after snake */
uint8_t FOOD_MEMIND = 5;  //SNAKE_NTILES;
uint8_t FOOD_NTILES = 3;

uint8_t FOOD_BISCUIT = 5; //FOOD_MEMIND
uint8_t FOOD_CARROT = 5 + 1; //FOOD_MEMIND + 1
uint8_t FOOD_TURNIP = 5 + 2; //FOOD_MEMIND + 2

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
    printf("     GAME OVER       ");
    printf("     Score: %d       ", score);
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
    printf("     YOU WIN!        ");
    printf("     Score: %d       ", score);
    for (x =0; x < 8; x++){
        printf("                    ");
    }
    waitpad(J_START);
    waitpadup();
}

UBYTE sprite_collision(struct Sprite* sp1, struct Sprite* sp2){
    /* This code peforms collision between the centers of sp1 with sp2 */
    uint8_t sp1_left = sp1->x + sp1->width/2;
    uint8_t sp1_right = sp1->x + sp1->width - sp1->width/2;
    uint8_t sp1_top = sp1->y + sp1->height/2;
    uint8_t sp1_bottom = sp1->y + sp1->height;
    uint8_t sp2_left = sp2->x;
    uint8_t sp2_right = sp2->x + sp2->width;
    uint8_t sp2_top = sp2->y;
    uint8_t sp2_bottom = sp2->y + sp2->height;

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

UBYTE background_collision(uint8_t x, uint8_t y, char* bkg_colliders){
    /* 
      Convert x, y into tile indices (20 wide, 18 tall) and check the
      bkg_colliders array at that index. If there is a 0x1 at that 
      location, then a collision with a background collider has occurred.
     */
    uint8_t collision;
    uint8_t tileind;
    uint8_t stride = 20;

    tileind = y / stride + (x % stride);

    if (bkg_colliders[tileind] == 1) {
      collision = 1;
    }
    else{
      collision = 0;
    }

    return collision;
}

void move_snake(struct SnakePart* head, int8_t x, int8_t y, char* background_colliders){
    /*
    For the snake movement, the head moves in the direction of the joypad 
    and the tail follows the head.
    */
    struct SnakePart* tail;

    // Process head movement first
    uint8_t headx;
    uint8_t heady;
    uint8_t newx;
    uint8_t newy;

    headx = head->sprite.x;
    heady = head->sprite.y;
    newx = head->sprite.x + x;
    newy = head->sprite.y + y;

    // Check collision with the screen borders
    if (newx >= (SCREEN_L + SCREEN_WIDTH)){
        newx = SCREEN_L + SCREEN_WIDTH;
    }
    else if (newx <= SCREEN_L){
        newx = SCREEN_L + head->sprite.width;
    }

    if (newy >= (SCREEN_T + SCREEN_HEIGHT)){
        newy = SCREEN_T + SCREEN_HEIGHT;
    }
    else if (newy <= SCREEN_T){
        newy = SCREEN_T + head->sprite.height;
    }

    // Check collision with background obstacles    
    if (background_collision(newx, newy, background_colliders)){
        newx = headx;
        newy = heady;
    }

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

void movefood(struct Sprite* food, struct SnakePart* snake){
    /* This function places a new food item at the location of the snake's tail tip */
    uint8_t new_spriteid;

    new_spriteid = food->spriteid+1;
    if (new_spriteid > 39){
        new_spriteid = 37;
    }
    move_sprite(food->spriteid, 0, 0);
    food->spriteid = new_spriteid;

    // Get the snake's tail
    while(snake->next != NULL){
        snake = snake->next;
    }

    food->x = snake->sprite.x;
    food->y = snake->sprite.y;
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
    uint8_t snake_size = SNAKE_MAX_SIZE;
    struct SnakePart snake_tail[SNAKE_MAX_SIZE];
    struct Sprite food_sprite;
    
    uint8_t move_direction = J_UP;
    uint8_t jpad = 0x0;
    uint8_t wait_loop_ind;
    uint8_t speed = 40;

    uint8_t food_sprite_id = 37;
    uint8_t next_snaketail_sprite_id = 0;
    uint8_t snake_tail_ind = 0;
    uint8_t game_over = 0;
    uint8_t score = 0;
    uint8_t score_increment = 1;

    uint8_t score_tiles[3];
    uint8_t lives_tiles[3];
    uint8_t lives = 3;

    /* Initialize font */
    font_init();
    min_font = font_load(font_min); // 36 tiles
    font_set(min_font);

    /* Load background */
    set_bkg_data(37, 16, background_tiles);
    set_bkg_data(53, 1, snake_spritesheet_data);

    lives_tiles[0] = 0x44;
    lives_tiles[1] = 0x44;
    lives_tiles[2] = 0x44;

    /* Load sprite data */
    set_sprite_data(SNAKE_MEMIND, SNAKE_NTILES, snake_spritesheet_data);
    set_sprite_data(FOOD_MEMIND, FOOD_NTILES, food);
    set_sprite_tile(food_sprite_id, FOOD_BISCUIT);
    set_sprite_tile(food_sprite_id+1, FOOD_CARROT);
    set_sprite_tile(food_sprite_id+2, FOOD_TURNIP);

    SHOW_BKG;
    SHOW_WIN;
    SHOW_SPRITES;
    DISPLAY_ON;

    while (1) {
        initrand(DIV_REG);

        jpad = 0x0;
        speed = 40;

        food_sprite_id = 37;
        next_snaketail_sprite_id = 0;
        snake_tail_ind = 0;
        game_over = 0;
        score_increment = 1;
        move_direction = J_UP;

        /* Main code */
        set_bkg_tiles(0,0,20,18,background_map);
        
        score2tile(score, score_tiles);
        // score2tile(score, &score_tiles);

        /* Load window */
        set_win_tiles(0, 0, 6, 1, scoremap);
        set_win_tiles(6, 0, 3, 1, score_tiles);
        set_win_tiles(16, 0, 3, 1, lives_tiles);
        move_win(7,136);

        /* Create food sprite */
        // Add code to randomly generate the x,y coordinates of the food. Copy to other places where we spawn new food.
        food_sprite.x = 72;
        food_sprite.y = 50;
        food_sprite.width = 8;
        food_sprite.height = 8;
        food_sprite.spriteid = food_sprite_id;
        move_sprite(food_sprite.spriteid, food_sprite.x, food_sprite.y);
        
        /* Create Snake head at index 0 of snake_tail array*/
        snake_tail[snake_tail_ind].sprite.x = 80;
        snake_tail[snake_tail_ind].sprite.y = 80;
        snake_tail[snake_tail_ind].sprite.width = 8;
        snake_tail[snake_tail_ind].sprite.height = 8;
        snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
        set_sprite_tile(next_snaketail_sprite_id,SNAKE_HEAD_UP);
        move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
        next_snaketail_sprite_id++;
        snake_tail_ind++;

        /* Create snake tail */
        snake_tail[snake_tail_ind].sprite.x = 80;
        snake_tail[snake_tail_ind].sprite.y = 80+8;
        snake_tail[snake_tail_ind].sprite.width = 8;
        snake_tail[snake_tail_ind].sprite.height = 8;
        snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
        set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
        move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
        snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
        next_snaketail_sprite_id++;
        snake_tail_ind++;   
        
        snake_tail[snake_tail_ind].sprite.x = 80;
        snake_tail[snake_tail_ind].sprite.y = 80+16;
        snake_tail[snake_tail_ind].sprite.width = 8;
        snake_tail[snake_tail_ind].sprite.height = 8;
        snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
        set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
        move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
        snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
        snake_tail[snake_tail_ind].next = NULL;
        next_snaketail_sprite_id++;
        snake_tail_ind++;

        SHOW_BKG;
        SHOW_WIN;
        SHOW_SPRITES;
        DISPLAY_ON;

        while(!game_over){
            if ((move_direction & J_UP) == J_UP){
                    move_snake(&snake_tail[0], 0, -8, &background_colliders[0]);
                    if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
                        movefood(&food_sprite, &snake_tail[0]);
                        score = score + score_increment;
                        if (snake_tail_ind < snake_size) {
                            snake_tail[snake_tail_ind].sprite.x = snake_tail[snake_tail_ind-1].sprite.x;
                            snake_tail[snake_tail_ind].sprite.y = snake_tail[snake_tail_ind-1].sprite.y;
                            snake_tail[snake_tail_ind].sprite.width = snake_tail[snake_tail_ind-1].sprite.width;
                            snake_tail[snake_tail_ind].sprite.height = snake_tail[snake_tail_ind-1].sprite.height;
                            snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
                            set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
                            move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
                            snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
                            snake_tail[snake_tail_ind].next = NULL;
                            next_snaketail_sprite_id++;
                            snake_tail_ind++;
                        }                    
                    } 
                    // else if (head_tail_collision(&snake_tail)){
                    else if (head_tail_collision(snake_tail)){
                        // Collided head with tail. End Game
                        game_over = 1;
                    }  
            }                     
            else if ((move_direction & J_DOWN) == J_DOWN){
                    move_snake(&snake_tail[0], 0, 8, &background_colliders[0]);
                    if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
                        movefood(&food_sprite, &snake_tail[0]);
                        score = score + score_increment;
                        if (snake_tail_ind < snake_size) {
                            snake_tail[snake_tail_ind].sprite.x = snake_tail[snake_tail_ind-1].sprite.x;
                            snake_tail[snake_tail_ind].sprite.y = snake_tail[snake_tail_ind-1].sprite.y;
                            snake_tail[snake_tail_ind].sprite.width = snake_tail[snake_tail_ind-1].sprite.width;
                            snake_tail[snake_tail_ind].sprite.height = snake_tail[snake_tail_ind-1].sprite.height;
                            snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
                            set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
                            move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
                            snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
                            snake_tail[snake_tail_ind].next = NULL;
                            next_snaketail_sprite_id++;
                            snake_tail_ind++;
                        }
                    }
                    // else if (head_tail_collision(&snake_tail)){
                    else if (head_tail_collision(snake_tail)){
                        // Collided head with tail. End Game
                        game_over = 1;
                    }
                    
            }
            else if (((move_direction & J_LEFT) == J_LEFT)){
                    move_snake(&snake_tail[0], -8, 0, &background_colliders[0]);
                    if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
                        // Ate food. Increment tail.
                        movefood(&food_sprite, &snake_tail[0]);
                        score++;
                        if (snake_tail_ind < snake_size) {
                            snake_tail[snake_tail_ind].sprite.x = snake_tail[snake_tail_ind-1].sprite.x;
                            snake_tail[snake_tail_ind].sprite.y = snake_tail[snake_tail_ind-1].sprite.y;
                            snake_tail[snake_tail_ind].sprite.width = snake_tail[snake_tail_ind-1].sprite.width;
                            snake_tail[snake_tail_ind].sprite.height = snake_tail[snake_tail_ind-1].sprite.height;
                            snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
                            set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
                            move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
                            snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
                            snake_tail[snake_tail_ind].next = NULL;
                            next_snaketail_sprite_id++;
                            snake_tail_ind++;
                        }
                    }
                    // else if (head_tail_collision(&snake_tail)){
                    else if (head_tail_collision(snake_tail)){
                        // Collided head with tail. End Game
                        game_over = 1;
                    }
                    
            }
            else if (((move_direction & J_RIGHT) == J_RIGHT)){
                    move_snake(&snake_tail[0], 8, 0, &background_colliders[0]);
                    if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
                        movefood(&food_sprite, &snake_tail[0]);
                        score = score + score_increment;
                        if (snake_tail_ind < snake_size) {
                            snake_tail[snake_tail_ind].sprite.x = snake_tail[snake_tail_ind-1].sprite.x;
                            snake_tail[snake_tail_ind].sprite.y = snake_tail[snake_tail_ind-1].sprite.y;
                            snake_tail[snake_tail_ind].sprite.width = snake_tail[snake_tail_ind-1].sprite.width;
                            snake_tail[snake_tail_ind].sprite.height = snake_tail[snake_tail_ind-1].sprite.height;
                            snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
                            set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
                            move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
                            snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
                            snake_tail[snake_tail_ind].next = NULL;
                            next_snaketail_sprite_id++;
                            snake_tail_ind++;
                        }
                    }
                    // else if (head_tail_collision(&snake_tail)){
                    else if (head_tail_collision(snake_tail)){
                        // Collided head with tail. End Game
                        game_over = 1;
                    }
            }
            score_increment = 1;
            // score2tile(score, &score_tiles);
            score2tile(score, score_tiles);
            set_win_tiles(6,0, 3, 1, score_tiles);
            
            if (snake_tail_ind > 37){
                win_screen(score);
                game_over = 1;
            }
            else if (snake_tail_ind > 30){
                speed = 25;
            }
            else if (snake_tail_ind > 15){
                speed = 30;
            }
            else if (snake_tail_ind > 10){
                speed = 35;
            }
            else if (snake_tail_ind > 5){
                speed = 37;
            }
            
            for (wait_loop_ind = 0; wait_loop_ind < speed; wait_loop_ind++){
                jpad = joypad();
                
                if (((jpad & 0x3) > 0) && ((move_direction & 0x3) == 0)){
                    // case where J_RIGHT or J_LEFT were pressed and
                    // snake is not already moving RIGHT or LEFT.
                    // This is valid user input.
                    move_direction = jpad;
                }
                else if (((jpad & 0xC) > 0) && ((move_direction & 0xC) == 0)){
                    // case where J_UP or J_DOWN were pressed and
                    // snake is not already moving UP or DOWN.
                    // This is valid user input.
                    move_direction = jpad;
                }  
                else if ((jpad & J_START) > 0){
                    // use logic to pause game
                    move_direction = jpad;
                }           
                wait_vbl_done();
            }

        }

        flash_sprites();
        lives -= 1;
        if (lives == 0){
            game_over_screen(score);
            lives = 3;
            score = 0;
            lives_tiles[0] = 0x44;
            lives_tiles[1] = 0x44;
            lives_tiles[2] = 0x44;
        }
        else{
            lives_tiles[lives] = 0x0;
        }

        /* Reset all variables */
        move_sprite(food_sprite.spriteid, 0, 0);
        tmphead = snake_tail;
        while(tmphead != NULL) {
            move_sprite(tmphead->sprite.spriteid, 0, 0);
            tmphead = tmphead->next;
        }
        
    }
}
