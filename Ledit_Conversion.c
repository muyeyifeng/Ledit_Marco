/*
 * Conversion_Module
 * Conversion_Module Marco for Ledit
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
module Conversion_Module
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// #include "Ledit_Conversion.h"
#include "ldata.h" /* Main UPI header. */
#include "Ledit_Safety.h"
    // 定义结构体表示二维点
    typedef struct
    {
        double x, y;
    } Point;

    Point computeCentroid(Point points[], int n);
    void computeInertiaMatrix(Point points[], int n, Point centroid, double inertia[2][2]);
    double computeEccentricity(double inertia[2][2]);
    void fitCircle(Point points[], int n, Point *circleCenter, double *radius);
    void GetObjectCoord(LObject selectedObject, long *box);
    void Approximate_Circle();
    void Approximate_Circle_Sample();
    void Conversion_func(void);
    void DrawPolygonByCenterRadiusAndEdgeNumber(LPoint center, long radius, int edgeNum, double rad);
    void Conversion_To_Polygon();
    LPoint *Calculate_Rounded_Hexagon_Points(LPoint center, LCoord R, double r);
    void Conversion_To_Rounded_Hexagon();

    Point computeCentroid(Point points[], int n)
    {
        if (n < 1) { Point zero = {0, 0}; return zero; }
        double cx = 0.0, cy = 0.0;
        int i;
        for (i = 0; i < n; i++)
        {
            cx += points[i].x;
            cy += points[i].y;
        }
        cx /= n;
        cy /= n;
        Point centroid = {cx, cy};
        return centroid;
    }

    // 计算惯性矩阵
    void computeInertiaMatrix(Point points[], int n, Point centroid, double inertia[2][2])
    {
        inertia[0][0] = 0.0; // Ixx
        inertia[0][1] = 0.0; // Ixy
        inertia[1][0] = 0.0; // Iyx
        inertia[1][1] = 0.0; // Iyy

        int i;
        for (i = 0; i < n; i++)
        {
            double dx = points[i].x - centroid.x;
            double dy = points[i].y - centroid.y;

            inertia[0][0] += dy * dy;
            inertia[0][1] -= dx * dy;
            inertia[1][0] -= dx * dy;
            inertia[1][1] += dx * dx;
        }
    }

    // 计算离心率
    double computeEccentricity(double inertia[2][2])
    {
        // 计算特征值
        double trace = inertia[0][0] + inertia[1][1];
        double det = inertia[0][0] * inertia[1][1] - inertia[0][1] * inertia[1][0];

        // 特征值公式: λ = (trace ± sqrt(trace^2 - 4 * det)) / 2
        double discriminant = fmax(0.0, trace * trace - 4 * det);
        double lambda1 = (trace + sqrt(discriminant)) / 2;
        double lambda2 = (trace - sqrt(discriminant)) / 2;

        // 计算离心率
        double a = fmax(lambda1, lambda2);
        double b = fmin(lambda1, lambda2);

        if (a == 0)
            return 0.0; // 如果主轴长度为0，返回0

        return sqrt(fmax(0.0, 1 - b / a));
    }

    // 最小二乘法拟合圆：计算最接近的圆心和半径
    void fitCircle(Point points[], int n, Point *circleCenter, double *radius)
    {
        Point mean = computeCentroid(points, n);
        double xx = 0, xy = 0, yy = 0, xq = 0, yq = 0;
        double det, dx = 0, dy = 0, sum = 0;
        int i;
        *circleCenter = mean; *radius = 0;
        if (n < 1) return;
        /* Centered algebraic least squares; O(n), no iterative division by distance. */
        for (i = 0; i < n; ++i) {
            double x = points[i].x - mean.x, y = points[i].y - mean.y;
            double q = x * x + y * y;
            xx += x * x; xy += x * y; yy += y * y;
            xq += x * q; yq += y * q;
        }
        det = xx * yy - xy * xy;
        if (det > 1e-12 * (xx + yy) * (xx + yy)) {
            dx = (xq * yy - yq * xy) / (2 * det);
            dy = (yq * xx - xq * xy) / (2 * det);
        }
        circleCenter->x += dx; circleCenter->y += dy;
        for (i = 0; i < n; ++i) sum += hypot(points[i].x - circleCenter->x, points[i].y - circleCenter->y);
        *radius = sum / n;
    }

    void GetObjectCoord(LObject selectedObject, long *box)
    {
        LRect rect;
        if (!selectedObject) return;
        rect = LObject_GetMbb(selectedObject);
        box[0] = rect.x0; box[1] = rect.y0;
        box[2] = rect.x1; box[3] = rect.y1;
    }

    void Approximate_Circle()
    {
        if (!LM_Context()) return;
        LCell Cell_Now = LCell_GetVisible();
        LFile File_Now = LCell_GetFile(Cell_Now);
        LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
        //****************************Input Params****************************//
        LSelection selectedInital = LSelection_GetList();

        while (selectedInital != NULL)
        {
            LObject selectedObject = LSelection_GetObject(selectedInital);
            // LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectedObject);
            switch (LObject_GetShape(selectedObject))
            {
            case 3: // LPolygon
            {
                LVertex vertex = LObject_GetVertexList(selectedObject);
                long cnt = LVertex_GetCount(selectedObject);
                if (cnt < 3 || cnt > LM_MAX_POINTS || LPolygon_HasCurve(selectedObject)) break;
                Point *points;
                points = (Point *)malloc((cnt) * sizeof(Point));
                if (!points) return;
                int j = 0;
                while (vertex != NULL && j < cnt)
                {
                    LPoint point = LVertex_GetPoint(vertex);
                    points[j].x = point.x;
                    points[j].y = point.y;
                    j++;
                    vertex = LVertex_GetNext(vertex);
                }
                // 计算质心
                Point centroid = computeCentroid(points, j);

                // 计算惯性矩阵
                double inertia[2][2];
                computeInertiaMatrix(points, j, centroid, inertia);

                // 计算离心率
                double eccentricity = computeEccentricity(inertia);
                LDialog_MsgBox(LFormat("The eccentricity of the polygon is: %lf, Roundness measure of polygons: %.2f", eccentricity, 1 - eccentricity));

                // 拟合最接近的圆
                Point circleCenter;
                double radius;
                fitCircle(points, j, &circleCenter, &radius);
                LDialog_MsgBox(LFormat("circleCenter is: (%.2f, %.2f), radius: %.2f", circleCenter.x, circleCenter.y, radius));
                LPoint newPoint;
                if (radius >= 1 && radius <= WORLD_MAX && LM_PointValid(circleCenter.x + radius, circleCenter.y + radius) && LM_PointValid(circleCenter.x - radius, circleCenter.y - radius))
                {
                    newPoint = LPoint_Set((long)circleCenter.x, (long)circleCenter.y);
                    LM_NewCircle(Cell_Now, LLayer_Now, newPoint, (long)radius);
                }
                free(points);
                break;
            }
            default:
                break;
            }
            selectedInital = LSelection_GetNext(selectedInital);
        }
        LDisplay_Refresh();
    }

    void Approximate_Circle_Sample()
    {
        if (!LM_Context()) return;
        LCell Cell_Now = LCell_GetVisible();
        LFile File_Now = LCell_GetFile(Cell_Now);
        LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
        //****************************Input Params****************************//
        LSelection selectedInital = LSelection_GetList();

        while (selectedInital != NULL)
        {
            LObject selectedObject = LSelection_GetObject(selectedInital);
            // LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectedObject);
            switch (LObject_GetShape(selectedObject))
            {
            case 3: // LPolygon
            {
                long box[4];
                box[0] = WORLD_MAX;  // x0
                box[1] = WORLD_MAX;  // y0
                box[2] = -WORLD_MAX; // x1
                box[3] = -WORLD_MAX; // y1
                GetObjectCoord(selectedObject, box);
                // LDialog_MsgBox(LFormat("x0: %d, y0: %d, x1: %d, y1: %d", box[0], box[1], box[2], box[3]));
                //  拟合最接近的圆
                Point circleCenter;
                circleCenter.x = (box[0] + box[2]) / 2;
                circleCenter.y = (box[1] + box[3]) / 2;
                double radius;
                radius = ((box[2] - box[0]) + (box[3] - box[1])) / 4.0;
                // LDialog_MsgBox(LFormat("r1: %d, r2: %d", box[2] - box[0], box[3] - box[1]));
                LPoint newPoint = LPoint_Set(circleCenter.x, circleCenter.y);
                LM_NewCircle(Cell_Now, LLayer_Now, newPoint, (long)radius);
                break;
            }
            default:
                break;
            }
            selectedInital = LSelection_GetNext(selectedInital);
        }
        LDisplay_Refresh();
    }

    void DrawPolygonByCenterRadiusAndEdgeNumber(LPoint center, long radius, int edgeNum, double rad)
    {
        LCell Cell_Now = LCell_GetVisible();
        LFile File_Now = LCell_GetFile(Cell_Now);
        LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
        if (edgeNum < 3 || edgeNum > LM_MAX_POINTS || radius <= 0 ||
            !LM_PointValid((double)center.x + radius, (double)center.y + radius) ||
            !LM_PointValid((double)center.x - radius, (double)center.y - radius)) return;
        LPoint *points;
        points = (LPoint *)malloc(edgeNum * sizeof(LPoint));
        if (!points) return;
        int i;
        for (i = 0; i < edgeNum; i++)
        {
            int point_x = center.x + radius * sin(rad + 2 * i * M_PI / edgeNum);
            int point_y = center.y + radius * cos(rad + 2 * i * M_PI / edgeNum);
            points[i] = LPoint_Set((long)point_x, (long)point_y);
        }
        LM_NewPolygon(Cell_Now, LLayer_Now, points, edgeNum);
        free(points);
    }

    void Conversion_To_Polygon()
    {
        if (!LM_Context()) return;
        //****************************Input Params****************************//
        LDialogItem Dialog_Items[2] = {{"Edge Number (>=3)", "3"},
                                       {"Rota Deg (0-360)", "0"}};
        int edgeNum;
        double deg;
        if (LM_Dialog("Conversion To Polygon", Dialog_Items, 2))
        {
            edgeNum = (int)atof(Dialog_Items[0].value); // get the edgeNum
            deg = atof(Dialog_Items[1].value);     // get the deg
        }
        else
        {
            return;
        }
        //****************************Input Params****************************//
        double rad = deg * M_PI / 180;
        if (edgeNum < 3)
        {
            LDialog_AlertBox("Polygon edge number must lagger than 3");
            return;
        }
        LSelection selectedInital = LSelection_GetList();
        while (selectedInital != NULL)
        {
            LObject selectedObject = LSelection_GetObject(selectedInital);
            if (LObject_GetShape(selectedObject) != 1)
            {
                selectedInital = LSelection_GetNext(selectedInital);
                continue;
            }
            LPoint center = LCircle_GetCenter(selectedObject);
            LCoord radius = LCircle_GetRadius(selectedObject);
            double radius_outer = radius / cos(1.0 * M_PI / edgeNum);
            if (radius_outer > WORLD_MAX) { LDialog_AlertBox("Polygon radius exceeds coordinate limits."); return; }
            DrawPolygonByCenterRadiusAndEdgeNumber(center, radius_outer, edgeNum, rad);
            selectedInital = LSelection_GetNext(selectedInital);
        }
        LDisplay_Refresh();
    }

    void Conversion_To_Rounded_Hexagon()
    {
        if (!LM_Context()) return;
        LCell Cell_Now = LCell_GetVisible();
        LFile File_Now = LCell_GetFile(Cell_Now);
        LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

        LSelection selectedInital = LSelection_GetList();
        if (selectedInital == NULL)
            return;
        LObject selectedObject = LSelection_GetObject(selectedInital);
        if (LObject_GetShape(selectedObject) != 1)
            return;
        LCoord radius = LCircle_GetRadius(selectedObject);
        double show = 1 / 3.7 * radius;

        //****************************Input Params****************************//
        char show_str_buf[21];
        snprintf(show_str_buf, sizeof(show_str_buf), "%.3f", show / LM_UnitsPerMicron());

        LDialogItem Dialog_Items[2];
        strcpy(Dialog_Items[0].prompt, "Rounded Radius (um, default -1)");
        strcpy(Dialog_Items[0].value, "-1");

        strcpy(Dialog_Items[1].prompt, "default value:");
        strcpy(Dialog_Items[1].value, show_str_buf);

        double roundRadius, useless;
        if (LM_Dialog("Conversion To Rounded Hexagon", Dialog_Items, 2))
        {
            roundRadius = atof(Dialog_Items[0].value); // get Radius ratio
            useless = atof(Dialog_Items[1].value);     // null value
        }
        else
        {
            return;
        }
        //****************************Input Params****************************//
        if (roundRadius < 0 && fabs(roundRadius + 1) > 1e-6)
        {
            LDialog_AlertBox("Invaild input value");
            return;
        }

        while (selectedInital != NULL)
        {
            LObject selectedObject = LSelection_GetObject(selectedInital);
            if (LObject_GetShape(selectedObject) != 1)
            {
                selectedInital = LSelection_GetNext(selectedInital);
                continue;
            }
            LPoint center = LCircle_GetCenter(selectedObject);
            LCoord radius = LCircle_GetRadius(selectedObject);
            LPoint *points;

            double r = roundRadius * LM_UnitsPerMicron();
            if (roundRadius <= 0)
                r = 1 / 3.7 * radius; // default value
            double curve = r * M_PI / 3;
            int n_coner = LM_CornerSamples(r);
            int n_sum = 6 * n_coner;
            points = r < radius ? Calculate_Rounded_Hexagon_Points(center, radius, r) : NULL;
            // DrawPolygonByCenterRadiusAndEdgeNumber(center, radius_outer, edgeNum, rad);
            if (r < radius && points)
                LM_NewPolygon(Cell_Now, LLayer_Now, points, n_sum);
            else if (r >= radius)
                LM_NewCircle(Cell_Now, LLayer_Now, center, radius);
            free(points);
            selectedInital = LSelection_GetNext(selectedInital);
        }
        LDisplay_Refresh();
    }

    LPoint *Calculate_Rounded_Hexagon_Points(LPoint center, LCoord R, double r)
    {
        double curve = r * M_PI / 3;
        int n_coner = LM_CornerSamples(r);
        if (!n_coner) return NULL;
        int n_sum = 6 * n_coner;
        double center_distance = ((double)R - r) * 2 / 3 * sqrt(3);
        double extent = fabs(center_distance) + r;
        if (!LM_PointValid(center.x + extent, center.y + extent) || !LM_PointValid(center.x - extent, center.y - extent)) return NULL;

        LPoint *points;
        points = (LPoint *)malloc(n_sum * sizeof(LPoint));

        if (points == NULL)
        {
            LDialog_AlertBox("Memory allocation failed for points array.");
            return NULL;
        }
        double delta_theta = M_PI / 3 / n_coner;
        int i;
        for (i = 0; i < n_sum; i++)
        {
            double theta = (i % n_coner) * delta_theta;
            int n = i / n_coner;
            double new_center_x = center.x + center_distance * cos(M_PI / 6 + n * M_PI / 3);
            double new_center_y = center.y + center_distance * sin(M_PI / 6 + n * M_PI / 3);
            double start_theta = n * M_PI / 3;
            points[i].x = (long)(new_center_x + r * cos(start_theta + theta));
            points[i].y = (long)(new_center_y + r * sin(start_theta + theta));
            // if(n > 0) {LDialog_MsgBox(LFormat("%d,%d",new_center_x,new_center_y));break;}
        }
        return points;
    }

    void Conversion_func(void)
    {
        LMacro_Register("Approximate_Circle_func", "Approximate_Circle");
        LMacro_Register("Approximate_Circle_Sample_func", "Approximate_Circle_Sample");
        LMacro_Register("Conversion_To_Polygon_func", "Conversion_To_Polygon");
        LMacro_Register("Conversion_To_Rounded_Hexagon_func", "Conversion_To_Rounded_Hexagon");
    }
} /* end of module Conversion_Module */

Conversion_func();
