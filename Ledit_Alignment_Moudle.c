/*
 * Alignment_Moudle
 * Alignment_Moudle Marco for Ledit
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
module Alignment_Moudle
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "Ledit_Alignment.h"
#include "ldata.h" /* Main UPI header. */
#include "Ledit_Safety.h"

	void Alignment_Left();
	void Alignment_Right();
	void Alignment_Bottom();
	void Alignment_Top();
	void Alignment_Center();
	void MoveObjectListCenter(long *xLeftTarget, long *xRightTarget, long *yBottomTarget, long *yTopTarget);
	void MoveObjectListLeft(long *xLeftTarget);
	void MoveObjectListRight(long *xRightTarget);
	void MoveObjectListBottom(long *yBottomTarget);
	void MoveObjectListTop(long *yTopTarget);
	void MoveObject(LObject selectedObject, long xOffset, long yOffset);
	void GetObjectListCoordRange(long *xLeft, long *xRight, long *yBottom, long *yTop);
	void GetObjectCoord(LObject selectedObject, long *xLeft, long *xRight, long *yBottom, long *yTop);
	void Register_Alignment_func(void);

	void Alignment_Left()
	{
        if (!LM_Context()) return;
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft, &xRight, &yBottom, &yTop);
		MoveObjectListLeft(&xLeft);
		LDisplay_Refresh();
	}

	void Alignment_Right()
	{
        if (!LM_Context()) return;
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft, &xRight, &yBottom, &yTop);
		MoveObjectListRight(&xRight);
		LDisplay_Refresh();
	}

	void Alignment_Bottom()
	{
        if (!LM_Context()) return;
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft, &xRight, &yBottom, &yTop);
		MoveObjectListBottom(&yBottom);
		LDisplay_Refresh();
	}

	void Alignment_Top()
	{
        if (!LM_Context()) return;
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft, &xRight, &yBottom, &yTop);
		MoveObjectListTop(&yTop);
		LDisplay_Refresh();
	}

	void Alignment_Center()
	{
        if (!LM_Context()) return;
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft, &xRight, &yBottom, &yTop);
		MoveObjectListCenter(&xLeft, &xRight, &yBottom, &yTop);
		LDisplay_Refresh();
	}

	void MoveObjectListCenter(long *xLeftTarget, long *xRightTarget, long *yBottomTarget, long *yTopTarget)
	{
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
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
				MoveObject(selectedObject, xOffset, yOffset);
			}
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
	}

	void MoveObjectListLeft(long *xLeftTarget)
	{
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
			if (xLeft != WORLD_MAX)
			{
				long _xLeftTarget = *xLeftTarget;
				long _xLeft = xLeft;
				long xOffset = _xLeftTarget - _xLeft;
				MoveObject(selectedObject, xOffset, 0);
			}
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
	}

	void MoveObjectListRight(long *xRightTarget)
	{
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
			if (xRight != -WORLD_MAX)
			{
				long _xRightTarget = *xRightTarget;
				long _xRight = xRight;
				long xOffset = _xRightTarget - _xRight;
				MoveObject(selectedObject, xOffset, 0);
			}
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
	}

	void MoveObjectListBottom(long *yBottomTarget)
	{
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
			if (yBottom != WORLD_MAX)
			{
				long _yBottomTarget = *yBottomTarget;
				long _yBottom = yBottom;
				long yOffset = _yBottomTarget - _yBottom;
				MoveObject(selectedObject, 0, yOffset);
			}
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
	}

	void MoveObjectListTop(long *yTopTarget)
	{
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);

			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject, &xLeft, &xRight, &yBottom, &yTop); // Function for selectedObject
			if (yTop != -WORLD_MAX)
			{
				long _yTopTarget = *yTopTarget;
				long _yTop = yTop;
				long yOffset = _yTopTarget - _yTop;
				MoveObject(selectedObject, 0, yOffset);
			}
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
	}

	void MoveObject(LObject selectedObject, long xOffset, long yOffset)
	{
        LTransform_Ex99 transform;
        if (!LM_CanMove(selectedObject, xOffset, yOffset)) {
            LDialog_AlertBox("Alignment would exceed layout coordinate limits.");
            return;
        }
        transform = LTransform_Zero_Ex99();
        transform.translation = LPoint_Set(xOffset, yOffset);
        LObject_Transform_Ex99(selectedObject, transform);
    }

	void GetObjectListCoordRange(long *xLeft, long *xRight, long *yBottom, long *yTop)
	{
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);
			GetObjectCoord(selectedObject, xLeft, xRight, yBottom, yTop); // Function for selectedObject xcoord

			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
	}

	void GetObjectCoord(LObject selectedObject, long *xLeft, long *xRight, long *yBottom, long *yTop)
	{
        LRect box;
        if (!selectedObject) return;
        box = LObject_GetMbb(selectedObject);
        if (box.x0 < *xLeft) *xLeft = box.x0;
        if (box.x1 > *xRight) *xRight = box.x1;
        if (box.y0 < *yBottom) *yBottom = box.y0;
        if (box.y1 > *yTop) *yTop = box.y1;
    }

	void Register_Alignment_func(void)
	{
		LMacro_Register("Alignment_Left_func", "Alignment_Left");
		LMacro_Register("Alignment_Right_func", "Alignment_Right");
		LMacro_Register("Alignment_Bottom_func", "Alignment_Bottom");
		LMacro_Register("Alignment_Top_func", "Alignment_Top");
		LMacro_Register("Alignment_Center_func", "Alignment_Center");
	}
} /* end of module Alignment_Moudle*/

Register_Alignment_func();
