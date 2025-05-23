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
    //#include "Ledit_Special.h"
    #include "ldata.h" /* Main UPI header. */

    
    void DrawTangentRing();
    int CalculateIntersection_Circle2Wire(LObject object1, LObject object2, long radius, LObject **torus);
    int CalculateIntersection_Circle2Circle(LObject object1, LObject object2, long radius, LObject **torus);
    double CalculatePolarAngle(LPoint p1, LPoint p2);
    int LSelection_GetNumber(LSelection selectedInital);
    void FindLineEquation(LPoint p1, LPoint p2, double **abc);
    bool CircleCorssWire(LPoint p1, LPoint p2, LObject circleLike);
    double GetPointDistance(LPoint point1, LPoint point2);
    LObject* Select_Boundry_Object(LSelection selectedInital, long search_radius);
    void Select_Boundry_Circles();
    bool Find_Next_Circle(LObject* orderedObjects, int orderedObjectNum, LObject* objects, int objectNum, double search_max_radius, double search_min_radius);
    LPoint CalculateCentroid(LPoint * points, int n);
    void Order_Circle(LObject* objects, int objectNum, long innerDistance, long outterDistance, long standerRadius);
    int Calculate_Outside_Circle(LPoint A, LPoint B, long L, LPoint* results);
    bool CheckIsCircles(LObject* objects, int objectNum, LPoint * centerPoints);
    double calculateAngleP1ToP2(LPoint P1, LPoint P2);
    double normalize_angle(double angle);
    void Draw_Boundry(LObject* orderedObjects, int orderedObjectNum, long innerDistance, long outterDistance, long standerRadius, long minDistance, LPoint shapeCenter);
    double get_short_arc_interpolated_angle(double start_angle, double end_angle);
    int calculate_arc_points(long center_x, long center_y, long radius, double start_angle_deg, double end_angle_deg, LPoint **result);
    void Order_Boundry_Circles();


    void SpecialPolygon_func(void);
    /**
     * @brief Draws a tangent ring on the current drawing context.
     *
     * This function is responsible for rendering a ring that is tangent to a specified
     * shape or set of points. The exact behavior depends on the implementation details
     * within the function body.
     *
     * Typical use cases include visualizing geometric constructions or highlighting
     * tangency relationships in polygonal shapes.
     *
     * @note Ensure that the necessary drawing context and parameters are set up before
     * calling this function.
     */
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

    /**
     * @brief Calculates the intersection points between a circle and a wire.
     *
     * This function computes the intersection(s) between a circle (defined by one object and a radius)
     * and a wire (defined by another object). The results are stored in the torus parameter.
     *
     * @param object1 The first LObject, representing the circle's center or definition.
     * @param object2 The second LObject, representing the wire.
     * @param radius The radius of the circle.
     * @param torus Pointer to a pointer where the resulting intersection objects will be stored.
     * @return int Returns the number of intersection points found, or a negative value on error.
     */
    int CalculateIntersection_Circle2Wire(LObject object1, LObject object2, long radius, LObject **torus)
    {
        *torus = (LObject *)malloc(4 * sizeof(LObject));
        LObject circle = LObject_GetShape(object1) == 1 ? object1 : object2;
        LObject wire = LObject_GetShape(object1) == 2 ? object1 : object2;

        LCell Cell_Now = LCell_GetVisible();
        // LFile File_Now = LCell_GetFile(Cell_Now);
        // LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

        LLayer LLayer_Now = LObject_GetLayer(Cell_Now, circle);

        /**
         * Checks if the number of vertices in the 'wire' object is not equal to 2.
         *
         * This condition is typically used to ensure that the 'wire' object represents
         * a valid segment or edge, which should consist of exactly two vertices.
         * If the count is not 2, the subsequent code may handle this as an error or
         * special case.
         *
         * @param wire The object whose vertices are being counted.
         * @return True if the number of vertices is not 2, false otherwise.
         */
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
        /**
         * Checks if the distance between two points is greater than the sum of 'r' and twice 'radius',
         * and also verifies that 'r' is less than 'distance'.
         *
         * This condition can be used, for example, to determine if two circles (or a circle and another shape)
         * are sufficiently far apart, considering their radii and an additional margin.
         *
         * @param distance The measured distance between two points or objects.
         * @param r The radius or relevant size of the first object.
         * @param radius The radius or relevant size of the second object.
         * @return true if the objects are sufficiently far apart based on the condition; false otherwise.
         */
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
        /**
         * Checks if the absolute difference between two angles (rad1 and rad2) 
         * is greater than π (M_PI). This is typically used to determine if the 
         * angular distance between two points on a circle exceeds half a rotation.
         *
         * @param rad1 The first angle in radians.
         * @param rad2 The second angle in radians.
         * @return true if the absolute difference is greater than π, false otherwise.
         */
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
        /**
         * Checks if the value of 'r' is greater than 'distance'.
         *
         * This conditional statement is typically used to determine whether a certain
         * radius or measurement ('r') exceeds a specified threshold ('distance').
         *
         * @param r The value to compare, often representing a radius or length.
         * @param distance The threshold value to compare against.
         * @return true if 'r' is greater than 'distance', false otherwise.
         */
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
            /**
             * Checks if the absolute difference between rad5 and rad6 is greater than π (M_PI).
             * This is typically used to determine if the angular distance between two angles
             * exceeds half a full rotation, which can be useful for handling angle wrapping
             * or detecting if two directions are on opposite sides of a circle.
             *
             * @param rad5 First angle in radians.
             * @param rad6 Second angle in radians.
             * @return true if the absolute difference is greater than π, false otherwise.
             */
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
            /**
             * Checks if the absolute difference between rad7 and rad8 exceeds π (M_PI).
             * This is typically used to determine if the angular difference between two angles
             * (in radians) crosses the -π/π boundary, which is important in angle normalization
             * or when handling circular measurements.
             *
             * @param rad7 First angle in radians.
             * @param rad8 Second angle in radians.
             * @return true if the absolute difference is greater than π, false otherwise.
             */
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

    /**
     * @brief Calculates the intersection points of two circles and creates torus objects at those points.
     *
     * This function computes the intersection points between two circles defined by `object1` and `object2`,
     * each with their own center and radius. An additional `radius` parameter is added to both circles' radii.
     * If the circles intersect, two torus objects are created at the intersection points and returned via the `torus` pointer.
     *
     * @param object1      The first circle object.
     * @param object2      The second circle object.
     * @param radius       The additional radius to be added to both circles.
     * @param torus        Output parameter. On success, points to an array of two LObject pointers representing the created torus objects.
     *
     * @return
     *   - 2  : If two intersection points (and torus objects) are found and created.
     *   - 0  : If there are no intersection points (the circles do not intersect).
     *   - -1 : If the circles are coincident (infinite intersection points).
     *
     * @note The caller is responsible for freeing the memory allocated for the torus array.
     */
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

        /**
         * Checks if two circles with radii R1 and R2 and center distance d do not intersect.
         *
         * The condition is true if:
         * - The distance between centers (d) is greater than the sum of the radii (R1 + R2), meaning the circles are separate.
         * - The distance between centers (d) is less than the absolute difference of the radii (|R1 - R2|), meaning one circle is completely inside the other without touching.
         */
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
            
            /**
             * Checks if the absolute difference between two angles (rad1 and rad2) is greater than π (M_PI).
             * This is typically used to determine if the angular distance between two points on a circle
             * exceeds half a revolution, which may be relevant for handling angle wrapping or special
             * polygonal calculations.
             *
             * @param rad1 First angle in radians.
             * @param rad2 Second angle in radians.
             * @return true if the absolute difference is greater than π, false otherwise.
             */
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

    /**
     * @brief Calculates the polar angle (in radians) between two points.
     *
     * This function computes the angle formed by the line segment from point p1 to point p2
     * with respect to the positive x-axis. The result is given in radians, typically in the range [-π, π].
     *
     * @param p1 The starting point (of type LPoint).
     * @param p2 The ending point (of type LPoint).
     * @return The polar angle in radians between p1 and p2.
     */
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

    /**
     * @brief Retrieves the number associated with the given selection.
     *
     * This function takes a LSelection object as input and returns an integer
     * representing the number associated with the specified selection.
     *
     * @param selectedInital The LSelection object for which the number is to be retrieved.
     * @return int The number associated with the given selection.
     */
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

    /**
     * @brief Calculates the coefficients (a, b, c) of the line equation ax + by + c = 0
     *        passing through two points p1 and p2.
     *
     * @param p1    The first point defining the line.
     * @param p2    The second point defining the line.
     * @param abc   Output parameter. Pointer to an array of three doubles where the
     *              computed coefficients (a, b, c) will be stored.
     */
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

    /**
     * @brief Determines if a line segment crosses a circle-like object.
     *
     * This function checks whether the line segment defined by points p1 and p2
     * intersects or crosses the given circle-like object.
     *
     * @param p1 The first endpoint of the line segment.
     * @param p2 The second endpoint of the line segment.
     * @param circleLike The circle-like object to check for intersection.
     * @return true if the line segment crosses the circle-like object, false otherwise.
     */
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

    /**
     * @brief Calculates the Euclidean distance between two points.
     *
     * This function computes the straight-line distance between two points
     * represented by the LPoint structure.
     *
     * @param point1 The first point.
     * @param point2 The second point.
     * @return The distance between point1 and point2 as a double.
     */
    double GetPointDistance(LPoint point1, LPoint point2)
	{
		return sqrt((double)(point1.x - point2.x) * (double)(point1.x - point2.x) + (double)(point1.y - point2.y) * (point1.y - point2.y));
	}

    /**
     * Select_Boundry_Object - Finds and returns the boundary object from a given selection.
     *
     * @param selectedInital: The initial selection of objects to search within.
     * @param search_radius: The radius within which to search for the boundary object.
     *
     * @return A pointer to the LObject representing the boundary object if found, or NULL otherwise.
     *
     * This function searches for a boundary object within the specified search radius
     * from the initial selection. It is typically used to identify the outermost or
     * edge object in a selection set.
     */
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

    /**
     * @brief Selects boundary circles in the current context.
     *
     * This function identifies and selects circles that form the boundary
     * of a given shape or region. The specific criteria for selection
     * depend on the implementation details and the data structures used
     * to represent circles and boundaries.
     *
     * Typical use cases include highlighting or processing the outermost
     * circles in a graphical or computational geometry application.
     *
     * @note Ensure that the relevant data structures are initialized before calling this function.
     */
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
        //LDialog_MsgBox(LFormat("find_level %d", orderedObjectNum));
        //debug
        if(orderedObjectNum > objectNum)return false;
        if(orderedObjectNum == objectNum)
        {
            //LDialog_MsgBox(LFormat("find_level %d", orderedObjectNum));
            LPoint firstObjectCenter = LCircle_GetCenter(orderedObjects[0]);
            //LDialog_MsgBox(LFormat("firstObjectCenter %d", firstObjectCenter.x));
            LPoint lastObjectCenter = LCircle_GetCenter(orderedObjects[orderedObjectNum-1]);
            //LDialog_MsgBox(LFormat("lastObjectCenter %d", lastObjectCenter.x));
            double distance = GetPointDistance(firstObjectCenter, lastObjectCenter);
            //LDialog_MsgBox(LFormat("distance %f", distance));
            if(distance < search_max_radius && distance > search_min_radius)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        int i,j;
        LObject thisObject = orderedObjects[orderedObjectNum - 1];
        LPoint center1 = LCircle_GetCenter(thisObject);
        for( i = 1; i < objectNum; i++)
        {
            LPoint center2 = LCircle_GetCenter(objects[i]);
            bool isExist = false;
            for( j = 0; j < orderedObjectNum; j++)
            {
                LPoint center3 = LCircle_GetCenter(orderedObjects[j]);
                if(center3.x == center2.x && center3.y == center2.y)
                {
                    isExist=true;
                    break;
                }
            }
            if(isExist) continue;
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
            // if(orderedObjectNum == objectNum)
            // {
            //     LDialog_MsgBox(LFormat("find_level %d", orderedObjectNum));
            //     LPoint firstObjectCenter = LCircle_GetCenter(orderedObjects[0]);
            //     LDialog_MsgBox(LFormat("firstObjectCenter %d", firstObjectCenter.x));
            //     LPoint lastObjectCenter = LCircle_GetCenter(orderedObjects[orderedObjectNum-1]);
            //     LDialog_MsgBox(LFormat("lastObjectCenter %d", lastObjectCenter.x));
            //     double distance = GetPointDistance(firstObjectCenter, lastObjectCenter);
            //     LDialog_MsgBox(LFormat("distance %f", distance));
            //     if(distance < search_max_radius && distance > search_min_radius)
            //     {
            //         return true;
            //     }
            //     else
            //     {
            //         return false;
            //     }
            // }
        }
        return false;
    }
    
    LPoint CalculateCentroid(LPoint * points, int n)
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

    void Order_Circle(LObject* objects, int objectNum, long innerDistance, long outterDistance, long standerRadius)
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
        //LDialog_MsgBox(LFormat("objects.x: %d, objects.y: %d", LCircle_GetCenter(objects[0]).x, LCircle_GetCenter(objects[0]).y));
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

        double search_max_radius = minDistance + 100;
        double search_min_radius = minDistance - 100;

        //LDialog_MsgBox(LFormat("search_max_radius: %f, search_min_radius: %f", search_max_radius, search_min_radius));
        // create a new array to store the ordered circles
        LObject* orderedObjects = (LObject *)malloc(objectNum * sizeof(LObject));
        orderedObjects[0] = objects[0];
        int orderedObjectNum = 1;
        //LDialog_MsgBox(LFormat("search_max_radius: %f, search_min_radius: %f", search_max_radius, search_min_radius));
        bool findSatus = Find_Next_Circle(orderedObjects, orderedObjectNum, objects, objectNum, search_max_radius, search_min_radius);
        if(findSatus)
        {
            LDialog_MsgBox("Order Success");
        }
        else
        {
            LDialog_MsgBox("Order Fail");
            return;
        }
        //calcu
        LPoint *centerPoints;
        centerPoints = (LPoint *)malloc((objectNum) * sizeof(LPoint));
        CheckIsCircles(objects, objectNum, centerPoints);
        LPoint center ;
        center = CalculateCentroid(centerPoints, objectNum);

        Draw_Boundry(orderedObjects, objectNum, innerDistance, outterDistance, standerRadius, (long)minDistance, center);
        
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

    int Calculate_Outside_Circle(LPoint A, LPoint B, long L, LPoint* results)
    {
        // 检查L是否为正
        if (L < 0) {
            return 0; // 距离不能为负
        }

        double dx = B.x - A.x;
        double dy = B.y - A.y;

        // 计算AB两点之间的距离的平方
        double d_AB_sq = dx * dx + dy * dy;

        // 如果两点重合，没有明确的垂直平分线
        if (d_AB_sq < 1e-9) { // 使用一个小的 epsilon 值来比较浮点数是否接近0
            // 如果L为0，则P就是A点，但通常这种情况不满足"垂直平分线"的定义
            // 如果L不为0，则无解
            return 0;
        }

        double d_AB = sqrt(d_AB_sq);

        // 检查L是否足够大，确保有解
        // L必须大于或等于AB长度的一半
        if (L * L < d_AB_sq / 4.0 - 1e-9) { // L^2 < (AB/2)^2 => L < AB/2
            return 0; // 无解
        }

        // 计算AB的中点M
        LPoint M;
        M.x = (A.x + B.x) / 2.0;
        M.y = (A.y + B.y) / 2.0;

        // 计算PM的长度（M到P的距离）
        double d_M_sq = L * L - d_AB_sq / 4.0;
        double d_M = sqrt(d_M_sq);

        // 单位方向向量 (垂直于AB)
        // 向量AB: (dx, dy)
        // 垂直向量: (-dy, dx) 或 (dy, -dx)
        double unit_perp_x1 = -dy / d_AB;
        double unit_perp_y1 = dx / d_AB;

        double unit_perp_x2 = dy / d_AB;
        double unit_perp_y2 = -dx / d_AB;

        // 计算第一个解P1
        results[0].x = (long)(M.x + d_M * unit_perp_x1);
        results[0].y = (long)(M.y + d_M * unit_perp_y1);

        // 计算第二个解P2
        results[1].x = (long)(M.x + d_M * unit_perp_x2);
        results[1].y = (long)(M.y + d_M * unit_perp_y2);

        // 理论上，如果L == d_AB / 2，那么d_M将为0，两个点会重合。
        // 但是在浮点数计算中，直接比较可能不准确，所以我们总是返回2个点，
        // 调用者可以检查这两个点是否重合。
        if (d_M < 1e-9) { // 如果PM长度接近0，则只有一个解（两个点重合）
            return 1;
        }
        return 2;
    }

    bool CheckIsCircles(LObject* objects, int objectNum, LPoint * centerPoints)
    {
        int i;
        for( i = 0 ; i< objectNum ; i++)
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

    double calculateAngleP1ToP2(LPoint P1, LPoint P2) 
    {
        // 构建从 P1 到 P2 的向量
        double vec_x = P2.x - P1.x;
        double vec_y = P2.y - P1.y;

        // 计算向量与正x轴的夹角 (弧度, 范围 -PI 到 PI)
        // atan2(y, x)
        double angle_rad = atan2(vec_y, vec_x);

        // 将弧度转换为度数
        double angle_degrees = angle_rad * 180.0 / M_PI;

        return angle_degrees;
    }
    
    double normalize_angle(double angle) {
        while (angle <= -180.0) angle += 360.0;
        while (angle > 180.0) angle -= 360.0;
        return angle;
    }

    void Draw_Boundry(LObject* orderedObjects, int orderedObjectNum, long innerDistance, long outterDistance, long standerRadius, long minDistance, LPoint shapeCenter)
    {
        LCell Cell_Now = LCell_GetVisible();
        LLayer LLayer_Now = LObject_GetLayer(Cell_Now, orderedObjects[0]);
        double startAngle = -135; // start from left bottom circle
        double angle1, angle2;
        int i,j;
        int currentSize = 0;
        int capacity = 1000;
        LPoint* totalPolygon = (LPoint*)malloc(capacity * sizeof(LPoint));
        for( i = 0 ;i < orderedObjectNum; i++)
        {
            LPoint circle1 = LCircle_GetCenter(orderedObjects[i % orderedObjectNum]);
            long radius1 = LCircle_GetRadius(orderedObjects[i % orderedObjectNum]);
            LPoint circle2 = LCircle_GetCenter(orderedObjects[(i+1) % orderedObjectNum]);
            long radius2 = LCircle_GetRadius(orderedObjects[(i+1) % orderedObjectNum]);
            long L = innerDistance + outterDistance + standerRadius * 2;
            //LDialog_MsgBox(LFormat("x1: %d, y1: %d\nx2: %d, y2: %d",circle1.x,circle1.y,circle2.x,circle2.y));
            LPoint results1[2];
            int resultNum = Calculate_Outside_Circle(circle1, circle2, L, results1);
            //LDialog_MsgBox(LFormat("resultNum1: %d",resultNum));
            LPoint result1;
            if(resultNum == 0) continue;
            if(resultNum == 1) result1 = results1[0];
            if(resultNum == 2)
            {
                result1 = GetPointDistance(shapeCenter, results1[0]) > GetPointDistance(shapeCenter, results1[1]) ? results1[0] :  results1[1];
            } 
            //LDialog_MsgBox(LFormat("x1: %d, y1: %d",result1.x,result1.y));

            LPoint circle3 = LCircle_GetCenter(orderedObjects[(i+2) % orderedObjectNum]);
            long radius3 = LCircle_GetRadius(orderedObjects[(i+2) % orderedObjectNum]);
            LPoint results2[2];
            resultNum = Calculate_Outside_Circle(circle2, circle3, L, results2);
            //LDialog_MsgBox(LFormat("resultNum2: %d",resultNum));
            LPoint result2;
            if(resultNum == 0) continue;
            if(resultNum == 1) result2 = results2[0];
            if(resultNum == 2)
            {
                result2 = GetPointDistance(shapeCenter, results2[0]) > GetPointDistance(shapeCenter, results2[1]) ? results2[0] :  results2[1];
            } 


            LPoint* part1;
            LPoint* part2;
            if(GetPointDistance(result1, result2) < minDistance * 0.9 ) 
            {
                LPoint results3[2];
                LPoint result3;
                resultNum = Calculate_Outside_Circle(circle1, circle3, L, results3);
                if(resultNum == 0) continue;
                if(resultNum == 1) result3 = results3[0];
                if(resultNum == 2)
                {
                    result3 = GetPointDistance(shapeCenter, results3[0]) > GetPointDistance(shapeCenter, results3[1]) ? results3[0] :  results3[1];
                } 
                // LCircle_New(Cell_Now, LLayer_Now, result3, standerRadius);
                
                angle1 = calculateAngleP1ToP2(circle1, result3);    
                angle2 = calculateAngleP1ToP2(circle3, result3);
                //LDialog_MsgBox(LFormat("a1: %f, a2: %f, a3: %f", startAngle, angle1, angle2));
                double _angle1 = normalize_angle(angle1 + 180);
                double _angle2 = normalize_angle(angle2 + 180);
                //start - >angle1
                //_angle1 -> _angle2
                //
                int cnt1 = calculate_arc_points(circle1.x, circle1.y, radius1 + innerDistance, startAngle, angle1, &part1);
                //LPolygon_New(Cell_Now, LLayer_Now, part1, cnt1);
                int cnt2 = calculate_arc_points(result3.x, result3.y, radius1 + outterDistance, _angle1, _angle2, &part2);
                //LPolygon_New(Cell_Now, LLayer_Now, part2, cnt2);
                if(capacity < 1.5 * (cnt1 + cnt2 + currentSize))
                {
                    int newCapacity = 1.5 * (cnt1 + cnt2 + currentSize);
                    LPoint* tempArray = (LPoint*)realloc(totalPolygon, newCapacity * sizeof(LPoint));
                    totalPolygon = tempArray; // 更新指针指向新的内存区域
                    capacity = newCapacity;   // 更新容量
                }

                // 添加新的元素
                for (j = 0; j < cnt1 ; j++) {
                    totalPolygon[currentSize] = part1[j];
                    currentSize++;
                }

                for (j = 0; j < cnt2 ; j++) {
                    totalPolygon[currentSize] = part2[j];
                    currentSize++;
                }

                startAngle = angle2;
                i++;
                continue;
                //special area
            }; 
            angle1 = calculateAngleP1ToP2(circle1, result1);
            angle2 = calculateAngleP1ToP2(circle2, result1);
            //LDialog_MsgBox(LFormat("a1: %f, a2: %f, a3: %f", startAngle, angle1, angle2));
            double _angle1 = normalize_angle(angle1 + 180);
            double _angle2 = normalize_angle(angle2 + 180);
            
            int cnt3 = calculate_arc_points(circle1.x, circle1.y, radius1 + innerDistance, startAngle, angle1, &part1);
            int cnt4 = calculate_arc_points(result1.x, result1.y, radius1 + outterDistance, _angle1, _angle2, &part2);
            if(capacity < 1.5 * (cnt3 + cnt4 + currentSize))
            {
                int newCapacity = 1.5 * (cnt3 + cnt4 + currentSize);
                LPoint* tempArray = (LPoint*)realloc(totalPolygon, newCapacity * sizeof(LPoint));
                totalPolygon = tempArray; // 更新指针指向新的内存区域
                capacity = newCapacity;   // 更新容量
            }

            // 添加新的元素
            for (j = 0; j < cnt3 ; j++) {
                totalPolygon[currentSize] = part1[j];
                currentSize++;
            }

            for (j = 0; j < cnt4 ; j++) {
                totalPolygon[currentSize] = part2[j];
                currentSize++;
            }
            startAngle = angle2;
            //break;//debug
        }
        LPoint* partSmall;
        LPoint circle0 = LCircle_GetCenter(orderedObjects[0]);
        long radius0 = LCircle_GetRadius(orderedObjects[0]);
        int small = calculate_arc_points(circle0.x, circle0.y, radius0 + innerDistance, startAngle, -135.0, &partSmall);
        
        if(capacity < small + currentSize)
        {
            int newCapacity = 1.5 * (small + currentSize);
            LPoint* tempArray = (LPoint*)realloc(totalPolygon, newCapacity * sizeof(LPoint));
            totalPolygon = tempArray; // 更新指针指向新的内存区域
            capacity = newCapacity;   // 更新容量
        }

        // 添加新的元素
        for (j = 0; j < small ; j++) {
            totalPolygon[currentSize] = partSmall[j];
            currentSize++;
        }


        LPolygon_New(Cell_Now, LLayer_Now, totalPolygon, currentSize-1);
    }

    double get_short_arc_interpolated_angle(double start_angle, double end_angle) {
        double diff = end_angle - start_angle;
    
        // 如果差值大于180度，调整方向以走短弧
        diff = normalize_angle(diff);
        return diff;
    }

    int calculate_arc_points(long center_x, long center_y, long radius,
                                double start_angle_deg, double end_angle_deg,
                                LPoint **result) {
        //LPoint result;
        double diff= get_short_arc_interpolated_angle(start_angle_deg, end_angle_deg);
        double curve = diff * (M_PI / 180.0) * radius;
        int num_segments = (int)abs(curve / 100);   //10 nm 精度
        *result = (LPoint *)malloc(num_segments * sizeof(LPoint));
        int i;
        for (i = 0; i < num_segments; ++i) {
            double t = (double)i / num_segments;
            
            double current_angle_deg = start_angle_deg + diff * t;
            // 将角度从度转换为弧度
            double current_angle_rad = current_angle_deg * (M_PI / 180.0);

            // 计算点的坐标
            (*result)[i].x = (long)(center_x + radius * cos(current_angle_rad));
            (*result)[i].y = (long)(center_y + radius * sin(current_angle_rad));
        }
        //LDialog_MsgBox(LFormat("num_segments: %d", num_segments));
        //LDialog_MsgBox(LFormat("a1: %d, a2: %d", (*result)[0].x, (*result)[0].y));
        return num_segments;
    }

    void Order_Boundry_Circles()
    {   
        LSelection selectedInital = LSelection_GetList();
        if(LSelection_GetNumber(selectedInital)==0)return;
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

        char str[21];
        sprintf(str, "%.3f", LCircle_GetRadius(objects[0])/1000.0);
        //LDialog_MsgBox(str);
        /****************************Input Params****************************/
		LDialogItem Dialog_Items[3] = {	{"innerDistance (um)", "3.9"},
                                        {"outterDistance (um)", "2.3"},
                                        {"standerRadius (um)", *str}};
		long innerDistance;
        long outterDistance;
        long standerRadius;
		if (LDialog_MultiLineInputBox("Draw Boundry", Dialog_Items, 3))
		{
			innerDistance = (long)(atof(Dialog_Items[0].value) * 1000); 
            outterDistance = (long)(atof(Dialog_Items[1].value) * 1000); 
            standerRadius = (long)(atof(Dialog_Items[2].value) * 1000); 
		}
		else
		{
			return;
		}
		/****************************Input Params****************************/

        //void Order_Circle(LObject* objects, int objectNum, long innerDistance, long outterDistance, long standerRadius)
        Order_Circle(objects, objectNum, innerDistance, outterDistance, standerRadius);
        LSelection_DeselectAll();
        LDisplay_Refresh();
    }


    void SpecialPolygon_func(void)
    {
        LMacro_Register("DrawTangentRing_func", "DrawTangentRing");
        LMacro_Register("Select_Boundry_Circles_func", "Select_Boundry_Circles");
        LMacro_Register("Order_Boundry_Circles_func", "Order_Boundry_Circles");
    }
} /* end of module Special_Polygon_Module */

SpecialPolygon_func();