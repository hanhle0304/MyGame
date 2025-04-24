#ifndef BOSS_H
#define BOSS_H

#include <SDL.h>

class Boss {
private:
    SDL_Rect rect;
    int health;
    int maxHealth;
    int verticalVelocity;
    bool isJumping;
    bool isOnGround;
    bool facingRight;
    bool isAttacking;
    bool isBlocking;
    bool isTakingDamage;
    bool isDead;
    bool hasDealtDamage;
    int currentRunFrame;
    int currentAttackFrame;
    int currentJumpFrame;
    int currentBlockFrame;
    int currentDamageFrame;
    int currentDeathFrame;
    Uint32 lastFrameTime;

    // Texture cho các hành động
    SDL_Texture* runSheet;
    SDL_Texture* attackSheet;
    SDL_Texture* jumpSheet;
    SDL_Texture* blockSheet;
    SDL_Texture* damageSheet;
    SDL_Texture* deathSheet;

    // Số lượng khung hình cho mỗi hành động
    int runFrameCount;
    int attackFrameCount;
    int jumpFrameCount;
    int blockFrameCount;
    int damageFrameCount;
    int deathFrameCount;

    // Kích thước khung hình cho mỗi hành động
    int runFrameWidth;
    int runFrameHeight;
    int attackFrameWidth;
    int attackFrameHeight;
    int jumpFrameWidth;
    int jumpFrameHeight;
    int blockFrameWidth;
    int blockFrameHeight;
    int damageFrameWidth;
    int damageFrameHeight;
    int deathFrameWidth;
    int deathFrameHeight;

public:
    Boss(int x, int y,
         SDL_Texture* run, int runCount, int runWidth, int runHeight,
         SDL_Texture* attack, int attackCount, int attackWidth, int attackHeight,
         SDL_Texture* jump, int jumpCount, int jumpWidth, int jumpHeight,
         SDL_Texture* block, int blockCount, int blockWidth, int blockHeight,
         SDL_Texture* damage, int damageCount, int damageWidth, int damageHeight,
         SDL_Texture* death, int deathCount, int deathWidth, int deathHeight);
    void Update(const SDL_Rect& playerRect, SDL_Rect* platforms, int platformCount);
    void Render(SDL_Renderer* renderer, SDL_Texture* idleTexture, Uint32 currentTime);
    void ReduceHealth(int amount);
    void SetDealtDamage(bool value) { hasDealtDamage = value; }
    bool HasDealtDamage() const { return hasDealtDamage; }
    SDL_Rect& GetRect() { return rect; }
    int GetHealth() const { return health; }
    int GetMaxHealth() const { return maxHealth; }
    bool IsAttacking() const { return isAttacking; }
    bool IsBlocking() const { return isBlocking; }
};

#endif
