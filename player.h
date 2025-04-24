#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>

class Player {
private:
    // Vị trí và kích thước nhân vật
    SDL_Rect rect;

    // Sức khỏe
    int health;
    const int maxHealth;

    // Trạng thái vật lý
    int verticalVelocity;
    bool isJumping;
    bool isDoubleJumping;
    bool isOnGround;
    bool facingRight;

    // Trạng thái hành động
    bool isAttacking;
    bool isBlocking;
    bool isDashing;
    bool canDash;
    bool isTakingDamage;
    bool isDead;

    // Trạng thái hoạt ảnh
    int currentRunFrame;
    int currentAttackFrame;
    int currentJumpFrame;
    int currentDamageFrame;
    int currentDeathFrame;
    Uint32 lastFrameTime;

    // Dash
    int dashSpeed;
    int dashDuration;
    Uint32 dashStartTime;

    // Texture hoạt ảnh
    SDL_Texture* idleTexture;
    SDL_Texture* runSheet;
    SDL_Texture* attackSheet;
    SDL_Texture* jumpSheet;
    SDL_Texture* blockSheet;
    SDL_Texture* damageSheet;
    SDL_Texture* deathSheet;

public:
    Player(int x, int y, SDL_Texture* idle, SDL_Texture* run, SDL_Texture* attack, SDL_Texture* jump,
           SDL_Texture* block, SDL_Texture* damage, SDL_Texture* death);
    void HandleInput(SDL_Event& e);
    void Update(SDL_Rect* platforms, int platformCount);
    void Render(SDL_Renderer* renderer);
    void TakeDamage(int amount);
    void Reset();
    SDL_Rect& GetRect() { return rect; }
    int GetHealth() const { return health; }
    bool IsAttacking() const { return isAttacking; }
    bool IsBlocking() const { return isBlocking; }
    bool IsDead() const { return isDead; }
};

#endif
