/*
 * Alignment_Module.h
 * Alignment_Module.h for Alignment_Module.c
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

void Alignment_Left();
void Alignment_Right();
void Alignment_Bottom();
void Alignment_Top();
void Alignment_Center();
void MoveObjectListCenter(long *xLeftTarget, long *xRightTarget, long *yBottomTarget, long *yTopTarget);
void MoveObjectListLeft(long *xLeftTarget);
void MoveObjectListRight(long *xRightTarget);
void MoveObjectListBottom(long *yBottomTarget);
void MoveObjectListTop(long *yTopTarget);
void MoveObject(LObject selectedObject, long xOffset, long yOffset);
void GetObjectListCoordRange(long *xLeft, long *xRight, long *yBottom, long *yTop);
void GetObjectCoord(LObject selectedObject, long *xLeft, long *xRight, long *yBottom, long *yTop);
