#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <ctime>
#include <sstream>

using namespace std;


const int GRID_SIZE = 9;
const int GAME_AREA_SIZE = 630;
const int UI_AREA_HEIGHT = 30;
const int SCREEN_WIDTH = GAME_AREA_SIZE;
const int SCREEN_HEIGHT = GAME_AREA_SIZE + UI_AREA_HEIGHT;
const int CELL_SIZE = GAME_AREA_SIZE / GRID_SIZE;
const int GAME_AREA_Y_OFFSET = UI_AREA_HEIGHT;

const int LINE_WIDTH = 2;
const int THICK_LINE_WIDTH = 4;
const int GAME_DURATION = 1800;


const SDL_Color WHITE = {255, 255, 255, 255};
const SDL_Color BLACK = {0, 0, 0, 255};
const SDL_Color GRAY = {220, 220, 220, 255};
const SDL_Color HIGHLIGHTED = {180, 210, 255, 180};
const SDL_Color NUMBER_COLOR = {0, 0, 150, 255};
const SDL_Color ORIGINAL_NUMBER_COLOR = {0, 0, 0, 255};
const SDL_Color MENU_TEXT_COLOR = {0, 0, 255, 255};
const SDL_Color TIMER_COLOR = {255, 0, 0, 255};
const SDL_Color TRIES_COLOR = {0, 100, 0, 255};

int selectedRow = -1;
int selectedCol = -1;


TTF_Font* gFont = nullptr;
TTF_Font* gFontSmall = nullptr;


Mix_Chunk* gSoundCorrect = nullptr;
Mix_Chunk* gSoundWrong = nullptr;


Mix_Music* gBackgroundMusic = nullptr;

SDL_Texture* backgroundTexture = nullptr;


enum GameState {
    MENU,
    RUNNING,
    PAUSED,
    GAME_OVER,
    WIN
};
GameState gameState = MENU;


enum PauseMenuSelection {
    RESUME,
    RESTART,
    QUIT
};

PauseMenuSelection currentSelection = RESUME;


bool isSafe(const vector<vector<int>>& board, int row, int col, int num);
bool solveSudoku(vector<vector<int>>& board);
void buildSudoku(vector<vector<int>>& board);
bool showMenu(SDL_Renderer* renderer);
void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int width);
void drawRectangle(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color);
void drawNumber(SDL_Renderer* renderer, int row, int col, int number, bool isOriginal);
void renderPauseScreen(SDL_Renderer* renderer);
void renderGameOverScreen(SDL_Renderer* renderer, const string& message);
void renderWinScreen(SDL_Renderer* renderer);
void drawTimer(SDL_Renderer* renderer, int timeLeft);
void drawTries(SDL_Renderer* renderer, int triesLeft);
SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer);
Mix_Chunk* loadSound(const string& path);
Mix_Music* loadMusic(const string& path); // Added prototype
void closeSDL(SDL_Window* window, SDL_Renderer* renderer);



bool isSafe(const vector<vector<int>>& board, int row, int col, int num) {

    for (int x = 0; x < GRID_SIZE; x++) {
        if (board[row][x] == num || board[x][col] == num)
            return false;
    }


    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = startRow; i < startRow + 3; i++) {
        for (int j = startCol; j < startCol + 3; j++) {

            if (board[i][j] == num)
                return false;
        }
    }

    return true;
}

bool solveSudoku(vector<vector<int>>& board) {
    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            if (board[row][col] == 0) {
                 vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9};
                 shuffle(numbers.begin(), numbers.end(), mt19937{random_device{}()});
                 for (int number : numbers) {
                    if (isSafe(board, row, col, number)) {
                        board[row][col] = number;
                        if (solveSudoku(board)) {
                            return true;
                        } else {
                            board[row][col] = 0;
                        }
                    }
                }
                return false;
            }
        }
    }
    return true;
}

void buildSudoku(vector<vector<int>>& board) {
    board.assign(GRID_SIZE, vector<int>(GRID_SIZE, 0));


    solveSudoku(board);
}

bool showMenu(SDL_Renderer* renderer) {
    SDL_Event event;
    bool inMenu = true;

    while (inMenu) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    return true;
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderClear(renderer);


        SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, "SUDOKU", MENU_TEXT_COLOR);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {(SCREEN_WIDTH - textSurface->w) / 2, SCREEN_HEIGHT / 4, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        textSurface = TTF_RenderText_Solid(gFontSmall, "Nhan ENTER de bat dau", BLACK);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textRect = {(SCREEN_WIDTH - textSurface->w) / 2, SCREEN_HEIGHT / 2, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

         textSurface = TTF_RenderText_Solid(gFontSmall, "Nhan ESC de thoat", BLACK);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textRect = {(SCREEN_WIDTH - textSurface->w) / 2, SCREEN_HEIGHT / 2 + 40, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);


        SDL_RenderPresent(renderer);
    }
    return false;
}

void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int width) {
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);


    int dx = (x1 == x2) ? 1 : 0;
    int dy = (y1 == y2) ? 1 : 0;

    for (int i = -width / 2; i < (width + 1) / 2; ++i) {
        SDL_RenderDrawLine(renderer, x1 + i * dx, y1 + i * dy, x2 + i * dx, y2 + i * dy);
    }
}

void drawRectangle(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void drawNumber(SDL_Renderer* renderer, int row, int col, int number, bool isOriginal) {
    if (number == 0) return;

    string text = to_string(number);

    SDL_Color textColor = isOriginal ? ORIGINAL_NUMBER_COLOR : NUMBER_COLOR;

    if (gFont == nullptr) {
        cerr << "Font chua duoc tai!" << endl;
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, text.c_str(), textColor);
    if (textSurface == nullptr) {
        cerr << "Khong the render text surface! SDL_ttf Error: " << TTF_GetError() << endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        cerr << "Khong thể tao texture tu text! SDL Error: " << SDL_GetError() << endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    int x = col * CELL_SIZE + (CELL_SIZE - textWidth) / 2;
    int y = row * CELL_SIZE + (CELL_SIZE - textHeight) / 2 + GAME_AREA_Y_OFFSET;

    SDL_Rect renderQuad = {x, y, textWidth, textHeight};
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderPauseScreen(SDL_Renderer* renderer) {

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect overlayRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlayRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);


    SDL_Color textColor;
    SDL_Surface* textSurface;
    SDL_Texture* textTexture;
    SDL_Rect textRect;
    int startY = SCREEN_HEIGHT / 3;
    int spacing = 60;


    textColor = (currentSelection == RESUME) ? HIGHLIGHTED : WHITE;
    textSurface = TTF_RenderText_Solid(gFont, "Tiep tuc (P)", textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textRect = {(SCREEN_WIDTH - textSurface->w) / 2, startY, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);


    textColor = (currentSelection == RESTART) ? HIGHLIGHTED : WHITE;
    textSurface = TTF_RenderText_Solid(gFont, "Choi lai (R)", textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textRect = {(SCREEN_WIDTH - textSurface->w) / 2, startY + spacing, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);


    textColor = (currentSelection == QUIT) ? HIGHLIGHTED : WHITE;
    textSurface = TTF_RenderText_Solid(gFont, "Thoat (Q)", textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textRect = {(SCREEN_WIDTH - textSurface->w) / 2, startY + 2 * spacing, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderGameOverScreen(SDL_Renderer* renderer, const string& message) {

    SDL_SetRenderDrawColor(renderer, 150, 0, 0, 180);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect overlayRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlayRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_Color textColor = WHITE;
    SDL_Surface* textSurfaceGameOver = TTF_RenderText_Solid(gFont, message.c_str(), textColor);
    SDL_Texture* textTextureGameOver = SDL_CreateTextureFromSurface(renderer, textSurfaceGameOver);

    SDL_Surface* textSurfaceOptions = TTF_RenderText_Solid(gFontSmall, "Nhan 'R' de Choi Lai hoac 'Q' de Thoat", textColor);
    SDL_Texture* textTextureOptions = SDL_CreateTextureFromSurface(renderer, textSurfaceOptions);

    SDL_Rect textRectGameOver;
    textRectGameOver.w = textSurfaceGameOver->w;
    textRectGameOver.h = textSurfaceGameOver->h;
    textRectGameOver.x = (SCREEN_WIDTH - textRectGameOver.w) / 2;
    textRectGameOver.y = SCREEN_HEIGHT / 3;

    SDL_Rect textRectOptions;
    textRectOptions.w = textSurfaceOptions->w;
    textRectOptions.h = textSurfaceOptions->h;
    textRectOptions.x = (SCREEN_WIDTH - textRectOptions.w) / 2;
    textRectOptions.y = SCREEN_HEIGHT * 2 / 3;


    SDL_RenderCopy(renderer, textTextureGameOver, NULL, &textRectGameOver);
    SDL_RenderCopy(renderer, textTextureOptions, NULL, &textRectOptions);

    SDL_FreeSurface(textSurfaceGameOver);
    SDL_DestroyTexture(textTextureGameOver);
    SDL_FreeSurface(textSurfaceOptions);
    SDL_DestroyTexture(textTextureOptions);
}

void renderWinScreen(SDL_Renderer* renderer) {

    SDL_SetRenderDrawColor(renderer, 0, 150, 0, 180);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect overlayRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlayRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_Color textColor = WHITE;
    SDL_Surface* textSurfaceWin = TTF_RenderText_Solid(gFont, "CHIEN THANG!", textColor);
    SDL_Texture* textTextureWin = SDL_CreateTextureFromSurface(renderer, textSurfaceWin);

    SDL_Surface* textSurfaceOptions = TTF_RenderText_Solid(gFontSmall, "Nhan 'R' de Choi Lai hoac 'Q' de Thoat", textColor);
    SDL_Texture* textTextureOptions = SDL_CreateTextureFromSurface(renderer, textSurfaceOptions);

    SDL_Rect textRectWin;
    textRectWin.w = textSurfaceWin->w;
    textRectWin.h = textSurfaceWin->h;
    textRectWin.x = (SCREEN_WIDTH - textRectWin.w) / 2;
    textRectWin.y = SCREEN_HEIGHT / 3;

    SDL_Rect textRectOptions;
    textRectOptions.w = textSurfaceOptions->w;
    textRectOptions.h = textSurfaceOptions->h;
    textRectOptions.x = (SCREEN_WIDTH - textRectOptions.w) / 2;
    textRectOptions.y = SCREEN_HEIGHT * 2 / 3;


    SDL_RenderCopy(renderer, textTextureWin, NULL, &textRectWin);
    SDL_RenderCopy(renderer, textTextureOptions, NULL, &textRectOptions);

    SDL_FreeSurface(textSurfaceWin);
    SDL_DestroyTexture(textTextureWin);
    SDL_FreeSurface(textSurfaceOptions);
    SDL_DestroyTexture(textTextureOptions);
}

void drawTimer(SDL_Renderer* renderer, int timeLeft) {
    int minutes = timeLeft / 60;
    int seconds = timeLeft % 60;

    stringstream timeString;

    timeString << "Time: " << (minutes < 10 ? "0" : "") << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;

    if (gFontSmall == nullptr) return;

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFontSmall, timeString.str().c_str(), TIMER_COLOR);
    if (!textSurface) return;
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect;
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.x = 15;
    textRect.y = (UI_AREA_HEIGHT - textRect.h) / 2;

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void drawTries(SDL_Renderer* renderer, int triesLeft) {
    stringstream triesString;
    triesString << "Loi sai con lai: " << triesLeft;

    if (gFontSmall == nullptr) return;

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFontSmall, triesString.str().c_str(), TRIES_COLOR);
     if (!textSurface) return;
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
     if (!textTexture) {
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect;
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.x = SCREEN_WIDTH - textRect.w - 15;
    textRect.y = (UI_AREA_HEIGHT - textRect.h) / 2;

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) {
        cerr << "Khong the tai hinh anh " << path << "! SDL_image Error: " << IMG_GetError() << endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (texture == nullptr) {
        cerr << "Khong the tao texture tu " << path << "! SDL Error: " << SDL_GetError() << endl;
    }

    SDL_FreeSurface(loadedSurface);
    return texture;
}

Mix_Chunk* loadSound(const string& path) {
    Mix_Chunk* sound = Mix_LoadWAV(path.c_str());
    if (sound == nullptr) {
        cerr << "Khong tai duoc am thanh! SDL_mixer Error: " << Mix_GetError() << endl;
    }
    return sound;
}


Mix_Music* loadMusic(const string& path) {
    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (music == nullptr) {
        cerr << "Khong tai duoc nhac nen! SDL_mixer Error: " << Mix_GetError() << endl;

    }
    return music;
}

void closeSDL(SDL_Window* window, SDL_Renderer* renderer) {

    SDL_DestroyTexture(backgroundTexture);
    TTF_CloseFont(gFont);
    gFont = nullptr;
    TTF_CloseFont(gFontSmall);
    gFontSmall = nullptr;
    Mix_FreeChunk(gSoundCorrect);
    gSoundCorrect = nullptr;
    Mix_FreeChunk(gSoundWrong);
    gSoundWrong = nullptr;


    Mix_FreeMusic(gBackgroundMusic);
    gBackgroundMusic = nullptr;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = nullptr;
    window = nullptr;

    Mix_Quit();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}


int main(int argc, char* argv[]) {
    srand(time(0));


    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        cerr << "SDL khong the khoi tao! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        cerr << "SDL_ttf khong the khoi tao! SDL_ttf Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cerr << "SDL_mixer khong the khoi tao! SDL_mixer Error: " << Mix_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        cerr << "SDL_image khong the khoi tao! SDL_image Error: " << IMG_GetError() << endl;
        Mix_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }


    SDL_Window* window = SDL_CreateWindow("Sudoku", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        cerr << "Cua so khong the tao! SDL_Error: " << SDL_GetError() << endl;
        closeSDL(nullptr, nullptr);
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        cerr << "Renderer khong the tao! SDL_Error: " << SDL_GetError() << endl;
        closeSDL(window, nullptr);
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);


    gFont = TTF_OpenFont("C:\\Windows\\Fonts\\Arialbd.ttf", 36);
    gFontSmall = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 18);
    if (gFont == nullptr || gFontSmall == nullptr) {
        cerr << "Khong tai duoc font! SDL_ttf Error: " << TTF_GetError() << endl;
        closeSDL(window, renderer);
        return 1;
    }


    gSoundCorrect = loadSound("C:\\Users\\ADMIN\\Documents\\DemoSDl\\SUDOKUfianl\\sudoku\\bin\\moving.mp3");
    gSoundWrong = loadSound("C:\\Users\\ADMIN\\Documents\\DemoSDl\\SUDOKUfianl\\sudoku\\bin\\moving.mp3");


    gBackgroundMusic = loadMusic("background.mp3");


    backgroundTexture = loadTexture("C:\\Users\\ADMIN\\Documents\\DemoSDl\\SUDOKUfianl\\sudoku\\bin\\Debug\\bikiniBottom.jpg", renderer);


    int triesLeft = 5;
    vector<vector<int>> sudokuSolution(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    vector<vector<int>> sudokuGrid(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    vector<vector<bool>> isOriginal(GRID_SIZE, vector<bool>(GRID_SIZE, false));
    int difficulty = 40;
    int timeLeft = GAME_DURATION;
    Uint32 startTime = 0;
    Uint32 pauseStartTime = 0;
    Uint32 totalPausedTime = 0;


    auto resetGame = [&]() {
        buildSudoku(sudokuSolution);
        sudokuGrid = sudokuSolution;
        isOriginal.assign(GRID_SIZE, vector<bool>(GRID_SIZE, true));

        int holesToMake = GRID_SIZE * GRID_SIZE - difficulty;
        if (holesToMake < 10) holesToMake = 10;
        if (holesToMake > 60) holesToMake = 60;

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distrib(0, GRID_SIZE * GRID_SIZE - 1);

        vector<bool> cellRemoved(GRID_SIZE * GRID_SIZE, false);
        int holesMade = 0;


        while(holesMade < holesToMake) {
             int cellIndex = distrib(gen);
             if (!cellRemoved[cellIndex]) {
                 int row = cellIndex / GRID_SIZE;
                 int col = cellIndex % GRID_SIZE;
                 sudokuGrid[row][col] = 0;
                 isOriginal[row][col] = false;
                 cellRemoved[cellIndex] = true;
                 holesMade++;
             }
        }


        triesLeft = 5;
        timeLeft = GAME_DURATION;
        gameState = RUNNING;
        selectedRow = -1;
        selectedCol = -1;
        startTime = SDL_GetTicks();
        totalPausedTime = 0;


        if (gBackgroundMusic != nullptr) {

            if (Mix_PlayMusic(gBackgroundMusic, -1) == -1) {
                 cerr << "Mix_PlayMusic failed: " << Mix_GetError() << endl;

            }
        }
    };


    if (!showMenu(renderer)) {
        closeSDL(window, renderer);
        return 0;
    } else {
        resetGame();

    }


    bool quit = false;
    SDL_Event event;

    while (!quit) {

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
                Mix_HaltMusic();
            }

            switch (gameState) {
                case MENU:

                    break;

                case RUNNING:
                    if (event.type == SDL_KEYDOWN) {
                        switch (event.key.keysym.sym) {
                            case SDLK_ESCAPE:
                            case SDLK_p:
                                gameState = PAUSED;
                                pauseStartTime = SDL_GetTicks();
                                currentSelection = RESUME;
                                Mix_PauseMusic();
                                break;
                            case SDLK_UP:
                                if (selectedRow > 0) selectedRow--;
                                else selectedRow =GRID_SIZE-1;
                                break;
                            case SDLK_DOWN:
                                if (selectedRow < GRID_SIZE - 1) selectedRow++;
                                else(selectedRow=0);
                                break;
                            case SDLK_LEFT:
                                if (selectedCol > 0) selectedCol--; else selectedCol = GRID_SIZE - 1;
                                break;
                            case SDLK_RIGHT:
                                if (selectedCol < GRID_SIZE - 1) selectedCol++; else selectedCol = 0;
                                break;
                            case SDLK_1: case SDLK_KP_1:
                            case SDLK_2: case SDLK_KP_2:
                            case SDLK_3: case SDLK_KP_3:
                            case SDLK_4: case SDLK_KP_4:
                            case SDLK_5: case SDLK_KP_5:
                            case SDLK_6: case SDLK_KP_6:
                            case SDLK_7: case SDLK_KP_7:
                            case SDLK_8: case SDLK_KP_8:
                            case SDLK_9: case SDLK_KP_9:
                            {
                                int number = (event.key.keysym.sym >= SDLK_KP_1 && event.key.keysym.sym <= SDLK_KP_9)
                                             ? event.key.keysym.sym - SDLK_KP_1 + 1
                                             : event.key.keysym.sym - SDLK_0;

                                if (selectedRow != -1 && selectedCol != -1 && !isOriginal[selectedRow][selectedCol]) {
                                    if (sudokuSolution[selectedRow][selectedCol] == number) {
                                        sudokuGrid[selectedRow][selectedCol] = number;
                                          Mix_PlayChannel(-1, gSoundCorrect, 0);
                                    } else {

                                        if (sudokuGrid[selectedRow][selectedCol] != number) {
                                             triesLeft--;
                                              Mix_PlayChannel(-1, gSoundWrong, 0);

                                        }
                                    }
                                }
                                break;
                            }
                            case SDLK_BACKSPACE:
                            case SDLK_DELETE:
                            case SDLK_0:
                            case SDLK_KP_0:
                                if (selectedRow != -1 && selectedCol != -1 && !isOriginal[selectedRow][selectedCol]) {
                                    sudokuGrid[selectedRow][selectedCol] = 0;
                                }
                                break;
                        }
                    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            int mouseX = event.button.x;
                            int mouseY = event.button.y;

                            if (mouseY >= GAME_AREA_Y_OFFSET && mouseY < SCREEN_HEIGHT) {
                                selectedCol = mouseX / CELL_SIZE;

                                selectedRow = (mouseY - GAME_AREA_Y_OFFSET) / CELL_SIZE;


                                if (selectedCol >= GRID_SIZE) selectedCol = GRID_SIZE - 1;
                                if (selectedRow >= GRID_SIZE) selectedRow = GRID_SIZE - 1;
                            } else {



                            }
                        }
                    }
                    break;

                case PAUSED:
                    if (event.type == SDL_KEYDOWN) {
                        switch (event.key.keysym.sym) {
                            case SDLK_UP:
                                currentSelection = (PauseMenuSelection)((currentSelection - 1 + 3) % 3);
                                break;
                            case SDLK_DOWN:
                                currentSelection = (PauseMenuSelection)((currentSelection + 1) % 3);
                                break;
                            case SDLK_RETURN: case SDLK_KP_ENTER:
                                switch (currentSelection) {
                                    case RESUME:
                                        gameState = RUNNING;

                                        totalPausedTime += (SDL_GetTicks() - pauseStartTime);
                                        Mix_ResumeMusic();
                                        break;
                                    case RESTART:
                                        resetGame();
                                        break;
                                    case QUIT:
                                        Mix_HaltMusic();
                                        quit = true;
                                        break;
                                }
                                break;
                            case SDLK_p:
                            case SDLK_ESCAPE:
                                gameState = RUNNING;
                                totalPausedTime += (SDL_GetTicks() - pauseStartTime);
                                Mix_ResumeMusic();
                                break;
                            case SDLK_r:
                                resetGame();
                                break;
                             case SDLK_q:
                                Mix_HaltMusic();
                                quit = true;
                                break;
                        }
                    }
                     else if (event.type == SDL_MOUSEBUTTONDOWN) {
                          int mouseX = event.button.x;
                          int mouseY = event.button.y;
                          int startY = SCREEN_HEIGHT / 3;
                          int spacing = 60;
                          int textHeight = TTF_FontHeight(gFont);

                          SDL_Rect resumeRect = {(SCREEN_WIDTH - 200) / 2, startY, 200, textHeight};
                          SDL_Rect restartRect = {(SCREEN_WIDTH - 200) / 2, startY + spacing, 200, textHeight};
                          SDL_Rect quitRect = {(SCREEN_WIDTH - 200) / 2, startY + 2 * spacing, 200, textHeight};

                          SDL_Point mousePoint = {mouseX, mouseY};

                          if (SDL_PointInRect(&mousePoint, &resumeRect)) {
                                gameState = RUNNING;
                                totalPausedTime += (SDL_GetTicks() - pauseStartTime);
                                Mix_ResumeMusic();
                          } else if (SDL_PointInRect(&mousePoint, &restartRect)) {
                                resetGame();
                          } else if (SDL_PointInRect(&mousePoint, &quitRect)) {
                                Mix_HaltMusic();
                                quit = true;
                          }
                     }
                    break;

                case GAME_OVER:
                case WIN:

                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_r) {
                            resetGame();
                        } else if (event.key.keysym.sym == SDLK_q || event.key.keysym.sym == SDLK_ESCAPE) {

                            quit = true;
                        }
                    }
                     else if (event.type == SDL_MOUSEBUTTONDOWN) {
                         resetGame();
                     }
                    break;
            }
        }

        if (gameState == RUNNING) {

            Uint32 currentTime = SDL_GetTicks();
            int elapsedSeconds = (currentTime - startTime - totalPausedTime) / 1000;
            timeLeft = GAME_DURATION - elapsedSeconds;

            if (timeLeft <= 0) {
                timeLeft = 0;
                gameState = GAME_OVER;
                Mix_HaltMusic();
            }

            if (triesLeft <= 0) {
                gameState = GAME_OVER;
                Mix_HaltMusic();
            }
            bool solved = true;
            for(int i = 0; i < GRID_SIZE; ++i) {
                for(int j = 0; j < GRID_SIZE; ++j) {
                    if(sudokuGrid[i][j] == 0) {
                        solved = false;
                        break;
                    }
                }
                if(!solved) break;
            }



            if (solved) {
                gameState = WIN;
                Mix_HaltMusic();
            }
        }
        drawRectangle(renderer, 0, 0, SCREEN_WIDTH, UI_AREA_HEIGHT, GRAY);

        if (backgroundTexture) {
             SDL_Rect destRect = {0, GAME_AREA_Y_OFFSET, GAME_AREA_SIZE, GAME_AREA_SIZE};
             SDL_RenderCopy(renderer, backgroundTexture, NULL, &destRect);
        } else {

             drawRectangle(renderer, 0, GAME_AREA_Y_OFFSET, GAME_AREA_SIZE, GAME_AREA_SIZE, WHITE);
        }
        drawTimer(renderer, (timeLeft > 0) ? timeLeft : 0);
        drawTries(renderer, (triesLeft > 0) ? triesLeft : 0);
        if (gameState == RUNNING || gameState == PAUSED) {

            if (selectedRow != -1 && selectedCol != -1) {

                drawRectangle(renderer, selectedCol * CELL_SIZE, selectedRow * CELL_SIZE + GAME_AREA_Y_OFFSET,
                              CELL_SIZE, CELL_SIZE, HIGHLIGHTED);
            }
            for (int i = 0; i <= GRID_SIZE; ++i) {
                int lineY = i * CELL_SIZE + GAME_AREA_Y_OFFSET;
                int lineWidth = (i % 3 == 0) ? THICK_LINE_WIDTH : LINE_WIDTH;
                drawLine(renderer, 0, lineY, GAME_AREA_SIZE, lineY, lineWidth);

                int lineX = i * CELL_SIZE;

                drawLine(renderer, lineX, GAME_AREA_Y_OFFSET, lineX, GAME_AREA_Y_OFFSET + GAME_AREA_SIZE, lineWidth);
            }
            for (int row = 0; row < GRID_SIZE; ++row) {
                for (int col = 0; col < GRID_SIZE; ++col) {
                    drawNumber(renderer, row, col, sudokuGrid[row][col], isOriginal[row][col]);
                }
            }
        }
        if (gameState == PAUSED) {
            renderPauseScreen(renderer);
        } else if (gameState == GAME_OVER) {
            renderGameOverScreen(renderer, triesLeft <= 0 ? "HET LUOT THU!" : "HET GIO!");
        } else if (gameState == WIN) {
             renderWinScreen(renderer);
        }
        SDL_RenderPresent(renderer);
    }
    closeSDL(window, renderer);
    return 0;
}
