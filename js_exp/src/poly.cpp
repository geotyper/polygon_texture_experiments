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

static void subdividePolygonRecursive(const std::vector<glm::vec2>& poly, int currentDepth, int maxDepth, std::vector<std::vector<glm::vec2>>& outputList) {
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
        subdividePolygonRecursive(subPoly, currentDepth + 1, maxDepth, outputList);
    }
}

static std::vector<glm::vec2> smoothPolygon(const std::vector<glm::vec2>& polygon, float tension, float pointsPerUnitLength) {
    int n = polygon.size();
    int nUnique = n;
    if (n >= 2 && glm::distance(polygon.front(), polygon.back()) < 1e-4f) {
        nUnique = n - 1;
    }

    if (nUnique < 3) return polygon;

    std::vector<glm::vec2> smoothed;

    for (int i = 0; i < nUnique; ++i) {
        glm::vec2 p0 = polygon[(i - 1 + nUnique) % nUnique];
        glm::vec2 p1 = polygon[i];
        glm::vec2 p2 = polygon[(i + 1) % nUnique];
        glm::vec2 p3 = polygon[(i + 2) % nUnique];

        float segmentLength = glm::distance(p1, p2);
        int subdivisions = (int)ceil(segmentLength * pointsPerUnitLength);
        if (subdivisions < 1) subdivisions = 1;

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

            if (!cell.empty())
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
                subdividePolygonRecursive(poly, 0, simParam.subdivisionDepth, subdividedResult);
            } else {
                subdividedResult.push_back(poly);
            }
        }
        result = subdividedResult;
    }

    if (simParam.enableSmoothing) {
        vector<vector<glm::vec2>> smoothedResult;
        for (auto& poly : result) {
            smoothedResult.push_back(smoothPolygon(poly, simParam.smoothingTension, simParam.smoothingPointsPerUnit));
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
     triangles_draw_vertex=arr.triangulatePolygons(polygonList_in,32, false,palette);
     triangles_draw_index=arr.triangles_draw_index;
}

vector<vector<TrianglesDrawStruct>> PolyLib::TriangulationPolygon(vector<vector<glm::vec2>>& polygonList_in, bool offset, int palette)
{
     vector<vector<TrianglesDrawStruct>> result;
     result=arr.triangulatePolygons(polygonList_in,32, offset,palette);
     return result;
}
