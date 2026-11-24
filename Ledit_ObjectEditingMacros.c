/*
 * Scale shapes, edit circle radii, remove matching objects, and place circles along polygon routes.
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
module ObjectEditingMacros
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "ldata.h" /* Main UPI header. */
#include "Ledit_MacroUndo.h"
#include "Ledit_OutputLayer.h"
#include "Ledit_Outline.h"
#include "Ledit_PolygonCircleRoute.h"

	void PlaceCirclesAlongPolygon(void);
	void AccumulateObjectBounds(LObject selectedObject, long *xLeft, long *xRight, long *yBottom, long *yTop);
	void SetCircleRadius(LObject circle, long target_radius);
	void ScaleObjectAroundPoint(LLayer outputLayer, LObject selectedObject, LPoint polygonCenter, double scaleFactor);
	void ScaleSelectionAroundPoint();
	void ScaleSelectionAroundObjectCenters();
	void OffsetSelectedCircleRadii();
	bool ObjectsHaveSameGeometry(LObject object1, LObject object2);
	void DeleteDuplicateObjects();
	void SetSelectedCircleRadii();
	void InspectObjectDistanceFromOrigin();
	void RegisterObjectEditingMacros(void);

	/* Place circle centers on the boundary or on a one-radius parallel route. */
	void PlaceCirclesAlongPolygon(void)
	{
		LCell cell = LCell_GetVisible();
		LSelection selection = LSelection_GetList();
		if (cell == NULL || selection == NULL)
		{
			LDialog_AlertBox("Select one or more polygons.");
			return;
		}

		const char *modes[] = {"Touch from inside", "Center on the boundary", "Touch from outside"};
		int mode = LDialog_PickList("Circle placement", modes, 3, 1);
		if (mode < 0 || mode >= 3)
			return;
		CircleRouteSide side = mode == 0 ? CircleRouteInside :
		                       mode == 2 ? CircleRouteOutside : CircleRouteOnBoundary;

		LDialogItem fields[] = {{"Circle radius (um)", "1.5"}, {"Path spacing (um)", "7.8"}};
		if (!LDialog_MultiLineInputBox("Place circles along a polygon", fields, 2))
			return;

		LFile file = LCell_GetFile(cell);
		LCoord radius, spacing;
		if (!ReadLayoutDistance(file, fields[0].value, &radius) || radius == 0 ||
		    !ReadLayoutDistance(file, fields[1].value, &spacing) || spacing == 0)
		{
			LDialog_AlertBox("Enter a positive circle radius and path spacing.");
			return;
		}
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

		int created = 0, skipped = 0, invalidRoutes = 0;
		while (selection != NULL)
		{
			LObject polygon = LSelection_GetObject(selection);
			selection = LSelection_GetNext(selection);
			if (polygon == NULL || LObject_GetShape(polygon) != LPolygon)
			{
				invalidRoutes++;
				continue;
			}

			LLayer layer = ResolveObjectOutputLayer(cell, &output, polygon);
			if (layer == NULL)
				continue;

			LayoutOutline edge = {0}, route = {0};
			int readable = ReadLayoutOutline(polygon, &edge, 0, 0.25, 0);
			CloseLayoutOutline(&edge);
			if (readable && BuildCircleCenterRoute(&edge, &route, side, radius, 0.25))
			{
				int count = PlaceCirclesOnCenterRoute(cell, file, layer, polygon, &route,
				                                     radius, spacing, side, &skipped);
				created += count;
				invalidRoutes += count == 0;
			}
			else
				invalidRoutes++;
			FreeLayoutOutline(&route);
			FreeLayoutOutline(&edge);
		}
		LDisplay_Refresh();
		ReportSkippedOutputLayers(&output);

		if (skipped > 0 || invalidRoutes > 0)
		{
			char message[190];
			snprintf(message, sizeof(message),
			         "Created %d circles. Skipped %d positions and %d objects with no usable route.",
			         created, skipped, invalidRoutes);
			LDialog_AlertBox(message);
		}
	}

	void AccumulateObjectBounds(LObject selectedObject, long *xLeft, long *xRight, long *yBottom, long *yTop)
	{
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		// Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			LRect rect = LBox_GetRect(selectedObject);
			// get rect xmin
			if (rect.x0 < rect.x1 && rect.x0 < *xLeft)
				*xLeft = rect.x0;
			if (rect.x1 < rect.x0 && rect.x1 < *xLeft)
				*xLeft = rect.x1;
			// get rect xmax
			if (rect.x0 < rect.x1 && rect.x1 > *xRight)
				*xRight = rect.x1;
			if (rect.x1 < rect.x0 && rect.x0 > *xRight)
				*xRight = rect.x0;
			// get rect ymin
			if (rect.y0 < rect.y1 && rect.y0 < *yBottom)
				*yBottom = rect.y0;
			if (rect.y1 < rect.y0 && rect.y1 < *yBottom)
				*yBottom = rect.y1;
			// get rect ymax
			if (rect.y0 < rect.y1 && rect.y1 > *yTop)
				*yTop = rect.y1;
			if (rect.y1 < rect.y0 && rect.y0 > *yTop)
				*yTop = rect.y0;
			break;
		}
		case 1: // LCircle
		{
			LPoint center = LCircle_GetCenter(selectedObject);
			LCoord r = LCircle_GetRadius(selectedObject);
			// get Circle xmin
			if (center.x - r < *xLeft)
				*xLeft = center.x - r;
			// get Circle xmax
			if (center.x + r > *xRight)
				*xRight = center.x + r;
			// get Circle ymin
			if (center.y - r < *yBottom)
				*yBottom = center.y - r;
			// get Circle ymax
			if (center.y + r > *yTop)
				*yTop = center.y + r;
			break;
		}
		case 2: // LWire
		case 3: // LPolygon
		{
			LVertex vertex = LObject_GetVertexList(selectedObject);
			while (vertex != NULL)
			{
				LPoint point = LVertex_GetPoint(vertex);
				// get Polygon xmin
				if (point.x < *xLeft)
					*xLeft = point.x;
				// get Polygon xmax
				if (point.x > *xRight)
					*xRight = point.x;
				// get Polygon ymin
				if (point.y < *yBottom)
					*yBottom = point.y;
				// get Polygon xmax
				if (point.y > *yTop)
					*yTop = point.y;
				vertex = LVertex_GetNext(vertex);
			}
			break;
		}
		case 4: // LTorus
		{
			LTorusParams pParams;
			LStatus lStatus = LTorus_GetParams(selectedObject, &pParams);
			if (lStatus == 0)
			{
				// get Torus xmin
				if (pParams.ptCenter.x - pParams.nOuterRadius < *xLeft)
					*xLeft = pParams.ptCenter.x - pParams.nOuterRadius;
				// get Torus xmax
				if (pParams.ptCenter.x + pParams.nOuterRadius > *xRight)
					*xRight = pParams.ptCenter.x + pParams.nOuterRadius;
				// get Torus ymin
				if (pParams.ptCenter.y - pParams.nOuterRadius < *yBottom)
					*yBottom = pParams.ptCenter.y - pParams.nOuterRadius;
				// get Torus ymax
				if (pParams.ptCenter.y + pParams.nOuterRadius > *yTop)
					*yTop = pParams.ptCenter.y + pParams.nOuterRadius;
			}
			break;
		}
		case 5: // LPie
		{
			LPieParams pParams;
			LStatus lStatus = LPie_GetParams(selectedObject, &pParams);
			if (lStatus == 0)
			{
				// get Pie xmin
				if (pParams.ptCenter.x - pParams.nRadius < *xLeft)
					*xLeft = pParams.ptCenter.x - pParams.nRadius;
				// get Pie xmax
				if (pParams.ptCenter.x + pParams.nRadius > *xRight)
					*xRight = pParams.ptCenter.x + pParams.nRadius;
				// get Pie ymin
				if (pParams.ptCenter.y - pParams.nRadius < *yBottom)
					*yBottom = pParams.ptCenter.y - pParams.nRadius;
				// get Pie ymax
				if (pParams.ptCenter.y + pParams.nRadius > *yTop)
					*yTop = pParams.ptCenter.y + pParams.nRadius;
			}
			break;
		}
		default:
			break;
		}
	}

	void SetCircleRadius(LObject circle, long target_radius)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, circle);

		LPoint center = LCircle_GetCenter(circle);
		if (target_radius != LCircle_GetRadius(circle) && RecordMacroObjectEdit(Cell_Now, circle))
			LCircle_Set(Cell_Now, circle, center, target_radius);
	}

	/* Apply the scale to coordinates relative to the supplied pivot and write the new shape to the output layer. */
	void ScaleObjectAroundPoint(LLayer outputLayer, LObject selectedObject, LPoint polygonCenter, double scaleFactor)
	{
		LCell Cell_Now = LCell_GetVisible();

		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		// Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
		case LBox:
		{
			LRect box = LBox_GetRect(selectedObject);
			LBox_New(Cell_Now, outputLayer,
					 polygonCenter.x + (box.x0 - polygonCenter.x) * scaleFactor,
					 polygonCenter.y + (box.y0 - polygonCenter.y) * scaleFactor,
					 polygonCenter.x + (box.x1 - polygonCenter.x) * scaleFactor,
					 polygonCenter.y + (box.y1 - polygonCenter.y) * scaleFactor);
			break;
		}
		case 1: // LCircle
		{
			LPoint center = LCircle_GetCenter(selectedObject);
			LCoord r = LCircle_GetRadius(selectedObject);
			long point_x = (long)(polygonCenter.x + (center.x - polygonCenter.x) * scaleFactor);
			long point_y = (long)(polygonCenter.y + (center.y - polygonCenter.y) * scaleFactor);
			LPoint newPoint = LPoint_Set(point_x, point_y);
			LCoord r_Scaled = (long)(r * scaleFactor);
			LCircle_New(Cell_Now, outputLayer, newPoint, r_Scaled);
			break;
		}
		case 3: // LPolygon
		{
			LVertex vertex = LObject_GetVertexList(selectedObject);
			long cnt = LVertex_GetCount(selectedObject);
			LPoint *points;
			points = (LPoint *)malloc((cnt) * sizeof(LPoint));
			if (points == NULL)
				break;
			int j = 0;
			while (vertex != NULL)
			{
				LPoint point = LVertex_GetPoint(vertex);
				// point.x
				// point.y
				long point_x = (long)(polygonCenter.x + (point.x - polygonCenter.x) * scaleFactor);
				long point_y = (long)(polygonCenter.y + (point.y - polygonCenter.y) * scaleFactor);
				points[j] = LPoint_Set(point_x, point_y);
				j++;
				vertex = LVertex_GetNext(vertex);
			}
			LPolygon_New(Cell_Now, outputLayer, points, j);
			free(points);
			break;
		}
		case 4: // LTorus
		{
			LTorusParams tParams;
			LStatus lStatus = LTorus_GetParams(selectedObject, &tParams);
			if (lStatus == 0)
			{
				/* Define parameters for a torus */
				LTorusParams _tParams;
				long point_x = (long)(polygonCenter.x + (tParams.ptCenter.x - polygonCenter.x) * scaleFactor);
				long point_y = (long)(polygonCenter.y + (tParams.ptCenter.y - polygonCenter.y) * scaleFactor);
				LPoint newPoint = LPoint_Set(point_x, point_y);

				_tParams.ptCenter = newPoint;
				_tParams.nInnerRadius = tParams.nInnerRadius * scaleFactor;
				_tParams.nOuterRadius = tParams.nOuterRadius * scaleFactor;
				_tParams.dStartAngle = tParams.dStartAngle;
				_tParams.dStopAngle = tParams.dStopAngle;

				LTorus_CreateNew(Cell_Now, outputLayer, &_tParams);
			}
			break;
		}
		case 5: // LPie
		{
			LPieParams pParams;
			LStatus lStatus = LPie_GetParams(selectedObject, &pParams);
			if (lStatus == 0)
			{
				LPieParams _pParams;
				long point_x = (long)(polygonCenter.x + (pParams.ptCenter.x - polygonCenter.x) * scaleFactor);
				long point_y = (long)(polygonCenter.y + (pParams.ptCenter.y - polygonCenter.y) * scaleFactor);
				LPoint newPoint = LPoint_Set(point_x, point_y);

				_pParams.ptCenter = newPoint;
				_pParams.nRadius = pParams.nRadius * scaleFactor;
				_pParams.dStartAngle = pParams.dStartAngle;
				_pParams.dStopAngle = pParams.dStopAngle;

				LPie_CreateNew(Cell_Now, outputLayer, &_pParams);
			}
			break;
		}
		default:
			break;
		}
	}

	void ScaleSelectionAroundPoint()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

		LDialogItem Dialog_Items[3] = {{"Scale Factor (>0)", "1"},
									   {"Center Coord X (um)", "0"},
									   {"Center Coord Y (um)", "0"}};
		double scaleFactor;
		double center_x, center_y;
		int keep;
		if (LDialog_MultiLineInputBox("Scale Selected Object", Dialog_Items, 3))
		{
			scaleFactor = (double)(atof(Dialog_Items[0].value)); // get the Scale Factor
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
			ScaleObjectAroundPoint(outputLayer, selectedObject, polygonCenter, scaleFactor);
		}
		LDisplay_Refresh();
		ReportSkippedOutputLayers(&output);
	}

	void ScaleSelectionAroundObjectCenters()
	{
		LCell cell = LCell_GetVisible();
		OutputLayerChoice output;
		if (!ChooseOutputLayer(cell, 1, &output))
			return;

		LDialogItem Dialog_Items[1] = {{"Scale Factor (>0)", "1"}};
		double scaleFactor;
		int keep;
		if (LDialog_MultiLineInputBox("Scale Selected Object", Dialog_Items, 1))
		{
			scaleFactor = (double)(atof(Dialog_Items[0].value)); // get the Scale Factor
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
			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			AccumulateObjectBounds(selectedObject, &xLeft, &xRight, &yBottom, &yTop);

			LPoint polygonCenter = LPoint_Set((long)((xLeft + xRight) / 2), (long)((yBottom + yTop) / 2));
			ScaleObjectAroundPoint(outputLayer, selectedObject, polygonCenter, scaleFactor);
		}
		LDisplay_Refresh();
		ReportSkippedOutputLayers(&output);
	}

	void OffsetSelectedCircleRadii()
	{
		LCell Cell_Now = LCell_GetVisible();
		LDialogItem Dialog_Items[2] = {{"Radius change (um, signed)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long radiusChange;
		int keep;
		if (LDialog_MultiLineInputBox("Edit selected objects", Dialog_Items, 2))
		{
			radiusChange = (long)(atof(Dialog_Items[0].value) * 1000); // get the radius_offset
			keep = atoi(Dialog_Items[1].value);						   // get the keep
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
			switch (LObject_GetShape(selectedObject))
			{
				long tar_radius;
			case 1: // LCircle
				tar_radius = LCircle_GetRadius(selectedObject) + radiusChange;
				SetCircleRadius(selectedObject, tar_radius);
				break;
			default:
				break;
			}
			if (keep != 1)
			{
				DeleteMacroObject(Cell_Now, selectedObject);
			}
		}
		LDisplay_Refresh();
	}

	/* Compare like shapes by their defining coordinates and parameters. */
	bool ObjectsHaveSameGeometry(LObject object1, LObject object2)
	{
		if (LObject_GetShape(object1) != LObject_GetShape(object2))
		{
			return false;
		}
		switch (LObject_GetShape(object1))
		{
		case 0: // LBox
		{
			LRect rect1 = LBox_GetRect(object1);
			LRect rect2 = LBox_GetRect(object2);
			return rect1.x0 == rect2.x0 && rect1.x1 == rect2.x1 && rect1.y0 == rect2.y0 && rect1.y1 == rect2.y1;
		}
		case 1: // LCircle
		{
			LPoint center1 = LCircle_GetCenter(object1);
			LCoord r1 = LCircle_GetRadius(object1);
			LPoint center2 = LCircle_GetCenter(object2);
			LCoord r2 = LCircle_GetRadius(object2);
			return center1.x == center2.x && center1.y == center2.y && r1 == r2;
		}
		case 3: // LPolygon
		{
			LVertex vertex1 = LObject_GetVertexList(object1);
			long cnt1 = LVertex_GetCount(object1);
			LVertex vertex2 = LObject_GetVertexList(object2);
			long cnt2 = LVertex_GetCount(object2);
			if (cnt1 != cnt2)
				return false;

			while (vertex1 != NULL)
			{
				LPoint point1 = LVertex_GetPoint(vertex1);
				LPoint point2 = LVertex_GetPoint(vertex2);
				if (point1.x != point2.x || point1.y != point2.y)
					return false;
				vertex1 = LVertex_GetNext(vertex1);
				vertex2 = LVertex_GetNext(vertex2);
			}
			return true;
		}
		case 4: // LTorus
		{
			LTorusParams tParams1;
			LStatus lStatus1 = LTorus_GetParams(object1, &tParams1);

			LTorusParams tParams2;
			LStatus lStatus2 = LTorus_GetParams(object2, &tParams2);
			if (lStatus1 == 0 && lStatus2 == 0)
			{
				return tParams1.ptCenter.x == tParams2.ptCenter.x && tParams1.ptCenter.y == tParams2.ptCenter.y && tParams1.nInnerRadius == tParams2.nInnerRadius && tParams1.nOuterRadius == tParams2.nOuterRadius && tParams1.dStartAngle == tParams2.dStartAngle && tParams1.dStopAngle == tParams2.dStopAngle;
			}
			else
			{
				return false;
			}
		}
		case 5: // LPie
		{
			LPieParams pParams1;
			LStatus lStatus1 = LPie_GetParams(object1, &pParams1);
			LPieParams pParams2;
			LStatus lStatus2 = LPie_GetParams(object2, &pParams2);
			if (lStatus1 == 0 && lStatus2 == 0)
			{
				return pParams1.ptCenter.x == pParams2.ptCenter.x && pParams1.ptCenter.y == pParams2.ptCenter.y && pParams1.nRadius == pParams2.nRadius && pParams1.dStartAngle == pParams2.dStartAngle && pParams1.dStopAngle == pParams2.dStopAngle;
			}
			else
			{
				return false;
			}
		}
		default:
			return false;
		}
	}

	void DeleteDuplicateObjects()
	{
		LCell cell = LCell_GetVisible();
		LSelection selection;
		size_t count = 0, i, j;
		for (selection = LSelection_GetList(); selection != NULL; selection = LSelection_GetNext(selection))
			count++;
		if (count < 2)
			return;
		LObject *objects = malloc(count * sizeof(*objects));
		if (objects == NULL)
		{
			LDialog_AlertBox("Could not read the selection.");
			return;
		}
		i = 0;
		for (selection = LSelection_GetList(); selection != NULL; selection = LSelection_GetNext(selection))
			objects[i++] = LSelection_GetObject(selection);

		/* Delete from the end so all preceding objects remain available for comparison. */
		for (i = count; i > 0; i--)
		{
			LObject candidate = objects[i - 1];
			if (candidate == NULL)
				continue;
			for (j = 0; j < i - 1; j++)
			{
				if (objects[j] != NULL && objects[j] != candidate &&
				    ObjectsHaveSameGeometry(objects[j], candidate))
				{
					DeleteMacroObject(cell, candidate);
					break;
				}
			}
		}
		free(objects);
		LDisplay_Refresh();
	}

	void SetSelectedCircleRadii()
	{
		LCell Cell_Now = LCell_GetVisible();
		LDialogItem Dialog_Items[2] = {{"Target radius (um)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long tar_radius;
		int keep;
		if (LDialog_MultiLineInputBox("Edit selected objects", Dialog_Items, 2))
		{
			tar_radius = (long)(atof(Dialog_Items[0].value) * 1000); // get the tar_radius
			keep = atoi(Dialog_Items[1].value);						 // get the keep
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
			switch (LObject_GetShape(selectedObject))
			{
			case 1: // LCircle
				SetCircleRadius(selectedObject, tar_radius);
				break;
			default:
				break;
			}
			if (keep != 1)
			{
				DeleteMacroObject(Cell_Now, selectedObject);
			}
		}
		LDisplay_Refresh();
	}

	void DeleteSelectedObjectsByShape()
	{
		LCell Cell_Now = LCell_GetVisible();
		/*
		typedef enum {
		LBox,
		LCircle,
		LWire,
		LPolygon,
		LTorus,
		LPie,
		LOtherObject,
		LObjInstance,
		LObjPort,
		LObjRuler
		} LShapeType;
		*/
		/*Declare a buffer to hold all elements of the pick list*/
		const char *Pick_List[] = {
			"LBox",
			"LCircle",
			"LWire",
			"LPolygon",
			"LTorus",
			"LPie",
			"LOtherObject",
			"LObjInstance",
			"LObjPort",
			"LObjRuler"};
		/*Number of elements in the pick list*/
		int Pick_Count = 10;
		/*Index of the item picked by user*/
		int Picked;
		/*Display the pick list with Inverter as the default selection*/
		Picked = LDialog_PickList("Select Deleted Element", Pick_List, Pick_Count, 0);

		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LSelection nextSelection = LSelection_GetNext(selection);
			LObject selectedObject = LSelection_GetObject(selection);

			if (selectedObject != NULL && LObject_GetShape(selectedObject) == (LShapeType)Picked)
			{
				DeleteMacroObject(Cell_Now, selectedObject);
			}

			selection = nextSelection;
		}

		LDisplay_Refresh();
	}

	void InspectObjectDistanceFromOrigin()
	{
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LPoint zero = LPoint_Set(0, 0);
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			LCoord distance = LObject_DistanceToPoint(selectedObject, zero, File_Now);
			LCoord a = WORLD_MAX + distance;
			double c = 1.0 * distance;
			LDialog_AlertBox(LFormat("distance: %ld, %ld, %ld, %f", distance, WORLD_MAX, a, c));
			selection = LSelection_GetNext(selection);
		}
	}

	DEFINE_UNDOABLE_MACRO(UndoableSetSelectedCircleRadii, SetSelectedCircleRadii, "Set Circle Radii")
	DEFINE_UNDOABLE_MACRO(UndoableOffsetSelectedCircleRadii, OffsetSelectedCircleRadii, "Offset Circle Radii")
	DEFINE_UNDOABLE_MACRO(UndoableDeleteDuplicateObjects, DeleteDuplicateObjects, "Delete Duplicates")
	DEFINE_UNDOABLE_MACRO(UndoableDeleteSelectedObjectsByShape, DeleteSelectedObjectsByShape, "Delete Objects by Shape")
	DEFINE_UNDOABLE_MACRO(UndoablePlaceCirclesAlongPolygon, PlaceCirclesAlongPolygon, "Place Circles Along Polygon")
	DEFINE_UNDOABLE_MACRO(UndoableScaleSelectionAroundObjectCenters, ScaleSelectionAroundObjectCenters, "Scale About Object Centers")
	DEFINE_UNDOABLE_MACRO(UndoableScaleSelectionAroundPoint, ScaleSelectionAroundPoint, "Scale About Point")

	void UndoLastMacro(void)
	{
		UndoLastMacroInCell();
	}

	void RegisterObjectEditingMacros(void)
	{
		LMacro_Register("Modifiy_Selected_Object_UnionSize_func", "UndoableSetSelectedCircleRadii");
		LMacro_Register("Modifiy_Selected_Object_Offset", "UndoableOffsetSelectedCircleRadii");
		LMacro_Register("Modifiy_Delete_Duplicates_func", "UndoableDeleteDuplicateObjects");
		LMacro_Register("Modifiy_Delete_Objects_ByType_func", "UndoableDeleteSelectedObjectsByShape");
		LMacro_Register("Route_Circle_func", "UndoablePlaceCirclesAlongPolygon");
		LMacro_Register("Scale_SelectedObject_ByObjectCenter_func", "UndoableScaleSelectionAroundObjectCenters");
		LMacro_Register("Scale_SelectedObject_SetCenter_func", "UndoableScaleSelectionAroundPoint");
		LMacro_Register("TestFunc_func", "InspectObjectDistanceFromOrigin");
		LMacro_Register("Undo Last Macro", "UndoLastMacro");
	}
} /* End of ObjectEditingMacros. */

RegisterObjectEditingMacros();
