#include <gb/gb.h>
#include <stdio.h>

#include "Sprite.c"
#include "snake_round.c"
#include "food.c"

UINT8 SCREEN_L = 40;
UINT8 SCREEN_WIDTH = 80;
UINT8 SCREEN_T = 40;
UINT8 SCREEN_HEIGHT = 80;

/*
The snake sprite has5 8x8 tiles:
    index 0: Head Up
    index 1: Head Left
    index 2: Head Dead Up
    index 3: Head Dead Left
    index 4: Body 
*/
/* Load snake sprite on the first 9 locations of sprite memory */
UINT8 SNAKE_MEMIND = 0;
UINT8 SNAKE_NTILES = 5;

UINT8 SNAKE_HEAD_UP = SNAKE_MEMIND + 0;
UINT8 SNAKE_HEAD_L = SNAKE_MEMIND + 1;
UINT8 SNAKE_DEAD_HEAD_UP = SNAKE_MEMIND + 2;
UINT8 SNAKE_DEAD_HEAD_L = SNAKE_MEMIND + 3;
UINT8 SNAKE_BODY = SNAKE_MEMIND + 4;


/* The food sprite has 3 8x8 tiles:
    index 0: biscuit
    index 1: carrot
    index 2: turnip
*/
/* Load the food sprite right after snake */
UINT8 FOOD_MEMIND = 5;//SNAKE_NTILES;
UINT8 FOOD_NTILES = 3;

UINT8 FOOD_BISCUIT = FOOD_MEMIND;
UINT8 FOOD_CARROT = FOOD_MEMIND + 1;
UINT8 FOOD_TURNIP = FOOD_MEMIND + 2;

void wait(UINT8 n){
    // Interrupt-based delay. Not CPU intensive.
    // Returns after n Vertical Blanking interrupts.
    UINT8 x;
    for (x = 0; x < n; x++){
        wait_vbl_done();
    }
}

void move_snake(struct SnakePart* head, UINT8 x, UINT8 y){
    /*
    For the snake movement, the head moves in the direction of the joypad 
    and the tail follows the head.
    */
    struct SnakePart* tail;

    // Process head movement first
    UINT8 headx = head->sprite.x;
    UINT8 heady = head->sprite.y;
    UINT8 newx = head->sprite.x + x;
    UINT8 newy = head->sprite.y + y;

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

void movefood(struct Sprite* food, UINT8 x, UINT8 y){
    UINT8 newx = food->x + x;
    UINT8 newy = food->y + y;

    if (newx >= (SCREEN_L + SCREEN_WIDTH)){
        newx = SCREEN_L + SCREEN_WIDTH;
    }
    else if (newx <= (SCREEN_L + food->width)){
        newx = SCREEN_L + food->width;
    }

    if (newy >= (SCREEN_T + SCREEN_HEIGHT)){
        newy = SCREEN_T + SCREEN_HEIGHT;
    }
    else if (newy <= (SCREEN_T + food->height)){
        newy = SCREEN_T + food->height;
    }

    food->x = newx;
    food->y = newy;
    move_sprite(food->spriteid, newx, newy);
}

UBYTE sprite_collision(struct Sprite* sp1, struct Sprite* sp2){
    /* This code peforms collision between the center of sp1 with sp2 */
    UINT8 sp1_left = sp1->x;
    UINT8 sp1_right = sp1->x + sp1->width;
    UINT8 sp1_top = sp1->y;
    UINT8 sp1_bottom = sp1->y + sp1->height;
    UINT8 sp2_left = sp2->x;
    UINT8 sp2_right = sp2->x + sp2->width;
    UINT8 sp2_top = sp2->y;
    UINT8 sp2_bottom = sp2->y + sp2->height;

    return (((sp1_left >= sp2_left) && (sp1_left <= sp2_right)) && ((sp1_top >= sp2_top) && (sp1_top <= sp2_bottom))) || (((sp2_left >= sp1_left) && (sp2_left <= sp1_right)) && ((sp2_top >= sp1_top) && (sp2_top <= sp1_bottom)));
}

// UBYTE background_collision(struct Sprite* sp1, struct BackgroundObstacle bkg[], UINT8 len){
//     /* This code peforms collision between the center of sp1 with 
//        each element of bkg[] */
//     UINT8 sp1_left = sp1->x;
//     UINT8 sp1_right = sp1->x + sp1->width;
//     UINT8 sp1_top = sp1->y;
//     UINT8 sp1_bottom = sp1->y + sp1->height;
//     UINT8 collision;
//     UINT8 bkg_left;
//     UINT8 bkg_right;
//     UINT8 bkg_top;
//     UINT8 bkg_bottom;
//     UINT8 i;

//     for (i=0; i < len; i++){
//         bkg_left = bkg.x;
//         bkg_right = bkg[i].x + bkg[i].width;
//         bkg_top = bkg[i].y;
//         bkg_bottom = bkg[i].y + bkg[i].height;

//         collision = (((sp1_left >= bkg_left) && (sp1_left <= bkg_right)) && ((sp1_top >= bkg_top) && (sp1_top <= bkg_bottom))) || (((bkg_left >= sp1_left) && (bkg_left <= sp1_right)) && ((bkg_top >= sp1_top) && (bkg_top <= sp1_bottom)));
//         if (collision) {
//             return 1;
//         }
//     }

//     return 0;
// }

void main(){
    UINT8 snake_size = 20;
    struct SnakePart snake_tail[20];
    struct Sprite food_sprite;
    UINT8 food_sprite_id = 0;
    UINT8 next_snaketail_sprite_id = 1;
    UINT8 snake_tail_ind = 0;

    /* Load sprite data */
    set_sprite_data(SNAKE_MEMIND, SNAKE_NTILES, snake_round_sprite);
    set_sprite_data(FOOD_MEMIND, FOOD_NTILES, food);
    
    /* Create food sprite */
    food_sprite.x = 72;
    food_sprite.y = 42;
    food_sprite.width = 8;
    food_sprite.height = 8;
    food_sprite.spriteid = food_sprite_id;
    set_sprite_tile(food_sprite_id,FOOD_BISCUIT);
    move_sprite(food_sprite.spriteid, food_sprite.x, food_sprite.y);
    

    /* Create Snake head at index 0 of snake_tail array*/
    snake_tail[snake_tail_ind].sprite.x = 72;
    snake_tail[snake_tail_ind].sprite.y = 72;
    snake_tail[snake_tail_ind].sprite.width = 8;
    snake_tail[snake_tail_ind].sprite.height = 8;
    snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
    set_sprite_tile(next_snaketail_sprite_id,SNAKE_HEAD_UP);
    move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
    next_snaketail_sprite_id++;
    snake_tail_ind++;

    /* Create snake tail */
    snake_tail[snake_tail_ind].sprite.x = 72;
    snake_tail[snake_tail_ind].sprite.y = 72+8;
    snake_tail[snake_tail_ind].sprite.width = 8;
    snake_tail[snake_tail_ind].sprite.height = 8;
    snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
    set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
    move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
    snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
    next_snaketail_sprite_id++;
    snake_tail_ind++;

    snake_tail[snake_tail_ind].sprite.x = 72;
    snake_tail[snake_tail_ind].sprite.y = 72+16;
    snake_tail[snake_tail_ind].sprite.width = 8;
    snake_tail[snake_tail_ind].sprite.height = 8;
    snake_tail[snake_tail_ind].sprite.spriteid = next_snaketail_sprite_id;
    set_sprite_tile(next_snaketail_sprite_id, SNAKE_BODY);
    move_sprite(snake_tail[snake_tail_ind].sprite.spriteid, snake_tail[snake_tail_ind].sprite.x, snake_tail[snake_tail_ind].sprite.y);
    snake_tail[snake_tail_ind-1].next = &snake_tail[snake_tail_ind];
    snake_tail[snake_tail_ind].next = NULL;
    next_snaketail_sprite_id++;
    snake_tail_ind++;

    SHOW_SPRITES;
    DISPLAY_ON;

    while(1){
        switch (joypad()){
            case J_UP:
                if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
                    movefood(&food_sprite, -8, 8);
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
                move_snake(&snake_tail[0], 0, -8);     
                break;
            
            case J_DOWN:
                if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
                    movefood(&food_sprite, 8, -8);
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
                move_snake(&snake_tail[0], 0, 8);
                break;

            case J_LEFT:
                if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
                    movefood(&food_sprite, 10, 10);
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
                move_snake(&snake_tail[0], -8, 0);
                break;
                
            case J_RIGHT:
                if (sprite_collision(&snake_tail[0].sprite, &food_sprite)){
                    movefood(&food_sprite, -8, 8);
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
                move_snake(&snake_tail[0], 8, 0);
                break;

            default:
                break;
        }

        // wait(5);
        delay(100);
    }
}
