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
    bool isDashing;
    bool canDash;
    bool isTakingDamage;
    bool isDead;
    bool isInvulnerable; // Thêm: trạng thái miễn nhiễm


    // Trạng thái hoạt ảnh
    int currentRunFrame;
    int currentAttackFrame;
    int currentJumpFrame;
    int currentDamageFrame;
    int currentDeathFrame;
    Uint32 lastFrameTime;
    Uint32 invulnerabilityStartTime; // Thêm: thời điểm bắt đầu miễn nhiễm
    static const Uint32 INVULNERABILITY_DURATION = 1500; // 500ms miễn nhiễm

    // Dash
    int dashSpeed;
    int dashDuration;
    Uint32 dashStartTime;

    // Texture hoạt ảnh
    SDL_Texture* idleTexture;
    SDL_Texture* runSheet;
    SDL_Texture* attackSheet;
    SDL_Texture* jumpSheet;
    SDL_Texture* damageSheet;
    SDL_Texture* deathSheet;

public:
    Player(int x, int y, SDL_Texture* idle, SDL_Texture* run, SDL_Texture* attack, SDL_Texture* jump,
           SDL_Texture* damage, SDL_Texture* death);
    void HandleInput(SDL_Event& e);
    void Update(SDL_Rect* platforms, int platformCount);
    void Render(SDL_Renderer* renderer);
    void TakeDamage(int amount);
    void Reset();
    SDL_Rect& GetRect() { return rect; }
    int GetHealth() const { return health; }
    bool IsAttacking() const { return isAttacking; }
    bool IsDead() const { return isDead; }
    bool IsInvulnerable() const { return isInvulnerable; } // Thêm: kiểm tra miễn nhiễm

};

#endif
