/*
 * Copy, rotate, and mirror layout objects and place them in linear, circular, or hexagonal arrays.
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

module ArrayMacros
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ldata.h" /* Main UPI header. */
#include "Ledit_MacroUndo.h"
#include "Ledit_OutputLayer.h"

	void MirrorSelectionAcrossLine();
	LObject MirrorRectangle(LLayer outputLayer, LObject selectRect, LPoint mirrorPoint, double rad);
	LObject MirrorCircle(LLayer outputLayer, LObject selectCircle, LPoint mirrorPoint, double rad);
	LObject MirrorPolygon(LLayer outputLayer, LObject selectPolygon, LPoint mirrorPoint, double rad);
	LObject MirrorTorus(LLayer outputLayer, LObject selectTorus, LPoint mirrorPoint, double rad);
	LObject MirrorPie(LLayer outputLayer, LObject selectPie, LPoint mirrorPoint, double rad);
	LObject MirrorObject(LLayer outputLayer, LObject selectedObject, LPoint mirrorPoint, double rad, int keep);
	void RotateSelectionAboutPoint();
	LObject RotateRectangle(LLayer outputLayer, LObject selectRect, LPoint rotateCenter, double rad);
	LObject RotateCircle(LLayer outputLayer, LObject selectCircle, LPoint rotateCenter, double rad);
	LObject RotatePolygon(LLayer outputLayer, LObject selectPolygon, LPoint rotateCenter, double rad);
	LObject RotateTorus(LLayer outputLayer, LObject selectTorus, LPoint rotateCenter, double rad);
	LObject RotatePie(LLayer outputLayer, LObject selectPie, LPoint rotateCenter, double rad);
	LObject RotateObject(LLayer outputLayer, LObject selectedObject, LPoint rotateCenter, double rad, int keep);
	void CreateRingArrayByAngle();
	void CreateRingArrayBySpacing();
	void FillShapeWithConcentricRings();
	void FillShapeWithHexagonalArray();
	int CountSelectedObjects(LSelection selection);
	void CreateLinearArrayByEdgeSpacing();
	void CopySelectionByEdgeSpacing(LCell cell, OutputLayerChoice *output, int arrayNumber, long distance, double deg);
	int CalculateEdgeSpacingOffset(LObject selectedObject, long distance, double rad, double *offset);
	void CreateLinearArrayByOffset();
	void CopySelectionByOffset(LCell cell, OutputLayerChoice *output, int arrayNumber, double xOffset, double yOffset);
	LObject CopyRectangle(LLayer outputLayer, LObject originObject, long xOffset, long yOffset);
	LObject CopyCircle(LLayer outputLayer, LObject originObject, long xOffset, long yOffset);
	LObject CopyPolygon(LLayer outputLayer, LObject originObject, long xOffset, long yOffset);
	LObject CopyTorus(LLayer outputLayer, LObject originObject, long xOffset, long yOffset);
	LObject CopyPie(LLayer outputLayer, LObject originObject, long xOffset, long yOffset);
	LObject CopyObject(LLayer outputLayer, LObject selectedObject, long xOffset, long yOffset);
	void AccumulateObjectBounds(LObject selectedObject, long *box);
	double DistanceBetweenPoints(LPoint point1, LPoint point2);
	double CalculatePolarAngle(LPoint p1, LPoint p2);
	LPoint RotatePoint(LPoint p1, LPoint p2, double rad);
	LPoint ReflectPointAcrossLine(LPoint p, LPoint linePoint, double rad);
	void CopySelectionByRadialOffset();
	void CopySelectionToRadius();
	void RegisterArrayMacros(void);

	void MirrorSelectionAcrossLine()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

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
		double rad = rotate * M_PI / 180;
		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			selection = LSelection_GetNext(selection);
			LLayer outputLayer = ResolveObjectOutputLayer(cell, &output, selectedObject);
			if (outputLayer == NULL)
				continue;

			LPoint mirrorPoint = LPoint_Set(xcoord, ycoord);
			MirrorObject(outputLayer, selectedObject, mirrorPoint, rad, keep);
			counter++;
		}
		LDisplay_Refresh();
		ReportSkippedOutputLayers(&output);
	}

	LObject MirrorRectangle(LLayer outputLayer, LObject selectRect, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LRect rect = LBox_GetRect(selectRect);
		LPoint p1 = LPoint_Set(rect.x0, rect.y0);
		LPoint p2 = LPoint_Set(rect.x0, rect.y1);
		LPoint p3 = LPoint_Set(rect.x1, rect.y1);
		LPoint p4 = LPoint_Set(rect.x1, rect.y0);
		LPoint points[4];
		points[0] = ReflectPointAcrossLine(p1, mirrorPoint, rad);
		points[1] = ReflectPointAcrossLine(p2, mirrorPoint, rad);
		points[2] = ReflectPointAcrossLine(p3, mirrorPoint, rad);
		points[3] = ReflectPointAcrossLine(p4, mirrorPoint, rad);
		return LPolygon_New(Cell_Now, outputLayer, points, 4);
	}

	LObject MirrorCircle(LLayer outputLayer, LObject selectCircle, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LPoint center = LCircle_GetCenter(selectCircle);
		LCoord r = LCircle_GetRadius(selectCircle);

		LPoint newPoint = ReflectPointAcrossLine(center, mirrorPoint, rad);

		return LCircle_New(Cell_Now, outputLayer, newPoint, r);
	}

	LObject MirrorPolygon(LLayer outputLayer, LObject selectPolygon, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LVertex vertex = LObject_GetVertexList(selectPolygon);
		long cnt = LVertex_GetCount(selectPolygon);
		LPoint *points;
		points = (LPoint *)malloc((cnt) * sizeof(LPoint));
		int j = 0;
		while (vertex != NULL)
		{
			LPoint point = LVertex_GetPoint(vertex);
			LPoint newPoint = ReflectPointAcrossLine(point, mirrorPoint, rad);
			points[j] = newPoint;
			j++;
			vertex = LVertex_GetNext(vertex);
		}
		return LPolygon_New(Cell_Now, outputLayer, points, cnt);
	}

	LObject MirrorTorus(LLayer outputLayer, LObject selectTorus, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LTorusParams tParams;
		LStatus lStatus = LTorus_GetParams(selectTorus, &tParams);
		if (lStatus == 0)
		{
			/* Define parameters for a torus */
			LTorusParams _tParams;

			LPoint newPoint = ReflectPointAcrossLine(tParams.ptCenter, mirrorPoint, rad);

			_tParams.ptCenter = newPoint;
			_tParams.nInnerRadius = tParams.nInnerRadius;
			_tParams.nOuterRadius = tParams.nOuterRadius;

			double angle = tParams.dStopAngle - tParams.dStartAngle;
			angle = angle < 0 ? 2 * 180 + angle : angle;

			double startAngle = tParams.dStartAngle > 180 ? tParams.dStartAngle - 2 * 180 : tParams.dStartAngle;

			_tParams.dStartAngle = 2 * rad * 180 / M_PI - startAngle - angle;
			_tParams.dStopAngle = 2 * rad * 180 / M_PI - startAngle;
			return LTorus_CreateNew(Cell_Now, outputLayer, &_tParams);
		}
		return NULL;
	}

	LObject MirrorPie(LLayer outputLayer, LObject selectPie, LPoint mirrorPoint, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LPieParams pParams;
		LStatus lStatus = LPie_GetParams(selectPie, &pParams);
		if (lStatus == 0)
		{
			LPieParams _pParams;
			LPoint newPoint = ReflectPointAcrossLine(pParams.ptCenter, mirrorPoint, rad);

			_pParams.ptCenter = newPoint;
			_pParams.nRadius = pParams.nRadius;

			double angle = pParams.dStopAngle - pParams.dStartAngle;
			angle = angle < 0 ? 2 * 180 + angle : angle;

			double startAngle = pParams.dStartAngle > 180 ? pParams.dStartAngle - 2 * 180 : pParams.dStartAngle;

			_pParams.dStartAngle = 2 * rad * 180 / M_PI - startAngle - angle;
			_pParams.dStopAngle = 2 * rad * 180 / M_PI - startAngle;
			return LPie_CreateNew(Cell_Now, outputLayer, &_pParams);
		}
		return NULL;
	}

	LObject MirrorObject(LLayer outputLayer, LObject selectedObject, LPoint mirrorPoint, double rad, int keep)
	{
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);

		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			return MirrorRectangle(outputLayer, selectedObject, mirrorPoint, rad);
		}
		case 1: // LCircle
		{
			return MirrorCircle(outputLayer, selectedObject, mirrorPoint, rad);
		}
		case 3: // LPolygon
		{
			return MirrorPolygon(outputLayer, selectedObject, mirrorPoint, rad);
		}
		case 4: // LTorus
		{
			return MirrorTorus(outputLayer, selectedObject, mirrorPoint, rad);
		}
		case 5: // LPie
		{
			return MirrorPie(outputLayer, selectedObject, mirrorPoint, rad);
		}
		default:
			return NULL;
		}
		LCell Cell_Now = LCell_GetVisible();
		if (keep != 1)
			DeleteMacroObject(Cell_Now, selectedObject);
	}

	void RotateSelectionAboutPoint()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

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
		double rad = rotate * M_PI / 180;

		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			selection = LSelection_GetNext(selection);
			LLayer outputLayer = ResolveObjectOutputLayer(cell, &output, selectedObject);
			if (outputLayer == NULL)
				continue;

			LPoint rotateCenter = LPoint_Set(xcoord, ycoord);
			RotateObject(outputLayer, selectedObject, rotateCenter, rad, keep);
			counter++;
		}
		LDisplay_Refresh();
		ReportSkippedOutputLayers(&output);
	}

	LObject RotateRectangle(LLayer outputLayer, LObject selectRect, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
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
		return LPolygon_New(Cell_Now, outputLayer, points, 4);
	}

	LObject RotateCircle(LLayer outputLayer, LObject selectCircle, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
		LPoint center = LCircle_GetCenter(selectCircle);
		LCoord r = LCircle_GetRadius(selectCircle);

		LPoint newPoint = RotatePoint(center, rotateCenter, rad);

		return LCircle_New(Cell_Now, outputLayer, newPoint, r);
	}

	LObject RotatePolygon(LLayer outputLayer, LObject selectPolygon, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
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
		return LPolygon_New(Cell_Now, outputLayer, points, cnt);
	}

	LObject RotateTorus(LLayer outputLayer, LObject selectTorus, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
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
			return LTorus_CreateNew(Cell_Now, outputLayer, &_tParams);
		}
		return NULL;
	}

	LObject RotatePie(LLayer outputLayer, LObject selectPie, LPoint rotateCenter, double rad)
	{
		LCell Cell_Now = LCell_GetVisible();
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
			return LPie_CreateNew(Cell_Now, outputLayer, &_pParams);
		}
		return NULL;
	}

	LObject RotateObject(LLayer outputLayer, LObject selectedObject, LPoint rotateCenter, double rad, int keep)
	{

		LShapeType selectedShapeType = LObject_GetShape(selectedObject);

		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			return RotateRectangle(outputLayer, selectedObject, rotateCenter, rad);
		}
		case 1: // LCircle
		{
			return RotateCircle(outputLayer, selectedObject, rotateCenter, rad);
		}
		case 3: // LPolygon
		{
			return RotatePolygon(outputLayer, selectedObject, rotateCenter, rad);
		}
		case 4: // LTorus
		{
			return RotateTorus(outputLayer, selectedObject, rotateCenter, rad);
		}
		case 5: // LPie
		{
			return RotatePie(outputLayer, selectedObject, rotateCenter, rad);
		}
		default:
			return NULL;
		}

		LCell Cell_Now = LCell_GetVisible();
		if (keep != 1)
			DeleteMacroObject(Cell_Now, selectedObject);
	}

	/* Use the selected circle center as the origin of the polar array. */
	void CreateRingArrayByAngle()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

		LSelection selection = LSelection_GetList();
		int selectedObjectNumber = CountSelectedObjects(selection);
		if (selectedObjectNumber != 1)
		{
			LDialog_AlertBox("Just Support 1 Object");
			return;
		}
		LObject object1 = LSelection_GetObject(selection);
		if (LObject_GetShape(object1) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		LLayer outputLayer = ResolveObjectOutputLayer(cell, &output, object1);
		if (outputLayer == NULL)
		{
			ReportSkippedOutputLayers(&output);
			return;
		}

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
		double rad = deg * M_PI / 180;

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
			CopyObject(outputLayer, object1, xOffset, yOffset);
		}
		LDisplay_Refresh();
	}

	void CreateRingArrayBySpacing()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

		LSelection selection = LSelection_GetList();
		int selectedObjectNumber = CountSelectedObjects(selection);
		if (selectedObjectNumber != 1)
		{
			LDialog_AlertBox("Just Support 1 Object");
			return;
		}
		LObject object1 = LSelection_GetObject(selection);
		if (LObject_GetShape(object1) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		LLayer outputLayer = ResolveObjectOutputLayer(cell, &output, object1);
		if (outputLayer == NULL)
		{
			ReportSkippedOutputLayers(&output);
			return;
		}

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
		double rad = deg * M_PI / 180;

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
			CopyObject(outputLayer, object1, xOffset, yOffset);
		}
		LDisplay_Refresh();
	}

	/* Use the smaller selected object as the repeated circle and the larger object as the boundary. */
	void FillShapeWithConcentricRings()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

		LSelection selection = LSelection_GetList();
		int selectedObjectNumber = CountSelectedObjects(selection);
		if (selectedObjectNumber != 2)
		{
			LDialog_AlertBox("Just Support 2 Object");
			return;
		}

		LObject object1 = LSelection_GetObject(selection);
		selection = LSelection_GetNext(selection);
		LObject object2 = LSelection_GetObject(selection);

		double area1 = LObject_Area(object1);
		double area2 = LObject_Area(object2);

		LObject smallerObject = area1 > area2 ? object2 : object1;
		LObject biggerObject = area1 > area2 ? object1 : object2;

		if (LObject_GetShape(smallerObject) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		LLayer outputLayer = ResolveObjectOutputLayer(cell, &output, smallerObject);
		if (outputLayer == NULL)
		{
			ReportSkippedOutputLayers(&output);
			return;
		}

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

		double rad = deg * M_PI / 180;
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);

		long box[4];
		box[0] = WORLD_MAX;	 // x0
		box[1] = WORLD_MAX;	 // y0
		box[2] = -WORLD_MAX; // x1
		box[3] = -WORLD_MAX; // y1
		AccumulateObjectBounds(biggerObject, box);
		LPoint center = LCircle_GetCenter(smallerObject);
		LCoord r = LCircle_GetRadius(smallerObject);
		if (2 * r + distance <= 0)
		{
			LDialog_AlertBox("Value of 'distance' error");
			return;
		}
		double distance1 = DistanceBetweenPoints(center, LPoint_Set(box[0], box[1])); // x0 y0
		double distance2 = DistanceBetweenPoints(center, LPoint_Set(box[0], box[3])); // x0 y1
		double distance3 = DistanceBetweenPoints(center, LPoint_Set(box[2], box[1])); // x1 y0
		double distance4 = DistanceBetweenPoints(center, LPoint_Set(box[2], box[3])); // x1 y1

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
					LCircle_New(Cell_Now, outputLayer, centerOffset, r);
			}
		}

		LDisplay_Refresh();
	}

	/* Generate staggered rows around the seed circle and keep centers inside the selected boundary. */
	void FillShapeWithHexagonalArray()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

		LSelection selection = LSelection_GetList();
		int selectedObjectNumber = CountSelectedObjects(selection);
		if (selectedObjectNumber != 2)
		{
			LDialog_AlertBox("Just Support 2 Object");
			return;
		}

		LObject object1 = LSelection_GetObject(selection);
		selection = LSelection_GetNext(selection);
		LObject object2 = LSelection_GetObject(selection);

		double area1 = LObject_Area(object1);
		double area2 = LObject_Area(object2);

		LObject smallerObject = area1 > area2 ? object2 : object1;
		LObject biggerObject = area1 > area2 ? object1 : object2;

		if (LObject_GetShape(smallerObject) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		LLayer outputLayer = ResolveObjectOutputLayer(cell, &output, smallerObject);
		if (outputLayer == NULL)
		{
			ReportSkippedOutputLayers(&output);
			return;
		}

		LDialogItem Dialog_Items[1] = {{"Array Edge Distance(um)", "1"}};
		long distance;
		double deg = 60;
		if (LDialog_MultiLineInputBox("Hexagon Properties", Dialog_Items, 1))
		{
			distance = (long)(atof(Dialog_Items[0].value) * 1000); // get the distance
		}
		else
		{
			return;
		}
		double rad = deg * M_PI / 180;
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);

		long box[4];
		box[0] = WORLD_MAX;	 // x0
		box[1] = WORLD_MAX;	 // y0
		box[2] = -WORLD_MAX; // x1
		box[3] = -WORLD_MAX; // y1
		AccumulateObjectBounds(biggerObject, box);
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
					LCircle_New(Cell_Now, outputLayer, centerOffset, r);
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
					LCircle_New(Cell_Now, outputLayer, centerOffset, r);
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
					LCircle_New(Cell_Now, outputLayer, centerOffset, r);
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
					LCircle_New(Cell_Now, outputLayer, centerOffset, r);
				}
				m++;
			}
			n++;
			yOffset = (2 * r + distance) * (sin(rad) - 1) * n;
		}
		LDisplay_Refresh();
	}

	int CountSelectedObjects(LSelection selection)
	{
		int counter = 0;
		while (selection != NULL)
		{
			selection = LSelection_GetNext(selection);
			counter++;
		}
		return counter;
	}

	void CreateLinearArrayByEdgeSpacing()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

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

		if ((distance >= 0 || arrayNumber > 1) || (deg >= 0 && deg <= 360))
		{
			CopySelectionByEdgeSpacing(cell, &output, arrayNumber, distance, deg);
		}

		LDisplay_Refresh();
		ReportSkippedOutputLayers(&output);
	}

	void CopySelectionByEdgeSpacing(LCell cell, OutputLayerChoice *output, int arrayNumber, long distance, double deg)
	{

		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			selection = LSelection_GetNext(selection);
			LLayer outputLayer = ResolveObjectOutputLayer(cell, output, selectedObject);
			if (outputLayer == NULL)
				continue;
			double rad = deg * M_PI / 180; //
			double offset[2];
			if (CalculateEdgeSpacingOffset(selectedObject, distance, rad, offset) == 1)
			{
				double xOffset = offset[0]; //
				double yOffset = offset[1]; //
				int i;
				for (i = 1; i < arrayNumber; i++)
				{
					CopyObject(outputLayer, selectedObject, i * xOffset, i * yOffset);
				}
				counter++;
			}
		}
	}

	/* Convert an edge-to-edge gap to a translation along the requested direction. */
	int CalculateEdgeSpacingOffset(LObject selectedObject, long distance, double rad, double *offset)
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

	void CreateLinearArrayByOffset()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

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

		if ((distance >= 0 || arrayNumber > 1) || (deg >= 0 && deg <= 360))
		{
			double rad = deg * M_PI / 180;
			double xOffset = distance * cos(rad);
			double yOffset = distance * sin(rad);
			CopySelectionByOffset(cell, &output, arrayNumber, xOffset, yOffset);
		}

		LDisplay_Refresh();
		ReportSkippedOutputLayers(&output);
	}

	void CopySelectionByOffset(LCell cell, OutputLayerChoice *output, int arrayNumber, double xOffset, double yOffset)
	{
		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			selection = LSelection_GetNext(selection);
			LLayer outputLayer = ResolveObjectOutputLayer(cell, output, selectedObject);
			if (outputLayer == NULL)
				continue;
			int i;
			for (i = 1; i < arrayNumber; i++)
			{
				CopyObject(outputLayer, selectedObject, (long)(i * xOffset), (long)(i * yOffset));
			}
			counter++;
		}
	}

	LObject CopyRectangle(LLayer outputLayer, LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LRect rect = LBox_GetRect(originObject);
		long rect_x0 = rect.x0 + xOffset;
		long rect_y0 = rect.y0 + yOffset;
		long rect_x1 = rect.x1 + xOffset;
		long rect_y1 = rect.y1 + yOffset;
		return LBox_New(Cell_Now, outputLayer, rect_x0, rect_y0, rect_x1, rect_y1);
	}

	LObject CopyCircle(LLayer outputLayer, LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LPoint center = LCircle_GetCenter(originObject);
		LCoord r = LCircle_GetRadius(originObject);

		long center_x = center.x + xOffset;
		long center_y = center.y + yOffset;
		LPoint centerOffset = LPoint_Set(center_x, center_y);
		return LCircle_New(Cell_Now, outputLayer, centerOffset, r);
	}

	LObject CopyPolygon(LLayer outputLayer, LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
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
		return LPolygon_New(Cell_Now, outputLayer, points, cnt);
	}

	LObject CopyTorus(LLayer outputLayer, LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
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
			return LTorus_CreateNew(Cell_Now, outputLayer, &_tParams);
		}
		return NULL;
	}

	LObject CopyPie(LLayer outputLayer, LObject originObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
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
			return LPie_CreateNew(Cell_Now, outputLayer, &_pParams);
		}
		return NULL;
	}

	LObject CopyObject(LLayer outputLayer, LObject selectedObject, long xOffset, long yOffset)
	{

		LShapeType selectedShapeType = LObject_GetShape(selectedObject);

		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			return CopyRectangle(outputLayer, selectedObject, xOffset, yOffset);
		}
		case 1: // LCircle
		{
			return CopyCircle(outputLayer, selectedObject, xOffset, yOffset);
		}
		case 3: // LPolygon
		{
			return CopyPolygon(outputLayer, selectedObject, xOffset, yOffset);
		}
		case 4: // LTorus
		{
			return CopyTorus(outputLayer, selectedObject, xOffset, yOffset);
		}
		case 5: // LPie
		{
			return CopyPie(outputLayer, selectedObject, xOffset, yOffset);
		}
		default:
			return NULL;
		}
	}

	void AccumulateObjectBounds(LObject selectedObject, long *box)
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

	double DistanceBetweenPoints(LPoint point1, LPoint point2)
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

	LPoint ReflectPointAcrossLine(LPoint p, LPoint linePoint, double rad)
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

	void CopySelectionByRadialOffset()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

		LDialogItem Dialog_Items[3] = {{"Offset (um) (Inf)", "0"},
									   {"Center Coord X (um)", "0"},
									   {"Center Coord Y (um)", "0"}};
		double offset;
		double center_x, center_y;
		int keep;
		if (LDialog_MultiLineInputBox("Scale Selected Object", Dialog_Items, 3))
		{
			offset = (double)(atof(Dialog_Items[0].value)); // get the Scale Factor
			center_x = (double)(atof(Dialog_Items[1].value));
			center_y = (double)(atof(Dialog_Items[2].value));
		}
		else
		{
			return;
		}
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			selection = LSelection_GetNext(selection);
			LLayer outputLayer = ResolveObjectOutputLayer(cell, &output, selectedObject);
			if (outputLayer == NULL)
				continue;
			LPoint polygonCenter = LPoint_Set((long)(center_x * 1000), (long)(center_y * 1000));
			long box[4];
			box[0] = WORLD_MAX;	 // x0
			box[1] = WORLD_MAX;	 // y0
			box[2] = -WORLD_MAX; // x1
			box[3] = -WORLD_MAX; // y1
			AccumulateObjectBounds(selectedObject, box);
			// calculate object center (box)
			long objectCenterX = (box[2] + box[0]) / 2;
			long objectCenterY = (box[3] + box[1]) / 2;
			long vectoryX = objectCenterX - (long)(center_x * 1000);
			long vectoryY = objectCenterY - (long)(center_y * 1000);
			double angle = atan2(vectoryY, vectoryX);
			long xOffset = cos(angle) * offset * 1000;
			long yOffset = sin(angle) * offset * 1000;
			CopyObject(outputLayer, selectedObject, xOffset, yOffset);

		}
		LDisplay_Refresh();
		ReportSkippedOutputLayers(&output);
	}

	/* Preserve each object bearing while placing its center at the requested distance from the origin. */
	void CopySelectionToRadius()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

		LDialogItem Dialog_Items[3] = {{"Radius (um) (Inf)", "0"},
									   {"Center Coord X (um)", "0"},
									   {"Center Coord Y (um)", "0"}};
		double radius;
		double center_x, center_y;
		int keep;
		if (LDialog_MultiLineInputBox("Scale Selected Object", Dialog_Items, 3))
		{
			radius = (double)(atof(Dialog_Items[0].value)); // get the Scale Factor
			center_x = (double)(atof(Dialog_Items[1].value));
			center_y = (double)(atof(Dialog_Items[2].value));
		}
		else
		{
			return;
		}
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			selection = LSelection_GetNext(selection);
			LLayer outputLayer = ResolveObjectOutputLayer(cell, &output, selectedObject);
			if (outputLayer == NULL)
				continue;
			LPoint polygonCenter = LPoint_Set((long)(center_x * 1000), (long)(center_y * 1000));
			long box[4];
			box[0] = WORLD_MAX;	 // x0
			box[1] = WORLD_MAX;	 // y0
			box[2] = -WORLD_MAX; // x1
			box[3] = -WORLD_MAX; // y1
			AccumulateObjectBounds(selectedObject, box);
			// calculate object center (box)
			long objectCenterX = (box[2] + box[0]) / 2;
			long objectCenterY = (box[3] + box[1]) / 2;
			long vectoryX = objectCenterX - (long)(center_x * 1000);
			long vectoryY = objectCenterY - (long)(center_y * 1000);
			double distanceToCenter = sqrt(1.0 * vectoryX * vectoryX + 1.0 * vectoryY * vectoryY);
			double angle = atan2(vectoryY, vectoryX);
			long xOffset = cos(angle) * (radius * 1000 - distanceToCenter);
			long yOffset = sin(angle) * (radius * 1000 - distanceToCenter);
			CopyObject(outputLayer, selectedObject, xOffset, yOffset);

		}
		LDisplay_Refresh();
		ReportSkippedOutputLayers(&output);
	}

	DEFINE_UNDOABLE_MACRO(UndoableCreateRingArrayBySpacing, CreateRingArrayBySpacing, "Ring Array by Spacing")
	DEFINE_UNDOABLE_MACRO(UndoableCreateRingArrayByAngle, CreateRingArrayByAngle, "Ring Array by Angle")
	DEFINE_UNDOABLE_MACRO(UndoableCreateLinearArrayByOffset, CreateLinearArrayByOffset, "Linear Array by Offset")
	DEFINE_UNDOABLE_MACRO(UndoableCreateLinearArrayByEdgeSpacing, CreateLinearArrayByEdgeSpacing, "Linear Array by Edge Spacing")
	DEFINE_UNDOABLE_MACRO(UndoableFillShapeWithHexagonalArray, FillShapeWithHexagonalArray, "Hexagonal Fill Array")
	DEFINE_UNDOABLE_MACRO(UndoableFillShapeWithConcentricRings, FillShapeWithConcentricRings, "Concentric Fill Array")
	DEFINE_UNDOABLE_MACRO(UndoableRotateSelectionAboutPoint, RotateSelectionAboutPoint, "Rotate Selection")
	DEFINE_UNDOABLE_MACRO(UndoableMirrorSelectionAcrossLine, MirrorSelectionAcrossLine, "Mirror Selection")
	DEFINE_UNDOABLE_MACRO(UndoableCopySelectionByRadialOffset, CopySelectionByRadialOffset, "Copy by Radial Offset")
	DEFINE_UNDOABLE_MACRO(UndoableCopySelectionToRadius, CopySelectionToRadius, "Copy to Radius")

	void RegisterArrayMacros(void)
	{
		// LMacro_BindToMenu( const char* menu, const char* macro_desc, const char* function_name );
		LMacro_Register("ArrayByRingSetDistance_func", "UndoableCreateRingArrayBySpacing");
		LMacro_Register("ArrayByRingSetRad_func", "UndoableCreateRingArrayByAngle");
		LMacro_Register("ArrayByOffsetOneDimension_func", "UndoableCreateLinearArrayByOffset");
		LMacro_Register("ArrayByEdgeDistanceOneDimension_func", "UndoableCreateLinearArrayByEdgeSpacing");
		LMacro_Register("ArrayInObjectByDistanceHexagon_func", "UndoableFillShapeWithHexagonalArray");
		LMacro_Register("ArrayInObjectByRing_func", "UndoableFillShapeWithConcentricRings");
		LMacro_Register("RotateObjectsByPoint_func", "UndoableRotateSelectionAboutPoint");
		LMacro_Register("MirrorObjectsByPointAndRad_func", "UndoableMirrorSelectionAcrossLine");
		LMacro_Register("Diffusion_Selected_Object_ByOffset_func", "UndoableCopySelectionByRadialOffset");
		LMacro_Register("Diffusion_Selected_Object_ByRadius_func", "UndoableCopySelectionToRadius");
	}
} /* End of ArrayMacros. */

RegisterArrayMacros();
