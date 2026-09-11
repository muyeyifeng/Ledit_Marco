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
// #include "Ledit_Modifiy.h"
#include "ldata.h" /* Main UPI header. */
#include "Ledit_Safety.h"

	void Route_Circle();
	int GetCrossPoint(LObject polygon, LPoint * crossPoint);
	int calculateIntersection(LPoint a, LPoint b, LPoint c, LPoint d, LPoint * intersection);
	int calculateCircleSegmentIntersection(LPoint a, LPoint b, LPoint center, double radius, LPoint *intersection1, LPoint *intersection2);
	void DrawCircleByRouteDistance(double distance, long radius, int circleNumber, LPoint start, LObject route);
	int isClockwise(LPoint a, LPoint b, LPoint c);
	double GetPointsDistance(LPoint point1, LPoint point2);
	void GetObjectCoord(LObject selectedObject, long *xLeft, long *xRight, long *yBottom, long *yTop);
	void Modifiy_Circle(LObject circle, long target_radius);
	void Scale_Object(LObject selectedObject, LPoint polygonCenter, double scaleFactor);
	void Scale_SelectedObject_SetCenter();
	void Scale_SelectedObject_ByObjectCenter();
	void Modifiy_Selected_Object_Offset();
	bool Compare_Objects(LObject object1, LObject object2);
	void Modifiy_Delete_Duplicates();
	void Modifiy_Selected_Object_UnionSize();
	void TestFunc();
	void Modifiy_func(void);

	void Route_Circle()
	{
        if (!LM_Context()) return;
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[2] = {{"Target radius (um)", "1.5"},
									   {"Distance (um)", "7.8"}};
		long tar_radius;
		long distance;
		if (LM_Dialog("Modifiy Selected Object", Dialog_Items, 2))
		{
			tar_radius = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the tar_radius
			distance = (long)(atof(Dialog_Items[1].value) * LM_UnitsPerMicron());	 // get the keep
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//

		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LLayer LLayer_Now = LLayer_GetCurrent(File_Now);
		if (distance <= 0 || tar_radius <= 0) { LDialog_AlertBox("Radius and distance must be positive."); return; }
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
				if (LPolygon_HasCurve(selectedObject) || perimeter / distance > LM_MAX_OBJECTS) break;
                num = (int)(perimeter / distance);
                if (num < 4) { LDialog_AlertBox("At least four circles must fit the route."); break; }

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
		while (routePoint1 != NULL && i < polygonPointsNum)
		{
			LPoint point1 = LVertex_GetPoint(routePoint1);
			LVertex routePoint2 = LVertex_GetNext(routePoint1);
			if (!routePoint2) routePoint2 = LObject_GetVertexList(polygon);
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
		if (a_coeff == 0 || radius < 0) return 0;
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
		if (t2 >= 0 && t2 <= 1 && (count == 0 || discriminant > 0))
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
        long n = LVertex_GetCount(route);
        LPoint *points;
        double perimeter = 0, startAt = 0, best = 1e100;
        int i, k, edge = 0;
        double base = 0;
        if (n < 3 || n > LM_MAX_POINTS || circleNumber < 1 || circleNumber > LM_MAX_OBJECTS || distance <= 0 || radius <= 0 || LPolygon_HasCurve(route)) return;
        points = (LPoint *)malloc(n * sizeof(LPoint));
        if (!points) return;
        if (LVertex_GetArray(route, points, n) != n) { free(points); return; }
        for (i = 0; i < n; ++i) {
            LPoint a = points[i], b = points[(i + 1) % n];
            double dx = (double)b.x - a.x, dy = (double)b.y - a.y;
            double len = hypot(dx, dy), t, error;
            if (len == 0) continue;
            t = ((start.x - a.x) * dx + (start.y - a.y) * dy) / (len * len);
            t = fmax(0, fmin(1, t));
            error = hypot(start.x - a.x - t * dx, start.y - a.y - t * dy);
            if (error < best) { best = error; startAt = perimeter + t * len; }
            perimeter += len;
        }
        if (perimeter <= 0) { free(points); return; }
        /* Start at the projected point and visit at most two laps of edges. */
        for (k = 0; k < circleNumber; ++k) {
            double target = fmod(startAt + k * (perimeter / circleNumber), perimeter);
            double len;
            LPoint a, b, p;
            if (target < base) { edge = 0; base = 0; }
            while (edge < n - 1) {
                len = GetPointsDistance(points[edge], points[(edge + 1) % n]);
                if (len > 0 && target <= base + len) break;
                base += len; ++edge;
            }
            a = points[edge]; b = points[(edge + 1) % n];
            len = GetPointsDistance(a, b);
            if (len == 0) continue;
            p = LPoint_Set((long)(a.x + (target - base) / len * (b.x - a.x)),
                           (long)(a.y + (target - base) / len * (b.y - a.y)));
            if (!LM_NewCircle(LCell_GetVisible(), LLayer_GetCurrent(LCell_GetFile(LCell_GetVisible())), p, radius)) break;
        }
        free(points);
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
		return hypot((double)point1.x - point2.x, (double)point1.y - point2.y);
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

	void Modifiy_Circle(LObject circle, long target_radius)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, circle);

		LPoint center = LCircle_GetCenter(circle);
		LCircle_Set(Cell_Now, circle, center, target_radius);
	}

	void Scale_Object(LObject selectedObject, LPoint polygonCenter, double scaleFactor)
	{
		LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, selectedObject);

		LRect bounds = LObject_GetMbb(selectedObject);
        if (!(scaleFactor > 0) ||
            !LM_PointValid(polygonCenter.x + ((double)bounds.x0 - polygonCenter.x) * scaleFactor, polygonCenter.y + ((double)bounds.y0 - polygonCenter.y) * scaleFactor) ||
            !LM_PointValid(polygonCenter.x + ((double)bounds.x1 - polygonCenter.x) * scaleFactor, polygonCenter.y + ((double)bounds.y1 - polygonCenter.y) * scaleFactor)) {
            LDialog_AlertBox("Invalid scale or result outside layout limits."); return;
        }
        LShapeType selectedShapeType = LObject_GetShape(selectedObject);
		// Function for selectedObject exmple get the left xcoord
		switch (selectedShapeType)
		{
		case 0: // LBox
        {
            LRect rect = LBox_GetRect(selectedObject);
            LBox_New(Cell_Now, LLayer_Now,
                (long)(polygonCenter.x + (rect.x0 - polygonCenter.x) * scaleFactor),
                (long)(polygonCenter.y + (rect.y0 - polygonCenter.y) * scaleFactor),
                (long)(polygonCenter.x + (rect.x1 - polygonCenter.x) * scaleFactor),
                (long)(polygonCenter.y + (rect.y1 - polygonCenter.y) * scaleFactor));
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
			if (r_Scaled > 0) LM_NewCircle(Cell_Now, LLayer_Now, newPoint, r_Scaled);
            break;
		}
		case 3: // LPolygon
		{
			LVertex vertex = LObject_GetVertexList(selectedObject);
			long cnt = LVertex_GetCount(selectedObject);
			if (cnt < 3 || cnt > LM_MAX_POINTS || LPolygon_HasCurve(selectedObject)) return;
            LPoint *points;
			points = (LPoint *)malloc((cnt) * sizeof(LPoint));
			if (!points) return;
            int j = 0;
			while (vertex != NULL && j < cnt)
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
			LM_NewPolygon(Cell_Now, LLayer_Now, points, j);
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

				LTorus_CreateNew(Cell_Now, LLayer_Now, &_tParams);
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

				LPie_CreateNew(Cell_Now, LLayer_Now, &_pParams);
			}
            break;
		}
		default:
			break;
		}
	}

	void Scale_SelectedObject_SetCenter()
	{
        if (!LM_Context()) return;
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[3] = {{"Scale Factor (>0)", "1"},
									   {"Center Coord X (um)", "0"},
									   {"Center Coord Y (um)", "0"}};
		double scaleFactor;
		double center_x, center_y;
		if (LM_Dialog("Scale Selected Object", Dialog_Items, 3))
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
			LPoint polygonCenter = LPoint_Set((long)(center_x * LM_UnitsPerMicron()), (long)(center_y * LM_UnitsPerMicron()));
			Scale_Object(selectedObject, polygonCenter, scaleFactor);
			selectedInital = LSelection_GetNext(selectedInital);
		}
		LDisplay_Refresh();
	}

	void Scale_SelectedObject_ByObjectCenter()
	{
        if (!LM_Context()) return;
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[1] = {{"Scale Factor (>0)", "1"}};
		double scaleFactor;
		if (LM_Dialog("Scale Selected Object", Dialog_Items, 1))
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
        if (!LM_Context()) return;
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[2] = {{"Radius Change (um) (±Inf)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long radiusChange;
		int keep;
		if (LM_Dialog("Modifiy Selected Object", Dialog_Items, 2))
		{
			radiusChange = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the radius_offset
			keep = (int)atof(Dialog_Items[1].value);						   // get the keep
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
        LSelection selection = LSelection_GetList();
        while (selection) {
            LObject selectedObject = LSelection_GetObject(selection);
            selection = LSelection_GetNext(selection);
            if (LObject_GetShape(selectedObject) == LCircle) {
                long target = LCircle_GetRadius(selectedObject) + radiusChange;
                LPoint center = LCircle_GetCenter(selectedObject);
                if (target <= 0 || !LM_PointValid((double)center.x + target, (double)center.y + target) || !LM_PointValid((double)center.x - target, (double)center.y - target)) {
                    LDialog_AlertBox("Invalid resulting radius; circle skipped."); continue;
                }
                if (keep) LM_NewCircle(Cell_Now, LObject_GetLayer(Cell_Now, selectedObject), center, target);
                else LCircle_Set(Cell_Now, selectedObject, center, target);
            }
        }
        LDisplay_Refresh();
}

	bool Compare_Objects(LObject object1, LObject object2)
	{
        if (!object1 || !object2 || LObject_GetLayer(LCell_GetVisible(), object1) != LObject_GetLayer(LCell_GetVisible(), object2) || LObject_GetGDSIIDataType(object1) != LObject_GetGDSIIDataType(object2)) return false;
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
            if (LPolygon_HasCurve(object1) || LPolygon_HasCurve(object2)) return false;
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

	void Modifiy_Delete_Duplicates()
	{
        LSelection selection;
        LObject *objects;
        int n = 0, i, j;
        if (!LM_Context()) return;
        selection = LSelection_GetList();
        while (selection) { ++n; selection = LSelection_GetNext(selection); }
        if (n < 2) return;
        if (n > 5000) { LDialog_AlertBox("Select at most 5000 objects for duplicate removal."); return; }
        objects = (LObject *)malloc(n * sizeof(LObject));
        if (!objects) return;
        selection = LSelection_GetList();
        for (i = 0; i < n; ++i) { objects[i] = LSelection_GetObject(selection); selection = LSelection_GetNext(selection); }
        for (i = 0; i < n; ++i) {
            if (!objects[i]) continue;
            for (j = i + 1; j < n; ++j) {
                if (objects[j] && Compare_Objects(objects[i], objects[j])) {
                    if (LObject_Delete(LCell_GetVisible(), objects[j]) == LStatusOK) objects[j] = NULL;
                }
            }
        }
        free(objects);
        LDisplay_Refresh();
    }

	void Modifiy_Selected_Object_UnionSize()
	{
        if (!LM_Context()) return;
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
		LDialogItem Dialog_Items[2] = {{"Target radius (um)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long tar_radius;
		int keep;
		if (LM_Dialog("Modifiy Selected Object", Dialog_Items, 2))
		{
			tar_radius = (long)(atof(Dialog_Items[0].value) * LM_UnitsPerMicron()); // get the tar_radius
			keep = (int)atof(Dialog_Items[1].value);						 // get the keep
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
        LSelection selection = LSelection_GetList();
        while (selection) {
            LObject selectedObject = LSelection_GetObject(selection);
            selection = LSelection_GetNext(selection);
            if (LObject_GetShape(selectedObject) == LCircle) {
                long target = tar_radius;
                LPoint center = LCircle_GetCenter(selectedObject);
                if (target <= 0 || !LM_PointValid((double)center.x + target, (double)center.y + target) || !LM_PointValid((double)center.x - target, (double)center.y - target)) {
                    LDialog_AlertBox("Invalid resulting radius; circle skipped."); continue;
                }
                if (keep) LM_NewCircle(Cell_Now, LObject_GetLayer(Cell_Now, selectedObject), center, target);
                else LCircle_Set(Cell_Now, selectedObject, center, target);
            }
        }
        LDisplay_Refresh();
}

	void Modifiy_Delete_Objects_ByType()
	{
        if (!LM_Context()) return;
		LCell Cell_Now = LCell_GetVisible();
		//****************************Input Params****************************//
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
		char *Pick_List[] = {
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

		//****************************Input Params****************************//
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LSelection nextSelection = LSelection_GetNext(selectedInital);
			LObject selectedObject = LSelection_GetObject(selectedInital);

			if (selectedObject != NULL && LObject_GetShape(selectedObject) == (LShapeType)Picked)
			{
				LObject_Delete(Cell_Now, selectedObject);
			}

			selectedInital = nextSelection;
		}

		LDisplay_Refresh();
	}

	void TestFunc()
	{
        if (!LM_Context()) return;
		LCell Cell_Now = LCell_GetVisible();
		LFile File_Now = LCell_GetFile(Cell_Now);
		LPoint zero = LPoint_Set(0, 0);
		LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
			LCoord distance = LObject_DistanceToPoint(selectedObject, zero, File_Now);
			LCoord a = WORLD_MAX + distance;
			double c = 1.0 * distance;
			LDialog_AlertBox(LFormat("distance: %ld, %ld, %ld, %f", distance, WORLD_MAX, a, c));
			selectedInital = LSelection_GetNext(selectedInital);
		}
	}

	void Modifiy_func(void)
	{
		LMacro_Register("Modifiy_Selected_Object_UnionSize_func", "Modifiy_Selected_Object_UnionSize");
		LMacro_Register("Modifiy_Selected_Object_Offset", "Modifiy_Selected_Object_Offset");
		LMacro_Register("Modifiy_Delete_Duplicates_func", "Modifiy_Delete_Duplicates");
		LMacro_Register("Modifiy_Delete_Objects_ByType_func", "Modifiy_Delete_Objects_ByType");
		LMacro_Register("Route_Circle_func", "Route_Circle");
		LMacro_Register("Scale_SelectedObject_ByObjectCenter_func", "Scale_SelectedObject_ByObjectCenter");
		LMacro_Register("Scale_SelectedObject_SetCenter_func", "Scale_SelectedObject_SetCenter");
		LMacro_Register("TestFunc_func", "TestFunc");
	}
} /* end of module Array_Module */

Modifiy_func();
