#ifndef GUI_H
#define GUI_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string>
#include <vector>

enum class GameState {
    MENU,
    PLAYING
};

struct Button {
    SDL_Rect rect;
    std::string text;
    SDL_Color normalColor;
    SDL_Color hoverColor;
    bool isHovered;
    bool isPressed;
    bool (*action)(GameState& state, bool& soundEnabled, Mix_Music* music, Mix_Chunk* gameOverSound, Mix_Chunk* attackSound);
};

class GUI {
private:
    TTF_Font* font;
    std::vector<Button> buttons;
    bool soundEnabled;
    SDL_Renderer* renderer;
    Mix_Music* backgroundMusic;
    Mix_Chunk* gameOverSound;
    Mix_Chunk* attackSound; // Thêm âm thanh tấn công

    void RenderText(const std::string& text, int x, int y, SDL_Color color);

public:
    GUI(SDL_Renderer* renderer, TTF_Font* font, Mix_Music* music, Mix_Chunk* gameOverSound, Mix_Chunk* attackSound);
    ~GUI();
    void Update(SDL_Event& e, GameState& state);
    void Render();
    bool IsSoundEnabled() const { return soundEnabled; }
};

#endif
