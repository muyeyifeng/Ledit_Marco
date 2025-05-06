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
#include "Ledit_Conversion.h"
#include "ldata.h" /* Main UPI header. */

    // 计算质心
    Point computeCentroid(Point points[], int n) {
        double cx = 0.0, cy = 0.0;
        int i;
        for (i = 0; i < n; i++) {
            cx += points[i].x;
            cy += points[i].y;
        }
        cx /= n;
        cy /= n;
        Point centroid = {cx, cy};
        return centroid;
    }

    // 计算惯性矩阵
    void computeInertiaMatrix(Point points[], int n, Point centroid, double inertia[2][2]) {
        inertia[0][0] = 0.0; // Ixx
        inertia[0][1] = 0.0; // Ixy
        inertia[1][0] = 0.0; // Iyx
        inertia[1][1] = 0.0; // Iyy

        int i;
        for (i = 0; i < n; i++) {
            double dx = points[i].x - centroid.x;
            double dy = points[i].y - centroid.y;

            inertia[0][0] += dy * dy;
            inertia[0][1] -= dx * dy;
            inertia[1][0] -= dx * dy;
            inertia[1][1] += dx * dx;
        }
    }

    // 计算离心率
    double computeEccentricity(double inertia[2][2]) {
        // 计算特征值
        double trace = inertia[0][0] + inertia[1][1];
        double det = inertia[0][0] * inertia[1][1] - inertia[0][1] * inertia[1][0];

        // 特征值公式: λ = (trace ± sqrt(trace^2 - 4 * det)) / 2
        double lambda1 = (trace + sqrt(trace * trace - 4 * det)) / 2;
        double lambda2 = (trace - sqrt(trace * trace - 4 * det)) / 2;

        // 计算离心率
        double a = fmax(lambda1, lambda2);
        double b = fmin(lambda1, lambda2);

        if (a == 0) return 0.0; // 如果主轴长度为0，返回0

        return sqrt(1 - (b * b) / (a * a));
    }

    // 最小二乘法拟合圆：计算最接近的圆心和半径
    void fitCircle(Point points[], int n, Point *circleCenter, double *radius) 
    {
        double cx = 0.0, cy = 0.0;
        double sum_d = 0.0;
        
        int i;
        for (i = 0; i < n; i++) {
            cx += points[i].x;
            cy += points[i].y;
        }
        cx /= n;
        cy /= n;

        // 初始圆心
        circleCenter->x = cx;
        circleCenter->y = cy;

        for (i = 0; i < n; i++) {
            double dist = sqrt((points[i].x - cx) * (points[i].x - cx) + (points[i].y - cy) * (points[i].y - cy));
            sum_d += dist;
        }
        *radius = sum_d / n;

        // 优化圆心和半径
        double epsilon = 0.0001; // 精度阈值
        double prev_radius = *radius;
        int MAX_LOOP = 100000;
        while (MAX_LOOP-- > 0) {
            double sum_x = 0.0, sum_y = 0.0;
            double sum_err = 0.0;
            // 计算新的圆心和半径
            for (i = 0; i < n; i++) {
                double dist = sqrt((points[i].x - circleCenter->x) * (points[i].x - circleCenter->x) + 
                                (points[i].y - circleCenter->y) * (points[i].y - circleCenter->y));
                double err = dist - *radius;
                sum_err += err;

                sum_x += points[i].x - (err / dist) * (points[i].x - circleCenter->x);
                sum_y += points[i].y - (err / dist) * (points[i].y - circleCenter->y);
            }

            // 更新圆心和半径
            circleCenter->x = sum_x / n;
            circleCenter->y = sum_y / n;

            *radius = sum_err / n + *radius;

            // 判断收敛条件
            if (fabs(*radius - prev_radius) < epsilon) {
                break;
            }
            prev_radius = *radius;
        }
    }

    void GetObjectCoord(LObject selectedObject, long *box)
	{
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		// Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			// get rect xmin
			LRect rect = LBox_GetRect(selectedObject);
			box[0] = rect.x0;
			box[1] = rect.y0;
			box[2] = rect.x1;
			box[3] = rect.y1;
			break;
		}
		case 1: // LCircle
		{
			LPoint center = LCircle_GetCenter(selectedObject);
			LCoord r = LCircle_GetRadius(selectedObject);
			box[0] = center.x - r;
			box[1] = center.y - r;
			box[2] = center.x + r;
			box[3] = center.y + r;
			break;
		}
		case 3: // LPolygon
		{
			LVertex vertex = LObject_GetVertexList(selectedObject);
			while (vertex != NULL)
			{
				LPoint point = LVertex_GetPoint(vertex);
				if (point.x < box[0])
					box[0] = point.x;
				if (point.y < box[1])
					box[1] = point.y;
				if (point.x > box[2])
					box[2] = point.x;
				if (point.y > box[3])
					box[3] = point.y;
				vertex = LVertex_GetNext(vertex);
			}
			break;
		}
		case 4: // LTorus
		{
			LTorusParams pParams;
			if (LTorus_GetParams(selectedObject, &pParams) != NULL)
			{
				box[0] = pParams.ptCenter.x - pParams.nOuterRadius;
				box[1] = pParams.ptCenter.y - pParams.nOuterRadius;
				box[2] = pParams.ptCenter.x + pParams.nOuterRadius;
				box[3] = pParams.ptCenter.y + pParams.nOuterRadius;
			}
			break;
		}
		case 5: // LPie
		{
			LPieParams pParams;
			if (LPie_GetParams(selectedObject, &pParams) != NULL)
			{
				box[0] = pParams.ptCenter.x - pParams.nRadius;
				box[1] = pParams.ptCenter.y - pParams.nRadius;
				box[2] = pParams.ptCenter.x + pParams.nRadius;
				box[3] = pParams.ptCenter.y + pParams.nRadius;
			}
			break;
		}
		default:
			break;
		}
		//
	}

	void Approximate_Circle()
	{
		LCell Cell_Now = LCell_GetVisible();
        LFile File_Now = LCell_GetFile(Cell_Now);
        LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		//****************************Input Params****************************//
		LSelection selectedInital = LSelection_GetList();

		LObject* selectedObjects;
		selectedObjects = (LObject *)malloc(sizeof(LObject));
		int j = 0;
		while(selectedInital != NULL)
		{
            LObject selectedObject = LSelection_GetObject(selectedInital);
            //LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectedObject);
            switch (LObject_GetShape(selectedObject))
		    {
                case 3: // LPolygon
                {
                    LVertex vertex = LObject_GetVertexList(selectedObject);
                    long cnt = LVertex_GetCount(selectedObject);
                    Point *points;
                    points = (Point *)malloc((cnt) * sizeof(Point));
                    int j = 0;
                    while (vertex != NULL)
                    {
                        LPoint point = LVertex_GetPoint(vertex);
                        points[j].x = point.x;
                        points[j].y = point.y;
                        j++;
                        vertex = LVertex_GetNext(vertex);
                    }
                    // 计算质心
                    Point centroid = computeCentroid(points, j-1);

                    // 计算惯性矩阵
                    double inertia[2][2];
                    computeInertiaMatrix(points, j-1, centroid, inertia);

                    // 计算离心率
                    double eccentricity = computeEccentricity(inertia);
                    LDialog_MsgBox(LFormat("The eccentricity of the polygon is: %lf, Roundness measure of polygons: %.2f", eccentricity, 1 - eccentricity));

                    // 拟合最接近的圆
                    Point circleCenter;
                    double radius;
                    fitCircle(points, j-1, &circleCenter, &radius);
                    LDialog_MsgBox(LFormat("circleCenter is: (%.2f, %.2f), radius: %.2f", circleCenter.x, circleCenter.y, radius));
                    LPoint newPoint = LPoint_Set(circleCenter.x, circleCenter.y);
                    LCircle_New(Cell_Now, LLayer_Now, newPoint, (long)radius);
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
		LCell Cell_Now = LCell_GetVisible();
        LFile File_Now = LCell_GetFile(Cell_Now);
        LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		//****************************Input Params****************************//
		LSelection selectedInital = LSelection_GetList();

		LObject* selectedObjects;
		selectedObjects = (LObject *)malloc(sizeof(LObject));
		int j = 0;
		while(selectedInital != NULL)
		{
            LObject selectedObject = LSelection_GetObject(selectedInital);
            //LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectedObject);
            switch (LObject_GetShape(selectedObject))
		    {
                case 3: // LPolygon
                {
                    long box[4];
                    box[0] = WORLD_MAX;	 // x0
                    box[1] = WORLD_MAX;	 // y0
                    box[2] = -WORLD_MAX; // x1
                    box[3] = -WORLD_MAX; // y1
                    GetObjectCoord(selectedObject, box);
                    //LDialog_MsgBox(LFormat("x0: %d, y0: %d, x1: %d, y1: %d", box[0], box[1], box[2], box[3]));
                    // 拟合最接近的圆
                    Point circleCenter;
                    circleCenter.x = (box[0] + box[2]) / 2; 
                    circleCenter.y = (box[1] + box[3]) / 2;
                    double radius;
                    radius = ((box[2] - box[0])+ (box[3] - box[1])) / 4.0;
                    //LDialog_MsgBox(LFormat("r1: %d, r2: %d", box[2] - box[0], box[3] - box[1]));
                    LPoint newPoint = LPoint_Set(circleCenter.x, circleCenter.y);
                    LCircle_New(Cell_Now, LLayer_Now, newPoint, (long)radius);
                    break;
                }
                default:
				    break;
            }
            selectedInital = LSelection_GetNext(selectedInital);
        }
        LDisplay_Refresh();
    }

    void Conversion_func(void)
	{
		LMacro_Register("Approximate_Circle_func","Approximate_Circle");
        LMacro_Register("Approximate_Circle_Sample_func","Approximate_Circle_Sample");
	}
}/* end of module Conversion_Module */

Conversion_func();