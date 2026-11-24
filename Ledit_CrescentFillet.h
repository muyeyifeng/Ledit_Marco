#ifndef LEDIT_CRESCENT_FILLET_H
#define LEDIT_CRESCENT_FILLET_H

typedef struct
{
    DPoint center;
    double radius;
    double orientation;
} FilletBoundaryCircle;

/* The two boundary chains must each follow a circle on the design grid. */
int FitFilletBoundaryCircle(const LayoutOutline *outline, int first, int last,
                            FilletBoundaryCircle *circle)
{
    int count = outline->count;
    int span = (last - first + count) % count;
    if (span < 2)
        return 0;

    DPoint origin = outline->points[first];
    DPoint middle = OutlineSubtract(outline->points[(first + span / 2) % count], origin);
    DPoint end = OutlineSubtract(outline->points[last], origin);
    double cross = OutlineCross(middle, end);
    if (fabs(cross) < 1e-8)
        return 0;
    double middleSquared = OutlineDot(middle, middle);
    double endSquared = OutlineDot(end, end);
    DPoint relative = OutlinePoint((middleSquared * end.y - endSquared * middle.y) / (2 * cross),
                                   (middle.x * endSquared - end.x * middleSquared) / (2 * cross));
    circle->center = OutlinePoint(origin.x + relative.x, origin.y + relative.y);
    circle->radius = hypot(relative.x, relative.y);
    circle->orientation = cross;

    int i;
    for (i = 1; i < span; i++)
    {
        double distance = OutlineDistance(outline->points[(first + i) % count], circle->center);
        if (fabs(distance - circle->radius) > 2.5)
            return 0;
    }
    return 1;
}

int ReadCrescentBoundary(const LayoutOutline *input,
                         FilletBoundaryCircle *outer, FilletBoundaryCircle *inner)
{
    int first = -1, second = -1, i;
    double firstTurn = M_PI / 3, secondTurn = M_PI / 3;
    double area = 0;
    for (i = 0; i < input->count; i++)
    {
        DPoint current = OutlineSubtract(input->points[i], input->points[0]);
        DPoint next = OutlineSubtract(input->points[(i + 1) % input->count], input->points[0]);
        area += OutlineCross(current, next);
    }
    double direction = area > 0 ? 1 : -1;
    for (i = 0; i < input->count; i++)
    {
        double turn = direction * FilletTurnAt(input, i);
        if (turn > firstTurn)
        {
            second = first;
            secondTurn = firstTurn;
            first = i;
            firstTurn = turn;
        }
        else if (turn > secondTurn)
        {
            second = i;
            secondTurn = turn;
        }
    }

    if (second < 0 || !FitFilletBoundaryCircle(input, first, second, outer) ||
        !FitFilletBoundaryCircle(input, second, first, inner) ||
        outer->orientation * inner->orientation >= 0)
        return 0;
    if (outer->orientation * direction < 0)
    {
        FilletBoundaryCircle swap = *outer;
        *outer = *inner;
        *inner = swap;
    }

    double distance = OutlineDistance(outer->center, inner->center);
    return distance > fabs(outer->radius - inner->radius) && distance < outer->radius + inner->radius;
}

double FilletDirectionAngle(DPoint center, DPoint toward)
{
    return atan2(toward.y - center.y, toward.x - center.x);
}

int AppendCrescentArc(LayoutOutline *outline, DPoint center, double radius,
                      double first, double last, double step, int ccw)
{
    return OutlineAppendArc(outline, center, radius, first,
                            OutlineSignedSweep(first, last, ccw), step, 0, 1);
}

/* An exact fillet center lies at the intersection of the two offset circles. */
int BuildCrescentFillets(LayoutOutline *output, FilletBoundaryCircle outer,
                         FilletBoundaryCircle inner, double radius, double step)
{
    DPoint axis = OutlineSubtract(inner.center, outer.center);
    double separation = hypot(axis.x, axis.y);
    axis.x /= separation;
    axis.y /= separation;
    double outerOffset = outer.radius - radius;
    double innerOffset = inner.radius + radius;
    double x = (separation * separation + (outerOffset - innerOffset) *
                (outerOffset + innerOffset)) / (2 * separation);
    double heightSquared = (outerOffset - x) * (outerOffset + x);
    if (outerOffset <= 0 || heightSquared <= 0)
        return 0;

    double dy = sqrt(heightSquared);
    DPoint mid = OutlinePoint(outer.center.x + x * axis.x, outer.center.y + x * axis.y);
    DPoint top = OutlinePoint(mid.x - dy * axis.y, mid.y + dy * axis.x);
    DPoint bottom = OutlinePoint(mid.x + dy * axis.y, mid.y - dy * axis.x);
    double outerTop = FilletDirectionAngle(outer.center, top);
    double outerBottom = FilletDirectionAngle(outer.center, bottom);
    double innerTop = FilletDirectionAngle(inner.center, top);
    double innerBottom = FilletDirectionAngle(inner.center, bottom);

    output->count = 0;
    output->twiceArcArea = 0;
    return AppendCrescentArc(output, outer.center, outer.radius, outerTop, outerBottom, step, 1) &&
           AppendCrescentArc(output, bottom, radius, outerBottom, innerBottom + M_PI, step, 1) &&
           AppendCrescentArc(output, inner.center, inner.radius, innerBottom, innerTop, step, 0) &&
           AppendCrescentArc(output, top, radius, innerTop + M_PI, outerTop, step, 1);
}

double RoundCircularCrescent(const LayoutOutline *source, LayoutOutline *output,
                             double requested, double minStep, double step)
{
    FilletBoundaryCircle outer, inner;
    if (!ReadCrescentBoundary(source, &outer, &inner))
        return 0;

    double separation = OutlineDistance(outer.center, inner.center);
    double limit = (separation + outer.radius - inner.radius) / 2;
    double radius = fmin(requested, limit);
    int attempt;
    for (attempt = 0; attempt < 8 && radius >= step; attempt++, radius -= step / 8)
    {
        if (BuildCrescentFillets(output, outer, inner, radius, step))
        {
            CloseLayoutOutline(output);
            if (ValidateRoundedOutline(output, minStep, 0))
                return radius;
        }
    }
    return 0;
}

#endif
