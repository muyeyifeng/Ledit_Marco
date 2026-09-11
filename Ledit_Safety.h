/* Shared helpers for the L-Edit macro modules. Keep beside the .c files. */
#ifndef LEDIT_MACRO_SAFETY_H
#define LEDIT_MACRO_SAFETY_H
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef __cplusplus
#include <stdbool.h>
#endif
#define LM_MAX_POINTS 100000
#define LM_MAX_OBJECTS 100000

int LM_Context(void)
{
    LCell cell = LCell_GetVisible();
    if (!cell || !LCell_GetFile(cell)) {
        LDialog_AlertBox("Open an editable layout cell first.");
        return 0;
    }
    return 1;
}

double LM_UnitsPerMicron(void)
{
    LFile file = LCell_GetFile(LCell_GetVisible());
    double unit = LFile_IntUtoMicrons(file, 1);
    return unit > 0 ? 1.0 / unit : 0;
}

/* Validate before atof or conversion to integral layout coordinates. */
int LM_Number(const char *text, double *value)
{
    char *end;
    double v;
    errno = 0;
    v = strtod(text, &end);
    if (end == text || errno == ERANGE) return 0;
    while (isspace((unsigned char)*end)) ++end;
    if (*end || !(v >= -1.0e9 && v <= 1.0e9)) return 0;
    *value = v;
    return 1;
}

int LM_Dialog(const char *title, LDialogItem *items, int count)
{
    int i;
    double unit;
    if (!LM_Context()) return 0;
    if (!LDialog_MultiLineInputBox(title, items, count)) return 0;
    unit = LM_UnitsPerMicron();
    if (!(unit > 0)) return 0;
    for (i = 0; i < count; ++i) {
        double v;
        const char *p = items[i].prompt;
        if (!LM_Number(items[i].value, &v) ||
            (strstr(p, "(um") && fabs(v) > WORLD_MAX / unit) ||
            ((strstr(p, "Number") || strstr(p, "number")) &&
             (v != floor(v) || v < 1 || v > LM_MAX_POINTS)) ||
            (strstr(p, "1 = yes") && v != 0 && v != 1)) {
            LDialog_AlertBox("Invalid number, count, flag, or coordinate range.");
            return 0;
        }
    }
    return 1;
}

int LM_PointValid(double x, double y)
{
    return x >= -WORLD_MAX && x <= WORLD_MAX && y >= -WORLD_MAX && y <= WORLD_MAX;
}

long LM_RoundCoord(double value)
{
    return (long)(value < 0 ? ceil(value - 0.5) : floor(value + 0.5));
}

LObject LM_NewPolygon(LCell cell, LLayer layer, LPoint *points, int count)
{
    int i, n = 0;
    LObject result;
    if (!cell || !layer || !points || count < 3 || count > LM_MAX_POINTS) return NULL;
    for (i = 0; i < count; ++i) {
        if (!LM_PointValid(points[i].x, points[i].y)) return NULL;
        if (n == 0 || points[i].x != points[n - 1].x || points[i].y != points[n - 1].y)
            points[n++] = points[i];
    }
    if (n > 1 && points[0].x == points[n - 1].x && points[0].y == points[n - 1].y) --n;
    if (n < 3) return NULL;
    result = LPolygon_New(cell, layer, points, n);
    if (!result) LDialog_AlertBox("Polygon creation failed.");
    return result;
}

LObject LM_NewCircle(LCell cell, LLayer layer, LPoint center, long radius)
{
    LObject result;
    if (!cell || !layer || radius <= 0 ||
        !LM_PointValid((double)center.x + radius, (double)center.y + radius) ||
        !LM_PointValid((double)center.x - radius, (double)center.y - radius)) return NULL;
    result = LCircle_New(cell, layer, center, radius);
    if (!result) LDialog_AlertBox("Circle creation failed.");
    return result;
}

/* Reject an out-of-world translation before mutating an object. */
int LM_CanMove(LObject object, double dx, double dy)
{
    LRect box;
    if (!object) return 0;
    box = LObject_GetMbb(object);
    return LM_PointValid(box.x0 + dx, box.y0 + dy) &&
           LM_PointValid(box.x1 + dx, box.y1 + dy);
}

int LM_CornerSamples(double radius)
{
    double n = ceil(radius * M_PI / 3 / (0.02 * LM_UnitsPerMicron()));
    if (!(radius > 0) || !(n <= LM_MAX_POINTS / 6)) return 0;
    return n < 2 ? 2 : (int)n;
}

int LM_CanTransform(LObject object, LPoint center, double rad, int mirror)
{
    LRect box = LObject_GetMbb(object);
    double c = cos(mirror ? 2 * rad : rad), s = sin(mirror ? 2 * rad : rad);
    int i;
    for (i = 0; i < 4; ++i) {
        double dx = (double)(i & 1 ? box.x1 : box.x0) - center.x;
        double dy = (double)(i & 2 ? box.y1 : box.y0) - center.y;
        double x = center.x + c * dx + (mirror ? s : -s) * dy;
        double y = center.y + s * dx + (mirror ? -c : c) * dy;
        if (!LM_PointValid(x, y)) return 0;
    }
    return 1;
}
#endif
