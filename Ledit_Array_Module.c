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
	#include "ldata.h"     /* Main UPI header. */

	#define PI 3.14159265358979323846
	void ArrayByEdgeDistanceOneDimension();
	void ArrayByOffsetOneDimension();
	void ArrayByRingSetDistance();
	void ArrayInObjectByDistanceHexagon();
	void _ArrayInObjectByDistanceHexagon(LPoint **point, int *num, long *_distance);
	void ArrayInObjectByDistanceHexagonAutoFixEdgeSize();
	void ArrayInObjectByRing();
	int CalculateOffset(LObject selectedObject, long distance, double rad, double *offset);
	double CalculatePolarAngle(LPoint p1, LPoint p2);
	void CopyObject(LObject selectedObject, int arrayNumber, double xOffset, double yOffset);
	void CopySelectedObjects(int arrayNumber, double xOffset, double yOffset);
	void CopySelectedObjectsByEdgeDistance(int arrayNumber, long distance, double deg);
	void GetObjectCoord(LObject selectedObject, long *box);
	double GetPointDistance(LPoint point1, LPoint point2);
	int LSelection_GetNumber(LSelection selectedInital);
	LPoint MirrorCoordinates(LPoint p1, LPoint p2, double rad);
	void MirrorObjectsByPointAndRad();
	void RotateObjectsByPoint();
	LPoint RotatePoint(LPoint p1, LPoint p2, double rad);
	

	void MirrorObjectsByPointAndRad()
	{
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{ "Mirror Center X Coordinate (nm)", "0" },
		{ "Mirror Center Y Coordinate (nm)", "0" },
		{ "Mirror degree (0-180)", "0" }};
		long xcoord;
		long ycoord;
		int rotate;
		if(LDialog_MultiLineInputBox("Array By Ring Set Distance", Dialog_Items, 3))
		{
			xcoord = atol(Dialog_Items[0].value); // get the xcoord
			ycoord = atol(Dialog_Items[1].value); // get the ycoord
			rotate = atoi(Dialog_Items[2].value); // get the rotate
		}
		else{
			return;
		}
		//****************************Input Params****************************//
		double rad = rotate * PI / 180;
		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);//

			LPoint selectedObjectCenter = LCircle_GetCenter(selectedObject);
			LPoint mirrorPoint = LPoint_Set(xcoord, ycoord);
			double distance = GetPointDistance(selectedObjectCenter, mirrorPoint);

			LPoint newPoint = MirrorCoordinates(selectedObjectCenter, mirrorPoint, rad);
			
			long xoffset = newPoint.x - selectedObjectCenter.x;
			long yoffset = newPoint.y - selectedObjectCenter.y;

			CopyObject(selectedObject, 2, xoffset, yoffset); // 2 means create new object 		

			//LDialog_AlertBox(LFormat("%d",selectedShapeType));
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		LDisplay_Refresh();
	}

	void RotateObjectsByPoint()
	{
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{ "Rotate Center X Coordinate (nm)", "0" },
		{ "Rotate Center Y Coordinate (nm)", "0" },
		{ "Rotate degree (0-360)", "0" }};
		long xcoord;
		long ycoord;
		int rotate;
		if(LDialog_MultiLineInputBox("Array By Ring Set Distance", Dialog_Items, 3))
		{
			xcoord = atol(Dialog_Items[0].value); // get the xcoord
			ycoord = atol(Dialog_Items[1].value); // get the ycoord
			rotate = atoi(Dialog_Items[2].value); // get the rotate
		}
		else{
			return;
		}
		//****************************Input Params****************************//
		double rad = rotate * PI / 180;

		int counter = 0;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LShapeType selectedShapeType = LObject_GetShape(selectedObject);//

			LPoint selectedObjectCenter = LCircle_GetCenter(selectedObject);
			LPoint rotateCenter = LPoint_Set(xcoord, ycoord);
			double distance = GetPointDistance(selectedObjectCenter, rotateCenter);

			LPoint newPoint = RotatePoint(selectedObjectCenter, rotateCenter, rad);
			
			long xoffset = newPoint.x - selectedObjectCenter.x;
			long yoffset = newPoint.y - selectedObjectCenter.y;

			CopyObject(selectedObject, 2, xoffset, yoffset); // 2 means create new object 		

			//LDialog_AlertBox(LFormat("%d",selectedShapeType));
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		LDisplay_Refresh();
	}

	void ArrayInObjectByDistanceHexagonAutoFixEdgeSize()
	{
    	double startAngle = PI / 3.0;
    	double interval = PI / 3.0; 

		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

		LPoint *circlePoints = NULL;;
		int *num = 0;
		long *_distance = 0;
		_ArrayInObjectByDistanceHexagon(&circlePoints, &num, &_distance);
		LSelection selectedInital = LSelection_GetList();

		LObject object1 = LSelection_GetObject(selectedInital);
		selectedInital = LSelection_GetNext(selectedInital);
		LObject object2 = LSelection_GetObject(selectedInital);
		
		double area1 = LObject_Area(object1);
		double area2 = LObject_Area(object2);

		LObject smallerObject = area1 > area2 ? object2 : object1;
		LObject biggerObject = area1 > area2 ? object1 : object2;
		int i, j;
		for(i = 0; i< num; i++)
		{
			double center2center = GetPointDistance(circlePoints[i], LCircle_GetCenter(biggerObject));
			double deltaL = center2center + LCircle_GetRadius(smallerObject) - LCircle_GetRadius(biggerObject);
			if(deltaL > 0)
			{
				
				double rad = CalculatePolarAngle(circlePoints[i], LCircle_GetCenter(biggerObject));
				double mod = fmod(rad + PI, PI / 3);
				int inter = (rad + PI) *3 / PI;
				if(mod > PI / 6) inter += 1;
				double direction = inter * PI / 3;

				long r_new = LCircle_GetRadius(smallerObject) - deltaL / 4;
				long offset1 = deltaL * 3 / 4;
				long dist = _distance;
				long offset2 = deltaL * 1 / 4 + 2 * r_new + dist;
				
				//Not considering non-axial movement direction, and the distance of movement is insufficient
				LPoint centerOffset = LPoint_Set((long)(circlePoints[i].x - cos(direction) * offset1), (long)(circlePoints[i].y - sin(direction) * offset1));
				LCircle_New(Cell_Now, LLayer_Now, centerOffset, r_new);
				
				for(j = inter - 1; j < inter + 2; j++)
				{
					direction = j * PI / 3;
					LPoint centerOffset = LPoint_Set((long)(circlePoints[i].x - cos(direction) * offset2), (long)(circlePoints[i].y - sin(direction) * offset2));
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r_new);
				}
			}
		}
		LDisplay_Refresh();
	}

	void ArrayByRingSetDistance()
	{
		LSelection selectedInital = LSelection_GetList();
		int selectedObjectNumber =	LSelection_GetNumber(selectedInital);
		if(selectedObjectNumber != 1)
		{
			LDialog_AlertBox("Just Support 1 Object");
			return;
		}
		LObject object1 = LSelection_GetObject(selectedInital);
		if(LObject_GetShape(object1) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{ "Array Edge Distance(nm)", "1000" },
		{ "Array Start Dimension deg (0-360)", "0" },
		{ "Ring Radius(nm)", "1000" }};
		long distance;
		double deg;
		long radius;
		if(LDialog_MultiLineInputBox("Array By Ring Set Distance", Dialog_Items, 3))
		{
			distance = atol(Dialog_Items[0].value); // get the distance
			deg = atoi(Dialog_Items[1].value); // get the deg
			radius = atol(Dialog_Items[2].value); // get the radius
		}
		else{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * PI / 180;
		//if small one inner big one
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		LPoint center = LCircle_GetCenter(object1);
		LCoord r = LCircle_GetRadius(object1);
		if(2 * r + distance <= 0)
		{
			LDialog_AlertBox("Value of 'distance' error");
			return;
		}
		//calculate how many circle in one ring
		double radius_ring = radius;
		int maxCircle = 2 * PI * radius_ring / (2 * r + distance);
		double rad_offset = 2 * PI / maxCircle;
		int j;
		for(j = 0; j < maxCircle; j++)
		{
			int yOffset = radius * sin(rad + rad_offset * j);
			int xOffset = radius * cos(rad + rad_offset * j);
			long center_x = center.x + xOffset;
			long center_y = center.y + yOffset;
			LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
			LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
		}

		LDisplay_Refresh();
	}

	void ArrayInObjectByRing()
	{
		LSelection selectedInital = LSelection_GetList();
		int selectedObjectNumber =	LSelection_GetNumber(selectedInital);
		if(selectedObjectNumber != 2)
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

		if(LObject_GetShape(smallerObject) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		//****************************Input Params****************************//
		LDialogItem Dialog_Items[2] = {{ "Array Edge Distance(nm)", "1000" },
		{ "Array Start Dimension deg (0-360)", "0" }};
		long distance;
		double deg;
		if(LDialog_MultiLineInputBox("Array In Object By Ring", Dialog_Items, 2))
		{
			distance = atol(Dialog_Items[0].value); // get the distance
			deg = atoi(Dialog_Items[1].value); // get the deg
		}
		else{
			return;
		}
		//****************************Input Params****************************//

		double rad = deg * PI / 180;
		//if small one inner big one
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		long box[4] ;
		box[0] = WORLD_MAX;	//x0
		box[1] = WORLD_MAX;	//y0
		box[2] = -WORLD_MAX;	//x1
		box[3] = -WORLD_MAX;	//y1
		GetObjectCoord(biggerObject, box);
		LPoint center = LCircle_GetCenter(smallerObject);
		LCoord r = LCircle_GetRadius(smallerObject);
		if(2 * r + distance <= 0)
		{
			LDialog_AlertBox("Value of 'distance' error");
			return;
		}
		double distance1 = GetPointDistance(center, LPoint_Set(box[0], box[1]));	// x0 y0
		double distance2 = GetPointDistance(center, LPoint_Set(box[0], box[3]));	// x0 y1
		double distance3 = GetPointDistance(center, LPoint_Set(box[2], box[1]));	// x1 y0
		double distance4 = GetPointDistance(center, LPoint_Set(box[2], box[3]));	// x1 y1

		//long time long too big, convert to double to calculate
		double radius_max = max(max(distance1, distance2),max(distance1, distance2));
		//calculate how many ring need
		int maxRingLayer = (int)(radius_max / (2 * r + distance));
		int i , j;
		for(i = 0; i < maxRingLayer; i++)
		{
			//calculate how many circle in one ring
			double radius_ring = (i + 1) * (2 * r + distance);
			int maxCircle = 2 * PI * radius_ring / (2 * r + distance);
			double rad_offset = 2 * PI / maxCircle;
			for(j = 0; j < maxCircle; j++)
			{
				int yOffset = (i + 1) * (2 * r + distance) * sin(rad + rad_offset * j);
				int xOffset = (i + 1) * (2 * r + distance) * cos(rad + rad_offset * j);
				long center_x = center.x + xOffset;
				long center_y = center.y + yOffset;
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if(LObject_DistanceToPoint( biggerObject, centerOffset, File_Now)<0)
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

	void _ArrayInObjectByDistanceHexagon(LPoint **points, int *num, long *_distance)
	{
		//Dynamic size
		int initalSize = 100;
		*points = (LPoint *)malloc(initalSize * sizeof(LPoint));

		LSelection selectedInital = LSelection_GetList();
		int selectedObjectNumber =	LSelection_GetNumber(selectedInital);
		if(selectedObjectNumber != 2)
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

		if(LObject_GetShape(smallerObject) != 1)
		{
			LDialog_AlertBox("Just Support Circle Object Array");
			return;
		}

		//****************************Input Params****************************//
		LDialogItem Dialog_Items[1] = {{ "Array Edge Distance(nm)", "1000" }};
		long distance;
		double deg = 60;
		if (LDialog_MultiLineInputBox ("Hexagon Properties", Dialog_Items, 1))
		{
			distance = atol(Dialog_Items[0].value); // get the distance
			*_distance = distance;
		}
		else{
			return;
		}
		//****************************Input Params****************************//
		double rad = deg * PI / 180;
		//if small one inner big one
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		long box[4] ;
		box[0] = WORLD_MAX;	//x0
		box[1] = WORLD_MAX;	//y0
		box[2] = -WORLD_MAX;	//x1
		box[3] = -WORLD_MAX;	//y1
		GetObjectCoord(biggerObject, box);
		LPoint center = LCircle_GetCenter(smallerObject);
		LCoord r = LCircle_GetRadius(smallerObject);
		if(2 * r + distance <= 0)
		{
			LDialog_AlertBox("Value of 'distance' error");
			return;
		}
		// phase 1 + +
		int n = 0;
		double yOffset = 0;
		double xOffset = 0;
		while(center.y + n * (2 * r + distance) + yOffset < box[3])
		{
			int m = 0;
			while(center.x + m * (2 * r + distance) + xOffset < box[2])
			{
				if( n == 0 && m == 0 )
				{
					m++;
					continue;
				}
				long center_x = center.x + m * (2 * r + distance) + xOffset;
				long center_y = center.y + n * (2 * r + distance) + yOffset;
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if(LObject_DistanceToPoint( biggerObject, centerOffset, File_Now)<0)
				{
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
					(*points)[*num] = centerOffset;
					(*num) = (*num) + 1;
					if(*num > initalSize / 2){
						initalSize *= 2;
						*points = (LPoint *)realloc(*points, initalSize * sizeof(LPoint));
					}
				}
				m++;
			}
			n++;
			yOffset = (2 * r + distance) * (sin(rad) -1)* n;
			xOffset = (2 * r + distance) * cos(rad) * (n % 2);
		}
		// phase 2 + -
		n = 1;
		xOffset = 0;
		yOffset = (2 * r + distance) * (sin(rad) -1)* n;
		while(center.y - (n * (2 * r + distance) + yOffset) > box[1])
		{
			int m = 0;
			xOffset = (2 * r + distance) * cos(rad) * (n % 2);
			while(center.x + m * (2 * r + distance) + xOffset < box[2])
			{
				long center_x = center.x + m * (2 * r + distance) + xOffset;
				long center_y = center.y - (n * (2 * r + distance) + yOffset);
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if(LObject_DistanceToPoint( biggerObject, centerOffset, File_Now)<0)
				{
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
					(*points)[*num] = centerOffset;
					(*num) = (*num) + 1;
					if(*num > initalSize / 2){
						initalSize *= 2;
						*points = (LPoint *)realloc(*points, initalSize * sizeof(LPoint));
					}
				}
				m++;
			}
			n++;
			yOffset = (2 * r + distance) * (sin(rad) -1)* n;
		}
		// phase 3 - -
		n = 0;
		xOffset = 0;
		yOffset = (2 * r + distance) * (sin(rad) -1)* n;
		while(center.y - (n * (2 * r + distance) + yOffset) > box[1])
		{
			int m = 0;
			xOffset = (2 * r + distance) * cos(rad) * (n % 2);
			while(center.x - (m * (2 * r + distance) + xOffset) > box[0])
			{
				if( n % 2 == 0 && m == 0 )
				{
					m++;
					continue;
				}
				long center_x = center.x - (m * (2 * r + distance) + xOffset);
				long center_y = center.y - (n * (2 * r + distance) + yOffset);
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if(LObject_DistanceToPoint( biggerObject, centerOffset, File_Now)<0)
				{
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
					(*points)[*num] = centerOffset;
					(*num) = (*num) + 1;
					if(*num > initalSize / 2){
						initalSize *= 2;
						*points = (LPoint *)realloc(*points, initalSize * sizeof(LPoint));
					}
				}
				m++;
			}
			n++;
			yOffset = (2 * r + distance) * (sin(rad) -1)* n;
		}
		// phase 4 - +
		n = 1;
		xOffset = 0;
		yOffset = (2 * r + distance) * (sin(rad) -1)* n;
		while(center.y + n * (2 * r + distance) + yOffset < box[3])
		{
			int m = 0;
			xOffset = (2 * r + distance) * cos(rad) * (n % 2);
			while(center.x - (m * (2 * r + distance) + xOffset) > box[0])
			{
				if( n % 2 == 0 && m == 0 )
				{
					m++;
					continue;
				}
				long center_x = center.x - (m * (2 * r + distance) + xOffset);
				long center_y = center.y + n * (2 * r + distance) + yOffset;
				LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
				if(LObject_DistanceToPoint( biggerObject, centerOffset, File_Now)<0)
				{
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
					(*points)[*num] = centerOffset;
					(*num) = (*num) + 1;
					if(*num > initalSize / 2){
						initalSize *= 2;
						*points = (LPoint *)realloc(*points, initalSize * sizeof(LPoint));
					}
				}
				m++;
			}
			n++;
			yOffset = (2 * r + distance) * (sin(rad) -1)* n;
		}
	}

	int LSelection_GetNumber(LSelection selectedInital)
	{
		int counter = 0;
		while(selectedInital != NULL)
		{
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
		return counter;
	}

	void ArrayByEdgeDistanceOneDimension()
	{
		//**************************************************//
		
		LDialogItem Dialog_Items[3] = {{ "Array Edge Distance(nm)", "1000" },
		{ "Array Number", "2" },
		{ "Array Dimension deg (0-360)", "0" }};
		long distance;
		int arrayNumber;
		double deg;
		if(LDialog_MultiLineInputBox("Array By Edge Distance One Dimension", Dialog_Items, 3))
		{
			distance = atol(Dialog_Items[0].value); // get the distance
			arrayNumber = atoi(Dialog_Items[1].value); // get the arrayNumber
			deg = atof(Dialog_Items[2].value); // get the deg
		}
		else{
			return;
		}
		
		//**************************************************//
		if((distance >= 0 || arrayNumber > 1) || (deg >= 0 && deg <= 360))
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
			double rad = deg * PI / 180;				//
			double *offset = (double *)malloc(sizeof(double));
			if(CalculateOffset(selectedObject, distance, rad, offset) == 1);
			{
				double xOffset = offset[0];	//
				double yOffset = offset[1];	//
				//LDialog_AlertBox(LFormat("xOffset:%f, yOffset:%f", xOffset, yOffset));
				CopyObject(selectedObject, arrayNumber, xOffset, yOffset);
				counter++;
			}
			selectedInital = LSelection_GetNext(selectedInital);
		}
	}

	int CalculateOffset(LObject selectedObject, long distance, double rad, double *offset)
	{
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		//Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
			case 0://LBox
			{
				return 0;
			}
			case 1://LCircle
			{
				LCoord r = LCircle_GetRadius(selectedObject);
				offset[0] = (2 * r + distance) * cos(rad);
				offset[1] = (2 * r + distance) * sin(rad);
				return 1;
			}
			case 3://LPolygon
			{
				return 0;
			}
			case 4://LTorus
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
			case 5://LPie
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
		//**************************************************//
		
		LDialogItem Dialog_Items[3] = {{ "Array Center Distance(nm)", "1000" },
		{ "Array Number", "2" },
		{ "Array Dimension deg (0-360)", "0" }};
		long distance;
		int arrayNumber;
		double deg;
		if(LDialog_MultiLineInputBox("Array By Offset One Dimension", Dialog_Items, 3))
		{
			distance = atol(Dialog_Items[0].value); // get the distance
			arrayNumber = atoi(Dialog_Items[1].value); // get the arrayNumber
			deg = atof(Dialog_Items[2].value); // get the deg
		}
		else{
			return;
		}
		
		//**************************************************//
		if((distance >= 0 || arrayNumber > 1) || (deg >= 0 && deg <= 360)){
			double rad = deg * PI / 180;
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

			CopyObject(selectedObject, arrayNumber, xOffset, yOffset);
			//LDialog_AlertBox(LFormat("%d",selectedShapeType));
			selectedInital = LSelection_GetNext(selectedInital);
			counter++;
		}
	}
	
	void CopyObject(LObject selectedObject, int arrayNumber, double xOffset, double yOffset)
	{
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		//LDialog_AlertBox(LFormat("xOffset:%f, yOffset:%f",xOffset, yOffset));
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		//Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
			case 0://LBox
			{
				//get rect xmin
				int i;
				for(i = 1; i < arrayNumber; i++)
				{
					LRect rect = LBox_GetRect(selectedObject);
					double rect_x0 = rect.x0 + i * xOffset;
					double rect_y0 = rect.y0 + i * yOffset;
					double rect_x1 = rect.x1 + i * xOffset;
					double rect_y1 = rect.y1 + i * yOffset;
					LBox_New(Cell_Now, LLayer_Now,
					(long)rect_x0, (long)rect_y0, (long)rect_x1, (long)rect_y1);
				}
				break;
			}
			case 1://LCircle
			{
				LPoint center = LCircle_GetCenter(selectedObject);
				LCoord r = LCircle_GetRadius(selectedObject);
				int i;
				for(i = 1; i < arrayNumber; i++)
				{
					double center_x = center.x + i * xOffset;
					double center_y = center.y + i * yOffset;
					LPoint centerOffset = LPoint_Set((long)center_x, (long)center_y);
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
				}
				break;
			}
			case 3://LPolygon
			{
				int i;
				for(i = 1; i < arrayNumber; i++)
				{
					LVertex vertex = LObject_GetVertexList(selectedObject);
					long cnt = LVertex_GetCount(selectedObject);
					LPoint	*points;
					points = (LPoint *)malloc((cnt) * sizeof(LPoint));
					int j = 0;
					while (vertex != NULL)
					{
						LPoint point = LVertex_GetPoint(vertex);
						double point_x = point.x + i * xOffset;
						double point_y = point.y + i * yOffset;
						points[j] = LPoint_Set((long)point_x, (long)point_y);
						j++;
						vertex = LVertex_GetNext(vertex);
					}
					LPolygon_New( Cell_Now, LLayer_Now, points, cnt  );
				}
				break;
				
			}
			case 4://LTorus
			{
				int i;
				for(i = 1; i < arrayNumber; i++)
				{
					LTorusParams tParams;
					LStatus lStatus = LTorus_GetParams(selectedObject, &tParams);
					if (lStatus == 0)
					{
						/* Define parameters for a torus */
						LTorusParams _tParams;
						double center_x = tParams.ptCenter.x + i * xOffset;
						double center_y = tParams.ptCenter.y + i * yOffset;
						_tParams.ptCenter = LPoint_Set((long)center_x, (long)center_y);
						_tParams.nInnerRadius = tParams.nInnerRadius;
						_tParams.nOuterRadius = tParams.nOuterRadius;
						_tParams.dStartAngle = tParams.dStartAngle;
						_tParams.dStopAngle = tParams.dStopAngle;
						LTorus_CreateNew(Cell_Now, LLayer_Now, &_tParams);
					}
				}
				break;
			}
			case 5://LPie
			{
				int i;
				for(i = 1; i < arrayNumber; i++)
				{
					LPieParams pParams;
					LStatus lStatus = LPie_GetParams(selectedObject, &pParams);
					if (lStatus == 0)
					{
						LPieParams _pParams;
						double center_x = pParams.ptCenter.x + i * xOffset;
						double center_y = pParams.ptCenter.y + i * yOffset;
						_pParams.ptCenter = LPoint_Set((long)center_x, (long)center_y);
						_pParams.nRadius = pParams.nRadius;
						_pParams.dStartAngle = pParams.dStartAngle;
						_pParams.dStopAngle = pParams.dStopAngle;
						LPie_CreateNew(Cell_Now, LLayer_Now, &_pParams);
					}
				}
				break;
			}
			default:
				break;
			}
		//
	}

	void GetObjectCoord(LObject selectedObject, long *box)
	{
		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		//Function for selectedObject exmple get the left xcoord
		switch(selectedShapeType)
		{
			case 0:	//LBox
				{
					//get rect xmin
					LRect rect = LBox_GetRect(selectedObject);
					box[0] = rect.x0;
					box[1] = rect.y0;
					box[2] = rect.x1;
					box[3] = rect.y1;
					break;
				}
			case 1:	//LCircle
				{
					LPoint center = LCircle_GetCenter(selectedObject);
					LCoord r = LCircle_GetRadius(selectedObject);
					box[0] = center.x - r;
					box[1] = center.y - r;
					box[2] = center.x + r;
					box[3] = center.y + r;
					break;
				}
			case 3:	//LPolygon
				{
					LVertex vertex = LObject_GetVertexList(selectedObject);
					while(vertex != NULL)
					{
						LPoint point = LVertex_GetPoint(vertex);
						if(point.x < box[0])
							box[0] = point.x;
						if(point.y < box[1])
							box[1] = point.y;
						if(point.x > box[2])
							box[2] = point.x;
						if(point.y > box[3])
							box[3] = point.y;
						vertex = LVertex_GetNext(vertex);
					}
					break;
				}
			case 4:	//LTorus
				{
					LTorusParams pParams;
					if(LTorus_GetParams(selectedObject, &pParams) != NULL)
					{
						box[0] = pParams.ptCenter.x - pParams.nOuterRadius;
						box[1] = pParams.ptCenter.y - pParams.nOuterRadius;
						box[2] = pParams.ptCenter.x + pParams.nOuterRadius;
						box[3] = pParams.ptCenter.y + pParams.nOuterRadius;
					}
					break;
				}
			case 5:	//LPie
				{
					LPieParams pParams;
					if(LPie_GetParams(selectedObject, &pParams) != NULL)
					{
						box[0] = pParams.ptCenter.x - pParams.nRadius;
						box[1]= pParams.ptCenter.y - pParams.nRadius;
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

		return angle;
	}

	LPoint RotatePoint(LPoint p1, LPoint p2, double rad) {
		double relX = p1.x - p2.x;
		double relY = p1.y - p2.y;
		
		long x = relX * cos(rad) - relY * sin(rad) + p2.x;
		long y = relX * sin(rad) + relY * cos(rad) + p2.y;
		return LPoint_Set(x, y);
	}

	LPoint _MirrorCoordinates(LPoint p1, LPoint p2, double rad)
	{
		double theta = 2 * rad;
		long xMirror = p1.x * cos(theta) + p1.y * sin(theta) + 2 * p2.x * cos(rad) - 2 * p2.y * sin(rad);
		long yMirror = p1.x * sin(theta) - p1.y * cos(theta) + 2 * p2.x * sin(rad) + 2 * p2.y * cos(rad);
		return LPoint_Set(xMirror, yMirror);
	}

	LPoint MirrorCoordinates(LPoint p, LPoint linePoint, double rad) 
	{
		// Calculate the tangent value of the line's angle
		double slopeTan = tan(rad);
		// Handle the case when tan(pi/2) is encountered
		if (abs(rad - PI/2) < 1e-6) {
			LPoint mirrorPoint;
			mirrorPoint.x = 2 * linePoint.x - p.x;
			mirrorPoint.y = p.y;
			return mirrorPoint;
		}

		// Calculate the constant term 'c' in the equation y = mx + c
		double c = linePoint.y - slopeTan * linePoint.x;

		// Calculate the perpendicular distance from the point to the line
		double distance = abs(slopeTan * p.x - p.y + c) / sqrt(1 + slopeTan * slopeTan);

		double A = slopeTan;
		double B = -1;
		double C = c;
		// Calculate the mirror coordinates of the point with respect to the line
		
		long mirrorx = -1 * (2 * A * B * p.y + (A * A - B * B) * p.x + 2 * A * C) / (B * B + A * A);
		long mirrory = -1 * (2 * A * B * p.x - (A * A - B * B) * p.y + 2 * B * C) / (B * B + A * A);
		return LPoint_Set(mirrorx, mirrory);
	}

	void  Array_func(void)
	{
		//LMacro_BindToMenu( const char* menu, const char* macro_desc, const char* function_name );
		LMacro_Register("ArrayByRingSetDistance_func", "ArrayByRingSetDistance");
		LMacro_Register("ArrayByOffsetOneDimension_func", "ArrayByOffsetOneDimension");
		LMacro_Register("ArrayByEdgeDistanceOneDimension_func", "ArrayByEdgeDistanceOneDimension");
		LMacro_Register("ArrayInObjectByDistanceHexagon_func", "ArrayInObjectByDistanceHexagon");
		LMacro_Register("ArrayInObjectByRing_func","ArrayInObjectByRing");
		LMacro_Register("ArrayInObjectByDistanceHexagonAutoFixEdgeSize_func","ArrayInObjectByDistanceHexagonAutoFixEdgeSize");
		LMacro_Register("RotateObjectsByPoint_func", "RotateObjectsByPoint");
		LMacro_Register("MirrorObjectsByPointAndRad_func", "MirrorObjectsByPointAndRad");
	}
} /* end of module Array_Module */

Array_func();