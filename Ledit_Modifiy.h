/*
 * Modifiy_Module.h
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

void Route_Circle();
int GetCrossPoint(LObject polygon, LPoint *crossPoint);
int calculateIntersection(LPoint a, LPoint b, LPoint c, LPoint d, LPoint *intersection);
int calculateCircleSegmentIntersection(LPoint a, LPoint b, LPoint center, double radius, LPoint *intersection1, LPoint *intersection2);
double GetPointsDistance(LPoint point1, LPoint point2);
void DrawCircleByRouteDistance(double distance, long radius, int circleNumber, LPoint start, LObject route); // LPolygon
int isClockwise(LPoint a, LPoint b, LPoint c);
void GetObjectCoord(LObject selectedObject, long *xLeft, long *xRight, long *yBottom, long *yTop);
double GetPointsDistance(LPoint point1, LPoint point2);
void Modifiy_Selected_Object_UnionSize();
void Modifiy_Selected_Object_Offset();
void Modifiy_Circle(LObject circle, long target_radius);