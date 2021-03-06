#ifndef KRR_graphics_types_h_
#define KRR_graphics_types_h_

#include <glad/glad.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  GLfloat x;
  GLfloat y;
  GLfloat w;
  GLfloat h;
} RECT;

typedef struct
{
  GLfloat w;
  GLfloat h;
} SIZE;

typedef struct
{
  GLfloat s;
  GLfloat t;
} TEXCOORD2D;

typedef struct
{
  GLfloat x;
  GLfloat y;
  GLfloat z;
} NORMAL;

typedef struct
{
  GLfloat x;
  GLfloat y;
} VERTEXPOS2D;

typedef struct
{
  GLfloat x;
  GLfloat y;
  GLfloat z;
} VERTEXPOS3D;

typedef struct
{
  VERTEXPOS2D position;
  TEXCOORD2D texcoord;
} VERTEXTEX2D;

typedef struct
{
  VERTEXPOS3D position;
  TEXCOORD2D texcoord;
} VERTEXTEX3D;

typedef struct
{
  VERTEXPOS3D position;
  NORMAL normal;
} VERTEXNORM3D;

typedef struct
{
  VERTEXPOS3D position;
  TEXCOORD2D texcoord;
  NORMAL normal;
} VERTEXTEXNORM3D;

typedef struct
{
  GLfloat r;
  GLfloat g;
  GLfloat b;
} COLOR3F;

typedef struct
{
  GLfloat r;
  GLfloat g;
  GLfloat b;
  GLfloat a;
} COLOR32;

typedef COLOR32 COLOR4F;

typedef struct
{
  VERTEXPOS2D pos;
  COLOR32 color;
} MULTCOLOR2D;

typedef struct
{
  VERTEXPOS3D pos;
  COLOR3F color;
  float attenuation_factor;
} LIGHT;

typedef struct
{
  GLfloat shine_dumper;
  GLfloat reflectvity;
} MATERIAL;

#ifdef __cplusplus
}
#endif

#endif
