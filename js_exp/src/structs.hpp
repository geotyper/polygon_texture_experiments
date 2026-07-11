
#ifndef STRUCTS_HPP
#define STRUCTS_HPP



#pragma once


#include "Constants.hpp"
#include <utility>
#include <stdlib.h>
#include "glm/ext.hpp"
#include "glm/glm.hpp"
#include <glm/vec3.hpp>
#include "vector"
#include "iostream"
#include "tuple"



#define FXPUBLISH

struct VertexPosNormUv {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct VertexPos {
    glm::vec2 pos;
};


struct VertexPosUV {
    glm::vec2 pos;
    glm::vec2 uv;
};

struct Vertex3PosUV {
    glm::vec3 pos;
    glm::vec2 uv;
};

struct InstanceParticle
{
    glm::vec2  instancePos;
    float  instanceRot;
    float instanceScale;
};

struct VertexPosColor {
    glm::vec2 pos;
    glm::vec4 color;
};


struct VertexPosNormUvColor {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 color;
};


struct ObjectInfo
{
    std::vector<unsigned int> pointsMapList;
};

struct TrianglesDrawStruct {
    glm::vec2 pos;
    glm::vec4 color;
};

struct PolygonInfo {
    glm::vec2 center;
    glm::vec2 minboxCoord;
    glm::vec2 maxboxCoord;
    float width;
    float height;
    float area;
    glm::vec2 transmit;

};


struct GraphicProcessor_
{
    struct buffers_{
        struct {
            GLuint vao;
            GLuint vbo;
            GLsizei nvertices;
            unsigned int resolution;
        } grid;
    } buffers;

    struct shaders_{
        GLuint grid;
    } shaders;

    struct uniforms_{
        struct {
            GLint mvp;
            GLint scaleFactor;
        } grid;
    } uniformsall;

    struct MeshBase_{
        GLuint vao;
        GLuint vao_inst;
        GLuint vbo;
        GLuint vbo_inst;
        GLuint ibo;
        GLsizei nvertices;
        GLenum indices_type;
        GLsizei nindices;
        unsigned int resolution;
        GLsizei instanceNum;

        struct {
            GLuint ubo;
            GLuint bindingPoint;
            GLuint blockId;
            GLint  blockSize;
        } uniform;

        GLuint prog;
    };

    MeshBase_ box;
    MeshBase_ circle;
    MeshBase_ circleInCircle;
    MeshBase_ constraints;
    MeshBase_ polygon;
    MeshBase_ polygonGA;
    MeshBase_ framebufferBox;

    struct MeshBaseInstance_{
        GLuint vao;
        GLuint vao_inst;
        GLuint vbo;
        GLuint vbo_inst;
        GLuint ibo;
        GLsizei nvertices;
        GLenum indices_type;
        GLsizei nindices;
        unsigned int resolution;
        GLsizei instanceNum;

        std::vector<uint32_t> indexBuffer;
        std::vector<VertexPos> vertexBuffer;
        std::vector<InstanceParticle> instanceData;

        struct {
            GLuint ubo;
            GLuint bindingPoint;
            GLuint blockId;
            GLint  blockSize;
        } uniform;

        GLuint prog;
    };

    MeshBaseInstance_ circleInstance;
    //Stipples_ boxes;

    struct RenderScreen_
    {
        GLuint framebuffer;
        GLuint textureColorbuffer;
        GLuint rbo;
    } renderscreenQuad;

    struct{
        struct Source_texture_{
            GLuint id;
            int width;
            int height;
        } source_texture;


    } textures;

};



struct uBlock
{
  glm::vec2  uWorldSize;
  glm::vec2  uTranslate;
  float uScale;
};


struct TEventData {
  float wheelDelta;
  float mouseX;
  float mouseY;
  bool bMouseMove;
  bool bRotatePressed;
  bool bTranslatePressed;
  bool bSpacePressed;
};

struct SimDynamicParameters{

/*
    Sim_parameters() {
     tick=0;
     dt=0.35f;
     maxVelocity=1.25f;
     radius=11.0f;
     noiseCoeff=1.0f;
     stiffnessCoeff=0.035;
     dampingCoeff=0.2;
     particlesNum=0;
     constraintsNum=0;
    }
    */
    int tick;
    float dt;
    float maxVelocity;
    float radius;
    float noiseCoeff;
    float stiffnessCoeff;
    float dampingCoeff;
    float initConstraintDist;
    float pointsMoveRange;
    float breakConstraintDist;

    int particlesNum;
    int constraintsNum;
    int pointPlacementMode = 0; // 0: Radial, 1: Rectangular, 2: Noise, 3: Spiral
    float minConstraintDist = 0.0f;
    bool useNoiseDisplacement = false;
    float noiseScale = 0.02f;
    float persistence = 0.5f;
    float lacunarity = 2.0f;
    int octaves = 4;
    glm::vec3 backgroundColor = glm::vec3(0.15f, 0.15f, 0.33f);
    int maxConstraintsPerPoint = 5;
    bool enableSmoothing = false;
    float smoothingTension = 0.5f;
    float smoothingPointsPerUnit = 0.2f;
    float smoothingZoom = 1.0f;
    float generalZoom = 1.0f;
    int subdivisionDepth = 0;
    int subdivisionMode = 0; // 0: Centroid, 1: Skeleton
    bool enablePolygonMerging = false;
    bool removeOverlappingPolygons = false;
    float subdivisionMinArea = 500.0f;
    glm::vec3 constraintColor = glm::vec3(0.75f, 0.90f, 0.75f);
    bool enableFractalSubdivision = false;
    int fractalSubdivisionDepth = 1;
    float fractalSubdivisionMinArea = 500.0f;
    float fractalSubdivisionScale = 0.7f;
    float fractalSubdivisionShift = 5.0f;
    float fractalSubdivisionRotateRange = 30.0f;

    int max_particles;
    int max_constraints;
    int mapCellsize;
    float rateChangePhase;
    float velocityDamping;

    int createConstrParam01=5;
    int createConstrParam02=3;
    int createConstrParam03=3;
    int createConstrParam04=3;
    int createConstrParam05=5;
    int createConstrParam06=3;
    int createConstrParam07=3;

};

struct SimRunParameters{
    bool runSimulation=true;
    bool restartSimulation=false;
    bool nextStep=false;

    bool runRemoveSimpleConstraint=false;

    bool drawPolygons=false;
    bool drawPoints=false;
    bool drawConstraints=false;
    bool drawPolyConstraints=false;
    bool drawNewPoints=false;
    bool drawTriangulatePolygons=false;
    bool drawTriangulatePolygonsOffset=false;
    bool drawTriangulatePolygonsGA=false;
    bool drawTriangulatePolygonsOffsetGA=false;

    int amountGenerateParticlesByTick=27;
    int  maxPolygonPoints=50;

    int  minusSmallestPolygons=0;
    int  minusBiggestPolygons=0;
    float polygonOffest=-6.5;
    bool generateParticles=false;
    bool generatePolygons=false;
    bool generateOffset=false;
    bool generateSweepLine=false;
    bool triangulatePolygons=false;
    bool recolorPolygons=false;

    bool runPolygonPipeline=false;

    GLuint particleTexture;
};



struct PointData
{
    glm::vec2 coord;
    glm::vec2 velocity;
    glm::vec2 force;
    glm::vec2 headTail;
    glm::vec2 prevNext;
    int radius;

    int neightbSize;
    int constructSize;
    int constructSizeMax;
    glm::vec2 phase;

    std::vector<int> neighbors;
};

struct Constraint
{
    bool cactive;
    GLuint id1;
    GLuint id2;
    float clength;
    bool broken;
    float resistance;
    float strength;

    glm::vec2 center;
    int id = 0;
    int test0 = 0, test1 = 0;
    int32_t origLine = -1;
    int32_t attr0 = -1; // used for testing
    bool ignore = false;
    int took = 0;
    int processed = 0;
    int lastDissolveStep = 0;
};


struct simArraySize
{
    int partilclesNum;
    int constraintsNum;
};


struct NeightbData
{
    glm::vec2 dist;
};

struct VoronoiData
{
    glm::ivec4 nb;
    glm::vec4 d;
};


struct PolygonData
{
    PolygonInfo polyInfo;

    // Init point and constraints
    std::vector<PointData> pointList;
    std::vector<Constraint> constraintList;

    //Draw variations
};


class GridObject
{
    public:
        GridObject() {};
        GridObject(int width, int height) : width_(width), height_(height), cells(width * height)
        {};

        inline void set(int width, int height) {
            width_=width;
            height_=height;
            int cellsSize=width * height;
            //cells.resize(cellsSize);

            for(int i=0; i<cellsSize;i++)
            {
                ObjectInfo tempObject;
                tempObject.pointsMapList.clear();
                cells.push_back(tempObject);
            }
        };

        inline void clear() {
            cells.clear();
        };

        inline void clearCells() {
            int cellsSize=width_ * height_;
            for(int i=0; i<cellsSize;i++)
            {
                ObjectInfo tempObject;
                tempObject.pointsMapList.clear();
                cells[i]=tempObject;
            }
        };

        ObjectInfo& at(int row, int column) { return cells[index(row, column)]; }
        ObjectInfo& at(glm::ivec2 pos) { return cells[indexPos(pos)]; }
        ObjectInfo& at(int indexCR) { return cells[indexCR]; }

   private:
        inline int index(int x, int y) {
            return int(y* width_ +x);
        }

        int indexPos(glm::ivec2 pos) {
            return int(pos.y * width_  + pos.x);
        }

        int width_;
        int height_;
        std::vector<ObjectInfo> cells;
};





struct Image_statistics{

    // statistics
    uint32_t acc_r = 0, acc_g = 0, acc_b = 0;
    uint8_t  min_r = 255, min_g = 255, min_b = 255;
    uint8_t  max_r = 0, max_g = 0, max_b = 0;

    uint32_t white = 0,
             black = 0;

    std::string function;
    int function_seed;
};






#endif //
