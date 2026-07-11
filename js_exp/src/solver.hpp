#ifndef SOLVER_HPP
#define SOLVER_HPP

#pragma once

#include "Constants.hpp"
#include "structs.hpp"
#include <cstdio>
#include <algorithm>
#include "unordered_map"
#include "poly.hpp"


#include "functionspace/GenerativeArt.h"
#include "functionspace/FunctionPool.h"
#include "support/texture.hpp"
#include "algorithm"

class Solver
{
public:
    Solver();
    int tick=0;
    PolyLib polyLib;

     unsigned int palette;


    //-------------------------
    GenerativeArt::Settings settings;
    GenerativeArt ga;

    Texture function_image;
    std::vector<uint8_t> colors;
    std::vector<Texture> textureList;
    std::vector<GLuint> textureIndexList;

    unsigned int lineSize=10;

    Image_statistics image_statistics;

    bool generate_image();
    void fillAllTextures();

    Image_statistics stat;


    glm::vec2 worldSize;

    std::vector<InstanceParticle> particleList;
    std::vector<PointData> pointList;

    std::vector<Constraint> constraintList;
    std::vector<glm::vec2>  constraintListDraw;

    std::unordered_map<GLuint,bool> constraintsLookUp;
    inline GLuint indexConstraint(GLuint x, GLuint y) {
        GLuint tempX;
        if(y<x)
        {
            tempX=y;
            y=x;
            x=tempX;
        }
        return GLuint(y* max_particlesLimit +x);
    }

    GridObject gridObject;

    glm::ivec2 mapCellSize;
    glm::ivec2 mapSize;

    void initParticles(int numParticles, glm::vec2 worldSize_, SimDynamicParameters &simDynParams);

    void ConstraintUpdate(SimDynamicParameters &simParam);
    void mapPointsUpdate();
    void Step(SimDynamicParameters &simParam);
    void ParticleMove(SimDynamicParameters &simParam);



    float randomFloat(float min, float max);
    static float hash2D(int x, int y);
    static float noise2D(float x, float y);
    static float fBm2D(float x, float y, int octaves, float lacunarity, float persistence);
    void RemoveSimpleConstraint(SimDynamicParameters &simParam, bool &resume);

    void initParticlesTest(int numParticles, glm::vec2 worldSize_, SimDynamicParameters &simDynParams);
    void texturesInit();
    float AngleBetweenVectors(glm::vec2 pointAcoord, glm::vec2 pointBcoord);
    void initParticles2(int numParticles, glm::vec2 worldSize_, SimDynamicParameters &simDynParams);
    float sign(float _val);
    void pointCircleMove(glm::vec3 &_out, float _angle);
    void squircle(glm::vec3 &_out, float _angle);
};

#endif // SOLVER_HPP
