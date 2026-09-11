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
// #include "Ledit_ShapeCenter.h"
#include "ldata.h" /* Main UPI header. */
#include "Ledit_Safety.h"

    void swap(LPoint * a, LPoint * b);
    double distance(LPoint p1, LPoint p2);
    int compare(const void *a, const void *b);
    LPoint CalculateCentroid(LPoint * points, int n);
    int LSelection_GetNumber(LSelection selectedInital);
    bool CheckIsCircles(LSelection selectedInital, LPoint * centerPoints);
    void DrawCircleAtShapeCenter(void);
    void ShapeCenter_func(void);

    LPoint globalReferencePoint; // global

    // swap two point
    void swap(LPoint * a, LPoint * b)
    {
        LPoint temp = *a;
        *a = *b;
        *b = temp;
    }

    // Calculate the distance between two points
    double distance(LPoint p1, LPoint p2)
    {
        return hypot((double)p1.x - p2.x, (double)p1.y - p2.y);
    }

    // Comparison functions sorted from small to large by polar angle and distance from the origin
    int compare(const void *a, const void *b)
    {
        LPoint *p1 = (LPoint *)a;
        LPoint *p2 = (LPoint *)b;

        long x1 = p1->x - globalReferencePoint.x;
        long y1 = p1->y - globalReferencePoint.y;
        long x2 = p2->x - globalReferencePoint.x;
        long y2 = p2->y - globalReferencePoint.y;

        double angle1 = atan2(y1, x1);
        double angle2 = atan2(y2, x2);
        //  If the polar angles are the same, sort by distance from the origin from smallest to largest.
        if (angle1 == angle2)
        {
            double distance1 = hypot(x1, y1);
            double distance2 = hypot(x2, y2);
            return distance1 == distance2 ? 0 : (distance1 < distance2 ? -1 : 1);
        }

        // Compare polar angles
        return (angle1 < angle2) ? -1 : 1;
    }

    // Calculate the centroid coordinates of a convex polygon
    LPoint CalculateCentroid(LPoint * points, int n)
    {
        if (!points || n < 3) return LPoint_Set(0, 0);
        // Find the bottom and leftmost point as the pole
        int minIndex = 0;
        int i;
        for (i = 1; i < n; i++)
        {
            if (points[i].y < points[minIndex].y || (points[i].y == points[minIndex].y && points[i].x < points[minIndex].x))
            {
                minIndex = i;
            }
        }

        swap(&points[0], &points[minIndex]);

        globalReferencePoint = points[0];

        // Sort by polar angle
        qsort(&points[1], n - 1, sizeof(LPoint), compare);

        double cx = 0.0, cy = 0.0;
        double totalArea = 0.0;
        double pot = 0.0;
        // Traverse each pair of adjacent points
        for (i = 0; i < n; i++)
        {
            int next = (i + 1) % n;

            double x1 = (double)points[i].x - points[0].x;
            double y1 = (double)points[i].y - points[0].y;
            double x2 = (double)points[next].x - points[0].x;
            double y2 = (double)points[next].y - points[0].y;

            // Calculate the cross product
            double crossProduct = x1 * y2 - x2 * y1;

            // Update centroid coordinates and total area
            cx += (x1 + x2) * crossProduct;
            cy += (y1 + y2) * crossProduct;
            totalArea += crossProduct;
        }

        if (totalArea == 0)
        {
            /* Collinear/coincident centers have no area centroid. */
            cx = cy = 0;
            for (i = 0; i < n; ++i) { cx += points[i].x; cy += points[i].y; }
            return LPoint_Set(LM_RoundCoord(cx / n), LM_RoundCoord(cy / n));
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
        while (selectedInital != NULL)
        {
            selectedInital = LSelection_GetNext(selectedInital);
            counter++;
        }
        return counter;
    }

    bool CheckIsCircles(LSelection selectedInital, LPoint * centerPoints)
    {
        int i = 0;
        while (selectedInital) {
            LObject object = LSelection_GetObject(selectedInital);
            if (!object || LObject_GetShape(object) != LCircle) return false;
            centerPoints[i++] = LCircle_GetCenter(object);
            selectedInital = LSelection_GetNext(selectedInital);
        }
        return true;
    }

    void DrawCircleAtShapeCenter()
    {
        if (!LM_Context()) return;
        LCell Cell_Now = LCell_GetVisible();
        LFile File_Now = LCell_GetFile(Cell_Now);
        LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

        LSelection selectedInital = LSelection_GetList();
        int selectedObjectNumber = LSelection_GetNumber(selectedInital);
        if (selectedObjectNumber < 2 || selectedObjectNumber > LM_MAX_POINTS)
        {
            LDialog_AlertBox("Need more than 2 Object");
            return;
        }
        LPoint *centerPoints;
        centerPoints = (LPoint *)malloc((selectedObjectNumber) * sizeof(LPoint));
        if (!centerPoints) return;
        if (!CheckIsCircles(LSelection_GetList(), centerPoints))
        {
            free(centerPoints);
            LDialog_AlertBox("All objects must be circles.");
            return;
        }
        //****************************Input Params****************************//
        LDialogItem Dialog_Items[1] = {{"Radius (um)", "1"}};
        long radius;
        if (LM_Dialog("Draw Circle At Shape Center", Dialog_Items, 1))
        {
            radius = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the radius
        }
        else
        {
            free(centerPoints);
            return;
        }
        //****************************Input Params****************************//

        if (radius <= 0) { free(centerPoints); return; }
        if (selectedObjectNumber == 2)
        {
            long x = (centerPoints[0].x + centerPoints[1].x) / 2;
            long y = (centerPoints[0].y + centerPoints[1].y) / 2;
            LPoint center = LPoint_Set(x, y);
            LM_NewCircle(Cell_Now, LLayer_Now, center, radius);
            free(centerPoints);
            LDisplay_Refresh();
            return;
        }

        LPoint center = CalculateCentroid(centerPoints, selectedObjectNumber);
        LM_NewCircle(Cell_Now, LLayer_Now, center, radius);
            free(centerPoints);
            LDisplay_Refresh();

        LDisplay_Refresh();
    }

    void ShapeCenter_func(void)
    {
        LMacro_Register("DrawCircleAtShapeCenter_func", "DrawCircleAtShapeCenter");
    }
} /* end of module ShapeCenter_Module */

ShapeCenter_func();
