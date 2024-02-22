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
	void ArrayInObjectByDistanceHexagon();
	void ArrayInObjectByRing();
	int CalculateOffset(LObject selectedObject, long distance, double rad, double *offset);
	void CopyObject(LObject selectedObject, int arrayNumber, double xOffset, double yOffset);
	void CopySelectedObjects(int arrayNumber, double xOffset, double yOffset);
	void CopySelectedObjectsByEdgeDistance(int arrayNumber, long distance, double deg);
	void GetObjectCoord(LObject selectedObject, long *box);
	int LSelection_GetNumber(LSelection selectedInital);

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
		if(LObject_DistanceToPoint( biggerObject, center, File_Now)>0)
		{
			LDialog_AlertBox("Smaller object need inner of the bigger object");
			return;
		}

		long center_x = center.x;
		long center_y = center.y;

		long edgeDistance_x = center_x - box[0] > box[2] - center_x ? center_x-box[0] : box[2] - center_x;
		long edgeDistance_y = center_y - box[1] > box[3] - center_y ? center_y-box[1] : box[3] - center_y;
		//long time long too big, convert to double to calculate
		double radius_max = sqrt((double)edgeDistance_x * edgeDistance_x + (double)edgeDistance_y * edgeDistance_y);
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
	}

	void ArrayInObjectByDistanceHexagon()
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
		LDialogItem Dialog_Items[1] = {{ "Array Edge Distance(nm)", "1000" }};
		long distance;
		double deg = 60;
		if (LDialog_MultiLineInputBox ("Hexagon Properties", Dialog_Items, 1))
		{
			distance = atol(Dialog_Items[0].value); // get the distance
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
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
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
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
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
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
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
					LCircle_New(Cell_Now, LLayer_Now, centerOffset, r);
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
	void  Array_func(void)
	{
		LMacro_BindToHotKey(KEY_F7, "ArrayByOffsetOneDimension_func", "ArrayByOffsetOneDimension");
		LMacro_BindToHotKey(KEY_F8, "ArrayByEdgeDistanceOneDimension_func", "ArrayByEdgeDistanceOneDimension");
		LMacro_BindToHotKey(KEY_F9, "ArrayInObjectByDistanceHexagon_func", "ArrayInObjectByDistanceHexagon");
		LMacro_BindToHotKey(KEY_F10, "ArrayInObjectByRing_func","ArrayInObjectByRing");
	}
} /* end of module Array_Module */

Array_func();