/*
 * Align selected layout objects to the bounds or center of the selection.
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
module AlignmentMacros
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ldata.h" /* Main UPI header. */
#include "Ledit_MacroUndo.h"

	void AlignSelectionLeft();
	void AlignSelectionRight();
	void AlignSelectionBottom();
	void AlignSelectionTop();
	void AlignSelectionCenters();
	void MoveSelectionToBoundsCenter(long *xLeftTarget, long *xRightTarget, long *yBottomTarget, long *yTopTarget);
	void MoveSelectionToLeftEdge(long *xLeftTarget);
	void MoveSelectionToRightEdge(long *xRightTarget);
	void MoveSelectionToBottomEdge(long *yBottomTarget);
	void MoveSelectionToTopEdge(long *yTopTarget);
	void TranslateObject(LObject selectedObject, long xOffset, long yOffset);
	void AccumulateSelectionBounds(long *xLeft, long *xRight, long *yBottom, long *yTop);
	void AccumulateObjectBounds(LObject selectedObject, long *xLeft, long *xRight, long *yBottom, long *yTop);
	void RegisterAlignmentMacros(void);

	/* Compute one reference boundary from the whole selection before moving any object. */
	void AlignSelectionLeft()
	{
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		AccumulateSelectionBounds(&xLeft, &xRight, &yBottom, &yTop);
		MoveSelectionToLeftEdge(&xLeft);
		LDisplay_Refresh();
	}

	void AlignSelectionRight()
	{
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		AccumulateSelectionBounds(&xLeft, &xRight, &yBottom, &yTop);
		MoveSelectionToRightEdge(&xRight);
		LDisplay_Refresh();
	}

	void AlignSelectionBottom()
	{
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		AccumulateSelectionBounds(&xLeft, &xRight, &yBottom, &yTop);
		MoveSelectionToBottomEdge(&yBottom);
		LDisplay_Refresh();
	}

	void AlignSelectionTop()
	{
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		AccumulateSelectionBounds(&xLeft, &xRight, &yBottom, &yTop);
		MoveSelectionToTopEdge(&yTop);
		LDisplay_Refresh();
	}

	void AlignSelectionCenters()
	{
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		AccumulateSelectionBounds(&xLeft, &xRight, &yBottom, &yTop);
		MoveSelectionToBoundsCenter(&xLeft, &xRight, &yBottom, &yTop);
		LDisplay_Refresh();
	}

	/* Each object moves from its own bounds center to the shared selection center. */
	void MoveSelectionToBoundsCenter(long *xLeftTarget, long *xRightTarget, long *yBottomTarget, long *yTopTarget)
	{
		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			AccumulateObjectBounds(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
			if (xLeft != WORLD_MAX)
			{
				long _xTarget = (*xLeftTarget + *xRightTarget) / 2;
				long _xL = xLeft;
				long _xR = xRight;
				long _x = (_xL + _xR) / 2;
				long xOffset = _xTarget - _x;

				long _yTarget = (*yBottomTarget + *yTopTarget) / 2;
				long _yB = yBottom;
				long _yT = yTop;
				long _y = (_yB + _yT) / 2;
				long yOffset = _yTarget - _y;
				TranslateObject(selectedObject, xOffset, yOffset);
			}
			selection = LSelection_GetNext(selection);
			counter++;
		}
	}

	void MoveSelectionToLeftEdge(long *xLeftTarget)
	{
		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			AccumulateObjectBounds(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
			if (xLeft != WORLD_MAX)
			{
				long _xLeftTarget = *xLeftTarget;
				long _xLeft = xLeft;
				long xOffset = _xLeftTarget - _xLeft;
				TranslateObject(selectedObject, xOffset, 0);
			}
			selection = LSelection_GetNext(selection);
			counter++;
		}
	}

	void MoveSelectionToRightEdge(long *xRightTarget)
	{
		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			AccumulateObjectBounds(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
			if (xRight != WORLD_MAX)
			{
				long _xRightTarget = *xRightTarget;
				long _xRight = xRight;
				long xOffset = _xRightTarget - _xRight;
				TranslateObject(selectedObject, xOffset, 0);
			}
			selection = LSelection_GetNext(selection);
			counter++;
		}
	}

	void MoveSelectionToBottomEdge(long *yBottomTarget)
	{
		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			AccumulateObjectBounds(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
			if (yBottom != WORLD_MAX)
			{
				long _yBottomTarget = *yBottomTarget;
				long _yBottom = yBottom;
				long yOffset = _yBottomTarget - _yBottom;
				TranslateObject(selectedObject, 0, yOffset);
			}
			selection = LSelection_GetNext(selection);
			counter++;
		}
	}

	void MoveSelectionToTopEdge(long *yTopTarget)
	{
		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			AccumulateObjectBounds(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
			if (yTop != WORLD_MAX)
			{
				long _yTopTarget = *yTopTarget;
				long _yTop = yTop;
				long yOffset = _yTopTarget - _yTop;
				TranslateObject(selectedObject, 0, yOffset);
			}
			selection = LSelection_GetNext(selection);
			counter++;
		}
	}

	/* Translate coordinates in place using the setter for each supported UPI shape. */
	void TranslateObject(LObject selectedObject, long xOffset, long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		if (selectedShapeType < LBox || selectedShapeType > LPie ||
		    (xOffset == 0 && yOffset == 0) || !RecordMacroObjectEdit(Cell_Now, selectedObject))
			return;

		switch (selectedShapeType)
		{
		case 0: // LBox
		{
			// get rect xmin
			LRect rect = LBox_GetRect(selectedObject);
			rect.x0 += xOffset;
			rect.y0 += yOffset;
			rect.x1 += xOffset;
			rect.y1 += yOffset;
			LBox_Set(Cell_Now, selectedObject, rect);
			break;
		}
		case 1: // LCircle
		{
			LPoint center = LCircle_GetCenter(selectedObject);
			LCoord r = LCircle_GetRadius(selectedObject);
			center.x += xOffset;
			center.y += yOffset;
			LCircle_Set(Cell_Now, selectedObject, center, r);
			break;
		}
		case 2: // LWire
		case 3: // LPolygon
		{
			LVertex vertex = LObject_GetVertexList(selectedObject);
			while (vertex != NULL)
			{
				LPoint point = LVertex_GetPoint(vertex);
				point.x += xOffset;
				point.y += yOffset;
				LVertex_SetPoint(vertex, point);
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
				pParams.ptCenter.x += xOffset;
				pParams.ptCenter.y += yOffset;
				LTorus_SetParams(Cell_Now, selectedObject, &pParams);
			}
			break;
		}
		case 5: // LPie
		{
			LPieParams pParams;
			LStatus lStatus = LPie_GetParams(selectedObject, &pParams);
			if (lStatus == 0)
			{
				pParams.ptCenter.x += xOffset;
				pParams.ptCenter.y += yOffset;
				LPie_SetParams(Cell_Now, selectedObject, &pParams);
			}
			break;
		}
		default:
			break;
		}
	}

	void AccumulateSelectionBounds(long *xLeft, long *xRight, long *yBottom, long *yTop)
	{
		int counter = 0;
		LSelection selection = LSelection_GetList();
		while (selection != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selection);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);
			AccumulateObjectBounds(selectedObject, xLeft, xRight, yBottom, yTop); // Function for selectedObject xcoord

			selection = LSelection_GetNext(selection);
			counter++;
		}
	}

	/* Expand the caller-provided coordinate range to include this object. */
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

	void AlignSelection(void)
	{
		const char *modes[] = {"Left edges", "Right edges", "Bottom edges", "Top edges", "Centers"};
		int mode = LDialog_PickList("Align selection", modes, 5, 0);
		switch (mode)
		{
		case 0:
			AlignSelectionLeft();
			break;
		case 1:
			AlignSelectionRight();
			break;
		case 2:
			AlignSelectionBottom();
			break;
		case 3:
			AlignSelectionTop();
			break;
		case 4:
			AlignSelectionCenters();
			break;
		}
	}

	DEFINE_UNDOABLE_MACRO(UndoableAlignSelection, AlignSelection, "Align Selection")

	void RegisterAlignmentMacros(void)
	{
		LMacro_Register("Align_Selection_func", "UndoableAlignSelection");
	}

} /* End of AlignmentMacros. */

RegisterAlignmentMacros();
