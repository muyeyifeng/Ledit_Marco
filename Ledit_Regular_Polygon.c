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
	#include "ldata.h"     /* Main UPI header. */

	#define PI 3.14159265358979323846
	void DrawEllipse();
	LPoint* CalculateEllipsePoints(long a, long b, int n, double rad);
	void DrawPolygonByCenterRadiusAndEdgeNumber(LPoint center, long radius, int edgeNum, double rad);
	void DrawPolygonByEdgeLenght();
	void DrawPolygonByRadius();
	void Polygon_func(void);

	void DrawEllipse()
	{
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[4] = {{ "Ellipse a", "1000" },
		{ "Ellipse b", "1000" },
		{ "Rota Deg (0-360)","0" },
		{ "Polygon number", "100" }};
		long a;
		long b;
		double deg;
		int n;
		if(LDialog_MultiLineInputBox(" Draw Ellipse", Dialog_Items, 4))
		{
			a = atol(Dialog_Items[0].value); // get the a
			b = atol(Dialog_Items[1].value); // get the b
			deg = atof(Dialog_Items[2].value); // get the deg
			n = atoi(Dialog_Items[3].value); // get the n
		}
		else{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * PI / 180;
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		LPoint	*points = CalculateEllipsePoints(a, b, n, rad);
		LPolygon_New( Cell_Now, LLayer_Now, points, n );
	}

	void DrawPolygonByEdgeLenght()
	{
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{ "Edge Number(>=3)", "3" },
		{ "EdgeLenght (nm)", "1000" },
		{ "Rota Deg (0-360)","0" }};
		int edgeNum;
		long edgeLen;
		double deg;
		if(LDialog_MultiLineInputBox(" Draw Polygon By Edge Lenght", Dialog_Items, 3))
		{
			edgeNum = atoi(Dialog_Items[0].value); // get the edgeNum
			edgeLen = atol(Dialog_Items[1].value); // get the edgeLen
			deg = atof(Dialog_Items[2].value); // get the deg
		}
		else{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * PI / 180;
		if(edgeNum < 3){
			LDialog_AlertBox("Polygon edge number must lagger than 3");
			return;
		}
		if(edgeLen <= 0){
			LDialog_AlertBox("Polygon edge length must lagger than 0");
			return;
		}
		LPoint center = LPoint_Set(0, 0);
		long radius = edgeLen / (2 * sin(PI / edgeNum));
		DrawPolygonByCenterRadiusAndEdgeNumber(center, radius, edgeNum, rad);

		LDisplay_Refresh();
	}

	void DrawPolygonByRadius()
	{
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{ "Edge Number(>=3)", "3" },
		{ "Radius (Outer)", "1000" },
		{ "Rota Deg (0-360)","0" }};
		int edgeNum;
		long radius;
		double deg;
		if(LDialog_MultiLineInputBox("Draw Polygon By Radius", Dialog_Items, 3))
		{
			edgeNum = atoi(Dialog_Items[0].value); // get the edgeNum
			radius = atol(Dialog_Items[1].value); // get the radius
			deg = atof(Dialog_Items[2].value); // get the deg
		}
		else{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * PI / 180;
		if(edgeNum < 3){
			LDialog_AlertBox("Polygon edge number must lagger than 3");
			return;
		}
		if(radius <= 0){
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
		LPoint	*points;
		points = (LPoint *)malloc((edgeNum) * sizeof(LPoint));
		int i;
		for(i = 0; i < edgeNum; i++)
		{
				int point_x = center.x + radius * sin(rad + 2 * i * PI / edgeNum);
				int point_y = center.y + radius * cos(rad + 2 * i * PI / edgeNum);
				points[i] = LPoint_Set((long)point_x, (long)point_y);
		}
		LPolygon_New( Cell_Now, LLayer_Now, points, edgeNum );
	}

	LPoint* CalculateEllipsePoints(long a, long b, int n, double rad)
	{
		double angle, x, y;
		LPoint	*points;
		points = (LPoint *)malloc((n) * sizeof(LPoint));

		double step = 2 * PI / n;
		int i;
		for ( i = 0; i < n; i++)
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
	
	void Polygon_func(void)
	{
		LMacro_Register("DrawPolygonByEdgeLenght_func", "DrawPolygonByEdgeLenght");
		LMacro_Register("DrawPolygonByRadius_func", "DrawPolygonByRadius");
		LMacro_Register("DrawEllipse_func", "DrawEllipse");
	}
} /* end of module Array_Module */

Polygon_func();