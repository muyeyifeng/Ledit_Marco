/*
 * Array_Module
 * Array_Module Marco for Ledit
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

module Array_Module
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Ledit_Array.h"
#include "ldata.h" /* Main UPI header. */

	void MirrorObjectsByPointAndRad()
	{
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[4] = {{"Mirror Center X Coordinate (um)", "0"},
									   {"Mirror Center Y Coordinate (um)", "0"},
									   {"Mirror Degree (0-180)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long xcoord;
		long ycoord;
		double rotate;
		int keep;
		if (LDialog_MultiLineInputBox("Mirror Objects By Point And Rad", Dialog_Items, 4))
		{
			xcoord = (long)(atof(Dialog_Items[0].value) * 1000); // get the xcoord
			ycoord = (long)(atof(Dialog_Items[1].value) * 1000); // get the ycoord
			rotate = atof(Dialog_Items[2].value);				 // get the rotate
			keep = atoi(Dialog_Items[3].value);					 // get the keep
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		double rad = rotate * M_PI / 180;
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);

			LPoint mirrorPoint = LPoint_Set(xcoord, ycoord);
			MirrorObject(selectedObject, mirrorPoint, rad, keep);
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		LDisplay_Refresh();
	}

	LObject Mirror_Rect(LObject selectRect, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectRect);
		LRect rect = LBox_GetRect(selectRect);
		LPoint p1 = LPoint_Set(rect.x0, rect.y0);
		LPoint p2 = LPoint_Set(rect.x0, rect.y1);
		LPoint p3 = LPoint_Set(rect.x1, rect.y1);
		LPoint p4 = LPoint_Set(rect.x1, rect.y0);
		LPoint points[4];
		points[0] = MirrorCoordinates(p1, mirrorPoint, rad);
		points[1] = MirrorCoordinates(p2, mirrorPoint, rad);
		points[2] = MirrorCoordinates(p3, mirrorPoint, rad);
		points[3] = MirrorCoordinates(p4, mirrorPoint, rad);
		return LPolygon_New(Cell_Now, LLayer_Now, points, 4);
	}

	LObject Mirror_Circle(LObject selectCircle, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectCircle);
		LPoint center = LCircle_GetCenter(selectCircle);
		LCoord r = LCircle_GetRadius(selectCircle);

		LPoint newPoint = MirrorCoordinates(center, mirrorPoint, rad);

		return LCircle_New(Cell_Now, LLayer_Now, newPoint, r);
	}

	LObject Mirror_Polygon(LObject selectPolygon, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectPolygon);
		LVertex vertex = LObject_GetVertexList(selectPolygon);
		long cnt = LVertex_GetCount(selectPolygon);
		LPoint *points;
		points = (LPoint *)malloc((cnt) * sizeof(LPoint));
		int j = 0;
		while (vertex != NULL)
		{
			LPoint point = LVertex_GetPoint(vertex);
			LPoint newPoint = MirrorCoordinates(point, mirrorPoint, rad);
			points[j] = newPoint;
			j++;
			vertex = LVertex_GetNext(vertex);
		}
		return LPolygon_New(Cell_Now, LLayer_Now, points, cnt);
	}

	LObject Mirror_Torus(LObject selectTorus, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectTorus);
		LTorusParams tParams;
		LStatus lStatus = LTorus_GetParams(selectTorus, &tParams);
		if (lStatus == 0)
		{
			/* Define parameters for a torus */
			LTorusParams _tParams;

			LPoint newPoint = MirrorCoordinates(tParams.ptCenter, mirrorPoint, rad);

			_tParams.ptCenter = newPoint;
			_tParams.nInnerRadius = tParams.nInnerRadius;
			_tParams.nOuterRadius = tParams.nOuterRadius;

			double angle = tParams.dStopAngle - tParams.dStartAngle;
			angle = angle < 0 ? 2 * 180 + angle : angle;

			double startAngle = tParams.dStartAngle > 180 ? tParams.dStartAngle - 2 * 180 : tParams.dStartAngle;

			_tParams.dStartAngle = 2 * rad * 180 / M_PI - startAngle - angle;
			_tParams.dStopAngle = 2 * rad * 180 / M_PI - startAngle;
			return LTorus_CreateNew(Cell_Now, LLayer_Now, &_tParams);
		}
		return NULL;
	}

	LObject Mirror_Pie(LObject selectPie, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectPie);
		LPieParams pParams;
		LStatus lStatus = LPie_GetParams(selectPie, &pParams);
		if (lStatus == 0)
		{
			LPieParams _pParams;
			LPoint newPoint = MirrorCoordinates(pParams.ptCenter, mirrorPoint, rad);

			_pParams.ptCenter = newPoint;
			_pParams.nRadius = pParams.nRadius;

			double angle = pParams.dStopAngle - pParams.dStartAngle;
			angle = angle < 0 ? 2 * 180 + angle : angle;

			double startAngle = pParams.dStartAngle > 180 ? pParams.dStartAngle - 2 * 180 : pParams.dStartAngle;

			_pParams.dStartAngle = 2 * rad * 180 / M_PI - startAngle - angle;
			_pParams.dStopAngle = 2 * rad * 180 / M_PI - startAngle;
			return LPie_CreateNew(Cell_Now, LLayer_Now, &_pParams);
		}
		return NULL;
	}

	LObject MirrorObject(LObject selectedObject, LPoint mirrorPoint, double rad, int keep)
	{
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);

		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			return Mirror_Rect(selectedObject, mirrorPoint, rad);
		}
		case 1: // LCircle
		{
			return Mirror_Circle(selectedObject, mirrorPoint, rad);
		}
		case 3: // LPolygon
		{
			return Mirror_Polygon(selectedObject, mirrorPoint, rad);
		}
		case 4: // LTorus
		{
			return Mirror_Torus(selectedObject, mirrorPoint, rad);
		}
		case 5: // LPie
		{
			return Mirror_Pie(selectedObject, mirrorPoint, rad);
		}
		default:
			return NULL;
		}
		LCell Cell_Now = LCell_GetVisible();
		if (keep != 1)
			LObject_Delete(Cell_Now, selectedObject);
	}

	void RotateObjectsByPoint()
	{
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[4] = {{"Rotate Center X Coordinate (um)", "0"},
									   {"Rotate Center Y Coordinate (um)", "0"},
									   {"Rotate degree (0-360)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long xcoord;
		long ycoord;
		double rotate;
		int keep;
		if (LDialog_MultiLineInputBox("Rotate Objects By Point", Dialog_Items, 4))
		{
			xcoord = (long)(atof(Dialog_Items[0].value) * 1000); // get the xcoord
			ycoord = (long)(atof(Dialog_Items[1].value) * 1000); // get the ycoord
			rotate = atof(Dialog_Items[2].value);				 // get the rotate
			keep = atoi(Dialog_Items[3].value);					 // get the rotate
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		double rad = rotate * M_PI / 180;

		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);

			LPoint rotateCenter = LPoint_Set(xcoord, ycoord);
			RotateObject(selectedObject, rotateCenter, rad, keep);
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		LDisplay_Refresh();
	}

	LObject Rotate_Rect(LObject selectRect, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectRect);
		LRect rect = LBox_GetRect(selectRect);

		LPoint p1 = LPoint_Set(rect.x0, rect.y0);
		LPoint p2 = LPoint_Set(rect.x0, rect.y1);
		LPoint p3 = LPoint_Set(rect.x1, rect.y1);
		LPoint p4 = LPoint_Set(rect.x1, rect.y0);
		LPoint points[4];
		points[0] = RotatePoint(p1, rotateCenter, rad);
		points[1] = RotatePoint(p2, rotateCenter, rad);
		points[2] = RotatePoint(p3, rotateCenter, rad);
		points[3] = RotatePoint(p4, rotateCenter, rad);
		return LPolygon_New(Cell_Now, LLayer_Now, points, 4);
	}

	LObject Rotate_Circle(LObject selectCircle, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectCircle);
		LPoint center = LCircle_GetCenter(selectCircle);
		LCoord r = LCircle_GetRadius(selectCircle);

		LPoint newPoint = RotatePoint(center, rotateCenter, rad);

		return LCircle_New(Cell_Now, LLayer_Now, newPoint, r);
	}

	LObject Rotate_Polygon(LObject selectPolygon, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectPolygon);
		LVertex vertex = LObject_GetVertexList(selectPolygon);
		long cnt = LVertex_GetCount(selectPolygon);
		LPoint *points;
		points = (LPoint *)malloc((cnt) * sizeof(LPoint));
		int j = 0;
		while (vertex != NULL)
		{
			LPoint point = LVertex_GetPoint(vertex);
			LPoint newPoint = RotatePoint(point, rotateCenter, rad);

			points[j] = newPoint;
			j++;
			vertex = LVertex_GetNext(vertex);
		}
		return LPolygon_New(Cell_Now, LLayer_Now, points, cnt);
	}

	LObject Rotate_Torus(LObject selectTorus, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectTorus);
		LTorusParams tParams;
		LStatus lStatus = LTorus_GetParams(selectTorus, &tParams);
		if (lStatus == 0)
		{
			/* Define parameters for a torus */
			LTorusParams _tParams;

			LPoint newPoint = RotatePoint(tParams.ptCenter, rotateCenter, rad);

			_tParams.ptCenter = newPoint;
			_tParams.nInnerRadius = tParams.nInnerRadius;
			_tParams.nOuterRadius = tParams.nOuterRadius;

			_tParams.dStartAngle = tParams.dStartAngle + rad * 180 / M_PI;
			_tParams.dStopAngle = tParams.dStopAngle + rad * 180 / M_PI;
			return LTorus_CreateNew(Cell_Now, LLayer_Now, &_tParams);
		}
		return NULL;
	}

	LObject Rotate_Pie(LObject selectPie, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectPie);
		LPieParams pParams;
		LStatus lStatus = LPie_GetParams(selectPie, &pParams);
		if (lStatus == 0)
		{
			LPieParams _pParams;
			LPoint newPoint = RotatePoint(pParams.ptCenter, rotateCenter, rad);

			_pParams.ptCenter = newPoint;
			_pParams.nRadius = pParams.nRadius;

			_pParams.dStartAngle = pParams.dStartAngle + rad * 180 / M_PI;
			_pParams.dStopAngle = pParams.dStopAngle + rad * 180 / M_PI;
			return LPie_CreateNew(Cell_Now, LLayer_Now, &_pParams);
		}
		return NULL;
	}

	LObject RotateObject(LObject selectedObject, LPoint rotateCenter, double rad, int keep)
	{

		LShapeType selectedShapeType = LObject_GetShape(selectedObject);

		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			return Rotate_Rect(selectedObject, rotateCenter, rad);
		}
		case 1: // LCircle
		{
			return Rotate_Circle(selectedObject, rotateCenter, rad);
		}
		case 3: // LPolygon
		{
			return Rotate_Polygon(selectedObject, rotateCenter, rad);
		}
		case 4: // LTorus
		{
			return Rotate_Torus(selectedObject, rotateCenter, rad);
		}
		case 5: // LPie
		{
			return Rotate_Pie(selectedObject, rotateCenter, rad);
		}
		default:
			return NULL;
		}

		LCell Cell_Now = LCell_GetVisible();
		if (keep != 1)
			LObject_Delete(Cell_Now, selectedObject);
	}

	void ArrayByRingSetRad()
	{
		LSelection selectedInital = LSelection_GetList();
		int selectedObjectNumber = LSelection_GetNumber(selectedInital);
		if (selectedObjectNumber != 1)
		{
			LDialog_AlertBox("Just Support 1 Object");
			return;
		}
		LObject object1 = LSelection_GetObject(selectedInital);
		if (LObject_GetShape(object1) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{"Array Degree Offset (0-360)", "0"},
									   {"Array Start Dimension deg (0-360)", "0"},
									   {"Ring Radius(um)", "1"}};
		double degOffset;
		double deg;
		long radius;
		if (LDialog_MultiLineInputBox("Array By Ring Set Rad", Dialog_Items, 3))
		{
			degOffset = atof(Dialog_Items[0].value);			 // get the distance
			deg = atof(Dialog_Items[1].value);					 // get the deg
			radius = (long)(atof(Dialog_Items[2].value) * 1000); // get the radius
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * M_PI / 180;
		LCell Cell_Now = LCell_GetVisible();

		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, object1);

		LPoint center = LCircle_GetCenter(object1);
		LCoord r = LCircle_GetRadius(object1);
		// calculate how many circle in one ring
		double rad_offset = degOffset * M_PI / 180;
		int maxCircle = ceil(2 * M_PI / rad_offset);
		int j;
		for (j = 0; j < maxCircle; j++)
		{
			int yOffset = radius * sin(rad + rad_offset * j);
			int xOffset = radius * cos(rad + rad_offset * j);
			CopyObject(object1, xOffset, yOffset);
		}
		LDisplay_Refresh();
	}

	void ArrayByRingSetDistance()
	{
		LSelection selectedInital = LSelection_GetList();
		int selectedObjectNumber = LSelection_GetNumber(selectedInital);
		if (selectedObjectNumber != 1)
		{
			LDialog_AlertBox("Just Support 1 Object");
			return;
		}
		LObject object1 = LSelection_GetObject(selectedInital);
		if (LObject_GetShape(object1) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		//****************************Input Params****************************//
		LDialogItem Dialog_Items[4] = {{"Array Edge Distance(um)", "1"},
									   {"Array Start Dimension deg (0-360)", "0"},
									   {"Ring Radius(um)", "1"},
									   {"Adaptive Distance (1 = yes, 0 = no)", "1"}};
		long distance;
		double deg;
		long radius;
		int adapDis;
		if (LDialog_MultiLineInputBox("Array By Ring Set Distance", Dialog_Items, 4))
		{
			distance = (long)(atof(Dialog_Items[0].value) * 1000); // get the distance
			deg = atof(Dialog_Items[1].value);					   // get the deg
			radius = (long)(atof(Dialog_Items[2].value) * 1000);   // get the radius
			adapDis = atoi(Dialog_Items[3].value);				   // Adaptive Distance
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * M_PI / 180;
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, object1);

		LPoint center = LCircle_GetCenter(object1);
		LCoord r = LCircle_GetRadius(object1);
		if (2 * r + distance <= 0)
		{
			LDialog_AlertBox("Value of 'distance' error");
			return;
		}
		// calculate how many circle in one ring
		double radius_ring = radius;
		int maxCircle = 2 * M_PI * radius_ring / (2 * r + distance);
		double rad_offset = 2 * M_PI / maxCircle;
		if (adapDis != 1)
		{
			rad_offset = asin(2 * distance / M_PI);
			maxCircle = ceil(2 * M_PI / rad_offset);
		}
		int j;
		for (j = 0; j < maxCircle; j++)
		{
			int yOffset = radius * sin(rad + rad_offset * j);
			int xOffset = radius * cos(rad + rad_offset * j);
			CopyObject(object1, xOffset, yOffset);
		}
		LDisplay_Refresh();
	}

	void ArrayInObjectByRing()
	{
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

		double area1 = LObject_Area(object1);
		double area2 = LObject_Area(object2);

		LObject smallerObject = area1 > area2 ? object2 : object1;
		LObject biggerObject = area1 > area2 ? object1 : object2;

		if (LObject_GetShape(smallerObject) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		//****************************Input Params****************************//
		LDialogItem Dialog_Items[2] = {{"Array Edge Distance(um)", "1"},
									   {"Array Start Dimension deg (0-360)", "0"}};
		long distance;
		double deg;
		if (LDialog_MultiLineInputBox("Array In Object By Ring", Dialog_Items, 2))
		{
			distance = (long)(atof(Dialog_Items[0].value) * 1000); // get the distance
			deg = atof(Dialog_Items[1].value);					   // get the deg
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//

		double rad = deg * M_PI / 180;
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);

		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, smallerObject);

		long box[4];
		box[0] = WORLD_MAX;	 // x0
		box[1] = WORLD_MAX;	 // y0
		box[2] = -WORLD_MAX; // x1
		box[3] = -WORLD_MAX; // y1
		GetObjectCoord(biggerObject, box);
		LPoint center = LCircle_GetCenter(smallerObject);
		LCoord r = LCircle_GetRadius(smallerObject);
		if (2 * r + distance <= 0)
		{
			LDialog_AlertBox("Value of 'distance' error");
			return;
		}
		double distance1 = GetPointDistance(center, LPoint_Set(box[0], box[1])); // x0 y0
		double distance2 = GetPointDistance(center, LPoint_Set(box[0], box[3])); // x0 y1
		double distance3 = GetPointDistance(center, LPoint_Set(box[2], box[1])); // x1 y0
		double distance4 = GetPointDistance(center, LPoint_Set(box[2], box[3])); // x1 y1

		// long time long too big, convert to double to calculate
		double radius_max = max(max(distance1, distance2), max(distance3, distance4));
		// calculate how many ring need
		int maxRingLayer = (int)(radius_max / (2 * r + distance));
		int i, j;
		for (i = 0; i < maxRingLayer; i++)
		{
			// calculate how many circle in one ring
			double radius_ring = (i + 1) * (2 * r + distance);
			int maxCircle = 2 * M_PI * radius_ring / (2 * r + distance);
			double rad_offset = 2 * M_PI / maxCircle;
			for (j = 0; j < maxCircle; j++)
			{
				int yOffset = (i + 1) * (2 * r + distance) * sin(rad + rad_offset * j);
				int xOffset = (i + 1) * (2 * r + distance) * cos(rad + rad_offset * j);
				long center_x = center.x + xOffset;
				long center_y = center.y + yOffset;
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if (LObject_DistanceToPoint(biggerObject, centerOffset, File_Now) < 0)
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
			}
		}

		LDisplay_Refresh();
	}

	void ArrayInObjectByDistanceHexagon()
	{
		LPoint *points = NULL;
		int *num = 0;
		long *_distance = 0;
		_ArrayInObjectByDistanceHexagon(&points, &num, &_distance);
		LDisplay_Refresh();
	}

	void _ArrayInObjectByDistanceHexagon(LPoint * *points, int *num, long *_distance)
	{
		// Dynamic size
		int initalSize = 100;
		*points = (LPoint *)malloc(initalSize * sizeof(LPoint));

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

		double area1 = LObject_Area(object1);
		double area2 = LObject_Area(object2);

		LObject smallerObject = area1 > area2 ? object2 : object1;
		LObject biggerObject = area1 > area2 ? object1 : object2;

		if (LObject_GetShape(smallerObject) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		//****************************Input Params****************************//
		LDialogItem Dialog_Items[1] = {{"Array Edge Distance(um)", "1"}};
		long distance;
		double deg = 60;
		if (LDialog_MultiLineInputBox("Hexagon Properties", Dialog_Items, 1))
		{
			distance = (long)(atof(Dialog_Items[0].value) * 1000); // get the distance
			*_distance = distance;
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * M_PI / 180;
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);

		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, smallerObject);

		long box[4];
		box[0] = WORLD_MAX;	 // x0
		box[1] = WORLD_MAX;	 // y0
		box[2] = -WORLD_MAX; // x1
		box[3] = -WORLD_MAX; // y1
		GetObjectCoord(biggerObject, box);
		LPoint center = LCircle_GetCenter(smallerObject);
		LCoord r = LCircle_GetRadius(smallerObject);
		if (2 * r + distance <= 0)
		{
			LDialog_AlertBox("Value of 'distance' error");
			return;
		}
		// phase 1 + +
		int n = 0;
		double yOffset = 0;
		double xOffset = 0;
		while (center.y + n * (2 * r + distance) + yOffset < box[3])
		{
			int m = 0;
			while (center.x + m * (2 * r + distance) + xOffset < box[2])
			{
				if (n == 0 && m == 0)
				{
					m++;
					continue;
				}
				long center_x = center.x + m * (2 * r + distance) + xOffset;
				long center_y = center.y + n * (2 * r + distance) + yOffset;
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if (LObject_DistanceToPoint(biggerObject, centerOffset, File_Now) < 0)
				{
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
					(*points)[*num] = centerOffset;
					(*num) = (*num) + 1;
					if (*num > initalSize / 2)
					{
						initalSize *= 2;
						*points = (LPoint *)realloc(*points, initalSize * sizeof(LPoint));
					}
				}
				m++;
			}
			n++;
			yOffset = (2 * r + distance) * (sin(rad) - 1) * n;
			xOffset = (2 * r + distance) * cos(rad) * (n % 2);
		}
		// phase 2 + -
		n = 1;
		xOffset = 0;
		yOffset = (2 * r + distance) * (sin(rad) - 1) * n;
		while (center.y - (n * (2 * r + distance) + yOffset) > box[1])
		{
			int m = 0;
			xOffset = (2 * r + distance) * cos(rad) * (n % 2);
			while (center.x + m * (2 * r + distance) + xOffset < box[2])
			{
				long center_x = center.x + m * (2 * r + distance) + xOffset;
				long center_y = center.y - (n * (2 * r + distance) + yOffset);
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if (LObject_DistanceToPoint(biggerObject, centerOffset, File_Now) < 0)
				{
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
					(*points)[*num] = centerOffset;
					(*num) = (*num) + 1;
					if (*num > initalSize / 2)
					{
						initalSize *= 2;
						*points = (LPoint *)realloc(*points, initalSize * sizeof(LPoint));
					}
				}
				m++;
			}
			n++;
			yOffset = (2 * r + distance) * (sin(rad) - 1) * n;
		}
		// phase 3 - -
		n = 0;
		xOffset = 0;
		yOffset = (2 * r + distance) * (sin(rad) - 1) * n;
		while (center.y - (n * (2 * r + distance) + yOffset) > box[1])
		{
			int m = 0;
			xOffset = (2 * r + distance) * cos(rad) * (n % 2);
			while (center.x - (m * (2 * r + distance) + xOffset) > box[0])
			{
				if (n % 2 == 0 && m == 0)
				{
					m++;
					continue;
				}
				long center_x = center.x - (m * (2 * r + distance) + xOffset);
				long center_y = center.y - (n * (2 * r + distance) + yOffset);
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if (LObject_DistanceToPoint(biggerObject, centerOffset, File_Now) < 0)
				{
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
					(*points)[*num] = centerOffset;
					(*num) = (*num) + 1;
					if (*num > initalSize / 2)
					{
						initalSize *= 2;
						*points = (LPoint *)realloc(*points, initalSize * sizeof(LPoint));
					}
				}
				m++;
			}
			n++;
			yOffset = (2 * r + distance) * (sin(rad) - 1) * n;
		}
		// phase 4 - +
		n = 1;
		xOffset = 0;
		yOffset = (2 * r + distance) * (sin(rad) - 1) * n;
		while (center.y + n * (2 * r + distance) + yOffset < box[3])
		{
			int m = 0;
			xOffset = (2 * r + distance) * cos(rad) * (n % 2);
			while (center.x - (m * (2 * r + distance) + xOffset) > box[0])
			{
				if (n % 2 == 0 && m == 0)
				{
					m++;
					continue;
				}
				long center_x = center.x - (m * (2 * r + distance) + xOffset);
				long center_y = center.y + n * (2 * r + distance) + yOffset;
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if (LObject_DistanceToPoint(biggerObject, centerOffset, File_Now) < 0)
				{
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
					(*points)[*num] = centerOffset;
					(*num) = (*num) + 1;
					if (*num > initalSize / 2)
					{
						initalSize *= 2;
						*points = (LPoint *)realloc(*points, initalSize * sizeof(LPoint));
					}
				}
				m++;
			}
			n++;
			yOffset = (2 * r + distance) * (sin(rad) - 1) * n;
		}
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

	void ArrayByEdgeDistanceOneDimension()
	{
		//****************************Input Params****************************//

		LDialogItem Dialog_Items[3] = {{"Array Edge Distance(um)", "1"},
									   {"Array Number", "2"},
									   {"Array Dimension deg (0-360)", "0"}};
		long distance;
		int arrayNumber;
		double deg;
		if (LDialog_MultiLineInputBox("Array By Edge Distance One Dimension", Dialog_Items, 3))
		{
			distance = (long)(atof(Dialog_Items[0].value) * 1000); // get the distance
			arrayNumber = atoi(Dialog_Items[1].value);			   // get the arrayNumber
			deg = atof(Dialog_Items[2].value);					   // get the deg
		}
		else
		{
			return;
		}

		//****************************Input Params****************************//
		if ((distance >= 0 || arrayNumber > 1) || (deg >= 0 && deg <= 360))
		{
			CopySelectedObjectsByEdgeDistance(arrayNumber, distance, deg);
		}

		LDisplay_Refresh();
	}

	void CopySelectedObjectsByEdgeDistance(int arrayNumber, long distance, double deg)
	{

		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			double rad = deg * M_PI / 180; //
			double *offset = (double *)malloc(sizeof(double));
			if (CalculateOffset(selectedObject, distance, rad, offset) == 1)
			{
				double xOffset = offset[0]; //
				double yOffset = offset[1]; //
				int i;
				for (i = 1; i < arrayNumber; i++)
				{
					CopyObject(selectedObject, xOffset, yOffset);
				}
				counter++;
			}
			selectedInital = LSelection_GetNext(selectedInital);
		}
	}

	int CalculateOffset(LObject selectedObject, long distance, double rad, double *offset)
	{
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		// Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			return 0;
		}
		case 1: // LCircle
		{
			LCoord r = LCircle_GetRadius(selectedObject);
			offset[0] = (2 * r + distance) * cos(rad);
			offset[1] = (2 * r + distance) * sin(rad);
			return 1;
		}
		case 3: // LPolygon
		{
			return 0;
		}
		case 4: // LTorus
		{
			LTorusParams tParams;
			LStatus lStatus = LTorus_GetParams(selectedObject, &tParams);
			if (lStatus == 0)
			{
				offset[0] = (2 * tParams.nOuterRadius + distance) * cos(rad);
				offset[1] = (2 * tParams.nOuterRadius + distance) * sin(rad);
			}
			return 1;
		}
		case 5: // LPie
		{
			LPieParams pParams;
			LStatus lStatus = LPie_GetParams(selectedObject, &pParams);
			if (lStatus == 0)
			{
				offset[0] = (2 * pParams.nRadius + distance) * cos(rad);
				offset[1] = (2 * pParams.nRadius + distance) * sin(rad);
			}
			return 1;
		}
		default:
			return 0;
		}
		//
	}

	void ArrayByOffsetOneDimension()
	{
		//****************************Input Params****************************//

		LDialogItem Dialog_Items[3] = {{"Array Center Distance(um)", "1"},
									   {"Array Number", "2"},
									   {"Array Dimension deg (0-360)", "0"}};
		long distance;
		int arrayNumber;
		double deg;
		if (LDialog_MultiLineInputBox("Array By Offset One Dimension", Dialog_Items, 3))
		{
			distance = (long)(atof(Dialog_Items[0].value) * 1000); // get the distance
			arrayNumber = atoi(Dialog_Items[1].value);			   // get the arrayNumber
			deg = atof(Dialog_Items[2].value);					   // get the deg
		}
		else
		{
			return;
		}

		//****************************Input Params****************************//
		if ((distance >= 0 || arrayNumber > 1) || (deg >= 0 && deg <= 360))
		{
			double rad = deg * M_PI / 180;
			double xOffset = distance * cos(rad);
			double yOffset = distance * sin(rad);
			CopySelectedObjects(arrayNumber, xOffset, yOffset);
		}

		LDisplay_Refresh();
	}

	void CopySelectedObjects(int arrayNumber, double xOffset, double yOffset)
	{
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			int i;
			for (i = 1; i < arrayNumber; i++)
			{
				CopyObject(selectedObject, (long)(i * xOffset), (long)(i * yOffset));
			}
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
	}

	LObject Copy_Rect(LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, originObject);
		LRect rect = LBox_GetRect(originObject);
		long rect_x0 = rect.x0 + xOffset;
		long rect_y0 = rect.y0 + yOffset;
		long rect_x1 = rect.x1 + xOffset;
		long rect_y1 = rect.y1 + yOffset;
		return LBox_New(Cell_Now, LLayer_Now, rect_x0, rect_y0, rect_x1, rect_y1);
	}

	LObject Copy_Circle(LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, originObject);
		LPoint center = LCircle_GetCenter(originObject);
		LCoord r = LCircle_GetRadius(originObject);

		long center_x = center.x + xOffset;
		long center_y = center.y + yOffset;
		LPoint centerOffset = LPoint_Set(center_x, center_y);
		return LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
	}

	LObject Copy_Polygon(LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, originObject);
		LVertex vertex = LObject_GetVertexList(originObject);
		long cnt = LVertex_GetCount(originObject);
		LPoint *points;
		points = (LPoint *)malloc((cnt) * sizeof(LPoint));
		int j = 0;
		while (vertex != NULL)
		{
			LPoint point = LVertex_GetPoint(vertex);
			long point_x = point.x + xOffset;
			long point_y = point.y + yOffset;
			points[j] = LPoint_Set(point_x, point_y);
			j++;
			vertex = LVertex_GetNext(vertex);
		}
		return LPolygon_New(Cell_Now, LLayer_Now, points, cnt);
	}

	LObject Copy_Torus(LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, originObject);
		LTorusParams tParams;
		LStatus lStatus = LTorus_GetParams(originObject, &tParams);
		if (lStatus == 0)
		{
			/* Define parameters for a torus */
			LTorusParams _tParams;
			long center_x = tParams.ptCenter.x + xOffset;
			long center_y = tParams.ptCenter.y + yOffset;
			_tParams.ptCenter = LPoint_Set(center_x, center_y);
			_tParams.nInnerRadius = tParams.nInnerRadius;
			_tParams.nOuterRadius = tParams.nOuterRadius;
			_tParams.dStartAngle = tParams.dStartAngle;
			_tParams.dStopAngle = tParams.dStopAngle;
			return LTorus_CreateNew(Cell_Now, LLayer_Now, &_tParams);
		}
		return NULL;
	}

	LObject Copy_Pie(LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, originObject);
		LPieParams pParams;
		LStatus lStatus = LPie_GetParams(originObject, &pParams);
		if (lStatus == 0)
		{
			LPieParams _pParams;
			long center_x = pParams.ptCenter.x + xOffset;
			long center_y = pParams.ptCenter.y + yOffset;
			_pParams.ptCenter = LPoint_Set(center_x, center_y);
			_pParams.nRadius = pParams.nRadius;
			_pParams.dStartAngle = pParams.dStartAngle;
			_pParams.dStopAngle = pParams.dStopAngle;
			return LPie_CreateNew(Cell_Now, LLayer_Now, &_pParams);
		}
		return NULL;
	}

	LObject CopyObject(LObject selectedObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();

		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectedObject);
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);

		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			return Copy_Rect(selectedObject, xOffset, yOffset);
		}
		case 1: // LCircle
		{
			return Copy_Circle(selectedObject, xOffset, yOffset);
		}
		case 3: // LPolygon
		{
			return Copy_Polygon(selectedObject, xOffset, yOffset);
		}
		case 4: // LTorus
		{
			return Copy_Torus(selectedObject, xOffset, yOffset);
		}
		case 5: // LPie
		{
			return Copy_Pie(selectedObject, xOffset, yOffset);
		}
		default:
			return NULL;
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

	double GetPointDistance(LPoint point1, LPoint point2)
	{
		return sqrt((double)(point1.x - point2.x) * (double)(point1.x - point2.x) + (double)(point1.y - point2.y) * (point1.y - point2.y));
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

	LPoint RotatePoint(LPoint p1, LPoint p2, double rad)
	{
		double relX = p1.x - p2.x;
		double relY = p1.y - p2.y;

		long x = relX * cos(rad) - relY * sin(rad) + p2.x;
		long y = relX * sin(rad) + relY * cos(rad) + p2.y;
		return LPoint_Set(x, y);
	}

	LPoint MirrorCoordinates(LPoint p, LPoint linePoint, double rad)
	{
		// Calculate the tangent value of the line's angle
		double slopeTan = tan(rad);
		// Handle the case when tan(pi/2) is encountered
		if (fabs(rad - M_PI / 2) < 1e-6)
		{
			LPoint mirrorPoint;
			mirrorPoint.x = 2 * linePoint.x - p.x;
			mirrorPoint.y = p.y;
			return mirrorPoint;
		}

		// Calculate the constant term 'c' in the equation y = mx + c
		double c = linePoint.y - slopeTan * linePoint.x;

		// Calculate the perpendicular distance from the point to the line
		double distance = fabs(slopeTan * p.x - p.y + c) / sqrt(1 + slopeTan * slopeTan);

		double A = slopeTan;
		double B = -1;
		double C = c;
		// Calculate the mirror coordinates of the point with respect to the line

		long mirrorx = 2 * (linePoint.y - c + slopeTan * p.x + slopeTan * slopeTan * p.y) / (1 + slopeTan * slopeTan);
		long mirrory = 2 * (slopeTan * linePoint.x - slopeTan * p.x + p.y + c) / (1 + slopeTan * slopeTan) - p.y;

		mirrorx = -1 * (2 * A * B * p.y + (A * A - B * B) * p.x + 2 * A * C) / (B * B + A * A);
		mirrory = -1 * (2 * A * B * p.x - (A * A - B * B) * p.y + 2 * B * C) / (B * B + A * A);
		return LPoint_Set(mirrorx, mirrory);
	}

	void Array_func(void)
	{
		// LMacro_BindToMenu( const char* menu, const char* macro_desc, const char* function_name );
		LMacro_Register("ArrayByRingSetDistance_func", "ArrayByRingSetDistance");
		LMacro_Register("ArrayByRingSetRad_func", "ArrayByRingSetRad");
		LMacro_Register("ArrayByOffsetOneDimension_func", "ArrayByOffsetOneDimension");
		LMacro_Register("ArrayByEdgeDistanceOneDimension_func", "ArrayByEdgeDistanceOneDimension");
		LMacro_Register("ArrayInObjectByDistanceHexagon_func", "ArrayInObjectByDistanceHexagon");
		LMacro_Register("ArrayInObjectByRing_func", "ArrayInObjectByRing");
		LMacro_Register("RotateObjectsByPoint_func", "RotateObjectsByPoint");
		LMacro_Register("MirrorObjectsByPointAndRad_func", "MirrorObjectsByPointAndRad");
	}
} /* end of module Array_Module */

Array_func();