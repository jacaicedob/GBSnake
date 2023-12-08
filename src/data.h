#include <gb/gb.h>

extern const unsigned char level1_background_colliders [];
extern const unsigned char level1_tiles[];
extern const unsigned char level1_ntiles;
extern const unsigned char level1_map[];
extern const unsigned char level1_titlescreen[];

extern const unsigned char level2_background_colliders [];
extern const unsigned char level2_tiles[];
extern const unsigned char level2_ntiles;
extern const unsigned char level2_map[];
extern const unsigned char level2_titlescreen[];

extern const unsigned char level3_background_colliders [];
extern const unsigned char level3_tiles[];
extern const unsigned char level3_ntiles;
extern const unsigned char level3_map[];
extern const unsigned char level3_titlescreen[];

extern const unsigned char level4_background_colliders [];
extern const unsigned char level4_tiles[];
extern const unsigned char level4_ntiles;
extern const unsigned char level4_map[];
extern const unsigned char level4_titlescreen[];

extern const unsigned char win_titlescreen[];
extern const unsigned char gameover_titlescreen[];
extern const unsigned char restart_titlescreen[];
extern const unsigned char game_titlescreen[];

extern const uint16_t frequencies[];
extern const uint8_t ch1_intro_music[];
extern const uint8_t ch2_intro_music[];
extern const uint8_t ch1_main_music[];
extern const uint8_t ch2_main_music[];

BANKREF_EXTERN(level1_background_colliders)
BANKREF_EXTERN(level1_tiles)
BANKREF_EXTERN(level1_ntiles)
BANKREF_EXTERN(level1_map)
BANKREF_EXTERN(level1_titlescreen)
BANKREF_EXTERN(level2_background_colliders)
BANKREF_EXTERN(level2_tiles)
BANKREF_EXTERN(level2_ntiles)
BANKREF_EXTERN(level2_map)
BANKREF_EXTERN(level2_titlescreen)
BANKREF_EXTERN(level3_background_colliders)
BANKREF_EXTERN(level3_tiles)
BANKREF_EXTERN(level3_ntiles)
BANKREF_EXTERN(level3_map)
BANKREF_EXTERN(level3_titlescreen)
BANKREF_EXTERN(level4_background_colliders)
BANKREF_EXTERN(level4_tiles)
BANKREF_EXTERN(level4_ntiles)
BANKREF_EXTERN(level4_map)
BANKREF_EXTERN(level4_titlescreen)

BANKREF_EXTERN(win_titlescreen)
BANKREF_EXTERN(gameover_titlescreen)
BANKREF_EXTERN(restart_titlescreen)
BANKREF_EXTERN(game_titlescreen)

BANKREF_EXTERN(frequencies)
BANKREF_EXTERN(ch1_intro_music)
BANKREF_EXTERN(ch2_intro_music)
BANKREF_EXTERN(ch1_main_music)
BANKREF_EXTERN(ch2_main_music)

struct LevelData{
  unsigned char * tiles;
  unsigned char * map;
  unsigned char * background_colliders;
  unsigned char ntiles;
  unsigned char head_startx;
  unsigned char head_starty;
  unsigned char head_dir;
  unsigned char food_startx;
  unsigned char food_starty;
  unsigned char left_boundary;
  unsigned char right_boundary;
  unsigned char top_boundary;
  unsigned char bottom_boundary;
  unsigned char next_level_len;
  unsigned char start_speed;
  unsigned char speedup;
  unsigned char speed_increase_len;
  unsigned char* titlescreen;
  char food_timer;
};
