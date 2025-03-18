#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <random>
#include<cstdlib>
#include<ctime>
using namespace std;

const int SCREEN_WIDTH = 630;
const int SCREEN_HEIGHT = 630;
const int GRID_SIZE = 9;
const int CELL_SIZE = SCREEN_WIDTH / GRID_SIZE;
const int LINE_WIDTH = 2;
const int THICK_LINE_WIDTH = 4;

// Màu sắc
const SDL_Color WHITE = {255, 255, 255, 255};
const SDL_Color BLACK = {0, 0, 0, 255};
const SDL_Color HIGHLIGHTED = {200, 200, 200, 255};
const SDL_Color NUMBER_COLOR = {0, 0, 0, 255}; // Màu chữ số
const SDL_Color MENU_TEXT_COLOR = {0, 0, 255, 255};


int selectedRow = -1;
int selectedCol = -1;


bool isSafe(vector<vector<int>>& board, int row, int col, int num) {
    for (int x = 0; x < GRID_SIZE; x++) {
        if (board[row][x] == num || board[x][col] == num)
            return false;
    }

    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i + startRow][j + startCol] == num)
                return false;
        }
    }

    return true;
}

bool fillSudoku(vector<vector<int>>& board, int row, int col) {
    if (row == GRID_SIZE - 1 && col == GRID_SIZE)
        return true;
    if (col == GRID_SIZE) {
        row++;
        col = 0;
    }
    if (board[row][col] != 0)
        return fillSudoku(board, row, col + 1);

    vector<int> numbers(GRID_SIZE);
    iota(numbers.begin(), numbers.end(), 1);
    int shuffleBoard=rand()%30+1;
    for(int i=0;i<shuffleBoard;i++){
        shuffle(numbers.begin(), numbers.end(), random_device());
    }
    shuffle(numbers.begin(), numbers.end(), random_device());

    for (int num : numbers) {
        if (isSafe(board, row, col, num)) {
            board[row][col] = num;
            if (fillSudoku(board, row, col + 1))
                return true;
            board[row][col] = 0;
        }
    }

    return false;
}

vector<vector<int>> generateSudoku() {
    vector<vector<int>> board(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    fillSudoku(board, 0, 0);
    return board;
}

// Font
TTF_Font* gFont = nullptr;
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

void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int width) {
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Vẽ một loạt các đường thẳng nhỏ để tạo ra đường kẻ dày
    for (int i = -width / 2; i <= width / 2; ++i) {
        SDL_RenderDrawLine(renderer, x1 + i, y1, x2 + i, y2); // Thay đổi x
        SDL_RenderDrawLine(renderer, x1, y1 + i, x2, y2 + i); // Thay đổi y (cho đường dọc)
    }
}

// Hàm vẽ một hình chữ nhật, được sử dụng để tô sáng ô được chọn
void drawRectangle(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}


void drawNumber(SDL_Renderer* renderer, int row, int col, int number) {

    if (number == 0) return;
    string text = to_string(number);
    SDL_Color textColor = NUMBER_COLOR;

    // Đảm bảo gFont hợp lệ
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

int main(int argc, char* argv[]) {
    srand(time(0));
    cout << "Hello" << endl;
    int TryLeft=3;
    vector<vector<int>> sudokuQuizz=generateSudoku();
    vector<vector<int>> sudokuGrid(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    for(int i=0;i<GRID_SIZE;i++){
        for(int k=0;k<GRID_SIZE;k++){
            sudokuGrid[i][k]=sudokuQuizz[i][k];
        }
    }
    cout << "Goodbye" << endl;

    int hole=30+rand()%21;
    while(hole>0){
        int R=rand()%9;
        int C=rand()%9;
        if(sudokuGrid[R][C]!=0){
            sudokuGrid[R][C]=0;
            hole--;
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    } else {
        cout << "SDL initialized successfully!" << endl;
    }


    if (TTF_Init() == -1) {
        cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return 1;
    } else {
        cout << "SDL_ttf initialized successfully!" << endl;
    }

    SDL_Window* window = SDL_CreateWindow("Sudoku Whiteboard", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    } else {
        cout << "Window created successfully!" << endl;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    } else {
        cout << "Renderer created successfully!" << endl;
    }

    gFont = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 24); // Thay "arial.ttf" bằng đường dẫn đến phông chữ của bạn
    if (gFont == nullptr) {
        cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    } else {
        cout << "Font loaded successfully!" << endl;
    }
    if (!showMenu(renderer)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    // Game loop
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT||TryLeft==0) {
                cout<<"game over"<<endl;
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                // Xử lý các phím mũi tên để di chuyển ô được chọn
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit=true;
                        break;
                    case SDLK_UP:
                        if (selectedRow > 0) {
                            selectedRow--;
                        }else if(selectedRow==0){
                            selectedRow=GRID_SIZE-1;
                        }
                        break;
                    case SDLK_DOWN:
                        if (selectedRow < GRID_SIZE - 1) {
                            selectedRow++;
                        }else if(selectedRow==GRID_SIZE-1) selectedRow=0;
                        break;
                    case SDLK_LEFT:
                        if (selectedCol > 0) {
                            selectedCol--;
                        }else if(selectedCol==0) selectedCol=GRID_SIZE-1;
                        break;
                    case SDLK_RIGHT:
                        if (selectedCol < GRID_SIZE - 1) {
                            selectedCol++;
                        }else if(selectedCol==GRID_SIZE-1) selectedCol=0;
                        break;
                    // Xử lý nhập số (1-9)
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
                            if (selectedRow != -1 && selectedCol != -1 && sudokuGrid[selectedRow][selectedCol] == 0) {
                                if (isSafe(sudokuGrid, selectedRow, selectedCol, number)) {
                                    sudokuGrid[selectedRow][selectedCol] = number;
                                }else{
                                    TryLeft--;
                                    cout<<"so lan thu con lai la:"<<TryLeft<<endl;
                                }
                            }
                            break;
                       }
                    // Xử lý xóa (0)
                }
            }
             else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Tính toán hàng và cột được chọn dựa trên vị trí chuột
                    selectedCol = event.button.x / CELL_SIZE;
                    selectedRow = event.button.y / CELL_SIZE;

                    // In ra hàng và cột đã chọn (chỉ để debug)
                    cout << "Selected Row: " << selectedRow << ", Selected Col: " << selectedCol << endl;
                }
            }
        }


        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderClear(renderer);


        if (selectedRow != -1 && selectedCol != -1) {
            drawRectangle(renderer, selectedCol * CELL_SIZE, selectedRow * CELL_SIZE, CELL_SIZE, CELL_SIZE, HIGHLIGHTED);
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
                drawNumber(renderer, row, col, sudokuGrid[row][col]);
            }
        }

        // Cập nhật màn hình
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Thêm delay
    }

    // Giải phóng tài nguyên
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(gFont); // Close the font
    TTF_Quit();
    SDL_Quit();

    return 0;
}















