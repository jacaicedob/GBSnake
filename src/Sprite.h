#include <gb/gb.h>
#include <stdint.h>

struct Sprite {
    uint8_t spriteid;
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
};

struct SnakePart {
    struct Sprite sprite;
    struct SnakePart* next;
};

struct BackgroundObstacle {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    struct BackgroundObstacle* next;
};
