#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "boss.h"
#include "player.h"

// Screen dimensions
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;

// Global SDL variables
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
SDL_Texture* level1Background = nullptr;
SDL_Texture* level2Background = nullptr;
SDL_Texture* level3Background = nullptr;
SDL_Texture* platformTexture = nullptr;
SDL_Texture* heartTexture = nullptr;
SDL_Texture* gameOverTexture = nullptr;
SDL_Texture* levelCompleteTexture = nullptr;

// Boss textures cho các màn
SDL_Texture* boss1Idle = nullptr;
SDL_Texture* boss1RunSheet = nullptr;
SDL_Texture* boss1AttackSheet = nullptr;
SDL_Texture* boss1JumpSheet = nullptr;
SDL_Texture* boss1BlockSheet = nullptr;
SDL_Texture* boss1DamageSheet = nullptr;
SDL_Texture* boss1DeathSheet = nullptr;
SDL_Texture* boss2Idle = nullptr;
SDL_Texture* boss2RunSheet = nullptr;
SDL_Texture* boss2AttackSheet = nullptr;
SDL_Texture* boss2JumpSheet = nullptr;
SDL_Texture* boss2BlockSheet = nullptr;
SDL_Texture* boss2DamageSheet = nullptr;
SDL_Texture* boss2DeathSheet = nullptr;
SDL_Texture* boss3Idle = nullptr;
SDL_Texture* boss3RunSheet = nullptr;
SDL_Texture* boss3AttackSheet = nullptr;
SDL_Texture* boss3JumpSheet = nullptr;
SDL_Texture* boss3BlockSheet = nullptr;
SDL_Texture* boss3DamageSheet = nullptr;
SDL_Texture* boss3DeathSheet = nullptr;

// Platforms cho các màn
SDL_Rect level1Platforms[] = {
    {300, 350, 200, 20},
    {600, 300, 150, 20},
    {900, 400, 180, 20}
};

SDL_Rect level2Platforms[] = {
    {250, 380, 180, 20},
    {550, 300, 200, 20},
    {850, 350, 150, 20}
};

SDL_Rect level3Platforms[] = {
    {200, 360, 180, 20},
    {500, 320, 200, 20},
    {800, 380, 150, 20}
};

// Game state
int currentLevel = 1;
bool levelTransition = false;
bool showGameOver = false;
bool showLevelComplete = false;
const Uint32 FRAME_DELAY = 70;
const int DEATH_FRAME_COUNT = 4;
const Uint32 LEVEL_COMPLETE_DURATION = 3000; // Thời gian hiển thị hoạt ảnh kết thúc màn

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

bool LoadAssets(SDL_Texture*& playerIdle, SDL_Texture*& runSheet, SDL_Texture*& attackSheet,
                SDL_Texture*& jumpSheet, SDL_Texture*& blockSheet, SDL_Texture*& damageSheet,
                SDL_Texture*& deathSheet) {
    level1Background = LoadTexture("map_and_objects/level1_background.png");
    level2Background = LoadTexture("map_and_objects/level2_background.png");
    level3Background = LoadTexture("map_and_objects/level3_background.png");
    platformTexture = LoadTexture("map_and_objects/platform.png");
    heartTexture = LoadTexture("map_and_objects/heart.png");
    gameOverTexture = LoadTexture("map_and_objects/game_over.png");
    levelCompleteTexture = LoadTexture("map_and_objects/level_complete.png");
    playerIdle = LoadTexture("player_assets/idle.png");
    runSheet = LoadTexture("player_assets/run.png");
    attackSheet = LoadTexture("player_assets/attack.png");
    jumpSheet = LoadTexture("player_assets/jump.png");
    blockSheet = LoadTexture("player_assets/block.png");
    damageSheet = LoadTexture("player_assets/damage.png");
    deathSheet = LoadTexture("player_assets/death.png");
    // Boss màn 1
    boss1Idle = LoadTexture("boss_assets/boss1/idle.png");
    boss1RunSheet = LoadTexture("boss_assets/boss1/run.png");
    boss1AttackSheet = LoadTexture("boss_assets/boss1/attack.png");
    boss1JumpSheet = LoadTexture("boss_assets/boss1/jump.png");
    boss1BlockSheet = LoadTexture("boss_assets/boss1/block.png");
    boss1DamageSheet = LoadTexture("boss_assets/boss1/damage.png");
    boss1DeathSheet = LoadTexture("boss_assets/boss1/death.png");
    // Boss màn 2
    boss2Idle = LoadTexture("boss_assets/boss2/idle.png");
    boss2RunSheet = LoadTexture("boss_assets/boss2/run.png");
    boss2AttackSheet = LoadTexture("boss_assets/boss2/attack.png");
    boss2JumpSheet = LoadTexture("boss_assets/boss2/jump.png");
    boss2BlockSheet = LoadTexture("boss_assets/boss2/block.png");
    boss2DamageSheet = LoadTexture("boss_assets/boss2/damage.png");
    boss2DeathSheet = LoadTexture("boss_assets/boss2/death.png");
    // Boss màn 3
    boss3Idle = LoadTexture("boss_assets/boss3/idle.png");
    boss3RunSheet = LoadTexture("boss_assets/boss3/run.png");
    boss3AttackSheet = LoadTexture("boss_assets/boss3/attack.png");
    boss3JumpSheet = LoadTexture("boss_assets/boss3/jump.png");
    boss3BlockSheet = LoadTexture("boss_assets/boss3/block.png");
    boss3DamageSheet = LoadTexture("boss_assets/boss3/damage.png");
    boss3DeathSheet = LoadTexture("boss_assets/boss3/death.png");
    return level1Background && level2Background && level3Background && platformTexture && heartTexture &&
           gameOverTexture && levelCompleteTexture && playerIdle && runSheet && attackSheet &&
           jumpSheet && blockSheet && damageSheet && deathSheet &&
           boss1Idle && boss1RunSheet && boss1AttackSheet && boss1JumpSheet && boss1BlockSheet &&
           boss1DamageSheet && boss1DeathSheet &&
           boss2Idle && boss2RunSheet && boss2AttackSheet && boss2JumpSheet && boss2BlockSheet &&
           boss2DamageSheet && boss2DeathSheet &&
           boss3Idle && boss3RunSheet && boss3AttackSheet && boss3JumpSheet && boss3BlockSheet &&
           boss3DamageSheet && boss3DeathSheet;
}

void CleanUp(SDL_Texture* playerIdle, SDL_Texture* runSheet, SDL_Texture* attackSheet,
             SDL_Texture* jumpSheet, SDL_Texture* blockSheet, SDL_Texture* damageSheet,
             SDL_Texture* deathSheet) {
    SDL_DestroyTexture(level1Background);
    SDL_DestroyTexture(level2Background);
    SDL_DestroyTexture(level3Background);
    SDL_DestroyTexture(platformTexture);
    SDL_DestroyTexture(heartTexture);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(levelCompleteTexture);
    SDL_DestroyTexture(playerIdle);
    SDL_DestroyTexture(runSheet);
    SDL_DestroyTexture(attackSheet);
    SDL_DestroyTexture(jumpSheet);
    SDL_DestroyTexture(blockSheet);
    SDL_DestroyTexture(damageSheet);
    SDL_DestroyTexture(deathSheet);
    SDL_DestroyTexture(boss1Idle);
    SDL_DestroyTexture(boss1RunSheet);
    SDL_DestroyTexture(boss1AttackSheet);
    SDL_DestroyTexture(boss1JumpSheet);
    SDL_DestroyTexture(boss1BlockSheet);
    SDL_DestroyTexture(boss1DamageSheet);
    SDL_DestroyTexture(boss1DeathSheet);
    SDL_DestroyTexture(boss2Idle);
    SDL_DestroyTexture(boss2RunSheet);
    SDL_DestroyTexture(boss2AttackSheet);
    SDL_DestroyTexture(boss2JumpSheet);
    SDL_DestroyTexture(boss2BlockSheet);
    SDL_DestroyTexture(boss2DamageSheet);
    SDL_DestroyTexture(boss2DeathSheet);
    SDL_DestroyTexture(boss3Idle);
    SDL_DestroyTexture(boss3RunSheet);
    SDL_DestroyTexture(boss3AttackSheet);
    SDL_DestroyTexture(boss3JumpSheet);
    SDL_DestroyTexture(boss3BlockSheet);
    SDL_DestroyTexture(boss3DamageSheet);
    SDL_DestroyTexture(boss3DeathSheet);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!Init()) return -1;

    srand(static_cast<unsigned>(time(0)));

    // Load textures
    SDL_Texture *playerIdle, *runSheet, *attackSheet, *jumpSheet, *blockSheet, *damageSheet, *deathSheet;
    if (!LoadAssets(playerIdle, runSheet, attackSheet, jumpSheet, blockSheet, damageSheet, deathSheet)) {
        CleanUp(playerIdle, runSheet, attackSheet, jumpSheet, blockSheet, damageSheet, deathSheet);
        return -1;
    }

// Initialize player and boss (màn 1)
    Player player(120, 400, playerIdle, runSheet, attackSheet, jumpSheet, blockSheet, damageSheet, deathSheet);
    Boss boss(800, 0,
              boss1RunSheet, 8, 128, 128,      // Run: 8 khung, 128x128
              boss1AttackSheet, 5, 128, 128,   // Attack: 6 khung, 128x128
              boss1JumpSheet, 9, 128, 128,    // Jump: 12 khung, 128x128
              boss1BlockSheet, 2, 128, 128,    // Block: 2 khung, 128x128
              boss1DamageSheet, 3, 128, 128,   // Damage: 2 khung, 128x128
              boss1DeathSheet, 5, 128, 128);   // Death: 4 khung, 128x128

    SDL_Event e;
    bool quit = false;

    // Biến theo dõi trạng thái chết của boss và hoạt ảnh kết thúc màn
    static bool bossDeathAnimationStarted = false;
    static Uint32 bossDeathStartTime = 0;
    static Uint32 levelCompleteStartTime = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN && showGameOver && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            } else {
                player.HandleInput(e);
            }
        }

        if (!showGameOver && !levelTransition && !showLevelComplete) {
            // Cập nhật player và boss
            player.Update(currentLevel == 1 ? level1Platforms : (currentLevel == 2 ? level2Platforms : level3Platforms), 3);
            boss.Update(player.GetRect(), currentLevel == 1 ? level1Platforms : (currentLevel == 2 ? level2Platforms : level3Platforms), 3);

            // Xử lý tấn công của boss
            if (!player.IsDead() && boss.IsAttacking() && SDL_HasIntersection(&player.GetRect(), &boss.GetRect())) {
                if (!player.IsBlocking() && !boss.HasDealtDamage()) {
                    player.TakeDamage(1);
                    boss.SetDealtDamage(true);
                }
            }

            // Nhân vật tấn công boss
            if (player.IsAttacking() && SDL_HasIntersection(&player.GetRect(), &boss.GetRect())) {
                if (!boss.IsBlocking()) {
                    boss.ReduceHealth(1);
                }
            }
        }

        // Kiểm tra boss chết để bắt đầu hoạt ảnh chết
        if (boss.GetHealth() <= 0 && !levelTransition && !showLevelComplete && !bossDeathAnimationStarted) {
            bossDeathAnimationStarted = true;
            bossDeathStartTime = SDL_GetTicks();
        }

        // Chuyển sang hoạt ảnh kết thúc màn sau khi hoạt ảnh chết hoàn tất
        if (bossDeathAnimationStarted && !levelTransition && !showLevelComplete) {
            if (SDL_GetTicks() - bossDeathStartTime >= DEATH_FRAME_COUNT * FRAME_DELAY) {
                showLevelComplete = true;
                levelCompleteStartTime = SDL_GetTicks();
            }
        }

        // Chuyển màn sau khi hoạt ảnh kết thúc màn hoàn tất
if (showLevelComplete && SDL_GetTicks() - levelCompleteStartTime >= LEVEL_COMPLETE_DURATION) {
            levelTransition = true;
            if (currentLevel == 1) {
                currentLevel = 2;
                player.Reset();
                boss = Boss(800, 0,
                            boss2RunSheet, 8, 128, 128,      // Run: 6 khung, 150x150
                            boss2AttackSheet, 4, 128, 128,   // Attack: 5 khung, 160x160
                            boss2JumpSheet, 7, 128, 128,    // Jump: 10 khung, 140x140
                            boss2BlockSheet, 2, 128, 128,    // Block: 3 khung, 145x145
                            boss2DamageSheet, 2, 128, 128,   // Damage: 3 khung, 150x150
                            boss2DeathSheet, 6, 128, 128);   // Death: 5 khung, 150x150
                levelTransition = false;
                showLevelComplete = false;
                bossDeathAnimationStarted = false;
            } else if (currentLevel == 2) {
                currentLevel = 3;
                player.Reset();
                boss = Boss(800, 0,
                            boss3RunSheet, 7, 128, 128,      // Run: 7 khung, 140x140
                            boss3AttackSheet, 5, 128, 128,   // Attack: 7 khung, 150x150
                            boss3JumpSheet, 6, 128, 128,     // Jump: 8 khung, 130x130
                            boss3BlockSheet, 1, 128, 128,    // Block: 2 khung, 140x140
                            boss3DamageSheet, 2, 128, 128,   // Damage: 4 khung, 140x140
                            boss3DeathSheet, 6, 128, 128);   // Death: 6 khung, 140x140
                levelTransition = false;
                showLevelComplete = false;
                bossDeathAnimationStarted = false;
            }
        }

        // Render
        SDL_RenderClear(g_renderer);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);

        if (showGameOver) {
            SDL_RenderCopy(g_renderer, gameOverTexture, nullptr, nullptr);
            boss.Render(g_renderer, currentLevel == 1 ? boss1Idle : (currentLevel == 2 ? boss2Idle : boss3Idle), SDL_GetTicks());
        } else if (showLevelComplete) {
            SDL_RenderCopy(g_renderer, levelCompleteTexture, nullptr, nullptr);
        } else {
            // Hiển thị background
            SDL_Texture* currentBackground = currentLevel == 1 ? level1Background : (currentLevel == 2 ? level2Background : level3Background);
            SDL_RenderCopy(g_renderer, currentBackground, nullptr, nullptr);

            // Hiển thị platforms
            SDL_Rect* currentPlatforms = currentLevel == 1 ? level1Platforms : (currentLevel == 2 ? level2Platforms : level3Platforms);
            for (int i = 0; i < 3; ++i) {
                SDL_RenderCopy(g_renderer, platformTexture, nullptr, &currentPlatforms[i]);
            }

            // Hiển thị sức khỏe của player
            for (int i = 0; i < player.GetHealth(); ++i) {
                SDL_Rect heartRect = {10 + i * 40, 10, 32, 32};
                SDL_RenderCopy(g_renderer, heartTexture, nullptr, &heartRect);
            }

            // Hiển thị thanh sức khỏe của boss
            SDL_Rect bossHealthBarRect = {SCREEN_WIDTH - 410, 10, 400, 30};
            int bossHealthWidth = (boss.GetHealth() * 400) / boss.GetMaxHealth();
            SDL_SetRenderDrawColor(g_renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(g_renderer, &bossHealthBarRect);
            SDL_SetRenderDrawColor(g_renderer, boss.GetHealth() < 30 ? 255 : 0, boss.GetHealth() < 30 ? 0 : 255, 0, 255);
            SDL_Rect bossHealthRect = {SCREEN_WIDTH - 410, 10, bossHealthWidth, 30};
            SDL_RenderFillRect(g_renderer, &bossHealthRect);

            // Render player và boss
            player.Render(g_renderer);
            boss.Render(g_renderer, currentLevel == 1 ? boss1Idle : (currentLevel == 2 ? boss2Idle : boss3Idle), SDL_GetTicks());

            // Kiểm tra trạng thái chết của player để hiển thị Game Over
            if (player.IsDead() && SDL_GetTicks() - bossDeathStartTime >= DEATH_FRAME_COUNT * FRAME_DELAY) {
                showGameOver = true;
            }
        }

        SDL_RenderPresent(g_renderer);
        SDL_Delay(16);
    }

    CleanUp(playerIdle, runSheet, attackSheet, jumpSheet, blockSheet, damageSheet, deathSheet);
    return 0;
}
