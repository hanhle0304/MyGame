#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "boss.h"

// Screen and player constants
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

// Player frame dimensions
const int RUN_FRAME_WIDTH = 128;
const int RUN_FRAME_HEIGHT = 128;
const int ATTACK_FRAME_WIDTH = 128;
const int ATTACK_FRAME_HEIGHT = 128;
const int JUMP_FRAME_WIDTH = 128;
const int JUMP_FRAME_HEIGHT = 128;
const int BLOCK_FRAME_WIDTH = 128;
const int BLOCK_FRAME_HEIGHT = 128;

// Boss frame counts
const int BOSS_RUN_FRAME_COUNT = 8;
const int BOSS_ATTACK_FRAME_COUNT = 6;
const int BOSS_JUMP_FRAME_COUNT = 12;
const int BOSS_BLOCK_FRAME_COUNT = 2;

// Boss frame dimensions
const int BOSS_RUN_FRAME_WIDTH = 160;
const int BOSS_RUN_FRAME_HEIGHT = 160;
const int BOSS_ATTACK_FRAME_WIDTH = 160;
const int BOSS_ATTACK_FRAME_HEIGHT = 160;
const int BOSS_JUMP_FRAME_WIDTH = 160;
const int BOSS_JUMP_FRAME_HEIGHT = 160;
const int BOSS_BLOCK_FRAME_WIDTH = 160;
const int BOSS_BLOCK_FRAME_HEIGHT = 160;

const Uint32 FRAME_DELAY = 70;

// Global SDL variables
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
SDL_Texture* g_background = nullptr;
SDL_Texture* g_playerIdle = nullptr;
SDL_Texture* runSheet = nullptr;
SDL_Texture* attackSheet = nullptr;
SDL_Texture* jumpSheet = nullptr;
SDL_Texture* blockSheet = nullptr;
SDL_Texture* platformTexture = nullptr;
SDL_Texture* heartTexture = nullptr;
SDL_Texture* bossIdle = nullptr;
SDL_Texture* bossRunSheet = nullptr;
SDL_Texture* bossAttackSheet = nullptr;
SDL_Texture* bossJumpSheet = nullptr;
SDL_Texture* bossBlockSheet = nullptr;

// Platforms
SDL_Rect platforms[] = {
    {300, 350, 200, 20},
    {600, 300, 150, 20},
    {900, 400, 180, 20}
};

// Player animation states
int currentRunFrame = 0;
int currentAttackFrame = 0;
int currentJumpFrame = 0;
Uint32 lastFrameTime = 0;

// Player physics and state
int verticalVelocity = 0;
bool isJumping = false;
bool isDoubleJumping = false;
bool isAttacking = false;
bool isBlocking = false;
bool isOnGround = false;
bool isDashing = false;
bool canDash = true;
bool facingRight = true;

int dashSpeed = 15;
int dashDuration = 200;
Uint32 dashStartTime = 0;

int health = 5;
const int maxHealth = 5;

bool Init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    g_window = SDL_CreateWindow("SDL2 Spritesheet Animation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!g_window) return false;
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer) return false;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return false;
    return true;
}

SDL_Texture* LoadTexture(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cout << "IMG_Load Error: " << IMG_GetError() << " (file: " << path << ")\n";
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

bool LoadAssets() {
    g_background = LoadTexture("bkground.png");
    g_playerIdle = LoadTexture("player.png");
    runSheet = LoadTexture("run_sheet.png");
    attackSheet = LoadTexture("attack_sheet.png");
    jumpSheet = LoadTexture("jump_sheet.png");
    blockSheet = LoadTexture("block_sheet.png");
    platformTexture = LoadTexture("platform.png");
    heartTexture = LoadTexture("heart.png");
    bossIdle = LoadTexture("boss_assets/idle.png");
    bossRunSheet = LoadTexture("boss_assets/run.png");
    bossAttackSheet = LoadTexture("boss_assets/attack.png");
    bossJumpSheet = LoadTexture("boss_assets/jump.png");
    bossBlockSheet = LoadTexture("boss_assets/block.png");
    return g_background && g_playerIdle && runSheet && attackSheet && jumpSheet && blockSheet &&
           platformTexture && heartTexture && bossIdle && bossRunSheet && bossAttackSheet && bossJumpSheet && bossBlockSheet;
}

void CleanUp() {
    SDL_DestroyTexture(g_background);
    SDL_DestroyTexture(g_playerIdle);
    SDL_DestroyTexture(runSheet);
    SDL_DestroyTexture(attackSheet);
    SDL_DestroyTexture(jumpSheet);
    SDL_DestroyTexture(blockSheet);
    SDL_DestroyTexture(platformTexture);
    SDL_DestroyTexture(heartTexture);
    SDL_DestroyTexture(bossIdle);
    SDL_DestroyTexture(bossRunSheet);
    SDL_DestroyTexture(bossAttackSheet);
    SDL_DestroyTexture(bossJumpSheet);
    SDL_DestroyTexture(bossBlockSheet);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    IMG_Quit();
    SDL_Quit();
}

bool CheckPlatformCollision(SDL_Rect& entity) {
    bool onPlatform = false;
    for (auto& p : platforms) {
        if (entity.y + entity.h <= p.y + 20 &&
            entity.y + entity.h + verticalVelocity >= p.y &&
            entity.x + entity.w > p.x && entity.x < p.x + p.w &&
            verticalVelocity > 0) {
            entity.y = p.y - entity.h;
            verticalVelocity = 0;
            isOnGround = true;
            isJumping = false;
            isDoubleJumping = false;
            canDash = true;
            onPlatform = true;
            break;
        }
    }
    return onPlatform;
}

int main(int argc, char* argv[]) {
    if (!Init() || !LoadAssets()) return -1;

    srand(static_cast<unsigned>(time(0)));

    SDL_Rect playerRect = {100, 400, 100, 100};
    SDL_Event e;
    bool quit = false;
    bool moveLeft = false, moveRight = false;

    Boss boss(800, 0, bossRunSheet, bossAttackSheet, bossJumpSheet, bossBlockSheet);

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_a: moveLeft = true; facingRight = false; break;
                    case SDLK_d: moveRight = true; facingRight = true; break;
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
                        if (!isAttacking) {
                            isAttacking = true;
                            currentAttackFrame = 0;
                            lastFrameTime = SDL_GetTicks();
                        }
                        break;
                    case SDLK_k: isBlocking = true; break;
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
                    case SDLK_a: moveLeft = false; break;
                    case SDLK_d: moveRight = false; break;
                    case SDLK_k: isBlocking = false; break;
                }
            }
        }

        if (!isBlocking) {
            if (isDashing) {
                int speed = dashSpeed;
                if (facingRight && playerRect.x + playerRect.w + speed < SCREEN_WIDTH) playerRect.x += speed;
                else if (!facingRight && playerRect.x - speed > 0) playerRect.x -= speed;
                if (SDL_GetTicks() - dashStartTime > dashDuration) isDashing = false;
            } else {
                if (moveLeft && playerRect.x > 0) playerRect.x -= PLAYER_SPEED;
                if (moveRight && playerRect.x + playerRect.w < SCREEN_WIDTH) playerRect.x += PLAYER_SPEED;
            }
        }

        verticalVelocity += GRAVITY;
        playerRect.y += verticalVelocity;
        isOnGround = CheckPlatformCollision(playerRect);

        if (playerRect.y + playerRect.h >= 500 && !isOnGround) {
            playerRect.y = 500 - playerRect.h;
            verticalVelocity = 0;
            isJumping = false;
            isDoubleJumping = false;
            isOnGround = true;
            canDash = true;
        }

        boss.Update(playerRect, platforms, sizeof(platforms)/sizeof(platforms[0]));

        if (boss.IsAttacking() && SDL_HasIntersection(&playerRect, &boss.GetRect())) {
            if (!isBlocking) {
                health -= 1; // Giảm 1 máu (1 trái tim)
                if (health < 0) health = 0;
            }
        }

        // Nhân vật tấn công boss
        if (isAttacking && SDL_HasIntersection(&playerRect, &boss.GetRect())) {
            if (!boss.IsBlocking()) {
                boss.ReduceHealth(3); // Gây 3 sát thương
            }
        }

        SDL_RenderClear(g_renderer);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
        SDL_RenderCopy(g_renderer, g_background, nullptr, nullptr);

        for (auto& p : platforms)
            SDL_RenderCopy(g_renderer, platformTexture, nullptr, &p);

        // Player health (hearts)
        for (int i = 0; i < health; ++i) {
            SDL_Rect heartRect = {10 + i * 40, 10, 32, 32};
            SDL_RenderCopy(g_renderer, heartTexture, nullptr, &heartRect);
        }

        // Boss health bar
        SDL_Rect bossHealthBarRect = {SCREEN_WIDTH - 410, 10, 400, 30};
        int bossHealthWidth = (boss.GetHealth() * 400) / boss.GetMaxHealth();
        SDL_SetRenderDrawColor(g_renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(g_renderer, &bossHealthBarRect);
        SDL_SetRenderDrawColor(g_renderer, boss.GetHealth() < 30 ? 255 : 0, boss.GetHealth() < 30 ? 0 : 255, 0, 255);
        SDL_Rect bossHealthRect = {SCREEN_WIDTH - 410, 10, bossHealthWidth, 30};
        SDL_RenderFillRect(g_renderer, &bossHealthRect);

        Uint32 currentTime = SDL_GetTicks();
        SDL_Rect srcRect, destRect;
        SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

        if (isBlocking) {
            srcRect = {0, 0, BLOCK_FRAME_WIDTH, BLOCK_FRAME_HEIGHT};
            destRect = {playerRect.x + (playerRect.w - BLOCK_FRAME_WIDTH) / 2, playerRect.y + (playerRect.h - BLOCK_FRAME_HEIGHT), BLOCK_FRAME_WIDTH, BLOCK_FRAME_HEIGHT};
            SDL_RenderCopyEx(g_renderer, blockSheet, &srcRect, &destRect, 0, nullptr, flip);
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
            destRect = {playerRect.x + (playerRect.w - ATTACK_FRAME_WIDTH) / 2, playerRect.y + (playerRect.h - ATTACK_FRAME_HEIGHT), ATTACK_FRAME_WIDTH, ATTACK_FRAME_HEIGHT};
            SDL_RenderCopyEx(g_renderer, attackSheet, &srcRect, &destRect, 0, nullptr, flip);
        } else if (isJumping || isDoubleJumping) {
            if (currentTime > lastFrameTime + FRAME_DELAY) {
                lastFrameTime = currentTime;
                currentJumpFrame = (currentJumpFrame + 1) % JUMP_FRAME_COUNT;
            }
            srcRect = {currentJumpFrame * JUMP_FRAME_WIDTH, 0, JUMP_FRAME_WIDTH, JUMP_FRAME_HEIGHT};
            destRect = {playerRect.x + (playerRect.w - JUMP_FRAME_WIDTH) / 2, playerRect.y + (playerRect.h - JUMP_FRAME_HEIGHT), JUMP_FRAME_WIDTH, JUMP_FRAME_HEIGHT};
            SDL_RenderCopyEx(g_renderer, jumpSheet, &srcRect, &destRect, 0, nullptr, flip);
        } else if (moveLeft || moveRight) {
            if (currentTime > lastFrameTime + FRAME_DELAY) {
                lastFrameTime = currentTime;
                currentRunFrame = (currentRunFrame + 1) % RUN_FRAME_COUNT;
            }
            srcRect = {currentRunFrame * RUN_FRAME_WIDTH, 0, RUN_FRAME_WIDTH, RUN_FRAME_HEIGHT};
            destRect = {playerRect.x + (playerRect.w - RUN_FRAME_WIDTH) / 2, playerRect.y + (playerRect.h - RUN_FRAME_HEIGHT), RUN_FRAME_WIDTH, RUN_FRAME_HEIGHT};
            SDL_RenderCopyEx(g_renderer, runSheet, &srcRect, &destRect, 0, nullptr, flip);
        } else {
            SDL_RenderCopyEx(g_renderer, g_playerIdle, nullptr, &playerRect, 0, nullptr, flip);
        }

        boss.Render(g_renderer, bossIdle, currentTime);

        SDL_RenderPresent(g_renderer);
        SDL_Delay(16);
    }

    CleanUp();
    return 0;
}
