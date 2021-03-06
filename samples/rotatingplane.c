/* sample program to render texture on the screen using KRR_TEXTURE
 *
 * updated:
 * - camera movement along x and y axis (as it's still orthographic projection matrix, z-axis won't be taken into effect)
 */


#include "usercode.h"
#include "functs.h"
#include "krr/foundation/common.h"
#include "krr/foundation/window.h"
#include "krr/foundation/util.h"
#include "krr/foundation/cam.h"
#include "krr/foundation/math.h"
#include "krr/graphics/util.h"
#include "krr/graphics/texturedpp2d.h"
#include "krr/graphics/font.h"
#include "krr/graphics/fontpp2d.h"
#include "krr/graphics/texture.h"
#include <math.h>

// don't use this elsewhere
#define CONTENT_BG_COLOR 0.f, 0.f, 0.f, 1.f

#ifndef DISABLE_FPS_CALC
#define FPS_BUFFER 7+1
char fps_text[FPS_BUFFER];
static KRR_FONT* fps_font = NULL;
#endif

// -- section of variables for maintaining aspect ratio -- //
static int g_screen_width;
static int g_screen_height;

static int g_logical_width;
static int g_logical_height;
static float g_logical_aspect;

static int g_offset_x = 0;
static int g_offset_y = 0;
// resolution independent scale for x and y
static float g_ri_scale_x = 1.0f;
static float g_ri_scale_y = 1.0f;
// resolution independent dimensions
static int g_ri_view_width;
static int g_ri_view_height;

static bool g_need_clipping = false;
// -- section of variables for maintaining aspect ratio -- //

// -- section of function signatures -- //
static void usercode_app_went_windowed_mode();
static void usercode_app_went_fullscreen();
// -- end of section of function signatures -- //

// basic shaders and font
static KRR_TEXSHADERPROG2D* texture_shader = NULL;
static KRR_FONTSHADERPROG2D* font_shader = NULL;
static KRR_FONT* font = NULL;

// TODO: define variables here
#define CLIPPED_TEX_SIZE 64.0f
static float rotx = 0.f, roty = 0.f;
static float scale_angle = 0.f, scale = 1.f;
static KRR_TEXTURE* texture = NULL;
static KRR_TEXTURE* texture_clipped = NULL;
static KRR_CAM cam;

void usercode_app_went_windowed_mode()
{
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
  SU_END(font_shader)
}

void usercode_app_went_fullscreen()
{
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
  SU_END(font_shader)
}

bool usercode_init(int screen_width, int screen_height, int logical_width, int logical_height)
{
  // FIXME: This code would works only if user starts with windowed mode, didnt test yet if start with fullscreen mode ...
  // set input screen dimensions
  g_screen_width = screen_width;
  g_screen_height = screen_height;

  g_logical_width = logical_width;
  g_logical_height = logical_height;
  g_logical_aspect = g_screen_width * 1.0f / g_screen_height;

  // start off with resolution matching the screen
  g_ri_view_width = g_screen_width;
  g_ri_view_height = g_screen_height;

  // calculate orthographic projection matrix
  // make sure near and far plane value are large enough to support rendering the scene
	glm_ortho(0.0, g_screen_width, g_screen_height, 0.0, -300, 600.0, g_ui_projection_matrix);
  glm_perspective(GLM_PI_4f, g_screen_width * 1.0f / g_screen_height, 0.01f, 10000.0f, g_projection_matrix);
  // calculate view matrix
  glm_mat4_identity(g_view_matrix);
	// calculate base modelview matrix (to reduce some of operations cost)
	glm_mat4_identity(g_base_model_matrix);

  // calculate base model for ui model matrix, and scale it
  glm_mat4_identity(g_base_ui_model_matrix);
	glm_scale(g_base_ui_model_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

  // initialize the viewport
  // define the area where to render, for now full screen
  glViewport(0, 0, g_screen_width, g_screen_height);

  // initialize clear color
  glClearColor(0.f, 0.f, 0.f, 1.f);

  // enable blending with default blend function
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  // enable depth testing
  glEnable(GL_DEPTH_TEST);

  // initially start user's camera looking at -z, and up with +y
  glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam.forward);
  glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam.up);
  glm_vec3_copy((vec3){0.0f, 1.0f, 20.0f}, cam.pos);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_util_print_callstack();
    KRR_LOGE("Error initializing OpenGL! %s", KRR_gputil_error_string(error));
    return false;
  }

  return true;
}

bool usercode_loadmedia()
{
  // load texture shader
  texture_shader = KRR_TEXSHADERPROG2D_new();
  if (!KRR_TEXSHADERPROG2D_load_program(texture_shader))
  {
    KRR_LOGE("Error loading texture shader");
    return false;
  }
  // set texture shader to all KRR_TEXTURE as active
  // note: set this now so KRR_TEXTURE is able to work in initialization
  shared_textured_shaderprogram = texture_shader;
  
  // load font shader
  font_shader = KRR_FONTSHADERPROG2D_new();
  if (!KRR_FONTSHADERPROG2D_load_program(font_shader))
  {
    KRR_LOGE("Error loading font shader");
    return false;
  }
  // set font shader to all KRR_FONT as active
  shared_font_shaderprogram = font_shader;

  // load font to render framerate
#ifndef DISABLE_FPS_CALC
  {
    fps_font = KRR_FONT_new();
    if (!KRR_FONT_load_freetype(fps_font, "res/fonts/Minecraft.ttf", 14))
    {
      KRR_LOGE("Unable to load font for rendering framerate");
      return false;
    }
  }
#endif
  
  // create font
  font = KRR_FONT_new();
  if (!KRR_FONT_load_freetype(font, "res/fonts/Minecraft.ttf", 40))
  {
    KRR_LOGE("Error to load font");
    return false;
  }

  // TODO: Load media here...
  // load texture
  texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(texture, "res/alien-arcade.png"))
  {
    KRR_LOGE("Cannot load alien-arcade texture");
    return false;
  }

  // load texture clipped
  texture_clipped = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(texture_clipped, "res/sheet.png"))
  {
    KRR_LOGE("Cannot load sheet texture");
    return false;
  }
	// END OF TODO

  // initially update all related matrices and related graphics stuff for both basic shaders
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
    // set texture unit
    KRR_TEXSHADERPROG2D_set_texture_sampler(texture_shader, 0);
  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
    // set texture unit
    KRR_FONTSHADERPROG2D_set_texture_sampler(font_shader, 0);
  SU_END(font_shader)

  return true;
}

void usercode_set_screen_dimension(Uint32 window_id, int screen_width, int screen_height)
{
  g_screen_width = screen_width;
  g_screen_height = screen_height;
}

void usercode_handle_event(SDL_Event *e, float delta_time)
{
  if (e->type == SDL_KEYDOWN)
  {
    int k = e->key.keysym.sym;

    // toggle fullscreen via enter key
    if (k == SDLK_RETURN)
    {
      // go windowed mode, currently in fullscreen mode
      if (gWindow->fullscreen)
      {
        KRR_WINDOW_set_fullscreen(gWindow, false);
        // set projection matrix back to normal
        KRR_gputil_adapt_to_normal(g_screen_width, g_screen_height);
        // reset relavant values back to normal
        g_offset_x = 0.0f;
        g_offset_y = 0.0f;
        g_ri_scale_x = 1.0f;
        g_ri_scale_y = 1.0f;
        g_ri_view_width = g_screen_width;
        g_ri_view_height = g_screen_height;
        g_need_clipping = false;
				
				// re-calculate orthographic projection matrix
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -300.0, 600.0, g_ui_projection_matrix);
        glm_perspective(GLM_PI_4f, g_ri_view_width * 1.0f / g_ri_view_height, 0.01f, 10000.0f, g_projection_matrix);

				// re-calculate base model matrix
				// no need to scale as it's uniform 1.0 now
				glm_mat4_identity(g_base_ui_model_matrix);

				// signal that app went windowed mode
				usercode_app_went_windowed_mode();
      }
      else
      {
        KRR_WINDOW_set_fullscreen(gWindow, true);
        // get new window's size
        int w, h;
        SDL_GetWindowSize(gWindow->window, &w, &h);
        // also adapt to letterbox
        KRR_gputil_adapt_to_letterbox(w, h, g_logical_width, g_logical_height, &g_ri_view_width, &g_ri_view_height, &g_offset_x, &g_offset_y);
        // calculate scale 
        g_ri_scale_x = g_ri_view_width * 1.0f / g_logical_width;
        g_ri_scale_y = g_ri_view_height * 1.0f / g_logical_height;
        g_need_clipping = true;

				// re-calculate orthographic projection matrix
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -300.0, 600.0, g_ui_projection_matrix);
        glm_perspective(GLM_PI_4f, g_ri_view_width * 1.0f / g_ri_view_height, 0.01f, 10000.0f, g_projection_matrix);

				// re-calculate base model matrix
				glm_mat4_identity(g_base_ui_model_matrix);
				// also scale
				glm_scale(g_base_ui_model_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

				// signal that app went fullscreen mode
				usercode_app_went_fullscreen();
      }
    }

    
  }

  // handle multiple key presses at once
  bool cam_update = false;
  const Uint8* key_state = SDL_GetKeyboardState(NULL);

  // move speed is distance per second
  #define MOVE_SPEED 30.0f
  #define MOVE_FACTOR 0.1f

  if (key_state[SDL_SCANCODE_A])
  {
    cam.topos[0] -= MOVE_SPEED;
    cam_update = true;
  }
  if (key_state[SDL_SCANCODE_D])
  {
    cam.topos[0] += MOVE_SPEED;
    cam_update = true;
  }
  if (key_state[SDL_SCANCODE_W])
  {
    cam.topos[1] -= MOVE_SPEED;
    cam_update = true;
  }
  if (key_state[SDL_SCANCODE_S])
  {
    cam.topos[1] += MOVE_SPEED;
    cam_update = true;
  }
}

void usercode_update(float delta_time)
{
  // lerp camera's position
  // always do this for smoothness
  KRR_math_lerpv(cam.pos, cam.topos, MOVE_FACTOR);

  // update view matrix
  KRR_gputil_create_view_matrix(cam.pos, cam.rot, 1.0f, g_view_matrix);

  KRR_SHADERPROG_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  KRR_SHADERPROG_unbind(texture_shader->program);

  // update transforming variables of quad
  scale = sin(glm_rad(scale_angle++)) * 2.f;
  
  rotx += 5.f;
  if (rotx >= 360.f)
  {
    rotx -= 360.f;
  }

  roty += 1.f;
  if (roty >= 360.f)
  {
    roty -= 360.f;
  }
}

void usercode_render_ui_text()
{
}

void usercode_render()
{
  // clear color buffer
  if (g_need_clipping)
    glClearColor(0.f, 0.f, 0.f, 1.f);
  else
    glClearColor(CONTENT_BG_COLOR);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // now clip content to be drawn only on content area (if needed)
  if (g_need_clipping)
  {
    // clear color for content area
    glEnable(GL_SCISSOR_TEST);
    glScissor(g_offset_x, g_offset_y, g_ri_view_width, g_ri_view_height);
    glClearColor(CONTENT_BG_COLOR);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  // TODO: render code goes here...
  // bind vao
  KRR_TEXTURE_bind_vao(texture);
    // bind shader program
    KRR_SHADERPROG_bind(texture_shader->program);

    // render normal texture
    glm_mat4_copy(g_base_ui_model_matrix, texture_shader->model_matrix);
    glm_translate(texture_shader->model_matrix, (vec3){g_logical_width*1.f/4.f, g_logical_height/2.f, 0.f});
    glm_scale_uni(texture_shader->model_matrix, scale);
    glm_rotate(texture_shader->model_matrix, glm_rad(roty), (vec3){0.f, 1.f, 0.f});
    glm_rotate(texture_shader->model_matrix, glm_rad(rotx), (vec3){1.f, 0.f, 0.f});
    glm_translate(texture_shader->model_matrix, (vec3){-texture->width/2.f, -texture->height/2.f, 0});

    KRR_TEXTURE_render(texture, 0.f, 0.f, NULL);

    // unbind shader program
    KRR_SHADERPROG_unbind(texture_shader->program);

  // bind clipped texture vao
  KRR_TEXTURE_bind_vao(texture_clipped);
    // bind shader program
    KRR_SHADERPROG_bind(texture_shader->program);

    // render clipped texture
    glm_mat4_copy(g_base_ui_model_matrix, texture_shader->model_matrix);
    glm_translate(texture_shader->model_matrix, (vec3){g_logical_width*3.f/4.f, g_logical_height/2.f, 0.f});
    glm_scale_uni(texture_shader->model_matrix, scale);
    glm_rotate(texture_shader->model_matrix, glm_rad(roty), (vec3){0.f, 1.f, 0.f});
    glm_rotate(texture_shader->model_matrix, glm_rad(rotx), (vec3){1.f, 0.f, 0.f});
    glm_translate(texture_shader->model_matrix, (vec3){-CLIPPED_TEX_SIZE/2.f, -CLIPPED_TEX_SIZE/2.f, 0});

    KRR_TEXTURE_render(texture_clipped, 0.f, 0.f, &(RECT){320.f, 0.f, CLIPPED_TEX_SIZE, CLIPPED_TEX_SIZE});

    // unbind shader program
    KRR_SHADERPROG_unbind(texture_shader->program);

  // unbind vao
  KRR_TEXTURE_unbind_vao(texture_clipped);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_render_fps(int avg_fps)
{
#ifndef DISABLE_FPS_CALC
  // form framerate string to render
  snprintf(fps_text, FPS_BUFFER-1, "%d", avg_fps);

  // bind vao
  KRR_FONT_bind_vao(fps_font);

  // use shared font shader
  KRR_SHADERPROG_bind(shared_font_shaderprogram->program);

    // start with clean state of modelview matrix
    glm_mat4_copy(g_base_ui_model_matrix, shared_font_shaderprogram->model_matrix);
    KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);

    // render text on top right
    KRR_FONT_render_textex(fps_font, fps_text, 0.f, 4.f, &(SIZE){g_logical_width, g_logical_height}, KRR_FONT_TEXTALIGNMENT_RIGHT | KRR_FONT_TEXTALIGNMENT_TOP);
  KRR_SHADERPROG_unbind(shared_font_shaderprogram->program);

  // unbind vao
  KRR_FONT_unbind_vao(fps_font);
#endif 
}

void usercode_close()
{
#ifndef DISABLE_FPS_CALC
  if (fps_font != NULL)
  {
    KRR_FONT_free(fps_font);
    fps_font = NULL;
  }
#endif
  if (font != NULL)
  {
    KRR_FONT_free(font);
    font = NULL;
  }
  if (font_shader != NULL)
  {
    KRR_FONTSHADERPROG2D_free(font_shader);
    font_shader = NULL;
  }
  if (texture != NULL)
  {
    KRR_TEXTURE_free(texture);
    texture = NULL;
  }
  if (texture_clipped != NULL)
  {
    KRR_TEXTURE_free(texture_clipped);
    texture_clipped = NULL;
  }
  if (texture_shader != NULL)
  {
    KRR_TEXSHADERPROG2D_free(texture_shader);
    texture_shader = NULL;
  }
}
