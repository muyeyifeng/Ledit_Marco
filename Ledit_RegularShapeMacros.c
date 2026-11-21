/*
 * Draw regular polygons from an edge length or circumradius and approximate ellipses with vertices.
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
module RegularShapeMacros
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ldata.h" /* Main UPI header. */
#include "Ledit_OutputLayer.h"

	void DrawEllipsePolygon();
	void DrawRegularPolygonBySideLength();
	void DrawRegularPolygonByCircumradius();
	void CreateRegularPolygon(LLayer outputLayer, LPoint center, long radius, int edgeNum, double rad);
	LPoint *SampleEllipseVertices(long a, long b, int n, double rad);
	void RegisterRegularShapeMacros(void);

	void DrawEllipsePolygon()
	{
		LLayer outputLayer = PickOutputLayer(LCell_GetVisible());
		if (outputLayer == NULL)
			return;

		LDialogItem Dialog_Items[4] = {{"Ellipse a (um)", "1"},
									   {"Ellipse b (um)", "1"},
									   {"Rota Deg (0-360)", "0"},
									   {"Polygon number", "100"}};
		long a;
		long b;
		double deg;
		int n;
		if (LDialog_MultiLineInputBox(" Draw Ellipse", Dialog_Items, 4))
		{
			a = (long)(atof(Dialog_Items[0].value) * 1000); // get the a
			b = (long)(atof(Dialog_Items[1].value) * 1000); // get the b
			deg = atof(Dialog_Items[2].value);				// get the deg
			n = atoi(Dialog_Items[3].value);				// get the n
		}
		else
		{
			return;
		}
		double rad = deg * M_PI / 180;
		LCell Cell_Now = LCell_GetVisible();
		LPoint *points = SampleEllipseVertices(a, b, n, rad);
		LPolygon_New(Cell_Now, outputLayer, points, n);
		LDisplay_Refresh();
	}

	/* Convert the requested side length to a circumradius before placing vertices. */
	void DrawRegularPolygonBySideLength()
	{
		LLayer outputLayer = PickOutputLayer(LCell_GetVisible());
		if (outputLayer == NULL)
			return;

		LDialogItem Dialog_Items[3] = {{"Edge Number (>=3)", "3"},
									   {"Side length (um)", "1"},
									   {"Rota Deg (0-360)", "0"}};
		int edgeNum;
		long edgeLen;
		double deg;
		if (LDialog_MultiLineInputBox("Draw regular polygon by side length", Dialog_Items, 3))
		{
			edgeNum = atoi(Dialog_Items[0].value);				  // get the edgeNum
			edgeLen = (long)(atof(Dialog_Items[1].value) * 1000); // get the edgeLen
			deg = atof(Dialog_Items[2].value);					  // get the deg
		}
		else
		{
			return;
		}
		double rad = deg * M_PI / 180;
		if (edgeNum < 3)
		{
			LDialog_AlertBox("A polygon needs at least three sides.");
			return;
		}
		if (edgeLen <= 0)
		{
			LDialog_AlertBox("The side length must be positive.");
			return;
		}
		LPoint center = LPoint_Set(0, 0);
		long radius = edgeLen / (2 * sin(M_PI / edgeNum));
		CreateRegularPolygon(outputLayer, center, radius, edgeNum, rad);

		LDisplay_Refresh();
	}

	void DrawRegularPolygonByCircumradius()
	{
		LLayer outputLayer = PickOutputLayer(LCell_GetVisible());
		if (outputLayer == NULL)
			return;

		LDialogItem Dialog_Items[3] = {{"Edge Number (>=3)", "3"},
									   {"Radius (Outer, um)", "1"},
									   {"Rota Deg (0-360)", "0"}};
		int edgeNum;
		long radius;
		double deg;
		if (LDialog_MultiLineInputBox("Draw Polygon By Radius", Dialog_Items, 3))
		{
			edgeNum = atoi(Dialog_Items[0].value);				 // get the edgeNum
			radius = (long)(atof(Dialog_Items[1].value) * 1000); // get the radius
			deg = atof(Dialog_Items[2].value);					 // get the deg
		}
		else
		{
			return;
		}
		double rad = deg * M_PI / 180;
		if (edgeNum < 3)
		{
			LDialog_AlertBox("A polygon needs at least three sides.");
			return;
		}
		if (radius <= 0)
		{
			LDialog_AlertBox("The circumradius must be positive.");
			return;
		}
		LPoint center = LPoint_Set(0, 0);
		CreateRegularPolygon(outputLayer, center, radius, edgeNum, rad);

		LDisplay_Refresh();
	}

	/* Sample one vertex per equal angular step around the supplied center. */
	void CreateRegularPolygon(LLayer outputLayer, LPoint center, long radius, int edgeNum, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LPoint *points;
		points = (LPoint *)malloc((edgeNum) * sizeof(LPoint));
		int i;
		for (i = 0; i < edgeNum; i++)
		{
			int point_x = center.x + radius * sin(rad + 2 * i * M_PI / edgeNum);
			int point_y = center.y + radius * cos(rad + 2 * i * M_PI / edgeNum);
			points[i] = LPoint_Set((long)point_x, (long)point_y);
		}
		LPolygon_New(Cell_Now, outputLayer, points, edgeNum);
	}

	/* Rotate the sampled ellipse coordinates before converting them to the layout grid. */
	LPoint *SampleEllipseVertices(long a, long b, int n, double rad)
	{
		double angle, x, y;
		LPoint *points;
		points = (LPoint *)malloc((n) * sizeof(LPoint));

		double step = 2 * M_PI / n;
		int i;
		for (i = 0; i < n; i++)
		{
			angle = i * step;
			x = a * cos(angle);
			y = b * sin(angle);

			double l = sqrt(x * x + y * y);
			double xx = x * cos(rad) - y * sin(rad);
			double yy = x * sin(rad) + y * cos(rad);
			points[i] = LPoint_Set((long)xx, (long)yy);
		}
		return points;
	}

	void RegisterRegularShapeMacros(void)
	{
		LMacro_Register("DrawPolygonByEdgeLenght_func", "DrawRegularPolygonBySideLength");
		LMacro_Register("DrawPolygonByRadius_func", "DrawRegularPolygonByCircumradius");
		LMacro_Register("DrawEllipse_func", "DrawEllipsePolygon");
	}
} /* End of RegularShapeMacros. */

RegisterRegularShapeMacros();
