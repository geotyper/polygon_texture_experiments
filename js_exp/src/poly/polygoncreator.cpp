#include "polygoncreator.hpp"


#include "colormap/colormap.h"

#include <iostream>
#include <math.h>

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

vector<vector<TrianglesDrawStruct>> ArrangementBuilder::triangulatePolygons(vector<vector<glm::vec2>>& polygonList_in,int num_colors, bool offset, int palette)
{
    std::cout << "[Triangulate] triangulatePolygons called with " << polygonList_in.size() << " polygons (offset=" << (offset ? "true" : "false") << ")." << std::endl;
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

    for(int ip=0;ip<polygonList_in.size();ip++)
    {
        if(polygonList_in[ip].size()<3)
            continue;

        Path subj;
        for(auto point:polygonList_in[ip])
        {
            subj << IntPoint(point.x,point.y);
        }

        double areaPoly=Area(subj);

        if(abs(areaPoly)<minPolygonArea)
            continue;

        int rnd_color=rand() % num_colors + 1;
        glm::vec4 tempColor=colorList[rnd_color];

        /*
        float area=0;
        Path subj;
        for(auto point:polygonList_in[ip])
        {
            subj <<
            IntPoint(point.x,point.y);
        }

        double areaPoly=Area(subj);

        if(areaPoly<11.0)
            continue;
*/
        TPPLPartition pp;

        TPPLPoly *poly=new TPPLPoly;
        TPPLPolyList *polys=new TPPLPolyList;

        int nPoints = polygonList_in[ip].size();
        if (nPoints >= 2 && glm::distance(polygonList_in[ip].front(), polygonList_in[ip].back()) < 1e-4f) {
            nPoints--; // skip the duplicate closed vertex to avoid degenerate 0-length edges
        }

        if (nPoints < 3) {
            delete poly;
            delete polys;
            continue;
        }

        poly->Init(nPoints);
        for (int i = 0; i < nPoints; i++) {
            (*poly)[i].x = polygonList_in[ip][i].x;
            (*poly)[i].y = polygonList_in[ip][i].y;
        }

        // Set orientation to CCW as expected by PolyPartition
        poly->SetOrientation(TPPLOrientation::TPPL_ORIENTATION_CCW);

        int res=pp.Triangulate_OPT(poly,polys);
        if (res == 0) {
            // Fallback 1: Ear Clipping (very robust)
            res = pp.Triangulate_EC(poly, polys);
        }
        if (res == 0) {
            // Fallback 2: Monotone triangulation
            res = pp.Triangulate_MONO(poly, polys);
        }

        PolygonData tempPolygon;
        calcPolygon(tempPolygon.polyInfo, polygonList_in[ip]);

        vector<TrianglesDrawStruct> tempVertex;
        vector<unsigned int> tempIndex;

        vector<VertexPosUV> tempVertexUV;

        float area=0;

        list<TPPLPoly>::iterator iter;
        if(res==1)
        {
            for (iter = polys->begin(); iter != polys->end(); iter++) {
                iter->SetOrientation( TPPLOrientation::TPPL_ORIENTATION_CW);
                Path subj;

                for (int i = 0; i < iter->GetNumPoints(); i++) {
                    TrianglesDrawStruct tempStructTriangle;
                    tempStructTriangle.pos=(glm::vec2(iter->GetPoint(i).x ,iter->GetPoint(i).y));
                    tempStructTriangle.color=tempColor;
                    tempVertex.push_back(tempStructTriangle);
                    tempIndex.push_back(i);

                    subj <<
                    IntPoint(iter->GetPoint(i).x ,iter->GetPoint(i).y);

                    VertexPosUV tempPosUV;
                    tempPosUV.pos=(glm::vec2(iter->GetPoint(i).x ,iter->GetPoint(i).y));

                    tempPosUV.uv=tempPosUV.pos-tempPolygon.polyInfo.minboxCoord+tempPolygon.polyInfo.center;

                    if(tempPolygon.polyInfo.width>0)
                        tempPosUV.uv.x/=1.5*tempPolygon.polyInfo.width;
                    else
                        tempPosUV.uv.x=0;
                    if(tempPolygon.polyInfo.height>0)
                        tempPosUV.uv.y/=1.5*tempPolygon.polyInfo.height;
                    else
                        tempPosUV.uv.y=0;

                    tempVertexUV.push_back(tempPosUV);
                }

                double areaTri=Area(subj);
                area+=abs(areaTri);
            }
        }

        tempPolygon.polyInfo.area=abs(area);

        triangles_draw_vertex.push_back(tempVertex);
        triangles_draw_index.push_back(tempIndex);

        delete poly;
        delete polys;

        if(offset==true)
        {
           areaListOffset.push_back({counter,abs(area)});
           triangles_draw_vertexOffsetUV.push_back(tempVertexUV);
           polygonDataOffset.push_back(tempPolygon);
           polygonColorIndicesOffset.push_back(rnd_color);
        }
        else
        {
           areaList.push_back({counter,abs(area)});
           triangles_draw_vertexUV.push_back(tempVertexUV);
           polygonData.push_back(tempPolygon);
           polygonColorIndices.push_back(rnd_color);
        }

        counter++;

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
    if (palette >= 5)
    {
        std::vector<std::vector<glm::vec4>> artisticPalettes = {
            // Palette 5: Neo-Art (Vibrant, sophisticated pop color palette)
            {
                glm::vec4(0.18f, 0.08f, 0.35f, 1.0f),    // Deep Aubergine
                glm::vec4(0.68f, 0.45f, 0.90f, 1.0f),    // Electric Lavender
                glm::vec4(0.00f, 0.72f, 0.70f, 1.0f),    // Rich Teal
                glm::vec4(0.95f, 0.65f, 0.10f, 1.0f),    // Amber Gold
                glm::vec4(0.92f, 0.28f, 0.32f, 1.0f),    // Neo Coral
                glm::vec4(0.42f, 0.05f, 0.25f, 1.0f),    // Plum Wine
                glm::vec4(0.85f, 0.95f, 0.88f, 1.0f)     // Mint Cream
            },
            // Palette 6: Noir (Deep charcoal, slate, ash, silver, with a dramatic Blood Burgundy accent)
            {
                glm::vec4(0.06f, 0.06f, 0.08f, 1.0f),    // Velvet Black
                glm::vec4(0.18f, 0.20f, 0.22f, 1.0f),    // Gunmetal Gray
                glm::vec4(0.35f, 0.38f, 0.42f, 1.0f),    // Foggy Slate
                glm::vec4(0.68f, 0.70f, 0.72f, 1.0f),    // Cool Silver
                glm::vec4(0.94f, 0.92f, 0.88f, 1.0f),    // Creamy Bone White
                glm::vec4(0.55f, 0.04f, 0.12f, 1.0f),    // Blood Burgundy (Accent)
                glm::vec4(0.10f, 0.11f, 0.15f, 1.0f)     // Deep Shadow Indigo
            },
            // Palette 7: Primitivism (Earthy ochres, terracottas, and natural pigments)
            {
                glm::vec4(0.15f, 0.13f, 0.12f, 1.0f),    // Charcoal Clay
                glm::vec4(0.72f, 0.31f, 0.20f, 1.0f),    // Burnt Terracotta
                glm::vec4(0.82f, 0.58f, 0.22f, 1.0f),    // Raw Ochre Yellow
                glm::vec4(0.42f, 0.48f, 0.38f, 1.0f),    // Sage Dust Green
                glm::vec4(0.38f, 0.24f, 0.18f, 1.0f),    // Earth Brown
                glm::vec4(0.92f, 0.88f, 0.80f, 1.0f),    // Bone Ash White
                glm::vec4(0.55f, 0.20f, 0.15f, 1.0f)     // Red Ochre
            },
            // Palette 8: Cyberpunk (Neon fuchsia, cyber navy, holographic cyan, and lime)
            {
                glm::vec4(0.95f, 0.05f, 0.52f, 1.0f),    // Neon Magenta
                glm::vec4(0.45f, 0.02f, 0.85f, 1.0f),    // Electric Violet
                glm::vec4(0.05f, 0.06f, 0.18f, 1.0f),    // Deep Cyber Navy
                glm::vec4(0.78f, 0.95f, 0.05f, 1.0f),    // Acid Lime Tint
                glm::vec4(0.00f, 0.85f, 0.88f, 1.0f),    // Holographic Cyan
                glm::vec4(0.95f, 0.42f, 0.08f, 1.0f),    // Dusk Tangerine
                glm::vec4(0.25f, 0.12f, 0.45f, 1.0f)     // Synthwave Purple
            },
            // Palette 9: Pastel / Dreams (Dreamy, soft-luminous, delicate pastel sky/cloudscape tones)
            {
                glm::vec4(0.92f, 0.78f, 0.78f, 1.0f),    // Dusty Soft Rose
                glm::vec4(0.82f, 0.75f, 0.88f, 1.0f),    // Wisteria Lavender
                glm::vec4(0.72f, 0.82f, 0.90f, 1.0f),    // Muted Powder Blue
                glm::vec4(0.78f, 0.88f, 0.82f, 1.0f),    // Pale Sage Mint
                glm::vec4(0.98f, 0.96f, 0.92f, 1.0f),    // Cream Pearl White
                glm::vec4(0.96f, 0.82f, 0.75f, 1.0f),    // Soft Apricot Peach
                glm::vec4(0.35f, 0.38f, 0.48f, 1.0f)     // Muted Twilight Indigo (Depth Contrast)
            },
            // Palette 10: Bauhaus / Constructivism (Warm tomato red, Prussian blue, mustard ochre, aged cream)
            {
                glm::vec4(0.78f, 0.15f, 0.12f, 1.0f),    // Vintage Bauhaus Red
                glm::vec4(0.12f, 0.28f, 0.48f, 1.0f),    // Muted Prussian Blue
                glm::vec4(0.88f, 0.68f, 0.18f, 1.0f),    // Mustard Ochre
                glm::vec4(0.93f, 0.89f, 0.80f, 1.0f),    // Aged Poster Cream
                glm::vec4(0.08f, 0.08f, 0.08f, 1.0f),    // Ink Black
                glm::vec4(0.62f, 0.32f, 0.15f, 1.0f),    // Rust Sienna
                glm::vec4(0.28f, 0.28f, 0.30f, 1.0f)     // Charcoal Slate
            }
        };

        int pIdx = (palette - 5);
        if (pIdx < 0) pIdx = 0;
        if (pIdx >= (int)artisticPalettes.size()) pIdx = (int)artisticPalettes.size() - 1;

        const auto& selectedPal = artisticPalettes[pIdx];
        int const size = num_colors + 1;
        for (int ic = 1; ic <= size; ++ic)
        {
            glm::vec4 c = selectedPal[(ic - 1) % selectedPal.size()];
            colorList.push_back(c);
        }
    }
    else
    {
        using namespace colormap;
        transform::Ether colorMapEther;
        transform::Space colorMapSpace;
        transform::Malachite colorMapMalachite;
        transform::Seismic colorMapSeismic;
        transform::MorningGlory colorMapMorningGlory;
        int const size = num_colors+1;
        for(int ic=1; ic<=size;++ic)
        {
            float const x = ic / (float)size;
            Color c;
            switch(palette)
            {
                case 0:
                    c= colorMapEther.getColor(x);
                    break;
                case 1:
                    c= colorMapSpace.getColor(x);
                    break;
                case 2:
                    c= colorMapMalachite.getColor(x);
                    break;
                case 3:
                    c= colorMapSeismic.getColor(x);
                    break;
                case 4:
                    c= colorMapMorningGlory.getColor(x);
                    break;
                default:
                    c= colorMapEther.getColor(x);
                    break;
            }
            colorList.push_back(glm::vec4(c.r ,c.g ,c.b ,1.0));
        }
    }
    return colorList;
}
