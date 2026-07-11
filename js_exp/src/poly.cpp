#include "poly.hpp"


PolyLib::PolyLib()
{

}

void PolyLib::FindIntersections(std::vector<PointData> &pointList, std::vector<Constraint> &constraintList_)
{
    pointListTemp.clear();
    constraintListTemp.clear();
    particleListTemp.clear();
    constraintsLookUpOldIndex.clear();
    constraintsLookUp.clear();

    std::vector<geometry::segment_t> segments;



    for (size_t  i = 0; i < constraintList_.size(); i++) {

        float x1=pointList[constraintList_[i].id1].coord.x;
        float y1=pointList[constraintList_[i].id1].coord.y;
        float x2=pointList[constraintList_[i].id2].coord.x;
        float y2=pointList[constraintList_[i].id2].coord.y;

        if(std::make_pair(x1, y1) > std::make_pair(x2, y2))
                    std::swap(x1, x2), std::swap(y1, y2);

        segments.push_back(geometry::segment_t{
                                geometry::point_t{ x1, y1 },
                                geometry::point_t{ x2, y2 },
                                i});
    }

     result = sweepline::find_intersections(segments);

     std::vector<glm::vec2> tmpPointsList;

     // 1. Copy all original points to pointListTemp and tmpPointsList
     for (size_t i = 0; i < pointList.size(); i++)
     {
         PointData tempPoint = pointList[i];
         tempPoint.neightbSize = 0;
         tempPoint.neighbors.clear();
         pointListTemp.push_back(tempPoint);
         tmpPointsList.push_back(tempPoint.coord);
     }

     // 2. Increment neightbSize based on original constraints
     for (size_t i = 0; i < constraintList_.size(); i++)
     {
         pointListTemp[constraintList_[i].id1].neightbSize++;
         pointListTemp[constraintList_[i].id2].neightbSize++;
     }

     // 3. Process sweepline intersections and add new intersection points
     for (auto res : result)
     {
         if (res.segments.size() == 0 or res.segments.data() == nullptr)
             continue;

         glm::vec2 intersectionPt(res.pt.x, res.pt.y);

         // Check if this intersection point is already in tmpPointsList
         int existingId = -1;
         for (int i = 0; i < tmpPointsList.size(); ++i)
         {
             if (glm::distance(tmpPointsList[i], intersectionPt) < 1e-4f)
             {
                 existingId = i;
                 break;
             }
         }

         if (existingId != -1)
         {
             pointListTemp[existingId].neightbSize += res.segments.size();
         }
         else
         {
             PointData tempPoint;
             tempPoint.coord = intersectionPt;
             tempPoint.neightbSize = res.segments.size();
             pointListTemp.push_back(tempPoint);
             tmpPointsList.push_back(tempPoint.coord);

             InstanceParticle instParticle;
             instParticle.instancePos = intersectionPt;
             instParticle.instanceRot = 0;
             instParticle.instanceScale = 3.0;
             particleListTemp.push_back(instParticle);
         }
     }

     // 4. Construct constraints based on intersections
     int counter = 0;
     for (auto res : result)
     {
         if (res.segments.size() == 0 or res.segments.data() == nullptr)
             continue;

         for (auto constr : res.segments)
         {
             auto findKey = constraintsLookUpOldIndex.find(constr);
             if (findKey != constraintsLookUpOldIndex.end()) {
                 continue;
             }
             else
             {
                 glm::vec2 firstPoint = glm::vec2(res.pt.x, res.pt.y);
                 
                 // Check if firstPoint is close to one of the original segment's endpoints
                 bool isEndpoint1 = glm::distance(pointList[constraintList_[constr].id1].coord, firstPoint) < 1e-4f;
                 bool isEndpoint2 = glm::distance(pointList[constraintList_[constr].id2].coord, firstPoint) < 1e-4f;

                 if (isEndpoint1 || isEndpoint2)
                 {
                      glm::vec2 secondPoint;
                      if (isEndpoint1)
                      {
                          secondPoint = pointList[constraintList_[constr].id2].coord;
                      }
                      else
                      {
                          secondPoint = pointList[constraintList_[constr].id1].coord;
                      }

                      int id1 = -1, id2 = -1;
                      bool findId1 = false;
                      bool findId2 = false;

                      for (int i = 0; i < tmpPointsList.size(); ++i)
                      {
                          if (glm::distance(tmpPointsList[i], firstPoint) < 1e-4f)
                          {
                              findId1 = true;
                              id1 = i;
                              break;
                          }
                      }
                      #ifdef FXPUBLISH
                      if (!findId1)
                         std::cout << "Error element1 not found in point vector\n";
                      #endif

                      for (int i = 0; i < tmpPointsList.size(); ++i)
                      {
                          if (glm::distance(tmpPointsList[i], secondPoint) < 1e-4f)
                          {
                              findId2 = true;
                              id2 = i;
                              break;
                          }
                      }
                      #ifdef FXPUBLISH
                      if (!findId2)
                         std::cout << "Error element2 not found in point vector\n";
                      #endif

                      if (findId1 and findId2)
                      {
                          GLuint resultIndex = indexConstraint(id1, id2);
                          constraintsLookUp.insert({resultIndex, {id1, id2}});

                          Constraint tempConstraint;
                          tempConstraint.id1 = id1;
                          tempConstraint.id2 = id2;
                          constraintListTemp.push_back(tempConstraint);

                          pointListTemp[id1].neighbors.push_back(id2);
                          pointListTemp[id2].neighbors.push_back(id1);
                      }
                  }
                  else
                  {
                      // The intersection point is strictly inside the segment, splitting it into two parts
                      glm::vec2 secondPoint1 = pointList[constraintList_[constr].id2].coord;
                      glm::vec2 secondPoint2 = pointList[constraintList_[constr].id1].coord;

                      int id1 = -1, id12 = -1, id22 = -1;
                      bool findId1 = false;
                      bool findId12 = false;
                      bool findId22 = false;

                      for (int i = 0; i < tmpPointsList.size(); ++i)
                      {
                          if (glm::distance(tmpPointsList[i], firstPoint) < 1e-4f)
                          {
                              findId1 = true;
                              id1 = i;
                              break;
                          }
                      }
                      #ifdef FXPUBLISH
                      if (!findId1)
                         std::cout << "Error element1 not found in point vector\n";
                      #endif

                      for (int i = 0; i < tmpPointsList.size(); ++i)
                      {
                          if (glm::distance(tmpPointsList[i], secondPoint1) < 1e-4f)
                          {
                              findId12 = true;
                              id12 = i;
                              break;
                          }
                      }
                      #ifdef FXPUBLISH
                      if (!findId12)
                         std::cout << "Error element2 not found in point vector\n";
                      #endif

                      for (int i = 0; i < tmpPointsList.size(); ++i)
                      {
                          if (glm::distance(tmpPointsList[i], secondPoint2) < 1e-4f)
                          {
                              findId22 = true;
                              id22 = i;
                              break;
                          }
                      }
                      #ifdef FXPUBLISH
                      if (!findId22)
                         std::cout << "Error element3 not found in point vector\n";
                      #endif

                      if (findId1 and findId12)
                      {
                         GLuint resultIndex12 = indexConstraint(id1, id12);
                         constraintsLookUp.insert({resultIndex12, {id1, id12}});

                         Constraint tempConstraint;
                         tempConstraint.id1 = id1;
                         tempConstraint.id2 = id12;
                         constraintListTemp.push_back(tempConstraint);

                         pointListTemp[id1].neighbors.push_back(id12);
                         pointListTemp[id12].neighbors.push_back(id1);
                      }

                      if (findId1 and findId22)
                      {
                         GLuint resultIndex22 = indexConstraint(id1, id22);
                         constraintsLookUp.insert({resultIndex22, {id1, id22}});

                         Constraint tempConstraint;
                         tempConstraint.id1 = id1;
                         tempConstraint.id2 = id22;
                         constraintListTemp.push_back(tempConstraint);

                         pointListTemp[id1].neighbors.push_back(id22);
                         pointListTemp[id22].neighbors.push_back(id1);
                      }
                  }

                  constraintsLookUpOldIndex.insert({constr, {counter, 1111111}});
              }
          }
          counter++;
      }

     std::cout<<result.size()<<std::endl;

     constraintListDraw.clear();



     for(int i=0;i<constraintListTemp.size();++i)
     {
         constraintListDraw.push_back(pointListTemp[constraintListTemp[i].id1].coord);
         constraintListDraw.push_back(pointListTemp[constraintListTemp[i].id2].coord);
     }

}



void PolyLib::DCEL(std::vector<PointData> &pointList, std::vector<Constraint> &constraintList_)
{

    using namespace ClipperLib;
    Paths subj(1), solution;

    //define outer blue 'subject' polygon
    subj[0] <<
      IntPoint(180,200) << IntPoint(260,200) <<
      IntPoint(260,150) << IntPoint(180,150);

    for(int i=0;i<constraintListTemp.size();++i)
    {
        pointListTemp[constraintListTemp[i].id1].coord;
        pointListTemp[constraintListTemp[i].id2].coord;

        subj[0] <<
          IntPoint((int)(pointListTemp[constraintListTemp[i].id1].coord.x),(int)(pointListTemp[constraintListTemp[i].id1].coord.y))
                <<
          IntPoint((int)(pointListTemp[constraintListTemp[i].id2].coord.x),(int)(pointListTemp[constraintListTemp[i].id2].coord.y));
    }

    //perform intersection ...
    Clipper c;
    c.AddPaths(subj, ptSubject, true);
    c.Execute(ctIntersection, solution, pftNonZero, pftNonZero);


}

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/create_straight_skeleton_2.h>
#include <boost/shared_ptr.hpp>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Straight_skeleton_2<K> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

static void subdividePolygonCentroidRecursive(const std::vector<glm::vec2>& poly, int currentDepth, int maxDepth, std::vector<std::vector<glm::vec2>>& outputList) {
    int n = poly.size();
    int nUnique = n;
    if (n >= 2 && glm::distance(poly.front(), poly.back()) < 1e-4f) {
        nUnique = n - 1;
    }

    if (nUnique < 3) {
        outputList.push_back(poly);
        return;
    }

    if (currentDepth >= maxDepth) {
        outputList.push_back(poly);
        return;
    }

    // Compute centroid
    glm::vec2 centroid(0.0f);
    for (int i = 0; i < nUnique; ++i) {
        centroid += poly[i];
    }
    centroid /= (float)nUnique;

    // Split into nUnique triangles
    for (int i = 0; i < nUnique; ++i) {
        glm::vec2 v1 = poly[i];
        glm::vec2 v2 = poly[(i + 1) % nUnique];

        std::vector<glm::vec2> subPoly = { v1, v2, centroid, v1 };
        subdividePolygonCentroidRecursive(subPoly, currentDepth + 1, maxDepth, outputList);
    }
}

static std::vector<glm::vec2> cleanPolygon(const std::vector<glm::vec2>& poly) {
    std::vector<glm::vec2> cleaned;
    int n = poly.size();
    if (n < 3) return poly;
    
    // Remove consecutive duplicate vertices
    for (int i = 0; i < n; ++i) {
        glm::vec2 p = poly[i];
        if (cleaned.empty() || glm::distance(cleaned.back(), p) > 1e-3f) {
            cleaned.push_back(p);
        }
    }
    
    // Check if last and first are duplicate
    while (cleaned.size() >= 3 && glm::distance(cleaned.front(), cleaned.back()) < 1e-3f) {
        cleaned.pop_back();
    }
    
    // Remove collinear vertices
    if (cleaned.size() < 3) return cleaned;
    std::vector<glm::vec2> finalPoly;
    int m = cleaned.size();
    for (int i = 0; i < m; ++i) {
        glm::vec2 p1 = cleaned[i];
        glm::vec2 p2 = cleaned[(i + 1) % m];
        glm::vec2 p3 = cleaned[(i + 2) % m];
        
        // Compute cross product of (p2 - p1) and (p3 - p2)
        float cross = (p2.x - p1.x) * (p3.y - p2.y) - (p2.y - p1.y) * (p3.x - p2.x);
        if (std::abs(cross) > 1e-3f) {
            finalPoly.push_back(p2);
        }
    }
    if (finalPoly.size() < 3) {
        return cleaned;
    }
    return finalPoly;
}

static void subdividePolygonSkeletonRecursive(const std::vector<glm::vec2>& poly, int currentDepth, int maxDepth, std::vector<std::vector<glm::vec2>>& outputList) {
    std::vector<glm::vec2> cleanPoly = cleanPolygon(poly);
    int nUnique = cleanPoly.size();

    if (nUnique < 3) {
        outputList.push_back(poly);
        return;
    }

    if (currentDepth >= maxDepth) {
        outputList.push_back(poly);
        return;
    }

    // Convert to CGAL Polygon_2
    Polygon_2 cgalPoly;
    for (int i = 0; i < nUnique; ++i) {
        cgalPoly.push_back(K::Point_2(cleanPoly[i].x, cleanPoly[i].y));
    }

    if (!cgalPoly.is_simple()) {
        subdividePolygonCentroidRecursive(poly, currentDepth, maxDepth, outputList);
        return;
    }

    double area = std::abs(CGAL::to_double(cgalPoly.area()));
    if (area < 25.0) {
        subdividePolygonCentroidRecursive(poly, currentDepth, maxDepth, outputList);
        return;
    }

    if (cgalPoly.is_clockwise_oriented()) {
        cgalPoly.reverse_orientation();
    }

    try {
        SsPtr ss = CGAL::create_interior_straight_skeleton_2(cgalPoly);
        if (!ss || ss->size_of_faces() == 0) {
            subdividePolygonCentroidRecursive(poly, currentDepth, maxDepth, outputList);
            return;
        }

        for (auto fit = ss->faces_begin(); fit != ss->faces_end(); ++fit) {
            auto h = fit->halfedge();
            std::vector<glm::vec2> subPoly;
            auto curr = h;
            bool validLoop = true;
            int vertexCount = 0;

            do {
                auto p = curr->vertex()->point();
                subPoly.push_back(glm::vec2(CGAL::to_double(p.x()), CGAL::to_double(p.y())));
                curr = curr->next();
                vertexCount++;

                if (vertexCount > 1000) {
                    validLoop = false;
                    break;
                }
            } while (curr != h);

            if (!validLoop || subPoly.size() < 3) {
                continue;
            }

            subPoly.push_back(subPoly.front()); // Close the loop
            subdividePolygonSkeletonRecursive(subPoly, currentDepth + 1, maxDepth, outputList);
        }
    } catch (...) {
        subdividePolygonCentroidRecursive(poly, currentDepth, maxDepth, outputList);
    }
}

static void subdividePolygonRecursive(const std::vector<glm::vec2>& poly, int currentDepth, int maxDepth, int mode, std::vector<std::vector<glm::vec2>>& outputList) {
    if (mode == 1) {
        subdividePolygonSkeletonRecursive(poly, currentDepth, maxDepth, outputList);
    } else {
        subdividePolygonCentroidRecursive(poly, currentDepth, maxDepth, outputList);
    }
}

static bool isPolygonValid(const std::vector<glm::vec2>& poly) {
    for (const auto& pt : poly) {
        if (std::isnan(pt.x) || std::isinf(pt.x) || std::isnan(pt.y) || std::isinf(pt.y)) {
            return false;
        }
    }
    return true;
}

static std::vector<glm::vec2> smoothPolygon(const std::vector<glm::vec2>& polygon, float tension, float pointsPerUnitLength) {
    int n = polygon.size();
    int nUnique = n;
    if (n >= 2 && glm::distance(polygon.front(), polygon.back()) < 1e-4f) {
        nUnique = n - 1;
    }

    if (nUnique < 3) return polygon;

    // Safety checks for parameters
    if (std::isnan(tension) || std::isinf(tension)) tension = 0.5f;
    if (std::isnan(pointsPerUnitLength) || std::isinf(pointsPerUnitLength) || pointsPerUnitLength <= 0.0f) {
        pointsPerUnitLength = 0.2f;
    }

    std::vector<glm::vec2> smoothed;

    for (int i = 0; i < nUnique; ++i) {
        glm::vec2 p0 = polygon[(i - 1 + nUnique) % nUnique];
        glm::vec2 p1 = polygon[i];
        glm::vec2 p2 = polygon[(i + 1) % nUnique];
        glm::vec2 p3 = polygon[(i + 2) % nUnique];

        // Safety check for vertices
        if (std::isnan(p0.x) || std::isinf(p0.x) || std::isnan(p0.y) || std::isinf(p0.y) ||
            std::isnan(p1.x) || std::isinf(p1.x) || std::isnan(p1.y) || std::isinf(p1.y) ||
            std::isnan(p2.x) || std::isinf(p2.x) || std::isnan(p2.y) || std::isinf(p2.y) ||
            std::isnan(p3.x) || std::isinf(p3.x) || std::isnan(p3.y) || std::isinf(p3.y)) {
            continue;
        }

        float segmentLength = glm::distance(p1, p2);
        if (std::isnan(segmentLength) || std::isinf(segmentLength)) {
            segmentLength = 0.0f;
        }

        int subdivisions = (int)ceil(segmentLength * pointsPerUnitLength);
        if (subdivisions < 1) subdivisions = 1;
        if (subdivisions > 100) subdivisions = 100; // Cap subdivisions to prevent OOM/CPU hangs

        for (int s = 0; s < subdivisions; ++s) {
            float t = (float)s / subdivisions;
            float t2 = t * t;
            float t3 = t2 * t;
            glm::vec2 catmull = 0.5f * (
                (2.0f * p1) +
                (-p0 + p2) * t +
                (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
            );
            glm::vec2 linear = (1.0f - t) * p1 + t * p2;
            glm::vec2 finalPt = (1.0f - tension) * catmull + tension * linear;
            smoothed.push_back(finalPt);
        }
    }

    if (!smoothed.empty()) {
        smoothed.push_back(smoothed.front()); // Close the loop
    }

    return smoothed;
}

vector<vector<glm::vec2>> PolyLib::testForPolygons(std::vector<PointData> &pointList, std::vector<Constraint> &constraintList, const SimDynamicParameters& simParam)
{
    vector<vector<glm::vec2>> result;

    typedef CGAL::Exact_predicates_exact_constructions_kernel  Kernel;
    typedef CGAL::Arr_segment_traits_2<Kernel>                 Traits_2;
    typedef Traits_2::Point_2                                  Point_2;
    typedef Traits_2::X_monotone_curve_2                       Segment_2;
    typedef CGAL::Arrangement_2<Traits_2>                      Arrangement_2;

    Arrangement_2 arr2D;
    std::vector<Segment_2> segments;

    for (size_t i = 0; i < constraintListTemp.size(); ++i)
    {
        glm::vec2 A = pointListTemp[constraintListTemp[i].id1].coord;
        glm::vec2 B = pointListTemp[constraintListTemp[i].id2].coord;

        // Skip degenerate segments
        if (glm::distance(A, B) < 1e-4f)
            continue;

        segments.push_back(Segment_2(Point_2(A.x, A.y), Point_2(B.x, B.y)));
    }

    try {
        CGAL::insert(arr2D, segments.begin(), segments.end());

        // Collect all bounded faces (cells)
        for (auto faceIter = arr2D.faces_begin(); faceIter != arr2D.faces_end(); ++faceIter)
        {
            if (faceIter->is_unbounded())
                continue;

            auto ccb = faceIter->outer_ccb();
            vector<glm::vec2> cell;

            auto curr = ccb;
            do {
                auto pt = curr->source()->point();
                double x = CGAL::to_double(pt.x());
                double y = CGAL::to_double(pt.y());
                cell.push_back(glm::vec2((float)x, (float)y));
                ++curr;
            } while (curr != ccb);

            if (!cell.empty() && isPolygonValid(cell))
            {
                cell.push_back(cell[0]); // close the loop
                result.push_back(cell);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[CGAL Error] Exception during Arrangement creation: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "[CGAL Error] Unknown exception during Arrangement creation." << std::endl;
    }

    // Apply recursive polygon subdivision for large polygons
    if (simParam.subdivisionDepth > 0) {
        vector<vector<glm::vec2>> subdividedResult;
        for (auto& poly : result) {
            double area = 0.0;
            int n = poly.size();
            int nUnique = n;
            if (n >= 2 && glm::distance(poly.front(), poly.back()) < 1e-4f) {
                nUnique = n - 1;
            }
            if (nUnique >= 3) {
                for (int i = 0; i < nUnique; ++i) {
                    glm::vec2 p1 = poly[i];
                    glm::vec2 p2 = poly[(i + 1) % nUnique];
                    area += (p1.x * p2.y - p2.x * p1.y);
                }
                area = std::abs(area) * 0.5;
            }

            if (area > simParam.subdivisionMinArea) {
                subdividePolygonRecursive(poly, 0, simParam.subdivisionDepth, simParam.subdivisionMode, subdividedResult);
            } else {
                subdividedResult.push_back(poly);
            }
        }
        result = subdividedResult;
    }

    if (simParam.enableSmoothing) {
        vector<vector<glm::vec2>> smoothedResult;
        for (auto& poly : result) {
            int n = poly.size();
            int nUnique = n;
            if (n >= 2 && glm::distance(poly.front(), poly.back()) < 1e-4f) {
                nUnique = n - 1;
            }
            if (nUnique > 0 && simParam.smoothingZoom != 1.0f) {
                glm::vec2 center(0.0f);
                for (int i = 0; i < nUnique; ++i) {
                    center += poly[i];
                }
                center /= (float)nUnique;

                auto scaledPoly = poly;
                for (int i = 0; i < n; ++i) {
                    scaledPoly[i] = center + (poly[i] - center) * simParam.smoothingZoom;
                }
                smoothedResult.push_back(smoothPolygon(scaledPoly, simParam.smoothingTension, simParam.smoothingPointsPerUnit));
            } else {
                smoothedResult.push_back(smoothPolygon(poly, simParam.smoothingTension, simParam.smoothingPointsPerUnit));
            }
        }
        result = smoothedResult;
    }

    arr.polygons = result;

    std::cout << "[CGAL PolygonDetector] Found " << result.size() << " cells/polygons." << std::endl;

    return result;
}

vector<vector<glm::vec2>> PolyLib::offsetPolygons(float offset)
{
    vector<vector<glm::vec2>> result;
    result=arr.offestPolygons(offset);
    return result;
}

void PolyLib::Triangulation(vector<vector<glm::vec2>>& polygonList_in, int palette)
{
     triangles_draw_vertex.clear();
     triangles_draw_vertex=arr.triangulatePolygons(polygonList_in,32, false,palette, false, false);
     triangles_draw_index=arr.triangles_draw_index;
}

vector<vector<TrianglesDrawStruct>> PolyLib::TriangulationPolygon(vector<vector<glm::vec2>>& polygonList_in, bool offset, int palette, bool mergePolygons, bool removeOverlappingPolygons)
{
     vector<vector<TrianglesDrawStruct>> result;
     result=arr.triangulatePolygons(polygonList_in,32, offset,palette, mergePolygons, removeOverlappingPolygons);
     return result;
}

void PolyLib::RecolorPolygons(int palette)
{
    int num_colors = 32;
    std::vector<glm::vec4> colorList = arr.generateColorList(palette, num_colors);

    // Recolor non-offset triangles
    if (triangles_draw_vertex.size() == arr.polygonColorIndices.size()) {
        for (size_t ip = 0; ip < triangles_draw_vertex.size(); ++ip) {
            int rnd_color = arr.polygonColorIndices[ip];
            if (rnd_color >= 0 && rnd_color < (int)colorList.size()) {
                glm::vec4 tempColor = colorList[rnd_color];
                for (auto& vertex : triangles_draw_vertex[ip]) {
                    vertex.color = tempColor;
                }
            }
        }
    }

    // Recolor offset triangles
    if (trianglesOffset_draw_vertex.size() == arr.polygonColorIndicesOffset.size()) {
        for (size_t ip = 0; ip < trianglesOffset_draw_vertex.size(); ++ip) {
            int rnd_color = arr.polygonColorIndicesOffset[ip];
            if (rnd_color >= 0 && rnd_color < (int)colorList.size()) {
                glm::vec4 tempColor = colorList[rnd_color];
                for (auto& vertex : trianglesOffset_draw_vertex[ip]) {
                    vertex.color = tempColor;
                }
            }
        }
    }
}
