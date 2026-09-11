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
// #include "Ledit_Array.h"
#include "ldata.h" /* Main UPI header. */
#include "Ledit_Safety.h"

	void MirrorObjectsByPointAndRad();
	LObject Mirror_Rect(LObject selectRect, LPoint mirrorPoint, double rad);
	LObject Mirror_Circle(LObject selectCircle, LPoint mirrorPoint, double rad);
	LObject Mirror_Polygon(LObject selectPolygon, LPoint mirrorPoint, double rad);
	LObject Mirror_Torus(LObject selectTorus, LPoint mirrorPoint, double rad);
	LObject Mirror_Pie(LObject selectPie, LPoint mirrorPoint, double rad);
	LObject MirrorObject(LObject selectedObject, LPoint mirrorPoint, double rad, int keep);
	void RotateObjectsByPoint();
	LObject Rotate_Rect(LObject selectRect, LPoint rotateCenter, double rad);
	LObject Rotate_Circle(LObject selectCircle, LPoint rotateCenter, double rad);
	LObject Rotate_Polygon(LObject selectPolygon, LPoint rotateCenter, double rad);
	LObject Rotate_Torus(LObject selectTorus, LPoint rotateCenter, double rad);
	LObject Rotate_Pie(LObject selectPie, LPoint rotateCenter, double rad);
	LObject RotateObject(LObject selectedObject, LPoint rotateCenter, double rad, int keep);
	void ArrayByRingSetRad();
	void ArrayByRingSetDistance();
	void ArrayInObjectByRing();
	void ArrayInObjectByDistanceHexagon();
	void _ArrayInObjectByDistanceHexagon(LPoint * *points, int *num, long *_distance);
	int LSelection_GetNumber(LSelection selectedInital);
	void ArrayByEdgeDistanceOneDimension();
	void CopySelectedObjectsByEdgeDistance(int arrayNumber, long distance, double deg);
	int CalculateOffset(LObject selectedObject, long distance, double rad, double *offset);
	void ArrayByOffsetOneDimension();
	void CopySelectedObjects(int arrayNumber, double xOffset, double yOffset);
	LObject Copy_Rect(LObject originObject, long xOffset, long yOffset);
	LObject Copy_Circle(LObject originObject, long xOffset, long yOffset);
	LObject Copy_Polygon(LObject originObject, long xOffset, long yOffset);
	LObject Copy_Torus(LObject originObject, long xOffset, long yOffset);
	LObject Copy_Pie(LObject originObject, long xOffset, long yOffset);
	LObject CopyObject(LObject selectedObject, long xOffset, long yOffset);
	void GetObjectCoord(LObject selectedObject, long *box);
	double GetPointDistance(LPoint point1, LPoint point2);
	double CalculatePolarAngle(LPoint p1, LPoint p2);
	LPoint RotatePoint(LPoint p1, LPoint p2, double rad);
	LPoint MirrorCoordinates(LPoint p, LPoint linePoint, double rad);
	void Diffusion_Selected_Object_ByOffset();
	void Diffusion_Selected_Object_ByRadius();
	void Array_func(void);

	void MirrorObjectsByPointAndRad()
	{
        if (!LM_Context()) return;
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[4] = {{"Mirror Center X Coordinate (um)", "0"},
									   {"Mirror Center Y Coordinate (um)", "0"},
									   {"Mirror Degree (0-180)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long xcoord;
		long ycoord;
		double rotate;
		int keep;
		if (LM_Dialog("Mirror Objects By Point And Rad", Dialog_Items, 4))
		{
			xcoord = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the xcoord
			ycoord = (long)(atof(Dialog_Items[1].value) * LM_UnitsPerMicron()); // get the ycoord
			rotate = atof(Dialog_Items[2].value);				 // get the rotate
			keep = (int)atof(Dialog_Items[3].value);					 // get the keep
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
			LSelection nextSelection = LSelection_GetNext(selectedInital);
            LObject selectedObject = LSelection_GetObject(selectedInital);

			LPoint mirrorPoint = LPoint_Set(xcoord, ycoord);
			MirrorObject(selectedObject, mirrorPoint, rad, keep);
			selectedInital = nextSelection;
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
		return LM_NewPolygon(Cell_Now, LLayer_Now, points, 4);
	}

	LObject Mirror_Circle(LObject selectCircle, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectCircle);
		LPoint center = LCircle_GetCenter(selectCircle);
		LCoord r = LCircle_GetRadius(selectCircle);

		LPoint newPoint = MirrorCoordinates(center, mirrorPoint, rad);

		return LM_NewCircle(Cell_Now, LLayer_Now, newPoint, r);
	}

	LObject Mirror_Polygon(LObject selectPolygon, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectPolygon);
		LVertex vertex = LObject_GetVertexList(selectPolygon);
		long cnt = LVertex_GetCount(selectPolygon);
		LPoint *points;
		if (cnt < 3 || cnt > LM_MAX_POINTS || LPolygon_HasCurve(selectPolygon)) {
            LDialog_AlertBox("Expected a straight polygon with 3..100000 vertices."); return NULL;
        }
        points = (LPoint *)malloc(cnt * sizeof(LPoint));
        if (!points) return NULL;
		int j = 0;
		while (vertex != NULL && j < cnt)
		{
			LPoint point = LVertex_GetPoint(vertex);
			LPoint newPoint = MirrorCoordinates(point, mirrorPoint, rad);
			points[j] = newPoint;
			j++;
			vertex = LVertex_GetNext(vertex);
		}
		LObject result = LM_NewPolygon(Cell_Now, LLayer_Now, points, j);
        free(points);
        return result;
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
        if (!LM_CanTransform(selectedObject, mirrorPoint, rad, 1)) { LDialog_AlertBox("Transform exceeds coordinate limits."); return NULL; }
        LObject result = NULL;
        switch (LObject_GetShape(selectedObject)) {
        case LBox: result = Mirror_Rect(selectedObject, mirrorPoint, rad); break;
        case LCircle: result = Mirror_Circle(selectedObject, mirrorPoint, rad); break;
        case LPolygon: result = Mirror_Polygon(selectedObject, mirrorPoint, rad); break;
        case LTorus: result = Mirror_Torus(selectedObject, mirrorPoint, rad); break;
        case LPie: result = Mirror_Pie(selectedObject, mirrorPoint, rad); break;
        default: return NULL;
        }
        if (result && !keep) LObject_Delete(LCell_GetVisible(), selectedObject);
        return result;
    }

	void RotateObjectsByPoint()
	{
        if (!LM_Context()) return;
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[4] = {{"Rotate Center X Coordinate (um)", "0"},
									   {"Rotate Center Y Coordinate (um)", "0"},
									   {"Rotate degree (0-360)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long xcoord;
		long ycoord;
		double rotate;
		int keep;
		if (LM_Dialog("Rotate Objects By Point", Dialog_Items, 4))
		{
			xcoord = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the xcoord
			ycoord = (long)(atof(Dialog_Items[1].value) * LM_UnitsPerMicron()); // get the ycoord
			rotate = atof(Dialog_Items[2].value);				 // get the rotate
			keep = (int)atof(Dialog_Items[3].value);					 // get the rotate
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
			LSelection nextSelection = LSelection_GetNext(selectedInital);
            LObject selectedObject = LSelection_GetObject(selectedInital);

			LPoint rotateCenter = LPoint_Set(xcoord, ycoord);
			RotateObject(selectedObject, rotateCenter, rad, keep);
			selectedInital = nextSelection;
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
		return LM_NewPolygon(Cell_Now, LLayer_Now, points, 4);
	}

	LObject Rotate_Circle(LObject selectCircle, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectCircle);
		LPoint center = LCircle_GetCenter(selectCircle);
		LCoord r = LCircle_GetRadius(selectCircle);

		LPoint newPoint = RotatePoint(center, rotateCenter, rad);

		return LM_NewCircle(Cell_Now, LLayer_Now, newPoint, r);
	}

	LObject Rotate_Polygon(LObject selectPolygon, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectPolygon);
		LVertex vertex = LObject_GetVertexList(selectPolygon);
		long cnt = LVertex_GetCount(selectPolygon);
		LPoint *points;
		if (cnt < 3 || cnt > LM_MAX_POINTS || LPolygon_HasCurve(selectPolygon)) {
            LDialog_AlertBox("Expected a straight polygon with 3..100000 vertices."); return NULL;
        }
        points = (LPoint *)malloc(cnt * sizeof(LPoint));
        if (!points) return NULL;
		int j = 0;
		while (vertex != NULL && j < cnt)
		{
			LPoint point = LVertex_GetPoint(vertex);
			LPoint newPoint = RotatePoint(point, rotateCenter, rad);

			points[j] = newPoint;
			j++;
			vertex = LVertex_GetNext(vertex);
		}
		LObject result = LM_NewPolygon(Cell_Now, LLayer_Now, points, j);
        free(points);
        return result;
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
        if (!LM_CanTransform(selectedObject, rotateCenter, rad, 0)) { LDialog_AlertBox("Transform exceeds coordinate limits."); return NULL; }
        LObject result = NULL;
        switch (LObject_GetShape(selectedObject)) {
        case LBox: result = Rotate_Rect(selectedObject, rotateCenter, rad); break;
        case LCircle: result = Rotate_Circle(selectedObject, rotateCenter, rad); break;
        case LPolygon: result = Rotate_Polygon(selectedObject, rotateCenter, rad); break;
        case LTorus: result = Rotate_Torus(selectedObject, rotateCenter, rad); break;
        case LPie: result = Rotate_Pie(selectedObject, rotateCenter, rad); break;
        default: return NULL;
        }
        if (result && !keep) LObject_Delete(LCell_GetVisible(), selectedObject);
        return result;
    }

	void ArrayByRingSetRad()
	{
        if (!LM_Context()) return;
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
		if (LM_Dialog("Array By Ring Set Rad", Dialog_Items, 3))
		{
			degOffset = atof(Dialog_Items[0].value);			 // get the distance
			deg = atof(Dialog_Items[1].value);					 // get the deg
			radius = (long)(atof(Dialog_Items[2].value) * LM_UnitsPerMicron()); // get the radius
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		if (!(degOffset > 0 && degOffset <= 360) || radius <= 0 || ceil(360 / degOffset) > LM_MAX_OBJECTS) {
            LDialog_AlertBox("Use a positive radius and angle step; at most 100000 copies."); return;
        }
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
        if (!LM_Context()) return;
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
		if (LM_Dialog("Array By Ring Set Distance", Dialog_Items, 4))
		{
			distance = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the distance
			deg = atof(Dialog_Items[1].value);					   // get the deg
			radius = (long)(atof(Dialog_Items[2].value) * LM_UnitsPerMicron());   // get the radius
			adapDis = (int)atof(Dialog_Items[3].value);				   // Adaptive Distance
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
		double spacing = 2.0 * r + distance;
        if (radius <= 0 || spacing > 2.0 * radius) {
            LDialog_AlertBox("Ring diameter must be at least the center spacing."); return;
        }
        double rad_offset = 2 * asin(spacing / (2.0 * radius));
        double count = floor(2 * M_PI / rad_offset + 1e-10);
        if (count < 2 || count > LM_MAX_OBJECTS) {
            LDialog_AlertBox("Ring copy count must be 2..100000."); return;
        }
        int maxCircle = (int)count;
        if (adapDis == 1) rad_offset = 2 * M_PI / maxCircle;
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
        if (!LM_Context()) return;
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
		if (LM_Dialog("Array In Object By Ring", Dialog_Items, 2))
		{
			distance = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the distance
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
		double radius_max = fmax(fmax(distance1, distance2), fmax(distance3, distance4));
		// calculate how many ring need
		if (radius_max / (2.0 * r + distance) > sqrt(LM_MAX_OBJECTS / M_PI) - 1) {
            LDialog_AlertBox("Ring array exceeds the candidate limit."); return;
        }
        int maxRingLayer = (int)(radius_max / (2 * r + distance));
		int i, j;
		for (i = 0; i < maxRingLayer; i++)
		{
			// calculate how many circle in one ring
			double radius_ring = (i + 1.0) * (2.0 * r + distance);
			int maxCircle = 2 * M_PI * radius_ring / (2 * r + distance);
			double rad_offset = 2 * M_PI / maxCircle;
			for (j = 0; j < maxCircle; j++)
			{
				int yOffset = (i + 1.0) * (2.0 * r + distance) * sin(rad + rad_offset * j);
				int xOffset = (i + 1.0) * (2.0 * r + distance) * cos(rad + rad_offset * j);
				long center_x = center.x + xOffset;
				long center_y = center.y + yOffset;
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if (LObject_DistanceToPoint(biggerObject, centerOffset, File_Now) < 0)
					LM_NewCircle(Cell_Now, LLayer_Now, centerOffset, r);
			}
		}

		LDisplay_Refresh();
	}

	void ArrayInObjectByDistanceHexagon()
	{
        if (!LM_Context()) return;
		LPoint *points = NULL;
		int num = 0;
		long _distance = 0;
		_ArrayInObjectByDistanceHexagon(&points, &num, &_distance);
        free(points);
		LDisplay_Refresh();
	}

	void _ArrayInObjectByDistanceHexagon(LPoint * *points, int *num, long *_distance)
	{
        LSelection selection = LSelection_GetList();
        LObject a, b, small, big;
        LPoint center;
        LRect box;
        LDialogItem items[1] = {{"Array Edge Distance(um)", "1"}};
        double spacing, rowHeight, rows, cols;
        long radius, gap;
        int row, col, row0, row1, col0, col1, capacity;
        *points = NULL; *num = 0; *_distance = 0;
        if (LSelection_GetNumber(selection) != 2) {
            LDialog_AlertBox("Select a circle and a containing object."); return;
        }
        a = LSelection_GetObject(selection);
        b = LSelection_GetObject(LSelection_GetNext(selection));
        small = LObject_Area(a) < LObject_Area(b) ? a : b;
        big = small == a ? b : a;
        if (LObject_GetShape(small) != LCircle) return;
        if (!LM_Dialog("Hexagon Properties", items, 1)) return;
        gap = (long)(atof(items[0].value) * LM_UnitsPerMicron());
        radius = LCircle_GetRadius(small);
        spacing = 2.0 * radius + gap;
        if (spacing <= 0) { LDialog_AlertBox("Center spacing must be positive."); return; }
        center = LCircle_GetCenter(small); box = LObject_GetMbb(big);
        rowHeight = spacing * sqrt(3.0) / 2;
        rows = ceil(((double)box.y1 - box.y0) / rowHeight) + 3;
        cols = ceil(((double)box.x1 - box.x0) / spacing) + 3;
        if (rows * cols > LM_MAX_OBJECTS) {
            LDialog_AlertBox("Hexagonal array exceeds 100000 candidate positions."); return;
        }
        capacity = (int)(rows * cols);
        *points = (LPoint *)malloc(capacity * sizeof(LPoint));
        if (!*points) return;
        row0 = (int)ceil(((double)box.y0 - center.y) / rowHeight);
        row1 = (int)floor(((double)box.y1 - center.y) / rowHeight);
        for (row = row0; row <= row1; ++row) {
            double shift = row % 2 ? spacing / 2 : 0;
            col0 = (int)ceil(((double)box.x0 - center.x - shift) / spacing);
            col1 = (int)floor(((double)box.x1 - center.x - shift) / spacing);
            for (col = col0; col <= col1; ++col) {
                LPoint p;
                if (row == 0 && col == 0) continue;
                p = LPoint_Set((long)(center.x + col * spacing + shift), (long)(center.y + row * rowHeight));
                if (LObject_DistanceToPoint(big, p, LCell_GetFile(LCell_GetVisible())) < 0) {
                    if (!CopyObject(small, p.x - center.x, p.y - center.y)) {
                        LDialog_AlertBox("Array stopped: object creation failed."); return;
                    }
                    (*points)[(*num)++] = p;
                }
            }
        }
        *_distance = gap;
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
        if (!LM_Context()) return;
		//****************************Input Params****************************//

		LDialogItem Dialog_Items[3] = {{"Array Edge Distance(um)", "1"},
									   {"Array Number", "2"},
									   {"Array Dimension deg (0-360)", "0"}};
		long distance;
		int arrayNumber;
		double deg;
		if (LM_Dialog("Array By Edge Distance One Dimension", Dialog_Items, 3))
		{
			distance = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the distance
			arrayNumber = (int)atof(Dialog_Items[1].value);			   // get the arrayNumber
			deg = atof(Dialog_Items[2].value);					   // get the deg
		}
		else
		{
			return;
		}

		//****************************Input Params****************************//
		if (distance >= 0 && arrayNumber > 1 && arrayNumber <= LM_MAX_OBJECTS && deg >= 0 && deg <= 360)
		{
			CopySelectedObjectsByEdgeDistance(arrayNumber, distance, deg);
		}

		LDisplay_Refresh();
	}

	void CopySelectedObjectsByEdgeDistance(int arrayNumber, long distance, double deg)
	{
        if (arrayNumber < 2 || (double)arrayNumber * LSelection_GetNumber(LSelection_GetList()) > LM_MAX_OBJECTS) { LDialog_AlertBox("Array exceeds 100000 objects."); return; }

		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			double rad = deg * M_PI / 180; //
			double offset[2] = {0, 0};
			if (CalculateOffset(selectedObject, distance, rad, offset) == 1)
			{
				double xOffset = offset[0]; //
				double yOffset = offset[1]; //
				if (!LM_CanMove(selectedObject, (arrayNumber - 1.0) * xOffset, (arrayNumber - 1.0) * yOffset)) { LDialog_AlertBox("Array exceeds coordinate limits."); return; }
                int i;
				for (i = 1; i < arrayNumber; i++)
				{
					CopyObject(selectedObject, i * xOffset, i * yOffset);
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
			return lStatus == LStatusOK;
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
			return lStatus == LStatusOK;
		}
		default:
			return 0;
		}
		//
	}

	void ArrayByOffsetOneDimension()
	{
        if (!LM_Context()) return;
		//****************************Input Params****************************//

		LDialogItem Dialog_Items[3] = {{"Array Center Distance(um)", "1"},
									   {"Array Number", "2"},
									   {"Array Dimension deg (0-360)", "0"}};
		long distance;
		int arrayNumber;
		double deg;
		if (LM_Dialog("Array By Offset One Dimension", Dialog_Items, 3))
		{
			distance = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the distance
			arrayNumber = (int)atof(Dialog_Items[1].value);			   // get the arrayNumber
			deg = atof(Dialog_Items[2].value);					   // get the deg
		}
		else
		{
			return;
		}

		//****************************Input Params****************************//
		if (distance >= 0 && arrayNumber > 1 && arrayNumber <= LM_MAX_OBJECTS && deg >= 0 && deg <= 360)
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
        if (arrayNumber < 2 || (double)arrayNumber * LSelection_GetNumber(LSelection_GetList()) > LM_MAX_OBJECTS) { LDialog_AlertBox("Array exceeds 100000 objects."); return; }
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			if (!LM_CanMove(selectedObject, (arrayNumber - 1.0) * xOffset, (arrayNumber - 1.0) * yOffset)) { LDialog_AlertBox("Array exceeds coordinate limits."); return; }
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
		return LM_NewCircle(Cell_Now, LLayer_Now, centerOffset, r);
	}

	LObject Copy_Polygon(LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, originObject);
		LVertex vertex = LObject_GetVertexList(originObject);
		long cnt = LVertex_GetCount(originObject);
		LPoint *points;
		if (cnt < 3 || cnt > LM_MAX_POINTS || LPolygon_HasCurve(originObject)) {
            LDialog_AlertBox("Expected a straight polygon with 3..100000 vertices."); return NULL;
        }
        points = (LPoint *)malloc(cnt * sizeof(LPoint));
        if (!points) return NULL;
		int j = 0;
		while (vertex != NULL && j < cnt)
		{
			LPoint point = LVertex_GetPoint(vertex);
			long point_x = point.x + xOffset;
			long point_y = point.y + yOffset;
			points[j] = LPoint_Set(point_x, point_y);
			j++;
			vertex = LVertex_GetNext(vertex);
		}
		LObject result = LM_NewPolygon(Cell_Now, LLayer_Now, points, j);
        free(points);
        return result;
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
        if (!LM_CanMove(selectedObject, xOffset, yOffset)) return NULL;
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
        LRect rect;
        if (!selectedObject) return;
        rect = LObject_GetMbb(selectedObject);
        box[0] = rect.x0; box[1] = rect.y0;
        box[2] = rect.x1; box[3] = rect.y1;
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
        double dx = (double)p.x - linePoint.x;
        double dy = (double)p.y - linePoint.y;
        double c = cos(2 * rad), t = sin(2 * rad);
        return LPoint_Set(LM_RoundCoord(linePoint.x + c * dx + t * dy),
                          LM_RoundCoord(linePoint.y + t * dx - c * dy));
    }

	void Diffusion_Selected_Object_ByOffset()
	{
        if (!LM_Context()) return;
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{"Offset (um) (Inf)", "0"},
									   {"Center Coord X (um)", "0"},
									   {"Center Coord Y (um)", "0"}};
		double offset;
		double center_x, center_y;
		if (LM_Dialog("Scale Selected Object", Dialog_Items, 3))
		{
			offset = (double)(atof(Dialog_Items[0].value)); // get the Scale Factor
			center_x = (double)(atof(Dialog_Items[1].value));
			center_y = (double)(atof(Dialog_Items[2].value));
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LPoint polygonCenter = LPoint_Set((long)(center_x * LM_UnitsPerMicron()), (long)(center_y * LM_UnitsPerMicron()));
			long box[4];
			box[0] = WORLD_MAX;	 // x0
			box[1] = WORLD_MAX;	 // y0
			box[2] = -WORLD_MAX; // x1
			box[3] = -WORLD_MAX; // y1
			GetObjectCoord(selectedObject, box);
			// calculate object center (box)
			long objectCenterX = (box[2] + box[0]) / 2;
			long objectCenterY = (box[3] + box[1]) / 2;
			long vectoryX = objectCenterX - (long)(center_x * LM_UnitsPerMicron());
			long vectoryY = objectCenterY - (long)(center_y * LM_UnitsPerMicron());
			double angle = atan2(vectoryY, vectoryX);
			long xOffset = cos(angle) * offset * LM_UnitsPerMicron();
			long yOffset = sin(angle) * offset * LM_UnitsPerMicron();
			CopyObject(selectedObject, xOffset, yOffset);

			selectedInital = LSelection_GetNext(selectedInital);
		}
		LDisplay_Refresh();
	}

	void Diffusion_Selected_Object_ByRadius()
	{
        if (!LM_Context()) return;
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{"Radius (um) (Inf)", "0"},
									   {"Center Coord X (um)", "0"},
									   {"Center Coord Y (um)", "0"}};
		double radius;
		double center_x, center_y;
		if (LM_Dialog("Scale Selected Object", Dialog_Items, 3))
		{
			radius = (double)(atof(Dialog_Items[0].value)); // get the Scale Factor
			center_x = (double)(atof(Dialog_Items[1].value));
			center_y = (double)(atof(Dialog_Items[2].value));
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LPoint polygonCenter = LPoint_Set((long)(center_x * LM_UnitsPerMicron()), (long)(center_y * LM_UnitsPerMicron()));
			long box[4];
			box[0] = WORLD_MAX;	 // x0
			box[1] = WORLD_MAX;	 // y0
			box[2] = -WORLD_MAX; // x1
			box[3] = -WORLD_MAX; // y1
			GetObjectCoord(selectedObject, box);
			// calculate object center (box)
			long objectCenterX = (box[2] + box[0]) / 2;
			long objectCenterY = (box[3] + box[1]) / 2;
			long vectoryX = objectCenterX - (long)(center_x * LM_UnitsPerMicron());
			long vectoryY = objectCenterY - (long)(center_y * LM_UnitsPerMicron());
			double distanceToCenter = sqrt(1.0 * vectoryX * vectoryX + 1.0 * vectoryY * vectoryY);
			double angle = atan2(vectoryY, vectoryX);
			long xOffset = cos(angle) * (radius * LM_UnitsPerMicron() - distanceToCenter);
			long yOffset = sin(angle) * (radius * LM_UnitsPerMicron() - distanceToCenter);
			CopyObject(selectedObject, xOffset, yOffset);

			selectedInital = LSelection_GetNext(selectedInital);
		}
		LDisplay_Refresh();
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
		LMacro_Register("Diffusion_Selected_Object_ByOffset_func", "Diffusion_Selected_Object_ByOffset");
		LMacro_Register("Diffusion_Selected_Object_ByRadius_func", "Diffusion_Selected_Object_ByRadius");
	}
} /* end of module Array_Module */

Array_func();
