#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "boss.h"
#include "player.h"
#include "gui.h"

// Screen dimensions
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;

// Global SDL variables
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
SDL_Texture* level1Background = nullptr;
SDL_Texture* level2Background = nullptr;
SDL_Texture* platformTexture = nullptr;
SDL_Texture* heartTexture = nullptr;
SDL_Texture* gameOverTexture = nullptr;
SDL_Texture* levelCompleteTexture = nullptr;
SDL_Texture* gameCompleteTexture = nullptr;

// Boss textures cho các màn
SDL_Texture* boss1Idle = nullptr;
SDL_Texture* boss1RunSheet = nullptr;
SDL_Texture* boss1AttackSheet = nullptr;
SDL_Texture* boss1JumpSheet = nullptr;
SDL_Texture* boss1DamageSheet = nullptr;
SDL_Texture* boss1DeathSheet = nullptr;
SDL_Texture* boss1DiveSheet = nullptr;
SDL_Texture* boss2Idle = nullptr;
SDL_Texture* boss2RunSheet = nullptr;
SDL_Texture* boss2AttackSheet = nullptr;
SDL_Texture* boss2JumpSheet = nullptr;
SDL_Texture* boss2DamageSheet = nullptr;
SDL_Texture* boss2DeathSheet = nullptr;

// MiniBoss textures
SDL_Texture* miniBossIdle = nullptr;
SDL_Texture* miniBossRunSheet = nullptr;
SDL_Texture* miniBossAttackSheet = nullptr;
SDL_Texture* miniBossJumpSheet = nullptr;
SDL_Texture* miniBossDamageSheet = nullptr;
SDL_Texture* miniBossDeathSheet = nullptr;
SDL_Texture* miniBossDiveSheet = nullptr;
SDL_Texture* miniBossShootSheet = nullptr;
SDL_Texture* miniBossArrowTexture = nullptr;

// Platforms cho các màn
SDL_Rect level1Platforms[] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

SDL_Rect level2Platforms[] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

// Game state
int currentLevel = 1;
bool levelTransition = false;
bool showGameOver = false;
bool showLevelComplete = false;
bool showGameComplete = false;
const Uint32 FRAME_DELAY = 70;
const int DEATH_FRAME_COUNT = 4;
const Uint32 LEVEL_COMPLETE_DURATION = 3000;
const Uint32 GAME_COMPLETE_DURATION = 3000;

bool Init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << "\n";
        return false;
    }
    g_window = SDL_CreateWindow("SDL2 Spritesheet Animation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!g_window) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        return false;
    }
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer) {
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cout << "IMG_Init Error: " << IMG_GetError() << "\n";
        return false;
    }
    if (TTF_Init() < 0) {
        std::cout << "TTF_Init Error: " << TTF_GetError() << "\n";
        return false;
    }
    if (Mix_Init(MIX_INIT_MP3) < 0) {
        std::cout << "Mix_Init Error: " << Mix_GetError() << "\n";
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "Mix_OpenAudio Error: " << Mix_GetError() << "\n";
        return false;
    }
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
    if (!texture) {
        std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << " (file: " << path << ")\n";
    } else {
        std::cout << "Successfully loaded texture: " << path << "\n";
    }
    return texture;
}

bool LoadAssets(SDL_Texture*& playerIdle, SDL_Texture*& runSheet, SDL_Texture*& attackSheet,
                SDL_Texture*& jumpSheet, SDL_Texture*& damageSheet,
                SDL_Texture*& deathSheet) {
    level1Background = LoadTexture("assets/map_and_objects/level1_background.png");
    level2Background = LoadTexture("assets/map_and_objects/level2_background.png");
    platformTexture = LoadTexture("assets/map_and_objects/platform.png");
    heartTexture = LoadTexture("assets/map_and_objects/heart.png");
    gameOverTexture = LoadTexture("assets/map_and_objects/game_over.png");
    levelCompleteTexture = LoadTexture("assets/map_and_objects/level_complete.png");
    gameCompleteTexture = LoadTexture("assets/map_and_objects/game_complete.png");
    playerIdle = LoadTexture("assets/player_assets/idle.png");
    runSheet = LoadTexture("assets/player_assets/run.png");
    attackSheet = LoadTexture("assets/player_assets/attack.png");
    jumpSheet = LoadTexture("assets/player_assets/jump.png");
    damageSheet = LoadTexture("assets/player_assets/damage.png");
    deathSheet = LoadTexture("assets/player_assets/death.png");
    // Boss màn 1
    boss1Idle = LoadTexture("assets/boss_assets/boss1/idle.png");
    boss1RunSheet = LoadTexture("assets/boss_assets/boss1/run.png");
    boss1AttackSheet = LoadTexture("assets/boss_assets/boss1/attack.png");
    boss1JumpSheet = LoadTexture("assets/boss_assets/boss1/jump.png");
    boss1DamageSheet = LoadTexture("assets/boss_assets/boss1/damage.png");
    boss1DeathSheet = LoadTexture("assets/boss_assets/boss1/death.png");
    boss1DiveSheet = LoadTexture("assets/boss_assets/boss1/dive.png");
    // Boss màn 2
    boss2Idle = LoadTexture("assets/boss_assets/boss2/idle.png");
    boss2RunSheet = LoadTexture("assets/boss_assets/boss2/run.png");
    boss2AttackSheet = LoadTexture("assets/boss_assets/boss2/attack.png");
    boss2JumpSheet = LoadTexture("assets/boss_assets/boss2/jump.png");
    boss2DamageSheet = LoadTexture("assets/boss_assets/boss2/damage.png");
    boss2DeathSheet = LoadTexture("assets/boss_assets/boss2/death.png");
    // MiniBoss textures
    miniBossIdle = LoadTexture("assets/miniboss/idle.png");
    miniBossRunSheet = LoadTexture("assets/miniboss/run.png");
    miniBossAttackSheet = LoadTexture("assets/miniboss/attack.png");
    miniBossJumpSheet = LoadTexture("assets/miniboss/jump.png");
    miniBossDamageSheet = LoadTexture("assets/miniboss/damage.png");
    miniBossDeathSheet = LoadTexture("assets/miniboss/death.png");
    miniBossDiveSheet = LoadTexture("assets/miniboss/dive.png");
    miniBossShootSheet = LoadTexture("assets/miniboss/shoot.png");
    miniBossArrowTexture = LoadTexture("assets/miniboss/arrow.png");

    return level1Background && level2Background && platformTexture && heartTexture &&
           gameOverTexture && levelCompleteTexture && gameCompleteTexture && playerIdle && runSheet && attackSheet &&
           jumpSheet && damageSheet && deathSheet &&
           boss1Idle && boss1RunSheet && boss1AttackSheet && boss1JumpSheet &&
           boss1DamageSheet && boss1DeathSheet && boss1DiveSheet &&
           boss2Idle && boss2RunSheet && boss2AttackSheet && boss2JumpSheet &&
           boss2DamageSheet && boss2DeathSheet &&
           miniBossIdle && miniBossRunSheet && miniBossAttackSheet && miniBossJumpSheet &&
           miniBossDamageSheet && miniBossDeathSheet && miniBossDiveSheet && miniBossShootSheet && miniBossArrowTexture;
}

void CleanUp(SDL_Texture* playerIdle, SDL_Texture* runSheet, SDL_Texture* attackSheet,
             SDL_Texture* jumpSheet, SDL_Texture* damageSheet,
             SDL_Texture* deathSheet, TTF_Font* font, Mix_Music* music, Mix_Chunk* gameOverSound, Mix_Chunk* attackSound) {
    SDL_DestroyTexture(level1Background);
    SDL_DestroyTexture(level2Background);
    SDL_DestroyTexture(platformTexture);
    SDL_DestroyTexture(heartTexture);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(levelCompleteTexture);
    SDL_DestroyTexture(gameCompleteTexture);
    SDL_DestroyTexture(playerIdle);
    SDL_DestroyTexture(runSheet);
    SDL_DestroyTexture(attackSheet);
    SDL_DestroyTexture(jumpSheet);
    SDL_DestroyTexture(damageSheet);
    SDL_DestroyTexture(deathSheet);
    SDL_DestroyTexture(boss1Idle);
    SDL_DestroyTexture(boss1RunSheet);
    SDL_DestroyTexture(boss1AttackSheet);
    SDL_DestroyTexture(boss1JumpSheet);
    SDL_DestroyTexture(boss1DamageSheet);
    SDL_DestroyTexture(boss1DeathSheet);
    SDL_DestroyTexture(boss1DiveSheet);
    SDL_DestroyTexture(boss2Idle);
    SDL_DestroyTexture(boss2RunSheet);
    SDL_DestroyTexture(boss2AttackSheet);
    SDL_DestroyTexture(boss2JumpSheet);
    SDL_DestroyTexture(boss2DamageSheet);
    SDL_DestroyTexture(boss2DeathSheet);
    SDL_DestroyTexture(miniBossIdle);
    SDL_DestroyTexture(miniBossRunSheet);
    SDL_DestroyTexture(miniBossAttackSheet);
    SDL_DestroyTexture(miniBossJumpSheet);
    SDL_DestroyTexture(miniBossDamageSheet);
    SDL_DestroyTexture(miniBossDeathSheet);
    SDL_DestroyTexture(miniBossDiveSheet);
    SDL_DestroyTexture(miniBossShootSheet);
    SDL_DestroyTexture(miniBossArrowTexture);
    if (font) TTF_CloseFont(font);
    if (music) Mix_FreeMusic(music);
    if (gameOverSound) Mix_FreeChunk(gameOverSound);
    if (attackSound) Mix_FreeChunk(attackSound); // Giải phóng âm thanh tấn công
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!Init()) {
        std::cout << "Initialization failed\n";
        return -1;
    }

    srand(static_cast<unsigned>(time(0)));

    // Load font
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 24);
    if (!font) {
        std::cout << "TTF_OpenFont Error: " << TTF_GetError() << "\n";
        CleanUp(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        return -1;
    }

    // Load music
    Mix_Music* backgroundMusic = Mix_LoadMUS("assets/audio/background_music.mp3");
    if (!backgroundMusic) {
        std::cout << "Mix_LoadMUS Error: " << Mix_GetError() << "\n";
        CleanUp(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, font, nullptr, nullptr, nullptr);
        return -1;
    }

    // Load Game Over sound
    Mix_Chunk* gameOverSound = Mix_LoadWAV("assets/audio/game_over.mp3");
    if (!gameOverSound) {
        std::cout << "Mix_LoadWAV Error: " << Mix_GetError() << "\n";
        CleanUp(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, font, backgroundMusic, nullptr, nullptr);
        return -1;
    }

    // Load Attack sound
    Mix_Chunk* attackSound = Mix_LoadWAV("assets/audio/attack.mp3");
    if (!attackSound) {
        std::cout << "Mix_LoadWAV Error: " << Mix_GetError() << "\n";
        CleanUp(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, font, backgroundMusic, gameOverSound, nullptr);
        return -1;
    }

    // Load textures
    SDL_Texture *playerIdle, *runSheet, *attackSheet, *jumpSheet, *damageSheet, *deathSheet;
    if (!LoadAssets(playerIdle, runSheet, attackSheet, jumpSheet, damageSheet, deathSheet)) {
        CleanUp(playerIdle, runSheet, attackSheet, jumpSheet, damageSheet, deathSheet, font, backgroundMusic, gameOverSound, attackSound);
        return -1;
    }

    // Initialize GUI
    GameState state = GameState::MENU;
    GUI gui(g_renderer, font, backgroundMusic, gameOverSound, attackSound); // Truyền attackSound

    // Initialize player and boss (màn 1)
    Player player(120, 400, playerIdle, runSheet, attackSheet, jumpSheet, damageSheet, deathSheet);
    Boss boss(800, 0,
              boss1RunSheet, 8, 128, 128,
              boss1AttackSheet, 5, 128, 128,
              boss1JumpSheet, 9, 128, 128,
              boss1DamageSheet, 3, 128, 128,
              boss1DeathSheet, 5, 128, 128,
              boss1DiveSheet, 5, 128, 128);

    SDL_Event e;
    bool quit = false;
    bool musicStarted = false;
    bool gameOverSoundPlayed = false;
    bool gameCompleteSoundPlayed = false;
    bool wasAttacking = false; // Theo dõi trạng thái tấn công trước đó

    // Biến theo dõi trạng thái chết của boss và hoạt ảnh
    bool bossDeathAnimationStarted = false;
    Uint32 bossDeathStartTime = 0;
    Uint32 levelCompleteStartTime = 0;
    Uint32 gameCompleteStartTime = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (state == GameState::MENU) {
                gui.Update(e, state);
            } else if (state == GameState::PLAYING) {
                if (e.type == SDL_KEYDOWN && (showGameOver || showGameComplete) && e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else {
                    player.HandleInput(e);
                }
            }
        }

        if (state == GameState::MENU) {
            gui.Render();
        } else if (state == GameState::PLAYING) {
            // Bắt đầu phát nhạc nếu chưa phát
            if (!musicStarted && Mix_PausedMusic() == 0) {
                Mix_PlayMusic(backgroundMusic, -1);
                std::cout << "Background music started\n";
                musicStarted = true;
            }

            if (!showGameOver && !levelTransition && !showLevelComplete && !showGameComplete) {
                // Cập nhật player và boss
                player.Update(currentLevel == 1 ? level1Platforms : level2Platforms, 3);
                boss.Update(player.GetRect(), currentLevel == 1 ? level1Platforms : level2Platforms, 3, currentLevel, player);

                // Phát âm thanh tấn công
                bool isAttacking = player.IsAttacking();
                if (isAttacking && !wasAttacking && gui.IsSoundEnabled()) {
                    Mix_PlayChannel(-1, attackSound, 0);
                    std::cout << "Player attack sound played\n";
                }
                wasAttacking = isAttacking;

                // Debug trạng thái
                std::cout << "Boss isAttacking: " << boss.IsAttacking() << ", isDashing: " << boss.IsDashing() << ", isDiving: " << boss.IsDiving() << "\n";
                std::cout << "Player isAttacking: " << player.IsAttacking() << "\n";
                if (SDL_HasIntersection(&player.GetRect(), &boss.GetRect())) {
                    std::cout << "Collision detected between player and boss\n";
                }

                // Xử lý tấn công của boss 2
                if (!player.IsDead() && currentLevel == 2 && boss.IsAttacking() && SDL_HasIntersection(&player.GetRect(), &boss.GetRect())) {
                    if (!boss.HasDealtDamage() && !player.IsInvulnerable()) {
                        std::cout << "Boss hits player!\n";
                        player.TakeDamage(1);
                        boss.SetDealtDamage(true);
                    }
                } else if (!SDL_HasIntersection(&player.GetRect(), &boss.GetRect())) {
                    boss.SetDealtDamage(false);
                }

                // Nhân vật tấn công boss
                if (player.IsAttacking() && SDL_HasIntersection(&player.GetRect(), &boss.GetRect())) {
                    std::cout << "Player attacks boss!\n";
                    boss.ReduceHealth(1);
                }
            }

            // Kiểm tra boss chết để bắt đầu hoạt ảnh chết
            if (boss.GetHealth() <= 0 && !levelTransition && !showLevelComplete && !bossDeathAnimationStarted && !showGameComplete) {
                bossDeathAnimationStarted = true;
                bossDeathStartTime = SDL_GetTicks();
            }

            // Chuyển sang hoạt ảnh kết thúc màn sau khi hoạt ảnh chết hoàn tất
            if (bossDeathAnimationStarted && !levelTransition && !showLevelComplete && !showGameComplete) {
                if (SDL_GetTicks() - bossDeathStartTime >= DEATH_FRAME_COUNT * FRAME_DELAY) {
                    showLevelComplete = true;
                    levelCompleteStartTime = SDL_GetTicks();
                }
            }

            // Chuyển màn hoặc hiển thị hoàn thành game
            if (showLevelComplete && SDL_GetTicks() - levelCompleteStartTime >= LEVEL_COMPLETE_DURATION) {
                levelTransition = true;
                if (currentLevel == 1) {
                    currentLevel = 2;
                    player.Reset();
                    boss = Boss(800, 0,
                                boss2RunSheet, 8, 128, 128,
                                boss2AttackSheet, 4, 128, 128,
                                boss2JumpSheet, 7, 128, 128,
                                boss2DamageSheet, 2, 128, 128,
                                boss2DeathSheet, 6, 128, 128,
                                nullptr, 0, 0, 0);
                    levelTransition = false;
                    showLevelComplete = false;
                    bossDeathAnimationStarted = false;
                } else if (currentLevel == 2) {
                    showGameComplete = true;
                    gameCompleteStartTime = SDL_GetTicks();
                    levelTransition = false;
                    showLevelComplete = false;
                    bossDeathAnimationStarted = false;
                    // Phát âm thanh Game Complete
                    if (!gameCompleteSoundPlayed && gui.IsSoundEnabled()) {
                        Mix_PlayChannel(-1, gameOverSound, 0);
                        std::cout << "Game Complete sound played\n";
                        gameCompleteSoundPlayed = true;
                    }
                }
            }

            // Thoát game sau khi hiển thị hoạt ảnh hoàn thành
            if (showGameComplete && SDL_GetTicks() - gameCompleteStartTime >= GAME_COMPLETE_DURATION) {
                quit = true;
            }

            // Render
            SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
            SDL_RenderClear(g_renderer);

            if (showGameOver) {
                SDL_RenderCopy(g_renderer, gameOverTexture, nullptr, nullptr);
                boss.Render(g_renderer, currentLevel == 1 ? boss1Idle : boss2Idle, SDL_GetTicks());
            } else if (showGameComplete) {
                SDL_RenderCopy(g_renderer, gameCompleteTexture, nullptr, nullptr);
            } else if (showLevelComplete) {
                SDL_RenderCopy(g_renderer, levelCompleteTexture, nullptr, nullptr);
            } else {
                // Hiển thị background
                SDL_Texture* currentBackground = currentLevel == 1 ? level1Background : level2Background;
                SDL_RenderCopy(g_renderer, currentBackground, nullptr, nullptr);

                // Hiển thị platforms
                SDL_Rect* currentPlatforms = currentLevel == 1 ? level1Platforms : level2Platforms;
                for (int i = 0; i < 3; ++i) {
                    SDL_RenderCopy(g_renderer, platformTexture, nullptr, &currentPlatforms[i]);
                }

                // Hiển thị sức khỏe của player
                for (int i = 0; i < player.GetHealth(); ++i) {
                    SDL_Rect heartRect = {10 + i * 40, 10, 32, 32};
                    SDL_RenderCopy(g_renderer, heartTexture, nullptr, &heartRect);
                }

                // Render player và boss
                player.Render(g_renderer);
                boss.Render(g_renderer, currentLevel == 1 ? boss1Idle : boss2Idle, SDL_GetTicks());

                // Kiểm tra trạng thái chết của player để hiển thị Game Over
                if (player.IsDead() && SDL_GetTicks() - bossDeathStartTime >= DEATH_FRAME_COUNT * FRAME_DELAY) {
                    showGameOver = true;
                    // Phát âm thanh Game Over
                    if (!gameOverSoundPlayed && gui.IsSoundEnabled()) {
                        Mix_PlayChannel(-1, gameOverSound, 0);
                        std::cout << "Game Over sound played\n";
                        gameOverSoundPlayed = true;
                    }
                }
            }

            SDL_RenderPresent(g_renderer);
            SDL_Delay(16);
        }
    }

    CleanUp(playerIdle, runSheet, attackSheet, jumpSheet, damageSheet, deathSheet, font, backgroundMusic, gameOverSound, attackSound);
    return 0;
}
