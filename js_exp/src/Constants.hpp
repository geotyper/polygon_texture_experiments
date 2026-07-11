#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP


#pragma once
#include <GL/glew.h>

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "glm/ext.hpp"
#include "glm/glm.hpp"

#include "string"

//#include "../external/CLI11/CLI11.hpp"
//#include "../external/png++/png.hpp"


#define PI  3.14159215
#define PHI 1.61803398

//Particles
#define particlesInitSize    30*30
#define max_particlesLimit   90*90//30*50

#define max_ConstraintLimit   max_particlesLimit*max_particlesLimit//30*50

#define particleRadius_init 2.0f
#define maxVelocity_init 1.5f

#define worldX 2000.0f
#define worldY 2000.0f
#define mapDivider 50

#define cellMaxParticles 50
#define neightbMaxParticles 800
#define neightbCalcMaxParticles 800
#define constraintsMaxParticles 300

#define screen_coeff 10.0f

//Circle init
#define resolutionCircle_init 64.0f

/* Maximum size per shader file (with include). 64 Ko */
#define MAX_SHADER_BUFFERSIZE  (128u*1024u)

#ifdef USE_JS
static const std::string  path_string="";
#else
static const std::string  path_string="../";
#endif

static const std::string  path_shader=path_string+"shaders/compute/";

//static const std::string  source_image_file_name="../images/hitl.jpg";

/* OpenGL debug macro */
#ifdef NDEBUG
# define CHECKGLERROR()
#else
# define CHECKGLERROR()    CheckGLError(__FILE__, __LINE__, "", true)
#endif

static
const char* GetErrorString(GLenum err) {
#define STRINGIFY(x) #x
  switch (err)
  {
    // [GetError]
    case GL_NO_ERROR:
      return STRINGIFY(GL_NO_ERROR);

    case GL_INVALID_ENUM:
      return STRINGIFY(GL_INVALID_ENUM);

    case GL_INVALID_VALUE:
      return STRINGIFY(GL_INVALID_VALUE);

    case GL_INVALID_OPERATION:
      return STRINGIFY(GL_INVALID_OPERATION);

    case GL_STACK_OVERFLOW:
      return STRINGIFY(GL_STACK_OVERFLOW);

    case GL_STACK_UNDERFLOW:
      return STRINGIFY(GL_STACK_UNDERFLOW);

    case GL_OUT_OF_MEMORY:
      return STRINGIFY(GL_OUT_OF_MEMORY);

    default:
      return "GetErrorString : Unknown constant";
  }
#undef STRINGIFY
}

inline void CheckGLError(const char* file, const int line, const char* errMsg, bool bExitOnFail) {
  GLenum err = glGetError();

  if (err != GL_NO_ERROR) {
    fprintf(stderr,
            "OpenGL error @ \"%s\" [%d] : %s [%s].\n",
            file, line, errMsg, GetErrorString(err));

    if (bExitOnFail) {
      exit(EXIT_FAILURE);
    }
  }
}


#endif // CONSTANTS_HPP
