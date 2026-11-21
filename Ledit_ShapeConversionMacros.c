/*
 * Convert layout geometry to other shapes and round polygon corners on the design grid.
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
module ShapeConversionMacros
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "ldata.h"
#include "Ledit_OutputLayer.h"
#include "Ledit_Outline.h"
#include "Ledit_CircleFit.h"
#include "Ledit_Fillet.h"

    enum CircleMode { EqualAreaCircle, InscribedCircle, EnclosingCircle };

    /* Measure each source outline once before applying the requested circle-fitting mode. */
    int MeasureConversionCircle(LCell cell, LLayer layer, LObject object, int mode,
                            LayoutCircle *circle, int *approximate)
    {
        LShapeType shape = LObject_GetShape(object);
        if (shape == LCircle)
        {
            LPoint center = LCircle_GetCenter(object);
            circle->center = OutlinePoint(center.x, center.y);
            circle->radius = LCircle_GetRadius(object);
            return circle->radius > 0;
        }

        int curved = shape == LPie || shape == LTorus || shape == LWire ||
                     (shape == LPolygon && LPolygon_HasCurve(object));
        double sagitta = 0.2;
        double margin = curved ? sagitta : 0;
        LayoutOutline outline = {0};
        int readable = ReadShapeOutline(cell, layer, object, &outline, 0, sagitta, 0);
        CloseLayoutOutline(&outline);
        double area = OutlineArea(&outline);
        if (!readable || outline.count < 3 || area <= 0)
        {
            FreeLayoutOutline(&outline);
            return 0;
        }

        if (mode == InscribedCircle)
        {
            *circle = FindInscribedCircle(&outline, margin, approximate);
            circle->radius = floor(circle->radius + 1e-8);
        }
        else if (mode == EnclosingCircle)
        {
            *circle = FindEnclosingCircle(&outline);
            circle->radius = ceil(circle->radius + margin - 1e-8);
        }
        else
        {
            double crossSum = 0;
            DPoint weighted = OutlinePoint(0, 0);
            DPoint origin = outline.points[0];
            int i;
            for (i = 1; i + 1 < outline.count; i++)
            {
                DPoint a = OutlineSubtract(outline.points[i], origin);
                DPoint b = OutlineSubtract(outline.points[i + 1], origin);
                double cross = OutlineCross(a, b);
                crossSum += cross;
                weighted.x += (a.x + b.x) * cross;
                weighted.y += (a.y + b.y) * cross;
            }
            circle->center = OutlinePoint(round(origin.x + weighted.x / (3 * crossSum)),
                                          round(origin.y + weighted.y / (3 * crossSum)));
            circle->radius = round(sqrt(area / M_PI));
        }

        FreeLayoutOutline(&outline);
        return circle->radius > 0 && circle->radius <= WORLD_MAX &&
               fabs(circle->center.x) + circle->radius <= WORLD_MAX &&
               fabs(circle->center.y) + circle->radius <= WORLD_MAX;
    }

    void ConvertSelectionToCircles(void)
    {
        LCell cell = LCell_GetVisible();
        LSelection selection = LSelection_GetList();
        if (cell == NULL || selection == NULL)
        {
            LDialog_AlertBox("Select shapes to convert to circles.");
            return;
        }

        const char *modes[] = {"Equal area", "Inscribed circle", "Enclosing circle"};
        int mode = LDialog_PickList("Convert to circle", modes, 3, EqualAreaCircle);
        if (mode < EqualAreaCircle || mode > EnclosingCircle)
            return;
        LLayer outputLayer = PickOutputLayer(cell);
        if (outputLayer == NULL)
            return;

        int created = 0, skipped = 0, approximate = 0;
        while (selection != NULL)
        {
            LObject object = LSelection_GetObject(selection);
            selection = LSelection_GetNext(selection);
            LayoutCircle circle;
            if (object != NULL && MeasureConversionCircle(cell, outputLayer, object, mode, &circle, &approximate) &&
                LCircle_New(cell, outputLayer, LPoint_Set((LCoord)circle.center.x, (LCoord)circle.center.y),
                            (LCoord)circle.radius) != NULL)
                created++;
            else
                skipped++;
        }
        LDisplay_Refresh();
        if (skipped || approximate)
            LDialog_AlertBox(LFormat("Created %d circles. Skipped %d unsupported or degenerate shapes.%s",
                                    created, skipped, approximate ? " At least one inscribed circle reached the search limit." : ""));
    }

    /* Keep the old command identifier bound to the current circle conversion dialog. */
    void ConvertSelectionToCirclesLegacy(void)
    {
        ConvertSelectionToCircles();
    }

    /* Use the source radius as the apothem so the resulting polygon circumscribes the circle. */
    void ConvertCirclesToRegularPolygons(void)
    {
        LCell cell = LCell_GetVisible();
        LSelection selection = LSelection_GetList();
        if (cell == NULL || selection == NULL)
            return;

        LDialogItem input[] = {{"Edge number (>=3)", "3"}, {"Rotation (deg)", "0"}};
        if (!LDialog_MultiLineInputBox("Conversion to polygon", input, 2))
            return;
        char *end;
        long sides = strtol(input[0].value, &end, 10);
        if (*end != '\0' || sides < 3 || sides > INT_MAX / (int)sizeof(LPoint))
        {
            LDialog_AlertBox("Enter an integer edge count of at least 3.");
            return;
        }
        double radians = strtod(input[1].value, &end) * M_PI / 180;
        if (*end != '\0' || !isfinite(radians))
        {
            LDialog_AlertBox("Enter a finite rotation angle.");
            return;
        }

        LLayer outputLayer = PickOutputLayer(cell);
        if (outputLayer == NULL)
            return;
        LPoint *points = malloc(sides * sizeof(*points));
        if (points == NULL)
        {
            LDialog_AlertBox("Could not allocate the polygon vertices.");
            return;
        }

        while (selection != NULL)
        {
            LObject object = LSelection_GetObject(selection);
            selection = LSelection_GetNext(selection);
            if (object == NULL || LObject_GetShape(object) != LCircle)
                continue;
            LPoint center = LCircle_GetCenter(object);
            double radius = LCircle_GetRadius(object) / cos(M_PI / sides);
            int i;
            for (i = 0; i < sides; i++)
            {
                double angle = radians + 2 * i * M_PI / sides;
                double x = center.x + radius * sin(angle);
                double y = center.y + radius * cos(angle);
                if (fabs(x) > WORLD_MAX || fabs(y) > WORLD_MAX)
                    break;
                points[i] = LPoint_Set((LCoord)x, (LCoord)y);
            }
            if (i == sides)
                LPolygon_New(cell, outputLayer, points, (int)sides);
        }
        free(points);
        LDisplay_Refresh();
    }

    /* Build a circumscribed hexagon before tessellating its corner fillets. */
    int CreateRoundedHexagon(LCell cell, LLayer outputLayer, LPoint center,
                           double apothem, double radius, double minStep)
    {
        LayoutOutline hexagon = {0}, rounded = {0};
        LayoutFillet corners[6];
        int i, created = 0;
        for (i = 0; i < 6; i++)
        {
            if (!OutlineAppend(&hexagon, OutlineArcPoint(OutlinePoint(center.x, center.y),
                                                        apothem / cos(M_PI / 6), M_PI / 6 + i * M_PI / 3)))
                break;
        }
        int reserveSpan;
        for (reserveSpan = 1; i == 6 && reserveSpan >= 0 && !created; reserveSpan--)
        {
            if (!BuildRoundedOutline(&hexagon, &rounded, corners, radius, minStep + sqrt(2.0), reserveSpan))
                continue;
            CloseLayoutOutline(&rounded);
            if (ValidateRoundedOutline(&rounded, minStep, 0))
            {
                LObject object = WriteRoundedOutline(cell, outputLayer, &rounded);
                if (object != NULL)
                {
                    SaveFilletSource(object, &hexagon, &rounded);
                    created = 1;
                }
            }
        }

        FreeLayoutOutline(&hexagon);
        FreeLayoutOutline(&rounded);
        return created;
    }

    void ConvertCirclesToRoundedHexagons(void)
    {
        LCell cell = LCell_GetVisible();
        LSelection selection = LSelection_GetList();
        if (cell == NULL || selection == NULL)
            return;
        LFile file = LCell_GetFile(cell);
        LSelection first = selection;
        while (first != NULL && LObject_GetShape(LSelection_GetObject(first)) != LCircle)
            first = LSelection_GetNext(first);
        if (first == NULL)
        {
            LDialog_AlertBox("Select at least one circle.");
            return;
        }

        double example = LFile_IntUtoMicrons(file, LCircle_GetRadius(LSelection_GetObject(first))) / 3.7;
        char title[160];
        snprintf(title, sizeof(title), "Rounded hexagon - auto: R / 3.7 (first: %.6g um)", example);
        LDialogItem input[] = {{"Corner radius (um), -1 = automatic", "-1"}};
        if (!LDialog_MultiLineInputBox(title, input, 1))
            return;

        char *end;
        double microns = strtod(input[0].value, &end);
        if (*end != '\0' || !isfinite(microns) || microns == 0 ||
            (microns < 0 && microns != -1) || microns > LFile_IntUtoMicrons(file, WORLD_MAX))
        {
            LDialog_AlertBox("Enter a positive corner radius or -1 for automatic.");
            return;
        }
        LLayer outputLayer = PickOutputLayer(cell);
        if (outputLayer == NULL)
            return;

        double explicitRadius = microns == -1 ? 0 : LFile_MicronsToIntU(file, microns);
        double minStep = RoundedMinimumPointSpacing(file);
        int skipped = 0;
        while (selection != NULL)
        {
            LObject object = LSelection_GetObject(selection);
            selection = LSelection_GetNext(selection);
            if (object == NULL || LObject_GetShape(object) != LCircle)
                continue;
            LPoint center = LCircle_GetCenter(object);
            LCoord size = LCircle_GetRadius(object);
            double radius = microns == -1 ? size / 3.7 : explicitRadius;
            if (radius >= size)
            {
                if (LCircle_New(cell, outputLayer, center, size) == NULL)
                    skipped++;
            }
            else if (!CreateRoundedHexagon(cell, outputLayer, center, size, radius, minStep))
                skipped++;
        }
        LDisplay_Refresh();
        if (skipped)
            LDialog_AlertBox(LFormat("Skipped %d rounded hexagons that could not be created at 10 nm point spacing.", skipped));
    }

    void RegisterShapeConversionMacros(void)
    {
        LMacro_Register("Approximate_Circle_func", "ConvertSelectionToCircles");
        LMacro_Register("Approximate_Circle_Sample_func", "ConvertSelectionToCirclesLegacy");
        LMacro_Register("Conversion_To_Polygon_func", "ConvertCirclesToRegularPolygons");
        LMacro_Register("Conversion_To_Rounded_Hexagon_func", "ConvertCirclesToRoundedHexagons");
        LMacro_Register("Round_Selected_Corners_func", "RoundSelectedCorners");
    }
} /* End of ShapeConversionMacros. */

RegisterShapeConversionMacros();
