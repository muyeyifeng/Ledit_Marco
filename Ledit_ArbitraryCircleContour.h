#ifndef LEDIT_ARBITRARY_CIRCLE_CONTOUR_H
#define LEDIT_ARBITRARY_CIRCLE_CONTOUR_H

typedef struct
{
    DPoint center;
    LCoord radius;
} BoundaryDisk;

typedef struct
{
    LObject *objects;
    unsigned int count;
    unsigned int capacity;
} BoundaryPieces;

/* The smoothing circle is externally tangent to both grown disks. */
int FindBoundaryFilletCenters(BoundaryDisk a, BoundaryDisk b, double smoothing,
                              DPoint centers[2], double *height)
{
    DPoint direction = OutlineSubtract(b.center, a.center);
    double distance = hypot(direction.x, direction.y);
    double first = a.radius + smoothing, second = b.radius + smoothing;
    if (distance <= fabs(first - second) || distance >= first + second)
        return 0;

    direction.x /= distance;
    direction.y /= distance;
    double along = (distance * distance + (first - second) * (first + second)) / (2 * distance);
    *height = sqrt(fmax(0, (first - along) * (first + along)));
    DPoint midpoint = OutlinePoint(a.center.x + along * direction.x,
                                    a.center.y + along * direction.y);
    centers[0] = OutlinePoint(midpoint.x - *height * direction.y, midpoint.y + *height * direction.x);
    centers[1] = OutlinePoint(midpoint.x + *height * direction.y, midpoint.y - *height * direction.x);
    return 1;
}

int BoundaryFilletIsExposed(const BoundaryDisk *disks, int count, int first, int second,
                             DPoint center, double smoothing)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (i != first && i != second &&
            OutlineDistance(disks[i].center, center) + 1e-6 < disks[i].radius + smoothing)
            return 0;
    }
    return 1;
}

/* Find a short, forward path through overlapping disks on the interior
   side of a fillet. Each link can serve as a seam hidden inside the union. */
int FindBoundaryBackingPath(const BoundaryDisk *disks, int count, int first, int last,
                            DPoint filletCenter, int *previous, int *queue)
{
    DPoint start = disks[first].center;
    DPoint axis = OutlineSubtract(disks[last].center, start);
    double span = hypot(axis.x, axis.y);
    if (span == 0)
        return 0;
    axis.x /= span;
    axis.y /= span;
    double exterior = OutlineCross(axis, OutlineSubtract(filletCenter, start)) > 0 ? 1 : -1;

    int i, head = 0, tail = 0;
    for (i = 0; i < count; i++)
        previous[i] = -1;
    previous[first] = first;
    queue[tail++] = first;

    while (head < tail)
    {
        int from = queue[head++];
        double progress = OutlineDot(axis, OutlineSubtract(disks[from].center, start));
        for (i = 0; i < count; i++)
        {
            if (previous[i] >= 0)
                continue;
            DPoint relative = OutlineSubtract(disks[i].center, start);
            double along = OutlineDot(axis, relative);
            double across = exterior * OutlineCross(axis, relative);
            if (i != last &&
                (along <= progress + 1e-6 || along >= span - 1e-6 ||
                 across > 1e-6 || across < -span))
                continue;
            double radii = (double)disks[i].radius + disks[from].radius;
            if (OutlineDistance(disks[i].center, disks[from].center) + 1 >= radii)
                continue;

            previous[i] = from;
            if (i == last)
                return 1;
            queue[tail++] = i;
        }
    }
    return 0;
}

int AppendBoundaryBacking(const BoundaryDisk *disks, int first, int last, const int *previous,
                          LayoutOutline *patch)
{
    int i;
    for (i = previous[last]; i != first; i = previous[i])
    {
        if (!OutlineAppend(patch, disks[i].center))
            return 0;
    }
    return 1;
}

int BuildBoundaryFilletPatch(BoundaryDisk a, BoundaryDisk b, DPoint center,
                              double smoothing, LayoutOutline *patch)
{
    patch->count = 0;
    patch->twiceArcArea = 0;
    DPoint first = OutlineSubtract(a.center, center);
    DPoint second = OutlineSubtract(b.center, center);
    double start = atan2(first.y, first.x);
    double sweep = atan2(OutlineCross(first, second), OutlineDot(first, second));
    /* The closing edges lie inside the grown disks or along their centerline.
       The only exposed part of this patch is the concave tangent arc. */
    return OutlineAppend(patch, a.center) &&
           OutlineAppendArc(patch, center, smoothing, start, sweep, 0, 0.25, 1) &&
           OutlineAppend(patch, b.center);
}

int RecordBoundaryPiece(LCell cell, BoundaryPieces *pieces, LObject object)
{
    if (object == NULL)
        return 0;
    if (pieces->count == pieces->capacity)
    {
        unsigned int capacity = pieces->capacity ? pieces->capacity * 2 : 16;
        LObject *objects = capacity > pieces->capacity &&
                           (size_t)capacity <= (size_t)-1 / sizeof(*objects) ?
                           realloc(pieces->objects, (size_t)capacity * sizeof(*objects)) : NULL;
        if (objects == NULL)
        {
            LObject_Delete(cell, object);
            return 0;
        }
        pieces->objects = objects;
        pieces->capacity = capacity;
    }
    pieces->objects[pieces->count++] = object;
    return 1;
}

int WriteBoundaryFilletPatch(LCell cell, LLayer layer, BoundaryPieces *pieces,
                              const LayoutOutline *patch)
{
    LPoint *points = malloc((size_t)patch->count * sizeof(*points));
    if (points == NULL)
        return 0;
    int i, count = 0, valid = 1;
    for (i = 0; i < patch->count; i++)
    {
        DPoint p = patch->points[i];
        double x = round(p.x), y = round(p.y);
        if (fabs(x) > WORLD_MAX || fabs(y) > WORLD_MAX)
        {
            valid = 0;
            break;
        }
        if (count == 0 || points[count - 1].x != x || points[count - 1].y != y)
            points[count++] = LPoint_Set((LCoord)x, (LCoord)y);
    }
    if (valid && count >= 3)
        valid = RecordBoundaryPiece(cell, pieces, LPolygon_New(cell, layer, points, count));
    free(points);
    return valid;
}

int AddBoundaryFilletPatches(LCell cell, LLayer layer, BoundaryPieces *pieces,
                             const BoundaryDisk *disks, int count, LCoord smoothing,
                             int *patches, int *openGaps)
{
    if (smoothing == 0)
        return 1;
    int *path = malloc(2 * (size_t)count * sizeof(*path));
    if (path == NULL)
        return 0;
    int *queue = path + count;
    LayoutOutline patch = {0};
    int i, j, valid = 1;
    for (i = 0; i < count && valid; i++)
    {
        for (j = i + 1; j < count && valid; j++)
        {
            BoundaryDisk a = disks[i], b = disks[j];
            DPoint centers[2];
            double height;
            if (!FindBoundaryFilletCenters(a, b, smoothing, centers, &height))
                continue;

            double separation = OutlineDistance(a.center, b.center);
            int gap = separation >= (double)a.radius + b.radius;
            int exposed[2];
            exposed[0] = BoundaryFilletIsExposed(disks, count, i, j, centers[0], smoothing);
            exposed[1] = BoundaryFilletIsExposed(disks, count, i, j, centers[1], smoothing);
            /* Two free-standing arcs must have room between them. A single
               exposed arc can instead close against a chain of grown disks. */
            int bridge = gap && height > smoothing + 1 && exposed[0] && exposed[1];
            int side, closed = 0;
            for (side = 0; side < 2 && valid; side++)
            {
                if (!exposed[side])
                    continue;
                int backed = gap && !bridge &&
                             FindBoundaryBackingPath(disks, count, i, j, centers[side], path, queue);
                if ((gap && !bridge && !backed) || (!gap && height <= smoothing + 1))
                    continue;

                valid = BuildBoundaryFilletPatch(a, b, centers[side], smoothing, &patch) &&
                        (!backed || AppendBoundaryBacking(disks, i, j, path, &patch)) &&
                        WriteBoundaryFilletPatch(cell, layer, pieces, &patch);
                if (valid)
                {
                    (*patches)++;
                    closed = 1;
                }
            }
            if (gap && !closed && (exposed[0] || exposed[1]))
                (*openGaps)++;
        }
    }
    FreeLayoutOutline(&patch);
    free(path);
    return valid;
}

#endif
