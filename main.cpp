/**
* Author:  Jack Ma
* Assignment: Lunar Landing
* Date due: 2023-11-8, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

#include <iostream>
#include <vector>
using namespace std;

#define PLATFORM_COUNT 29
struct GameState {
    Entity *player;
    Entity *platforms;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;

float missionPass = 0; // 0 = ongoing, 1 = pass, -1 = fail

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint fontTextureID;

GLuint LoadTexture(const char* filePath) {
    // STEP 1: Loading the image file
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        cout << "Unable to load image. Make sure the path is correct";
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Lunar Landing!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    // Load up our shaders
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    // Defines every translation, rotations, or scaling applied to an object
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
   
    // Initialize Game Objects
    
    // Initialize Player
    state.player = new Entity();
    state.player->position = glm::vec3(0, 3.25, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -0.1f, 0);
    state.player->speed = 1.5f;
    state.player->textureID = LoadTexture("assets/playerShip3_orange.png");
    
    state.player->height = 1.0f;
    state.player->width = 1.0f;
    
    state.player->jumpPower = 5.0f;
    
    // Images
    state.platforms = new Entity[PLATFORM_COUNT];
    GLuint platformTextureID = LoadTexture("assets/platformPack_tile016.png");
    GLuint landingSiteTextureID = LoadTexture("assets/platformPack_tile015.png");
    fontTextureID = LoadTexture("assets/font1.png");
    
    // building the world design
    float x = -5.5f;
    float y = -3.25f;
    for (int i = 0; i < 11; ++i) {
        state.platforms[i].textureID = platformTextureID;
        state.platforms[i].position = glm::vec3(x, y, 0);
        x += 1;
    }
    x = -4.5f;
    y = -2.25f;
    for (int i = 11; i < 25; ++i) {
        if (y >= 4.25f) {
            y = -2.25f;
            x = 4.5f;
        }
        state.platforms[i].textureID = platformTextureID;
        state.platforms[i].position = glm::vec3(x, y, 0);
        y += 1;
    }
    x = 1;
    y = 1;
    for (int i = 25; i < PLATFORM_COUNT; ++i) {
        if (i == 27) {
            y = -1;
            x = -3;
        }
        state.platforms[i].textureID = platformTextureID;
        state.platforms[i].position = glm::vec3(x, y, 0);
        x += 1;
    }
    
    state.platforms[7].textureID = landingSiteTextureID;
    state.platforms[8].textureID = landingSiteTextureID;
    
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].Update(0, NULL, 0);
    }
 
}

void ProcessInput() {
    
    state.player->movement = glm::vec3(0);
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        // Move the player left
                        break;
                        
                    case SDLK_RIGHT:
                        // Move the player right
                        break;
                        
                    case SDLK_SPACE:
                        // Some sort of action
                        if (state.player ->collidedBottom) {
                            state.player->jump = true;
                        }
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->acceleration.x += -1.0f;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        state.player->acceleration.x += 1.0f;
    }
    
    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }

}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;
void Update() {
    if (state.player->collidedLeft || state.player->collidedRight || state.player->collidedTop) {
        missionPass = -1;
    }
    if (state.player->collidedBottom == true) {
        if (state.platforms[7].collidedTop == true || state.platforms[8].collidedTop == true) {
            missionPass = 1;
        }
        else {
            missionPass = -1;
        }
    }
    if (missionPass == -1 || missionPass == 1) {
        return;
    }
   float ticks = (float)SDL_GetTicks() / 1000.0f;
   float deltaTime = ticks - lastTicks;
   lastTicks = ticks;
   deltaTime += accumulator;
   if (deltaTime < FIXED_TIMESTEP) {
       accumulator = deltaTime;
       return;
   }
   while (deltaTime >= FIXED_TIMESTEP) {
       state.player->Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
       deltaTime -= FIXED_TIMESTEP;
   }
   accumulator = deltaTime;
}

void DrawText(ShaderProgram *program, GLuint fontTextureID, std::string text,
              float size, float spacing, glm::vec3 position) {
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;
    std::vector<float> vertices;
    std::vector<float> texCoords;
    for(int i = 0; i < text.size(); i++) {
        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
        });
        texCoords.insert(texCoords.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
        });
    } // end of for loop
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}


void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].Render(&program);
    }
    state.player->Render(&program);
    if (missionPass == -1) {
        DrawText(&program, fontTextureID, "Mission Failed!", 1, -0.5, glm::vec3(-3.375f, 0, 0));
    }
    else if (missionPass == 1) {
        DrawText(&program, fontTextureID, "Mission Succeeded!", 1, -0.5, glm::vec3(-4.125f, 0, 0));
    }
    SDL_GL_SwapWindow(displayWindow);
}


void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
