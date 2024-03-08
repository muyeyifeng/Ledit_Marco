/*
 * Ledit_Special_Module.h
 * Ledit_Special_Module.h for Ledit_Special_Module.c
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

int CalculateIntersection_Circle2Circle(LObject object1, LObject object2, long radius, LObject **torus);
int CalculateIntersection_Circle2Wire(LObject object1, LObject object2, long radius, LObject **torus);
double CalculatePolarAngle(LPoint p1, LPoint p2);
void DrawTangentRing();
void FindLineEquation(LPoint p1, LPoint p2, double **abc);
int LSelection_GetNumber(LSelection selectedInital);