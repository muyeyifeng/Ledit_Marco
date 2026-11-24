#ifndef LEDIT_FILLET_H
#define LEDIT_FILLET_H

#define ROUNDED_MAX_TURN (M_PI / 3 + 1e-8)

typedef struct
{
    double angle;
    double desiredTrim;
    double trim;
} LayoutFillet;

double FilletTurn(DPoint a, DPoint b, DPoint c)
{
    DPoint incoming = OutlineSubtract(b, a);
    DPoint outgoing = OutlineSubtract(c, b);
    return atan2(OutlineCross(incoming, outgoing), OutlineDot(incoming, outgoing));
}

double FilletOrientation(DPoint a, DPoint b, DPoint c)
{
    return OutlineCross(OutlineSubtract(b, a), OutlineSubtract(c, a));
}

int FilletSegmentsIntersect(DPoint a, DPoint b, DPoint c, DPoint d)
{
    if (fmax(a.x, b.x) < fmin(c.x, d.x) || fmax(c.x, d.x) < fmin(a.x, b.x) ||
        fmax(a.y, b.y) < fmin(c.y, d.y) || fmax(c.y, d.y) < fmin(a.y, b.y))
        return 0;

    double abc = FilletOrientation(a, b, c);
    double abd = FilletOrientation(a, b, d);
    double cda = FilletOrientation(c, d, a);
    double cdb = FilletOrientation(c, d, b);
    return ((abc <= 0 && abd >= 0) || (abc >= 0 && abd <= 0)) &&
           ((cda <= 0 && cdb >= 0) || (cda >= 0 && cdb <= 0));
}

typedef struct
{
    int index;
    double left, right;
} FilletEdge;

int CompareFilletEdges(const void *first, const void *second)
{
    double a = ((const FilletEdge *)first)->left;
    double b = ((const FilletEdge *)second)->left;
    return (a > b) - (a < b);
}

int RoundedOutlineIsSimple(const LayoutOutline *outline)
{
    int count = outline->count;
    FilletEdge *edges = malloc(count * sizeof(*edges));
    if (edges == NULL)
        return 0;

    int i, j;
    for (i = 0; i < count; i++)
    {
        double a = outline->points[i].x;
        double b = outline->points[(i + 1) % count].x;
        edges[i].index = i;
        edges[i].left = fmin(a, b);
        edges[i].right = fmax(a, b);
    }
    qsort(edges, count, sizeof(*edges), CompareFilletEdges);

    int simple = 1;
    for (i = 0; i < count && simple; i++)
    {
        int a = edges[i].index;
        int b = (a + 1) % count;
        for (j = i + 1; j < count && edges[j].left <= edges[i].right; j++)
        {
            int c = edges[j].index;
            int d = (c + 1) % count;
            if (b != c && a != d &&
                FilletSegmentsIntersect(outline->points[a], outline->points[b],
                                        outline->points[c], outline->points[d]))
            {
                simple = 0;
                break;
            }
        }
    }
    free(edges);
    return simple;
}

int ValidateRoundedOutline(LayoutOutline *outline, double minStep, int convexCircle)
{
    int count = outline->count;
    int i;
    if (count < 3)
        return 0;

    for (i = 0; i < count; i++)
    {
        DPoint *point = &outline->points[i];
        point->x = round(point->x);
        point->y = round(point->y);
        if (fabs(point->x) > WORLD_MAX || fabs(point->y) > WORLD_MAX)
            return 0;
    }

    for (i = 0; i < count; i++)
    {
        int next = (i + 1) % count;
        int previous = (i + count - 1) % count;
        DPoint a = outline->points[i];
        DPoint b = outline->points[next];
        if (OutlineDistance(a, b) < minStep ||
            fabs(FilletTurn(outline->points[previous], a, b)) > ROUNDED_MAX_TURN)
            return 0;
    }
    return OutlineArea(outline) > 0 && (convexCircle || RoundedOutlineIsSimple(outline));
}

int BuildRoundedOutline(const LayoutOutline *source, LayoutOutline *result,
                        LayoutFillet *corners, double radius, double step, int reserveSpan)
{
    int count = source->count;
    int i;
    result->count = 0;
    result->twiceArcArea = 0;

    for (i = 0; i < count; i++)
    {
        DPoint a = source->points[(i + count - 1) % count];
        DPoint b = source->points[i];
        DPoint c = source->points[(i + 1) % count];
        double turn = FilletTurn(a, b, c);
        if (fabs(turn) >= M_PI - 1e-6)
            return 0;
        if (source->smooth[i] || fabs(turn) < 1e-6)
            turn = 0;
        corners[i].angle = turn;
        corners[i].trim = corners[i].desiredTrim = radius * tan(fabs(turn) / 2);
    }

    /* Keep a grid-safe straight span, or let neighboring arcs meet at a single tangent point. */
    for (i = 0; i < count; i++)
    {
        int next = (i + 1) % count;
        double length = OutlineDistance(source->points[i], source->points[next]);
        double available = fmax(0, length - (reserveSpan ? step : 0));
        double demand = corners[i].desiredTrim + corners[next].desiredTrim;
        if (demand > available)
        {
            double ratio = available / demand;
            corners[i].trim = fmin(corners[i].trim, corners[i].desiredTrim * ratio);
            corners[next].trim = fmin(corners[next].trim, corners[next].desiredTrim * ratio);
        }
    }

    for (i = 0; i < count; i++)
    {
        DPoint b = source->points[i];
        double turn = corners[i].angle;
        if (turn == 0)
        {
            if (!OutlineAppend(result, b))
                return 0;
            continue;
        }

        double trim = corners[i].trim;
        DPoint incoming = OutlineSubtract(b, source->points[(i + count - 1) % count]);
        double length = hypot(incoming.x, incoming.y);
        if (trim <= 0 || length <= 0)
            return 0;
        incoming.x /= length;
        incoming.y /= length;
        DPoint start = OutlinePoint(b.x - incoming.x * trim, b.y - incoming.y * trim);
        double localRadius = trim / tan(fabs(turn) / 2);
        double signedRadius = turn > 0 ? localRadius : -localRadius;
        DPoint center = OutlinePoint(start.x - incoming.y * signedRadius,
                                     start.y + incoming.x * signedRadius);
        double startAngle = atan2(start.y - center.y, start.x - center.x);
        int segments = OutlineArcSegments(localRadius, turn, step, 0);
        if (segments == 0 || fabs(turn) / segments > ROUNDED_MAX_TURN ||
            !OutlineAppendArc(result, center, localRadius, startAngle, turn, step, 0, 1))
            return 0;
    }
    return 1;
}

int RefineRoundedOutline(LayoutOutline *outline, double radius, double minStep, double step)
{
    LayoutOutline refined = {0};
    int pass;
    for (pass = 0; pass < 3; pass++)
    {
        int i, sharp = 0;
        for (i = 0; i < outline->count; i++)
        {
            int previous = (i + outline->count - 1) % outline->count;
            int next = (i + 1) % outline->count;
            int smooth = fabs(FilletTurn(outline->points[previous], outline->points[i],
                                        outline->points[next])) <= ROUNDED_MAX_TURN;
            outline->smooth[i] = (unsigned char)smooth;
            sharp += !smooth;
        }
        if (sharp == 0)
            break;

        LayoutFillet *corners = malloc(outline->count * sizeof(*corners));
        if (corners == NULL)
            break;
        int rebuilt = BuildRoundedOutline(outline, &refined, corners, radius, step, 1);
        free(corners);
        if (!rebuilt)
            break;

        if (ValidateRoundedOutline(&refined, minStep, 0))
        {
            FreeLayoutOutline(outline);
            *outline = refined;
            return 1;
        }

        LayoutOutline previous = *outline;
        *outline = refined;
        refined = previous;
    }
    FreeLayoutOutline(&refined);
    return 0;
}

LObject WriteRoundedOutline(LCell cell, LLayer layer, const LayoutOutline *outline)
{
    LPoint *points = malloc(outline->count * sizeof(*points));
    if (points == NULL)
        return NULL;
    int i;
    for (i = 0; i < outline->count; i++)
        points[i] = LPoint_Set((LCoord)outline->points[i].x, (LCoord)outline->points[i].y);
    LObject created = LPolygon_New(cell, layer, points, outline->count);
    free(points);
    return created;
}

void PreserveTessellatedFillets(LayoutOutline *outline, double step)
{
    int count = outline->count;
    int i;
    for (i = 0; i < count; i++)
    {
        DPoint previous = outline->points[(i + count - 1) % count];
        DPoint point = outline->points[i];
        DPoint next = outline->points[(i + 1) % count];
        /* A saved polygon stores each arc chord as a straight edge. When a chord
           has room for only one minimum-length span, keep its gentle joins. */
        if (fabs(FilletTurn(previous, point, next)) <= ROUNDED_MAX_TURN &&
            fmin(OutlineDistance(previous, point), OutlineDistance(point, next)) < 2 * step)
            outline->smooth[i] = 1;
    }
}

void RemoveFilletVertex(LayoutOutline *outline, int index)
{
    int remaining = --outline->count - index;
    memmove(outline->points + index, outline->points + index + 1,
            remaining * sizeof(*outline->points));
    memmove(outline->smooth + index, outline->smooth + index + 1,
            remaining * sizeof(*outline->smooth));
}

double FilletTurnAt(const LayoutOutline *outline, int index)
{
    int count = outline->count;
    return FilletTurn(outline->points[(index + count - 1) % count], outline->points[index],
                      outline->points[(index + 1) % count]);
}

#include "Ledit_CrescentFillet.h"

int MergeCloseFilletCorners(LayoutOutline *outline, double radius, double step)
{
    int i = 0, changed = 0;
    while (i < outline->count && outline->count > 3)
    {
        int count = outline->count;
        int next = (i + 1) % count;
        DPoint a = outline->points[i], b = outline->points[next];
        DPoint incoming = OutlineSubtract(a, outline->points[(i + count - 1) % count]);
        DPoint outgoing = OutlineSubtract(outline->points[(i + 2) % count], b);
        DPoint gap = OutlineSubtract(b, a);
        double span = hypot(gap.x, gap.y);
        double incomingLength = hypot(incoming.x, incoming.y);
        double outgoingLength = hypot(outgoing.x, outgoing.y);
        double firstTurn = FilletTurnAt(outline, i), secondTurn = FilletTurnAt(outline, next);
        double combinedTurn = fabs(firstTurn + secondTurn);

        double trimDemand = radius * (tan(fabs(firstTurn) / 2) + tan(fabs(secondTurn) / 2));
        int close = fmin(fabs(firstTurn), fabs(secondTurn)) >= M_PI / 12 &&
                    fmin(incomingLength, outgoingLength) >= 1.5 * span &&
                    span < trimDemand + step;

        /* Replace a short face between two turns in the same direction with
           the intersection of the two longer supporting edges. */
        if (close && firstTurn * secondTurn > 0 && combinedTurn < M_PI - 1e-6)
        {
            double cross = OutlineCross(incoming, outgoing);
            double alongIncoming = OutlineCross(gap, outgoing) / cross;
            double alongOutgoing = OutlineCross(gap, incoming) / cross;
            double reach = fmax(2 * fmax(span, step),
                                fmin(radius, 0.25 * fmin(incomingLength, outgoingLength)));
            if (alongIncoming >= 0 && alongOutgoing <= 0 &&
                alongIncoming * incomingLength <= reach && -alongOutgoing * outgoingLength <= reach)
            {
                outline->points[i] = OutlinePoint(a.x + alongIncoming * incoming.x,
                                                  a.y + alongIncoming * incoming.y);
                outline->smooth[i] = 0;
                RemoveFilletVertex(outline, next);
                changed = 1;
                if (next == 0)
                    i = 0;
                continue;
            }
        }
        else if (close && firstTurn * secondTurn < 0 && span < 2 * step)
        {
            /* Opposite turns can form a short jog between parallel edges.
               Spread the transition along the edges while keeping its anchors. */
            double reach = fmin(radius, 0.25 * fmin(incomingLength, outgoingLength));
            DPoint start = OutlinePoint(a.x - incoming.x * reach / incomingLength,
                                        a.y - incoming.y * reach / incomingLength);
            DPoint end = OutlinePoint(b.x + outgoing.x * reach / outgoingLength,
                                      b.y + outgoing.y * reach / outgoingLength);
            DPoint bridge = OutlineSubtract(end, start);
            double startTurn = fabs(atan2(OutlineCross(incoming, bridge), OutlineDot(incoming, bridge)));
            double endTurn = fabs(atan2(OutlineCross(bridge, outgoing), OutlineDot(bridge, outgoing)));
            if (OutlineDistance(start, end) >= step &&
                startTurn < fabs(firstTurn) && endTurn < fabs(secondTurn))
            {
                outline->points[i] = start;
                outline->points[next] = end;
                outline->smooth[i] = startTurn <= M_PI / 12;
                outline->smooth[next] = endTurn <= M_PI / 12;
                changed = 1;
                i++;
            }
        }
        i++;
    }
    return changed;
}

int ThinFilletSamples(LayoutOutline *outline, double minStep)
{
    int changed = 0, i = 0;
    while (i < outline->count && outline->count > 3)
    {
        int next = (i + 1) % outline->count;
        if (OutlineDistance(outline->points[i], outline->points[next]) < minStep)
        {
            double turn = fabs(FilletTurnAt(outline, i)), nextTurn = fabs(FilletTurnAt(outline, next));
            int drop = turn < nextTurn ? i : next;
            double smallerTurn = fmin(turn, nextTurn);
            if (outline->smooth[drop] || smallerTurn <= M_PI / 4)
            {
                RemoveFilletVertex(outline, drop);
                changed = 1;
                if (drop == 0)
                    i = 0;
                else if (drop == i)
                    i--;
                continue;
            }
        }
        i++;
    }
    return changed;
}

int CanExtendFilletSide(const LayoutOutline *outline, int neighbor)
{
    double limit = outline->smooth[neighbor] ? M_PI / 4 : M_PI / 6;
    return fabs(FilletTurnAt(outline, neighbor)) <= limit;
}

int ExtendSharpFilletSides(LayoutOutline *outline, double step)
{
    int changed = 0, i = 0;
    while (i < outline->count && outline->count > 3)
    {
        int count = outline->count;
        double turn = fabs(FilletTurnAt(outline, i));
        if (turn <= ROUNDED_MAX_TURN || turn >= M_PI - 1e-6)
        {
            i++;
            continue;
        }

        /* A tessellated crescent can have many small chords next to a genuine
           cusp. Use a longer chord on each side so a fillet can span that cusp. */
        double segments = ceil(turn / (0.9 * M_PI / 3));
        double trim = step * tan(turn / 2) / (2 * sin(turn / (2 * segments)));
        double requiredLength = 1.2 * trim + step;
        int previous = (i + count - 1) % count, next = (i + 1) % count;
        double incomingLength = OutlineDistance(outline->points[i], outline->points[previous]);
        double outgoingLength = OutlineDistance(outline->points[i], outline->points[next]);
        int extendPrevious = incomingLength < requiredLength && CanExtendFilletSide(outline, previous);
        int extendNext = outgoingLength < requiredLength && CanExtendFilletSide(outline, next);
        if (!extendPrevious && !extendNext)
        {
            i++;
            continue;
        }

        int drop = extendPrevious && (!extendNext || incomingLength <= outgoingLength) ? previous : next;
        RemoveFilletVertex(outline, drop);
        if (drop < i)
            i--;
        changed = 1;
    }
    return changed;
}

int TryRoundedFillets(const LayoutOutline *input, LayoutOutline *output, LayoutFillet *corners,
                      double radius, double minStep, double step)
{
    int reserveSpan;
    for (reserveSpan = 1; reserveSpan >= 0; reserveSpan--)
    {
        if (!BuildRoundedOutline(input, output, corners, radius, step, reserveSpan))
            continue;
        CloseLayoutOutline(output);
        if (ValidateRoundedOutline(output, minStep, 0) ||
            RefineRoundedOutline(output, radius, minStep, step))
            return 1;
    }
    return 0;
}

int TryNarrowFilletCap(const LayoutOutline *outline, int tip, LayoutOutline *capped,
                       double radiusLimit, double step, int widest, double *appliedRadius)
{
    int count = outline->count;
    int first = (tip + count - 1) % count, last = (tip + 1) % count;
    double beforeLength = OutlineDistance(outline->points[tip], outline->points[first]);
    double afterLength = OutlineDistance(outline->points[tip], outline->points[last]);
    double perimeter = 0;
    int i;
    for (i = 0; i < count; i++)
        perimeter += OutlineDistance(outline->points[i], outline->points[(i + 1) % count]);

    int trimmed = 1, bestFirst = -1, bestLast = -1;
    double bestRadius = 0, bestStart = 0, bestSweep = 0;
    DPoint bestCenter = OutlinePoint(0, 0);
    /* Keep the search local so rounding does not remove the main body of a crescent. */
    while (trimmed + 3 < count && (!widest || beforeLength + afterLength <= 0.16 * perimeter))
    {
        int before = (first + count - 1) % count, after = (last + 1) % count;
        DPoint a = outline->points[first], b = outline->points[last];
        DPoint center = OutlinePoint((a.x + b.x) / 2, (a.y + b.y) / 2);
        double radius = OutlineDistance(a, b) / 2;
        int segments = OutlineArcSegments(radius, M_PI, step, 0);
        if (radius > bestRadius && radius <= radiusLimit && segments >= 4)
        {
            DPoint incoming = OutlineSubtract(a, outline->points[before]);
            DPoint radial = OutlineSubtract(a, center);
            double startAngle = atan2(radial.y, radial.x);
            double sweep = OutlineCross(radial, incoming) > 0 ? M_PI : -M_PI;
            DPoint firstSample = OutlineArcPoint(center, radius, startAngle + sweep / segments);
            DPoint lastSample = OutlineArcPoint(center, radius, startAngle + sweep * (segments - 1) / segments);
            /* Leave margin for the change in direction after grid rounding. */
            if (fabs(FilletTurn(outline->points[before], a, firstSample)) < 0.85 * ROUNDED_MAX_TURN &&
                fabs(FilletTurn(lastSample, b, outline->points[after])) < 0.85 * ROUNDED_MAX_TURN)
            {
                bestFirst = first;
                bestLast = last;
                bestRadius = radius;
                bestCenter = center;
                bestStart = startAngle;
                bestSweep = sweep;
                if (!widest || radiusLimit - radius <= step / 2)
                    break;
            }
        }

        int canGrowBefore = CanExtendFilletSide(outline, first);
        int canGrowAfter = CanExtendFilletSide(outline, last);
        if (!canGrowBefore && !canGrowAfter)
            break;
        if (canGrowBefore && (!canGrowAfter || beforeLength <= afterLength))
        {
            beforeLength += OutlineDistance(a, outline->points[before]);
            first = before;
        }
        else
        {
            afterLength += OutlineDistance(b, outline->points[after]);
            last = after;
        }
        trimmed++;
    }
    if (bestFirst < 0)
        return 0;

    capped->count = 0;
    capped->twiceArcArea = 0;
    for (i = bestLast;; i = (i + 1) % count)
    {
        if (!OutlineAppend(capped, outline->points[i]))
            return 0;
        capped->smooth[capped->count - 1] = outline->smooth[i] || i == bestFirst || i == bestLast;
        if (i == bestFirst)
            break;
    }
    *appliedRadius = bestRadius;
    return OutlineAppendArc(capped, bestCenter, bestRadius, bestStart, bestSweep, step, 0, 0);
}

int CapNarrowFilletTips(LayoutOutline *outline, double radiusLimit, double step, int widest,
                        double *smallestRadius)
{
    LayoutOutline capped = {0};
    int tip = 0, changed = 0;
    *smallestRadius = HUGE_VAL;
    while (tip < outline->count)
    {
        int previous = (tip + outline->count - 1) % outline->count;
        int next = (tip + 1) % outline->count;
        double applied;
        if (fabs(FilletTurnAt(outline, tip)) >= (widest ? 0.75 : 0.85) * M_PI &&
            CanExtendFilletSide(outline, previous) && CanExtendFilletSide(outline, next) &&
            TryNarrowFilletCap(outline, tip, &capped, radiusLimit, step, widest, &applied))
        {
            *smallestRadius = fmin(*smallestRadius, applied);
            LayoutOutline previous = *outline;
            *outline = capped;
            capped = previous;
            changed = 1;
            /* The new polygon begins just after the cap. Check any other tips. */
            tip = 0;
        }
        else
            tip++;
    }
    FreeLayoutOutline(&capped);
    return changed;
}

int CopyFilletSamples(const LayoutOutline *input, LayoutOutline *output)
{
    if (output->capacity < input->count)
    {
        output->count = 0;
        int i;
        for (i = 0; i < input->count; i++)
        {
            if (!OutlineAppend(output, input->points[i]))
                return 0;
        }
    }

    output->count = input->count;
    output->twiceArcArea = 0;
    memcpy(output->points, input->points, input->count * sizeof(*input->points));
    memcpy(output->smooth, input->smooth, input->count * sizeof(*input->smooth));
    return 1;
}

#include "Ledit_SampledFillet.h"

int HasConcaveFilletBoundary(const LayoutOutline *outline)
{
    /* Integrate signed turns to distinguish curved recesses from grid jitter. */
    double positive = 0, negative = 0;
    int positiveBend = 0, negativeBend = 0;
    int i;
    for (i = 0; i < 2 * outline->count; i++)
    {
        double turn = FilletTurnAt(outline, i % outline->count);
        positive = fmax(0, positive + turn);
        negative = fmin(0, negative + turn);
        positiveBend |= positive > M_PI / 4;
        negativeBend |= negative < -M_PI / 4;
        if (positiveBend && negativeBend)
            return 1;
    }
    return 0;
}

int RoundFilletContour(LayoutOutline *input, LayoutOutline *output, double radius,
                       double minStep, double step, double *appliedRadius)
{
    LayoutOutline prepared = {0};
    LayoutFillet *corners = malloc(input->count * sizeof(*corners));
    if (corners == NULL)
        return 0;

    PreserveTessellatedFillets(input, step);
    int i, attempt, valid = 0;
    double tryRadius = radius;
    double smallestCapRadius;
    if (HasConcaveFilletBoundary(input) && CopyFilletSamples(input, &prepared))
    {
        ThinFilletSamples(&prepared, minStep);
        if (CapNarrowFilletTips(&prepared, radius, step, 1, &smallestCapRadius) &&
            ValidateRoundedOutline(&prepared, minStep, 0))
        {
            FreeLayoutOutline(output);
            *output = prepared;
            *appliedRadius = smallestCapRadius;
            free(corners);
            return 1;
        }
    }
    for (attempt = 0; attempt < 12 && !valid && CopyFilletSamples(input, &prepared); attempt++, tryRadius *= 0.8)
    {
        int merged = MergeCloseFilletCorners(&prepared, tryRadius, step);
        if (merged)
            valid = TryRoundedFillets(&prepared, output, corners, tryRadius, minStep, step);
        else
            valid = TryRoundedFillets(input, output, corners, tryRadius, minStep, step);
        if (!valid)
        {
            int gentle = 0;
            for (i = 0; i < prepared.count; i++)
            {
                if (!prepared.smooth[i] && fabs(FilletTurnAt(&prepared, i)) <= M_PI / 6)
                {
                    prepared.smooth[i] = 1;
                    gentle = 1;
                }
            }
            int thinned = ThinFilletSamples(&prepared, minStep);
            int extended = ExtendSharpFilletSides(&prepared, step);
            if (gentle || thinned || extended)
                valid = TryRoundedFillets(&prepared, output, corners, tryRadius, minStep, step);
        }
        if (!valid && merged)
            valid = TryRoundedFillets(input, output, corners, tryRadius, minStep, step);
    }

    if (!valid && CopyFilletSamples(input, &prepared))
    {
        ThinFilletSamples(&prepared, minStep);
        if (CapNarrowFilletTips(&prepared, radius, step, 0, &smallestCapRadius))
            valid = ValidateRoundedOutline(&prepared, minStep, 0) ||
                    RefineRoundedOutline(&prepared, radius, minStep, step);
        if (valid)
        {
            FreeLayoutOutline(output);
            *output = prepared;
            *appliedRadius = smallestCapRadius;
            prepared.points = NULL;
            prepared.smooth = NULL;
        }
    }
    FreeLayoutOutline(&prepared);
    free(corners);
    return valid;
}

#define FILLET_SOURCE_PROPERTY "LeditFilletSourceV2"
#define LEGACY_FILLET_SOURCE_PROPERTY "LeditFilletSourceV1"

typedef struct
{
    unsigned int hash;
    unsigned int count;
} FilletSourceHeader;

unsigned int FilletVertexHash(DPoint point)
{
    unsigned int x = (unsigned int)(LCoord)round(point.x);
    unsigned int y = (unsigned int)(LCoord)round(point.y);
    unsigned int value = (x * 0x9e3779b1U) ^ (y * 0x85ebca77U);
    return (value ^ (value >> 16)) * 0x7feb352dU;
}

/* L-Edit can reorder a contour or discard redundant straight-edge vertices. */
unsigned int FilletOutlineHash(const LayoutOutline *outline, int ignoreStraightVertices)
{
    unsigned int hash = 0, first = 0, previous = 0;
    int retained = 0, i;
    for (i = 0; i < outline->count; i++)
    {
        DPoint point = outline->points[i];
        if (ignoreStraightVertices)
        {
            DPoint start = outline->points[(i + outline->count - 1) % outline->count];
            DPoint end = outline->points[(i + 1) % outline->count];
            if (FilletOrientation(start, point, end) == 0 &&
                OutlineDot(OutlineSubtract(point, start), OutlineSubtract(end, point)) > 0)
                continue;
        }

        unsigned int current = FilletVertexHash(point);
        if (retained == 0)
            first = current;
        else
            hash += (previous ^ current) * (previous + current) + previous;
        previous = current;
        retained++;
    }
    if (retained > 0)
        hash += (previous ^ first) * (previous + first) + previous;
    return hash;
}

unsigned int FilletSourceSize(unsigned int count)
{
    unsigned int vertexSize = sizeof(DPoint) + sizeof(unsigned char);
    if (count < 3 || count > (UINT_MAX - sizeof(FilletSourceHeader)) / vertexSize)
        return 0;
    return sizeof(FilletSourceHeader) + count * vertexSize;
}

void SaveFilletSource(LObject object, const LayoutOutline *source, const LayoutOutline *rounded)
{
    unsigned int size = FilletSourceSize(source->count);
    unsigned char *data = size == 0 ? NULL : malloc(size);
    if (data == NULL)
        return;

    FilletSourceHeader header = {FilletOutlineHash(rounded, 1), (unsigned int)source->count};
    size_t pointBytes = source->count * sizeof(*source->points);
    memcpy(data, &header, sizeof(header));
    memcpy(data + sizeof(header), source->points, pointBytes);
    memcpy(data + sizeof(header) + pointBytes, source->smooth, source->count);
    LEntity_AssignBlobProperty((LEntity)object, FILLET_SOURCE_PROPERTY, data, size);
    free(data);
}

int ReadFilletSource(LObject object, const LayoutOutline *selected, unsigned int hash,
                     LayoutOutline *outline)
{
    const char *property = FILLET_SOURCE_PROPERTY;
    if (LEntity_PropertyExists((LEntity)object, property) != LStatusOK)
    {
        property = LEGACY_FILLET_SOURCE_PROPERTY;
        if (LEntity_PropertyExists((LEntity)object, property) != LStatusOK)
            return 0;
        hash = FilletOutlineHash(selected, 0);
    }
    unsigned int size = LEntity_GetPropertyValueSize((LEntity)object, property);
    if (size < FilletSourceSize(3))
        return 0;

    unsigned char *data = malloc(size);
    if (data == NULL)
        return 0;

    int valid = 0;
    if (LEntity_GetPropertyValue((LEntity)object, property, data, size) == LStatusOK)
    {
        FilletSourceHeader header;
        memcpy(&header, data, sizeof(header));
        if (header.hash == hash && header.count <= INT_MAX && size == FilletSourceSize(header.count))
        {
            unsigned int i;
            for (i = 0; i < header.count; i++)
            {
                DPoint point;
                memcpy(&point, data + sizeof(header) + i * sizeof(point), sizeof(point));
                if (!isfinite(point.x) || !isfinite(point.y) ||
                    fabs(point.x) > WORLD_MAX || fabs(point.y) > WORLD_MAX ||
                    !OutlineAppend(outline, point))
                    break;
                outline->smooth[outline->count - 1] =
                    data[sizeof(header) + header.count * sizeof(point) + i] != 0;
            }
            valid = i == header.count && outline->count == (int)header.count && OutlineArea(outline) > 0;
        }
    }
    free(data);
    return valid;
}

double FilletEdgeLength(const LayoutOutline *outline, int edge)
{
    return OutlineDistance(outline->points[edge], outline->points[(edge + 1) % outline->count]);
}

/* Recover a legacy circular fillet from its sampled arc and both tangent edges. */
int InferFilletCorner(const LayoutOutline *outline, int left, int right, int chords,
                      double tolerance, DPoint *vertex)
{
    if (chords < 3)
        return 0;

    int count = outline->count;
    DPoint a = outline->points[(left + 1) % count], b = outline->points[right];
    DPoint incoming = OutlineSubtract(a, outline->points[left]);
    DPoint outgoing = OutlineSubtract(outline->points[(right + 1) % count], b);
    double cross = OutlineCross(incoming, outgoing);
    double turn = atan2(cross, OutlineDot(incoming, outgoing));
    if (fabs(turn) < M_PI / 12 || fabs(turn) >= 0.95 * M_PI)
        return 0;

    DPoint gap = OutlineSubtract(b, a);
    double alongIncoming = OutlineCross(gap, outgoing) / cross;
    double alongOutgoing = OutlineCross(gap, incoming) / cross;
    double length = hypot(incoming.x, incoming.y);
    double firstTrim = alongIncoming * length;
    double lastTrim = -alongOutgoing * hypot(outgoing.x, outgoing.y);
    double radius = (firstTrim + lastTrim) / (2 * tan(fabs(turn) / 2));
    if (firstTrim <= 0 || lastTrim <= 0 || fabs(firstTrim - lastTrim) > tolerance ||
        radius <= tolerance)
        return 0;

    double signedRadius = turn > 0 ? radius : -radius;
    DPoint center = OutlinePoint(a.x - incoming.y * signedRadius / length,
                                 a.y + incoming.x * signedRadius / length);
    int i = (left + 1) % count;
    for (;; i = (i + 1) % count)
    {
        if (fabs(OutlineDistance(outline->points[i], center) - radius) > tolerance)
            return 0;
        if (i == right)
            break;
    }

    *vertex = OutlinePoint(a.x + alongIncoming * incoming.x, a.y + alongIncoming * incoming.y);
    return fabs(vertex->x) <= WORLD_MAX && fabs(vertex->y) <= WORLD_MAX;
}

int InferLegacyFilletSource(const LayoutOutline *outline, LayoutOutline *source, double step)
{
    int count = outline->count;
    double shortest = HUGE_VAL;
    int i;
    for (i = 0; i < count; i++)
    {
        if (fabs(FilletTurnAt(outline, i)) > ROUNDED_MAX_TURN)
            return 0;
        shortest = fmin(shortest, FilletEdgeLength(outline, i));
    }

    double longEdge = 3 * fmax(shortest, step);
    int start = 0;
    while (start < count && FilletEdgeLength(outline, start) < longEdge)
        start++;
    if (start == count)
        return 0;

    int edge = start, remaining = count, inferred = 0;
    while (remaining > 0)
    {
        int right = (edge + 1) % count, chords = 0;
        while (right != start && FilletEdgeLength(outline, right) < longEdge)
        {
            right = (right + 1) % count;
            chords++;
        }

        DPoint vertex;
        if (right != edge && InferFilletCorner(outline, edge, right, chords, 4 + shortest, &vertex))
        {
            if (!OutlineAppend(source, vertex))
                return 0;
            inferred++;
        }
        else
        {
            i = (edge + 1) % count;
            for (;; i = (i + 1) % count)
            {
                if (!OutlineAppend(source, outline->points[i]))
                    return 0;
                source->smooth[source->count - 1] = outline->smooth[i];
                if (i == right)
                    break;
            }
        }
        remaining -= chords + 1;
        edge = right;
    }

    CloseLayoutOutline(source);
    return inferred > 0 && source->count >= 3 && RoundedOutlineIsSimple(source);
}

typedef enum
{
    RoundCreated,
    RoundUnreadable,
    RoundUnfit,
    RoundWriteFailed,
    RoundSameGeometry,
    RoundResultCount
} RoundedPolygonResult;

RoundedPolygonResult CreateRoundedPolygon(LCell cell, LLayer layer, LObject source,
                                          double radius, double minStep,
                                          LObject *createdObject, double *appliedRadius)
{
    *createdObject = NULL;
    *appliedRadius = 0;
    LayoutOutline input = {0};
    LayoutOutline output = {0};
    LayoutOutline sourceContour = {0};
    RoundedPolygonResult result = RoundUnreadable;
    LShapeType shape = LObject_GetShape(source);
    int circle = shape == LCircle;
    double step = minStep + sqrt(2.0); /* Grid rounding can shorten a chord by up to sqrt(2). */
    int readable = ReadShapeOutline(cell, layer, source, &input, step, 0, 8 * step);
    CloseLayoutOutline(&input);

    if (readable && input.count >= 3 && OutlineArea(&input) > 0)
    {
        result = RoundUnfit;
        int valid = 0;
        int plainPolygon = shape == LPolygon && !LPolygon_HasCurve(source);
        unsigned int inputHash = plainPolygon ? FilletOutlineHash(&input, 1) : 0;
        const LayoutOutline *rounded = &output;
        LayoutOutline *base = &input;
        if (plainPolygon)
        {
            int recovered = ReadFilletSource(source, &input, inputHash, &sourceContour);
            if (!recovered)
            {
                sourceContour.count = 0;
                recovered = InferLegacyFilletSource(&input, &sourceContour, step);
            }
            if (recovered)
                base = &sourceContour;
        }

        if (circle)
        {
            rounded = &input;
            valid = ValidateRoundedOutline(&input, minStep, 1);
        }
        else
        {
            *appliedRadius = RoundCircularCrescent(base, &output, radius, minStep, step);
            if (*appliedRadius == 0)
                *appliedRadius = RoundSampledFilletCorners(base, &output, radius, minStep, step);
            valid = *appliedRadius > 0 || RoundFilletContour(base, &output, radius, minStep, step, appliedRadius);
        }

        if (!valid && base == &input && !circle && ValidateRoundedOutline(&input, minStep, 0))
        {
            rounded = &input;
            valid = 1;
        }
        if (valid && plainPolygon && inputHash == FilletOutlineHash(rounded, 1) &&
            LObject_GetLayer(cell, source) == layer)
            result = RoundSameGeometry;
        else if (valid)
        {
            *createdObject = WriteRoundedOutline(cell, layer, rounded);
            result = *createdObject != NULL ? RoundCreated : RoundWriteFailed;
            if (*createdObject != NULL && !circle)
                SaveFilletSource(*createdObject, base, rounded);
        }
    }
    FreeLayoutOutline(&sourceContour);
    FreeLayoutOutline(&output);
    FreeLayoutOutline(&input);
    return result;
}

double RoundedMinimumPointSpacing(LFile file)
{
    return 0.01 / LFile_IntUtoMicrons(file, 1);
}

void RoundSelectedCorners(void)
{
    LCell cell = LCell_GetVisible();
    LSelection selection = LSelection_GetList();
    if (cell == NULL || selection == NULL)
    {
        LDialog_AlertBox("Select shapes to round.");
        return;
    }

    LDialogItem input[] = {{"Corner radius (um)", "0.5"}};
    if (!LDialog_MultiLineInputBox("Round corners - select result, report crescent radius (10 nm)", input, 1))
        return;

    char *end;
    double microns = strtod(input[0].value, &end);
    while (*end == ' ' || *end == '\t')
        end++;
    LFile file = LCell_GetFile(cell);
    if (*end != '\0' || !isfinite(microns) || microns <= 0 ||
        microns > LFile_IntUtoMicrons(file, WORLD_MAX))
    {
        LDialog_AlertBox("Enter a positive corner radius within the design coordinate range.");
        return;
    }

    OutputLayerChoice output;
    if (!ChooseOutputLayer(cell, 1, &output))
        return;

    double radius = LFile_MicronsToIntU(file, microns);
    double minStep = RoundedMinimumPointSpacing(file);
    int selectionCount = 0;
    LSelection item;
    for (item = selection; item != NULL; item = LSelection_GetNext(item))
        selectionCount++;
    LObject *objects = malloc(selectionCount * sizeof(*objects));
    if (objects == NULL)
    {
        LDialog_AlertBox("Could not allocate the list of rounded polygons.");
        return;
    }

    /* Creating layout objects can change the live selection. Snapshot the inputs first. */
    int i = 0;
    for (item = selection; item != NULL; item = LSelection_GetNext(item))
        objects[i++] = LSelection_GetObject(item);

    int counts[RoundResultCount] = {0};
    int crescentCount = 0;
    double minCrescentRadius = HUGE_VAL, maxCrescentRadius = 0;
    for (i = 0; i < selectionCount; i++)
    {
        LObject object = objects[i];
        if (object == NULL)
        {
            counts[RoundUnreadable]++;
            continue;
        }

        LLayer layer = ResolveObjectOutputLayer(cell, &output, object);
        if (layer == NULL)
            continue;
        double applied = 0;
        LObject created = NULL;
        RoundedPolygonResult result = CreateRoundedPolygon(cell, layer, object, radius, minStep,
                                                            &created, &applied);
        if (created != NULL)
            objects[counts[RoundCreated]] = created;
        counts[result]++;
        if (applied > 0 && (result == RoundCreated || result == RoundSameGeometry))
        {
            crescentCount++;
            minCrescentRadius = fmin(minCrescentRadius, applied);
            maxCrescentRadius = fmax(maxCrescentRadius, applied);
        }
    }

    int selectedResults = 0;
    if (counts[RoundCreated] > 0)
    {
        LSelection_DeselectAll();
        for (i = 0; i < counts[RoundCreated]; i++)
        {
            if (LSelection_AddObject(objects[i]) == LStatusOK)
                selectedResults++;
        }
    }
    free(objects);
    LDisplay_Refresh();
    ReportSkippedOutputLayers(&output);

    char crescentReport[160] = "";
    if (crescentCount > 0)
    {
        double unit = LFile_IntUtoMicrons(file, 1);
        if (crescentCount == 1 || maxCrescentRadius == minCrescentRadius)
            snprintf(crescentReport, sizeof(crescentReport),
                     "\nTip radius (um): requested %.6g, applied %.6g",
                     microns, maxCrescentRadius * unit);
        else
            snprintf(crescentReport, sizeof(crescentReport),
                     "\nTip radius (um): requested %.6g, applied %.6g to %.6g",
                     microns, minCrescentRadius * unit, maxCrescentRadius * unit);
    }

    if (crescentCount || counts[RoundUnreadable] || counts[RoundUnfit] ||
        counts[RoundWriteFailed] || counts[RoundSameGeometry] || selectedResults < counts[RoundCreated])
    {
        char message[512];
        snprintf(message, sizeof(message),
                 "Rounded polygons created: %d (selected: %d)\n"
                 "Unsupported or unreadable outlines: %d\n"
                 "No valid fit at 10 nm spacing: %d\n"
                 "L-Edit could not write polygon: %d\n"
                 "Geometry unchanged at requested radius: %d%s",
                 counts[RoundCreated], selectedResults, counts[RoundUnreadable], counts[RoundUnfit],
                 counts[RoundWriteFailed], counts[RoundSameGeometry], crescentReport);
        LDialog_AlertBox(message);
    }
}

#endif
