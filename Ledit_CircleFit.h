#ifndef LEDIT_CIRCLE_FIT_H
#define LEDIT_CIRCLE_FIT_H

typedef struct
{
    DPoint center;
    double radius;
} LayoutCircle;

typedef struct
{
    long left, bottom, right, top;
} CircleSearchBounds;

LayoutCircle CircleFromDiameter(DPoint a, DPoint b)
{
    LayoutCircle circle;
    circle.center = OutlinePoint((a.x + b.x) / 2, (a.y + b.y) / 2);
    circle.radius = OutlineDistance(a, b) / 2;
    return circle;
}

int CircleContains(LayoutCircle circle, DPoint point)
{
    return OutlineDistance(circle.center, point) <= circle.radius + 1e-7;
}

LayoutCircle CircleThroughTwo(const DPoint *points, int count, DPoint a, DPoint b)
{
    LayoutCircle diameter = CircleFromDiameter(a, b);
    LayoutCircle left = diameter, right = diameter;
    left.radius = right.radius = -1;
    DPoint ab = OutlineSubtract(b, a);
    double ab2 = OutlineDot(ab, ab);
    int i;
    for (i = 0; i < count; i++)
    {
        DPoint c = points[i];
        if (CircleContains(diameter, c))
            continue;

        DPoint ac = OutlineSubtract(c, a);
        double side = OutlineCross(ab, ac);
        if (side == 0)
            continue;
        double ac2 = OutlineDot(ac, ac);
        DPoint relative = OutlinePoint((ac.y * ab2 - ab.y * ac2) / (2 * side),
                                       (ab.x * ac2 - ac.x * ab2) / (2 * side));
        LayoutCircle candidate;
        candidate.center = OutlinePoint(a.x + relative.x, a.y + relative.y);
        candidate.radius = hypot(relative.x, relative.y);
        double offset = OutlineCross(ab, relative);
        if (side > 0 && (left.radius < 0 || offset > OutlineCross(ab, OutlineSubtract(left.center, a))))
            left = candidate;
        if (side < 0 && (right.radius < 0 || offset < OutlineCross(ab, OutlineSubtract(right.center, a))))
            right = candidate;
    }

    if (left.radius < 0 && right.radius < 0)
        return diameter;
    if (right.radius < 0 || (left.radius >= 0 && left.radius <= right.radius))
        return left;
    return right;
}

LayoutCircle FindEnclosingCircle(LayoutOutline *outline)
{
    DPoint *points = outline->points;
    unsigned int seed = 0x93ac84d1U;
    int i, j;
    for (i = outline->count - 1; i > 0; i--)
    {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        j = seed % (i + 1);
        DPoint point = points[i];
        points[i] = points[j];
        points[j] = point;
    }

    LayoutCircle circle = { {0, 0}, -1 };
    for (i = 0; i < outline->count; i++)
    {
        if (CircleContains(circle, points[i]))
            continue;
        circle = CircleFromDiameter(points[i], points[i]);
        for (j = 0; j < i; j++)
        {
            if (!CircleContains(circle, points[j]))
                circle = circle.radius == 0 ? CircleFromDiameter(points[i], points[j]) :
                         CircleThroughTwo(points, j + 1, points[i], points[j]);
        }
    }

    circle.center.x = round(circle.center.x);
    circle.center.y = round(circle.center.y);
    circle.radius = 0;
    for (i = 0; i < outline->count; i++)
        circle.radius = fmax(circle.radius, OutlineDistance(circle.center, points[i]));
    return circle;
}

double SignedOutlineDistance(const LayoutOutline *outline, DPoint point)
{
    int inside = 0;
    double shortestSquared = HUGE_VAL;
    int i;
    for (i = 0; i < outline->count; i++)
    {
        DPoint a = outline->points[i];
        DPoint b = outline->points[(i + 1) % outline->count];
        DPoint edge = OutlineSubtract(b, a);
        DPoint gap = OutlineSubtract(point, a);
        double lengthSquared = OutlineDot(edge, edge);
        if (lengthSquared > 0)
        {
            double t = fmax(0, fmin(1, OutlineDot(edge, gap) / lengthSquared));
            DPoint delta = OutlinePoint(gap.x - t * edge.x, gap.y - t * edge.y);
            shortestSquared = fmin(shortestSquared, OutlineDot(delta, delta));
        }
        if ((a.y > point.y) != (b.y > point.y) &&
            point.x < a.x + (point.y - a.y) * (b.x - a.x) / (b.y - a.y))
            inside = !inside;
    }
    return (inside ? 1 : -1) * sqrt(shortestSquared);
}

/* The signed distance can grow by at most the distance to the search point. */
void SearchInscribedCircle(const LayoutOutline *outline, CircleSearchBounds bounds,
                           LayoutCircle *best, double margin, int *remaining)
{
    if (*remaining <= 0)
        return;
    (*remaining)--;
    long width = bounds.right - bounds.left;
    long height = bounds.top - bounds.bottom;
    long x = bounds.left + width / 2;
    long y = bounds.bottom + height / 2;
    DPoint center = OutlinePoint(x, y);
    double radius = SignedOutlineDistance(outline, center) - margin;
    if (radius > best->radius)
    {
        best->center = center;
        best->radius = radius;
    }

    if (radius + hypot(bounds.right - x, bounds.top - y) < floor(best->radius) + 1 ||
        (width == 0 && height == 0))
        return;

    CircleSearchBounds second = bounds;
    if (width >= height)
    {
        bounds.right = x;
        second.left = x + 1;
    }
    else
    {
        bounds.top = y;
        second.bottom = y + 1;
    }
    SearchInscribedCircle(outline, bounds, best, margin, remaining);
    SearchInscribedCircle(outline, second, best, margin, remaining);
}

LayoutCircle FindInscribedCircle(const LayoutOutline *outline, double margin, int *approximate)
{
    double left = outline->points[0].x, right = left;
    double bottom = outline->points[0].y, top = bottom;
    int i;
    for (i = 1; i < outline->count; i++)
    {
        left = fmin(left, outline->points[i].x);
        right = fmax(right, outline->points[i].x);
        bottom = fmin(bottom, outline->points[i].y);
        top = fmax(top, outline->points[i].y);
    }

    CircleSearchBounds bounds = {(long)ceil(left), (long)ceil(bottom), (long)floor(right), (long)floor(top)};
    LayoutCircle circle = {{0, 0}, 0};
    if (bounds.left <= bounds.right && bounds.bottom <= bounds.top)
    {
        circle.center = OutlinePoint(round((left + right) / 2), round((bottom + top) / 2));
        circle.radius = fmax(0, SignedOutlineDistance(outline, circle.center) - margin);
        double upperBound = fmin(right - left, top - bottom) / 2 - margin;
        if (upperBound >= floor(circle.radius) + 1)
        {
            /* Bound work on long narrow or highly tessellated contours. */
            int remaining = (int)fmax(64, fmin(1000000, 15000000.0 / outline->count));
            SearchInscribedCircle(outline, bounds, &circle, margin, &remaining);
            if (remaining == 0)
                *approximate = 1;
        }
    }
    return circle;
}

#endif
