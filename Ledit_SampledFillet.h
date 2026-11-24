#ifndef LEDIT_SAMPLED_FILLET_H
#define LEDIT_SAMPLED_FILLET_H

#define FILLET_SUPPORT_JOIN 2

typedef struct
{
    int edge;
    int span;
    DPoint direction;
    DPoint offsetOrigin;
    double length;
    double distanceFromTip;
} FilletSupport;

typedef struct
{
    int incomingEdge, outgoingEdge;
    DPoint center;
    double startAngle, sweep, cost;
} SampledFillet;

int CollectFilletSupports(const LayoutOutline *outline, int tip, int direction,
                          double signedRadius, double reach, FilletSupport *supports)
{
    int index = tip, used = 0, span;
    int count = outline->count;
    double distance = 0;
    for (span = 1; span < count - 1 && distance <= reach; span++)
    {
        int next = (index + direction + count) % count;
        int edge = direction < 0 ? next : index;
        DPoint a = outline->points[edge];
        DPoint vector = OutlineSubtract(outline->points[(edge + 1) % count], a);
        double length = hypot(vector.x, vector.y);
        if (length <= 0)
            break;
        DPoint unit = OutlinePoint(vector.x / length, vector.y / length);
        FilletSupport support = {edge, span, unit,
                                 OutlinePoint(a.x - unit.y * signedRadius, a.y + unit.x * signedRadius),
                                 length, distance};
        supports[used++] = support;

        if (outline->smooth[next] == FILLET_SUPPORT_JOIN || !CanExtendFilletSide(outline, next))
            break;
        distance += length;
        index = next;
    }
    return used;
}

/* On a broad arc, chords near the minimum spacing oversample below grid accuracy. */
double SampledFilletChord(double radius, double minimum)
{
    return fmax(minimum, 2 * sqrt(radius));
}

int FindSampledFillet(const LayoutOutline *outline, int tip, double radius, double step,
                      FilletSupport *incoming, FilletSupport *outgoing, SampledFillet *best)
{
    double turn = FilletTurnAt(outline, tip);
    double signedRadius = turn > 0 ? radius : -radius;
    double reach = 3 * fmax(radius, radius * tan(fabs(turn) / 2)) + step;
    int incomingCount = CollectFilletSupports(outline, tip, -1, signedRadius, reach, incoming);
    int outgoingCount = CollectFilletSupports(outline, tip, 1, signedRadius, reach, outgoing);
    double tolerance = step / 2;
    double chord = SampledFilletChord(radius, step);
    best->cost = HUGE_VAL;
    int i, j;
    for (i = 0; i < incomingCount && incoming[i].distanceFromTip <= best->cost + tolerance; i++)
    {
        FilletSupport a = incoming[i];
        for (j = 0; j < outgoingCount; j++)
        {
            FilletSupport b = outgoing[j];
            if (a.span + b.span >= outline->count ||
                a.distanceFromTip + b.distanceFromTip > best->cost + tolerance)
                break;

            double cross = OutlineCross(a.direction, b.direction);
            if (cross * turn <= 0 || fabs(cross) < 1e-8)
                continue;
            DPoint gap = OutlineSubtract(b.offsetOrigin, a.offsetOrigin);
            double alongIncoming = OutlineCross(gap, b.direction) / cross;
            double alongOutgoing = OutlineCross(gap, a.direction) / cross;
            if (alongIncoming < -tolerance || alongIncoming > a.length + tolerance ||
                alongOutgoing < -tolerance || alongOutgoing > b.length + tolerance)
                continue;

            double sweep = atan2(cross, OutlineDot(a.direction, b.direction));
            int segments = OutlineArcSegments(radius, sweep, chord, 0);
            if (segments == 0 || fabs(sweep) / segments > 0.85 * ROUNDED_MAX_TURN)
                continue;

            double extension = fmax(0, fmax(-alongIncoming, alongIncoming - a.length)) +
                               fmax(0, fmax(-alongOutgoing, alongOutgoing - b.length));
            double cost = a.distanceFromTip + b.distanceFromTip +
                          a.length - alongIncoming + alongOutgoing + 4 * extension;
            if (cost >= best->cost)
                continue;

            best->incomingEdge = a.edge;
            best->outgoingEdge = b.edge;
            best->center = OutlinePoint(a.offsetOrigin.x + alongIncoming * a.direction.x,
                                        a.offsetOrigin.y + alongIncoming * a.direction.y);
            best->startAngle = atan2(-a.direction.x * signedRadius, a.direction.y * signedRadius);
            best->sweep = sweep;
            best->cost = cost;
        }
    }
    return best->cost != HUGE_VAL;
}

int StitchSampledFillet(const LayoutOutline *outline, LayoutOutline *output,
                        SampledFillet fillet, double radius, double step)
{
    output->count = 0;
    output->twiceArcArea = 0;
    double chord = SampledFilletChord(radius, step);
    if (!OutlineAppendArc(output, fillet.center, radius, fillet.startAngle, fillet.sweep, chord, 0, 1))
        return 0;
    output->smooth[0] = FILLET_SUPPORT_JOIN;
    output->smooth[output->count - 1] = FILLET_SUPPORT_JOIN;

    DPoint start = output->points[0], end = output->points[output->count - 1];
    int count = outline->count;
    int first = (fillet.outgoingEdge + 1) % count;
    int last = fillet.incomingEdge;
    /* A join can fall just short of a source vertex. Drop redundant points
       there so the transition does not leave a sub-grid chord at either end. */
    while (first != last && OutlineDistance(end, outline->points[first]) < step)
        first = (first + 1) % count;
    while (last != first && OutlineDistance(start, outline->points[last]) < step)
        last = (last + count - 1) % count;

    int index;
    for (index = first;; index = (index + 1) % count)
    {
        if (!OutlineAppend(output, outline->points[index]))
            return 0;
        output->smooth[output->count - 1] = outline->smooth[index];
        if (index == last)
            break;
    }

    CloseLayoutOutline(output);
    return output->count >= 3 && RoundedOutlineIsSimple(output);
}

int HasSampledFilletCorner(const LayoutOutline *outline, double radius, double step)
{
    int i;
    for (i = 0; i < outline->count; i++)
    {
        int previous = (i + outline->count - 1) % outline->count;
        int next = (i + 1) % outline->count;
        double turn = fabs(FilletTurnAt(outline, i));
        if (turn <= ROUNDED_MAX_TURN)
            continue;
        double trim = radius * tan(turn / 2);
        if ((CanExtendFilletSide(outline, previous) &&
             OutlineDistance(outline->points[previous], outline->points[i]) < trim + step) ||
            (CanExtendFilletSide(outline, next) &&
             OutlineDistance(outline->points[next], outline->points[i]) < trim + step))
            return 1;
    }
    return 0;
}

typedef struct
{
    int first, last;
} FilletSampleSpan;

/* Reduce grid jitter before using a sampled edge as a tangent. Every discarded
   vertex stays within the tolerance of the retained polyline. */
void SimplifyFilletSamples(LayoutOutline *outline, double tolerance)
{
    int count = outline->count;
    unsigned char *keep = calloc(count, sizeof(*keep));
    FilletSampleSpan *stack = malloc((size_t)count * sizeof(*stack));
    if (keep == NULL || stack == NULL)
    {
        free(keep);
        free(stack);
        return;
    }

    int split = 1, i;
    for (i = 2; i < count; i++)
    {
        if (OutlineDistance(outline->points[0], outline->points[i]) >
            OutlineDistance(outline->points[0], outline->points[split]))
            split = i;
    }

    int pending = 0;
    stack[pending++] = (FilletSampleSpan){0, split};
    stack[pending++] = (FilletSampleSpan){split, count};
    keep[0] = keep[split] = 1;
    while (pending > 0)
    {
        FilletSampleSpan span = stack[--pending];
        DPoint start = outline->points[span.first];
        DPoint line = OutlineSubtract(outline->points[span.last % count], start);
        double lengthSquared = OutlineDot(line, line);
        double maxError = tolerance;
        int furthest = -1;
        for (i = span.first + 1; i < span.last; i++)
        {
            DPoint delta = OutlineSubtract(outline->points[i], start);
            double fraction = lengthSquared > 0 ?
                              fmax(0, fmin(1, OutlineDot(delta, line) / lengthSquared)) : 0;
            double error = hypot(delta.x - fraction * line.x, delta.y - fraction * line.y);
            if (error > maxError)
            {
                maxError = error;
                furthest = i;
            }
        }
        if (furthest >= 0)
        {
            keep[furthest] = 1;
            stack[pending++] = (FilletSampleSpan){span.first, furthest};
            stack[pending++] = (FilletSampleSpan){furthest, span.last};
        }
    }

    int retained = 0;
    for (i = 0; i < count; i++)
        retained += keep[i];
    if (retained >= 3)
    {
        int next = 0;
        for (i = 0; i < count; i++)
        {
            if (keep[i])
            {
                outline->points[next] = outline->points[i];
                outline->smooth[next++] = outline->smooth[i];
            }
        }
        outline->count = retained;
    }
    free(stack);
    free(keep);
}

double RoundSampledFilletCorners(const LayoutOutline *source, LayoutOutline *output,
                                 double requested, double minStep, double step)
{
    if (!HasSampledFilletCorner(source, requested, step))
        return 0;
    LayoutOutline working = {0}, candidate = {0}, probe = {0};
    double applied = 0;
    if (!CopyFilletSamples(source, &working))
        goto cleanup;
    SimplifyFilletSamples(&working, 1.5); /* Internal units, close to one grid diagonal. */
    ThinFilletSamples(&working, minStep);

    /* Each successful stitch removes one sharp corner. Newly sampled arcs and
       their joins are excluded from later searches in the same outline. */
    int pass;
    double smallest = requested;
    for (pass = 0; pass < source->count; pass++)
    {
        int tip = 0;
        while (tip < working.count &&
               (working.smooth[tip] == FILLET_SUPPORT_JOIN ||
                fabs(FilletTurnAt(&working, tip)) <= ROUNDED_MAX_TURN))
            tip++;
        if (tip == working.count)
        {
            if (pass > 0 && ValidateRoundedOutline(&working, minStep, 0))
            {
                FreeLayoutOutline(output);
                *output = working;
                working.points = NULL;
                working.smooth = NULL;
                applied = smallest;
            }
            break;
        }

        FilletSupport *supports = malloc(2 * (size_t)working.count * sizeof(*supports));
        if (supports == NULL)
            break;
        double radius = requested;
        SampledFillet fillet;
        int attempt, found = 0;
        for (attempt = 0; attempt < 16 && radius >= step; attempt++, radius *= 0.8)
        {
            if (FindSampledFillet(&working, tip, radius, step, supports,
                                  supports + working.count, &fillet) &&
                StitchSampledFillet(&working, &candidate, fillet, radius, step))
            {
                found = 1;
                break;
            }
        }
        if (found && attempt > 0)
        {
            /* The coarse search gives a fit and a failed larger radius.
               Refine between them before accepting a smaller fillet. */
            double upper = radius / 0.8;
            int refinement;
            for (refinement = 0; refinement < 5 && upper - radius > step; refinement++)
            {
                double middle = (radius + upper) / 2;
                if (FindSampledFillet(&working, tip, middle, step, supports,
                                      supports + working.count, &fillet) &&
                    StitchSampledFillet(&working, &probe, fillet, middle, step))
                {
                    LayoutOutline previous = candidate;
                    candidate = probe;
                    probe = previous;
                    radius = middle;
                }
                else
                    upper = middle;
            }
        }
        free(supports);
        if (!found)
            break;

        LayoutOutline previous = working;
        working = candidate;
        candidate = previous;
        smallest = fmin(smallest, radius);
    }

cleanup:
    FreeLayoutOutline(&probe);
    FreeLayoutOutline(&candidate);
    FreeLayoutOutline(&working);
    return applied;
}

#endif
