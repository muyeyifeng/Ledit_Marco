#ifndef LEDIT_POLYGON_CIRCLE_ROUTE_H
#define LEDIT_POLYGON_CIRCLE_ROUTE_H

typedef enum
{
    CircleRouteOnBoundary,
    CircleRouteInside,
    CircleRouteOutside
} CircleRouteSide;

double CircleRouteArea(const LayoutOutline *outline)
{
    double area = 0;
    int i;
    for (i = 1; i + 1 < outline->count; i++)
        area += OutlineCross(OutlineSubtract(outline->points[i], outline->points[0]),
                             OutlineSubtract(outline->points[i + 1], outline->points[0]));
    return area / 2;
}

DPoint CircleRouteUnit(DPoint vector)
{
    double length = hypot(vector.x, vector.y);
    return OutlinePoint(vector.x / length, vector.y / length);
}

int BuildCircleCenterRoute(const LayoutOutline *edge, LayoutOutline *route,
                            CircleRouteSide side, LCoord radius, double gridError)
{
    double area = CircleRouteArea(edge);
    if (edge->count < 3 || fabs(area) < 1e-8)
        return 0;
    double offset = side == CircleRouteOnBoundary ? 0 : (double)radius;
    if ((area < 0) != (side == CircleRouteOutside))
        offset = -offset;

    int i;
    for (i = 0; i < edge->count; i++)
    {
        DPoint p = edge->points[i];
        if (offset == 0)
        {
            if (!OutlineAppend(route, p))
                return 0;
            continue;
        }
        DPoint incoming = CircleRouteUnit(OutlineSubtract(p, edge->points[(i + edge->count - 1) % edge->count]));
        DPoint outgoing = CircleRouteUnit(OutlineSubtract(edge->points[(i + 1) % edge->count], p));
        double cross = OutlineCross(incoming, outgoing);
        double dot = OutlineDot(incoming, outgoing);
        if (fabs(cross) < 1e-10 && dot > 0)
        {
            if (!OutlineAppend(route, OutlinePoint(p.x - incoming.y * offset, p.y + incoming.x * offset)))
                return 0;
        }
        else if (cross * offset < 0)
        {
            /* The center slides around an exposed vertex at one radius away. */
            double start = atan2(incoming.x * offset, -incoming.y * offset);
            if (!OutlineAppendArc(route, p, fabs(offset), start, atan2(cross, dot), 0, gridError, 1))
                return 0;
        }
        else if (1 + dot > 1e-8)
        {
            /* On an inside join, intersect the two parallel center lines. */
            double scale = offset / (1 + dot);
            if (!OutlineAppend(route, OutlinePoint(p.x - (incoming.y + outgoing.y) * scale,
                                                    p.y + (incoming.x + outgoing.x) * scale)))
                return 0;
        }
        else
            return 0;
    }
    CloseLayoutOutline(route);
    return route->count >= 3;
}

double LocateCircleRouteStart(const LayoutOutline *route, double *length)
{
    double bottom = route->points[0].y, top = bottom;
    int i;
    for (i = 1; i < route->count; i++)
    {
        bottom = fmin(bottom, route->points[i].y);
        top = fmax(top, route->points[i].y);
    }

    double midY = (bottom + top) / 2;
    double right = -HUGE_VAL, start = 0;
    *length = 0;
    for (i = 0; i < route->count; i++)
    {
        DPoint a = route->points[i], b = route->points[(i + 1) % route->count];
        double span = OutlineDistance(a, b);
        /* Anchor the first circle at the rightmost middle-line crossing. */
        if ((a.y > midY) != (b.y > midY))
        {
            double fraction = (midY - a.y) / (b.y - a.y);
            double x = a.x + fraction * (b.x - a.x);
            if (x > right)
            {
                right = x;
                start = *length + fraction * span;
            }
        }
        *length += span;
    }
    return start;
}

int SnapCircleRouteCenter(LFile file, LObject polygon, DPoint center, CircleRouteSide side,
                          LCoord radius, LPoint *result)
{
    if (!isfinite(center.x) || !isfinite(center.y) ||
        fabs(center.x) + radius > WORLD_MAX || fabs(center.y) + radius > WORLD_MAX)
        return 0;
    if (side == CircleRouteOnBoundary)
    {
        *result = LPoint_Set((LCoord)round(center.x), (LCoord)round(center.y));
        return 1;
    }

    /* Snap toward the side that keeps the entire circle clear of the boundary. */
    double best = HUGE_VAL;
    int x, y;
    for (x = 0; x < 2; x++)
    {
        for (y = 0; y < 2; y++)
        {
            DPoint snapped = OutlinePoint(x ? ceil(center.x) : floor(center.x),
                                          y ? ceil(center.y) : floor(center.y));
            double movement = OutlineDistance(center, snapped);
            if (movement >= best || fabs(snapped.x) + radius > WORLD_MAX ||
                fabs(snapped.y) + radius > WORLD_MAX)
                continue;
            LPoint point = LPoint_Set((LCoord)snapped.x, (LCoord)snapped.y);
            double clearance = LObject_DistanceToPoint(polygon, point, file);
            if (side == CircleRouteInside)
                clearance = -clearance;
            if (clearance >= radius && clearance <= (double)radius + 2)
            {
                *result = point;
                best = movement;
            }
        }
    }
    return best < HUGE_VAL;
}

int PlaceCirclesOnCenterRoute(LCell cell, LFile file, LLayer layer, LObject polygon,
                              const LayoutOutline *route, LCoord radius, LCoord spacing,
                              CircleRouteSide side, int *skipped)
{
    double length;
    double start = LocateCircleRouteStart(route, &length);
    if (length <= 0)
        return 0;
    double desired = floor(length / spacing);
    if (desired > INT_MAX)
        return 0;
    int count = (int)fmax(1, desired);
    if (count >= 4)
        count -= count % 4;
    double interval = length / count;

    int i, edge = 0, created = 0;
    double edgeStart = 0, distance = start;
    for (i = 0; i < count; i++, distance += interval)
    {
        if (distance >= length)
        {
            distance -= length;
            edge = 0;
            edgeStart = 0;
        }

        while (edge < route->count)
        {
            DPoint a = route->points[edge], b = route->points[(edge + 1) % route->count];
            double span = OutlineDistance(a, b);
            if (span > 0 && (distance <= edgeStart + span || edge == route->count - 1))
            {
                double fraction = fmax(0, fmin(1, (distance - edgeStart) / span));
                DPoint center = OutlinePoint(a.x + fraction * (b.x - a.x), a.y + fraction * (b.y - a.y));
                LPoint snapped;
                if (SnapCircleRouteCenter(file, polygon, center, side, radius, &snapped) &&
                    LCircle_New(cell, layer, snapped, radius) != NULL)
                    created++;
                else
                    (*skipped)++;
                break;
            }
            edgeStart += span;
            edge++;
        }
    }
    return created;
}

#endif
