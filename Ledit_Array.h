/*
 * Array_Module.h
 * Array_Module.h for Array_Module.c
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
#include "ldata.h"

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
void _ArrayInObjectByDistanceHexagon(LPoint **points, int *num, long *_distance);
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