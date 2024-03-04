/*
 * ShapeCenter_Module
 * ShapeCenter_Module Marco for Ledit
 *
 * Copyright (c) 2024 muyeyifeng
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
module ShapeCenter_Module
{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
	#include "ldata.h"     /* Main UPI header. */

    #define PI 3.14159265358979323846
    LPoint globalReferencePoint;  // global
    // swap two point
    void swap(LPoint *a, LPoint *b) {
        LPoint temp = *a;
        *a = *b;
        *b = temp;
    }
    // Calculate the distance between two points
    double distance(LPoint p1, LPoint p2) {
        return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    }

    // Comparison functions sorted from small to large by polar angle and distance from the origin
    int compare(const void* a, const void* b) {
		LPoint* p1 = (LPoint*)a;
		LPoint* p2 = (LPoint*)b;

        long x1 = p1->x - globalReferencePoint.x;
        long y1 = p1->y - globalReferencePoint.y;
        long x2 = p2->x - globalReferencePoint.x;
        long y2 = p2->y - globalReferencePoint.y;
        
		//double angle1 = atan2(p1->y, p1->x); 
        double angle1 = atan2(y1, x1); 
		//LDialog_AlertBox(LFormat("angle1:%f", angle1));
		//double angle2 = atan2(p2->y, p2->x);
        double angle2 = atan2(y2, x2);
		//LDialog_AlertBox(LFormat("angle2:%f", angle2));
		// If the polar angles are the same, sort by distance from the origin from smallest to largest.
		if (angle1 == angle2) {
			double distance1 = hypot(x1, y1);
			double distance2 = hypot(x2, y2);
			return (distance1 < distance2) ? -1 : 1;
		}
		
		// Compare polar angles
		return (angle1 < angle2) ? -1 : 1;
    }

    // Calculate the centroid coordinates of a convex polygon
    LPoint CalculateCentroid(LPoint* points, int n) 
    {
        // Find the bottom and leftmost point as the pole
        int minIndex = 0;
        int i;
        for (i = 1; i < n; i++) {
            if (points[i].y < points[minIndex].y || (points[i].y == points[minIndex].y && points[i].x < points[minIndex].x)) {
                minIndex = i;
            }
        }
        
        swap(&points[0], &points[minIndex]);

        globalReferencePoint=points[0];

        // Sort by polar angle
        qsort(&points[1], n - 1, sizeof(LPoint), compare);

        double cx = 0.0, cy = 0.0;
        double totalArea = 0.0;
        double pot = 0.0;
        // Traverse each pair of adjacent points
        for (i = 0; i < n; i++) 
        {
            int next = (i + 1) % n;

            long x1 = points[i].x - points[0].x;
            long y1 = points[i].y - points[0].y;
            long x2 = points[next].x - points[0].x;
            long y2 = points[next].y - points[0].y;

            // Calculate the cross product
            //double crossProduct = points[i].x * points[next].y - points[next].x * points[i].y;
            double crossProduct = x1 * y2 - x2 * y1;

            // Update centroid coordinates and total area
            //cx += (points[i].x + points[next].x) * crossProduct;
            //cy += (points[i].y + points[next].y) * crossProduct;
            cx += (x1 + x2) * crossProduct;
            cy += (y1 + y2) * crossProduct;
            totalArea += crossProduct;
        }

        if(totalArea == 0)
        {
            return LPoint_Set(0, 0);
        }

        // Calculate centroid coordinates
        cx /= (3 * totalArea);
        cy /= (3 * totalArea);

        // Return centroid coordinates
        LPoint centroid = LPoint_Set((long)(cx + points[0].x), (long)(cy + points[0].y));
        return centroid;
    }

	int LSelection_GetNumber(LSelection selectedInital)
	{
		int counter = 0;
		while(selectedInital != NULL)
		{
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		return counter;
	}

   bool CheckIsCircles(LSelection selectedInital, LPoint* centerPoints)
	{
        int i=0;
		while(selectedInital != NULL)
		{
            LObject object1 = LSelection_GetObject(selectedInital);
            LPoint center = LCircle_GetCenter(object1);
            centerPoints[i] = center;
            //LDialog_AlertBox(LFormat("x:%d, y:%d", center.x, center.y));
            if(LObject_GetShape(object1) != 1)
            {
                return false;
            }
			selectedInital = LSelection_GetNext(selectedInital);
            i++;
		}
		return true;
    }

    void DrawCircleAtShapeCenter()
    {
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

	   LSelection selectedInital = LSelection_GetList();
		int selectedObjectNumber =	LSelection_GetNumber(selectedInital);
      if(selectedObjectNumber < 2)
		{
			LDialog_AlertBox("Need more than 2 Object");
			return;
		}
		LPoint* centerPoints;
		centerPoints = (LPoint *)malloc((selectedObjectNumber) * sizeof(LPoint));
		if(!CheckIsCircles(LSelection_GetList(), centerPoints))
		{
		   LDialog_AlertBox("All object must be circel");
		return;
		}
      //****************************Input Params****************************//
		LDialogItem Dialog_Items[1] = {{ "Radius (um)", "1" }};
		long radius;
		if(LDialog_MultiLineInputBox("Draw Circle At Shape Center", Dialog_Items, 1))
		{
			radius = (long)(atof(Dialog_Items[0].value) * 1000); // get the radius
		}
		else{
			return;
		}
		//****************************Input Params****************************//
    
      if(selectedObjectNumber == 2)
		{
			long x = (centerPoints[0].x + centerPoints[1].x) / 2;
			long y = (centerPoints[0].y + centerPoints[1].y) / 2;
			LPoint center = LPoint_Set(x, y);
			LCircle_New(Cell_Now, LLayer_Now, center, radius);
			return;
		}

		LPoint center = CalculateCentroid(centerPoints, selectedObjectNumber);
		//LDialog_AlertBox(LFormat("x:%d, y:%d", center.x, center.y));
		LCircle_New(Cell_Now, LLayer_Now, center, radius);

        LDisplay_Refresh();
    }

	void ShapeCenter_func(void)
	{
		LMacro_Register("DrawCircleAtShapeCenter_func", "DrawCircleAtShapeCenter");
	}
} /* end of module ShapeCenter_Module */

ShapeCenter_func();