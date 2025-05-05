#include "boss.h"
#include "player.h"
#include <SDL.h>
#include <iostream>
#include <cmath>

// Constants
const int SCREEN_WIDTH = 1200;
const int BOSS_SPEED = 3;
const int DASH_SPEED = 8;
const int DIVE_SPEED_VERTICAL = 8;
const int DIVE_SPEED_HORIZONTAL = 14;
const int GRAVITY = 1;
const int JUMP_STRENGTH = -20;
const Uint32 FRAME_DELAY = 500;
const Uint32 DASH_DURATION = 1000;
const Uint32 DIVE_DURATION = 2500;
const int MIN_DISTANCE = 400; // Khoảng cách tối thiểu
const int MAX_DISTANCE = 1000; // Khoảng cách tối đa
const Uint32 ATTACK_COOLDOWN = 1000;
const Uint32 SHOOT_COOLDOWN = 1500;
const int RETREAT_DISTANCE = 200;
const int IDLE_DISTANCE = 600;

// Global MiniBoss textures (khai báo extern để truy cập từ main.cpp)
extern SDL_Texture* miniBossIdle;
extern SDL_Texture* miniBossRunSheet;
extern SDL_Texture* miniBossAttackSheet;
extern SDL_Texture* miniBossJumpSheet;
extern SDL_Texture* miniBossDamageSheet;
extern SDL_Texture* miniBossDeathSheet;
extern SDL_Texture* miniBossDiveSheet;
extern SDL_Texture* miniBossShootSheet;
extern SDL_Texture* miniBossArrowTexture;

Boss::Boss(int x, int y,
           SDL_Texture* run, int runCount, int runWidth, int runHeight,
           SDL_Texture* attack, int attackCount, int attackWidth, int attackHeight,
           SDL_Texture* jump, int jumpCount, int jumpWidth, int jumpHeight,
           SDL_Texture* damage, int damageCount, int damageWidth, int damageHeight,
           SDL_Texture* death, int deathCount, int deathWidth, int deathHeight,
           SDL_Texture* dive, int diveCount, int diveWidth, int diveHeight)
    : rect{x, y, 120, 120}, health(1000), maxHealth(1000), verticalVelocity(0), horizontalDiveVelocity(0),
      isJumping(false), isOnGround(false), facingRight(false),
      isAttacking(false), isDashing(false), isDiving(false), isTakingDamage(false), isDead(false),
      isIdle(false), isRetreating(false), hasDealtDamage(false), currentRunFrame(0), currentAttackFrame(0),
      currentJumpFrame(0), currentDamageFrame(0), currentDeathFrame(0), retreatStartX(0),
      lastFrameTime(0), dashStartTime(0), diveStartTime(0), lastAttackTime(0),
      runSheet(run), runFrameCount(runCount), runFrameWidth(runWidth), runFrameHeight(runHeight),
      attackSheet(attack), attackFrameCount(attackCount), attackFrameWidth(attackWidth), attackFrameHeight(attackHeight),
      jumpSheet(jump), jumpFrameCount(jumpCount), jumpFrameWidth(jumpWidth), jumpFrameHeight(jumpHeight),
      damageSheet(damage), damageFrameCount(damageCount), damageFrameWidth(damageWidth), damageFrameHeight(damageHeight),
      deathSheet(death), deathFrameCount(deathCount), deathFrameWidth(deathWidth), deathFrameHeight(deathHeight),
      diveSheet(dive), diveFrameCount(diveCount), diveFrameWidth(diveWidth), diveFrameHeight(diveHeight),
      hasSummonedMiniBoss(false) {}

Boss::~Boss() {
    for (Boss* miniBoss : miniBosses) {
        delete miniBoss;
    }
}

void Boss::RenderHealthBar(SDL_Renderer* renderer) {
    if (isDead) return;

    // Vẽ khung ngoài (màu xám)
    SDL_Rect outerRect = {rect.x + (rect.w - 100) / 2, rect.y - 20, 100, 10};
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderFillRect(renderer, &outerRect);

    // Vẽ thanh máu (màu đỏ, tỷ lệ với health/maxHealth)
    float healthRatio = static_cast<float>(health) / maxHealth;
    int healthWidth = static_cast<int>(100 * healthRatio);
    SDL_Rect healthRect = {rect.x + (rect.w - 100) / 2, rect.y - 20, healthWidth, 10};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthRect);
}

void Boss::Update(const SDL_Rect& playerRect, SDL_Rect* platforms, int platformCount, int currentLevel, Player& player) {
    if (isDead) return;

    int distance = std::abs(rect.x + rect.w / 2 - (playerRect.x + playerRect.w / 2));
    std::cout << "Distance to player: " << distance << "\n";

    // Kiểm tra va chạm với hitbox tấn công của nhân vật
    if (player.IsAttacking() && !isDead && !isTakingDamage) {
        SDL_Rect attackHitbox = player.GetAttackHitbox();
        if (SDL_HasIntersection(&rect, &attackHitbox)) {
            std::cout << "Boss hit by player attack!\n";
            ReduceHealth(10); // Giảm 10 máu mỗi lần bị tấn công
        }
    }

    if (currentLevel == 2 && health <= 0.4 * maxHealth && !hasSummonedMiniBoss) {
        MiniBoss* miniBoss = new MiniBoss(
            rect.x, rect.y,
            miniBossRunSheet, 8, 128, 128,
            miniBossAttackSheet, 6, 128, 128,
            miniBossJumpSheet, 9, 128, 128,
            miniBossDamageSheet, 3, 128, 128,
            miniBossDeathSheet, 5, 128, 128,
            miniBossDiveSheet, 5, 128, 128,
            miniBossShootSheet, 4, 128, 128,
            miniBossArrowTexture, 64, 64
        );
        miniBosses.push_back(miniBoss);
        hasSummonedMiniBoss = true;
        std::cout << "Boss 2 summons MiniBoss at x=" << rect.x + 100 << "\n";
    }

    for (Boss* miniBoss : miniBosses) {
        miniBoss->Update(playerRect, platforms, platformCount, currentLevel, player);
    }

    if (currentLevel == 1 || currentLevel == 2) {
        bool canAttack = (SDL_GetTicks() - lastAttackTime >= ATTACK_COOLDOWN);
        if (isIdle && canAttack) {
            isIdle = false;
            std::cout << "Boss " << currentLevel << " exits idle state after cooldown\n";
        }
        std::cout << "Boss " << currentLevel << " isIdle: " << isIdle << ", isRetreating: " << isRetreating << ", canAttack: " << canAttack << "\n";

        if (isRetreating && !isIdle) {
            bool moved = false;
            if (playerRect.x < rect.x && rect.x < SCREEN_WIDTH - rect.w) {
                rect.x += BOSS_SPEED;
                facingRight = true;
                moved = true;
            } else if (playerRect.x > rect.x && rect.x > 0) {
                rect.x -= BOSS_SPEED;
                facingRight = false;
                moved = true;
            }
            int movedDistance = std::abs(rect.x - retreatStartX);
            std::cout << "Boss " << currentLevel << " retreating: x=" << rect.x << ", moved=" << movedDistance << "\n";

            if (rect.x <= 0 || rect.x + rect.w >= SCREEN_WIDTH) {
                isRetreating = false;
                isIdle = true;
                std::cout << "Boss " << currentLevel << " stops retreating at screen edge (x=" << rect.x << ") and enters idle state\n";
            }
            else if (movedDistance >= RETREAT_DISTANCE) {
                isRetreating = false;
                isIdle = true;
                std::cout << "Boss " << currentLevel << " stops retreating after moving " << movedDistance << " pixels and enters idle state\n";
            }
        } else if (!isIdle && !isDashing && !isAttacking && !isJumping && !isDiving && !isRetreating) {
            std::cout << "Boss " << currentLevel << " stays still while player approaches\n";
        }
    }

    if (currentLevel == 2 && !isAttacking && !isJumping && !isDashing && !isRetreating && !isIdle) {
        if (playerRect.x < rect.x && rect.x > 0) {
            rect.x -= BOSS_SPEED;
            facingRight = false;
        } else if (playerRect.x > rect.x && rect.x + rect.w < SCREEN_WIDTH) {
            rect.x += BOSS_SPEED;
            facingRight = true;
        }
        std::cout << "Boss " << currentLevel << " chases player\n";
    }

    bool canAttack = (SDL_GetTicks() - lastAttackTime >= ATTACK_COOLDOWN);
    if (!canAttack) {
        std::cout << "Boss " << currentLevel << " in attack cooldown: " << (ATTACK_COOLDOWN - (SDL_GetTicks() - lastAttackTime)) << "ms remaining\n";
    }

    if (!isIdle && !isRetreating && !isAttacking && !isTakingDamage && !isJumping && !isDashing && !isDiving && canAttack) {
        int action = rand() % 100;
        if ((currentLevel == 1 || currentLevel == 2) && distance >= MIN_DISTANCE && distance <= MAX_DISTANCE) {
            if (action < 30 && isOnGround) {
                isDashing = true;
                dashStartTime = SDL_GetTicks();
                currentAttackFrame = 0;
                lastFrameTime = SDL_GetTicks();
                hasDealtDamage = false;
                std::cout << "Boss " << currentLevel << " starts dashing\n";
            } else if (action < 60 && isOnGround) {
                isJumping = true;
                verticalVelocity = JUMP_STRENGTH;
                isOnGround = false;
                currentJumpFrame = 0;
                lastFrameTime = SDL_GetTicks();
                std::cout << "Boss " << currentLevel << " starts jump-dive attack\n";
            }
        } else if (action < 40 && isOnGround) {
            if (currentLevel == 1 || currentLevel == 2) {
                isDashing = true;
                dashStartTime = SDL_GetTicks();
                currentAttackFrame = 0;
                lastFrameTime = SDL_GetTicks();
                hasDealtDamage = false;
                std::cout << "Boss " << currentLevel << " starts dashing (outside ideal range)\n";
            }
        } else if (action < 70 && isOnGround) {
            isJumping = true;
            verticalVelocity = JUMP_STRENGTH;
            isOnGround = false;
            currentJumpFrame = 0;
            lastFrameTime = SDL_GetTicks();
            std::cout << "Boss " << currentLevel << " starts jumping\n";
        }
    }

    if ((currentLevel == 1 || currentLevel == 2) && isJumping && !isDiving && verticalVelocity >= 0) {
        isJumping = false;
        isDiving = true;
        diveStartTime = SDL_GetTicks();
        currentAttackFrame = 0;
        lastFrameTime = SDL_GetTicks();
        std::cout << "Boss " << currentLevel << " starts diving\n";
    }

    if (isDiving && (currentLevel == 1 || currentLevel == 2)) {
        if (playerRect.x < rect.x) {
            horizontalDiveVelocity = -DIVE_SPEED_HORIZONTAL;
            facingRight = false;
        } else {
            horizontalDiveVelocity = DIVE_SPEED_HORIZONTAL;
            facingRight = true;
        }

        rect.x += horizontalDiveVelocity;
        verticalVelocity = DIVE_SPEED_VERTICAL;
        std::cout << "Boss " << currentLevel << " diving: horizontal=" << horizontalDiveVelocity << ", vertical=" << verticalVelocity << "\n";

        if (rect.x < 0) rect.x = 0;
        if (rect.x + rect.w > SCREEN_WIDTH) rect.x = SCREEN_WIDTH - rect.w;

        SDL_Rect attackHitbox = {
            rect.x + (facingRight ? 30 : -30),
            rect.y + 20,
            60,
            60
        };
        std::cout << "Dive hitbox: x=" << attackHitbox.x << ", y=" << attackHitbox.y << ", w=" << attackHitbox.w << ", h=" << attackHitbox.h << "\n";

        if (SDL_HasIntersection(&attackHitbox, &playerRect)) {
            std::cout << "Boss " << currentLevel << " dive collides with player\n";
            if (!player.IsDead() && !player.IsInvulnerable() && !hasDealtDamage) {
                std::cout << "Boss " << currentLevel << " dives and hits player!\n";
                player.TakeDamage(1);
                hasDealtDamage = true;
            }
            isDiving = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "Boss " << currentLevel << " dive ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        } else if (SDL_GetTicks() - diveStartTime >= DIVE_DURATION) {
            std::cout << "Boss " << currentLevel << " dive timeout\n";
            isDiving = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "Boss " << currentLevel << " dive ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        }
    }

    if (isDashing && (currentLevel == 1 || currentLevel == 2)) {
        if (playerRect.x < rect.x && rect.x > 0) {
            rect.x -= DASH_SPEED;
            facingRight = false;
        } else if (playerRect.x > rect.x && rect.x + rect.w < SCREEN_WIDTH) {
            rect.x += DASH_SPEED;
            facingRight = true;
        }

        SDL_Rect attackHitbox = {
            rect.x + (facingRight ? 20 : -20),
            rect.y,
            80,
            100
        };
        std::cout << "Dash hitbox: x=" << attackHitbox.x << ", y=" << attackHitbox.y << ", w=" << attackHitbox.w << ", h=" << attackHitbox.h << "\n";

        if (SDL_HasIntersection(&attackHitbox, &playerRect)) {
            std::cout << "Boss " << currentLevel << " dash collides with player\n";
            if (!player.IsDead() && !player.IsInvulnerable() && !hasDealtDamage) {
                std::cout << "Boss " << currentLevel << " dashes and hits player!\n";
                player.TakeDamage(1);
                hasDealtDamage = true;
            }
            isDashing = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "Boss " << currentLevel << " dash ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        } else if (SDL_GetTicks() - dashStartTime >= DASH_DURATION) {
            std::cout << "Boss " << currentLevel << " dash timeout\n";
            isDashing = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "Boss " << currentLevel << " dash ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        }
    }

    verticalVelocity += GRAVITY;
    rect.y += verticalVelocity;

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
            if (isDiving && (currentLevel == 1 || currentLevel == 2)) {
                isDiving = false;
                currentAttackFrame = 0;
                lastAttackTime = SDL_GetTicks();
                isRetreating = true;
                retreatStartX = rect.x;
                std::cout << "Boss " << currentLevel << " dive ends on platform, starting cooldown and retreating from x=" << retreatStartX << "\n";
            }
            onPlatform = true;
            break;
        }
    }

    if (rect.y + rect.h >= 500 && !onPlatform) {
        rect.y = 500 - rect.h;
        verticalVelocity = 0;
        isJumping = false;
        if (isDiving && (currentLevel == 1 || currentLevel == 2)) {
            isDiving = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "Boss " << currentLevel << " dive ends on ground, starting cooldown and retreating from x=" << retreatStartX << "\n";
        }
        isOnGround = true;
    }

    if (isAttacking && currentAttackFrame >= attackFrameCount - 1) {
        isAttacking = false;
        currentAttackFrame = 0;
        lastAttackTime = SDL_GetTicks();
        std::cout << "Boss " << currentLevel << " stops attacking, starting cooldown\n";
        if ((currentLevel == 1 || currentLevel == 2) && distance > IDLE_DISTANCE) {
            isIdle = true;
            std::cout << "Boss " << currentLevel << " enters idle state after attack\n";
        }
    }
}

void Boss::Render(SDL_Renderer* renderer, SDL_Texture* idleTexture, Uint32 currentTime) {
    // Vẽ thanh máu
    RenderHealthBar(renderer);

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
        srcRect = {currentDamageFrame * damageFrameWidth, 0, damageFrameWidth, deathFrameHeight};
        destRect = {rect.x + (rect.w - damageFrameWidth) / 2, rect.y + (rect.h - deathFrameHeight), damageFrameWidth, deathFrameHeight};
        SDL_RenderCopyEx(renderer, damageSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isDiving) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentAttackFrame++;
        }
        srcRect = {currentAttackFrame * diveFrameWidth, 0, diveFrameWidth, diveFrameHeight};
        destRect = {rect.x + (rect.w - diveFrameWidth) / 2, rect.y + (rect.h - diveFrameHeight), diveFrameWidth, diveFrameHeight};
        SDL_RenderCopyEx(renderer, diveSheet, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "Rendering dive animation: frame " << currentAttackFrame << "\n";
    } else if (isAttacking || isDashing) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentAttackFrame++;
        }
        srcRect = {currentAttackFrame * attackFrameWidth, 0, attackFrameWidth, attackFrameHeight};
        destRect = {rect.x + (rect.w - attackFrameWidth) / 2, rect.y + (rect.h - attackFrameHeight), attackFrameWidth, attackFrameHeight};
        SDL_RenderCopyEx(renderer, attackSheet, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "Rendering attack/dash animation: frame " << currentAttackFrame << "\n";
    } else if (isJumping) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentJumpFrame = (currentJumpFrame + 1) % jumpFrameCount;
        }
        srcRect = {currentJumpFrame * jumpFrameWidth, 0, jumpFrameWidth, jumpFrameHeight};
        destRect = {rect.x + (rect.w - jumpFrameWidth) / 2, rect.y + (rect.h - jumpFrameHeight), jumpFrameWidth, jumpFrameHeight};
        SDL_RenderCopyEx(renderer, jumpSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isIdle) {
        srcRect = {0, 0, runFrameWidth, runFrameHeight};
        destRect = {rect.x + (rect.w - runFrameWidth) / 2, rect.y + (rect.h - runFrameHeight), runFrameWidth, runFrameHeight};
        SDL_RenderCopyEx(renderer, idleTexture, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "Rendering idle animation\n";
    } else if (isRetreating) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentRunFrame = (currentRunFrame + 1) % runFrameCount;
        }
        srcRect = {currentRunFrame * runFrameWidth, 0, runFrameWidth, runFrameHeight};
        destRect = {rect.x + (rect.w - runFrameWidth) / 2, rect.y + (rect.h - runFrameHeight), runFrameWidth, runFrameHeight};
        SDL_RenderCopyEx(renderer, runSheet, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "Rendering retreat animation (using run): frame " << currentRunFrame << "\n";
    } else {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentRunFrame = (currentRunFrame + 1) % runFrameCount;
        }
        srcRect = {currentRunFrame * runFrameWidth, 0, runFrameWidth, runFrameHeight};
        destRect = {rect.x + (rect.w - runFrameWidth) / 2, rect.y + (rect.h - runFrameHeight), runFrameWidth, runFrameHeight};
        SDL_RenderCopyEx(renderer, runSheet, &srcRect, &destRect, 0, nullptr, flip);
    }

    // Vẽ MiniBoss và thanh máu của chúng
    for (Boss* miniBoss : miniBosses) {
        miniBoss->Render(renderer, idleTexture, currentTime);
    }
}

void Boss::ReduceHealth(int amount) {
    if (isDead) return;
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

MiniBoss::MiniBoss(int x, int y,
                   SDL_Texture* run, int runCount, int runWidth, int runHeight,
                   SDL_Texture* attack, int attackCount, int attackWidth, int attackHeight,
                   SDL_Texture* jump, int jumpCount, int jumpWidth, int jumpHeight,
                   SDL_Texture* damage, int damageCount, int damageWidth, int damageHeight,
                   SDL_Texture* death, int deathCount, int deathWidth, int deathHeight,
                   SDL_Texture* dive, int diveCount, int diveWidth, int diveHeight,
                   SDL_Texture* shoot, int shootCount, int shootWidth, int shootHeight,
                   SDL_Texture* arrow, int arrowWidth, int arrowHeight)
    : Boss(x, y, run, runCount, runWidth, runHeight, attack, attackCount, attackWidth, attackHeight,
           jump, jumpCount, jumpWidth, jumpHeight, damage, damageCount, damageWidth, damageHeight,
           death, deathCount, deathWidth, deathHeight, dive, diveCount, diveWidth, diveHeight),
      isShooting(false), shootStartTime(0), arrowTexture(arrow),
      shootSheet(shoot), shootFrameCount(shootCount), shootFrameWidth(shootWidth), shootFrameHeight(shootHeight) {
    health = 1000;
    maxHealth = 1000;
}

void MiniBoss::Update(const SDL_Rect& playerRect, SDL_Rect* platforms, int platformCount, int currentLevel, Player& player) {
    if (isDead) return;

    int distance = std::abs(rect.x + rect.w / 2 - (playerRect.x + playerRect.w / 2));
    std::cout << "MiniBoss distance to player: " << distance << "\n";

    // Kiểm tra va chạm với hitbox tấn công của nhân vật
    if (player.IsAttacking() && !isDead && !isTakingDamage) {
        SDL_Rect attackHitbox = player.GetAttackHitbox();
        if (SDL_HasIntersection(&rect, &attackHitbox)) {
            std::cout << "MiniBoss hit by player attack!\n";
            ReduceHealth(10); // Giảm 10 máu mỗi lần bị tấn công
        }
    }

    if (currentLevel == 2) {
        bool canAttack = (SDL_GetTicks() - lastAttackTime >= ATTACK_COOLDOWN);
        if (isIdle && canAttack) {
            isIdle = false;
            std::cout << "MiniBoss exits idle state after cooldown\n";
        }

        if (isRetreating && !isIdle) {
            bool moved = false;
            if (playerRect.x < rect.x && rect.x < SCREEN_WIDTH - rect.w) {
                rect.x += BOSS_SPEED;
                facingRight = true;
                moved = true;
            } else if (playerRect.x > rect.x && rect.x > 0) {
                rect.x -= BOSS_SPEED;
                facingRight = false;
                moved = true;
            }
            int movedDistance = std::abs(rect.x - retreatStartX);
            std::cout << "MiniBoss retreating: x=" << rect.x << ", moved=" << movedDistance << "\n";

            if (rect.x <= 0 || rect.x + rect.w >= SCREEN_WIDTH) {
                isRetreating = false;
                isIdle = true;
                std::cout << "MiniBoss stops retreating at screen edge (x=" << rect.x << ") and enters idle state\n";
            }
            else if (movedDistance >= RETREAT_DISTANCE) {
                isRetreating = false;
                isIdle = true;
                std::cout << "MiniBoss stops retreating after moving " << movedDistance << " pixels and enters idle state\n";
            }
        } else if (!isIdle && !isDashing && !isAttacking && !isJumping && !isDiving && !isRetreating && !isShooting) {
            if (playerRect.x < rect.x && rect.x > 0) {
                rect.x -= BOSS_SPEED;
                facingRight = false;
            } else if (playerRect.x > rect.x && rect.x + rect.w < SCREEN_WIDTH) {
                rect.x += BOSS_SPEED;
                facingRight = true;
            }
            // Đặt MiniBoss ngang hàng với nhân vật
            rect.y = playerRect.y;
            // Đảm bảo MiniBoss không chìm dưới sàn
            if (rect.y + rect.h > 500) {
                rect.y = 500 - rect.h;
            }
            std::cout << "MiniBoss chases player on same y-line: y=" << rect.y << "\n";
        }
    }

    bool canAttack = (SDL_GetTicks() - lastAttackTime >= ATTACK_COOLDOWN);
    bool canShoot = (SDL_GetTicks() - shootStartTime >= SHOOT_COOLDOWN);
    if (!canAttack) {
        std::cout << "MiniBoss in attack cooldown: " << (ATTACK_COOLDOWN - (SDL_GetTicks() - lastAttackTime)) << "ms remaining\n";
    }
    if (!canShoot) {
        std::cout << "MiniBoss in shoot cooldown: " << (SHOOT_COOLDOWN - (SDL_GetTicks() - shootStartTime)) << "ms remaining\n";
    }

    if (!isIdle && !isRetreating && !isAttacking && !isTakingDamage && !isJumping && !isDashing && !isDiving && !isShooting && canAttack) {
        int action = rand() % 100;
        if (currentLevel == 2 && distance >= MIN_DISTANCE && distance <= MAX_DISTANCE) {
            if (action < 70 && isOnGround && canShoot) {
                isShooting = true;
                shootStartTime = SDL_GetTicks();
                currentAttackFrame = 0;
                lastFrameTime = SDL_GetTicks();
                hasDealtDamage = false;
                // Điều chỉnh vị trí khởi tạo mũi tên (64x64)
                int arrowX = rect.x + (facingRight ? rect.w : -64);
                int arrowY = rect.y + (rect.h - 64) / 2; // Căn giữa theo chiều cao
                arrows.emplace_back(arrowX, arrowY, facingRight, 64, 64);
                std::cout << "MiniBoss shoots arrow at x=" << arrowX << ", y=" << arrowY << ", facingRight=" << facingRight << "\n";
            } else if (action < 80 && isOnGround) {
                isDashing = true;
                dashStartTime = SDL_GetTicks();
                currentAttackFrame = 0;
                lastFrameTime = SDL_GetTicks();
                hasDealtDamage = false;
                std::cout << "MiniBoss starts dashing\n";
            } else if (action < 90 && isOnGround) {
                isJumping = true;
                verticalVelocity = JUMP_STRENGTH;
                isOnGround = false;
                currentJumpFrame = 0;
                lastFrameTime = SDL_GetTicks();
                std::cout << "MiniBoss starts jump-dive attack\n";
            }
        } else if (action < 40 && isOnGround) {
            isDashing = true;
            dashStartTime = SDL_GetTicks();
            currentAttackFrame = 0;
            lastFrameTime = SDL_GetTicks();
            hasDealtDamage = false;
            std::cout << "MiniBoss starts dashing (outside ideal range)\n";
        } else if (action < 70 && isOnGround) {
            isJumping = true;
            verticalVelocity = JUMP_STRENGTH;
            isOnGround = false;
            currentJumpFrame = 0;
            lastFrameTime = SDL_GetTicks();
            std::cout << "MiniBoss starts jumping\n";
        }
    }

    if (currentLevel == 2 && isJumping && !isDiving && verticalVelocity >= 0) {
        isJumping = false;
        isDiving = true;
        diveStartTime = SDL_GetTicks();
        currentAttackFrame = 0;
        lastFrameTime = SDL_GetTicks();
        std::cout << "MiniBoss starts diving\n";
    }

    if (isDiving && currentLevel == 2) {
        if (playerRect.x < rect.x) {
            horizontalDiveVelocity = -DIVE_SPEED_HORIZONTAL;
            facingRight = false;
        } else {
            horizontalDiveVelocity = DIVE_SPEED_HORIZONTAL;
            facingRight = true;
        }

        rect.x += horizontalDiveVelocity;
        verticalVelocity = DIVE_SPEED_VERTICAL;
        std::cout << "MiniBoss diving: horizontal=" << horizontalDiveVelocity << ", vertical=" << verticalVelocity << "\n";

        if (rect.x < 0) rect.x = 0;
        if (rect.x + rect.w > SCREEN_WIDTH) rect.x = SCREEN_WIDTH - rect.w;

        SDL_Rect attackHitbox = {
            rect.x + (facingRight ? 30 : -30),
            rect.y + 20,
            60,
            60
        };
        std::cout << "MiniBoss dive hitbox: x=" << attackHitbox.x << ", y=" << attackHitbox.y << ", w=" << attackHitbox.w << ", h=" << attackHitbox.h << "\n";

        if (SDL_HasIntersection(&attackHitbox, &playerRect)) {
            std::cout << "MiniBoss dive collides with player\n";
            if (!player.IsDead() && !player.IsInvulnerable() && !hasDealtDamage) {
                std::cout << "MiniBoss dives and hits player!\n";
                player.TakeDamage(1);
                hasDealtDamage = true;
            }
            isDiving = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "MiniBoss dive ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        } else if (SDL_GetTicks() - diveStartTime >= DIVE_DURATION) {
            std::cout << "MiniBoss dive timeout\n";
            isDiving = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "MiniBoss dive ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        }
    }

    if (isDashing && currentLevel == 2) {
        if (playerRect.x < rect.x && rect.x > 0) {
            rect.x -= DASH_SPEED;
            facingRight = false;
        } else if (playerRect.x > rect.x && rect.x + rect.w < SCREEN_WIDTH) {
            rect.x += DASH_SPEED;
            facingRight = true;
        }

        SDL_Rect attackHitbox = {
            rect.x + (facingRight ? 20 : -20),
            rect.y,
            80,
            100
        };
        std::cout << "MiniBoss dash hitbox: x=" << attackHitbox.x << ", y=" << attackHitbox.y << ", w=" << attackHitbox.w << ", h=" << attackHitbox.h << "\n";

        if (SDL_HasIntersection(&attackHitbox, &playerRect)) {
            std::cout << "MiniBoss dash collides with player\n";
            if (!player.IsDead() && !player.IsInvulnerable() && !hasDealtDamage) {
                std::cout << "MiniBoss dashes and hits player!\n";
                player.TakeDamage(1);
                hasDealtDamage = true;
            }
            isDashing = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "MiniBoss dash ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        } else if (SDL_GetTicks() - dashStartTime >= DASH_DURATION) {
            std::cout << "MiniBoss dash timeout\n";
            isDashing = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "MiniBoss dash ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        }
    }

    if (isShooting && currentLevel == 2) {
        if (SDL_GetTicks() - shootStartTime >= 1000) { // Thời gian để quan sát mũi tên
            isShooting = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "MiniBoss stops shooting, starting cooldown and retreating from x=" << retreatStartX << "\n";
        }
    }

    // Cập nhật vị trí mũi tên
    for (auto it = arrows.begin(); it != arrows.end();) {
        it->rect.x += it->velocity;
        std::cout << "Arrow position: x=" << it->rect.x << ", y=" << it->rect.y << "\n";
        if (it->rect.x < 0 || it->rect.x > SCREEN_WIDTH) {
            it = arrows.erase(it);
            std::cout << "Arrow removed (out of bounds)\n";
        } else {
            if (SDL_HasIntersection(&it->rect, &playerRect)) {
                if (!player.IsDead() && !player.IsInvulnerable()) {
                    std::cout << "Arrow hits player!\n";
                    player.TakeDamage(1);
                }
                it = arrows.erase(it);
                std::cout << "Arrow removed (hit player)\n";
            } else {
                ++it;
            }
        }
    }

    verticalVelocity += GRAVITY;
    rect.y += verticalVelocity;

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
            if (isDiving && currentLevel == 2) {
                isDiving = false;
                currentAttackFrame = 0;
                lastAttackTime = SDL_GetTicks();
                isRetreating = true;
                retreatStartX = rect.x;
                std::cout << "MiniBoss dive ends on platform, starting cooldown and retreating from x=" << retreatStartX << "\n";
            }
            onPlatform = true;
            break;
        }
    }

    if (rect.y + rect.h >= 500 && !onPlatform) {
        rect.y = 500 - rect.h;
        verticalVelocity = 0;
        isJumping = false;
        if (isDiving && currentLevel == 2) {
            isDiving = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks();
            isRetreating = true;
            retreatStartX = rect.x;
            std::cout << "MiniBoss dive ends on ground, starting cooldown and retreating from x=" << retreatStartX << "\n";
        }
        isOnGround = true;
    }

    if (isAttacking && currentAttackFrame >= attackFrameCount - 1) {
        isAttacking = false;
        currentAttackFrame = 0;
        lastAttackTime = SDL_GetTicks();
        isRetreating = true;
        retreatStartX = rect.x;
        std::cout << "MiniBoss stops attacking, starting cooldown and retreating from x=" << retreatStartX << "\n";
        if (currentLevel == 2 && distance > IDLE_DISTANCE) {
            isIdle = true;
            std::cout << "MiniBoss enters idle state after attack\n";
        }
    }
}

void MiniBoss::Render(SDL_Renderer* renderer, SDL_Texture* idleTexture, Uint32 currentTime) {
    // Vẽ thanh máu cho MiniBoss
    RenderHealthBar(renderer);

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
        srcRect = {currentDamageFrame * damageFrameWidth, 0, damageFrameWidth, deathFrameHeight};
        destRect = {rect.x + (rect.w - damageFrameWidth) / 2, rect.y + (rect.h - deathFrameHeight), damageFrameWidth, deathFrameHeight};
        SDL_RenderCopyEx(renderer, damageSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isDiving) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentAttackFrame++;
        }
        srcRect = {currentAttackFrame * diveFrameWidth, 0, diveFrameWidth, diveFrameHeight};
        destRect = {rect.x + (rect.w - diveFrameWidth) / 2, rect.y + (rect.h - diveFrameHeight), diveFrameWidth, diveFrameHeight};
        SDL_RenderCopyEx(renderer, diveSheet, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "MiniBoss rendering dive animation: frame " << currentAttackFrame << "\n";
    } else if (isShooting) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentAttackFrame = (currentAttackFrame + 1) % shootFrameCount;
        }
        srcRect = {currentAttackFrame * shootFrameWidth, 0, shootFrameWidth, shootFrameHeight};
        destRect = {rect.x + (rect.w - shootFrameWidth) / 2, rect.y + (rect.h - shootFrameHeight), shootFrameWidth, shootFrameHeight};
        SDL_RenderCopyEx(renderer, shootSheet, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "MiniBoss rendering shoot animation: frame " << currentAttackFrame << "\n";
    } else if (isAttacking || isDashing) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentAttackFrame++;
        }
        srcRect = {currentAttackFrame * attackFrameWidth, 0, attackFrameWidth, attackFrameHeight};
        destRect = {rect.x + (rect.w - attackFrameWidth) / 2, rect.y + (rect.h - attackFrameHeight), attackFrameWidth, attackFrameHeight};
        SDL_RenderCopyEx(renderer, attackSheet, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "MiniBoss rendering attack/dash animation: frame " << currentAttackFrame << "\n";
    } else if (isJumping) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentJumpFrame = (currentJumpFrame + 1) % jumpFrameCount;
        }
        srcRect = {currentJumpFrame * jumpFrameWidth, 0, jumpFrameWidth, jumpFrameHeight};
        destRect = {rect.x + (rect.w - jumpFrameWidth) / 2, rect.y + (rect.h - jumpFrameHeight), jumpFrameWidth, jumpFrameHeight};
        SDL_RenderCopyEx(renderer, jumpSheet, &srcRect, &destRect, 0, nullptr, flip);
    } else if (isIdle) {
        srcRect = {0, 0, runFrameWidth, runFrameHeight};
        destRect = {rect.x + (rect.w - runFrameWidth) / 2, rect.y + (rect.h - runFrameHeight), runFrameWidth, runFrameHeight};
        SDL_RenderCopyEx(renderer, miniBossIdle, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "MiniBoss rendering idle animation\n";
    } else if (isRetreating) {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentRunFrame = (currentRunFrame + 1) % runFrameCount;
        }
        srcRect = {currentRunFrame * runFrameWidth, 0, runFrameWidth, runFrameHeight};
        destRect = {rect.x + (rect.w - runFrameWidth) / 2, rect.y + (rect.h - runFrameHeight), runFrameWidth, runFrameHeight};
        SDL_RenderCopyEx(renderer, runSheet, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "MiniBoss rendering retreat animation (using run): frame " << currentRunFrame << "\n";
    } else {
        if (currentTime > lastFrameTime + FRAME_DELAY) {
            lastFrameTime = currentTime;
            currentRunFrame = (currentRunFrame + 1) % runFrameCount;
        }
        srcRect = {currentRunFrame * runFrameWidth, 0, runFrameWidth, runFrameHeight};
        destRect = {rect.x + (rect.w - runFrameWidth) / 2, rect.y + (rect.h - runFrameHeight), runFrameWidth, runFrameHeight};
        SDL_RenderCopyEx(renderer, runSheet, &srcRect, &destRect, 0, nullptr, flip);
    }

    // Vẽ các mũi tên bay thẳng
    for (const auto& arrow : arrows) {
        SDL_Rect arrowSrcRect = {0, 0, arrow.frameWidth, arrow.frameHeight}; // Lấy toàn bộ texture 64x64
        SDL_Rect arrowDestRect = {arrow.rect.x, arrow.rect.y, arrow.frameWidth, arrow.frameHeight};
        SDL_RendererFlip arrowFlip = arrow.facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderCopyEx(renderer, arrowTexture, &arrowSrcRect, &arrowDestRect, 0.0, nullptr, arrowFlip);
        std::cout << "Rendering flying arrow at x=" << arrowDestRect.x << ", y=" << arrowDestRect.y << ", facingRight=" << arrow.facingRight << "\n";
    }
}
