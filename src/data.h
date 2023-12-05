#include <gb/gb.h>

extern const unsigned char level1_background_colliders [];
extern const unsigned char level1_tiles[];
extern const short level1_ntiles;
extern const unsigned char level1_map[];
extern const unsigned char level1_titlescreen[];

extern const unsigned char level2_background_colliders [];
extern const unsigned char level2_tiles[];
extern const short level2_ntiles;
extern const unsigned char level2_map[];
extern const unsigned char level2_titlescreen[];

extern const unsigned char level3_background_colliders [];
extern const unsigned char level3_tiles[];
extern const short level3_ntiles;
extern const unsigned char level3_map[];
extern const unsigned char level3_titlescreen[];

extern const unsigned char level4_background_colliders [];
extern const unsigned char level4_tiles[];
extern const short level4_ntiles;
extern const unsigned char level4_map[];
extern const unsigned char level4_titlescreen[];

extern const unsigned char win_titlescreen[];
extern const unsigned char gameover_titlescreen[];
extern const unsigned char restart_titlescreen[];
extern const unsigned char game_titlescreen[];

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

struct LevelData{
  unsigned char * tiles;
  unsigned char * map;
  unsigned char * background_colliders;
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
