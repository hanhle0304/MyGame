// boss.cpp
#include "boss.h"
#include <SDL.h>
#include <iostream>

Boss::Boss(int x, int y, SDL_Texture* run, SDL_Texture* attack, SDL_Texture* jump, SDL_Texture* block)
    : runSheet(run), attackSheet(attack), jumpSheet(jump), blockSheet(block),
      rect{x, 500 - RUN_FRAME_HEIGHT, RUN_FRAME_WIDTH, RUN_FRAME_HEIGHT},
      verticalVelocity(0), isJumping(false), isDoubleJumping(false), isAttacking(false),
      isBlocking(false), isOnGround(true), isDashing(false), canDash(true), facingRight(true),
      currentRunFrame(0), currentAttackFrame(0), currentJumpFrame(0), currentIdleFrame(0), lastFrameTime(0),
      dashSpeed(10), dashDuration(200), dashStartTime(0), health(100), maxHealth(100) {}

void Boss::Update(const SDL_Rect& playerRect, const SDL_Rect* platforms, int platformCount) {
    this->playerRect = playerRect;

    verticalVelocity += GRAVITY;
    rect.y += verticalVelocity;

    isOnGround = false;
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
            break;
        }
    }

    if (rect.y + rect.h >= 500) {
        rect.y = 500 - rect.h;
        verticalVelocity = 0;
        isOnGround = true;
        isJumping = false;
        isDoubleJumping = false;
    }

    if (isJumping && isOnGround) {
        verticalVelocity = JUMP_STRENGTH;
        isJumping = false;
    }

    if (!isAttacking && isOnGround && abs(playerRect.x - rect.x) < 300 && rand() % 100 < 2) {
        isAttacking = true;
        currentAttackFrame = 0;
        lastFrameTime = SDL_GetTicks();
    }
    if (!isAttacking && !isBlocking && rand() % 100 < 5) {
        isBlocking = true;
        lastFrameTime = SDL_GetTicks();
        }
    if (isBlocking && SDL_GetTicks() - lastFrameTime > 500) { // Chặn trong 0.5 giây
        isBlocking = false;
        }
}

void Boss::Render(SDL_Renderer* renderer, SDL_Texture* idleTexture, Uint32 currentTime) {
    SDL_Rect srcRect, destRect;
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_Texture* texture = idleTexture;

    if (isAttacking) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentAttackFrame++;
            if (currentAttackFrame >= ATTACK_FRAME_COUNT) {
                isAttacking = false;
                currentAttackFrame = 0;
            }
        }
        texture = attackSheet;
        srcRect = {currentAttackFrame * ATTACK_FRAME_WIDTH, 0, ATTACK_FRAME_WIDTH, ATTACK_FRAME_HEIGHT};
    } else if (isJumping || isDoubleJumping) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentJumpFrame = (currentJumpFrame + 1) % JUMP_FRAME_COUNT;
        }
        texture = jumpSheet;
        srcRect = {currentJumpFrame * JUMP_FRAME_WIDTH, 0, JUMP_FRAME_WIDTH, JUMP_FRAME_HEIGHT};
    } else if (isBlocking) {
        texture = blockSheet;
        srcRect = {0, 0, BLOCK_FRAME_WIDTH, BLOCK_FRAME_HEIGHT};
    } else if (!isOnGround) {
        texture = jumpSheet;
        srcRect = {currentJumpFrame * JUMP_FRAME_WIDTH, 0, JUMP_FRAME_WIDTH, JUMP_FRAME_HEIGHT};
    } else {
        bool isMoving = false;
        if (playerRect.x < rect.x - 50) {
            rect.x -= SPEED;
            facingRight = false;
            isMoving = true;
        } else if (playerRect.x > rect.x + 50) {
            rect.x += SPEED;
            facingRight = true;
            isMoving = true;
        }

        if (isMoving) {
            if (currentTime > lastFrameTime + FRAME_DELAY) {
                lastFrameTime = currentTime;
                currentRunFrame = (currentRunFrame + 1) % RUN_FRAME_COUNT;
            }
            texture = runSheet;
            srcRect = {currentRunFrame * RUN_FRAME_WIDTH, 0, RUN_FRAME_WIDTH, RUN_FRAME_HEIGHT};
        } else {
            if (currentTime > lastFrameTime + FRAME_DELAY) {
                lastFrameTime = currentTime;
                currentIdleFrame = (currentIdleFrame + 1) % IDLE_FRAME_COUNT;
            }
            texture = idleTexture;
            srcRect = {currentIdleFrame * RUN_FRAME_WIDTH, 0, RUN_FRAME_WIDTH, RUN_FRAME_HEIGHT};
        }
    }

    destRect = {rect.x, rect.y, RUN_FRAME_WIDTH, RUN_FRAME_HEIGHT};
    SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0, nullptr, flip);
}
