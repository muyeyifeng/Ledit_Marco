/*
 * Polygon_Module.h
 * Polygon_Module.h for Polygon_Module.c
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

void DrawEllipse();
LPoint *CalculateEllipsePoints(long a, long b, int n, double rad);
void DrawPolygonByCenterRadiusAndEdgeNumber(LPoint center, long radius, int edgeNum, double rad);
void DrawPolygonByEdgeLenght();
void DrawPolygonByRadius();
void Polygon_func(void);