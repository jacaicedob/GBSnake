#include <gb/gb.h>

struct Sprite {
    UBYTE spriteid;
    UINT8 x;
    UINT8 y;
    UINT8 width;
    UINT8 height;
};

struct SnakePart {
    struct Sprite sprite;
    struct SnakePart* next;
};

struct BackgroundObstacle {
    UINT8 x;
    UINT8 y;
    UINT8 width;
    UINT8 height;
    struct BackgroundObstacle* next;
};
