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
const Uint32 FRAME_DELAY = 70;
const Uint32 DASH_DURATION = 1000;
const Uint32 DIVE_DURATION = 2500;
const int MIN_DISTANCE = 400; // Khoảng cách tối thiểu
const int MAX_DISTANCE = 1000; // Khoảng cách tối đa

Boss::Boss(int x, int y,
           SDL_Texture* run, int runCount, int runWidth, int runHeight,
           SDL_Texture* attack, int attackCount, int attackWidth, int attackHeight,
           SDL_Texture* jump, int jumpCount, int jumpWidth, int jumpHeight,
           SDL_Texture* damage, int damageCount, int damageWidth, int damageHeight,
           SDL_Texture* death, int deathCount, int deathWidth, int deathHeight,
           SDL_Texture* dive, int diveCount, int diveWidth, int diveHeight)
    : rect{x, y, 120, 120}, health(100), maxHealth(100), verticalVelocity(0), horizontalDiveVelocity(0),
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
      diveSheet(dive), diveFrameCount(diveCount), diveFrameWidth(diveWidth), diveFrameHeight(diveHeight) {}

void Boss::Update(const SDL_Rect& playerRect, SDL_Rect* platforms, int platformCount, int currentLevel, Player& player) {
    if (isDead) return;

    // Tính khoảng cách tới player
    int distance = std::abs(rect.x + rect.w / 2 - (playerRect.x + playerRect.w / 2));
    std::cout << "Distance to player: " << distance << "\n";

    // Kiểm tra trạng thái idle và retreating cho boss 1
    if (currentLevel == 1) {
        bool canAttack = (SDL_GetTicks() - lastAttackTime >= ATTACK_COOLDOWN);
        if (isIdle && canAttack) {
            isIdle = false;
            std::cout << "Boss 1 exits idle state after cooldown\n";
        }
        std::cout << "Boss 1 isIdle: " << isIdle << ", isRetreating: " << isRetreating << ", canAttack: " << canAttack << "\n";

        // Xử lý lùi lại sau chiêu
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
            std::cout << "Boss 1 retreating: x=" << rect.x << ", moved=" << movedDistance << "\n";

            // Kiểm tra chạm giới hạn màn hình
            if (rect.x <= 0 || rect.x + rect.w >= SCREEN_WIDTH) {
                isRetreating = false;
                isIdle = true;
                std::cout << "Boss 1 stops retreating at screen edge (x=" << rect.x << ") and enters idle state\n";
            }
            // Kiểm tra nếu di chuyển đủ RETREAT_DISTANCE
            else if (movedDistance >= RETREAT_DISTANCE) {
                isRetreating = false;
                isIdle = true;
                std::cout << "Boss 1 stops retreating after moving " << movedDistance << " pixels and enters idle state\n";
            }
        } else if (!isIdle && !isDashing && !isAttacking && !isJumping && !isDiving && !isRetreating) {
            std::cout << "Boss 1 stays still while player approaches\n";
        }
    }

    // Di chuyển cho boss 2, 3 (đuổi theo player)
    if (currentLevel != 1 && !isAttacking && !isJumping) {
        if (playerRect.x < rect.x && rect.x > 0) {
            rect.x -= BOSS_SPEED;
            facingRight = false;
        } else if (playerRect.x > rect.x && rect.x + rect.w < SCREEN_WIDTH) {
            rect.x += BOSS_SPEED;
            facingRight = true;
        }
        std::cout << "Boss " << currentLevel << " chases player\n";
    }

    // Kiểm tra cooldown trước khi tấn công
    bool canAttack = (SDL_GetTicks() - lastAttackTime >= ATTACK_COOLDOWN);
    if (!canAttack) {
        std::cout << "Boss 1 in attack cooldown: " << (ATTACK_COOLDOWN - (SDL_GetTicks() - lastAttackTime)) << "ms remaining\n";
    }

    // Quyết định hành động ngẫu nhiên (chỉ khi không idle và không retreating)
    if (!isIdle && !isRetreating && !isAttacking && !isTakingDamage && !isJumping && !isDashing && !isDiving && canAttack) {
        int action = rand() % 100;
        if (currentLevel == 1 && distance >= MIN_DISTANCE && distance <= MAX_DISTANCE) {
            // Xác suất trong khoảng cách lý tưởng
            if (action < 30 && isOnGround) {
                // Chiêu lướt
                isDashing = true;
                dashStartTime = SDL_GetTicks();
                currentAttackFrame = 0;
                lastFrameTime = SDL_GetTicks();
                hasDealtDamage = false;
                std::cout << "Boss 1 starts dashing\n";
            } else if (action < 60 && isOnGround) {
                // Chiêu nhảy lao chém
                isJumping = true;
                verticalVelocity = JUMP_STRENGTH;
                isOnGround = false;
                currentJumpFrame = 0;
                lastFrameTime = SDL_GetTicks();
                std::cout << "Boss 1 starts jump-dive attack\n";
            }
        } else if (action < 40 && isOnGround) {
            // Hành động ngoài khoảng cách lý tưởng hoặc boss 2, 3
            if (currentLevel == 1) {
                isDashing = true;
                dashStartTime = SDL_GetTicks();
                currentAttackFrame = 0;
                lastFrameTime = SDL_GetTicks();
                hasDealtDamage = false;
                std::cout << "Boss 1 starts dashing (outside ideal range)\n";
            } else {
                isAttacking = true;
                currentAttackFrame = 0;
                lastFrameTime = SDL_GetTicks();
                hasDealtDamage = false;
                std::cout << "Boss starts attacking\n";
            }
        } else if (action < 70 && isOnGround) {
            // Nhảy bình thường
            isJumping = true;
            verticalVelocity = JUMP_STRENGTH;
            isOnGround = false;
            currentJumpFrame = 0;
            lastFrameTime = SDL_GetTicks();
            std::cout << "Boss starts jumping\n";
        }
    }

    // Xử lý nhảy lao chém cho boss 1
    if (currentLevel == 1 && isJumping && !isDiving && verticalVelocity >= 0) {
        isJumping = false;
        isDiving = true;
        diveStartTime = SDL_GetTicks();
        currentAttackFrame = 0;
        lastFrameTime = SDL_GetTicks();
        std::cout << "Boss 1 starts diving\n";
    }

    // Xử lý lao chém chéo cho boss 1
    if (isDiving && currentLevel == 1) {
        // Cập nhật hướng dive mỗi frame dựa trên vị trí player
        if (playerRect.x < rect.x) {
            horizontalDiveVelocity = -DIVE_SPEED_HORIZONTAL;
            facingRight = false;
        } else {
            horizontalDiveVelocity = DIVE_SPEED_HORIZONTAL;
            facingRight = true;
        }

        // Di chuyển chéo
        rect.x += horizontalDiveVelocity;
        verticalVelocity = DIVE_SPEED_VERTICAL;
        std::cout << "Boss 1 diving: horizontal=" << horizontalDiveVelocity << ", vertical=" << verticalVelocity << "\n";

        // Giới hạn vị trí ngang
        if (rect.x < 0) rect.x = 0;
        if (rect.x + rect.w > SCREEN_WIDTH) rect.x = SCREEN_WIDTH - rect.w;

        // Tạo hitbox nhỏ hơn cho dive
        SDL_Rect attackHitbox = {
            rect.x + (facingRight ? 30 : -30), // Dịch 30 pixel phía trước
            rect.y + 20,                      // Dịch 20 pixel xuống dưới
            60,                               // Rộng 60
            60                                // Cao 60
        };
        std::cout << "Dive hitbox: x=" << attackHitbox.x << ", y=" << attackHitbox.y << ", w=" << attackHitbox.w << ", h=" << attackHitbox.h << "\n";

        // Kiểm tra va chạm với player
        if (SDL_HasIntersection(&attackHitbox, &playerRect)) {
            std::cout << "Boss 1 dive collides with player\n";
            if (!player.IsDead() && !player.IsInvulnerable() && !hasDealtDamage) {
                std::cout << "Boss 1 dives and hits player!\n";
                player.TakeDamage(1);
                hasDealtDamage = true;
            }
            isDiving = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks(); // Cập nhật thời điểm tấn công
            isRetreating = true; // Bắt đầu lùi lại
            retreatStartX = rect.x; // Lưu vị trí bắt đầu lùi
            std::cout << "Boss 1 dive ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        } else if (SDL_GetTicks() - diveStartTime >= DIVE_DURATION) {
            std::cout << "Boss 1 dive timeout\n";
            isDiving = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks(); // Cập nhật thời điểm tấn công
            isRetreating = true; // Bắt đầu lùi lại
            retreatStartX = rect.x; // Lưu vị trí bắt đầu lùi
            std::cout << "Boss 1 dive ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        }
    }

    // Xử lý lướt cho boss 1
    if (isDashing && currentLevel == 1) {
        if (playerRect.x < rect.x && rect.x > 0) {
            rect.x -= DASH_SPEED;
            facingRight = false;
        } else if (playerRect.x > rect.x && rect.x + rect.w < SCREEN_WIDTH) {
            rect.x += DASH_SPEED;
            facingRight = true;
        }

        // Tạo hitbox nhỏ hơn cho lướt
        SDL_Rect attackHitbox = {
            rect.x + (facingRight ? 20 : -20), // Dịch 20 pixel phía trước
            rect.y,                           // Giữ y
            80,                               // Rộng 80
            100                               // Cao 100
        };
        std::cout << "Dash hitbox: x=" << attackHitbox.x << ", y=" << attackHitbox.y << ", w=" << attackHitbox.w << ", h=" << attackHitbox.h << "\n";

        // Kiểm tra va chạm và gây sát thương
        if (SDL_HasIntersection(&attackHitbox, &playerRect)) {
            std::cout << "Boss 1 dash collides with player\n";
            if (!player.IsDead() && !player.IsInvulnerable() && !hasDealtDamage) {
                std::cout << "Boss 1 dashes and hits player!\n";
                player.TakeDamage(1);
                hasDealtDamage = true;
            }
            isDashing = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks(); // Cập nhật thời điểm tấn công
            isRetreating = true; // Bắt đầu lùi lại
            retreatStartX = rect.x; // Lưu vị trí bắt đầu lùi
            std::cout << "Boss 1 dash ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        } else if (SDL_GetTicks() - dashStartTime >= DASH_DURATION) {
            std::cout << "Boss 1 dash timeout\n";
            isDashing = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks(); // Cập nhật thời điểm tấn công
            isRetreating = true; // Bắt đầu lùi lại
            retreatStartX = rect.x; // Lưu vị trí bắt đầu lùi
            std::cout << "Boss 1 dash ends, starting cooldown and retreating from x=" << retreatStartX << "\n";
        }
    }

    // Vật lý (vẫn áp dụng khi idle hoặc retreating để xử lý trọng lực)
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
            if (isDiving) {
                isDiving = false;
                currentAttackFrame = 0;
                lastAttackTime = SDL_GetTicks(); // Cập nhật thời điểm tấn công
                isRetreating = true; // Bắt đầu lùi lại
                retreatStartX = rect.x; // Lưu vị trí bắt đầu lùi
            }
            onPlatform = true;
            break;
        }
    }

    // Kiểm tra chạm mặt đất
    if (rect.y + rect.h >= 500 && !onPlatform) {
        rect.y = 500 - rect.h;
        verticalVelocity = 0;
        isJumping = false;
        if (isDiving) {
            isDiving = false;
            currentAttackFrame = 0;
            lastAttackTime = SDL_GetTicks(); // Cập nhật thời điểm tấn công
            isRetreating = true; // Bắt đầu lùi lại
            retreatStartX = rect.x; // Lưu vị trí bắt đầu lùi
        }
        isOnGround = true;
    }

    // Đặt lại trạng thái sau khi hoàn thành hành động
    if (isAttacking && currentAttackFrame >= attackFrameCount - 1) {
        isAttacking = false;
        currentAttackFrame = 0;
        lastAttackTime = SDL_GetTicks(); // Cập nhật thời điểm tấn công (cho boss 2, 3)
        std::cout << "Boss stops attacking, starting cooldown\n";
        // Kiểm tra idle sau khi tấn công kết thúc (cho boss 2, 3)
        if (currentLevel == 1 && distance > IDLE_DISTANCE) {
            isIdle = true;
            std::cout << "Boss 1 enters idle state after attack\n";
        }
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
    } else if (isIdle || isRetreating) {
        // Hiển thị hoạt ảnh idle khi đứng yên hoặc lùi lại
        srcRect = {0, 0, runFrameWidth, runFrameHeight}; // Giả sử idleTexture có kích thước giống runSheet
        destRect = {rect.x + (rect.w - runFrameWidth) / 2, rect.y + (rect.h - runFrameHeight), runFrameWidth, runFrameHeight};
        SDL_RenderCopyEx(renderer, idleTexture, &srcRect, &destRect, 0, nullptr, flip);
        std::cout << "Rendering idle animation\n";
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
