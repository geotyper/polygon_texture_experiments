#pragma once

#define GLM_FORCE_PURE
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <exception>
//#include "log.h"



struct TriangleVertices {
    unsigned int one;
    unsigned int two;
    unsigned int three;
    TriangleVertices(){};
    TriangleVertices ( int one,int two, int three ) : one(one), two(two), three(three){};
};

class MeshData {
    public:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec4> colors;
        std::vector<TriangleVertices> triangelVertices;

        std::vector<unsigned int> lineIndices;

        int numberVertices() { return vertices.size();};
        int numberIndices()  { return 3*triangelVertices.size();};

        void ClearAll()
        {
            vertices.clear();
            normals.clear();
            triangelVertices.clear();
        }

        void removeUnusedVertices ();
        void transformMesh(std::function<glm::vec3 (glm::vec3 &)> transformVerticeFn);
        
};
