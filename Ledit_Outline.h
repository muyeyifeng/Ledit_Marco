#ifndef LEDIT_OUTLINE_H
#define LEDIT_OUTLINE_H

typedef struct
{
    DPoint *points;
    int count;
    int capacity;
    unsigned char *smooth;
    double twiceArcArea;
} LayoutOutline;

DPoint OutlinePoint(double x, double y)
{
    DPoint point;
    point.x = x;
    point.y = y;
    return point;
}

DPoint OutlineSubtract(DPoint a, DPoint b)
{
    return OutlinePoint(a.x - b.x, a.y - b.y);
}

double OutlineCross(DPoint a, DPoint b)
{
    return a.x * b.y - a.y * b.x;
}

double OutlineDot(DPoint a, DPoint b)
{
    return a.x * b.x + a.y * b.y;
}

double OutlineDistance(DPoint a, DPoint b)
{
    return hypot(a.x - b.x, a.y - b.y);
}

int OutlineAppend(LayoutOutline *outline, DPoint point)
{
    if (outline->count > 0 &&
        OutlineDistance(outline->points[outline->count - 1], point) < 1e-7)
        return 1;
    if (outline->count == outline->capacity)
    {
        if (outline->capacity > INT_MAX / 2)
            return 0;
        int capacity = outline->capacity ? 2 * outline->capacity : 32;
        DPoint *points = realloc(outline->points, capacity * sizeof(*points));
        if (points == NULL)
            return 0;
        outline->points = points;
        unsigned char *smooth = realloc(outline->smooth, capacity * sizeof(*smooth));
        if (smooth == NULL)
            return 0;
        outline->smooth = smooth;
        outline->capacity = capacity;
    }
    outline->smooth[outline->count] = 0;
    outline->points[outline->count++] = point;
    return 1;
}

DPoint OutlineArcPoint(DPoint center, double radius, double angle)
{
    return OutlinePoint(center.x + radius * cos(angle), center.y + radius * sin(angle));
}

double OutlineSignedSweep(double start, double stop, int counterclockwise)
{
    double sweep = fmod(stop - start, 2 * M_PI);
    if (counterclockwise && sweep <= 0)
        sweep += 2 * M_PI;
    if (!counterclockwise && sweep >= 0)
        sweep -= 2 * M_PI;
    return sweep;
}

/* Use a minimum chord for rounding and a maximum sagitta for circle measurement. */
int OutlineArcSegments(double radius, double sweep, double minStep, double sagitta)
{
    if (radius <= 0 || fabs(sweep) < 1e-9)
        return 0;

    double segments;
    if (minStep > 0)
    {
        if (2 * radius < minStep)
            return 0;
        double minAngle = 2 * asin(minStep / (2 * radius));
        segments = floor(fabs(sweep) / minAngle);
    }
    else
    {
        double maxAngle = 4 * asin(sqrt(fmin(1.0, sagitta / (2 * radius))));
        segments = ceil(fabs(sweep) / fmin(M_PI / 8, maxAngle));
    }
    return segments > 0 && segments < INT_MAX ? (int)segments : 0;
}

int OutlineAppendArc(LayoutOutline *outline, DPoint center, double radius,
                     double start, double sweep, double minStep, double sagitta,
                     int endpoints)
{
    int segments = OutlineArcSegments(radius, sweep, minStep, sagitta);
    if (segments == 0)
        return 0;
    outline->twiceArcArea += radius * radius * (sweep - segments * sin(sweep / segments));
    int closed = endpoints && fabs(fabs(sweep) - 2 * M_PI) < 1e-9;
    int i;
    for (i = !endpoints; i < segments + endpoints - closed; i++)
    {
        double angle = start + sweep * ((double)i / segments);
        int previousCount = outline->count;
        if (!OutlineAppend(outline, OutlineArcPoint(center, radius, angle)))
            return 0;
        if (i > 0 && i < segments && outline->count > previousCount)
            outline->smooth[outline->count - 1] = 1;
    }
    return 1;
}

double OutlineSourceArcStep(double radius, double sweep, double minStep, double cornerSpacing)
{
    if (minStep <= 0 || cornerSpacing <= minStep)
        return minStep;
    double segments = ceil(fabs(sweep) / (M_PI / 6));
    if (segments <= 0)
        return minStep;
    double chord = 2 * radius * sin(fabs(sweep) / (2 * segments));
    return fmax(minStep, fmin(cornerSpacing, chord * (1 - 1e-9)));
}

int ReadLayoutOutline(LObject object, LayoutOutline *outline, double minStep, double sagitta, double cornerSpacing)
{
    LShapeType shape = LObject_GetShape(object);
    switch (shape)
    {
    case LBox:
    {
        LRect box = LBox_GetRect(object);
        return OutlineAppend(outline, OutlinePoint(box.x0, box.y0)) &&
               OutlineAppend(outline, OutlinePoint(box.x1, box.y0)) &&
               OutlineAppend(outline, OutlinePoint(box.x1, box.y1)) &&
               OutlineAppend(outline, OutlinePoint(box.x0, box.y1));
    }
    case LCircle:
    {
        LPoint center = LCircle_GetCenter(object);
        return OutlineAppendArc(outline, OutlinePoint(center.x, center.y), LCircle_GetRadius(object),
                                0, 2 * M_PI, minStep, sagitta, 1);
    }
    case LPolygon:
    {
        LVertex vertex;
        for (vertex = LObject_GetVertexList(object); vertex != NULL; vertex = LVertex_GetNext(vertex))
        {
            LPoint point = LVertex_GetPoint(vertex);
            if (!OutlineAppend(outline, OutlinePoint(point.x, point.y)))
                return 0;
            if (LVertex_HasCurve(vertex))
            {
                LPoint center, start, end;
                LCoord radius;
                LArcDirection direction;
                if (LVertex_GetCurveEX(object, vertex, &center, &radius, &start, &end, &direction) != LStatusOK)
                    return 0;
                if (direction == INVALID)
                    return 0;
                LArcDirection exactDirection = direction;
                DPoint exactCenter = LVertex_GetCurveExactCenter(object, vertex, radius, &exactDirection);
                if (exactDirection == INVALID)
                    exactCenter = OutlinePoint(center.x, center.y);
                double startAngle = atan2(start.y - exactCenter.y, start.x - exactCenter.x);
                double stopAngle = atan2(end.y - exactCenter.y, end.x - exactCenter.x);
                double sweep = OutlineSignedSweep(startAngle, stopAngle, direction == CCW);
                double arcRadius = OutlineDistance(exactCenter, OutlinePoint(start.x, start.y));
                double step = OutlineSourceArcStep(arcRadius, sweep, minStep, cornerSpacing);
                /* A sub-step input arc contributes its endpoints to rounding. */
                if (minStep > 0 && arcRadius * fabs(sweep) < minStep)
                    continue;
                if (!OutlineAppendArc(outline, exactCenter, arcRadius, startAngle, sweep, step, sagitta, 0))
                    return 0;
            }
        }
        return 1;
    }
    case LPie:
    case LTorus:
    {
        LPoint center;
        double start, stop, outer, inner = 0;
        if (shape == LPie)
        {
            LPieParams pie;
            if (LPie_GetParams(object, &pie) != LStatusOK)
                return 0;
            center = pie.ptCenter;
            start = pie.dStartAngle;
            stop = pie.dStopAngle;
            outer = pie.nRadius;
        }
        else
        {
            LTorusParams torus;
            if (LTorus_GetParams(object, &torus) != LStatusOK)
                return 0;
            center = torus.ptCenter;
            start = torus.dStartAngle;
            stop = torus.dStopAngle;
            outer = torus.nOuterRadius;
            inner = torus.nInnerRadius;
        }
        start *= M_PI / 180;
        stop *= M_PI / 180;
        double sweep = OutlineSignedSweep(start, stop, 1);
        int full = sweep > 2 * M_PI - 1e-8 || sweep < 1e-8;
        if (full && inner > 0)
            return 0; /* A full annulus has a hole. */
        if (full)
            sweep = 2 * M_PI;
        DPoint origin = OutlinePoint(center.x, center.y);
        double outerStep = OutlineSourceArcStep(outer, sweep, minStep, cornerSpacing);
        if (!OutlineAppendArc(outline, origin, outer, start, sweep, outerStep, sagitta, 1))
            return 0;
        if (full)
            return 1;
        if (inner <= 0)
            return OutlineAppend(outline, origin);
        double innerStep = OutlineSourceArcStep(inner, sweep, minStep, cornerSpacing);
        return OutlineAppendArc(outline, origin, inner, start + sweep, -sweep, innerStep, sagitta, 1);
    }
    default:
        return 0;
    }
}

int ReadShapeOutline(LCell cell, LLayer layer, LObject object, LayoutOutline *outline,
                     double minStep, double sagitta, double cornerSpacing)
{
    if (LObject_GetShape(object) != LWire)
        return ReadLayoutOutline(object, outline, minStep, sagitta, cornerSpacing);

#ifndef EXCLUDE_LEDIT_LEGACY_UPI
    /* The conversion can replace its input. Give it a copy on the writable layer. */
    LObject copy = LObject_Copy(cell, layer, object);
    if (copy == NULL || copy == object)
        return 0;
    LObject polygon = LPolygon_WireToPolygon(cell, layer, copy);
    int readable = polygon != NULL && polygon != object && LObject_GetShape(polygon) == LPolygon &&
                   ReadLayoutOutline(polygon, outline, minStep, sagitta, cornerSpacing);

    LObject candidate = LObject_GetList(cell, layer);
    while (candidate != NULL)
    {
        if (candidate == copy)
            break;
        candidate = LObject_GetNext(candidate);
    }
    if (polygon != NULL && polygon != object && polygon != candidate &&
        LObject_Delete(cell, polygon) != LStatusOK)
        readable = 0;
    if (candidate != NULL && LObject_Delete(cell, candidate) != LStatusOK)
        readable = 0;
    return readable;
#else
    return 0;
#endif
}

void FreeLayoutOutline(LayoutOutline *outline)
{
    free(outline->points);
    free(outline->smooth);
}

void CloseLayoutOutline(LayoutOutline *outline)
{
    if (outline->count > 1 &&
        OutlineDistance(outline->points[0], outline->points[outline->count - 1]) < 1e-7)
        outline->count--;
}

double OutlineArea(const LayoutOutline *outline)
{
    double twiceArea = 0;
    int i;
    for (i = 1; i + 1 < outline->count; i++)
        twiceArea += OutlineCross(OutlineSubtract(outline->points[i], outline->points[0]),
                                  OutlineSubtract(outline->points[i + 1], outline->points[0]));
    return fabs(twiceArea + outline->twiceArcArea) / 2;
}

/* Dialog distances are in microns. Keep conversions within the layout coordinate range. */
int ReadLayoutDistance(LFile file, const char *value, LCoord *distance)
{
    char *end;
    double microns = strtod(value, &end);
    if (end == value)
        return 0;
    while (*end == ' ' || *end == '\t')
        end++;
    if (*end != '\0' || !isfinite(microns) || microns < 0 ||
        microns > LFile_IntUtoMicrons(file, WORLD_MAX))
        return 0;
    *distance = LFile_MicronsToIntU(file, microns);
    return 1;
}

#endif
