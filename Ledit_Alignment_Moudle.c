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
	#include "ldata.h"     /* Main UPI header. */

	void Alignment_Left()
	{
		long* xLeft = WORLD_MAX;
		long* xRight = -WORLD_MAX;
		long* yBottom = WORLD_MAX;
		long* yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft,&xRight,&yBottom,&yTop);
		//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
		MoveObjectListLeft(&xLeft);
		LDisplay_Refresh();
	}
	void Alignment_Right()
	{
		long* xLeft = WORLD_MAX;
		long* xRight = -WORLD_MAX;
		long* yBottom = WORLD_MAX;
		long* yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft,&xRight,&yBottom,&yTop);
		//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
		MoveObjectListRight(&xRight);
		LDisplay_Refresh();
	}
	void Alignment_Bottom()
	{
		long* xLeft = WORLD_MAX;
		long* xRight = -WORLD_MAX;
		long* yBottom = WORLD_MAX;
		long* yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft,&xRight,&yBottom,&yTop);
		//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
		MoveObjectListBottom(&yBottom);
		LDisplay_Refresh();
	}
	void Alignment_Top()
	{
		long* xLeft = WORLD_MAX;
		long* xRight = -WORLD_MAX;
		long* yBottom = WORLD_MAX;
		long* yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft,&xRight,&yBottom,&yTop);
		//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
		MoveObjectListTop(&yTop);
		LDisplay_Refresh();
	}
	void Alignment_Center()
	{
		long* xLeft = WORLD_MAX;
		long* xRight = -WORLD_MAX;
		long* yBottom = WORLD_MAX;
		long* yTop = -WORLD_MAX;
		GetObjectListCoordRange(&xLeft,&xRight,&yBottom,&yTop);
		//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
		MoveObjectListCenter(&xLeft,&xRight,&yBottom,&yTop);
		LDisplay_Refresh();
	}
	
	void MoveObjectListCenter(long* xLeftTarget,long* xRightTarget,long* yBottomTarget,long* yTopTarget) {
		//LDialog_AlertBox(LFormat("xLeftTarget:%d",*xLeftTarget));
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);//

			long* xLeft = WORLD_MAX;
			long* xRight = -WORLD_MAX;
			long* yBottom = WORLD_MAX;
			long* yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject,&xLeft,&xRight,&yBottom,&yTop);//Function for selectedObject
			//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
			if (xLeft != WORLD_MAX)
			{
				long _xTarget = (*xLeftTarget+*xRightTarget)/2;
				long _xL = xLeft;
				long _xR = xRight;
				long _x = (_xL+_xR)/2;
				long xOffset = _xTarget - _x;
				
				long _yTarget = (*yBottomTarget+*yTopTarget)/2;
				long _yB = yBottom;
				long _yT = yTop;
				long _y = (_yB+_yT)/2;
				long yOffset = _yTarget - _y;
				MoveObject(selectedObject,xOffset,yOffset);
			}
			//LDialog_AlertBox(LFormat("%d",selectedShapeType));
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		//LDialog_AlertBox(LFormat("Completed.\nTotal %d Object.\nxLeft:%d",counter,xLeft));
	}

	void MoveObjectListLeft(long* xLeftTarget) {
		//LDialog_AlertBox(LFormat("xLeftTarget:%d",*xLeftTarget));
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);//

			long* xLeft = WORLD_MAX;
			long* xRight = -WORLD_MAX;
			long* yBottom = WORLD_MAX;
			long* yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject,&xLeft,&xRight,&yBottom,&yTop);//Function for selectedObject
			//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
			if (xLeft != WORLD_MAX)
			{
				//LDialog_AlertBox(LFormat("xLeft:%d",xLeft));
				long _xLeftTarget = *xLeftTarget;
				long _xLeft = xLeft;
				long xOffset = _xLeftTarget - _xLeft;
				MoveObject(selectedObject,xOffset,0);
			}
			//LDialog_AlertBox(LFormat("%d",selectedShapeType));
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		//LDialog_AlertBox(LFormat("Completed.\nTotal %d Object.\nxLeft:%d",counter,xLeft));
	}

	void MoveObjectListRight(long* xRightTarget) {
		//LDialog_AlertBox(LFormat("xLeftTarget:%d",*xLeftTarget));
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);//

			long* xLeft = WORLD_MAX;
			long* xRight = -WORLD_MAX;
			long* yBottom = WORLD_MAX;
			long* yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject,&xLeft,&xRight,&yBottom,&yTop);//Function for selectedObject
			//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
			if (xRight != WORLD_MAX)
			{
				//LDialog_AlertBox(LFormat("xLeft:%d",xLeft));
				long _xRightTarget = *xRightTarget;
				long _xRight = xRight;
				long xOffset = _xRightTarget - _xRight;
				MoveObject(selectedObject,xOffset,0);
			}
			//LDialog_AlertBox(LFormat("%d",selectedShapeType));
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		//LDialog_AlertBox(LFormat("Completed.\nTotal %d Object.\nxLeft:%d",counter,xLeft));
	}
	
	void MoveObjectListBottom(long* yBottomTarget) {
		//LDialog_AlertBox(LFormat("xLeftTarget:%d",*xLeftTarget));
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);//

			long* xLeft = WORLD_MAX;
			long* xRight = -WORLD_MAX;
			long* yBottom = WORLD_MAX;
			long* yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject,&xLeft,&xRight,&yBottom,&yTop);//Function for selectedObject
			//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
			if (yBottom != WORLD_MAX)
			{
				//LDialog_AlertBox(LFormat("xLeft:%d",xLeft));
				long _yBottomTarget = *yBottomTarget;
				long _yBottom = yBottom;
				long yOffset = _yBottomTarget - _yBottom;
				MoveObject(selectedObject,0,yOffset);
			}
			//LDialog_AlertBox(LFormat("%d",selectedShapeType));
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		//LDialog_AlertBox(LFormat("Completed.\nTotal %d Object.\nxLeft:%d",counter,xLeft));
	}

	void MoveObjectListTop(long* yTopTarget) {
		//LDialog_AlertBox(LFormat("xLeftTarget:%d",*xLeftTarget));
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);//

			long* xLeft = WORLD_MAX;
			long* xRight = -WORLD_MAX;
			long* yBottom = WORLD_MAX;
			long* yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject,&xLeft,&xRight,&yBottom,&yTop);//Function for selectedObject
			//LDialog_AlertBox(LFormat("xLeft:%d, xRight:%d, yBottom:%d, yTop:%d",xLeft,xRight,yBottom,yTop));
			if (yTop != WORLD_MAX)
			{
				//LDialog_AlertBox(LFormat("xLeft:%d",xLeft));
				long _yTopTarget = *yTopTarget;
				long _yTop = yTop;
				long yOffset = _yTopTarget - _yTop;
				MoveObject(selectedObject,0,yOffset);
			}
			//LDialog_AlertBox(LFormat("%d",selectedShapeType));
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		//LDialog_AlertBox(LFormat("Completed.\nTotal %d Object.\nxLeft:%d",counter,xLeft));
	}
	
	void  MoveObject(LObject selectedObject,long xOffset,long yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		//Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
			case 0://LBox
			{
				//get rect xmin
				LRect rect = LBox_GetRect(selectedObject);
				rect.x0 += xOffset; rect.y0 += yOffset;
				rect.x1 += xOffset; rect.y1 += yOffset;
				LBox_Set(Cell_Now,selectedObject,rect);
				break;
			}
			case 1://LCircle
			{
				LPoint center = LCircle_GetCenter(selectedObject);
				LCoord r = LCircle_GetRadius(selectedObject);
				center.x += xOffset;
				center.y += yOffset;
				LCircle_Set(Cell_Now,selectedObject,center,r);
				break;
			}
			case 3://LPolygon
			{
				LVertex vertex = LObject_GetVertexList(selectedObject);
				while (vertex != NULL)
				{
					LPoint point = LVertex_GetPoint(vertex);
					point.x += xOffset;
					point.y += yOffset;
					LVertex_SetPoint(vertex,point);
					vertex = LVertex_GetNext(vertex);
				}
				break;
			}
			case 4://LTorus
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
			case 5://LPie
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
		//
	}

	void GetObjectListCoordRange(long* xLeft,long* xRight,long* yBottom,long* yTop)
	{
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);
			GetObjectCoord(selectedObject,xLeft,xRight,yBottom,yTop);//Function for selectedObject xcoord

			//LDialog_AlertBox(LFormat("%d",selectedShapeType));
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		//LDialog_AlertBox(LFormat("Completed.\nTotal %d Object.\nxLeft:%d",counter,xLeft));
	}

	void GetObjectCoord(LObject selectedObject,long* xLeft,long* xRight,long* yBottom,long* yTop)
	{
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		//Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
			case 0://LBox
			{
				LRect rect = LBox_GetRect(selectedObject);
				//get rect xmin
				if (rect.x0 < rect.x1 && rect.x0 < *xLeft)
					*xLeft = rect.x0;
				if (rect.x1 < rect.x0 && rect.x1 < *xLeft)
					*xLeft = rect.x1;
				//get rect xmax
				if (rect.x0 < rect.x1 && rect.x1 > *xRight)
					*xRight = rect.x1;
				if (rect.x1 < rect.x0 && rect.x0 > *xRight)
					*xRight = rect.x0;
				//get rect ymin
				if (rect.y0 < rect.y1 && rect.y0 < *yBottom)
					*yBottom = rect.y0;
				if (rect.y1 < rect.y0 && rect.y1 < *yBottom)
					*yBottom = rect.y1;
				//get rect ymax
				if (rect.y0 < rect.y1 && rect.y1 > *yTop)
					*yTop = rect.y1;
				if (rect.y1 < rect.y0 && rect.y0 > *yTop)
					*yTop = rect.y0;
				break;
			}
			case 1://LCircle
			{
				LPoint center = LCircle_GetCenter(selectedObject);
				LCoord r = LCircle_GetRadius(selectedObject);
				//get Circle xmin
				if (center.x - r < *xLeft)
					*xLeft = center.x - r;
				//get Circle xmax
				if (center.x + r > *xRight)
					*xRight = center.x + r;
				//get Circle ymin
				if (center.y - r < *yBottom)
					*yBottom = center.y - r;
				//get Circle ymax
				if (center.y + r > *yTop)
					*yTop = center.y + r;
				break;
			}
			case 3://LPolygon
			{
				LVertex vertex = LObject_GetVertexList(selectedObject);
				while (vertex != NULL)
				{
					LPoint point = LVertex_GetPoint(vertex);
					//get Polygon xmin
					if (point.x < *xLeft)
						*xLeft = point.x;
					//get Polygon xmax
					if (point.x > *xRight)
						*xRight = point.x;
					//get Polygon ymin
					if (point.y < *yBottom)
						*yBottom = point.y;
					//get Polygon xmax
					if (point.y > *yTop)
						*yTop = point.y;
					vertex = LVertex_GetNext(vertex);
				}
				break;
			}
			case 4://LTorus
			{
				LTorusParams pParams;
				LStatus lStatus = LTorus_GetParams(selectedObject, &pParams);
				if (lStatus == 0)
				{
					//get Torus xmin
					if (pParams.ptCenter.x - pParams.nOuterRadius < *xLeft)
						*xLeft = pParams.ptCenter.x - pParams.nOuterRadius;
					//get Torus xmax
					if (pParams.ptCenter.x + pParams.nOuterRadius > *xRight)
						*xRight = pParams.ptCenter.x + pParams.nOuterRadius;
					//get Torus ymin
					if (pParams.ptCenter.y - pParams.nOuterRadius < *yBottom)
						*yBottom = pParams.ptCenter.y - pParams.nOuterRadius;
					//get Torus ymax
					if (pParams.ptCenter.y + pParams.nOuterRadius > *yTop)
						*yTop = pParams.ptCenter.y + pParams.nOuterRadius;
				}
				break;
			}
			case 5://LPie
			{
				LPieParams pParams;
				LStatus lStatus = LPie_GetParams(selectedObject, &pParams);
				if (lStatus == 0)
				{
					//get Pie xmin
					if (pParams.ptCenter.x - pParams.nRadius < *xLeft)
						*xLeft = pParams.ptCenter.x - pParams.nRadius;
					//get Pie xmax
					if (pParams.ptCenter.x + pParams.nRadius > *xRight)
						*xRight = pParams.ptCenter.x + pParams.nRadius;
					//get Pie ymin
					if (pParams.ptCenter.y - pParams.nRadius < *yBottom)
						*yBottom = pParams.ptCenter.y - pParams.nRadius;
					//get Pie ymax
					if (pParams.ptCenter.y + pParams.nRadius > *yTop)
						*yTop = pParams.ptCenter.y + pParams.nRadius;
				}
				break;
			}
			default:
				break;
			}
		//
	}

	void  Register_Alignment_func(void)
	{
		LMacro_BindToHotKey(KEY_F2, "Left_func", "Alignment_Left");
		LMacro_BindToHotKey(KEY_F3, "Right_func", "Alignment_Right");
		LMacro_BindToHotKey(KEY_F4, "Bottom_func", "Alignment_Bottom");
		LMacro_BindToHotKey(KEY_F5, "Top_func", "Alignment_Top");
		LMacro_BindToHotKey(KEY_F6, "Center_func", "Alignment_Center");
	}
} /* end of module Alignment_Moudle*/

Register_Alignment_func();
