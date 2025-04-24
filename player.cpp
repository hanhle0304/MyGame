#include "player.h"
#include <SDL.h>

// Constants
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;
const int PLAYER_SPEED = 5;
const int GRAVITY = 1;
const int JUMP_STRENGTH = -18;

// Player frame counts
const int RUN_FRAME_COUNT = 8;
const int ATTACK_FRAME_COUNT = 6;
const int JUMP_FRAME_COUNT = 12;
const int BLOCK_FRAME_COUNT = 2;
const int DAMAGE_FRAME_COUNT = 2;
const int DEATH_FRAME_COUNT = 3;

// Player frame dimensions
const int RUN_FRAME_WIDTH = 128;
const int RUN_FRAME_HEIGHT = 128;
const int ATTACK_FRAME_WIDTH = 128;
const int ATTACK_FRAME_HEIGHT = 128;
const int JUMP_FRAME_WIDTH = 128;
const int JUMP_FRAME_HEIGHT = 128;
const int BLOCK_FRAME_WIDTH = 128;
const int BLOCK_FRAME_HEIGHT = 128;
const int DAMAGE_FRAME_WIDTH = 128;
const int DAMAGE_FRAME_HEIGHT = 128;
const int DEATH_FRAME_WIDTH = 128;
const int DEATH_FRAME_HEIGHT = 128;

const Uint32 FRAME_DELAY = 70;

Player::Player(int x, int y, SDL_Texture* idle, SDL_Texture* run, SDL_Texture* attack, SDL_Texture* jump,
               SDL_Texture* block, SDL_Texture* damage, SDL_Texture* death)
    : rect{x, y, 120, 120}, health(5), maxHealth(5), verticalVelocity(0),
      isJumping(false), isDoubleJumping(false), isOnGround(false), facingRight(true),
      isAttacking(false), isBlocking(false), isDashing(false), canDash(true),
      isTakingDamage(false), isDead(false),
      currentRunFrame(0), currentAttackFrame(0), currentJumpFrame(0),
      currentDamageFrame(0), currentDeathFrame(0), lastFrameTime(0),
      dashSpeed(15), dashDuration(200), dashStartTime(0),
      idleTexture(idle), runSheet(run), attackSheet(attack), jumpSheet(jump),
      blockSheet(block), damageSheet(damage), deathSheet(death) {}

void Player::HandleInput(SDL_Event& e) {
    if (isDead) return;

    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_a:
                facingRight = false;
                break;
            case SDLK_d:
                facingRight = true;
                break;
            case SDLK_SPACE:
                if (isOnGround) {
                    isJumping = true;
                    verticalVelocity = JUMP_STRENGTH;
                    isOnGround = false;
                } else if (!isDoubleJumping) {
                    isDoubleJumping = true;
                    verticalVelocity = JUMP_STRENGTH;
                }
                break;
            case SDLK_j:
                if (!isAttacking && !isTakingDamage) {
                    isAttacking = true;
                    currentAttackFrame = 0;
                    lastFrameTime = SDL_GetTicks();
                }
                break;
            case SDLK_k:
                isBlocking = true;
                break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                if (!isDashing && canDash) {
                    isDashing = true;
                    dashStartTime = SDL_GetTicks();
                    canDash = false;
                }
                break;
        }
    } else if (e.type == SDL_KEYUP) {
        switch (e.key.keysym.sym) {
            case SDLK_k:
                isBlocking = false;
                break;
        }
    }
}

void Player::Update(SDL_Rect* platforms, int platformCount) {
    if (isDead) return;

    // Di chuyển ngang
    bool moveLeft = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A];
    bool moveRight = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D];

    if (!isBlocking && !isTakingDamage) {
        if (isDashing) {
            int speed = dashSpeed;
            if (facingRight && rect.x + rect.w + speed < SCREEN_WIDTH) rect.x += speed;
            else if (!facingRight && rect.x - speed > 0) rect.x -= speed;
            if (SDL_GetTicks() - dashStartTime > dashDuration) isDashing = false;
        } else {
            if (moveLeft && rect.x > 0) rect.x -= PLAYER_SPEED;
            if (moveRight && rect.x + rect.w < SCREEN_WIDTH) rect.x += PLAYER_SPEED;
        }
    }

    // Vật lý
    verticalVelocity += GRAVITY;
    rect.y += verticalVelocity;

    // Kiểm tra va chạm với nền tảng
    bool onPlatform = false;
    for (int i = 0; i < platformCount; ++i) {
        if (rect.y + rect.h <= platforms[i].y + 20 &&
            rect.y + rect.h + verticalVelocity >= platforms[i].y &&
            rect.x + rect.w > platforms[i].x && rect.x < platforms[i].x + platforms[i].w &&
            verticalVelocity > 0) {
            rect.y = platforms[i].y - rect.h;
            verticalVelocity = 0;
            isOnGround = true;
            isJumping = false;
            isDoubleJumping = false;
            canDash = true;
            onPlatform = true;
            break;
        }
    }

    // Kiểm tra chạm mặt đất
    if (rect.y + rect.h >= 500 && !onPlatform) {
        rect.y = 500 - rect.h;
        verticalVelocity = 0;
        isJumping = false;
        isDoubleJumping = false;
        isOnGround = true;
        canDash = true;
    }
}

void Player::Render(SDL_Renderer* renderer) {
    Uint32 currentTime = SDL_GetTicks();
    SDL_Rect srcRect, destRect;
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    if (isDead) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentDeathFrame++;
        }
        srcRect = {currentDeathFrame * DEATH_FRAME_WIDTH, 0, DEATH_FRAME_WIDTH, DEATH_FRAME_HEIGHT};
        destRect = {rect.x + (rect.w - DEATH_FRAME_WIDTH) / 2, rect.y + (rect.h - DEATH_FRAME_HEIGHT), DEATH_FRAME_WIDTH, DEATH_FRAME_HEIGHT};
        SDL_RenderCopyEx(renderer, deathSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isTakingDamage) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentDamageFrame++;
            if (currentDamageFrame >= DAMAGE_FRAME_COUNT) {
                isTakingDamage = false;
                currentDamageFrame = 0;
            }
        }
        srcRect = {currentDamageFrame * DAMAGE_FRAME_WIDTH, 0, DAMAGE_FRAME_WIDTH, DAMAGE_FRAME_HEIGHT};
        destRect = {rect.x + (rect.w - DAMAGE_FRAME_WIDTH) / 2, rect.y + (rect.h - DAMAGE_FRAME_HEIGHT), DAMAGE_FRAME_WIDTH, DAMAGE_FRAME_HEIGHT};
        SDL_RenderCopyEx(renderer, damageSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isBlocking) {
        srcRect = {0, 0, BLOCK_FRAME_WIDTH, BLOCK_FRAME_HEIGHT};
        destRect = {rect.x + (rect.w - BLOCK_FRAME_WIDTH) / 2, rect.y + (rect.h - BLOCK_FRAME_HEIGHT), BLOCK_FRAME_WIDTH, BLOCK_FRAME_HEIGHT};
        SDL_RenderCopyEx(renderer, blockSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isAttacking) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentAttackFrame++;
            if (currentAttackFrame >= ATTACK_FRAME_COUNT) {
                isAttacking = false;
                currentAttackFrame = 0;
            }
        }
        srcRect = {currentAttackFrame * ATTACK_FRAME_WIDTH, 0, ATTACK_FRAME_WIDTH, ATTACK_FRAME_HEIGHT};
        destRect = {rect.x + (rect.w - ATTACK_FRAME_WIDTH) / 2, rect.y + (rect.h - ATTACK_FRAME_HEIGHT), ATTACK_FRAME_WIDTH, ATTACK_FRAME_HEIGHT};
        SDL_RenderCopyEx(renderer, attackSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isJumping || isDoubleJumping) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentJumpFrame = (currentJumpFrame + 1) % JUMP_FRAME_COUNT;
        }
        srcRect = {currentJumpFrame * JUMP_FRAME_WIDTH, 0, JUMP_FRAME_WIDTH, JUMP_FRAME_HEIGHT};
        destRect = {rect.x + (rect.w - JUMP_FRAME_WIDTH) / 2, rect.y + (rect.h - JUMP_FRAME_HEIGHT), JUMP_FRAME_WIDTH, JUMP_FRAME_HEIGHT};
        SDL_RenderCopyEx(renderer, jumpSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A] || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D]) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentRunFrame = (currentRunFrame + 1) % RUN_FRAME_COUNT;
        }
        srcRect = {currentRunFrame * RUN_FRAME_WIDTH, 0, RUN_FRAME_WIDTH, RUN_FRAME_HEIGHT};
        destRect = {rect.x + (rect.w - RUN_FRAME_WIDTH) / 2, rect.y + (rect.h - RUN_FRAME_HEIGHT), RUN_FRAME_WIDTH, RUN_FRAME_HEIGHT};
        SDL_RenderCopyEx(renderer, runSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else {
        SDL_RenderCopyEx(renderer, idleTexture, nullptr, &rect, 0, nullptr, flip);
    }
}

void Player::TakeDamage(int amount) {
    if (isBlocking || isDead) return;
    health -= amount;
    if (health <= 0) {
        health = 0;
        isDead = true;
        currentDeathFrame = 0;
        lastFrameTime = SDL_GetTicks();
    } else {
        isTakingDamage = true;
        currentDamageFrame = 0;
        lastFrameTime = SDL_GetTicks();
    }
}

void Player::Reset() {
    rect = {120, 400, 120, 120};
    health = maxHealth;
    verticalVelocity = 0;
    isJumping = false;
    isDoubleJumping = false;
    isOnGround = false;
    isAttacking = false;
    isBlocking = false;
    isDashing = false;
    canDash = true;
    facingRight = true;
    isTakingDamage = false;
    isDead = false;
    currentRunFrame = 0;
    currentAttackFrame = 0;
    currentJumpFrame = 0;
    currentDamageFrame = 0;
    currentDeathFrame = 0;
    lastFrameTime = 0;
}
