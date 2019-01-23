/**
 * Korori Main Source File
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "SDL.h"
#include "SDL_image.h"
#include "foundation/common.h"
#include "foundation/window.h"
#include "graphics/common.h"
#include "graphics/texture.h"
#include "graphics/spritesheet.h"
#include "graphics/font.h"

#include "usercode.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define LOGICAL_WIDTH 640
#define LOGICAL_HEIGHT 480
#define SETFRAME(var, arg1, arg2, arg3, arg4)		\
  do {										\
    var.x = arg1;							\
    var.y = arg2;							\
    var.w = arg3;							\
    var.h = arg4;							\
  } while(0)

// cap thus using fixed deltaTime step
#define TARGET_FPS 60
#define FIXED_DELTATIME 1.0f / TARGET_FPS

// -- functions
bool init();
bool setup();
void handleEvent(SDL_Event *e, float deltaTime);
void update(float deltaTime);
void render(float deltaTime);
void close();

// opengl context
SDL_GLContext opengl_context;

// -- variables
bool quit = false;

// independent time loop
Uint32 currTime = 0;
Uint32 prevTime = 0;

bool init() {
  // initialize sdl
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
    return false;
  }
  
  // use core profile of opengl 3.3
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  // just to be pragmatic
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
  // following these 3 lines might not be needed
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // create window
  // if we set SDL_WINDOW_OPENGL flag then renderer won't be created for this window
  // thus make sure you cannot use LTexture anymore as it heavilty use renderer as created in KRR_WINDOW
  gWindow = KRR_WINDOW_new("Korori - Test", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL, 0);
  if (gWindow == NULL) {
    SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
    return false;
  }
	// listen to window's resize event
	gWindow->on_window_resize = usercode_set_screen_dimension;

  // create opengl context
  opengl_context = SDL_GL_CreateContext(gWindow->window);
  if (opengl_context == NULL)
  {
    SDL_Log("OpenGL context could not be created: %s", SDL_GetError());
    return false;
  }

  // check opengl version we got
  printf("OpenGL version %s\nGLSL version: %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

  // use vsync
  if (SDL_GL_SetSwapInterval(1) != 0)
  {
    SDL_Log("Warning: Unable to enable vsync! %s", SDL_GetError());
  }

  // init glew
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK)
  {
    SDL_Log("Failed initialize glew! %s", glewGetErrorString(glewError));
    return false;
  }

  // make sure OpenGL 3.3 is supported
  if (!GLEW_VERSION_3_3)
  {
    SDL_Log("OpenGL 3.3 not supported!");
    return false;
  }

  // relay call to user's code in separate file
  if (!usercode_init(SCREEN_WIDTH, SCREEN_HEIGHT, LOGICAL_WIDTH, LOGICAL_HEIGHT))
  {
    SDL_Log("Failed to initialize user's code initializing function");
    return false;
  }

  // initialize png loading
  // see https://www.libsdl.org/projects/SDL_image/docs/SDL_image.html#SEC8
  // returned from IMG_Init is all flags initted, so we could check for all possible
  // flags we aim for
  int imgFlags = IMG_INIT_PNG;
  int inittedFlags = IMG_Init(imgFlags);
  if ( (inittedFlags & imgFlags) != imgFlags)
  {
    // from document, not always that error string from IMG_GetError() will be set
    // so don't depend on it, just for pure information
    SDL_Log("SDL_Image could not initialize! SDL_image Error: %s", IMG_GetError());
    return false;
  }

  return true;
}

// include any asset loading sequence, and preparation code here
bool setup()
{
  // load media from usercode
  if (!usercode_loadmedia())
  {
    SDL_Log("Failed to load media from usercode");
    return false;
  }

  return true;
}

void handleEvent(SDL_Event *e, float deltaTime)
{
  // user requests quit
  if (e->type == SDL_QUIT ||
      (e->key.keysym.sym == SDLK_ESCAPE))
  {
    quit = true;
  }
  else
  {
    // relay call to user's code in separate file
    usercode_handle_event(e, deltaTime);
  }
}

void update(float deltaTime)
{
  // relay call to user's code in separate file
  usercode_update(deltaTime);
}

void render(float deltaTime)
{
  if (!gWindow->is_minimized)
  {
    // relay call to user's code in separate file
    usercode_render();

#ifndef DISABLE_FPS_CALC
    // render fps
    usercode_render_fps((int)common_avgFPS);
#endif
  }
}

void close()
{
  // relay call to user's code in separate file
  usercode_close();

  // destroy window
  KRR_WINDOW_free(gWindow);

  IMG_Quit();
  SDL_Quit();
}

int main(int argc, char* args[])
{
  // start up SDL and create window
  if (!init())
  {
    SDL_Log("Failed to initialize");
  }	
  else
  {
    // load media, and set up
    if (!setup())
    {
      SDL_Log("Failed to setup!");
    }
    else
    {
      // event handler
      SDL_Event e;

      // while application is running
      while (!quit)
      {
        // prepare delta time to feed to both handleEvent(), update(), and render()
        prevTime = currTime;
        currTime = SDL_GetTicks();
        // calculate per second
        float deltaTime = (currTime - prevTime) / 1000.0f;

        // fixed step
        common_frameTime += deltaTime;

#ifndef DISABLE_FPS_CALC
				// update accumulated time for calculating framerate
        common_frameAccumTime += deltaTime;
#endif
        if (common_frameTime >= FIXED_DELTATIME)
        {
#ifndef DISABLE_FPS_CALC
          common_frameCount++;

          // check to reset frame time
          if (common_frameAccumTime >= 1.0f)
          {
            common_avgFPS = common_frameCount / common_frameAccumTime;
            common_frameCount = 0;
            common_frameAccumTime -= 1.0f;
          }
#endif
          common_frameTime = 0.0f;

          // handle events on queue
          // if it's 0, then it has no pending event
          // we keep polling all event in each game loop until there is no more pending one left
          while (SDL_PollEvent(&e) != 0)
          {
            // update user's handleEvent()
            handleEvent(&e, FIXED_DELTATIME);
          }

          update(FIXED_DELTATIME);
          render(FIXED_DELTATIME);
        }
        else {
          render(0); 
        }

        // update screen
        SDL_GL_SwapWindow(gWindow->window);
      }
    }
  }

  // free resource and close SDL
  close();

  return 0;
}

