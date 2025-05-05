#include <SDL.h>
#include <SDL_image.h>
#include <deque>
#include <ctime>
#include <iostream>
#include <algorithm>

const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 700;
const int TILE_SIZE = 30;

enum Direction { UP, DOWN, LEFT, RIGHT };

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

class SnakeGame {
public:
    SnakeGame() {
        SDL_Init(SDL_INIT_VIDEO);
        IMG_Init(IMG_INIT_PNG);

        window = SDL_CreateWindow("Rắn Săn Mồi", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, 0);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        // Load textures
        backgroundTex = loadTexture("assets/background.png");
        snakeTex = loadTexture("assets/snake.png");
        foodTex = loadTexture("assets/food.png");
        gameOverTex = loadTexture("assets/gameover.png");

        reset();
    }

    ~SnakeGame() {
        SDL_DestroyTexture(backgroundTex);
        SDL_DestroyTexture(snakeTex);
        SDL_DestroyTexture(foodTex);
        SDL_DestroyTexture(gameOverTex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
    }

    void reset() {
        snake.clear();
        snake.push_back({10, 10});
        direction = RIGHT;
        spawnFood();
        gameOver = false;
    }

    SDL_Texture* loadTexture(const std::string& path) {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            std::cerr << "Không tải được ảnh: " << path << "\n";
            exit(1);
        }
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        return tex;
    }

    void spawnFood() {
        food.x = rand() % (SCREEN_WIDTH / TILE_SIZE);
        food.y = rand() % (SCREEN_HEIGHT / TILE_SIZE);
    }

    void run() {
        Uint32 lastUpdate = SDL_GetTicks();
        bool running = true;
        while (running) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) running = false;
                if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_UP:    if (direction != DOWN) direction = UP; break;
                        case SDLK_DOWN:  if (direction != UP) direction = DOWN; break;
                        case SDLK_LEFT:  if (direction != RIGHT) direction = LEFT; break;
                        case SDLK_RIGHT: if (direction != LEFT) direction = RIGHT; break;
                        case SDLK_r:     if (gameOver) {reset();} break;
                    }
                }
            }

            Uint32 now = SDL_GetTicks();
            if (now - lastUpdate > 140) {
                update();
                render();
                lastUpdate = now;
            }
        }
    }

    void update() {
        if (gameOver) {
            return;
        }

        Point head = snake.front();
        switch (direction) {
            case UP:    head.y--; break;
            case DOWN:  head.y++; break;
            case LEFT:  head.x--; break;
            case RIGHT: head.x++; break;
        }

        if (head.x < 0 || head.y < 0 ||
            head.x >= SCREEN_WIDTH / TILE_SIZE ||
            head.y >= SCREEN_HEIGHT / TILE_SIZE ||
            std::find(snake.begin(), snake.end(), head) != snake.end()) {
            gameOver = true;
            return;
        }

        snake.push_front(head);

        if (head == food) {
            spawnFood();
        } else {
            snake.pop_back();
        }
    }

    void render() {
        // Background
        SDL_Rect bg_pos {
            0, 0,
            SCREEN_WIDTH, SCREEN_HEIGHT
        };
        SDL_RenderCopy(renderer, backgroundTex, nullptr, nullptr);

        // Snake
        for (const auto& p : snake) {
            SDL_Rect dst = {p.x * TILE_SIZE, p.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_RenderCopy(renderer, snakeTex, nullptr, &dst);
        }

        // Food
        SDL_Rect foodRect = {food.x * TILE_SIZE, food.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(renderer, foodTex, nullptr, &foodRect);

        if (gameOver) {
            // In ra texture gameoverTex
            SDL_Rect dst;
            dst.w = 200;  // hoặc width của ảnh
            dst.h = 100;  // hoặc height của ảnh
            dst.x = (SCREEN_WIDTH - dst.w) / 2;
            dst.y = (SCREEN_HEIGHT - dst.h) / 2;
            SDL_RenderCopy(renderer, gameOverTex, nullptr, &dst);
        }

        SDL_RenderPresent(renderer);
    }

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* backgroundTex = nullptr;
    SDL_Texture* snakeTex = nullptr;
    SDL_Texture* foodTex = nullptr;
    SDL_Texture* gameOverTex = nullptr;
    std::deque<Point> snake;
    Point food;
    Direction direction;
    bool gameOver = false;
};

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned>(time(nullptr)));
    SnakeGame game;
    game.run();
    return 0;
}
