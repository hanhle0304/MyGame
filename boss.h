#ifndef BOSS_H
#define BOSS_H

#include <SDL.h>
#include <vector>

class Player; // Forward declaration

struct Arrow {
    SDL_Rect rect;
    int velocity;
    bool facingRight;
    int frameWidth;
    int frameHeight;
    Arrow(int x, int y, bool facingRight, int fWidth, int fHeight)
        : rect{x, y, fWidth, fHeight}, velocity(facingRight ? 12 : -12), facingRight(facingRight),
          frameWidth(fWidth), frameHeight(fHeight) {}
};

class Boss {
protected:
    SDL_Rect rect;
    int health;
    int maxHealth;
    int verticalVelocity;
    int horizontalDiveVelocity;
    bool isJumping;
    bool isOnGround;
    bool facingRight;
    bool isAttacking;
    bool isDashing;
    bool isDiving;
    bool isTakingDamage;
    bool isDead;
    bool isIdle;
    bool isRetreating;
    bool hasDealtDamage;
    int currentRunFrame;
    int currentAttackFrame;
    int currentJumpFrame;
    int currentDamageFrame;
    int currentDeathFrame;
    int retreatStartX;
    Uint32 lastFrameTime;
    Uint32 dashStartTime;
    Uint32 diveStartTime;
    Uint32 lastAttackTime;

    SDL_Texture* runSheet;
    SDL_Texture* attackSheet;
    SDL_Texture* jumpSheet;
    SDL_Texture* damageSheet;
    SDL_Texture* deathSheet;
    SDL_Texture* diveSheet;

    int runFrameCount;
    int attackFrameCount;
    int jumpFrameCount;
    int damageFrameCount;
    int deathFrameCount;
    int diveFrameCount;

    int runFrameWidth;
    int runFrameHeight;
    int attackFrameWidth;
    int attackFrameHeight;
    int jumpFrameWidth;
    int jumpFrameHeight;
    int damageFrameWidth;
    int damageFrameHeight;
    int deathFrameWidth;
    int deathFrameHeight;
    int diveFrameWidth;
    int diveFrameHeight;

    static const Uint32 ATTACK_COOLDOWN = 2000;
    static const Uint32 SHOOT_COOLDOWN = 2000;
    static const int IDLE_DISTANCE = 600;
    static const int RETREAT_DISTANCE = 400;

    std::vector<Boss*> miniBosses;
    bool hasSummonedMiniBoss;

    void RenderHealthBar(SDL_Renderer* renderer); // Phương thức vẽ thanh máu

public:
    Boss(int x, int y,
         SDL_Texture* run, int runCount, int runWidth, int runHeight,
         SDL_Texture* attack, int attackCount, int attackWidth, int attackHeight,
         SDL_Texture* jump, int jumpCount, int jumpWidth, int jumpHeight,
         SDL_Texture* damage, int damageCount, int damageWidth, int damageHeight,
         SDL_Texture* death, int deathCount, int deathWidth, int deathHeight,
         SDL_Texture* dive, int diveCount, int diveWidth, int diveHeight);
    virtual ~Boss();
    virtual void Update(const SDL_Rect& playerRect, SDL_Rect* platforms, int platformCount, int currentLevel, Player& player);
    virtual void Render(SDL_Renderer* renderer, SDL_Texture* idleTexture, Uint32 currentTime);
    void ReduceHealth(int amount);
    void SetDealtDamage(bool value) { hasDealtDamage = value; }
    bool HasDealtDamage() const { return hasDealtDamage; }
    SDL_Rect& GetRect() { return rect; }
    int GetHealth() const { return health; }
    int GetMaxHealth() const { return maxHealth; }
    bool IsAttacking() const { return isAttacking; }
    bool IsDashing() const { return isDashing; }
    bool IsDiving() const { return isDiving; }
};

class MiniBoss : public Boss {
private:
    bool isShooting;
    Uint32 shootStartTime;
    std::vector<Arrow> arrows;
    SDL_Texture* arrowTexture;
    SDL_Texture* shootSheet;
    int shootFrameCount;
    int shootFrameWidth;
    int shootFrameHeight;

public:
    MiniBoss(int x, int y,
             SDL_Texture* run, int runCount, int runWidth, int runHeight,
             SDL_Texture* attack, int attackCount, int attackWidth, int attackHeight,
             SDL_Texture* jump, int jumpCount, int jumpWidth, int jumpHeight,
             SDL_Texture* damage, int damageCount, int damageWidth, int damageHeight,
             SDL_Texture* death, int deathCount, int deathWidth, int deathHeight,
             SDL_Texture* dive, int diveCount, int diveWidth, int diveHeight,
             SDL_Texture* shoot, int shootCount, int shootWidth, int shootHeight,
             SDL_Texture* arrow, int arrowWidth, int arrowHeight);
    void Update(const SDL_Rect& playerRect, SDL_Rect* platforms, int platformCount, int currentLevel, Player& player) override;
    void Render(SDL_Renderer* renderer, SDL_Texture* idleTexture, Uint32 currentTime) override;
};

#endif
