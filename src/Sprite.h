#include <gb/gb.h>
#include <stdint.h>

struct Sprite {
    uint8_t spriteid;
    uint8_t x;
    uint8_t y;
    uint8_t size;
    uint8_t timer;
    uint8_t animation_frame;
    char animation_state;
};

struct SnakePart {
    struct Sprite sprite;
    struct SnakePart* next;
};

struct BackgroundObstacle {
    uint8_t x;
    uint8_t y;
    uint8_t size;
    // uint8_t width;
    // uint8_t height;
    struct BackgroundObstacle* next;
};
