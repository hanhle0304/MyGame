#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;
int PLAYER_SPEED = 5;

const int RUN_FRAME_COUNT = 4;
const int ATTACK_FRAME_COUNT = 4;
const int JUMP_FRAME_COUNT = 12;
const int BLOCK_FRAME_COUNT = 1;  // Số lượng khung hình block chỉ còn 1

SDL_Texture* runFrames[RUN_FRAME_COUNT];
SDL_Texture* attackFrames[ATTACK_FRAME_COUNT];
SDL_Texture* jumpFrames[JUMP_FRAME_COUNT];
SDL_Texture* blockFrames[BLOCK_FRAME_COUNT];  // Mảng khung hình block, chỉ còn 1 frame
SDL_Texture* platformTexture = nullptr;

int currentRunFrame = 0;
int currentAttackFrame = 0;
int currentJumpFrame = 0;
int currentBlockFrame = 0;  // Chỉ số khung hình block

Uint32 lastFrameTime = 0;
const Uint32 FRAME_DELAY = 100;

SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
SDL_Texture* g_background = nullptr;
SDL_Texture* g_playerIdle = nullptr;

const int PLATFORM_COUNT = 3;
SDL_Rect platforms[PLATFORM_COUNT] = {
    {300, 350, 200, 20},
    {600, 300, 150, 20},
    {900, 400, 180, 20}
};

bool isJumping = false;
bool isDoubleJumping = false; // Biến kiểm tra nhảy kép
bool isAttacking = false;
bool isBlocking = false;  // Biến kiểm tra xem nhân vật có đang block hay không
bool facingRight = true;

int verticalVelocity = 0;
const int GRAVITY = 1;
const int JUMP_STRENGTH = -20;
bool isOnGround = false;

bool Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL Init failed! " << SDL_GetError() << std::endl;
        return false;
    }

    g_window = SDL_CreateWindow("SDL2 Jump & Platforms",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                SCREEN_WIDTH,
                                SCREEN_HEIGHT,
                                SDL_WINDOW_SHOWN);
    if (!g_window)
    {
        std::cout << "Window creation failed! " << SDL_GetError() << std::endl;
        return false;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer)
    {
        std::cout << "Renderer creation failed! " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        std::cout << "SDL_image init failed! " << IMG_GetError() << std::endl;
        return false;
    }

    return true;
}

SDL_Texture* LoadTexture(const std::string& path)
{
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface)
    {
        std::cout << "Failed to load: " << path << " | " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

bool LoadFrames()
{
    for (int i = 0; i < RUN_FRAME_COUNT; ++i)
    {
        runFrames[i] = LoadTexture("run" + std::to_string(i + 1) + ".png");
        if (!runFrames[i]) return false;
    }

    for (int i = 0; i < ATTACK_FRAME_COUNT; ++i)
    {
        attackFrames[i] = LoadTexture("attack" + std::to_string(i + 1) + ".png");
        if (!attackFrames[i]) return false;
    }

    for (int i = 0; i < JUMP_FRAME_COUNT; ++i)
    {
        jumpFrames[i] = LoadTexture("jump" + std::to_string(i + 1) + ".png");
        if (!jumpFrames[i]) return false;
    }

    // Chỉ cần tải một khung hình cho block
    blockFrames[0] = LoadTexture("block1.png");
    if (!blockFrames[0]) return false;

    platformTexture = LoadTexture("platform.png");
    if (!platformTexture) return false;

    return true;
}

void CleanUp()
{
    for (int i = 0; i < RUN_FRAME_COUNT; ++i)
        if (runFrames[i]) SDL_DestroyTexture(runFrames[i]);

    for (int i = 0; i < ATTACK_FRAME_COUNT; ++i)
        if (attackFrames[i]) SDL_DestroyTexture(attackFrames[i]);

    for (int i = 0; i < JUMP_FRAME_COUNT; ++i)
        if (jumpFrames[i]) SDL_DestroyTexture(jumpFrames[i]);

    // Chỉ cần giải phóng một khung hình block
    if (blockFrames[0]) SDL_DestroyTexture(blockFrames[0]);

    if (platformTexture) SDL_DestroyTexture(platformTexture);
    if (g_playerIdle) SDL_DestroyTexture(g_playerIdle);
    if (g_background) SDL_DestroyTexture(g_background);
    if (g_renderer) SDL_DestroyRenderer(g_renderer);
    if (g_window) SDL_DestroyWindow(g_window);
    IMG_Quit();
    SDL_Quit();
}

bool IsOnAnyPlatform(SDL_Rect player)
{
    for (int i = 0; i < PLATFORM_COUNT; ++i)
    {
        SDL_Rect p = platforms[i];
        if (player.y + player.h == p.y &&
            player.x + player.w > p.x &&
            player.x < p.x + p.w)
        {
            return true;
        }
    }
    return false;
}

bool CheckPlatformCollision(SDL_Rect& player)
{
    for (int i = 0; i < PLATFORM_COUNT; ++i)
    {
        SDL_Rect p = platforms[i];

        // Kiểm tra va chạm trong khi rơi
        if (player.y + player.h <= p.y + 10 &&
            player.y + player.h + verticalVelocity >= p.y &&
            player.x + player.w > p.x &&
            player.x < p.x + p.w)
        {
            player.y = p.y - player.h; // Đặt lại vị trí của nhân vật ở đúng trên platform
            verticalVelocity = 0; // Reset vận tốc rơi
            isJumping = false; // Dừng nhảy
            isDoubleJumping = false; // Reset nhảy kép
            isOnGround = true; // Đặt nhân vật đang ở trên mặt đất
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[])
{
    if (!Init()) return -1;

    g_background = LoadTexture("bkground.png");
    if (!g_background) return -1;

    g_playerIdle = LoadTexture("player.png");
    if (!g_playerIdle) return -1;

    if (!LoadFrames()) return -1;

    SDL_Rect playerRect = {100, 400, 100, 100};
    bool isQuit = false;
    SDL_Event e;
    bool moveLeft = false, moveRight = false;

    while (!isQuit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) isQuit = true;
            else if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_a: moveLeft = true; facingRight = false; break;
                    case SDLK_d: moveRight = true; facingRight = true; break;
                    case SDLK_SPACE:
                        if (isOnGround)
                        {
                            isJumping = true;
                            verticalVelocity = JUMP_STRENGTH;
                            isOnGround = false;
                        }
                        else if (!isDoubleJumping)
                        {
                            isDoubleJumping = true;
                            verticalVelocity = JUMP_STRENGTH;
                        }
                        break;
                    case SDLK_j:
                        if (!isAttacking)
                        {
                            isAttacking = true;
                            currentAttackFrame = 0;
                            lastFrameTime = SDL_GetTicks();
                        }
                        break;
                    case SDLK_k:  // Phím K cho block
                        if (!isBlocking)
                        {
                            isBlocking = true;
                            currentBlockFrame = 0;  // Luôn hiển thị block1
                            lastFrameTime = SDL_GetTicks();
                        }
                        break;
                }
            }
            else if (e.type == SDL_KEYUP)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_a: moveLeft = false; break;
                    case SDLK_d: moveRight = false; break;
                    case SDLK_k: isBlocking = false; break;  // Dừng block khi phím K được thả ra
                }
            }
        }

        // Nếu đang block thì không di chuyển
        if (!isBlocking)
        {
            if (moveLeft && playerRect.x > 0) playerRect.x -= PLAYER_SPEED;
            if (moveRight && playerRect.x + playerRect.w < SCREEN_WIDTH) playerRect.x += PLAYER_SPEED;
        }

        // Trọng lực
        verticalVelocity += GRAVITY;
        playerRect.y += verticalVelocity;

        // Kiểm tra va chạm với platform khi nhân vật đang rơi
        isOnGround = CheckPlatformCollision(playerRect);

        // Kiểm tra mặt đất
        if (playerRect.y + playerRect.h >= 500)
        {
            playerRect.y = 500 - playerRect.h;
            verticalVelocity = 0;
            isJumping = false;
            isDoubleJumping = false; // Reset nhảy kép khi tiếp đất
            isOnGround = true;
        }

        // Vẽ màn hình
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_background, nullptr, nullptr);

        for (int i = 0; i < PLATFORM_COUNT; ++i)
        {
            SDL_RenderCopy(g_renderer, platformTexture, nullptr, &platforms[i]);
        }

        SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        Uint32 currentTime = SDL_GetTicks();

        // Vẽ hoạt ảnh block (chỉ 1 frame block1)
        if (isBlocking)
        {
            SDL_RenderCopyEx(g_renderer, blockFrames[0], nullptr, &playerRect, 0, nullptr, flip);
        }
        // Vẽ các hoạt ảnh khác (run, jump, attack)
        else if (isAttacking)
        {
            if (currentTime > lastFrameTime + FRAME_DELAY)
            {
                lastFrameTime = currentTime;
                currentAttackFrame++;
                if (currentAttackFrame >= ATTACK_FRAME_COUNT)
                {
                    isAttacking = false;
                    currentAttackFrame = 0;
                }
            }
            SDL_RenderCopyEx(g_renderer, attackFrames[currentAttackFrame], nullptr, &playerRect, 0, nullptr, flip);
        }
        else if (isJumping || isDoubleJumping)
        {
            if (currentTime > lastFrameTime + FRAME_DELAY)
            {
                lastFrameTime = currentTime;
                currentJumpFrame = (currentJumpFrame + 1) % JUMP_FRAME_COUNT;
            }
            SDL_RenderCopyEx(g_renderer, jumpFrames[currentJumpFrame], nullptr, &playerRect, 0, nullptr, flip);
        }
        else if (moveLeft || moveRight)
        {
            if (currentTime > lastFrameTime + FRAME_DELAY)
            {
                lastFrameTime = currentTime;
                currentRunFrame = (currentRunFrame + 1) % RUN_FRAME_COUNT;
            }
            SDL_RenderCopyEx(g_renderer, runFrames[currentRunFrame], nullptr, &playerRect, 0, nullptr, flip);
        }
        else
        {
            SDL_RenderCopyEx(g_renderer, g_playerIdle, nullptr, &playerRect, 0, nullptr, flip);
        }

        SDL_RenderPresent(g_renderer);
        SDL_Delay(16);
    }

    CleanUp();
    return 0;
}
