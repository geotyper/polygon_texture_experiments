#ifndef POLY_HPP
#define POLY_HPP


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define TINYCOLORMAP_WITH_GLM
#include "support/tinycolormap.hpp"

//#include "poly/ofxPolygonDetector.h"
//#include "poly/PolyDetector.h"
#include "structs.hpp"
#include "unordered_map"
#include "poly/SweepLine/sweepline.hpp"
#include <algorithm>
#include "poly/polygoncreator.hpp"
#include "poly/clipper.hpp"

//#include "poly/Polygon.cpp"

using namespace std;

class PolyLib
{
public:
    PolyLib();


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

    ArrangementBuilder arr;

    std::vector<PointData> pointListTemp;//=pointList;
    std::vector<Constraint> constraintListTemp;//=constraintList;
    std::vector<InstanceParticle> particleListTemp;

    std::unordered_map<GLuint,std::pair<int,int>> constraintsLookUp;
    std::unordered_map<GLuint,std::pair<int,int>> constraintsLookUpOldIndex;

    std::vector<glm::vec2>  constraintListDraw;

    std::vector<Constraint> constraintList;

    vector<vector<glm::vec2>> testForPolygons(std::vector<PointData>& pointList, std::vector<Constraint>& constraintList, const SimDynamicParameters& simParam);

    std::vector<sweepline::intersection_t> result;

    vector<vector<TrianglesDrawStruct>> triangles_draw_vertex;
    vector<vector<unsigned int>> triangles_draw_index;

    vector<vector<TrianglesDrawStruct>> trianglesOffset_draw_vertex;
    vector<vector<unsigned int>> trianglesOffset_draw_index;


    void FindIntersections(std::vector<PointData> &pointList, std::vector<Constraint> &constraintList);
    void DCEL(std::vector<PointData> &pointList, std::vector<Constraint> &constraintList_);
    vector<vector<glm::vec2> > offsetPolygons(float offest);

    //vector<vector<TrianglesDrawStruct> > TriangulationPolygon(vector<vector<glm::vec2> > &polygonList_in);;
    vector<vector<TrianglesDrawStruct> > TriangulationPolygon(vector<vector<glm::vec2> > &polygonList_in, bool offset, int palette);
    void Triangulation(vector<vector<glm::vec2> > &polygonList_in, int palette);
    void RecolorPolygons(int palette);
};

#endif // POLY_HPP
