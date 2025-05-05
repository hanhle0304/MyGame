#include "gui.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>

// Hàm hành động cho các nút
bool StartAction(GameState& state, bool& soundEnabled, Mix_Music* music, Mix_Chunk* gameOverSound, Mix_Chunk* attackSound) {
    state = GameState::PLAYING;
    if (soundEnabled && Mix_PausedMusic()) {
        Mix_ResumeMusic();
        std::cout << "Background music resumed\n";
    }
    std::cout << "Starting game\n";
    return true;
}

bool OptionAction(GameState& state, bool& soundEnabled, Mix_Music* music, Mix_Chunk* gameOverSound, Mix_Chunk* attackSound) {
    soundEnabled = !soundEnabled;
    if (soundEnabled) {
        Mix_ResumeMusic();
        std::cout << "Background music resumed\n";
    } else {
        Mix_PauseMusic();
        Mix_HaltChannel(-1); // Dừng mọi âm thanh trên tất cả kênh
        std::cout << "Background music paused and sound effects stopped\n";
    }
    std::cout << "Sound toggled: " << (soundEnabled ? "ON" : "OFF") << "\n";
    return false;
}

bool ExitAction(GameState& state, bool& soundEnabled, Mix_Music* music, Mix_Chunk* gameOverSound, Mix_Chunk* attackSound) {
    std::cout << "Exiting game\n";
    SDL_Quit();
    exit(0);
    return false;
}

GUI::GUI(SDL_Renderer* renderer, TTF_Font* font, Mix_Music* music, Mix_Chunk* gameOverSound, Mix_Chunk* attackSound)
    : renderer(renderer), font(font), soundEnabled(true), backgroundMusic(music), gameOverSound(gameOverSound), attackSound(attackSound) {
    // Khởi tạo các nút
    Button startButton = {
        {500, 200, 200, 50},
        "Start",
        {0, 255, 0, 255},
        {100, 255, 100, 255},
        false,
        false,
        StartAction
    };

    Button optionButton = {
        {500, 300, 200, 50},
        "Option",
        {255, 255, 0, 255},
        {255, 255, 100, 255},
        false,
        false,
        OptionAction
    };

    Button exitButton = {
        {500, 400, 200, 50},
        "Exit",
        {255, 0, 0, 255},
        {255, 100, 100, 255},
        false,
        false,
        ExitAction
    };

    buttons.push_back(startButton);
    buttons.push_back(optionButton);
    buttons.push_back(exitButton);
}

GUI::~GUI() {
    // Font, music, gameOverSound, attackSound được giải phóng trong main.cpp
}

void GUI::Update(SDL_Event& e, GameState& state) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    for (auto& button : buttons) {
        button.isHovered = (mouseX >= button.rect.x && mouseX <= button.rect.x + button.rect.w &&
                            mouseY >= button.rect.y && mouseY <= button.rect.y + button.rect.h);

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT && button.isHovered) {
            button.isPressed = true;
        }
        if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && button.isPressed && button.isHovered) {
            button.isPressed = false;
            button.action(state, soundEnabled, backgroundMusic, gameOverSound, attackSound);
        }
    }
}

void GUI::RenderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << "\n";
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << "\n";
        SDL_FreeSurface(surface);
        return;
    }
    int textW, textH;
    SDL_QueryTexture(texture, nullptr, nullptr, &textW, &textH);
    SDL_Rect dstRect = {x - textW / 2, y - textH / 2, textW, textH};
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void GUI::Render() {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderClear(renderer);

    for (const auto& button : buttons) {
        SDL_Color color = button.isHovered ? button.hoverColor : button.normalColor;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &button.rect);
        RenderText(button.text, button.rect.x + button.rect.w / 2, button.rect.y + button.rect.h / 2, {255, 255, 255, 255});
    }

    std::string soundText = "Sound: " + std::string(soundEnabled ? "ON" : "OFF");
    RenderText(soundText, 600, 500, {255, 255, 255, 255});

    SDL_RenderPresent(renderer);
}
