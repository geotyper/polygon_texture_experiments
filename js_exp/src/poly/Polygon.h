// Author: Axel Antoine
// mail: ax.antoine@gmail.com
// website: https://axantoine.com
// 02/02/2021

// Loki, Inria project-team with Université de Lille
// within the Joint Research Unit UMR 9189 CNRS-Centrale
// Lille-Université de Lille, CRIStAL.
// https://loki.lille.inria.fr

// LICENCE: Licence.md

#ifndef _POLYGON_H
#define _POLYGON_H

#pragma once

#include <vector>
#include "Point.h"

extern "C" {
    #include "gpc.h"
}

typedef std::vector<Point> Contour;
typedef std::vector<Contour> ContourList;



inline void midFirstArray(int* array, int n) {
    // The goal here is to fill array such as mid indices are first in the array
    // such as:
    // [0, 1, 2, 3, 4] => [2, 3, 1, 4, 0]
    // [0, 1, 2, 3, 4, 5, 6, 7] => [3, 4, 2, 5, 1, 6, 0, 7]

    int mid = int(n/2);
    int idx=0;
    for(int j=mid; j<n; j++){
        array[idx] = j;
        idx+=2;
    }
    idx = 1;
    for(int j=mid-1;j>=0; j--){
        array[idx] = j;
        idx+=2;
    }
}






class Polygon
{
public:
    Contour contour;
    ContourList holes;

	Polygon(const Contour &contour, const ContourList &holes);
	~Polygon();
    bool getInsidePoint(Point &point);
    double getPolyTristripArea();

private:
    gpc_polygon* poly;
    gpc_tristrip* tristrip;

    void buildGPCPolygon();
    void fillGPCVertexListFromContour(
        gpc_vertex_list &vertex_list,
        const Contour &contour);
};

#endif // _POLYGON_H

