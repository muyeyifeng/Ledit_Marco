/*
 * Special_Polygon_Module
 * Special_Polygon_Module Marco for Ledit
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
module Special_Polygon_Module
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Ledit_Special.h"
#include "ldata.h" /* Main UPI header. */

    void DrawTangentRing()
    {
        LCell Cell_Now = LCell_GetVisible();
        LSelection selectedInital = LSelection_GetList();
        int selectedObjectNumber = LSelection_GetNumber(selectedInital);
        if (selectedObjectNumber != 2)
        {
            LDialog_AlertBox("Just Support 2 Object");
            return;
        }

        LObject object1 = LSelection_GetObject(selectedInital);
        selectedInital = LSelection_GetNext(selectedInital);
        LObject object2 = LSelection_GetObject(selectedInital);

        //****************************Input Params****************************//
        LDialogItem Dialog_Items[2] = {{"Tangent Circle InnerRadius (um)", "1"},
                                       {"Tangent Circle OuterRadius-InnerRadius (um)", "1"}};
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
        //****************************Input Params****************************//

        long *x1;
        long *y1;
        long *x2;
        long *y2;
        LObject *torus;
        int rootPoints = 0;
        if (LObject_GetShape(object1) == 1 && LObject_GetShape(object2) == 1)
        {
            rootPoints = CalculateIntersection_Circle2Circle(object1, object2, radius, &torus);
        }
        if ((LObject_GetShape(object1) == 1 && LObject_GetShape(object2) == 2) || (LObject_GetShape(object1) == 2 && LObject_GetShape(object2) == 1))
        {
            rootPoints = CalculateIntersection_Circle2Wire(object1, object2, radius, &torus);
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

    int CalculateIntersection_Circle2Wire(LObject object1, LObject object2, long radius, LObject **torus)
    {
        *torus = (LObject *)malloc(4 * sizeof(LObject));
        LObject circle = LObject_GetShape(object1) == 1 ? object1 : object2;
        LObject wire = LObject_GetShape(object1) == 2 ? object1 : object2;

        LCell Cell_Now = LCell_GetVisible();
        // LFile File_Now = LCell_GetFile(Cell_Now);
        // LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

        LLayer LLayer_Now = LObject_GetLayer(Cell_Now, circle);

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
        FindLineEquation(point1, point2, &abc);
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
            LDialog_AlertBox(LFormat("No root: distance: %f, r +  2 * radius: %d", distance, r + 2 * radius));
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

        (*torus)[0] = LTorus_CreateNew(Cell_Now, LLayer_Now, &tParams1);
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

        (*torus)[1] = LTorus_CreateNew(Cell_Now, LLayer_Now, &tParams2);
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

            (*torus)[2] = LTorus_CreateNew(Cell_Now, LLayer_Now, &tParams3);

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

            (*torus)[3] = LTorus_CreateNew(Cell_Now, LLayer_Now, &tParams4);
            return 4;
        }
        return 2;
    }

    int CalculateIntersection_Circle2Circle(LObject object1, LObject object2, long radius, LObject **torus)
    {
        *torus = (LObject *)malloc(2 * sizeof(LObject));
        LCell Cell_Now = LCell_GetVisible();
        // LFile File_Now = LCell_GetFile(Cell_Now);
        // LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

        LLayer LLayer_Now = LObject_GetLayer(Cell_Now, object1);

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

            (*torus)[0] = LTorus_CreateNew(Cell_Now, LLayer_Now, &tParams1);
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

            (*torus)[1] = LTorus_CreateNew(Cell_Now, LLayer_Now, &tParams2);
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

    void FindLineEquation(LPoint p1, LPoint p2, double **abc)
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

    bool CircleCorssWire(LPoint p1, LPoint p2, LObject circleLike)
    {
        LShapeType objectShape = LObject_GetShape(circleLike);
        switch (objectShape)
        {
            case 1: // LCircle
            {
                return;
            }
            case 4: // LTorus
            {
                return;
            }
            case 5: // LPie
            {
                return;
            }
        }
    }

    double GetPointDistance(LPoint point1, LPoint point2)
	{
		return sqrt((double)(point1.x - point2.x) * (double)(point1.x - point2.x) + (double)(point1.y - point2.y) * (point1.y - point2.y));
	}

    LObject* Select_Boundry_Object(LSelection selectedInital, long search_radius)
	{
		//LSelection selectedInital = LSelection_GetList();
        while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);
            if(selectedShapeType != 1)
                return;
			selectedInital = LSelection_GetNext(selectedInital);
		}

        selectedInital = LSelection_GetList();
        int objectNum = LSelection_GetNumber(selectedInital);
        LObject* objects = (LObject *)malloc(objectNum * sizeof(LObject));

        int i, j;
        for(i = 0; i < objectNum; i++)
        {
            objects[i] = LSelection_GetObject(selectedInital);
            selectedInital = LSelection_GetNext(selectedInital);
        }

        LSelection_DeselectAll();
        
        LObject* boundryObjects = (LObject *)malloc(objectNum * sizeof(LObject));
        int boundryObjectNum = 0;
        for(i = 0; i < objectNum; i++)
        {
            int counter = 0;
            LObject object1 = objects[i];
            LPoint center1 = LCircle_GetCenter(object1);
            for(j = 0; j < objectNum; j++)
            {
                LObject object2 = objects[j];
                LPoint center2 = LCircle_GetCenter(object2);
                if(GetPointDistance(center1, center2) < search_radius){
                    counter++;
                }
            }
            if(counter < 7){
                boundryObjects[boundryObjectNum] = object1;
                boundryObjectNum++;
                LSelection_AddObject(object1);
            }
            counter = 0;
        }
		LDisplay_Refresh();
	}

    void Select_Boundry_Circles()
    {
        //****************************Input Params****************************//
		LDialogItem Dialog_Items[1] = {	{"Radius (um) (Inf)", "0"}};
		double search_radius;
		if (LDialog_MultiLineInputBox("Select Boundry Circles", Dialog_Items, 1))
		{
			search_radius = (long)(atof(Dialog_Items[0].value) * 1000); // get the Scale Factor
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
        LSelection selectedInital = LSelection_GetList();
        Select_Boundry_Object(selectedInital, search_radius);
    }

    bool Find_Next_Circle(LObject* orderedObjects, int orderedObjectNum, LObject* objects, int objectNum, double search_max_radius, double search_min_radius)
    {
        int i;
        LObject thisObject = orderedObjects[orderedObjectNum - 1];
        LPoint center1 = LCircle_GetCenter(thisObject);
        for( i = 0; i < objectNum; i++)
        {
            LPoint center2 = LCircle_GetCenter(objects[i]);
            if(GetPointDistance(center1, center2) < search_max_radius && GetPointDistance(center1, center2) > search_min_radius)
            {
                orderedObjects[orderedObjectNum] = objects[i];
                orderedObjectNum++;
                if(Find_Next_Circle(orderedObjects, orderedObjectNum, objects, objectNum, search_max_radius, search_min_radius))
                {
                    return true;
                }
                else
                {
                    orderedObjectNum--;
                }
            }
            if(orderedObjectNum == objectNum)
            {
                LPoint firstObjectCenter = LCircle_GetCenter(orderedObjects[0]);
                LPoint lastObjectCenter = LCircle_GetCenter(orderedObjects[orderedObjectNum - 1]);
                double distance = GetPointDistance(firstObjectCenter, lastObjectCenter);
                if(distance < search_max_radius && distance > search_min_radius)
                {
                    return true;
                }
                else
                {
                    return false;
                }
                
            }
        }
        return false;
    }

    void Order_Circle(LObject* objects, int objectNum)
    {
        
        //put the left bottom circle to the first
        int i;
        for( i = 1; i < objectNum; i++)
        {
            LPoint center1 = LCircle_GetCenter(objects[0]);
            LPoint center2 = LCircle_GetCenter(objects[i]);
            if(center1.x > center2.x || (center1.x == center2.x && center1.y > center2.y))
            {
                LObject temp = objects[0];
                objects[0] = objects[i];
                objects[i] = temp;
            }
        }
        
        //get the minimun circle distance
        double minDistance = WORLD_MAX;
        for(i = 1; i < objectNum; i++)
        {
            LPoint center1 = LCircle_GetCenter(objects[0]);
            LPoint center2 = LCircle_GetCenter(objects[i]);
            double distance = GetPointDistance(center1, center2);
            if(distance < minDistance)
            {
                minDistance = distance;
            }
        }

        double search_max_radius = minDistance * 1.2;
        double search_min_radius = minDistance * 0.8;

        // create a new array to store the ordered circles
        LObject* orderedObjects = (LObject *)malloc(objectNum * sizeof(LObject));
        orderedObjects[0] = objects[0];
        int orderedObjectNum = 1;
        Find_Next_Circle(orderedObjects, orderedObjectNum, objects, objectNum, search_max_radius, search_min_radius);
        
        //deselect all objects
        LSelection_DeselectAll();
        // test: change the size of the selected objects by order
        LCell Cell_Now = LCell_GetVisible();
        for(i = 0; i < objectNum; i++)
        {   
            LObject object = orderedObjects[i];
            LPoint center = LCircle_GetCenter(object);
            long radius = LCircle_GetRadius(object);
            radius = radius + 1000*i;
            LCircle_Set(Cell_Now, object, center, radius);
        }
    }

    void Order_Boundry_Circles()
    {
        //****************************Input Params****************************//
		// LDialogItem Dialog_Items[1] = {	{"Radius (um) (Inf)", "0"}};
		// double search_radius;
		// if (LDialog_MultiLineInputBox("Select Boundry Circles", Dialog_Items, 1))
		// {
		// 	search_radius = (long)(atof(Dialog_Items[0].value) * 1000); // get the Scale Factor
		// }
		// else
		// {
		// 	return;
		// }
		//****************************Input Params****************************//
        LSelection selectedInital = LSelection_GetList();
        while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);
            if(selectedShapeType != 1)
                return;
			selectedInital = LSelection_GetNext(selectedInital);
		}

        selectedInital = LSelection_GetList();
        int objectNum = LSelection_GetNumber(selectedInital);
        LObject* objects = (LObject *)malloc(objectNum * sizeof(LObject));

        int i, j;
        for(i = 0; i < objectNum; i++)
        {
            objects[i] = LSelection_GetObject(selectedInital);
            selectedInital = LSelection_GetNext(selectedInital);
        }
        Order_Circle(objects, objectNum);
    }


    void SpecialPolygon_func(void)
    {
        LMacro_Register("DrawTangentRing_func", "DrawTangentRing");
        LMacro_Register("Select_Boundry_Circles_func", "Select_Boundry_Circles");
        LMacro_Register("Order_Boundry_Circles_func", "Order_Boundry_Circles");
    }
} /* end of module Special_Polygon_Module */

SpecialPolygon_func();
