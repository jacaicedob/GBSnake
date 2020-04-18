#include <gb/gb.h>
#include <stdio.h>

#include "Sprite.c"
#include "snake_round.c"
#include "food.c"

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
UINT8 FOOD_MEMIND = SNAKE_NTILES;
UINT8 FOOD_NTILES = 3;

UINT8 FOOD_BISCUIT = FOOD_MEMIND;
UINT8 FOOD_CARROT = FOOD_MEMIND + 1;
UINT8 FOOD_TURNIP = FOOD_MEMIND + 2;

UINT8 SCREEN_WIDTH = 160;
UINT8 SCREEN_HEIGHT = 144;

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
    if (newx >= SCREEN_WIDTH){
        newx = SCREEN_WIDTH;
    }
    else if (newx <= 0){
        newx = head->sprite.width;
    }

    if (newy >= SCREEN_HEIGHT){
        newy = SCREEN_HEIGHT;
    }
    else if (newy <= 0){
        newy = head->sprite.height;
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

    if (newx >= SCREEN_WIDTH){
        newx = SCREEN_WIDTH;
    }
    else if (newx <= food->width){
        newx = food->width;
    }

    if (newy >= SCREEN_HEIGHT){
        newy = SCREEN_HEIGHT;
    }
    else if (newy <= food->height){
        newy = food->height;
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

UBYTE background_collision(struct Sprite* sp1, struct BackgroundObstacle bkg[], size_t len){
    /* This code peforms collision between the center of sp1 with 
       each element of bkg[] */
    UINT8 sp1_left = sp1->x;
    UINT8 sp1_right = sp1->x + sp1->width;
    UINT8 sp1_top = sp1->y;
    UINT8 sp1_bottom = sp1->y + sp1->height;
    UINT8 collision;

    for (UINT8 i; i < len; i++){
        UINT8 bkg_left = sp2->x;
        UINT8 bkg_right = bkg->x + bkg->width;
        UINT8 bkg_top = bkg->y;
        UINT8 bkg_bottom = bkg->y + bkg->height;

        collision = (((sp1_left >= bkg_left) && (sp1_left <= bkg_right)) && ((sp1_top >= bkg_top) && (sp1_top <= bkg_bottom))) || (((bkg_left >= sp1_left) && (bkg_left <= sp1_right)) && ((bkg_top >= sp1_top) && (bkg_top <= sp1_bottom)));
        if (collision) {
            return 1;
        }
    }

    return 0;
}

void main(){
    // struct Sprite snake_head;
    struct SnakePart snake_head;
    struct SnakePart snake_tail[10];
    struct Sprite food_sprite;
    UINT8 next_body_sprite = 2;
    UINT8 tail_ind = 0;

    /* Load sprite data and set tiles */
    set_sprite_data(SNAKE_MEMIND,SNAKE_NTILES,snake);
    set_sprite_data(FOOD_MEMIND,FOOD_NTILES,food);
    set_sprite_tile(0,SNAKE_HEAD_UP);
    set_sprite_tile(1,FOOD_BISCUIT);

    /* Setup Snake head */
    snake_head.sprite.x = 72;
    snake_head.sprite.y = 72;
    snake_head.sprite.width = 8;
    snake_head.sprite.height = 8;
    snake_head.sprite.spriteid = 0;
    move_sprite(snake_head.sprite.spriteid, snake_head.sprite.x, snake_head.sprite.y);

    snake_tail[tail_ind].sprite.x = 72;
    snake_tail[tail_ind].sprite.y = 72+8;
    snake_tail[tail_ind].sprite.width = 8;
    snake_tail[tail_ind].sprite.height = 8;
    snake_tail[tail_ind].sprite.spriteid = next_body_sprite;
    set_sprite_tile(next_body_sprite++,SNAKE_BODY);
    move_sprite(snake_tail[tail_ind].sprite.spriteid, snake_tail[tail_ind].sprite.x, snake_tail[tail_ind].sprite.y);
    snake_head.next = &snake_tail[tail_ind];
    tail_ind++;

    // snake_tail[tail_ind].sprite.x = 72;
    // snake_tail[tail_ind].sprite.y = 72+16;
    // snake_tail[tail_ind].sprite.width = 8;
    // snake_tail[tail_ind].sprite.height = 8;
    // snake_tail[tail_ind].sprite.spriteid = next_body_sprite;
    // set_sprite_tile(next_body_sprite++, SNAKE_BODY);
    // move_sprite(snake_tail[tail_ind].sprite.spriteid, snake_tail[tail_ind].sprite.x, snake_tail[tail_ind].sprite.y);
    // snake_tail[tail_ind-1].next = &snake_tail[tail_ind];
    // snake_tail[tail_ind].next = NULL;
    // tail_ind++;

    /* Draw food */
    food_sprite.x = 72;
    food_sprite.y = 32;
    food_sprite.width = 8;
    food_sprite.height = 8;
    food_sprite.spriteid = 1;
    move_sprite(food_sprite.spriteid, food_sprite.x, food_sprite.y);

    SHOW_SPRITES;
    DISPLAY_ON;

    while(1){
        switch (joypad()){
            case J_UP:
                
                if (sprite_collision(&snake_head.sprite, &food_sprite)){
                    movefood(&food_sprite, -8, 8);
                    // snake_tail[tail_ind].sprite.x = snake_tail[tail_ind-1].sprite.x;
                    // snake_tail[tail_ind].sprite.y = snake_tail[tail_ind-1].sprite.y;
                    // snake_tail[tail_ind].sprite.width = snake_tail[tail_ind-1].sprite.width;
                    // snake_tail[tail_ind].sprite.height = snake_tail[tail_ind-1].sprite.height;
                    // snake_tail[tail_ind].sprite.spriteid = next_body_sprite;
                    // set_sprite_tile(next_body_sprite++, SNAKE_BODY);
                    // move_sprite(snake_tail[tail_ind].sprite.spriteid, snake_tail[tail_ind].sprite.x, snake_tail[tail_ind].sprite.y);
                    // snake_tail[tail_ind-1].next = &snake_tail[tail_ind];
                    // snake_tail[tail_ind].next = NULL;
                    // tail_ind++;
                }   
                move_snake(&snake_head, 0, -8);     
                break;
            
            case J_DOWN:
                move_snake(&snake_head, 0, 8);
                if (sprite_collision(&snake_head.sprite, &food_sprite)){
                    movefood(&food_sprite, 8, -8);
                }
                break;

            case J_LEFT:
                move_snake(&snake_head, -8, 0);
                if (sprite_collision(&snake_head.sprite, &food_sprite)){
                    movefood(&food_sprite, 10, 10);
                }
                break;
                
            case J_RIGHT:
                move_snake(&snake_head, 8, 0);
                if (sprite_collision(&snake_head.sprite, &food_sprite)){
                    movefood(&food_sprite, -8, 8);
                }
                break;

            default:
                break;
        }

        // wait(5);
        delay(100);
    }
}
