/*
 * ShapeCenter_Module.h
 * ShapeCenter_Module.h for ShapeCenter_Module.c
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

void swap(LPoint * a, LPoint * b);
double distance(LPoint p1, LPoint p2);
int compare(const void *a, const void *b);
LPoint CalculateCentroid(LPoint * points, int n);
int LSelection_GetNumber(LSelection selectedInital);
bool CheckIsCircles(LSelection selectedInital, LPoint * centerPoints);
void DrawCircleAtShapeCenter();