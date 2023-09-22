#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>

const int SCREEN_WIDTH = 1100;
const int SCREEN_HEIGHT = 800;

class CustomTexture {
private:
    SDL_Texture* texture;
    SDL_Rect dstArea;

public:
    CustomTexture();
    ~CustomTexture();

    bool createBlank(int width, int height);
    bool createFromImage(std::string path);
    bool createFromText(std::string text, SDL_Color textColor);
    void render(int x, int y, double angle = 0.0, SDL_Point* angleCenter = NULL);
    void free();

    SDL_Texture* getTexture();
    SDL_Rect getArea();
};

bool init();
bool loadMedia();
void close();

SDL_Window* mainWindow = NULL;
SDL_Renderer* mainRenderer = NULL;

TTF_Font* mainFont = NULL;

CustomTexture promptTexture_1;
CustomTexture promptTexture_2;

// Regulator
CustomTexture regulatorTexture;
CustomTexture regulatorTextTexture;

const int REGULATOR_MAX_ANGLE = 270;
const int REGULATOR_TORQUE = 10;
const int REGULATOR_AREA_WIDTH = 400;

SDL_Point regulatorCenter = { REGULATOR_AREA_WIDTH / 2, SCREEN_HEIGHT / 5 };
SDL_Rect regulatorArea = { REGULATOR_AREA_WIDTH / 4 - 10, SCREEN_HEIGHT / 5 - 10, 20, 20 };
SDL_Color regulatorColor = { 0,0,0,255 };

// Windmill
CustomTexture windmillTexture;
CustomTexture windFanTexture;

SDL_Rect windmillArea = { SCREEN_WIDTH / 3, SCREEN_HEIGHT - 300, 50, 300 };

CustomTexture::CustomTexture() {
    texture = NULL;
    dstArea = { 0,0,0,0 };
}

CustomTexture::~CustomTexture() {
    free();
}

bool CustomTexture::createBlank(int width, int height) {
    texture = SDL_CreateTexture(mainRenderer, SDL_GetWindowPixelFormat(mainWindow), SDL_TEXTUREACCESS_TARGET, width, height);
    
    if (texture == NULL) {
        printf("Unable to create target rendering texture! SDL error: %s\n", SDL_GetError());
    }
    else {
        dstArea.w = width;
        dstArea.h = height;
    }
    return texture != NULL;
}

bool CustomTexture::createFromImage(std::string path) {
    free();

    SDL_Surface* loadSurface = IMG_Load("wind_fan.png");
    if (loadSurface == NULL) {
        printf("Unable to load surface from text! SDL_IMG error: %s\n", IMG_GetError());
    }
    else {
        texture = SDL_CreateTextureFromSurface(mainRenderer, loadSurface);
        if (texture == NULL) {
            printf("Unable to create texture from surface! SDL error: %s\n", SDL_GetError());
        }
        else {
            dstArea.w = loadSurface->w;
            dstArea.h = loadSurface->h;
        }
        SDL_FreeSurface(loadSurface);
        loadSurface = NULL;
    }
    return texture != NULL;
}

bool CustomTexture::createFromText(std::string text, SDL_Color textColor) {
    free();

    SDL_Surface* loadSurface = TTF_RenderText_Solid(mainFont, text.c_str(), textColor);
    if (loadSurface == NULL) {
        printf("Unable to load surface from text! SDL_TTF error: %s\n", TTF_GetError());
    }
    else {
        texture = SDL_CreateTextureFromSurface(mainRenderer, loadSurface);
        if (texture == NULL) {
            printf("Unable to create texture from surface! SDL error: %s\n", SDL_GetError());
        }
        else {
            dstArea.w = loadSurface->w;
            dstArea.h = loadSurface->h;
        }
        SDL_FreeSurface(loadSurface);
        loadSurface = NULL;
    }
    return texture != NULL;
}

void CustomTexture::render(int x, int y, double angle, SDL_Point* angleCenter) {
    dstArea.x = x;
    dstArea.y = y;
    SDL_RenderCopyEx(mainRenderer, texture, NULL, &dstArea, angle, angleCenter, SDL_FLIP_NONE);
}

void CustomTexture::free() {
    if (texture != NULL) {
        SDL_DestroyTexture(texture);
        texture = NULL;
        dstArea = { 0,0,0,0 };
    }
}

SDL_Texture* CustomTexture::getTexture() {
    return texture;
}

SDL_Rect CustomTexture::getArea() {
    return dstArea;
}

bool init() {
    bool successFlag = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not initialize SDL! SDL Error: %s\n", SDL_GetError());
        successFlag = false;
    }
    else {
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            printf("WARNING: Linear texture filtering is not enabled!\n");
        }

        mainWindow = SDL_CreateWindow("Windmill", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

        if (mainWindow == NULL) {
            printf("Could not create window! SDL Error: %s\n", SDL_GetError());
            successFlag = false;
        }
        else {
            mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

            if (mainRenderer == NULL) {
                printf("Could not create renderer! SDL Error: %s\n", SDL_GetError());
                successFlag = false;
            }
            else {
                SDL_SetRenderDrawColor(mainRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

                if (TTF_Init() < 0) {
                    printf("Could not initialize TTF! SDL_TTF error: %s\n", TTF_GetError());
                    successFlag = false;
                }

                int imgFlag = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlag) & imgFlag)) {
                    printf("Could not initialize SDL_IMAGE! SDL_TTF error: %s\n", IMG_GetError());
                    successFlag = false;
                }
            }
        }
    }
    return successFlag;
}

bool loadMedia() {
    bool successFlag = true;

    mainFont = TTF_OpenFont("Montserrat.ttf", 28);
    if (mainFont == NULL) {
        printf("Unable to open font! SDL_TTF error: %s\n", TTF_GetError());
        successFlag = false;
    }

    if (!regulatorTexture.createBlank(REGULATOR_AREA_WIDTH, SCREEN_HEIGHT / 5 + 150)) {
        printf("Failed to create regulator texture!\n");
        successFlag = false;
    }

    if (!windmillTexture.createBlank(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        printf("Failed to create windmill texture!\n");
        successFlag = false;
    }

    if (!windFanTexture.createFromImage("wind_fan.png")) {
        printf("Failed to create texture from wind fan image!\n");
        successFlag = false;
    }

    SDL_Color promptColor = { 0,0,0,255 };

    if (!promptTexture_1.createFromText("Use Up button to speed up", promptColor)) {
        printf("Unable to create texture for promt text 1!\n");
        successFlag = false;
    }

    if (!promptTexture_2.createFromText("Use Down button to speed down", promptColor)) {
        printf("Unable to create texture for promt text 2!\n");
        successFlag = false;
    }

    if (!regulatorTextTexture.createFromText("0", regulatorColor)) {
        printf("Unable to create texture for regulator text!\n");
        successFlag = false;
    }

    return successFlag;

}

void close() {
    TTF_CloseFont(mainFont);
    mainFont = NULL;

    regulatorTexture.free();
    regulatorTextTexture.free();
    windmillTexture.free();

    SDL_DestroyRenderer(mainRenderer);
    mainRenderer = NULL;

    SDL_DestroyWindow(mainWindow);
    mainWindow = NULL;

    SDL_Quit();
}

int main(int argc, char* args[]) {
    if (!init()) {
        printf("Failed to initialize!\n");
    }
    else {
        if (!loadMedia()) {
            printf("Failed to load media!\n");
        }
        else {
            bool quit = false;
            SDL_Event e;

            double regulatorAngle = 0;
            int regulatorPower = 0;
            bool regulatorAngleChanged = false;

            double windFanAngle = 0;
            SDL_Point windFanCenter = { windFanTexture.getArea().w / 2, windFanTexture.getArea().h / 2 };

            while (!quit) {
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }

                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_UP) {
                            regulatorAngle += REGULATOR_TORQUE;
                            regulatorAngleChanged = true;

                            if (regulatorAngle > REGULATOR_MAX_ANGLE) {
                                regulatorAngle = REGULATOR_MAX_ANGLE;
                                regulatorAngleChanged = false;
                            }
                        }
                        else if (e.key.keysym.sym == SDLK_DOWN) {
                            regulatorAngle -= REGULATOR_TORQUE;
                            regulatorAngleChanged = true;

                            if (regulatorAngle < 0) {
                                regulatorAngle = 0;
                                regulatorAngleChanged = false;
                            }
                        }
                    }
                }

                // Windmill drawing
                SDL_SetRenderTarget(mainRenderer, windmillTexture.getTexture());
                
                SDL_SetRenderDrawColor(mainRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(mainRenderer);
                
                SDL_SetRenderDrawColor(mainRenderer, 150, 150, 150, 0xFF);
                SDL_RenderFillRect(mainRenderer, &windmillArea);
                SDL_RenderDrawRect(mainRenderer, &windmillArea);

                // Regulator drawing
                SDL_SetRenderTarget(mainRenderer, regulatorTexture.getTexture());

                SDL_SetRenderDrawColor(mainRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(mainRenderer);

                SDL_SetRenderDrawColor(mainRenderer, regulatorColor.r, regulatorColor.g, regulatorColor.b, regulatorColor.a);
                SDL_RenderFillRect(mainRenderer, &regulatorArea);
                SDL_RenderDrawRect(mainRenderer, &regulatorArea);

                SDL_SetRenderTarget(mainRenderer, NULL);

                // Rendering
                windmillTexture.render(0, 0);

                regulatorTexture.render(SCREEN_WIDTH - REGULATOR_AREA_WIDTH, 0, regulatorAngle, &regulatorCenter);
                
                if (regulatorAngleChanged) {
                    regulatorPower = regulatorAngle * 100 / REGULATOR_MAX_ANGLE;
                    regulatorTextTexture.createFromText(std::to_string(regulatorPower), regulatorColor);
                }

                regulatorTextTexture.render(SCREEN_WIDTH - REGULATOR_AREA_WIDTH / 2 - regulatorTextTexture.getArea().w / 2, SCREEN_HEIGHT * 1 / 5 - regulatorTextTexture.getArea().h / 2);
                
                promptTexture_1.render(SCREEN_WIDTH / 3 - (promptTexture_1.getArea().w / 2), SCREEN_HEIGHT / 6);
                promptTexture_2.render(SCREEN_WIDTH / 3 - (promptTexture_2.getArea().w / 2), promptTexture_1.getArea().y + promptTexture_1.getArea().h + 10);

                windFanAngle += regulatorPower / 8.0;

                if (windFanAngle > 360) {
                    windFanAngle -= 360;
                }

                windFanTexture.render(SCREEN_WIDTH / 3 - windFanTexture.getArea().w / 2 + 25, SCREEN_HEIGHT - windmillArea.h - 10 - windFanTexture.getArea().h / 2, windFanAngle, &windFanCenter);

                SDL_RenderPresent(mainRenderer);
            }
        }
    }
    close();
    return 0;
}