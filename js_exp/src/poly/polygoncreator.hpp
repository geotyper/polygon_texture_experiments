#ifndef POLYGONCREATOR_HPP
#define POLYGONCREATOR_HPP


#include <vector>
#include "../structs.hpp"
#include "polypartition.h"
#include "delaunator.hpp"

#include "clipper.hpp"

#define TINYCOLORMAP_WITH_GLM
#include "tinycolormap.hpp"

using namespace std;

inline bool sortbysecdesc(const pair<int,float> &a,
                   const pair<int,float> &b)
{
       return a.second>b.second;
}


class ArrangementBuilder {

public:
    //const PolygonList* getPolygons(const PointList& points);
    const std::vector<std::vector<glm::vec2>> getPolygonsVector(std::vector<PointData> &pointList, std::vector<Constraint> &constraintList);
    const std::vector<std::vector<glm::vec2>> offestPolygons(float offest);
    vector<vector<TrianglesDrawStruct> > triangulatePolygons(vector<vector<glm::vec2> > &polygonList_in, int num_colors, bool offset, int palette, bool mergePolygons);
    vector<vector<TrianglesDrawStruct> > triangulatePolygonsVoronoi(vector<vector<glm::vec2> > &polygonList_in, int num_colors);

    float minPolygonArea = 18.0f;

    vector<vector<glm::vec2>> polygons;
    vector<vector<glm::vec2>> polygons_scale;
    vector<vector<glm::vec2>> polygons_draw;

    std::vector<std::vector<glm::vec2>> allPolygonsOffest;
   // std::vector<std::vector<glm::vec2>> triangulatePolygons;
    vector<vector<TrianglesDrawStruct>> triangles_draw_vertex;
    vector<vector<unsigned int>> triangles_draw_index;

    std::vector<PolygonData> polygonData;
    std::vector<PolygonData> polygonDataOffset;

    std::vector<int> polygonColorIndices;
    std::vector<int> polygonColorIndicesOffset;

    vector<vector<VertexPosUV>> triangles_draw_vertexUV;
    vector<vector<VertexPosUV>> triangles_draw_vertexOffsetUV;

    vector<pair<int,float>> areaList;
    vector<pair<int,float>> areaListOffset;


    void calcPolygon(PolygonInfo &polygonInfo, std::vector<glm::vec2> &polygon);
    std::vector<glm::vec4> generateColorList(int palette, int num_colors);


private:


};

#endif // POLYGONCREATOR_HPP
