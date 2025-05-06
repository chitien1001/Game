#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <deque>
#include <ctime>
#include <iostream>
#include <algorithm>

const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 700;
const int TILE_SIZE = 30;

enum Direction { UP, DOWN, LEFT, RIGHT };
enum GameState { MENU, PLAYING };
struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

class SnakeGame {
public:
    SnakeGame() {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
        IMG_Init(IMG_INIT_PNG);
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

        window = SDL_CreateWindow("Rắn Săn Mồi", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, 0);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        backgroundTex = loadTexture("assets/background.png");
        snakeTex = loadTexture("assets/snake.png");
        foodTex = loadTexture("assets/food.png");
        gameOverTex = loadTexture("assets/gameover.png");
        menuTex = loadTexture("assets/menu.png");

        eatSound = Mix_LoadWAV("assets/eat.wav");
        gameOverSound = Mix_LoadWAV("assets/gameover.wav");
        bgMusic = Mix_LoadMUS("assets/bg_music.mp3");

        if (!eatSound || !gameOverSound || !bgMusic) {
            std::cerr << "Không tải được âm thanh: " << Mix_GetError() << std::endl;
            exit(1);
        }
        Mix_VolumeChunk(eatSound, 80);
        Mix_VolumeChunk(gameOverSound, 100);
        Mix_VolumeMusic(30);

        Mix_PlayMusic(bgMusic, -1);
        reset();
    }

    ~SnakeGame() {
        Mix_FreeChunk(eatSound);
        Mix_FreeChunk(gameOverSound);
        Mix_FreeMusic(bgMusic);
        Mix_CloseAudio();
        SDL_DestroyTexture(backgroundTex);
        SDL_DestroyTexture(snakeTex);
        SDL_DestroyTexture(foodTex);
        SDL_DestroyTexture(gameOverTex);
        SDL_DestroyTexture(menuTex);
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
        if (!Mix_PlayingMusic()) {
        Mix_PlayMusic(bgMusic, -1);
    }
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

                if (state == MENU) {
                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_RETURN) {
                            reset();
                            state = PLAYING;
                        } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                            running = false;
                        }
                    }
                } else if (state == PLAYING) {
                    if (e.type == SDL_KEYDOWN) {
                        switch (e.key.keysym.sym) {
                            case SDLK_UP:    if (direction != DOWN) direction = UP; break;
                            case SDLK_DOWN:  if (direction != UP) direction = DOWN; break;
                            case SDLK_LEFT:  if (direction != RIGHT) direction = LEFT; break;
                            case SDLK_RIGHT: if (direction != LEFT) direction = RIGHT; break;
                            case SDLK_r:     if (gameOver) { reset(); } break;
                            case SDLK_ESCAPE: state = MENU; break;
                        }
                    }
                }
            }

            Uint32 now = SDL_GetTicks();
            if (state == PLAYING && now - lastUpdate > 140) {
                update();
                render();
                lastUpdate = now;
            } else if (state == MENU) {
                renderMenu();
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
            Mix_HaltMusic();
            Mix_PlayChannel(-1, gameOverSound, 0);
            return;
        }

        snake.push_front(head);

        if (head == food) {
            Mix_PlayChannel(-1, eatSound, 0);
            spawnFood();
        } else {
            snake.pop_back();
        }
    }

    void render() {

        SDL_Rect bg_pos {
            0, 0,
            SCREEN_WIDTH, SCREEN_HEIGHT
        };
        SDL_RenderCopy(renderer, backgroundTex, nullptr, nullptr);


        for (const auto& p : snake) {
            SDL_Rect dst = {p.x * TILE_SIZE, p.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_RenderCopy(renderer, snakeTex, nullptr, &dst);
        }


        SDL_Rect foodRect = {food.x * TILE_SIZE, food.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(renderer, foodTex, nullptr, &foodRect);

        if (gameOver) {

            SDL_Rect dst;
            dst.w = 400;
            dst.h = 300;
            dst.x = (SCREEN_WIDTH - dst.w) / 2;
            dst.y = (SCREEN_HEIGHT - dst.h) / 2;
            SDL_RenderCopy(renderer, gameOverTex, nullptr, &dst);
        }

        SDL_RenderPresent(renderer);
    }
    void renderMenu() {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, menuTex, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* backgroundTex = nullptr;
    SDL_Texture* snakeTex = nullptr;
    SDL_Texture* foodTex = nullptr;
    SDL_Texture* gameOverTex = nullptr;
    SDL_Texture* menuTex = nullptr;
    Mix_Chunk* eatSound = nullptr;
    Mix_Chunk* gameOverSound = nullptr;
    Mix_Music* bgMusic = nullptr;
    std::deque<Point> snake;
    Point food;
    Direction direction;
    bool gameOver = false;
    GameState state = MENU;
};

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned>(time(nullptr)));
    SnakeGame game;
    game.run();
    return 0;
}
