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

const int SCREEN_WIDTH = 630;
const int SCREEN_HEIGHT = 630;
const int GRID_SIZE = 9;
const int CELL_SIZE = SCREEN_WIDTH / GRID_SIZE;
const int LINE_WIDTH = 2;
const int THICK_LINE_WIDTH = 4;
const int GAME_DURATION = 1800;

// Màu sắc
const SDL_Color WHITE = {255, 255, 255, 255};
const SDL_Color BLACK = {0, 0, 0, 255};
const SDL_Color HIGHLIGHTED = {200, 200, 200, 255};
const SDL_Color NUMBER_COLOR = {0, 0, 0, 255};
const SDL_Color MENU_TEXT_COLOR = {0, 0, 255, 255};
const SDL_Color TIMER_COLOR = {255, 0, 0, 255};

int selectedRow = -1;
int selectedCol = -1;

// Font
TTF_Font* gFont = nullptr;

// Âm thanh
Mix_Chunk* gSound = nullptr;

//Texture background
SDL_Texture* backgroundTexture = nullptr;

//Trạng thái Game
enum GameState {
    MENU,
    RUNNING,
    PAUSED,
    GAME_OVER
};
GameState gameState = MENU;

// Các lựa chọn trong menu pause
enum PauseMenuSelection {
    RESUME,
    RESTART,
    QUIT
};

PauseMenuSelection currentSelection = RESUME;

// Hàm kiểm tra tính hợp lệ của số trong Sudoku
bool isSafe(vector<vector<int>>& board, int row, int col, int num) {
    // Kiểm tra hàng và cột
    for (int x = 0; x < GRID_SIZE; x++) {
        if (board[row][x] == num || board[x][col] == num)
            return false;
    }

    // Kiểm tra ô 3x3
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = startRow; i < startRow + 3; i++) {
        for (int j = startCol; j < startCol + 3; j++) {
            if (board[i][j] == num) // Bỏ điều kiện thừa (i!=row || j!=col)
                return false;
        }
    }

    return true;
}

// Hàm giải Sudoku (Backtracking)
bool solveSudoku(vector<vector<int>>& board) {
    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            if (board[row][col] == 0) {
                for (int number = 1; number <= GRID_SIZE; number++) {
                    if (isSafe(board, row, col, number)) {
                        board[row][col] = number;

                        if (solveSudoku(board)) {
                            return true; // Đệ quy thành công
                        }
                        else {
                            board[row][col] = 0; // Backtrack
                        }
                    }
                }
                return false; // Không có số nào hợp lệ
            }
        }
    }
    return true; // Đã giải xong
}

// Hàm tạo Sudoku
void buildSudoku(vector<vector<int>>& board) {
    // Khởi tạo bảng với 0
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            board[i][j] = 0;
        }
    }

    // Điền các số một cách ngẫu nhiên và giải Sudoku
    solveSudoku(board);
}

// Hàm hiển thị menu
bool showMenu(SDL_Renderer* renderer) {
    SDL_Event event;
    bool inMenu = true;

    while (inMenu) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    return true;
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderClear(renderer);

        SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, "Nhan ENTER de bat dau", MENU_TEXT_COLOR);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        SDL_RenderPresent(renderer);
    }
    return false;
}

// Hàm vẽ đường thẳng
void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int width) {
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);

    // Không cần thiết phải set blend mode ở đây, chỉ cần set một lần khi khởi tạo renderer
    // SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);


    for (int i = -width / 2; i <= width / 2; ++i) {
        SDL_RenderDrawLine(renderer, x1 + i, y1, x2 + i, y2);
        SDL_RenderDrawLine(renderer, x1, y1 + i, x2, y2 + i);
    }
}

// Hàm vẽ hình chữ nhật
void drawRectangle(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

// Hàm vẽ số
void drawNumber(SDL_Renderer* renderer, int row, int col, int number, bool isOriginal = false) {
    if (number == 0) return;

    string text = to_string(number);
    SDL_Color textColor = isOriginal ? BLACK : NUMBER_COLOR; // Màu khác cho số ban đầu

    if (gFont == nullptr) {
        cerr << "Font chưa được tải!" << endl;
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, text.c_str(), textColor);
    if (textSurface == nullptr) {
        cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    int x = col * CELL_SIZE + (CELL_SIZE - textWidth) / 2;
    int y = row * CELL_SIZE + (CELL_SIZE - textHeight) / 2;

    SDL_Rect renderQuad = {x, y, textWidth, textHeight};
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// Hàm vẽ màn hình pause
void renderPauseScreen(SDL_Renderer* renderer) {
    // Tạo một bề mặt (surface) màu đen mờ
    SDL_Surface* pauseSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
    SDL_FillRect(pauseSurface, NULL, SDL_MapRGBA(pauseSurface->format, 0, 0, 0, 128)); // Đen, alpha = 128 (mờ)

    // Tạo một texture từ bề mặt
    SDL_Texture* pauseTexture = SDL_CreateTextureFromSurface(renderer, pauseSurface);

    // Vẽ texture lên renderer
    SDL_RenderCopy(renderer, pauseTexture, NULL, NULL);

    // Giải phóng bề mặt và texture để tránh rò rỉ bộ nhớ
    SDL_FreeSurface(pauseSurface);
    SDL_DestroyTexture(pauseTexture);

    // Vẽ các lựa chọn
    SDL_Color textColor;
    SDL_Surface* textSurface;
    SDL_Texture* textTexture;
    SDL_Rect textRect;

    // Resume
    textColor = (currentSelection == RESUME) ? HIGHLIGHTED : WHITE;
    textSurface = TTF_RenderText_Solid(gFont, "Resume", textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.x = (SCREEN_WIDTH - textRect.w) / 2;
    textRect.y = SCREEN_HEIGHT / 3;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    // Restart
    textColor = (currentSelection == RESTART) ? HIGHLIGHTED : WHITE;
    textSurface = TTF_RenderText_Solid(gFont, "Restart", textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.x = (SCREEN_WIDTH - textRect.w) / 2;
    textRect.y = SCREEN_HEIGHT / 2;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    // Quit
    textColor = (currentSelection == QUIT) ? HIGHLIGHTED : WHITE;
    textSurface = TTF_RenderText_Solid(gFont, "Quit", textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.x = (SCREEN_WIDTH - textRect.w) / 2;
    textRect.y = SCREEN_HEIGHT * 2 / 3;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// Hàm vẽ màn hình game over
void renderGameOverScreen(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255};
    SDL_Surface* textSurfaceGameOver = TTF_RenderText_Solid(gFont, "GAME OVER!", textColor);
    SDL_Texture* textTextureGameOver = SDL_CreateTextureFromSurface(renderer, textSurfaceGameOver);

    SDL_Surface* textSurfacePlayAgain = TTF_RenderText_Solid(gFont, "Press 'R' to Play Again or 'Q' to Quit", textColor);
    SDL_Texture* textTexturePlayAgain = SDL_CreateTextureFromSurface(renderer, textSurfacePlayAgain);

    SDL_Rect textRectGameOver;
    textRectGameOver.w = textSurfaceGameOver->w;
    textRectGameOver.h = textSurfaceGameOver->h;
    textRectGameOver.x = (SCREEN_WIDTH - textRectGameOver.w) / 2;
    textRectGameOver.y = (SCREEN_HEIGHT - textRectGameOver.h) / 3;

    SDL_Rect textRectPlayAgain;
    textRectPlayAgain.w = textSurfacePlayAgain->w;
    textRectPlayAgain.h = textSurfacePlayAgain->h;
    textRectPlayAgain.x = (SCREEN_WIDTH - textRectPlayAgain.w) / 2;
    textRectPlayAgain.y = (SCREEN_HEIGHT - textRectPlayAgain.h) * 2 / 3;


    SDL_RenderCopy(renderer, textTextureGameOver, NULL, &textRectGameOver);
    SDL_RenderCopy(renderer, textTexturePlayAgain, NULL, &textRectPlayAgain);

    SDL_FreeSurface(textSurfaceGameOver);
    SDL_DestroyTexture(textTextureGameOver);
    SDL_FreeSurface(textSurfacePlayAgain);
    SDL_DestroyTexture(textTexturePlayAgain);
}

// Hàm vẽ timer
void drawTimer(SDL_Renderer* renderer, int timeLeft) {
    int minutes = timeLeft / 60;
    int seconds = timeLeft % 60;

    stringstream timeString;
    timeString << "Time: " << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, timeString.str().c_str(), TIMER_COLOR);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect;
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.x = 10;
    textRect.y = 10;

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// Hàm load texture
SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) {
        cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (texture == nullptr) {
        cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << endl;
    }

    SDL_FreeSurface(loadedSurface);
    return texture;
}

// Hàm load âm thanh
Mix_Chunk* loadSound(const string& path) {
    Mix_Chunk* sound = Mix_LoadWAV(path.c_str());
    if (sound == nullptr) {
        cerr << "Failed to load sound effect! SDL_mixer Error: " << Mix_GetError() << endl;
    }
    return sound;
}

int main(int argc, char* argv[]) {
    srand(time(0));

    int TryLeft = 3;
    vector<vector<int>> sudokuQuizz(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    buildSudoku(sudokuQuizz); // Tạo Sudoku đã giải

    vector<vector<int>> sudokuGrid = sudokuQuizz; // Sao chép để tạo bảng trò chơi
    vector<vector<bool>> isOriginal(GRID_SIZE, vector<bool>(GRID_SIZE, true)); // Đánh dấu các ô ban đầu

    int hole = 40; // Số lượng ô để loại bỏ
    int holeLeft = hole;

    // Tạo các ô trống
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(0, GRID_SIZE - 1);
    for (int i = 0; i < hole; ++i) {
        int row, col;
        do {
            row = distrib(gen);
            col = distrib(gen);
        } while (sudokuGrid[row][col] == 0);  // Đảm bảo xóa các ô có giá trị

        sudokuGrid[row][col] = 0;
        isOriginal[row][col] = false;
        holeLeft--;
    }


    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {  // Thêm SDL_INIT_AUDIO
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;  // Or the appropriate flags for the image types you want to load
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
        Mix_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }


    SDL_Window* window = SDL_CreateWindow("Sudoku Whiteboard", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        Mix_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        Mix_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    //Set màu nền cho renderer
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    gFont = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 24);
    if (gFont == nullptr) {
        cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        Mix_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Game timer
    int timeLeft = GAME_DURATION;
    Uint32 startTime = 0;


    auto resetGame = [&]() {
        buildSudoku(sudokuQuizz);
        sudokuGrid = sudokuQuizz;
        isOriginal.assign(GRID_SIZE, vector<bool>(GRID_SIZE, true));

        holeLeft = hole;

        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int k = 0; k < GRID_SIZE; ++k) {
                isOriginal[i][k] = true;
            }
        }

        for (int i = 0; i < hole; ++i) {
            int row, col;
            do {
                row = distrib(gen);
                col = distrib(gen);
            } while (sudokuGrid[row][col] == 0);

            sudokuGrid[row][col] = 0;
            isOriginal[row][col] = false;
            holeLeft--;
        }
        TryLeft = 3;
        timeLeft = GAME_DURATION;
        gameState = RUNNING;
        selectedRow = -1;
        selectedCol = -1;
        startTime = SDL_GetTicks();
    };

    if (!showMenu(renderer)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    else {
        gameState = RUNNING; // Sau menu thì chuyển sang running
        startTime = SDL_GetTicks(); // Start the timer when the game starts
    }

    // Game loop
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            else if (gameState == GAME_OVER) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_r) {
                        resetGame();
                    }
                    else if (event.key.keysym.sym == SDLK_q) {
                        quit = true;
                    }
                }
            }
            else if (gameState == PAUSED) { // Xử lý menu pause
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:
                            currentSelection = (PauseMenuSelection)((currentSelection - 1 + 3) % 3);
                            break;
                        case SDLK_DOWN:
                            currentSelection = (PauseMenuSelection)((currentSelection + 1) % 3);
                            break;
                        case SDLK_RETURN:
                            switch (currentSelection) {
                                case RESUME:
                                    gameState = RUNNING;
                                    startTime = SDL_GetTicks() - (GAME_DURATION - timeLeft) * 1000; // tiếp tục thời gian
                                    break;
                                case RESTART:
                                    resetGame();
                                    break;
                                case QUIT:
                                    quit = true;
                                    break;
                            }
                            break;
                        case SDLK_ESCAPE:
                            gameState = RUNNING;
                            startTime = SDL_GetTicks() - (GAME_DURATION - timeLeft) * 1000;
                            break;
                    }
                }
            }
            else if (event.type == SDL_KEYDOWN) {

                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_p:
                        if (gameState == RUNNING) {
                            gameState = PAUSED;
                        }
                        break;
                    case SDLK_UP:
                        if (selectedRow > 0) {
                            selectedRow--;
                        }
                        break;
                    case SDLK_DOWN:
                        if (selectedRow < GRID_SIZE - 1) {
                            selectedRow++;
                        }
                        break;
                    case SDLK_LEFT:
                        if (selectedCol > 0) {
                            selectedCol--;
                        }
                        break;
                    case SDLK_RIGHT:
                        if (selectedCol < GRID_SIZE - 1) {
                            selectedCol++;
                        }
                        break;
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                    case SDLK_4:
                    case SDLK_5:
                    case SDLK_6:
                    case SDLK_7:
                    case SDLK_8:
                    case SDLK_9:
                    {
                        int number = event.key.keysym.sym - SDLK_0;
                        if (selectedRow != -1 && selectedCol != -1 && !isOriginal[selectedRow][selectedCol]) {
                            if (isSafe(sudokuGrid, selectedRow, selectedCol, number)) {
                                sudokuGrid[selectedRow][selectedCol] = number;
                                //holeLeft--; Không giảm holeLeft khi điền số
                                Mix_PlayChannel(-1, gSound, 0); // Phát âm thanh
                            }
                            else {
                                TryLeft--;
                            }
                        }
                        break;
                    }

                    case SDLK_BACKSPACE:
                    case SDLK_DELETE:
                        if (selectedRow != -1 && selectedCol != -1 && !isOriginal[selectedRow][selectedCol]) {
                            sudokuGrid[selectedRow][selectedCol] = 0;
                        }
                        break;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    selectedCol = event.button.x / CELL_SIZE;
                    selectedRow = event.button.y / CELL_SIZE;
                }
            }
        }

        // Update game state
        if (gameState == RUNNING) {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 elapsedTime = currentTime - startTime;
            if (elapsedTime >= 1000) {
                timeLeft--;
                startTime = currentTime; // Reset startTime sau mỗi giây
            }

            if (timeLeft <= 0 || TryLeft == 0) {
                gameState = GAME_OVER;
            }

            //Kiểm tra xem người chơi đã giải xong chưa
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
                cout << "Congratulations! You solved the Sudoku!" << endl;
                quit = true;
            }
        }

        //render
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderClear(renderer);

        //Vẽ background
         SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        //Vẽ game
        if (gameState == RUNNING || gameState == PAUSED) { // Vẽ game khi đang RUNNING hoặc PAUSED
            if (selectedRow != -1 && selectedCol != -1) {
                drawRectangle(renderer, selectedCol * CELL_SIZE, selectedRow * CELL_SIZE, CELL_SIZE, CELL_SIZE,
                              HIGHLIGHTED);
            }

            SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);

            for (int i = 0; i <= GRID_SIZE; ++i) {
                int x = i * CELL_SIZE;
                int lineWidth = (i % 3 == 0) ? THICK_LINE_WIDTH : LINE_WIDTH;  // Đường kẻ đậm cho các khối 3x3
                drawLine(renderer, x, 0, x, SCREEN_HEIGHT, lineWidth);
            }

            // Vẽ các đường ngang
            for (int i = 0; i <= GRID_SIZE; ++i) {
                int y = i * CELL_SIZE;
                int lineWidth = (i % 3 == 0) ? THICK_LINE_WIDTH : LINE_WIDTH; // Đường kẻ đậm cho các khối 3x3
                drawLine(renderer, 0, y, SCREEN_WIDTH, y, lineWidth);
            }

            // Vẽ các số trong lưới
            for (int row = 0; row < GRID_SIZE; ++row) {
                for (int col = 0; col < GRID_SIZE; ++col) {
                    drawNumber(renderer, row, col, sudokuGrid[row][col], isOriginal[row][col]);
                }
            }
        }

        //Vẽ timer (luôn vẽ ngoài lưới sudoku)
        drawTimer(renderer, timeLeft);

        //Vẽ pause
        if (gameState == PAUSED) {
            renderPauseScreen(renderer);
        }

        if (gameState == GAME_OVER) {
            renderGameOverScreen(renderer);
        }


        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    // Giải phóng bộ nhớ và đóng SDL
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(gFont);
    Mix_FreeChunk(gSound);
    Mix_Quit();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
