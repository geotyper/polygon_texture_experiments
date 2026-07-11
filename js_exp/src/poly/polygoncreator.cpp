#include "polygoncreator.hpp"


#include "colormap/colormap.h"

#include <iostream>
#include <math.h>
#include <numeric>
#include <algorithm>

#define EPSILON 1E-5

const std::vector<std::vector<glm::vec2> > ArrangementBuilder::getPolygonsVector(std::vector<PointData> &pointList, std::vector<Constraint> &constraintList)
{

    polygons.clear();


    return polygons;

}


void  ArrangementBuilder::calcPolygon(PolygonInfo& polygonInfo, std::vector<glm::vec2>& polygon)
{

    float maxX=0;
    float maxY=0;
    float minX=100000;
    float minY=100000;
    for(auto point:polygon)
    {
        if(maxX<point.x)
            maxX=point.x;
        if(maxY<point.y)
            maxY=point.y;

        if(minX>point.x)
            minX=point.x;
        if(minY>point.y)
            minY=point.y;

        polygonInfo.maxboxCoord=glm::vec2(maxX,maxY);
        polygonInfo.minboxCoord=glm::vec2(minX,minY);

        polygonInfo.width=maxX-minX;
        polygonInfo.height=maxY-minY;
        polygonInfo.center=glm::vec2((maxX-minX)/2.0,(maxY-minY)/2.0);
        polygonInfo.transmit=glm::vec2(minX,minY);

    }
}


using namespace ClipperLib;
const std::vector<std::vector<glm::vec2> > ArrangementBuilder::offestPolygons(float offest)
{
    allPolygonsOffest.clear();
    std::cout << "[Offset] offestPolygons called. input polygons.size() = " << polygons.size() << ", offset = " << offest << std::endl;

    int skippedSmall = 0;
    int skippedEmpty = 0;

    for(auto polygon:polygons)
    {
        Path subj;
        Paths solution;
        for(auto point:polygon)
        {
            subj <<
            IntPoint(point.x,point.y);
        }

        double areaPoly=Area(subj);

        if(abs(areaPoly)<minPolygonArea)
        {
            skippedSmall++;
            continue;
        }

        // Dynamically scale the offset so it never exceeds 35% of the polygon's characteristic width.
        // This keeps smaller/subdivided polygons from shrinking too abruptly or disappearing.
        float charWidth = sqrt(abs(areaPoly));
        float maxAllowedOffset = charWidth * 0.35f;
        float actualOffset = offest;
        if (std::abs(offest) > maxAllowedOffset) {
            actualOffset = (offest < 0.0f) ? -maxAllowedOffset : maxAllowedOffset;
        }

        ClipperOffset co;
        co.AddPath(subj, jtSquare, etClosedPolygon);
        co.Execute(solution, actualOffset);

        if (solution.empty())
        {
            skippedEmpty++;
            continue;
        }

        std::vector<glm::vec2> tempPolygonOffset;

        for(auto point:solution[0])
        {
            tempPolygonOffset.push_back(glm::vec2(point.X,point.Y));
        }
        tempPolygonOffset.push_back(glm::vec2(solution[0][0].X,solution[0][0].Y));

        allPolygonsOffest.push_back(tempPolygonOffset);
    }

    std::cout << "[Offset] Finished offestPolygons. Generated " << allPolygonsOffest.size() 
              << " polygons. (Skipped: small=" << skippedSmall << ", empty=" << skippedEmpty << ")" << std::endl;

    return allPolygonsOffest;

}

vector<vector<TrianglesDrawStruct>> ArrangementBuilder::triangulatePolygons(vector<vector<glm::vec2>>& polygonList_in,int num_colors, bool offset, int palette, bool mergePolygons, bool removeOverlappingPolygons)
{
    std::cout << "[Triangulate] triangulatePolygons called with " << polygonList_in.size() << " polygons (offset=" << (offset ? "true" : "false") << ", merge=" << (mergePolygons ? "true" : "false") << ", removeOverlap=" << (removeOverlappingPolygons ? "true" : "false") << ")." << std::endl;

    if (removeOverlappingPolygons) {
        struct PolyInfo {
            int originalIndex;
            double area;
            float minX, maxX, minY, maxY;
            bool active;
        };

        std::vector<PolyInfo> infos(polygonList_in.size());
        for (size_t i = 0; i < polygonList_in.size(); ++i) {
            infos[i].originalIndex = i;
            infos[i].active = true;

            if (polygonList_in[i].size() < 3) {
                infos[i].active = false;
                continue;
            }

            Path path;
            float minX = polygonList_in[i][0].x;
            float maxX = minX;
            float minY = polygonList_in[i][0].y;
            float maxY = minY;

            for (auto pt : polygonList_in[i]) {
                path << IntPoint(pt.x, pt.y);
                if (pt.x < minX) minX = pt.x;
                if (pt.x > maxX) maxX = pt.x;
                if (pt.y < minY) minY = pt.y;
                if (pt.y > maxY) maxY = pt.y;
            }

            infos[i].area = abs(Area(path));
            infos[i].minX = minX;
            infos[i].maxX = maxX;
            infos[i].minY = minY;
            infos[i].maxY = maxY;
        }

        std::vector<int> sortedIndices(infos.size());
        std::iota(sortedIndices.begin(), sortedIndices.end(), 0);
        std::sort(sortedIndices.begin(), sortedIndices.end(), [&](int a, int b) {
            return infos[a].area > infos[b].area;
        });

        for (size_t i = 0; i < sortedIndices.size(); ++i) {
            int idxA = sortedIndices[i];
            if (!infos[idxA].active) continue;

            for (size_t j = i + 1; j < sortedIndices.size(); ++j) {
                int idxB = sortedIndices[j];
                if (!infos[idxB].active) continue;

                // Bounding box check
                if (infos[idxA].maxX < infos[idxB].minX || infos[idxB].maxX < infos[idxA].minX ||
                    infos[idxA].maxY < infos[idxB].minY || infos[idxB].maxY < infos[idxA].minY) {
                    continue; // No overlap possible
                }

                // Clipper intersection check
                Clipper c;
                Path pathA, pathB;
                for (auto pt : polygonList_in[idxA]) pathA << IntPoint(pt.x, pt.y);
                for (auto pt : polygonList_in[idxB]) pathB << IntPoint(pt.x, pt.y);

                c.AddPath(pathA, ptSubject, true);
                c.AddPath(pathB, ptClip, true);

                Paths inter;
                c.Execute(ctIntersection, inter);

                double interArea = 0;
                for (const auto& p : inter) {
                    interArea += abs(Area(p));
                }

                // If they overlap significantly (e.g. intersection area > 10% of the smaller polygon's area, or > 5.0)
                if (interArea > 5.0 && interArea > 0.05 * infos[idxB].area) {
                    infos[idxB].active = false; // Deactivate smaller polygon
                }
            }
        }

        std::vector<std::vector<glm::vec2>> filtered;
        for (size_t i = 0; i < polygonList_in.size(); ++i) {
            if (infos[i].active) {
                filtered.push_back(polygonList_in[i]);
            }
        }
        polygonList_in = filtered;
    }

    int counter=0;
    triangles_draw_vertex.clear();
    triangles_draw_index.clear();


    if(offset==true)
    {
       polygonDataOffset.clear();
       areaListOffset.clear();
       triangles_draw_vertexOffsetUV.clear();
       polygonColorIndicesOffset.clear();
    }
    else
    {
       polygonData.clear();
       areaList.clear();
       triangles_draw_vertexUV.clear();
       polygonColorIndices.clear();
    }

    vector<glm::vec4> colorList = generateColorList(palette, num_colors);

    if (mergePolygons)
    {
        // 1. Group polygon indices by randomized color
        std::vector<std::vector<int>> colorGroups(num_colors + 1);
        for (int ip = 0; ip < (int)polygonList_in.size(); ++ip) {
            if (polygonList_in[ip].size() < 3) continue;

            Path subj;
            for (auto point : polygonList_in[ip]) {
                subj << IntPoint(point.x, point.y);
            }
            double areaPoly = Area(subj);
            if (abs(areaPoly) < minPolygonArea) continue;

            int rnd_color = rand() % num_colors + 1;
            colorGroups[rnd_color].push_back(ip);
        }

        // 2. For each color, union all polygons in that group
        for (int cIdx = 1; cIdx <= num_colors; ++cIdx) {
            if (colorGroups[cIdx].empty()) continue;

            Clipper c;
            for (int ip : colorGroups[cIdx]) {
                Path path;
                for (auto point : polygonList_in[ip]) {
                    path << IntPoint(point.x, point.y);
                }
                c.AddPath(path, ptSubject, true);
            }

            PolyTree solution;
            c.Execute(ctUnion, solution, pftNonZero, pftNonZero);

            PolyNode* node = solution.GetFirst();
            while (node) {
                if (!node->IsHole() && !node->Contour.empty()) {
                    TPPLPolyList localPolys;

                    TPPLPoly outerPoly;
                    outerPoly.Init(node->Contour.size());
                    for (size_t i = 0; i < node->Contour.size(); ++i) {
                        outerPoly[i].x = node->Contour[i].X;
                        outerPoly[i].y = node->Contour[i].Y;
                    }
                    outerPoly.SetOrientation(TPPLOrientation::TPPL_ORIENTATION_CCW);
                    outerPoly.SetHole(false);
                    localPolys.push_back(outerPoly);

                    for (auto* child : node->Childs) {
                        if (child->IsHole() && !child->Contour.empty()) {
                            TPPLPoly holePoly;
                            holePoly.Init(child->Contour.size());
                            for (size_t i = 0; i < child->Contour.size(); ++i) {
                                holePoly[i].x = child->Contour[i].X;
                                holePoly[i].y = child->Contour[i].Y;
                            }
                            holePoly.SetOrientation(TPPLOrientation::TPPL_ORIENTATION_CW);
                            holePoly.SetHole(true);
                            localPolys.push_back(holePoly);
                        }
                    }

                    TPPLPartition pp;
                    TPPLPolyList polys;
                    int res = pp.Triangulate_EC(&localPolys, &polys);
                    if (res == 0) {
                        res = pp.Triangulate_MONO(&localPolys, &polys);
                    }

                    if (res == 1) {
                        PolygonInfo parentPolyInfo;
                        std::vector<glm::vec2> parentVertices;
                        for (size_t i = 0; i < node->Contour.size(); ++i) {
                            parentVertices.push_back(glm::vec2(node->Contour[i].X, node->Contour[i].Y));
                        }
                        calcPolygon(parentPolyInfo, parentVertices);

                        float maxDim = std::max(parentPolyInfo.width, parentPolyInfo.height);
                        float denom = maxDim > 0.0f ? maxDim : 1.0f;
                        float centerX = parentPolyInfo.minboxCoord.x + parentPolyInfo.width * 0.5f;
                        float centerY = parentPolyInfo.minboxCoord.y + parentPolyInfo.height * 0.5f;

                        for (auto iter = polys.begin(); iter != polys.end(); iter++) {
                            iter->SetOrientation(TPPLOrientation::TPPL_ORIENTATION_CW);

                            std::vector<TrianglesDrawStruct> tempVertex;
                            std::vector<unsigned int> tempIndex;
                            std::vector<VertexPosUV> tempVertexUV;

                            PolygonData tempPolygon;
                            tempPolygon.polyInfo = parentPolyInfo;

                            Path subj;
                            for (int i = 0; i < iter->GetNumPoints(); i++) {
                                TrianglesDrawStruct tempStructTriangle;
                                tempStructTriangle.pos = glm::vec2(iter->GetPoint(i).x, iter->GetPoint(i).y);
                                tempStructTriangle.color = colorList[cIdx];
                                tempVertex.push_back(tempStructTriangle);
                                tempIndex.push_back(i);

                                subj << IntPoint(iter->GetPoint(i).x, iter->GetPoint(i).y);

                                VertexPosUV tempPosUV;
                                tempPosUV.pos = glm::vec2(iter->GetPoint(i).x, iter->GetPoint(i).y);
                                tempPosUV.uv.x = 0.5f + (tempPosUV.pos.x - centerX) / denom;
                                tempPosUV.uv.y = 0.5f + (tempPosUV.pos.y - centerY) / denom;
                                tempVertexUV.push_back(tempPosUV);
                            }

                            double areaTri = Area(subj);

                            triangles_draw_vertex.push_back(tempVertex);
                            triangles_draw_index.push_back(tempIndex);

                            if (offset == true) {
                                areaListOffset.push_back({counter, abs(areaTri)});
                                triangles_draw_vertexOffsetUV.push_back(tempVertexUV);
                                polygonDataOffset.push_back(tempPolygon);
                                polygonColorIndicesOffset.push_back(cIdx);
                            } else {
                                areaList.push_back({counter, abs(areaTri)});
                                triangles_draw_vertexUV.push_back(tempVertexUV);
                                polygonData.push_back(tempPolygon);
                                polygonColorIndices.push_back(cIdx);
                            }
                            counter++;
                        }
                    }
                }
                node = node->GetNext();
            }
        }
    }
    else
    {
        for(int ip=0;ip<polygonList_in.size();ip++)
        {
            if(polygonList_in[ip].size()<3)
                continue;

            Path path;
            for(auto point:polygonList_in[ip])
            {
                path << IntPoint(point.x,point.y);
            }

            double areaPoly=Area(path);
            if(abs(areaPoly)<minPolygonArea)
                continue;

            int rnd_color=rand() % num_colors + 1;

            Clipper c;
            c.AddPath(path, ptSubject, true);

            PolyTree solution;
            c.Execute(ctUnion, solution, pftNonZero, pftNonZero);

            PolyNode* node = solution.GetFirst();
            while (node) {
                if (!node->IsHole() && !node->Contour.empty()) {
                    TPPLPolyList localPolys;

                    TPPLPoly outerPoly;
                    outerPoly.Init(node->Contour.size());
                    for (size_t i = 0; i < node->Contour.size(); ++i) {
                        outerPoly[i].x = node->Contour[i].X;
                        outerPoly[i].y = node->Contour[i].Y;
                    }
                    outerPoly.SetOrientation(TPPLOrientation::TPPL_ORIENTATION_CCW);
                    outerPoly.SetHole(false);
                    localPolys.push_back(outerPoly);

                    for (auto* child : node->Childs) {
                        if (child->IsHole() && !child->Contour.empty()) {
                            TPPLPoly holePoly;
                            holePoly.Init(child->Contour.size());
                            for (size_t i = 0; i < child->Contour.size(); ++i) {
                                holePoly[i].x = child->Contour[i].X;
                                holePoly[i].y = child->Contour[i].Y;
                            }
                            holePoly.SetOrientation(TPPLOrientation::TPPL_ORIENTATION_CW);
                            holePoly.SetHole(true);
                            localPolys.push_back(holePoly);
                        }
                    }

                    TPPLPartition pp;
                    TPPLPolyList polys;
                    int res = pp.Triangulate_EC(&localPolys, &polys);
                    if (res == 0) {
                        res = pp.Triangulate_MONO(&localPolys, &polys);
                    }

                    if (res == 1) {
                        PolygonInfo parentPolyInfo;
                        calcPolygon(parentPolyInfo, polygonList_in[ip]);

                        float maxDim = std::max(parentPolyInfo.width, parentPolyInfo.height);
                        float denom = maxDim > 0.0f ? maxDim : 1.0f;
                        float centerX = parentPolyInfo.minboxCoord.x + parentPolyInfo.width * 0.5f;
                        float centerY = parentPolyInfo.minboxCoord.y + parentPolyInfo.height * 0.5f;

                        for (auto iter = polys.begin(); iter != polys.end(); iter++) {
                            iter->SetOrientation(TPPLOrientation::TPPL_ORIENTATION_CW);

                            std::vector<TrianglesDrawStruct> tempVertex;
                            std::vector<unsigned int> tempIndex;
                            std::vector<VertexPosUV> tempVertexUV;

                            PolygonData tempPolygon;
                            tempPolygon.polyInfo = parentPolyInfo;

                            Path subj;
                            for (int i = 0; i < iter->GetNumPoints(); i++) {
                                TrianglesDrawStruct tempStructTriangle;
                                tempStructTriangle.pos = glm::vec2(iter->GetPoint(i).x, iter->GetPoint(i).y);
                                tempStructTriangle.color = colorList[rnd_color];
                                tempVertex.push_back(tempStructTriangle);
                                tempIndex.push_back(i);

                                subj << IntPoint(iter->GetPoint(i).x, iter->GetPoint(i).y);

                                VertexPosUV tempPosUV;
                                tempPosUV.pos = glm::vec2(iter->GetPoint(i).x, iter->GetPoint(i).y);
                                tempPosUV.uv.x = 0.5f + (tempPosUV.pos.x - centerX) / denom;
                                tempPosUV.uv.y = 0.5f + (tempPosUV.pos.y - centerY) / denom;
                                tempVertexUV.push_back(tempPosUV);
                            }

                            double areaTri = Area(subj);

                            triangles_draw_vertex.push_back(tempVertex);
                            triangles_draw_index.push_back(tempIndex);

                            if (offset == true) {
                                areaListOffset.push_back({counter, abs(areaTri)});
                                triangles_draw_vertexOffsetUV.push_back(tempVertexUV);
                                polygonDataOffset.push_back(tempPolygon);
                                polygonColorIndicesOffset.push_back(rnd_color);
                            } else {
                                areaList.push_back({counter, abs(areaTri)});
                                triangles_draw_vertexUV.push_back(tempVertexUV);
                                polygonData.push_back(tempPolygon);
                                polygonColorIndices.push_back(rnd_color);
                            }
                            counter++;
                        }
                    }
                }
                node = node->GetNext();
            }
        }
    }

    if(offset==true)
    {
       sort(areaListOffset.begin(),areaListOffset.end(),sortbysecdesc);
    }
    else
    {
       sort(areaList.begin(),areaList.end(),sortbysecdesc);
    }

    std::cout << "[Triangulate] Successfully generated " << (offset ? areaListOffset.size() : areaList.size()) << " triangulated shapes." << std::endl;

    return triangles_draw_vertex;
}


vector<vector<TrianglesDrawStruct>> ArrangementBuilder::triangulatePolygonsVoronoi(vector<vector<glm::vec2>>& polygonList_in,int num_colors)
{
    triangles_draw_vertex.clear();
    triangles_draw_index.clear();

    vector<glm::vec4> colorList;
    for(int ic=1; ic<=num_colors+1;++ic)
    {
        colorList.push_back(glm::vec4(tinycolormap::GetColor(float(ic)/num_colors,tinycolormap::ColormapType::Viridis).ConvertToGLM(),1.0));
    }

    for(int ip=0;ip<polygonList_in.size();ip++)
    {

        if(polygonList_in[ip].size()==0)
                continue;
        float area=0;
        int rnd_color=rand() % num_colors + 1;
        glm::vec4 tempColor=colorList[rnd_color];
/*
        Path subj;
        for(auto point:allPolygonsOffest[ip])
        {
            subj <<
            IntPoint(point.x,point.y);
        }

        double areaPoly=Area(subj);

        if(areaPoly<18.0)
            continue;
*/
        std::vector<double> coords;

        for (int i = 0; i < polygonList_in[ip].size(); i++) {
            coords.push_back( polygonList_in[ip][i].x);
            coords.push_back( polygonList_in[ip][i].y);
        }

        //triangulation happens here
         delaunator::Delaunator d(coords);

         vector<TrianglesDrawStruct> tempVertex;
         vector<unsigned int> tempIndex;
         for(std::size_t i = 0; i < d.triangles.size(); i+=3)
         {
             /*
                 printf(
                     "Triangle points: [[%f, %f], [%f, %f], [%f, %f]]\n",
                     d.coords[2 * d.triangles[i]],        //tx0
                     d.coords[2 * d.triangles[i] + 1],    //ty0
                     d.coords[2 * d.triangles[i + 1]],    //tx1
                     d.coords[2 * d.triangles[i + 1] + 1],//ty1
                     d.coords[2 * d.triangles[i + 2]],    //tx2
                     d.coords[2 * d.triangles[i + 2] + 1] //ty2
                 );
                 */

                 TrianglesDrawStruct tempStructTriangle1;
                 tempStructTriangle1.pos=glm::vec2(d.coords[2 * d.triangles[i]],d.coords[2 * d.triangles[i] + 1]);
                 tempStructTriangle1.color=tempColor;
                 tempVertex.push_back(tempStructTriangle1);
                 tempIndex.push_back(d.triangles[i]);

                 TrianglesDrawStruct tempStructTriangle2;
                 tempStructTriangle2.pos=glm::vec2(d.coords[2 * d.triangles[i + 1]],d.coords[2 * d.triangles[i + 1] + 1]);
                 tempStructTriangle2.color=tempColor;
                 tempVertex.push_back(tempStructTriangle2);
                 tempIndex.push_back(d.triangles[i+1]);

                 TrianglesDrawStruct tempStructTriangle3;
                 tempStructTriangle3.pos=glm::vec2(d.coords[2 * d.triangles[i + 2]],d.coords[2 * d.triangles[i + 2] + 1]);
                 tempStructTriangle3.color=tempColor;
                 tempVertex.push_back(tempStructTriangle3);
                 tempIndex.push_back(d.triangles[i+2]);

             }

     triangles_draw_vertex.push_back(tempVertex);
     triangles_draw_index.push_back(tempIndex);

    }

    return triangles_draw_vertex;

}

std::vector<glm::vec4> ArrangementBuilder::generateColorList(int palette, int num_colors)
{
    std::vector<glm::vec4> colorList;
    std::vector<std::vector<glm::vec4>> artisticPalettes = {
        // Palette 0: Kintsugi (Ink & Gold)
        {
            glm::vec4(0.08f, 0.08f, 0.09f, 1.0f),    // Charcoal Ink
            glm::vec4(0.20f, 0.20f, 0.22f, 1.0f),    // Dark Stone
            glm::vec4(0.92f, 0.88f, 0.82f, 1.0f),    // Ivory Paper
            glm::vec4(0.85f, 0.62f, 0.15f, 1.0f),    // Liquid Gold (Accent)
            glm::vec4(0.50f, 0.48f, 0.45f, 1.0f),    // Muted Warm Grey
            glm::vec4(0.96f, 0.94f, 0.90f, 1.0f)     // Bone White
        },
        // Palette 1: Blueprint (Cobalt & Technical)
        {
            glm::vec4(0.05f, 0.08f, 0.15f, 1.0f),    // Deep Navy
            glm::vec4(0.12f, 0.22f, 0.45f, 1.0f),    // Cobalt Blue
            glm::vec4(0.00f, 0.68f, 0.85f, 1.0f),    // Technical Cyan (Accent)
            glm::vec4(0.92f, 0.94f, 0.96f, 1.0f),    // Blueprint White
            glm::vec4(0.35f, 0.42f, 0.50f, 1.0f),    // Steel Blue Grey
            glm::vec4(0.08f, 0.12f, 0.22f, 1.0f)     // Shadow Blue
        },
        // Palette 2: Malachite & Rust
        {
            glm::vec4(0.06f, 0.12f, 0.09f, 1.0f),    // Deep Jade
            glm::vec4(0.20f, 0.32f, 0.25f, 1.0f),    // Forest Shadow
            glm::vec4(0.48f, 0.62f, 0.50f, 1.0f),    // Muted Sage
            glm::vec4(0.78f, 0.38f, 0.22f, 1.0f),    // Raw Copper Rust (Accent)
            glm::vec4(0.93f, 0.91f, 0.86f, 1.0f),    // Warm Cream
            glm::vec4(0.12f, 0.18f, 0.15f, 1.0f)     // Dark Spruce
        },
        // Palette 3: Crimson & Steel
        {
            glm::vec4(0.08f, 0.08f, 0.08f, 1.0f),    // Ink Black
            glm::vec4(0.22f, 0.24f, 0.26f, 1.0f),    // Gunmetal
            glm::vec4(0.76f, 0.12f, 0.18f, 1.0f),    // Crimson Red (Accent)
            glm::vec4(0.95f, 0.95f, 0.95f, 1.0f),    // Pure White
            glm::vec4(0.48f, 0.50f, 0.52f, 1.0f),    // Steel Grey
            glm::vec4(0.14f, 0.15f, 0.16f, 1.0f)     // Charcoal
        },
        // Palette 4: Sage & Sand
        {
            glm::vec4(0.12f, 0.16f, 0.15f, 1.0f),    // Dark Sage Shadow
            glm::vec4(0.38f, 0.45f, 0.40f, 1.0f),    // Soft Sage Green
            glm::vec4(0.78f, 0.72f, 0.62f, 1.0f),    // Warm Sand Ochre
            glm::vec4(0.94f, 0.92f, 0.86f, 1.0f),    // Pale Bone Cream
            glm::vec4(0.68f, 0.35f, 0.20f, 1.0f),    // Terracotta (Accent)
            glm::vec4(0.22f, 0.26f, 0.24f, 1.0f)     // Olive Slate
        },
        // Palette 5: Bauhaus Constructivism
        {
            glm::vec4(0.78f, 0.15f, 0.12f, 1.0f),    // Vintage Bauhaus Red
            glm::vec4(0.12f, 0.28f, 0.48f, 1.0f),    // Muted Prussian Blue
            glm::vec4(0.88f, 0.68f, 0.18f, 1.0f),    // Mustard Ochre
            glm::vec4(0.93f, 0.89f, 0.80f, 1.0f),    // Aged Poster Cream
            glm::vec4(0.08f, 0.08f, 0.08f, 1.0f),    // Ink Black
            glm::vec4(0.62f, 0.32f, 0.15f, 1.0f),    // Rust Sienna
            glm::vec4(0.28f, 0.28f, 0.30f, 1.0f)     // Charcoal Slate
        },
        // Palette 6: Noir
        {
            glm::vec4(0.06f, 0.06f, 0.08f, 1.0f),    // Velvet Black
            glm::vec4(0.18f, 0.20f, 0.22f, 1.0f),    // Gunmetal Gray
            glm::vec4(0.35f, 0.38f, 0.42f, 1.0f),    // Foggy Slate
            glm::vec4(0.68f, 0.70f, 0.72f, 1.0f),    // Cool Silver
            glm::vec4(0.94f, 0.92f, 0.88f, 1.0f),    // Creamy Bone White
            glm::vec4(0.55f, 0.04f, 0.12f, 1.0f),    // Blood Burgundy (Accent)
            glm::vec4(0.10f, 0.11f, 0.15f, 1.0f)     // Deep Shadow Indigo
        },
        // Palette 7: Ochre & Charcoal
        {
            glm::vec4(0.15f, 0.13f, 0.12f, 1.0f),    // Charcoal Clay
            glm::vec4(0.72f, 0.31f, 0.20f, 1.0f),    // Burnt Terracotta
            glm::vec4(0.82f, 0.58f, 0.22f, 1.0f),    // Raw Ochre Yellow
            glm::vec4(0.42f, 0.48f, 0.38f, 1.0f),    // Sage Dust Green
            glm::vec4(0.38f, 0.24f, 0.18f, 1.0f),    // Earth Brown
            glm::vec4(0.92f, 0.88f, 0.80f, 1.0f),    // Bone Ash White
            glm::vec4(0.55f, 0.20f, 0.15f, 1.0f)     // Red Ochre
        },
        // Palette 8: Cyberpunk Graphic
        {
            glm::vec4(0.04f, 0.04f, 0.08f, 1.0f),    // Pitch Black
            glm::vec4(0.12f, 0.10f, 0.18f, 1.0f),    // Deep Graphite
            glm::vec4(0.95f, 0.05f, 0.52f, 1.0f),    // Neon Magenta (Accent)
            glm::vec4(0.00f, 0.85f, 0.88f, 1.0f),    // Neon Cyan
            glm::vec4(0.45f, 0.02f, 0.85f, 1.0f),    // Neon Violet
            glm::vec4(0.78f, 0.95f, 0.05f, 1.0f),    // Acid Lime
            glm::vec4(0.20f, 0.22f, 0.30f, 1.0f)     // Cool Tech Grey
        },
        // Palette 9: Muted Rose & Slate
        {
            glm::vec4(0.92f, 0.78f, 0.78f, 1.0f),    // Dusty Soft Rose
            glm::vec4(0.82f, 0.75f, 0.88f, 1.0f),    // Wisteria Lavender
            glm::vec4(0.72f, 0.82f, 0.90f, 1.0f),    // Muted Powder Blue
            glm::vec4(0.98f, 0.96f, 0.92f, 1.0f),    // Cream Pearl White
            glm::vec4(0.35f, 0.38f, 0.48f, 1.0f),    // Muted Twilight Indigo
            glm::vec4(0.15f, 0.16f, 0.22f, 1.0f)     // Slate Shadow
        },
        // Palette 10: Ash & Amber
        {
            glm::vec4(0.08f, 0.08f, 0.10f, 1.0f),    // Coal Black
            glm::vec4(0.25f, 0.26f, 0.28f, 1.0f),    // Ash Grey
            glm::vec4(0.92f, 0.65f, 0.08f, 1.0f),    // Warm Amber Gold (Accent)
            glm::vec4(0.94f, 0.92f, 0.88f, 1.0f),    // Bone Cream
            glm::vec4(0.55f, 0.56f, 0.58f, 1.0f),    // Concrete Grey
            glm::vec4(0.18f, 0.19f, 0.22f, 1.0f)     // Charcoal Shadow
        }
    };

    if (palette < 0) palette = 0;
    if (palette >= (int)artisticPalettes.size()) palette = (int)artisticPalettes.size() - 1;

    const auto& selectedPal = artisticPalettes[palette];
    int const size = num_colors + 1;
    for (int ic = 1; ic <= size; ++ic)
    {
        glm::vec4 c = selectedPal[(ic - 1) % selectedPal.size()];
        colorList.push_back(c);
    }
    return colorList;
}
