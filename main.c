#include <gb/gb.h>
#include <gb/font.h>
#include <stdio.h>

#include "Sprite.c"
#include "snake_round.c"
#include "food.c"
#include "bkg_tiles.c"
#include "bkg_map.c"
#include "Background_data.c"
#include "Background_map.c"
#include "windowmap.c"

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
    // Interrupt-based delay.
    // Returns after n Vertical Blanking interrupts (screen refreshes)
    UINT8 x;
    for (x = 0; x < n; x++){
        wait_vbl_done();
    }
}

void game_over_screen(UINT8 score){
    UINT8 x;

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
    // waitpad("J_START");
}

void win_screen(UINT8 score){
    UINT8 x;

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
    // waitpad("J_START");
}

UBYTE sprite_collision(struct Sprite* sp1, struct Sprite* sp2){
    /* This code peforms collision between the centers of sp1 with sp2 */
    UINT8 sp1_left = sp1->x + sp1->width/2;
    UINT8 sp1_right = sp1->x + sp1->width - sp1->width/2;
    UINT8 sp1_top = sp1->y + sp1->height/2;
    UINT8 sp1_bottom = sp1->y + sp1->height;
    UINT8 sp2_left = sp2->x;
    UINT8 sp2_right = sp2->x + sp2->width;
    UINT8 sp2_top = sp2->y;
    UINT8 sp2_bottom = sp2->y + sp2->height;

    return (((sp1_left >= sp2_left) && (sp1_left <= sp2_right)) && ((sp1_top >= sp2_top) && (sp1_top <= sp2_bottom))) || (((sp2_left >= sp1_left) && (sp2_left <= sp1_right)) && ((sp2_top >= sp1_top) && (sp2_top <= sp1_bottom)));
}

UINT8 head_tail_collision(struct SnakePart* head){
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

UBYTE background_collision(UINT8 x, UINT8 y, UINT8 width, UINT8 height, struct BackgroundObstacle* bkg){
    /* This code peforms collision between the center of sp1 with 
       each element of bkg */
    UINT8 sp1_left;
    UINT8 sp1_right;
    UINT8 sp1_top;
    UINT8 sp1_bottom;
    UINT8 bkg_left;
    UINT8 bkg_right;
    UINT8 bkg_top;
    UINT8 bkg_bottom;
    UINT8 collision;

    sp1_left = x + width/2;
    sp1_right = x + width/2;
    sp1_top = y + height/2;
    sp1_bottom = y + height/2;
    
    while(1){
        bkg_left = bkg->x;
        bkg_right = bkg->x + bkg->width;
        bkg_top = bkg->y;
        bkg_bottom = bkg->y + bkg->height;
        
        if ((((sp1_left >= bkg_left) && (sp1_left <= bkg_right)) && ((sp1_top >= bkg_top) && (sp1_top <= bkg_bottom))) || (((bkg_left >= sp1_left) && (bkg_left <= sp1_right)) && ((bkg_top >= sp1_top) && (bkg_top <= sp1_bottom)))) {
            collision = 1;
            break;
        }

        if (bkg->next != NULL){
            bkg = bkg->next;
        }
        else {
            collision = 0;
            break;
        }
    }

    return collision;
}

void move_snake(struct SnakePart* head, UINT8 x, UINT8 y, struct BackgroundObstacle* bkg){
    /*
    For the snake movement, the head moves in the direction of the joypad 
    and the tail follows the head.
    */
    struct SnakePart* tail;

    // Process head movement first
    UINT8 headx;
    UINT8 heady;
    UINT8 newx;
    UINT8 newy;

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
    if (background_collision(newx, newy, head->sprite.width, head->sprite.height, bkg)){
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
    UINT8 new_spriteid;

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

void score2tile(UINT8 score, UINT8* score_tiles){
    if (score < 255){
        score_tiles[0] = (score / 100) + 0x02;
        score_tiles[1] = (score - (score/100)*100)/10 + 0x02;
        score_tiles[2] = (score - (score/10)*10) + 0x02;
    }
    else {
        score_tiles[0] = 0x02;
        score_tiles[1] = 0x02;
        score_tiles[2] = 0x02;
    }
}

void main(){
    font_t min_font;
    UINT8 snake_size = 37;
    struct SnakePart snake_tail[37];
    struct Sprite food_sprite;
    struct  BackgroundObstacle bkg_obs[6];
    
    UINT8 move_direction = J_UP;
    UINT8 jpad = 0x0;
    UINT8 wait_loop_ind;
    UINT8 speed = 40;

    UINT8 food_sprite_id = 37;
    UINT8 next_snaketail_sprite_id = 0;
    UINT8 snake_tail_ind = 0;
    UINT8 bkg_obs_ind = 0;
    UINT8 game_over = 0;
    UINT8 score = 0;
    UINT8 score_increment = 1;

    UINT8 score_tiles[3];
    score2tile(score, &score_tiles);

    /* Initialize font */
    font_init();
    min_font = font_load(font_min); // 36 tiles
    font_set(min_font);

    /* Load background */
    set_bkg_data(37, 31, Background_data);//bkg_tiles);
    set_bkg_tiles(0,0,20,18,Background_map);//bkg_map);

    /* Load window */
    set_win_tiles(0,0,6,1,scoremap);
    set_win_tiles(7,0, 3, 1, score_tiles);
    move_win(7,136);

    /* Define background obstacles */
    bkg_obs[bkg_obs_ind].x = 64;
    bkg_obs[bkg_obs_ind].y = 112;
    bkg_obs[bkg_obs_ind].width = 24;
    bkg_obs[bkg_obs_ind].height = 8;
    bkg_obs_ind++;
    // bkg_obs[bkg_obs_ind].x = 80;
    // bkg_obs[bkg_obs_ind].y = 112;
    // bkg_obs[bkg_obs_ind].width = 8;
    // bkg_obs[bkg_obs_ind].height = 8;
    // bkg_obs[bkg_obs_ind-1].next = &bkg_obs[bkg_obs_ind];
    // bkg_obs_ind++;
    // bkg_obs[bkg_obs_ind].x = 72;
    // bkg_obs[bkg_obs_ind].y = 112;
    // bkg_obs[bkg_obs_ind].width = 8;
    // bkg_obs[bkg_obs_ind].height = 8;
    // bkg_obs[bkg_obs_ind-1].next = &bkg_obs[bkg_obs_ind];
    // bkg_obs_ind++;
    bkg_obs[bkg_obs_ind].x = 72;
    bkg_obs[bkg_obs_ind].y = 104;
    bkg_obs[bkg_obs_ind].width = 8;
    bkg_obs[bkg_obs_ind].height = 8;
    bkg_obs[bkg_obs_ind-1].next = &bkg_obs[bkg_obs_ind];
    bkg_obs[bkg_obs_ind].next = NULL;
    bkg_obs_ind++;

    /* Load sprite data */
    set_sprite_data(SNAKE_MEMIND, SNAKE_NTILES, snake_round_sprite);
    set_sprite_data(FOOD_MEMIND, FOOD_NTILES, food);

    /* Create food sprite */
    food_sprite.x = 72;
    food_sprite.y = 50;
    food_sprite.width = 8;
    food_sprite.height = 8;
    food_sprite.spriteid = food_sprite_id;
    set_sprite_tile(food_sprite_id,FOOD_BISCUIT);
    set_sprite_tile(food_sprite_id+1,FOOD_CARROT);
    set_sprite_tile(food_sprite_id+2,FOOD_TURNIP);
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

    SHOW_BKG;
    SHOW_WIN;
    SHOW_SPRITES;
    DISPLAY_ON;

    while(!game_over){
        if ((move_direction & J_UP) == J_UP){
                move_snake(&snake_tail[0], 0, -8, &bkg_obs[0]);
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
                else if (head_tail_collision(&snake_tail)){
                    // Collided head with tail. End Game
                    game_over = 1;
                }  
        }                     
        else if ((move_direction & J_DOWN) == J_DOWN){
                move_snake(&snake_tail[0], 0, 8, &bkg_obs[0]);
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
                else if (head_tail_collision(&snake_tail)){
                    // Collided head with tail. End Game
                    game_over = 1;
                }
                
        }
        else if (((move_direction & J_LEFT) == J_LEFT)){
                move_snake(&snake_tail[0], -8, 0, &bkg_obs[0]);
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
                else if (head_tail_collision(&snake_tail)){
                    // Collided head with tail. End Game
                    game_over = 1;
                }
                
        }
        else if (((move_direction & J_RIGHT) == J_RIGHT)){
                move_snake(&snake_tail[0], 8, 0, &bkg_obs[0]);
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
                else if (head_tail_collision(&snake_tail)){
                    // Collided head with tail. End Game
                    game_over = 1;
                }
        }
        score_increment = 1; //(UINT8) (snake_tail_ind / 4) + 1;
        score2tile(score, &score_tiles);
        set_win_tiles(7,0, 3, 1, score_tiles);
        
        if (snake_tail_ind > 37){
            win_screen(score);
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

        // wait(40);
        // Interrupt-based delay.
        // Returns after n Vertical Blanking interrupts (screen refreshes)
        
        for (wait_loop_ind = 0; wait_loop_ind < speed; wait_loop_ind++){
            jpad = joypad();
            if (jpad != 0 && (jpad & J_A) != J_A && (jpad & J_B) != J_B && (jpad & J_SELECT) != J_SELECT){
                move_direction = jpad;
            }
            wait_vbl_done();
        }

    }
    game_over_screen(score);
}
