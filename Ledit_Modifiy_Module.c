/*
 * Modifiy_Module
 * Modifiy_Module Marco for Ledit
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
module Modifiy_Module
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Ledit_Modifiy.h"
#include "ldata.h" /* Main UPI header. */

	void Route_Circle()
	{
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[2] = {{"Target radius (um)", "1.5"},
									   {"Distance (um)", "7.8"}};
		long tar_radius;
		long distance;
		if (LDialog_MultiLineInputBox("Modifiy Selected Object", Dialog_Items, 2))
		{
			tar_radius = (long)(atof(Dialog_Items[0].value) * 1000); // get the tar_radius
			distance = (long)(atof(Dialog_Items[1].value) * 1000);	 // get the keep
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//

		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		double perimeter, true_distance;
		int num;
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			switch (LObject_GetShape(selectedObject))
			{
			case 1: // LCircle
				break;
			case 3: // LPolygon
				perimeter = LObject_Perimeter(selectedObject);
				num = (int)(perimeter / distance);

				while (num % 4 != 0)
					num--;						 // 由于对称，需要是偶数的数量
				true_distance = perimeter / num; // 计算真实的间距

				LPoint manul_Start;
				if (GetCrossPoint(selectedObject, &manul_Start) == 1)
				{
					DrawCircleByRouteDistance(true_distance, tar_radius, num, manul_Start, selectedObject);
				}
				break;
			default:
				break;
			}
			selectedInital = LSelection_GetNext(selectedInital);
		}
		LDisplay_Refresh();
	}

	int GetCrossPoint(LObject polygon, LPoint * crossPoint)
	{
		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		GetObjectCoord(polygon, &xLeft, &xRight, &yBottom, &yTop);
		LPoint polygonCenter = LPoint_Set((long)((xLeft + xRight) / 2), (long)((yBottom + yTop) / 2));
		long virtualX = polygonCenter.x + (xRight - xLeft);
		long virtualY = polygonCenter.y;
		// LDialog_MsgBox(LFormat("virtual.x: %d,y: %d",virtualX,virtualY));
		LPoint virtualPoint = LPoint_Set(virtualX, virtualY);
		long polygonPointsNum = LVertex_GetCount(polygon);

		LVertex routePoint1 = LObject_GetVertexList(polygon); // firstPoint

		int i = 0;
		while (i <= polygonPointsNum)
		{
			LPoint point1 = LVertex_GetPoint(routePoint1);
			LVertex routePoint2 = LVertex_GetNext(routePoint1);
			LPoint point2 = LVertex_GetPoint(routePoint2);
			if (calculateIntersection(polygonCenter, virtualPoint, point1, point2, crossPoint) == 1)
			{
				// LDialog_MsgBox(LFormat("polygonCenter.x: %d,y: %d\npoint1.x: %d,y: %d\npoint2.x: %d,y: %d",polygonCenter.x,polygonCenter.y,point1.x,point1.y,point2.x,point2.y));
				return 1;
			}

			routePoint1 = routePoint2;
			i++;
		}
		return 0;
	}

	// 判断两条线段是否相交并计算交点
	int calculateIntersection(LPoint a, LPoint b, LPoint c, LPoint d, LPoint * intersection)
	{
		// 计算向量差
		double denominator = (b.x * 1.0 - a.x) * (d.y * 1.0 - c.y) - (b.y * 1.0 - a.y) * (d.x * 1.0 - c.x);

		// 判断是否平行，平行则无交点
		if (denominator == 0.0)
		{
			return 0; // 无交点
		}

		// 使用参数方程计算t和u
		double t = (-1.0 * c.x * a.y + 1.0 * d.x * a.y + 1.0 * a.x * c.y - 1.0 * d.x * c.y - 1.0 * a.x * d.y + 1.0 * c.x * d.y) / denominator;
		double u = -(-1.0 * b.x * a.y + 1.0 * c.x * a.y + 1.0 * a.x * b.y - 1.0 * c.x * b.y - 1.0 * a.x * c.y + 1.0 * b.x * c.y) / denominator;

		// 检查t和u是否在[0, 1]范围内，表示线段相交
		if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
		{
			// 计算交点坐标
			intersection->x = (long)(a.x + t * (b.x - a.x));
			intersection->y = (long)(a.y + t * (b.y - a.y));
			// LDialog_MsgBox(LFormat("intersection.x: %d,y: %d",intersection->x,intersection->y));
			return 1; // 存在交点
		}

		return 0; // 无交点
	}

	// 计算线段AB与圆C的交点，圆心为center，半径为radius
	int calculateCircleSegmentIntersection(LPoint a, LPoint b, LPoint center, double radius, LPoint *intersection1, LPoint *intersection2)
	{
		// 线段方向向量
		double dx = b.x - a.x;
		double dy = b.y - a.y;

		// 圆心到线段A的向量
		double fx = a.x - center.x;
		double fy = a.y - center.y;

		// 二次方程系数
		double a_coeff = dx * dx + dy * dy;
		double b_coeff = 2 * (fx * dx + fy * dy);
		double c_coeff = fx * fx + fy * fy - radius * radius;

		// 计算判别式
		double discriminant = b_coeff * b_coeff - 4 * a_coeff * c_coeff;

		// 如果判别式小于0，则没有交点
		if (discriminant < 0)
		{
			return 0; // 无交点
		}

		// 计算两个交点对应的参数t
		discriminant = sqrt(discriminant);

		double t1 = (-b_coeff - discriminant) / (2 * a_coeff);
		double t2 = (-b_coeff + discriminant) / (2 * a_coeff);

		int count = 0;

		// 判断t1是否在[0, 1]之间
		if (t1 >= 0 && t1 <= 1)
		{
			intersection1->x = a.x + t1 * dx;
			intersection1->y = a.y + t1 * dy;
			count++;
		}

		// 判断t2是否在[0, 1]之间
		if (t2 >= 0 && t2 <= 1)
		{
			if (count == 0)
			{
				intersection1->x = a.x + t2 * dx;
				intersection1->y = a.y + t2 * dy;
			}
			else
			{
				intersection2->x = a.x + t2 * dx;
				intersection2->y = a.y + t2 * dy;
			}
			count++;
		}

		return count; // 返回交点个数
	}

	void DrawCircleByRouteDistance(double distance, long radius, int circleNumber, LPoint start, LObject route) // LPolygon
	{
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);

		long xLeft = WORLD_MAX;
		long xRight = -WORLD_MAX;
		long yBottom = WORLD_MAX;
		long yTop = -WORLD_MAX;
		GetObjectCoord(route, &xLeft, &xRight, &yBottom, &yTop);
		LPoint polygonCenter = LPoint_Set((long)((xLeft + xRight) / 2), (long)((yBottom + yTop) / 2));
		long routePointsNum = LVertex_GetCount(route);

		LVertex routePoint1 = LObject_GetVertexList(route); // firstPoint

		LPoint *point_arr = (LPoint *)malloc((routePointsNum) * sizeof(LPoint));

		LVertex_GetArray(route, point_arr, routePointsNum);

		// LDialog_MsgBox(LFormat("point_arr[1] x: %d,y:%d\n,point_arr[0] x: %d,y:%d",point_arr[1].x,point_arr[1].y,point_arr[0].x,point_arr[0].y));

		LPoint point0 = start;
		LCircle_New(Cell_Now, LLayer_Now, point0, radius);
		int i = 0, j = 0;
		while (i <= circleNumber)
		{
			LPoint point1 = point_arr[j];
			LPoint point2 = point_arr[(j + 1) % routePointsNum];

			LPoint intersection1;
			LPoint intersection2;
			if (calculateCircleSegmentIntersection(point1, point2, point0, distance, &intersection1, &intersection2) > 0)
			{
				/*
				if(isClockwise(polygonCenter, point0 ,intersection1) == 1)
				{
					//LCircle_New(Cell_Now, LLayer_Now, intersection1, radius);
					point0 = intersection1;
					i++;
				}
				if(isClockwise(polygonCenter, point0 ,intersection2) == 1)
				{
					//LCircle_New(Cell_Now, LLayer_Now, intersection2, radius);
					point0 = intersection2;
					i++;
				}
				*/
				point0 = intersection1;
				LCircle_New(Cell_Now, LLayer_Now, point0, radius);
				i++;
			}
			j = (j + 1) % routePointsNum;
		}
	}

	// 判断三个点是否构成顺时针方向
	int isClockwise(LPoint a, LPoint b, LPoint c)
	{
		// 计算叉积
		double cross_product = 1.0 * (b.x - a.x) * (c.y - a.y) - 1.0 * (b.y - a.y) * (c.x - a.x);

		if (cross_product < 0)
		{
			return 1; // 顺时针
		}
		else if (cross_product > 0)
		{
			return -1; // 逆时针
		}
		else
		{
			return 0; // 共线
		}
	}

	double GetPointsDistance(LPoint point1, LPoint point2)
	{
		return sqrt((point1.x - point2.x) * (point1.x - point2.x) + (point1.y - point2.y) * (point1.y - point2.y));
	}

	void GetObjectCoord(LObject selectedObject, long *xLeft, long *xRight, long *yBottom, long *yTop)
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

	void Modifiy_Circle(LObject circle, long target_radius)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, circle);

		LPoint center = LCircle_GetCenter(circle);
		LCircle_Set(Cell_Now, circle, center, target_radius);
	}

	void Scale_Object(LObject selectedObject,LPoint polygonCenter, double scaleFactor)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectedObject);

		LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		// Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
			case 0: // LBox
			{
			}
			case 1: // LCircle
			{
				LPoint center = LCircle_GetCenter(selectedObject);
				LCoord r = LCircle_GetRadius(selectedObject);
				long point_x = (long)(polygonCenter.x + (center.x- polygonCenter.x)*scaleFactor);
				long point_y = (long)(polygonCenter.y + (center.y- polygonCenter.y)*scaleFactor);
				LPoint newPoint = LPoint_Set(point_x, point_y);
				LCoord r_Scaled=(long)(r*scaleFactor);
				LCircle_New(Cell_Now, LLayer_Now, newPoint, r_Scaled);
			}
			case 3: // LPolygon
			{
				LVertex vertex = LObject_GetVertexList(selectedObject);
				long cnt = LVertex_GetCount(selectedObject);
				LPoint *points;
				points = (LPoint *)malloc((cnt) * sizeof(LPoint));
				int j = 0;
				while (vertex != NULL)
				{
					LPoint point = LVertex_GetPoint(vertex);
					//point.x
					//point.y
					long point_x = (long)(polygonCenter.x + (point.x- polygonCenter.x)*scaleFactor);
					long point_y = (long)(polygonCenter.y + (point.y- polygonCenter.y)*scaleFactor);
					points[j] = LPoint_Set(point_x, point_y);
					j++;
					vertex = LVertex_GetNext(vertex);
				}
				LPolygon_New(Cell_Now, LLayer_Now, points, cnt);
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
					long point_x = (long)(polygonCenter.x + (tParams.ptCenter.x- polygonCenter.x)*scaleFactor);
					long point_y = (long)(polygonCenter.y + (tParams.ptCenter.y- polygonCenter.y)*scaleFactor);
					LPoint newPoint = LPoint_Set(point_x, point_y);

					_tParams.ptCenter = newPoint;
					_tParams.nInnerRadius = tParams.nInnerRadius*scaleFactor;
					_tParams.nOuterRadius = tParams.nOuterRadius*scaleFactor;
					_tParams.dStartAngle = tParams.dStartAngle;
					_tParams.dStopAngle = tParams.dStopAngle;

					LTorus_CreateNew(Cell_Now, LLayer_Now, &_tParams);
				}
			}
			case 5: // LPie
			{
				LPieParams pParams;
				LStatus lStatus = LPie_GetParams(selectedObject, &pParams);
				if (lStatus == 0)
				{
					LPieParams _pParams;
					long point_x = (long)(polygonCenter.x + (_pParams.ptCenter.x- polygonCenter.x)*scaleFactor);
					long point_y = (long)(polygonCenter.y + (_pParams.ptCenter.y- polygonCenter.y)*scaleFactor);
					LPoint newPoint = LPoint_Set(point_x, point_y);

					_pParams.ptCenter = newPoint;
					_pParams.nRadius = pParams.nRadius*scaleFactor;
					_pParams.dStartAngle = pParams.dStartAngle;
					_pParams.dStopAngle = pParams.dStopAngle;
					
					LPie_CreateNew(Cell_Now, LLayer_Now, &_pParams);
				}
			}
			default:
				break;
		}
	}

	void Scale_SelectedObject_SetCenter()
	{
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {	{"Scale Factor (>0)", "1"},
										{"Center Coord X (um)","0"},
										{"Center Coord Y (um)","0"}};
		double scaleFactor;
		double center_x,center_y;
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
		//****************************Input Params****************************//
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LPoint polygonCenter = LPoint_Set((long)(center_x*1000),(long)(center_y*1000));
			Scale_Object(selectedObject, polygonCenter, scaleFactor);
			selectedInital = LSelection_GetNext(selectedInital);
		}
		LDisplay_Refresh();
	}

	void Scale_SelectedObject_ByObjectCenter()
	{
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
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
		//****************************Input Params****************************//
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			long xLeft = WORLD_MAX;
			long xRight = -WORLD_MAX;
			long yBottom = WORLD_MAX;
			long yTop = -WORLD_MAX;
			GetObjectCoord(selectedObject, &xLeft, &xRight, &yBottom, &yTop);

			LPoint polygonCenter = LPoint_Set((long)((xLeft + xRight) / 2), (long)((yBottom + yTop) / 2));
			Scale_Object(selectedObject, polygonCenter, scaleFactor);
			selectedInital = LSelection_GetNext(selectedInital);
		}
		LDisplay_Refresh();
	}

	void Modifiy_Selected_Object_Offset()
	{
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[2] = {{"Radius Change (um) (±Inf)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long radiusChange;
		int keep;
		if (LDialog_MultiLineInputBox("Modifiy Selected Object", Dialog_Items, 2))
		{
			radiusChange = (long)(atof(Dialog_Items[0].value) * 1000); // get the radius_offset
			keep = atoi(Dialog_Items[1].value);						   // get the keep
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
			switch (LObject_GetShape(selectedObject))
			{
				long tar_radius;
			case 1: // LCircle
				tar_radius = LCircle_GetRadius(selectedObject) + radiusChange;
				Modifiy_Circle(selectedObject, tar_radius);
				break;
			default:
				break;
			}
			if (keep != 1)
			{
				LObject_Delete(Cell_Now, selectedObject);
			}
			selectedInital = LSelection_GetNext(selectedInital);
		}
		LDisplay_Refresh();
	}

	bool Compare_Objects(LObject object1, LObject object2)
	{
		if(LObject_GetShape(object1) != LObject_GetShape(object2))
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
				if(cnt1 != cnt2) return false;

				while (vertex1 != NULL)
				{
					LPoint point1 = LVertex_GetPoint(vertex1);
					LPoint point2 = LVertex_GetPoint(vertex2);
					if(point1.x != point2.x || point1.y != point2.y) return false;
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
				}else{
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
					return 	pParams1.ptCenter.x == pParams2.ptCenter.x && pParams1.ptCenter.y == pParams2.ptCenter.y && pParams1.nRadius == pParams2.nRadius && pParams1.dStartAngle == pParams2.dStartAngle && pParams1.dStopAngle == pParams2.dStopAngle;
				}else{
					return false;
				}
			}
			default:
				return false;
		}
	}

	void Modifiy_Delete_Duplicates()
	{
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LSelection loop1SelectionPoint = LSelection_GetList();

		LObject* selectedObjects;
		selectedObjects = (LObject *)malloc(sizeof(LObject));
		int j = 0;
		while(loop1SelectionPoint != NULL)
		{
			LObject loop1Object = LSelection_GetObject(loop1SelectionPoint);

			LSelection loop2SelectionPoint = loop1SelectionPoint;
			loop2SelectionPoint = LSelection_GetNext(loop2SelectionPoint);
			while(loop2SelectionPoint != NULL)
			{
				LObject loop2Object = LSelection_GetObject(loop2SelectionPoint);
				if(Compare_Objects(loop1Object,loop2Object))
				{
					//LDialog_MsgBox(LFormat("object1: %d, object2: %d", LObject_GetShape(loop1Object), LObject_GetShape(loop2Object)));
					selectedObjects[j]= loop2Object;
					selectedObjects = (LObject *)realloc(selectedObjects, (j + 2) * sizeof(LObject));  // 将内存大小扩展
					j++;
				}
				loop2SelectionPoint = LSelection_GetNext(loop2SelectionPoint);
			}
			loop1SelectionPoint = LSelection_GetNext(loop1SelectionPoint);
		}
		int i;
		for(i = 0;i < j; i++)
		{
			LObject_Delete(Cell_Now, selectedObjects[i]);
		}
		LDisplay_Refresh();
	}

	void Modifiy_Selected_Object_UnionSize()
	{
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[2] = {{"Target radius (um)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long tar_radius;
		int keep;
		if (LDialog_MultiLineInputBox("Modifiy Selected Object", Dialog_Items, 2))
		{
			tar_radius = (long)(atof(Dialog_Items[0].value) * 1000); // get the tar_radius
			keep = atoi(Dialog_Items[1].value);						 // get the keep
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
			switch (LObject_GetShape(selectedObject))
			{
			case 1: // LCircle
				Modifiy_Circle(selectedObject, tar_radius);
				break;
			default:
				break;
			}
			if (keep != 1)
			{
				LObject_Delete(Cell_Now, selectedObject);
			}
			selectedInital = LSelection_GetNext(selectedInital);
		}
		LDisplay_Refresh();
	}

	void TestFunc()
	{
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LPoint zero = LPoint_Set(0,0);
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LCoord distance = LObject_DistanceToPoint(selectedObject, zero, File_Now);
			LCoord a = WORLD_MAX + distance;
			double c =1.0*distance;
			LDialog_AlertBox(LFormat("distance: %ld, %ld, %ld, %f", distance, WORLD_MAX, a ,c));
			selectedInital = LSelection_GetNext(selectedInital);
		}
	}



	void Modifiy_func(void)
	{
		LMacro_Register("Modifiy_Selected_Object_UnionSize_func", "Modifiy_Selected_Object_UnionSize");
		LMacro_Register("Modifiy_Selected_Object_Offset", "Modifiy_Selected_Object_Offset");
		LMacro_Register("Modifiy_Delete_Duplicates_func","Modifiy_Delete_Duplicates");
		LMacro_Register("Route_Circle_func", "Route_Circle");
		LMacro_Register("Scale_SelectedObject_ByObjectCenter_func","Scale_SelectedObject_ByObjectCenter");
		LMacro_Register("Scale_SelectedObject_SetCenter_func","Scale_SelectedObject_SetCenter");
		LMacro_Register("TestFunc_func","TestFunc");
	}
} /* end of module Array_Module */

Modifiy_func();