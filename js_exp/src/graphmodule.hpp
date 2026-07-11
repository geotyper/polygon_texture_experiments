#ifndef GRAPHMODULE_HPP
#define GRAPHMODULE_HPP

#pragma once


#include "glm/ext.hpp"
#include "glm/glm.hpp"

#include "Constants.hpp"
#include "support/shaderutils.hpp"
#include "support/meshData.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#ifdef USE_JS
#include <emscripten.h>
#endif

#include "solver.hpp"
#include <SDL.h>
//#include <SDL_opengles2.h>

static SDL_Window* screen;
static SDL_GLContext gContext;

class GraphModule
{
public:

  //  const char* glsl_version = "#version 100";
    const char* glsl_version = "#version 300 es";
    unsigned int fxrand;

    ShaderUtils shaderutils;
    GraphicProcessor_ graphicProcessor;
    MeshData mesh;
    SimRunParameters simRunParam;
    SimDynamicParameters simDynParam;

    //SDL_Window* screen;
    //SDL_GLContext gContext;

    //------------------------Main block Functions--------------------------------------------------
    Solver solver;
    GraphModule();

    bool InitSDL();
    SDL_Event event;
    bool InitImGui();
    bool InitShaders();

    bool RunLoop();
    bool Step();

    bool initRun=true;

    //---------------------------------------------------------------------
    int mainScreenWidth= 1350;
    int mainScreenHeight=1000;

    uint width;
    uint height;

    float f = 0.0f;
    int counter = 0;

   // const uint resolutionCircle=resolutionCircle_init;
    float sx=1.0f;
    float sy=1.0f;

    //int cone_resolution=64;

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static void  main_loop(void* arg);
    double prevTime=-1;
    double timeSincePrevious=0;

    vector<vector<glm::vec2>> polygonsList;
    vector<vector<glm::vec2>> polygonsListOffset;

    bool InitframeBuffers();

    void draw_mesh(GraphicProcessor_ ::MeshBase_ &meshIn, glm::vec3 translate, const glm::mat4x4 &mvp, const glm::vec4 &color, bool bFill);
    void RenderMesh(GraphicProcessor_::MeshBase_ &meshIn, glm::vec3 translate, const glm::mat4x4 &view, const glm::mat4x4 &viewProj);
    void UniformBlocksCreate();

    //-------------------Render Block--------------------------
    void draw_ImGui();
    void draw_Constraints();
    void draw_Polygons();
    void draw_Points(std::vector<InstanceParticle> &pointList);
    void draw_Constraints(std::vector<glm::vec2> &constraintListDraw);
    void draw_Polygons(const vector<vector<TrianglesDrawStruct> > &triangulatePolygons);
    void draw_Polygons(vector<vector<TrianglesDrawStruct> > &triangulatePolygons_in, vector<vector<unsigned int> > &triangulatePolygonsIndex_in);
    void draw_PolygonsSort(vector<vector<TrianglesDrawStruct> > &triangulatePolygons_in, vector<vector<unsigned int> > &triangulatePolygonsIndex_in);
    void draw_PolygonsUV(vector<vector<VertexPosUV> > &triangulatePolygons_in, vector<vector<unsigned int> > &triangulatePolygonsIndex_in);
    bool ProgressStep();

    //-------------------Support Block--------------------------
    void squircle(glm::vec3 &_out, float _angle);
    float sign(float _val);
    void pointCircleMove(glm::vec3 &_out, float _angle);

    void Box_create();
    void CircleCreateSegment(GraphicProcessor_::MeshBase_ &meshIn, float radius, int numberSegments);
    void CircleInCirclrCreate(GraphicProcessor_::MeshBase_ &meshIn, float radius, int numberSegments);
    void CircleCreateInstance(GraphicProcessor_::MeshBaseInstance_ &meshIn, float radius, int numberSegments, std::vector<InstanceParticle> &particleList_);
    void Constraint_create();
    void polygonCreate();
    void createPolygonGA();

    void update_mesh_geometry(GraphicProcessor_ ::MeshBase_ &meshIn, MeshData &mesh);

    GLuint texture_new();
    GLuint texture32F_new();
    GLuint setup_texture_emptyWHRGBA32F(int widths_, int heights_);

    bool FrameBufferDrawStep();
    void FramebufferBox_create();
    void FramebufferBoxElement();

    void UniformBlocksUpdate();



};

#endif // GRAPHMODULE_HPP
