/*
 * Build tangent rings and derive polygon contours from ordered arrangements of circles.
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
module ContourMacros
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "ldata.h" /* Main UPI header. */
#include "Ledit_OutputLayer.h"
#include "Ledit_Outline.h"
#include "Ledit_ArbitraryCircleContour.h"
    bool ReadCircleCenters(LObject * objects, int objectNum, LPoint *centerPoints);
    double CalculatePolarAngle(LPoint p1, LPoint p2);
    int CreateCircleTangentRings(LLayer outputLayer, LObject object1, LObject object2, long radius, LObject **torus);
    int CreateWireTangentRings(LLayer outputLayer, LObject object1, LObject object2, long radius, LObject **torus);
    int FindEquidistantPoints(LPoint A, LPoint B, long L, LPoint *results);
    int SampleRoundedHexagonArc(LPoint center, LCoord R, double start_angle_deg, double end_angle_deg, LPoint **result, long RoundRadius);
    double DistanceBetweenPoints(LPoint point1, LPoint point2);
    double CalculateVectorAngleCosine(LPoint point1, LPoint point2);
    double CalculateBearingDegrees(LPoint P1, LPoint P2);
    int SampleCircularArc(long center_x, long center_y, long radius, double start_angle_deg, double end_angle_deg, LPoint **result);
    LPoint CalculateMeanPoint(LPoint * points, int n);
    double CalculateShortArcSweep(double start_angle, double end_angle);
    void DrawTangentRing();
    void CreateCircularContour(LLayer outputLayer, LObject * orderedObjects, int orderedObjectNum, long innerDistance, long outerDistance, long referenceRadius, long minDistance, LPoint shapeCenter, LPoint *helperCenters);
    void CreateRoundedHexagonContour(LLayer outputLayer, LObject * orderedObjects, int orderedObjectNum, long innerDistance, long outerDistance, long referenceRadius, long minDistance, LPoint shapeCenter, long RoundRadius, LPoint *helperCenters);
    void CalculateLineCoefficients(LPoint p1, LPoint p2, double **abc);
    bool FindNextBoundaryCircle(LObject * orderedObjects, int orderedObjectNum, LObject *objects, int objectNum, double search_max_radius, double search_min_radius);
    int CountSelectedObjects(LSelection selection);
    double NormalizeAngleDegrees(double angle);
    void OrderCirclesAndDrawContour(LLayer outputLayer, LObject * objects, int objectNum, long innerDistance, long outerDistance, long referenceRadius, LPoint *helperCenters);
    void OrderCirclesAndDrawRoundedHexagonContour(LLayer outputLayer, LObject * objects, int objectNum, long innerDistance, long outerDistance, long referenceRadius, long RoundRadius, LPoint *helperCenters);
    void SelectBoundaryCirclesByNeighborCount(LSelection selection, long search_radius);
    void SelectBoundaryCircles();
    void RegisterContourMacros(void);

    void DrawTangentRing()
    {
        LLayer outputLayer = PickOutputLayer(LCell_GetVisible());
        if (outputLayer == NULL)
            return;

        LCell Cell_Now = LCell_GetVisible();
        LSelection selection = LSelection_GetList();
        int selectedObjectNumber = CountSelectedObjects(selection);
        if (selectedObjectNumber != 2)
        {
            LDialog_AlertBox("Just Support 2 Object");
            return;
        }

        LObject object1 = LSelection_GetObject(selection);
        selection = LSelection_GetNext(selection);
        LObject object2 = LSelection_GetObject(selection);

        LDialogItem Dialog_Items[2] = {{"Tangent Circle InnerRadius (um)", "1"},
                                       {"Ring width (um)", "1"}};
        long radius;
        long thickness;
        if (LDialog_MultiLineInputBox("Tangent Circle Radius", Dialog_Items, 2))
        {
            // To make sure torus inner other objects: radius + 10 nm
            radius = (long)(atof(Dialog_Items[0].value) * 1000) + 10; // get the radius
            thickness = (long)(atof(Dialog_Items[1].value) * 1000);   // get the thickness
        }
        else
        {
            return;
        }

        long *x1;
        long *y1;
        long *x2;
        long *y2;
        LObject *torus;
        int rootPoints = 0;
        if (LObject_GetShape(object1) == 1 && LObject_GetShape(object2) == 1)
        {
            rootPoints = CreateCircleTangentRings(outputLayer, object1, object2, radius, &torus);
        }
        if ((LObject_GetShape(object1) == 1 && LObject_GetShape(object2) == 2) || (LObject_GetShape(object1) == 2 && LObject_GetShape(object2) == 1))
        {
            rootPoints = CreateWireTangentRings(outputLayer, object1, object2, radius, &torus);
        }
        while (rootPoints > 0)
        {
            LTorusParams tParams;
            LTorus_GetParams(torus[rootPoints - 1], &tParams);
            tParams.nInnerRadius = radius;
            tParams.nOuterRadius = radius + thickness;
            LTorus_SetParams(Cell_Now, torus[rootPoints - 1], &tParams);
            rootPoints--;
        }
        LDisplay_Refresh();
    }

    /* Convert the two-vertex wire to a line and solve for candidate tangent arc centers. */
    int CreateWireTangentRings(LLayer outputLayer, LObject object1, LObject object2, long radius, LObject **torus)
    {
        *torus = (LObject *)malloc(4 * sizeof(LObject));
        LObject circle = LObject_GetShape(object1) == 1 ? object1 : object2;
        LObject wire = LObject_GetShape(object1) == 2 ? object1 : object2;

        LCell Cell_Now = LCell_GetVisible();
        // LFile File_Now = LCell_GetFile(Cell_Now);

        if (LVertex_GetCount(wire) != 2)
        {
            LDialog_AlertBox("Just Support 2 Point Wire");
            return 0;
        }
        LVertex vertex = LObject_GetVertexList(wire);
        LPoint point1 = LVertex_GetPoint(vertex);
        vertex = LVertex_GetNext(vertex);
        LPoint point2 = LVertex_GetPoint(vertex);
        double *abc;
        CalculateLineCoefficients(point1, point2, &abc);
        double rad = 0.0;

        double a = abc[0];
        double b = abc[1];
        double c = abc[2];

        LPoint center = LCircle_GetCenter(circle);
        double delta = a * center.x + b * center.y + c;

        if (fabs(b) < 1e-6)
        {
            rad = 3 * M_PI / 2;
        }
        else
        {
            rad = -atan(a / b);
        }

        double distance = fabs(delta) / sqrt(a * a + b * b);
        long r = LCircle_GetRadius(circle);
        // if line outside of circle
        if (distance > r + 2 * radius && r < distance)
        {
            LDialog_AlertBox(LFormat("%fx + %fy + %f = 0", a, b, c));
            LDialog_AlertBox(LFormat("y = %fx + %f", -a / b, -c / b));
            LDialog_AlertBox(LFormat("No root: distance: %f, r + 2 * radius: %ld", distance, r + 2 * radius));
            return 0;
        }

        double theta = acos((distance - radius) / (r + radius));
        double rho1 = delta < 0 ? rad + M_PI / 2 - theta : rad - M_PI / 2 - theta;
        double rho2 = delta < 0 ? rad + M_PI / 2 + theta : rad - M_PI / 2 + theta;
        long xx1 = center.x + (r + radius) * cos(rho1);
        long yy1 = center.y + (r + radius) * sin(rho1);
        long xx2 = center.x + (r + radius) * cos(rho2);
        long yy2 = center.y + (r + radius) * sin(rho2);

        LPoint p1 = LPoint_Set(xx1, yy1);
        double rad1 = rho1 + M_PI;
        double rad2 = delta < 0 ? rad + M_PI / 2 : rad + 3 * M_PI / 2;
        rad1 = fmod(rad1, 2 * M_PI);
        rad2 = fmod(rad2, 2 * M_PI);
        LTorusParams tParams1;
        tParams1.ptCenter = p1;
        tParams1.nInnerRadius = 1000;
        tParams1.nOuterRadius = 2000;
        // reset in DrawTangentRing()
        if (fabs(rad1 - rad2) > M_PI)
        {
            tParams1.dStartAngle = fmax(fabs(rad1), fabs(rad2)) * 180 / M_PI;
            tParams1.dStopAngle = fmin(fabs(rad1), fabs(rad2)) * 180 / M_PI;
        }
        else
        {
            tParams1.dStartAngle = fmin(fabs(rad1), fabs(rad2)) * 180 / M_PI;
            tParams1.dStopAngle = fmax(fabs(rad1), fabs(rad2)) * 180 / M_PI;
        }

        (*torus)[0] = LTorus_CreateNew(Cell_Now, outputLayer, &tParams1);
        LPoint p2 = LPoint_Set(xx2, yy2);
        double rad3 = rho2 + M_PI;
        double rad4 = rad2;
        rad3 = fmod(rad3, 2 * M_PI);
        rad4 = fmod(rad4, 2 * M_PI);
        LTorusParams tParams2;
        tParams2.ptCenter = p2;
        tParams2.nInnerRadius = 1000;
        tParams2.nOuterRadius = 2000;
        // set in DrawTangentRing()
        if (fabs(rad3 - rad4) > M_PI)
        {
            tParams2.dStartAngle = fmax(fabs(rad3), fabs(rad4)) * 180 / M_PI;
            tParams2.dStopAngle = fmin(fabs(rad3), fabs(rad4)) * 180 / M_PI;
        }
        else
        {
            tParams2.dStartAngle = fmin(fabs(rad3), fabs(rad4)) * 180 / M_PI;
            tParams2.dStopAngle = fmax(fabs(rad3), fabs(rad4)) * 180 / M_PI;
        }

        (*torus)[1] = LTorus_CreateNew(Cell_Now, outputLayer, &tParams2);
        // specially line cross circle
        if (r > distance)
        {
            theta = acos((distance + radius) / (r + radius));
            double rho3 = delta < 0 ? rad + M_PI / 2 - theta : rad - M_PI / 2 - theta;
            double rho4 = delta < 0 ? rad + M_PI / 2 + theta : rad - M_PI / 2 + theta;
            long xx3 = center.x + (r + radius) * cos(rho3);
            long yy3 = center.y + (r + radius) * sin(rho3);
            long xx4 = center.x + (r + radius) * cos(rho4);
            long yy4 = center.y + (r + radius) * sin(rho4);

            LPoint p3 = LPoint_Set(xx3, yy3);
            double rad5 = rho3 + M_PI;
            double rad6 = delta > 0 ? rad + M_PI / 2 : rad + 3 * M_PI / 2;
            rad5 = fmod(rad5, 2 * M_PI);
            rad6 = fmod(rad6, 2 * M_PI);
            LTorusParams tParams3;
            tParams3.ptCenter = p3;
            tParams3.nInnerRadius = 1000;
            tParams3.nOuterRadius = 2000;
            // reset in DrawTangentRing()
            if (fabs(rad5 - rad6) > M_PI)
            {
                tParams3.dStartAngle = fmax(fabs(rad5), fabs(rad6)) * 180 / M_PI;
                tParams3.dStopAngle = fmin(fabs(rad5), fabs(rad6)) * 180 / M_PI;
            }
            else
            {
                tParams3.dStartAngle = fmin(fabs(rad5), fabs(rad6)) * 180 / M_PI;
                tParams3.dStopAngle = fmax(fabs(rad5), fabs(rad6)) * 180 / M_PI;
            }

            (*torus)[2] = LTorus_CreateNew(Cell_Now, outputLayer, &tParams3);

            LPoint p4 = LPoint_Set(xx4, yy4);
            double rad7 = rho4 + M_PI;
            double rad8 = rad6;
            rad7 = fmod(rad7, 2 * M_PI);
            rad8 = fmod(rad8, 2 * M_PI);
            LTorusParams tParams4;
            tParams4.ptCenter = p4;
            tParams4.nInnerRadius = 1000;
            tParams4.nOuterRadius = 2000;
            // reset in DrawTangentRing()
            if (fabs(rad7 - rad8) > M_PI)
            {
                tParams4.dStartAngle = fmax(fabs(rad7), fabs(rad8)) * 180 / M_PI;
                tParams4.dStopAngle = fmin(fabs(rad7), fabs(rad8)) * 180 / M_PI;
            }
            else
            {
                tParams4.dStartAngle = fmin(fabs(rad7), fabs(rad8)) * 180 / M_PI;
                tParams4.dStopAngle = fmax(fabs(rad7), fabs(rad8)) * 180 / M_PI;
            }

            (*torus)[3] = LTorus_CreateNew(Cell_Now, outputLayer, &tParams4);
            return 4;
        }
        return 2;
    }

    /* Intersect the offset circles to find centers tangent to both source circles. */
    int CreateCircleTangentRings(LLayer outputLayer, LObject object1, LObject object2, long radius, LObject **torus)
    {
        *torus = (LObject *)malloc(2 * sizeof(LObject));
        LCell Cell_Now = LCell_GetVisible();
        // LFile File_Now = LCell_GetFile(Cell_Now);

        LPoint centerp1 = LCircle_GetCenter(object1);
        LPoint centerp2 = LCircle_GetCenter(object2);
        long R1 = LCircle_GetRadius(object1) + radius;
        long R2 = LCircle_GetRadius(object2) + radius;

        long x1 = centerp1.x;
        long y1 = centerp1.y;

        long x2 = centerp2.x;
        long y2 = centerp2.y;

        double d = sqrt((double)(x2 - x1) * (x2 - x1) + (double)(y2 - y1) * (y2 - y1));

        if (d > R1 + R2 || d < fabs(R1 - R2))
        {
            return 0;
        }
        else if (d == 0 && R1 == R2)
        {
            return -1;
        }
        else
        {
            double a = ((double)R1 * R1 - (double)R2 * R2 + d * d) / (2 * d);
            double h = sqrt((double)R1 * R1 - a * a);

            double xMid = x1 + a * (x2 - x1) / d;
            double yMid = y1 + a * (y2 - y1) / d;

            long xx1 = (long)(xMid + h * (y2 - y1) / d);
            long yy1 = (long)(yMid - h * (x2 - x1) / d);

            long xx2 = (long)(xMid - h * (y2 - y1) / d);
            long yy2 = (long)(yMid + h * (x2 - x1) / d);

            LPoint p1 = LPoint_Set(xx1, yy1);
            double rad1 = CalculatePolarAngle(p1, centerp1);
            double rad2 = CalculatePolarAngle(p1, centerp2);
            LTorusParams tParams1;
            tParams1.ptCenter = p1;
            tParams1.nInnerRadius = 1000;
            tParams1.nOuterRadius = 2000;
            // reset in DrawTangentRing()
            if (fabs(rad1 - rad2) > M_PI)
            {
                tParams1.dStartAngle = fmax(fabs(rad1), fabs(rad2)) * 180 / M_PI;
                tParams1.dStopAngle = fmin(fabs(rad1), fabs(rad2)) * 180 / M_PI;
            }
            else
            {
                tParams1.dStartAngle = fmin(fabs(rad1), fabs(rad2)) * 180 / M_PI;
                tParams1.dStopAngle = fmax(fabs(rad1), fabs(rad2)) * 180 / M_PI;
            }

            (*torus)[0] = LTorus_CreateNew(Cell_Now, outputLayer, &tParams1);
            LPoint p2 = LPoint_Set(xx2, yy2);
            double rad3 = CalculatePolarAngle(p2, centerp1);
            double rad4 = CalculatePolarAngle(p2, centerp2);
            LTorusParams tParams2;
            tParams2.ptCenter = p2;
            tParams2.nInnerRadius = 1000;
            tParams2.nOuterRadius = 2000;
            // reset in DrawTangentRing()
            if (fabs(rad3 - rad4) > M_PI)
            {
                tParams2.dStartAngle = fmax(fabs(rad3), fabs(rad4)) * 180 / M_PI;
                tParams2.dStopAngle = fmin(fabs(rad3), fabs(rad4)) * 180 / M_PI;
            }
            else
            {
                tParams2.dStartAngle = fmin(fabs(rad3), fabs(rad4)) * 180 / M_PI;
                tParams2.dStopAngle = fmax(fabs(rad3), fabs(rad4)) * 180 / M_PI;
            }

            (*torus)[1] = LTorus_CreateNew(Cell_Now, outputLayer, &tParams2);
            return 2;
        }
    }

    double CalculatePolarAngle(LPoint p1, LPoint p2)
    {
        // Calculate the differences between the two points
        double deltaX = p2.x - p1.x;
        double deltaY = p2.y - p1.y;

        // Use the arctangent function to calculate the polar angle
        double angle = atan2(deltaY, deltaX);
        if (angle < 0)
            angle = 2 * M_PI + angle;
        return angle;
    }

    int CountSelectedObjects(LSelection selection)
    {
        int counter = 0;
        while (selection != NULL)
        {
            selection = LSelection_GetNext(selection);
            counter++;
        }
        return counter;
    }

    void CalculateLineCoefficients(LPoint p1, LPoint p2, double **abc)
    {
        *abc = (double *)malloc(3 * sizeof(double));
        if (p2.x - p1.x != 0)
        {
            // Calculate the slope m
            double m = (double)(p2.y - p1.y) / (p2.x - p1.x);
            // Convert to standard form ax + by + c = 0
            (*abc)[0] = (double)(p2.y - p1.y) / (p1.x - p2.x);
            (*abc)[1] = 1.0;
            (*abc)[2] = ((double)p2.y * p1.x - (double)p1.y * p2.x) / (p2.x - p1.x);
        }
        else
        {
            // If slope does not exist, the line is vertical, and the equation is x = x1
            (*abc)[0] = 1.0;
            (*abc)[1] = 0.0;
            (*abc)[2] = -p1.x;
        }
        // y = kx +b
        // k = -a / b
    }

    double DistanceBetweenPoints(LPoint point1, LPoint point2)
    {
        return sqrt((double)(point1.x - point2.x) * (double)(point1.x - point2.x) + (double)(point1.y - point2.y) * (point1.y - point2.y));
    }

    /* Use the number of nearby circles to identify sites on the edge of the array. */
    void SelectBoundaryCirclesByNeighborCount(LSelection selection, long search_radius)
    {
        // LSelection selection = LSelection_GetList();
        while (selection != NULL)
        {
            LObject selectedObject = LSelection_GetObject(selection);
            LShapeType selectedShapeType = LObject_GetShape(selectedObject);
            if (selectedShapeType != 1)
                return;
            selection = LSelection_GetNext(selection);
        }

        selection = LSelection_GetList();
        int objectNum = CountSelectedObjects(selection);
        LObject *objects = (LObject *)malloc(objectNum * sizeof(LObject));

        int i, j;
        for (i = 0; i < objectNum; i++)
        {
            objects[i] = LSelection_GetObject(selection);
            selection = LSelection_GetNext(selection);
        }

        LSelection_DeselectAll();

        LObject *boundaryObjects = (LObject *)malloc(objectNum * sizeof(LObject));
        int boundaryObjectCount = 0;
        for (i = 0; i < objectNum; i++)
        {
            int counter = 0;
            LObject object1 = objects[i];
            LPoint center1 = LCircle_GetCenter(object1);
            for (j = 0; j < objectNum; j++)
            {
                LObject object2 = objects[j];
                LPoint center2 = LCircle_GetCenter(object2);
                if (DistanceBetweenPoints(center1, center2) < search_radius)
                {
                    counter++;
                }
            }
            if (counter < 7)
            {
                boundaryObjects[boundaryObjectCount] = object1;
                boundaryObjectCount++;
                LSelection_AddObject(object1);
            }
            counter = 0;
        }
        LDisplay_Refresh();
    }

    void SelectBoundaryCircles()
    {
        LDialogItem Dialog_Items[1] = {{"Radius (um) (Inf)", "0"}};
        double search_radius;
        if (LDialog_MultiLineInputBox("Select boundary circles", Dialog_Items, 1))
        {
            search_radius = (long)(atof(Dialog_Items[0].value) * 1000); // get the Scale Factor
        }
        else
        {
            return;
        }
        LSelection selection = LSelection_GetList();
        SelectBoundaryCirclesByNeighborCount(selection, search_radius);
    }

    bool FindNextBoundaryCircle(LObject * orderedObjects, int orderedObjectNum, LObject *objects, int objectNum, double search_max_radius, double search_min_radius)
    {

        if (orderedObjectNum > objectNum)
            return false;
        if (orderedObjectNum == objectNum)
        {
            LPoint firstObjectCenter = LCircle_GetCenter(orderedObjects[0]);
            LPoint lastObjectCenter = LCircle_GetCenter(orderedObjects[orderedObjectNum - 1]);
            double distance = DistanceBetweenPoints(firstObjectCenter, lastObjectCenter);
            if (distance < search_max_radius && distance > search_min_radius)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        int i, j;
        LObject thisObject = orderedObjects[orderedObjectNum - 1];
        LPoint center1 = LCircle_GetCenter(thisObject);
        for (i = 1; i < objectNum; i++)
        {
            LPoint center2 = LCircle_GetCenter(objects[i]);
            bool isExist = false;
            for (j = 0; j < orderedObjectNum; j++)
            {
                LPoint center3 = LCircle_GetCenter(orderedObjects[j]);
                if (center3.x == center2.x && center3.y == center2.y)
                {
                    isExist = true;
                    break;
                }
            }
            if (isExist)
                continue;
            if (DistanceBetweenPoints(center1, center2) < search_max_radius && DistanceBetweenPoints(center1, center2) > search_min_radius)
            {
                orderedObjects[orderedObjectNum] = objects[i];
                orderedObjectNum++;
                if (FindNextBoundaryCircle(orderedObjects, orderedObjectNum, objects, objectNum, search_max_radius, search_min_radius))
                {
                    return true;
                }
                else
                {
                    orderedObjectNum--;
                }
            }
        }
        return false;
    }

    LPoint CalculateMeanPoint(LPoint * points, int n)
    {
        // Find the bottom and leftmost point as the pole
        long xSum = 0, ySum = 0;
        int i;
        for (i = 0; i < n; i++)
        {
            xSum += points[i].x;
            ySum += points[i].y;
        }

        // Return centroid coordinates
        LPoint centroid = LPoint_Set((long)(xSum / n), (long)(ySum / n));
        return centroid;
    }

    void OrderCirclesAndDrawContour(LLayer outputLayer, LObject * objects, int objectNum, long innerDistance, long outerDistance, long referenceRadius, LPoint *helperCenters)
    {

        // put the left bottom circle to the first
        int i;
        for (i = 1; i < objectNum; i++)
        {
            LPoint center1 = LCircle_GetCenter(objects[0]);
            LPoint center2 = LCircle_GetCenter(objects[i]);
            if (center1.x > center2.x || (center1.x == center2.x && center1.y > center2.y))
            {
                LObject temp = objects[0];
                objects[0] = objects[i];
                objects[i] = temp;
            }
        }
        // get the minimun circle distance
        double minDistance = WORLD_MAX;
        for (i = 1; i < objectNum; i++)
        {
            LPoint center1 = LCircle_GetCenter(objects[0]);
            LPoint center2 = LCircle_GetCenter(objects[i]);
            double distance = DistanceBetweenPoints(center1, center2);
            if (distance < minDistance)
            {
                minDistance = distance;
            }
        }

        double search_max_radius = minDistance + 100;
        double search_min_radius = minDistance - 100;

        //  create a new array to store the ordered circles
        LObject *orderedObjects = (LObject *)malloc(objectNum * sizeof(LObject));
        orderedObjects[0] = objects[0];
        int orderedObjectNum = 1;
        bool findSatus = FindNextBoundaryCircle(orderedObjects, orderedObjectNum, objects, objectNum, search_max_radius, search_min_radius);
        if (findSatus)
        {
            LDialog_MsgBox("Order Success");
        }
        else
        {
            LDialog_MsgBox("Order Fail");
            return;
        }
        // calcu
        LPoint *centerPoints;
        centerPoints = (LPoint *)malloc((objectNum) * sizeof(LPoint));
        ReadCircleCenters(objects, objectNum, centerPoints);
        LPoint center;
        center = CalculateMeanPoint(centerPoints, objectNum);

        CreateCircularContour(outputLayer, orderedObjects, objectNum, innerDistance, outerDistance, referenceRadius, (long)minDistance, center, helperCenters);

        // test: change the size of the selected objects by order
        // LCell Cell_Now = LCell_GetVisible();
        // for(i = 0; i < objectNum; i++)
        // {
        //     LObject object = orderedObjects[i];
        //     LPoint center = LCircle_GetCenter(object);
        //     long radius = LCircle_GetRadius(object);
        //     radius = radius + 1000*i;
        //     LCircle_Set(Cell_Now, object, center, radius);
        // }
    }

    int FindEquidistantPoints(LPoint A, LPoint B, long L, LPoint *results)
    {
        if (L < 0)
        {
            return 0;
        }

        double dx = B.x - A.x;
        double dy = B.y - A.y;

        double d_AB_sq = dx * dx + dy * dy;

        // Coincident endpoints do not define a unique bisector.
        if (d_AB_sq < 1e-9)
        {
            return 0;
        }

        double d_AB = sqrt(d_AB_sq);

        // The radius must reach the midpoint of the chord.
        if (L * L < d_AB_sq / 4.0 - 1e-9)
        {             // L^2 < (AB/2)^2 => L < AB/2
            return 0;
        }

        LPoint M;
        M.x = (A.x + B.x) / 2.0;
        M.y = (A.y + B.y) / 2.0;

        // Find the displacement along the perpendicular bisector.
        double d_M_sq = L * L - d_AB_sq / 4.0;
        double d_M = sqrt(d_M_sq);

        double unit_perp_x1 = -dy / d_AB;
        double unit_perp_y1 = dx / d_AB;

        double unit_perp_x2 = dy / d_AB;
        double unit_perp_y2 = -dx / d_AB;

        results[0].x = (long)(M.x + d_M * unit_perp_x1);
        results[0].y = (long)(M.y + d_M * unit_perp_y1);

        results[1].x = (long)(M.x + d_M * unit_perp_x2);
        results[1].y = (long)(M.y + d_M * unit_perp_y2);

        // When the displacement is zero, both solutions coincide at the midpoint.
        if (d_M < 1e-9)
        {
            return 1;
        }
        return 2;
    }

    bool ReadCircleCenters(LObject * objects, int objectNum, LPoint *centerPoints)
    {
        int i;
        for (i = 0; i < objectNum; i++)
        {
            LObject object1 = objects[i];
            if (LObject_GetShape(object1) != 1)
            {
                return false;
            }
            LPoint center = LCircle_GetCenter(object1);
            centerPoints[i] = center;
        }
        return true;
    }

    double CalculateBearingDegrees(LPoint P1, LPoint P2)
    {
        double vec_x = P2.x - P1.x;
        double vec_y = P2.y - P1.y;

        // atan2(y, x)
        double angle_rad = atan2(vec_y, vec_x);

        double angle_degrees = angle_rad * 180.0 / M_PI;

        return angle_degrees;
    }

    double NormalizeAngleDegrees(double angle)
    {
        while (angle <= -180.0)
            angle += 360.0;
        while (angle > 180.0)
            angle -= 360.0;
        return angle;
    }

    LLayer CreateBoundaryHelperLayer(LFile file)
    {
        LLayer last = NULL, layer;
        for (layer = LLayer_GetList(file); layer != NULL; layer = LLayer_GetNext(layer))
            last = layer;

        char name[40];
        int suffix;
        for (suffix = 1; suffix <= 10000; suffix++)
        {
            snprintf(name, sizeof(name), "Boundary_Helpers_%d", suffix);
            if (LLayer_Find(file, name) != NULL)
                continue;
            if (LLayer_New(file, last, name) != LStatusOK)
                break;
            return LLayer_Find(file, name);
        }
        return NULL;
    }

    void PublishBoundaryHelpers(LCell cell, const LPoint *centers, int count, LCoord radius)
    {
        if (count == 0)
            return;
        LLayer layer = CreateBoundaryHelperLayer(LCell_GetFile(cell));
        if (layer == NULL)
        {
            LDialog_AlertBox("Boundary created. Could not create the helper circle layer.");
            return;
        }

        int i;
        for (i = 0; i < count; i++)
        {
            if (LCircle_New(cell, layer, centers[i], radius) == NULL)
            {
                LDialog_AlertBox("Boundary created. Could not draw all helper circles.");
                return;
            }
        }
    }

    /* Join sampled circular pieces in boundary order to produce one polygon. */
    void CreateCircularContour(LLayer outputLayer, LObject * orderedObjects, int orderedObjectNum, long innerDistance, long outerDistance, long referenceRadius, long minDistance, LPoint shapeCenter, LPoint *helperCenters)
    {
        LCell Cell_Now = LCell_GetVisible();
        double startAngle = -135; // start from left bottom circle
        double angle1, angle2;
        int i, j, helperCount = 0;
        int currentSize = 0;
        int capacity = 1000;
        LPoint *totalPolygon = (LPoint *)malloc(capacity * sizeof(LPoint));
        if (totalPolygon == NULL)
        {
            LDialog_AlertBox("Could not allocate the boundary polygon.");
            return;
        }
        for (i = 0; i < orderedObjectNum; i++)
        {
            LPoint circle1 = LCircle_GetCenter(orderedObjects[i % orderedObjectNum]);
            long radius1 = LCircle_GetRadius(orderedObjects[i % orderedObjectNum]);
            LPoint circle2 = LCircle_GetCenter(orderedObjects[(i + 1) % orderedObjectNum]);
            long radius2 = LCircle_GetRadius(orderedObjects[(i + 1) % orderedObjectNum]);
            long L = innerDistance + outerDistance + referenceRadius * 2;
            LPoint results1[2];
            int resultNum = FindEquidistantPoints(circle1, circle2, L, results1);
            LPoint result1;
            if (resultNum == 0)
                continue;
            if (resultNum == 1)
                result1 = results1[0];
            if (resultNum == 2)
            {
                result1 = DistanceBetweenPoints(shapeCenter, results1[0]) > DistanceBetweenPoints(shapeCenter, results1[1]) ? results1[0] : results1[1];
            }

            LPoint circle3 = LCircle_GetCenter(orderedObjects[(i + 2) % orderedObjectNum]);
            long radius3 = LCircle_GetRadius(orderedObjects[(i + 2) % orderedObjectNum]);
            LPoint results2[2];
            resultNum = FindEquidistantPoints(circle2, circle3, L, results2);
            LPoint result2;
            if (resultNum == 0)
                continue;
            if (resultNum == 1)
                result2 = results2[0];
            if (resultNum == 2)
            {
                result2 = DistanceBetweenPoints(shapeCenter, results2[0]) > DistanceBetweenPoints(shapeCenter, results2[1]) ? results2[0] : results2[1];
            }

            LPoint *part1;
            LPoint *part2;
            if (DistanceBetweenPoints(result1, result2) < minDistance * 0.9)
            {
                LPoint results3[2];
                LPoint result3;
                resultNum = FindEquidistantPoints(circle1, circle3, L, results3);
                if (resultNum == 0)
                    continue;
                if (resultNum == 1)
                    result3 = results3[0];
                if (resultNum == 2)
                {
                    result3 = DistanceBetweenPoints(shapeCenter, results3[0]) > DistanceBetweenPoints(shapeCenter, results3[1]) ? results3[0] : results3[1];
                }

                angle1 = CalculateBearingDegrees(circle1, result3);
                angle2 = CalculateBearingDegrees(circle3, result3);
                double _angle1 = NormalizeAngleDegrees(angle1 + 180);
                double _angle2 = NormalizeAngleDegrees(angle2 + 180);
                // start - >angle1
                //_angle1 -> _angle2
                //
                int cnt1 = SampleCircularArc(circle1.x, circle1.y, radius1 + innerDistance, startAngle, angle1, &part1);
                int cnt2 = SampleCircularArc(result3.x, result3.y, radius1 + outerDistance, _angle1, _angle2, &part2);
                if (capacity < 1.5 * (cnt1 + cnt2 + currentSize))
                {
                    int newCapacity = 1.5 * (cnt1 + cnt2 + currentSize);
                    LPoint *tempArray = (LPoint *)realloc(totalPolygon, newCapacity * sizeof(LPoint));
                    totalPolygon = tempArray;
                    capacity = newCapacity;
                }

                for (j = 0; j < cnt1; j++)
                {
                    totalPolygon[currentSize] = part1[j];
                    currentSize++;
                }

                for (j = 0; j < cnt2; j++)
                {
                    totalPolygon[currentSize] = part2[j];
                    currentSize++;
                }

                if (helperCenters != NULL)
                    helperCenters[helperCount++] = result3;
                startAngle = angle2;
                i++;
                continue;
                // special area
            };
            angle1 = CalculateBearingDegrees(circle1, result1);
            angle2 = CalculateBearingDegrees(circle2, result1);
            double _angle1 = NormalizeAngleDegrees(angle1 + 180);
            double _angle2 = NormalizeAngleDegrees(angle2 + 180);

            int cnt3 = SampleCircularArc(circle1.x, circle1.y, radius1 + innerDistance, startAngle, angle1, &part1);
            int cnt4 = SampleCircularArc(result1.x, result1.y, radius1 + outerDistance, _angle1, _angle2, &part2);
            if (capacity < 1.5 * (cnt3 + cnt4 + currentSize))
            {
                int newCapacity = 1.5 * (cnt3 + cnt4 + currentSize);
                LPoint *tempArray = (LPoint *)realloc(totalPolygon, newCapacity * sizeof(LPoint));
                totalPolygon = tempArray;
                capacity = newCapacity;
            }

            for (j = 0; j < cnt3; j++)
            {
                totalPolygon[currentSize] = part1[j];
                currentSize++;
            }

            for (j = 0; j < cnt4; j++)
            {
                totalPolygon[currentSize] = part2[j];
                currentSize++;
            }
            if (helperCenters != NULL)
                helperCenters[helperCount++] = result1;
            startAngle = angle2;
        }
        LPoint *partSmall;
        LPoint circle0 = LCircle_GetCenter(orderedObjects[0]);
        long radius0 = LCircle_GetRadius(orderedObjects[0]);
        int small = SampleCircularArc(circle0.x, circle0.y, radius0 + innerDistance, startAngle, -135.0, &partSmall);

        if (capacity < small + currentSize)
        {
            int newCapacity = 1.5 * (small + currentSize);
            LPoint *tempArray = (LPoint *)realloc(totalPolygon, newCapacity * sizeof(LPoint));
            totalPolygon = tempArray;
            capacity = newCapacity;
        }

        for (j = 0; j < small; j++)
        {
            totalPolygon[currentSize] = partSmall[j];
            currentSize++;
        }

        if (currentSize > 3 &&
            LPolygon_New(Cell_Now, outputLayer, totalPolygon, currentSize - 1) != NULL)
            PublishBoundaryHelpers(Cell_Now, helperCenters, helperCount, referenceRadius);
        free(totalPolygon);
    }

    double CalculateShortArcSweep(double start_angle, double end_angle)
    {
        double diff = end_angle - start_angle;

        // Wrap the signed sweep to the shortest arc.
        diff = NormalizeAngleDegrees(diff);
        return diff;
    }

    int SampleCircularArc(long center_x, long center_y, long radius,
                             double start_angle_deg, double end_angle_deg,
                             LPoint **result)
    {
        // LPoint result;
        double diff = CalculateShortArcSweep(start_angle_deg, end_angle_deg);
        double curve = diff * (M_PI / 180.0) * radius;
        int num_segments = (int)abs(curve / 100); // Sample at approximately 100 nm along the arc.
        *result = (LPoint *)malloc(num_segments * sizeof(LPoint));
        int i;
        for (i = 0; i < num_segments; ++i)
        {
            double t = (double)i / num_segments;

            double current_angle_deg = start_angle_deg + diff * t;
            double current_angle_rad = current_angle_deg * (M_PI / 180.0);

            (*result)[i].x = (long)(center_x + radius * cos(current_angle_rad));
            (*result)[i].y = (long)(center_y + radius * sin(current_angle_rad));
        }
        return num_segments;
    }

    LObject *ReadBoundaryCircleSelection(int *count)
    {
        LSelection selection = LSelection_GetList();
        *count = CountSelectedObjects(selection);
        if (*count == 0)
            return NULL;

        LObject *objects = malloc((size_t)*count * sizeof(*objects));
        if (objects == NULL)
            return NULL;
        int i;
        for (i = 0; i < *count; i++, selection = LSelection_GetNext(selection))
        {
            objects[i] = LSelection_GetObject(selection);
            if (objects[i] == NULL || LObject_GetShape(objects[i]) != LCircle)
            {
                free(objects);
                return NULL;
            }
        }
        return objects;
    }

    void DrawArbitraryCircleBoundary(void)
    {
        LCell cell = LCell_GetVisible();
        if (cell == NULL)
            return;
        int count;
        LObject *circles = ReadBoundaryCircleSelection(&count);
        if (circles == NULL)
        {
            LDialog_AlertBox("Select one or more circles.");
            return;
        }

        LDialogItem fields[] = {{"Distance from circle edge (um)", "1"},
                                {"Smoothing radius (um)", "1"}};
        if (!LDialog_MultiLineInputBox("Arbitrary circle boundary", fields, 2))
        {
            free(circles);
            return;
        }
        LFile file = LCell_GetFile(cell);
        LCoord distance, smoothing;
        if (!ReadLayoutDistance(file, fields[0].value, &distance) ||
            !ReadLayoutDistance(file, fields[1].value, &smoothing))
        {
            free(circles);
            LDialog_AlertBox("Enter nonnegative distances and smoothing radius.");
            return;
        }

        BoundaryDisk *disks = malloc((size_t)count * sizeof(*disks));
        if (disks == NULL)
        {
            free(circles);
            LDialog_AlertBox("Could not allocate the circle boundary.");
            return;
        }
        int i, valid = 1;
        for (i = 0; i < count; i++)
        {
            LPoint center = LCircle_GetCenter(circles[i]);
            double radius = (double)LCircle_GetRadius(circles[i]) + distance;
            if (radius <= 0 || fabs((double)center.x) + radius > WORLD_MAX ||
                fabs((double)center.y) + radius > WORLD_MAX)
            {
                valid = 0;
                break;
            }
            disks[i].center = OutlinePoint(center.x, center.y);
            disks[i].radius = (LCoord)radius;
        }
        free(circles);
        if (!valid)
        {
            free(disks);
            LDialog_AlertBox("A grown circle exceeds the layout coordinate range.");
            return;
        }

        LLayer layer = PickOutputLayer(cell);
        if (layer == NULL)
        {
            free(disks);
            return;
        }

        BoundaryPieces pieces = {0};
        for (i = 0; i < count && valid; i++)
        {
            LPoint center = LPoint_Set((LCoord)disks[i].center.x, (LCoord)disks[i].center.y);
            valid = RecordBoundaryPiece(cell, &pieces, LCircle_New(cell, layer, center, disks[i].radius));
        }

        int patches = 0, openGaps = 0;
        if (valid)
            valid = AddBoundaryFilletPatches(cell, layer, &pieces, disks, count, smoothing, &patches, &openGaps);
        free(disks);

        /* Union the temporary disks and patches. The source circles never enter
           the Boolean operation. Disconnected components and holes are retained. */
        if (valid && pieces.count > 1)
            valid = LCell_BooleanOperation(cell, LBoolOp_OR, 0, pieces.objects, pieces.count,
                                           NULL, 0, layer, LFALSE) == LStatusOK;
        if (!valid || pieces.count > 1)
        {
            unsigned int piece;
            for (piece = 0; piece < pieces.count; piece++)
                LObject_Delete(cell, pieces.objects[piece]);
        }
        free(pieces.objects);
        LDisplay_Refresh();

        if (!valid)
            LDialog_AlertBox("Could not create the arbitrary circle boundary.");
        else if (openGaps > 0)
        {
            char message[150];
            snprintf(message, sizeof(message),
                     "Added %d tangent arcs. Left %d nearby gaps open for manual editing.",
                     patches, openGaps);
            LDialog_AlertBox(message);
        }
    }

    void DrawRegularBoundary(void)
    {
        LCell cell = LCell_GetVisible();
        if (cell == NULL)
            return;
        int count;
        LObject *objects = ReadBoundaryCircleSelection(&count);
        if (objects == NULL || count < 3)
        {
            free(objects);
            LDialog_AlertBox("Select at least three boundary circles.");
            return;
        }

        const char *types[] = {"Circular", "Rounded hexagonal"};
        int type = LDialog_PickList("Regular boundary shape", types, 2, 0);
        if (type >= 0 && type < 2)
        {
            LFile file = LCell_GetFile(cell);
            LDialogItem fields[] = {{"Inner distance (um)", "3.9"},
                                    {"Outer distance (um)", "2.3"},
                                    {"Reference radius (um)", ""},
                                    {"Draw helper circles (0 or 1)", "0"},
                                    {"Corner radius (um), -1 = auto", "-1"}};
            snprintf(fields[2].value, sizeof(fields[2].value), "%.9g",
                     LFile_IntUtoMicrons(file, LCircle_GetRadius(objects[0])));
            if (LDialog_MultiLineInputBox("Regular boundary parameters", fields, type == 0 ? 4 : 5))
            {
                LCoord inner, outer, reference, corner = -1;
                int drawHelpers = strcmp(fields[3].value, "1") == 0;
                if (!drawHelpers && strcmp(fields[3].value, "0") != 0)
                    LDialog_AlertBox("Enter 0 or 1 for Draw helper circles.");
                else if (!ReadLayoutDistance(file, fields[0].value, &inner) ||
                         !ReadLayoutDistance(file, fields[1].value, &outer) ||
                         !ReadLayoutDistance(file, fields[2].value, &reference) || reference == 0 ||
                         (type == 1 && strcmp(fields[4].value, "-1") != 0 &&
                          !ReadLayoutDistance(file, fields[4].value, &corner)))
                {
                    LDialog_AlertBox("Enter valid distances and a positive reference radius.");
                }
                else
                {
                    LLayer layer = PickOutputLayer(cell);
                    if (layer != NULL)
                    {
                        LPoint *helperCenters = drawHelpers ? malloc((size_t)count * sizeof(*helperCenters)) : NULL;
                        if (drawHelpers && helperCenters == NULL)
                            LDialog_AlertBox("Could not allocate helper circles. Drawing the boundary only.");
                        if (type == 0)
                            OrderCirclesAndDrawContour(layer, objects, count, inner, outer, reference, helperCenters);
                        else
                            OrderCirclesAndDrawRoundedHexagonContour(layer, objects, count, inner, outer,
                                                                    reference, corner, helperCenters);
                        free(helperCenters);
                        LSelection_DeselectAll();
                        LDisplay_Refresh();
                    }
                }
            }
        }
        free(objects);
    }

    void OrderCirclesAndDrawRoundedHexagonContour(LLayer outputLayer, LObject * objects, int objectNum, long innerDistance, long outerDistance, long referenceRadius, long RoundRadius, LPoint *helperCenters)
    {

        // put the left bottom circle to the first
        int i;
        for (i = 1; i < objectNum; i++)
        {
            LPoint center1 = LCircle_GetCenter(objects[0]);
            LPoint center2 = LCircle_GetCenter(objects[i]);
            if (center1.x > center2.x || (center1.x == center2.x && center1.y > center2.y))
            {
                LObject temp = objects[0];
                objects[0] = objects[i];
                objects[i] = temp;
            }
        }
        // get the minimun circle distance
        double minDistance = WORLD_MAX;
        for (i = 1; i < objectNum; i++)
        {
            LPoint center1 = LCircle_GetCenter(objects[0]);
            LPoint center2 = LCircle_GetCenter(objects[i]);
            double distance = DistanceBetweenPoints(center1, center2);
            if (distance < minDistance)
            {
                minDistance = distance;
            }
        }

        double search_max_radius = minDistance + 100;
        double search_min_radius = minDistance - 100;

        //  create a new array to store the ordered circles
        LObject *orderedObjects = (LObject *)malloc(objectNum * sizeof(LObject));
        orderedObjects[0] = objects[0];
        int orderedObjectNum = 1;
        bool findSatus = FindNextBoundaryCircle(orderedObjects, orderedObjectNum, objects, objectNum, search_max_radius, search_min_radius);
        if (findSatus)
        {
            LDialog_MsgBox("Order Success");
        }
        else
        {
            LDialog_MsgBox("Order Fail");
            return;
        }
        // calcu
        LPoint *centerPoints;
        centerPoints = (LPoint *)malloc((objectNum) * sizeof(LPoint));
        ReadCircleCenters(objects, objectNum, centerPoints);
        LPoint center;
        center = CalculateMeanPoint(centerPoints, objectNum);

        CreateRoundedHexagonContour(outputLayer, orderedObjects, objectNum, innerDistance, outerDistance, referenceRadius, (long)minDistance, center, RoundRadius, helperCenters);
    }

    /* Follow the same boundary order while sampling rounded hexagonal pieces. */
    void CreateRoundedHexagonContour(LLayer outputLayer, LObject * orderedObjects, int orderedObjectNum, long innerDistance, long outerDistance, long referenceRadius, long minDistance, LPoint shapeCenter, long RoundRadius, LPoint *helperCenters)
    {
        // referenceRadius means the radius of the circle
        LCell Cell_Now = LCell_GetVisible();

        double startAngle = -135; // start from left bottom circle
        double angle1, angle2;
        int i, j, helperCount = 0;
        long currentSize = 0;
        long capacity = 1000;
        LPoint *totalPolygon = (LPoint *)malloc(capacity * sizeof(LPoint));
        if (totalPolygon == NULL)
        {
            LDialog_AlertBox("Could not allocate the boundary polygon.");
            return;
        }
        for (i = 0; i < orderedObjectNum; i++)
        {
            LObject circle_1 = orderedObjects[i];
            LObject circle_2 = orderedObjects[(i + 1) % orderedObjectNum];
            LObject circle_3 = orderedObjects[(i + 2) % orderedObjectNum];
            LPoint center_1 = LCircle_GetCenter(circle_1);
            LPoint center_2 = LCircle_GetCenter(circle_2);
            LPoint center_3 = LCircle_GetCenter(circle_3);

            double delta_X = minDistance / 2.0;
            double delta_Y = ((referenceRadius + innerDistance) * 2.0 - minDistance / 2.0) * tan(M_PI / 6) + (referenceRadius + outerDistance) / cos(M_PI / 6);
            double delta_L = sqrt(delta_X * delta_X + delta_Y * delta_Y);

            LPoint results1[2], results2[2], results3[2];
            int resultNum1, resultNum2, resultNum3;
            resultNum1 = FindEquidistantPoints(center_1, center_2, delta_L, results1);
            resultNum2 = FindEquidistantPoints(center_2, center_3, delta_L, results2);

            double sp_delta_Y = minDistance / 2.0 * sqrt(3);
            double sp_L = minDistance / 2.0 * sqrt(3) / cos(M_PI / 6);
            double sp_x1 = (2 * referenceRadius + innerDistance + outerDistance - sp_L) / sin(M_PI / 6);
            double sp_x2 = minDistance / 2.0 * sqrt(3) * tan(M_PI / 6);
            double sp_delta_X = sp_x1 + sp_x2;
            delta_L = sqrt(sp_delta_X * sp_delta_X + sp_delta_Y * sp_delta_Y);
            resultNum3 = FindEquidistantPoints(center_1, center_3, delta_L, results3);

            LPoint result1, result2, result3;
            if (resultNum1 == 0 || resultNum2 == 0 || resultNum3 == 0)
                continue;
            if (resultNum1 == 1)
                result1 = results1[0];
            if (resultNum1 == 2)
            {
                result1 = DistanceBetweenPoints(shapeCenter, results1[0]) > DistanceBetweenPoints(shapeCenter, results1[1]) ? results1[0] : results1[1];
            }
            if (resultNum2 == 1)
                result2 = results2[0];
            if (resultNum2 == 2)
            {
                result2 = DistanceBetweenPoints(shapeCenter, results2[0]) > DistanceBetweenPoints(shapeCenter, results2[1]) ? results2[0] : results2[1];
            }
            if (resultNum3 == 1)
                result3 = results3[0];
            if (resultNum3 == 2)
            {
                result3 = DistanceBetweenPoints(shapeCenter, results3[0]) > DistanceBetweenPoints(shapeCenter, results3[1]) ? results3[0] : results3[1];
            }

            LPoint *part1;
            LPoint *part2;

            if (DistanceBetweenPoints(result1, result2) < minDistance * 0.9)
            {


                angle1 = CalculateBearingDegrees(center_1, result3);
                angle2 = CalculateBearingDegrees(center_3, result3);
                double _angle1 = NormalizeAngleDegrees(angle1 + 180);
                double _angle2 = NormalizeAngleDegrees(angle2 + 180);
                // start - >angle1
                //_angle1 -> _angle2
                //
                int cnt1 = SampleRoundedHexagonArc(center_1, referenceRadius + innerDistance, startAngle, angle1, &part1, RoundRadius + innerDistance);
                int cnt2 = SampleRoundedHexagonArc(result3, referenceRadius + outerDistance, _angle1, _angle2, &part2, RoundRadius + outerDistance);
                if (capacity < 1.5 * (cnt1 + cnt2 + currentSize))
                {
                    long newCapacity = (long)(1.5 * (cnt1 + cnt2 + currentSize));
                    LPoint *tempArray = (LPoint *)realloc(totalPolygon, newCapacity * sizeof(LPoint));
                    totalPolygon = tempArray;
                    capacity = newCapacity;
                }

                for (j = 0; j < cnt1; j++)
                {
                    totalPolygon[currentSize] = part1[j];
                    currentSize++;
                }

                for (j = 0; j < cnt2; j++)
                {
                    totalPolygon[currentSize] = part2[j];
                    currentSize++;
                }

                if (helperCenters != NULL)
                    helperCenters[helperCount++] = result3;
                startAngle = angle2;
                i++;
                continue;
                // special area
            }
            else
            {
                angle1 = CalculateBearingDegrees(center_1, result1);
                angle2 = CalculateBearingDegrees(center_2, result1);
                double _angle1 = NormalizeAngleDegrees(angle1 + 180);
                double _angle2 = NormalizeAngleDegrees(angle2 + 180);

                int cnt3 = SampleRoundedHexagonArc(center_1, referenceRadius + innerDistance, startAngle, angle1, &part1, RoundRadius + innerDistance);
                int cnt4 = SampleRoundedHexagonArc(result1, referenceRadius + outerDistance, _angle1, _angle2, &part2, RoundRadius + outerDistance);
                if (capacity < 1.5 * (cnt3 + cnt4 + currentSize))
                {
                    int newCapacity = 1.5 * (cnt3 + cnt4 + currentSize);
                    LPoint *tempArray = (LPoint *)realloc(totalPolygon, newCapacity * sizeof(LPoint));
                    totalPolygon = tempArray;
                    capacity = newCapacity;
                }

                for (j = 0; j < cnt3; j++)
                {
                    totalPolygon[currentSize] = part1[j];
                    currentSize++;
                }

                for (j = 0; j < cnt4; j++)
                {
                    totalPolygon[currentSize] = part2[j];
                    currentSize++;
                }
                if (helperCenters != NULL)
                    helperCenters[helperCount++] = result1;
                startAngle = angle2;
            }

        }

        LPoint *partSmall;
        LPoint circle0 = LCircle_GetCenter(orderedObjects[0]);
        long radius0 = LCircle_GetRadius(orderedObjects[0]);
        int small = SampleRoundedHexagonArc(circle0, radius0 + innerDistance, startAngle, -135.0, &partSmall, RoundRadius + innerDistance);

        if (capacity < small + currentSize)
        {
            long newCapacity = (long)(1.5 * (small + currentSize));
            LPoint *tempArray = (LPoint *)realloc(totalPolygon, newCapacity * sizeof(LPoint));
            totalPolygon = tempArray;
            capacity = newCapacity;
        }

        for (j = 0; j < small; j++)
        {
            totalPolygon[currentSize] = partSmall[j];
            currentSize++;
        }

        if (currentSize > 3 &&
            LPolygon_New(Cell_Now, outputLayer, totalPolygon, currentSize - 1) != NULL)
            PublishBoundaryHelpers(Cell_Now, helperCenters, helperCount, referenceRadius);
        free(totalPolygon);
    }

    int SampleRoundedHexagonArc(LPoint center, LCoord R, double start_angle_deg, double end_angle_deg,
                                         LPoint **result, long RoundRadius)
    {
        double r = 1 / 3.7 * R; // default value
        if (RoundRadius > 0)
        {
            r = RoundRadius;
        }
        double curve = r * M_PI / 3;
        int n_coner = curve / 20; // Sample at approximately 20 nm along each corner.
        int n_sum = 6 * n_coner;
        double center_distance = ((double)R - r) * 2 / 3 * sqrt(3);
        *result = (LPoint *)malloc(n_sum * sizeof(LPoint));

        if (*result == NULL)
        {
            LDialog_AlertBox("Memory allocation failed for points array.");
            return 0;
        }

        double diff = CalculateShortArcSweep(start_angle_deg, end_angle_deg);
        double delta_theta = M_PI / 3 / n_coner * abs(diff) / diff;
        int i, j;
        long count = 0;

        for (i = 0; i < n_sum; i++)
        {
            double theta = (i % n_coner) * delta_theta;
            int n = i / n_coner;
            double new_center_x = center.x + center_distance * cos(M_PI / 6 + n * M_PI / 3);
            double new_center_y = center.y + center_distance * sin(M_PI / 6 + n * M_PI / 3);
            double start_theta = diff > 0 ? n * M_PI / 3 : (n + 1) * M_PI / 3;

            long x, y;
            x = (long)(new_center_x + r * cos(start_theta + theta));
            y = (long)(new_center_y + r * sin(start_theta + theta));

            LPoint p;
            p.x = x;
            p.y = y;
            (*result)[i] = p;
            count++;
        }

        // Filter the samples to the requested angular interval.
        long filtered_count = 0;
        for (i = 0; i < count; i++)
        {
            double angle = CalculateBearingDegrees(center, (*result)[i]);
            angle = NormalizeAngleDegrees(angle);
            double start_angle_norm = NormalizeAngleDegrees(start_angle_deg);
            double end_angle_norm = NormalizeAngleDegrees(end_angle_deg);

            bool is_between;
            if (diff > 0) // Clockwise.
            {
                if (start_angle_norm < end_angle_norm)
                    is_between = (angle >= start_angle_norm && angle <= end_angle_norm);
                else
                    is_between = (angle >= start_angle_norm || angle <= end_angle_norm);
            }
            else // Counterclockwise.
            {
                if (start_angle_norm > end_angle_norm)
                    is_between = (angle <= start_angle_norm && angle >= end_angle_norm);
                else
                    is_between = (angle <= start_angle_norm || angle >= end_angle_norm);
            }

            if (is_between)
            {
                (*result)[filtered_count] = (*result)[i];
                filtered_count++;
            }
        }

        // Sort the retained points by distance along the signed sweep, including across the angular seam.
        for (i = 0; i < filtered_count - 1; i++)
        {
            for (j = 0; j < filtered_count - i - 1; j++)
            {
                double angle1 = CalculateBearingDegrees(center, (*result)[j]);
                double angle2 = CalculateBearingDegrees(center, (*result)[j + 1]);
                angle1 = NormalizeAngleDegrees(angle1);
                angle2 = NormalizeAngleDegrees(angle2);

                bool need_swap;
                if (diff > 0) // Clockwise.
                {
                    if (angle1 < angle2)
                        need_swap = (angle2 - angle1 > 180);
                    else
                        need_swap = (angle1 - angle2 < 180);
                }
                else // Counterclockwise.
                {
                    if (angle1 > angle2)
                        need_swap = (angle1 - angle2 > 180);
                    else
                        need_swap = (angle2 - angle1 < 180);
                }

                if (need_swap)
                {
                    LPoint temp = (*result)[j];
                    (*result)[j] = (*result)[j + 1];
                    (*result)[j + 1] = temp;
                }
            }
        }

        return filtered_count;
    }

    double CalculateVectorAngleCosine(LPoint point1, LPoint point2)
    {
        double x1 = point1.x * 1.0;
        double y1 = point1.y * 1.0;
        double x2 = point2.x * 1.0;
        double y2 = point2.y * 1.0;
        double theta = (x1 * x2 + y1 * y2) / (sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2));
        return theta;
    }

    void RegisterContourMacros(void)
    {
        LMacro_Register("DrawTangentRing_func", "DrawTangentRing");
        LMacro_Register("Select_Boundry_Circles_func", "SelectBoundaryCircles");
        LMacro_Register("Draw_Regular_Boundary_func", "DrawRegularBoundary");
        LMacro_Register("Draw_Arbitrary_Circle_Boundary_func", "DrawArbitraryCircleBoundary");
    }
} /* End of ContourMacros. */

RegisterContourMacros();
