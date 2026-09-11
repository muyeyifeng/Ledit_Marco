/*
 * Polygon_Module
 * Polygon_Module Marco for Ledit
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
module Polygon_Module
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// #include "Ledit_Regular.h"
#include "ldata.h" /* Main UPI header. */
#include "Ledit_Safety.h"

	void DrawEllipse();
	void DrawPolygonByEdgeLenght();
	void DrawPolygonByRadius();
	void DrawPolygonByCenterRadiusAndEdgeNumber(LPoint center, long radius, int edgeNum, double rad);
	LPoint *CalculateEllipsePoints(long a, long b, int n, double rad);
	void Polygon_func(void);

	void DrawEllipse()
	{
        if (!LM_Context()) return;
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[4] = {{"Ellipse a (um)", "1"},
									   {"Ellipse b (um)", "1"},
									   {"Rota Deg (0-360)", "0"},
									   {"Polygon number", "100"}};
		long a;
		long b;
		double deg;
		int n;
		if (LM_Dialog(" Draw Ellipse", Dialog_Items, 4))
		{
			a = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the a
			b = (long)(atof(Dialog_Items[1].value) * LM_UnitsPerMicron()); // get the b
			deg = atof(Dialog_Items[2].value);				// get the deg
			n = (int)atof(Dialog_Items[3].value);				// get the n
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * M_PI / 180;
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		LPoint *points = CalculateEllipsePoints(a, b, n, rad);
        if (!points) return;
		LM_NewPolygon(Cell_Now, LLayer_Now, points, n);
        free(points);
		LDisplay_Refresh();
	}

	void DrawPolygonByEdgeLenght()
	{
        if (!LM_Context()) return;
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{"Edge Number (>=3)", "3"},
									   {"EdgeLenght (um)", "1"},
									   {"Rota Deg (0-360)", "0"}};
		int edgeNum;
		long edgeLen;
		double deg;
		if (LM_Dialog(" Draw Polygon By Edge Lenght", Dialog_Items, 3))
		{
			edgeNum = (int)atof(Dialog_Items[0].value);				  // get the edgeNum
			edgeLen = (long)(atof(Dialog_Items[1].value) * LM_UnitsPerMicron()); // get the edgeLen
			deg = atof(Dialog_Items[2].value);					  // get the deg
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * M_PI / 180;
		if (edgeNum < 3 || edgeNum > LM_MAX_POINTS)
		{
			LDialog_AlertBox("Polygon edge number must lagger than 3");
			return;
		}
		if (edgeLen <= 0)
		{
			LDialog_AlertBox("Polygon edge length must lagger than 0");
			return;
		}
		LPoint center = LPoint_Set(0, 0);
		double radiusValue = edgeLen / (2 * sin(M_PI / edgeNum));
        if (radiusValue > WORLD_MAX) { LDialog_AlertBox("Polygon exceeds coordinate limits."); return; }
        long radius = (long)radiusValue;
		DrawPolygonByCenterRadiusAndEdgeNumber(center, radius, edgeNum, rad);

		LDisplay_Refresh();
	}

	void DrawPolygonByRadius()
	{
        if (!LM_Context()) return;
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{"Edge Number (>=3)", "3"},
									   {"Radius (Outer, um)", "1"},
									   {"Rota Deg (0-360)", "0"}};
		int edgeNum;
		long radius;
		double deg;
		if (LM_Dialog("Draw Polygon By Radius", Dialog_Items, 3))
		{
			edgeNum = (int)atof(Dialog_Items[0].value);				 // get the edgeNum
			radius = (long)(atof(Dialog_Items[1].value) * LM_UnitsPerMicron()); // get the radius
			deg = atof(Dialog_Items[2].value);					 // get the deg
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * M_PI / 180;
		if (edgeNum < 3 || edgeNum > LM_MAX_POINTS)
		{
			LDialog_AlertBox("Polygon edge number must lagger than 3");
			return;
		}
		if (radius <= 0)
		{
			LDialog_AlertBox("Polygon radius must lagger than 0");
			return;
		}
		LPoint center = LPoint_Set(0, 0);
		DrawPolygonByCenterRadiusAndEdgeNumber(center, radius, edgeNum, rad);

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

	LPoint *CalculateEllipsePoints(long a, long b, int n, double rad)
	{
		if (a <= 0 || b <= 0 || n < 3 || n > LM_MAX_POINTS) {
            LDialog_AlertBox("Use positive axes and 3..100000 vertices."); return NULL;
        }
        double angle, x, y;
        double cr = cos(rad), sr = sin(rad);
		LPoint *points;
		points = (LPoint *)malloc(n * sizeof(LPoint));
        if (!points) return NULL;

		double step = 2 * M_PI / n;
		int i;
		for (i = 0; i < n; i++)
		{
			angle = i * step;
			x = a * cos(angle);
			y = b * sin(angle);


			double xx = x * cr - y * sr;
			double yy = x * sr + y * cr;
			points[i] = LPoint_Set((long)xx, (long)yy);
		}
		return points;
	}

	void Polygon_func(void)
	{
		LMacro_Register("DrawPolygonByEdgeLenght_func", "DrawPolygonByEdgeLenght");
		LMacro_Register("DrawPolygonByRadius_func", "DrawPolygonByRadius");
		LMacro_Register("DrawEllipse_func", "DrawEllipse");
	}
} /* end of module Array_Module */

Polygon_func();
