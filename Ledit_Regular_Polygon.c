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
	void DrawPolygonByCenterRadiusAndEdgeNumber(LPoint center, long radius, int edgeNum, double rad);
	void DrawPolygonByEdgeLenght();
	void DrawPolygonByRadius();
	void Polygon_func(void);

	void DrawPolygonByEdgeLenght()
	{
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{ "Edge Number(>=3)", "3" },
		{ "EdgeLenght (nm)", "1000" },
		{ "Rota Deg (0-360)","0" }};
		int edgeNum;
		long edgeLen;
		int deg;
		if(LDialog_MultiLineInputBox(" Draw Polygon By Edge Lenght", Dialog_Items, 3))
		{
			edgeNum = atoi(Dialog_Items[0].value); // get the edgeNum
			edgeLen = atol(Dialog_Items[1].value); // get the edgeLen
			deg = atoi(Dialog_Items[2].value); // get the deg
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
		DrawPolygonByCenterRadiusAndEdgeNumber(center, radius, edgeNum, deg);

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
		int deg;
		if(LDialog_MultiLineInputBox("Draw Polygon By Radius", Dialog_Items, 3))
		{
			edgeNum = atoi(Dialog_Items[0].value); // get the edgeNum
			radius = atol(Dialog_Items[1].value); // get the radius
			deg = atoi(Dialog_Items[2].value); // get the deg
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
		DrawPolygonByCenterRadiusAndEdgeNumber(center, radius, edgeNum, deg);

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
	
	void Polygon_func(void)
	{
		LMacro_Register("DrawPolygonByEdgeLenght_func", "DrawPolygonByEdgeLenght");
		LMacro_Register("DrawPolygonByRadius_func", "DrawPolygonByRadius");
		//LMacro_BindToHotKey(KEY_F11, "ArrayByOffsetOneDimension_func", "ArrayByOffsetOneDimension");
		//LMacro_BindToHotKey(KEY_F8, "ArrayByEdgeDistanceOneDimension_func", "ArrayByEdgeDistanceOneDimension");
		//LMacro_BindToHotKey(KEY_F9, "ArrayInObjectByDistanceHexagon_func", "ArrayInObjectByDistanceHexagon");
		//LMacro_BindToHotKey(KEY_F10, "ArrayInObjectByRing_func","ArrayInObjectByRing");
	}
} /* end of module Array_Module */

Polygon_func();