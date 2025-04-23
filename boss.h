#ifndef BOSS_H
#define BOSS_H

#include <SDL.h>
#include <cstdlib>
#include<algorithm>

class Boss {
private :
    SDL_Rect rect;
    SDL_Texture* runSheet;
    SDL_Texture* attackSheet;
    SDL_Texture* jumpSheet;
    SDL_Texture* blockSheet;
    int verticalVelocity;
    bool isJumping;
    bool isDoubleJumping;
    bool isAttacking;
    bool isBlocking;
    bool isOnGround;
    bool isDashing;
    bool canDash;
    bool facingRight;
    int currentIdleFrame;
    int currentRunFrame;
    int currentAttackFrame;
    int currentJumpFrame;
    Uint32 lastFrameTime;
    int dashSpeed;
    int dashDuration;
    Uint32 dashStartTime;
    int health;
    const int maxHealth;
    SDL_Rect playerRect; // Thêm để lưu vị trí người chơi
    static const int SPEED = 4;
    static const int GRAVITY = 1;
    static const int JUMP_STRENGTH = -18;
    static const Uint32 FRAME_DELAY = 70;
    static const int IDLE_FRAME_COUNT = 1;
    static const int RUN_FRAME_COUNT = 7;
    static const int ATTACK_FRAME_COUNT = 5;
    static const int JUMP_FRAME_COUNT = 6;
    static const int BLOCK_FRAME_COUNT = 1;
    static const int IDLE_FRAME_WIDTH = 128;
    static const int IDLE_FRAME_HEIGHT = 128;
    static const int RUN_FRAME_WIDTH = 128;
    static const int RUN_FRAME_HEIGHT = 128;
    static const int ATTACK_FRAME_WIDTH = 128;
    static const int ATTACK_FRAME_HEIGHT = 128;
    static const int JUMP_FRAME_WIDTH = 128;
    static const int JUMP_FRAME_HEIGHT = 128;
    static const int BLOCK_FRAME_WIDTH = 128;
    static const int BLOCK_FRAME_HEIGHT = 128;
    static const int SCREEN_WIDTH = 1200;
    static const int SCREEN_HEIGHT = 600;

public:
    Boss(int x, int y, SDL_Texture* run, SDL_Texture* attack, SDL_Texture* jump, SDL_Texture* block);
    void Update(const SDL_Rect& playerRect, const SDL_Rect* platforms, int platformCount);
    void Render(SDL_Renderer* renderer, SDL_Texture* idleTexture, Uint32 currentTime);
    SDL_Rect& GetRect() { return rect; }
    bool IsAttacking() const { return isAttacking; }
    bool IsBlocking() const { return isBlocking; }
    int GetHealth() const { return health; }
    int GetMaxHealth() const { return maxHealth; }
    void ReduceHealth(int damage) { health = std::max(0, health - damage); }
};

#endif
