#include "boss.h"
#include <SDL.h>
#include <iostream>

// Constants
const int SCREEN_WIDTH = 1200;
const int BOSS_SPEED = 3;
const int GRAVITY = 1;
const int JUMP_STRENGTH = -15;
const Uint32 FRAME_DELAY = 70;

Boss::Boss(int x, int y,
           SDL_Texture* run, int runCount, int runWidth, int runHeight,
           SDL_Texture* attack, int attackCount, int attackWidth, int attackHeight,
           SDL_Texture* jump, int jumpCount, int jumpWidth, int jumpHeight,
           SDL_Texture* block, int blockCount, int blockWidth, int blockHeight,
           SDL_Texture* damage, int damageCount, int damageWidth, int damageHeight,
           SDL_Texture* death, int deathCount, int deathWidth, int deathHeight)
    : rect{x, y, 120, 120}, health(100), maxHealth(100), verticalVelocity(0),
      isJumping(false), isOnGround(false), facingRight(false),
      isAttacking(false), isBlocking(false), isTakingDamage(false), isDead(false),
      hasDealtDamage(false), currentRunFrame(0), currentAttackFrame(0),
      currentJumpFrame(0), currentBlockFrame(0), currentDamageFrame(0),
      currentDeathFrame(0), lastFrameTime(0),
      runSheet(run), runFrameCount(runCount), runFrameWidth(runWidth), runFrameHeight(runHeight),
      attackSheet(attack), attackFrameCount(attackCount), attackFrameWidth(attackWidth), attackFrameHeight(attackHeight),
      jumpSheet(jump), jumpFrameCount(jumpCount), jumpFrameWidth(jumpWidth), jumpFrameHeight(jumpHeight),
      blockSheet(block), blockFrameCount(blockCount), blockFrameWidth(blockWidth), blockFrameHeight(blockHeight),
      damageSheet(damage), damageFrameCount(damageCount), damageFrameWidth(damageWidth), damageFrameHeight(damageHeight),
      deathSheet(death), deathFrameCount(deathCount), deathFrameWidth(deathWidth), deathFrameHeight(deathHeight) {}

void Boss::Update(const SDL_Rect& playerRect, SDL_Rect* platforms, int platformCount) {
    if (isDead) return;

    // Di chuyển về phía player
    if (playerRect.x < rect.x && rect.x > 0) {
        rect.x -= BOSS_SPEED;
        facingRight = false;
    } else if (playerRect.x > rect.x && rect.x + rect.w < SCREEN_WIDTH) {
        rect.x += BOSS_SPEED;
        facingRight = true;
    }

    // Quyết định hành động ngẫu nhiên
    if (!isAttacking && !isBlocking && !isTakingDamage && !isJumping) {
        int action = rand() % 100;
        if (action < 30 && isOnGround) { // Tăng xác suất tấn công
            isAttacking = true;
            currentAttackFrame = 0;
            lastFrameTime = SDL_GetTicks();
            hasDealtDamage = false; // Đặt lại để có thể gây sát thương
        } else if (action < 50 && isOnGround) {
            isBlocking = true;
            currentBlockFrame = 0;
            lastFrameTime = SDL_GetTicks();
        } else if (action < 70 && isOnGround) {
            isJumping = true;
            verticalVelocity = JUMP_STRENGTH;
            isOnGround = false;
            currentJumpFrame = 0;
            lastFrameTime = SDL_GetTicks();
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
            onPlatform = true;
            break;
        }
    }

    // Kiểm tra chạm mặt đất
    if (rect.y + rect.h >= 500 && !onPlatform) {
        rect.y = 500 - rect.h;
        verticalVelocity = 0;
        isJumping = false;
        isOnGround = true;
    }

    // Đặt lại trạng thái sau khi hoàn thành hành động
    if (isAttacking && currentAttackFrame >= attackFrameCount - 1) {
        isAttacking = false;
        currentAttackFrame = 0;
    }
    if (isBlocking && currentBlockFrame >= blockFrameCount - 1) {
        isBlocking = false;
        currentBlockFrame = 0;
    }
}

void Boss::Render(SDL_Renderer* renderer, SDL_Texture* idleTexture, Uint32 currentTime) {
    SDL_Rect srcRect, destRect;
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    if (isDead) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentDeathFrame++;
        }
        srcRect = {currentDeathFrame * deathFrameWidth, 0, deathFrameWidth, deathFrameHeight};
        destRect = {rect.x + (rect.w - deathFrameWidth) / 2, rect.y + (rect.h - deathFrameHeight), deathFrameWidth, deathFrameHeight};
        SDL_RenderCopyEx(renderer, deathSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isTakingDamage) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentDamageFrame++;
            if (currentDamageFrame >= damageFrameCount) {
                isTakingDamage = false;
                currentDamageFrame = 0;
            }
        }
        srcRect = {currentDamageFrame * damageFrameWidth, 0, damageFrameWidth, damageFrameHeight};
        destRect = {rect.x + (rect.w - damageFrameWidth) / 2, rect.y + (rect.h - damageFrameHeight), damageFrameWidth, damageFrameHeight};
        SDL_RenderCopyEx(renderer, damageSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isBlocking) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentBlockFrame = (currentBlockFrame + 1) % blockFrameCount;
        }
        srcRect = {currentBlockFrame * blockFrameWidth, 0, blockFrameWidth, blockFrameHeight};
        destRect = {rect.x + (rect.w - blockFrameWidth) / 2, rect.y + (rect.h - blockFrameHeight), blockFrameWidth, blockFrameHeight};
        SDL_RenderCopyEx(renderer, blockSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isAttacking) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentAttackFrame++;
        }
        srcRect = {currentAttackFrame * attackFrameWidth, 0, attackFrameWidth, attackFrameHeight};
        destRect = {rect.x + (rect.w - attackFrameWidth) / 2, rect.y + (rect.h - attackFrameHeight), attackFrameWidth, attackFrameHeight};
        SDL_RenderCopyEx(renderer, attackSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isJumping) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentJumpFrame = (currentJumpFrame + 1) % jumpFrameCount;
        }
        srcRect = {currentJumpFrame * jumpFrameWidth, 0, jumpFrameWidth, jumpFrameHeight};
        destRect = {rect.x + (rect.w - jumpFrameWidth) / 2, rect.y + (rect.h - jumpFrameHeight), jumpFrameWidth, jumpFrameHeight};
        SDL_RenderCopyEx(renderer, jumpSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentRunFrame = (currentRunFrame + 1) % runFrameCount;
        }
        srcRect = {currentRunFrame * runFrameWidth, 0, runFrameWidth, runFrameHeight};
        destRect = {rect.x + (rect.w - runFrameWidth) / 2, rect.y + (rect.h - runFrameHeight), runFrameWidth, runFrameHeight};
        SDL_RenderCopyEx(renderer, runSheet, &srcRect, &destRect, 0, nullptr, flip);
    }
}

void Boss::ReduceHealth(int amount) {
    if (isDead) return;
    if (isBlocking) {
        std::cout << "Boss is blocking, no damage taken\n";
        return;
    }
    health -= amount;
    std::cout << "Boss health: " << health << "\n";
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
