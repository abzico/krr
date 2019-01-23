#include "doublemulticolorpp2d.h"
#include <stdlib.h>
#include "foundation/common.h"

KRR_DMULTICSHADERPROG2D* KRR_DMULTICSHADERPROG2D_new()
{
  KRR_DMULTICSHADERPROG2D* out = malloc(sizeof(KRR_DMULTICSHADERPROG2D));

  // init zeros first
  out->program = NULL;
  out->vertex_pos2d_location = -1;
  out->multicolor1_location = -1;
  out->multicolor2_location = -1;
  out->projection_matrix_location = -1;
  out->modelview_matrix_location = -1;
  glm_mat4_identity(out->projection_matrix);
  glm_mat4_identity(out->modelview_matrix);

  // init
  out->program = KRR_SHADERPROG_new();

  return out;
}

void KRR_DMULTICSHADERPROG2D_free_internals(KRR_DMULTICSHADERPROG2D* program)
{
  // free underlying shader program
  KRR_SHADERPROG_free(program->program);

  program->vertex_pos2d_location = -1;
  program->multicolor1_location = -1;
  program->multicolor2_location = -1;
  program->projection_matrix_location = -1;
  program->modelview_matrix_location = -1;

  glm_mat4_identity(program->projection_matrix);
  glm_mat4_identity(program->modelview_matrix);
}

void KRR_DMULTICSHADERPROG2D_free(KRR_DMULTICSHADERPROG2D* program)
{
  // free internals
  KRR_DMULTICSHADERPROG2D_free_internals(program);

  // free source
  free(program);
  program = NULL;
}

bool KRR_DMULTICSHADERPROG2D_load_program(KRR_DMULTICSHADERPROG2D* program)
{
  // create a new program
  GLuint program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader_id = KRR_SHADERPROG_load_shader_from_file("res/shaders/doublemulticolorpp2d.vert", GL_VERTEX_SHADER);
  if (vertex_shader_id == 0)
  {
    KRR_LOGE("Unable to load vertex shader from file");

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // attach vertex shader to shader program
  glAttachShader(program_id, vertex_shader_id);
  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_LOGE("Error attaching vertex shader");
    KRR_SHADERPROG_print_shader_log(vertex_shader_id);

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // load fragment shader
  GLuint fragment_shader_id = KRR_SHADERPROG_load_shader_from_file("res/shaders/doublemulticolorpp2d.frag", GL_FRAGMENT_SHADER);
  if (fragment_shader_id == 0)
  {
    KRR_LOGE("Unable to load fragment shader from file");

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // attach fragment shader to program
  glAttachShader(program_id, fragment_shader_id);
  // check for errors
  error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_LOGE("Error attaching fragment shader");
    KRR_SHADERPROG_print_shader_log(fragment_shader_id);

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // link program
  glLinkProgram(program_id);
  error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_LOGE("Error linking program");
    KRR_SHADERPROG_print_program_log(program_id);

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete fragment shader
    glDeleteShader(fragment_shader_id);
    fragment_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // set result program id to underlying program
  program->program->program_id = program_id;

  // mark shader for delete
  glDeleteShader(vertex_shader_id);
  glDeleteShader(fragment_shader_id);

  // get attribute locations
  program->vertex_pos2d_location = glGetAttribLocation(program_id, "vertex_pos2d");
  if (program->vertex_pos2d_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of vertex_pos2d");
  }
  program->multicolor1_location = glGetAttribLocation(program_id, "multicolor1");
  if (program->multicolor1_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of multicolor1");
  }
  program->multicolor2_location = glGetAttribLocation(program_id, "multicolor2");
  if (program->multicolor2_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of multicolor2");
  }

  // get uniform locations
  program->projection_matrix_location = glGetUniformLocation(program_id, "projection_matrix");
  if (program->projection_matrix_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of projection_matrix");
  }
  program->modelview_matrix_location = glGetUniformLocation(program_id, "modelview_matrix");
  if (program->modelview_matrix_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of modelview_matrix");
  }

  return true; 
}
