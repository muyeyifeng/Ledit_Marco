/* Test doubles model only the UPI contracts used by these regression cases. */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "ldata.h"
#include "Ledit_Safety.h"
/* ldata.h defines assert(x) as (x); tests must use a real failing check. */
#undef assert
#define assert(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); exit(1); } } while (0)

typedef struct { double x, y; } Point;
union _LObject { struct { int shape, layer, datatype, deleted, curved; LPoint center; long radius; } data; };
struct _LSelection { LObject object; LSelection next; };
static union _LObject objects[4], replacement;
static struct _LSelection selections[4];
static int deletedCount, createdCount, failCreation, contextMissing;
static double unitMicrons = .001;
static LPoint routePoints[4], created[1000];
static const char *dialogOverride;
static int layerCount = 3, pickChoice = 0, failLayerCreate;
static LLayer lastCreatedLayer;
static int polygonCount;
static char layerNames[16][MAX_LAYER_NAME] = {"", "Metal", "Boundary_Helpers_1", "Boundary_Helpers_3"};
LPoint globalReferencePoint;

LCell LCell_GetVisible(void) { return contextMissing ? NULL : (LCell)1; }
LFile LCell_GetFile(LCell cell) { return cell ? (LFile)1 : NULL; }
double LFile_IntUtoMicrons(LFile file, LCoord value) { return value * unitMicrons; }
void LDialog_AlertBox(const char *msg) { (void)msg; }
int LDialog_MultiLineInputBox(const char *title, LDialogItem *items, int count) { if (dialogOverride) strcpy(items[0].value, dialogOverride); return 1; }
void LDisplay_Refresh(void) {}
LPoint LPoint_Set(LCoord x, LCoord y) { LPoint p; p.x = x; p.y = y; return p; }
LRect LObject_GetMbb(LObject object) { LRect r; r.x0 = object->data.center.x - object->data.radius; r.x1 = object->data.center.x + object->data.radius; r.y0 = object->data.center.y - object->data.radius; r.y1 = object->data.center.y + object->data.radius; return r; }
double LObject_Area(LObject object) { return M_PI * object->data.radius * object->data.radius; }
LCoord LObject_DistanceToPoint(LObject object, LPoint p, LFile file) { return (long)(hypot((double)p.x - object->data.center.x, (double)p.y - object->data.center.y) - object->data.radius); }
LLayer LLayer_GetCurrent(LFile file) { return (LLayer)1; }
LLayer LLayer_GetList(LFile file) { return layerCount ? (LLayer)1 : NULL; }
LLayer LLayer_GetNext(LLayer layer) { size_t i = (size_t)layer; return i < layerCount ? (LLayer)(i + 1) : NULL; }
LLayer LLayer_GetSpecial(LFile file, LSpecialLayer special) { return NULL; }
char *LLayer_GetName(LLayer layer, const char *name, const int maxlen) { snprintf((char *)name, maxlen, "%s", layerNames[(size_t)layer]); return (char *)name; }
LLayer LLayer_Find(LFile file, const char *name) { int i; for (i = 1; i <= layerCount; ++i) if (!strcmp(name, layerNames[i])) return (LLayer)(size_t)i; return NULL; }
LStatus LLayer_New(LFile file, LLayer preceding, const char *name) { if (failLayerCreate) return LCreateError; assert(!LLayer_Find(file, name) && layerCount < 15); strcpy(layerNames[++layerCount], name); return LStatusOK; }
int LDialog_PickList(const char *title, const char *choices[], int total, int defaultChoice) { assert(total == layerCount && defaultChoice == 0); assert(!strcmp(choices[0], "Metal")); return pickChoice; }
LLayer LObject_GetLayer(LCell cell, LObject object) { return (LLayer)(size_t)object->data.layer; }
LShapeType LObject_GetShape(LObject object) { assert(object && !object->data.deleted); return (LShapeType)object->data.shape; }
int LObject_GetGDSIIDataType(LObject object) { return object->data.datatype; }
LPoint LCircle_GetCenter(LObject object) { assert(object && !object->data.deleted); return object->data.center; }
LCoord LCircle_GetRadius(LObject object) { return object->data.radius; }
LRect LBox_GetRect(LObject object) { LRect r = {0}; return r; }
int LPolygon_HasCurve(LObject object) { return object->data.curved; }
LVertex LObject_GetVertexList(LObject object) { return NULL; }
LVertex LVertex_GetNext(LVertex vertex) { return NULL; }
LPoint LVertex_GetPoint(LVertex vertex) { return LPoint_Set(0, 0); }
long LVertex_GetCount(LObject object) { return 4; }
long LVertex_GetArray(LObject object, LPoint *out, const int maxpoints) { assert(maxpoints == 4); memcpy(out, routePoints, sizeof(routePoints)); return 4; }
LStatus LTorus_GetParams(LObject object, LTorusParams *p) { return LBadObject; }
LStatus LPie_GetParams(LObject object, LPieParams *p) { return LBadObject; }
LStatus LObject_Delete(LCell cell, LObject object) { assert(!object->data.deleted); object->data.deleted = 1; ++deletedCount; return LStatusOK; }
LSelection LSelection_GetList(void) { return selections; }
LSelection LSelection_GetNext(LSelection selection) { return selection->next; }
LObject LSelection_GetObject(LSelection selection) { return selection->object; }
LObject LCircle_New(LCell cell, LLayer layer, LPoint p, LCoord radius) { assert(createdCount < 1000); lastCreatedLayer = layer; created[createdCount++] = p; return &replacement; }
LObject LPolygon_New(LCell cell, LLayer layer, LPoint *points, const int count) { lastCreatedLayer = layer; ++polygonCount; return &replacement; }
LObject CopyObject(LObject object, long dx, long dy) { return LCircle_New((LCell)1, (LLayer)1, LPoint_Set(object->data.center.x + dx, object->data.center.y + dy), object->data.radius); }

static int allocations, failAllocation;
static void *checked_malloc(size_t bytes) { void *p; if (failAllocation) return NULL; p = malloc(bytes); if (p) ++allocations; return p; }
static void checked_free(void *p) { if (p) --allocations; free(p); }
#define malloc checked_malloc
#define free checked_free

#define TRANSFORM_STUB(name) LObject name(LObject o, LPoint p, double rad) { return failCreation ? NULL : &replacement; }
TRANSFORM_STUB(Mirror_Rect) TRANSFORM_STUB(Mirror_Circle) TRANSFORM_STUB(Mirror_Polygon)
TRANSFORM_STUB(Mirror_Torus) TRANSFORM_STUB(Mirror_Pie)
TRANSFORM_STUB(Rotate_Rect) TRANSFORM_STUB(Rotate_Circle) TRANSFORM_STUB(Rotate_Polygon)
TRANSFORM_STUB(Rotate_Torus) TRANSFORM_STUB(Rotate_Pie)

LPoint MirrorCoordinates(LPoint, LPoint, double);
int CalculateOffset(LObject, long, double, double *);
LObject MirrorObject(LObject, LPoint, double, int);
LObject RotateObject(LObject, LPoint, double, int);
int calculateIntersection(LPoint, LPoint, LPoint, LPoint, LPoint *);
int calculateCircleSegmentIntersection(LPoint, LPoint, LPoint, double, LPoint *, LPoint *);
double GetPointsDistance(LPoint, LPoint);
void DrawCircleByRouteDistance(double, long, int, LPoint, LObject);
bool Compare_Objects(LObject, LObject);
void Modifiy_Delete_Duplicates(void);
int Calculate_Outside_Circle(LPoint, LPoint, long, LPoint *);
double normalize_angle(double);
double get_short_arc_interpolated_angle(double, double);
int calculate_arc_points(long, long, long, double, double, LPoint **);
Point computeCentroid(Point *, int);
void computeInertiaMatrix(Point *, int, Point, double [2][2]);
double computeEccentricity(double [2][2]);
void fitCircle(Point *, int, Point *, double *);
void swap(LPoint *, LPoint *);
int compare(const void *, const void *);
LPoint CalculateCentroid(LPoint *, int);
LPoint *CalculateEllipsePoints(long, long, int, double);
int LSelection_GetNumber(LSelection);
void _ArrayInObjectByDistanceHexagon(LPoint **, int *, long *);
LLayer ChooseBoundaryLayer(LFile);
LLayer ChooseConversionLayer(LFile);
LLayer CreateHelperLayer(LFile);
void DrawPolygonByCenterRadiusAndEdgeNumber(LPoint, long, int, double, LLayer);
LPoint *Calculate_Rounded_Hexagon_Points(LPoint, LCoord, double);
void Conversion_To_Polygon(void);
void Conversion_To_Rounded_Hexagon(void);

int main(void)
{
    double v, offset[2], inertia[2][2], radius;
    LPoint p, q, out[2], *arc;
    Point pts[4] = {{100000, 0}, {0, 100000}, {-100000, 0}, {0, -100000}}, center;
    int i, count;
    assert(LM_Number(" 1.25 ", &v) && v == 1.25);
    assert(!LM_Number("", &v) && !LM_Number("abc", &v));
    assert(!LM_Number("1um", &v) && !LM_Number("nan", &v));
    assert(!LM_Number("inf", &v) && !LM_Number("1e999", &v));
    {
        LDialogItem item[1] = {{"Edge Number", "3.5"}};
        assert(!LM_Dialog("test", item, 1));
        strcpy(item[0].value, "3"); assert(LM_Dialog("test", item, 1));
        contextMissing = 1; assert(!LM_Context()); contextMissing = 0;
    }
    assert(fabs(LM_UnitsPerMicron() - 1000) < 1e-9);
    unitMicrons = .005; assert(fabs(LM_UnitsPerMicron() - 200) < 1e-9); unitMicrons = .001;
    p = MirrorCoordinates(LPoint_Set(10, 20), LPoint_Set(2, 3), M_PI / 2);
    assert(p.x == -6 && p.y == 20);
    p = MirrorCoordinates(LPoint_Set(10, 20), LPoint_Set(2, 3), 3 * M_PI / 2);
    assert(labs(p.x + 6) <= 1 && labs(p.y - 20) <= 1);
    objects[0].data.shape = LCircle; objects[0].data.radius = 100;
    assert(CalculateOffset(objects, 50, 0, offset) && offset[0] == 250 && offset[1] == 0);
    objects[0].data.shape = LTorus; assert(!CalculateOffset(objects, 50, 0, offset));
    objects[0].data.shape = LCircle; deletedCount = 0; failCreation = 1;
    assert(!MirrorObject(objects, p, 0, 0) && !deletedCount);
    failCreation = 0; assert(MirrorObject(objects, p, 0, 1) && !deletedCount);
    assert(RotateObject(objects, p, 0, 0) && deletedCount == 1);
    objects[0].data.deleted = 0;
    assert(calculateIntersection(LPoint_Set(0, 0), LPoint_Set(10, 0), LPoint_Set(5, -5), LPoint_Set(5, 5), &p));
    assert(p.x == 5 && p.y == 0);
    assert(!calculateCircleSegmentIntersection(LPoint_Set(1, 1), LPoint_Set(1, 1), LPoint_Set(0, 0), 5, &p, &q));
    assert(calculateCircleSegmentIntersection(LPoint_Set(-10, 5), LPoint_Set(10, 5), LPoint_Set(0, 0), 5, &p, &q) == 1);
    assert(Calculate_Outside_Circle(LPoint_Set(0, 0), LPoint_Set(600000, 0), 500000, out) == 2);
    assert(out[0].x == 300000 && labs(out[0].y) == 400000);
    assert(!Calculate_Outside_Circle(LPoint_Set(0, 0), LPoint_Set(0, 0), 1, out));
    count = calculate_arc_points(0, 0, 1, 0, 1, &arc); assert(count >= 2); free(arc);
    assert(calculate_arc_points(0, 0, -1, 0, 90, &arc) == -1 && !arc);
    center = computeCentroid(pts, 4); assert(center.x == 0 && center.y == 0);
    computeInertiaMatrix(pts, 4, center, inertia); assert(computeEccentricity(inertia) == 0);
    fitCircle(pts, 4, &center, &radius); assert(fabs(radius - 100000) < .001);
    pts[0].x = 500010; pts[0].y = 10; pts[1].x = 10; pts[1].y = 500010; pts[2].x = -299990; pts[2].y = -399990;
    fitCircle(pts, 3, &center, &radius);
    assert(fabs(center.x - 10) < .001 && fabs(center.y - 10) < .001 && fabs(radius - 500000) < .001);
    for (i = 0; i < 4; ++i) pts[i].x = pts[i].y = 0;
    fitCircle(pts, 4, &center, &radius); assert(radius == 0 && center.x == 0 && center.y == 0);
    routePoints[0] = LPoint_Set(0, 0); routePoints[1] = LPoint_Set(100, 0);
    routePoints[2] = LPoint_Set(100, 100); routePoints[3] = LPoint_Set(0, 100);
    createdCount = 0; DrawCircleByRouteDistance(50, 1, 8, routePoints[0], objects);
    assert(createdCount == 8 && created[1].x == 50 && created[1].y == 0 && created[7].x == 0 && created[7].y == 50);
    /* Three equal circles must delete each redundant handle exactly once. */
    for (i = 0; i < 4; ++i) {
        memset(&objects[i], 0, sizeof(objects[i])); objects[i].data.shape = LCircle;
        objects[i].data.layer = i == 3 ? 2 : 1;
        selections[i].object = &objects[i]; selections[i].next = i < 3 ? &selections[i + 1] : NULL;
    }
    deletedCount = 0; Modifiy_Delete_Duplicates();
    assert(deletedCount == 2 && !objects[0].data.deleted && !objects[3].data.deleted);
    {
        LPoint corners[4] = {{100000, 100000}, {100000, 700000}, {500000, 700000}, {500000, 100000}};
        p = CalculateCentroid(corners, 4); assert(p.x == 400000 && p.y == 300000);
        corners[0] = LPoint_Set(100, 50); corners[1] = LPoint_Set(200, 50); corners[2] = LPoint_Set(300, 50);
        p = CalculateCentroid(corners, 3); assert(p.x == 200 && p.y == 50);
        assert(!CalculateEllipsePoints(1, 1, 0, 0));
        arc = CalculateEllipsePoints(100, 50, 4, 0); assert(arc && arc[0].x == 100 && arc[1].y == 50); free(arc);
    }
    {
        long gap; int j;
        objects[0].data.deleted = objects[1].data.deleted = 0;
        objects[0].data.radius = 10; objects[1].data.radius = 100;
        selections[1].next = NULL; dialogOverride = "0"; createdCount = 0;
        _ArrayInObjectByDistanceHexagon(&arc, &count, &gap);
        assert(arc && count > 0 && count == createdCount && gap == 0);
        for (i = 0; i < count; ++i) {
            assert(arc[i].x != 0 || arc[i].y != 0);
            for (j = i + 1; j < count; ++j) assert(arc[i].x != arc[j].x || arc[i].y != arc[j].y);
        }
        free(arc); dialogOverride = NULL;
    }
    failAllocation = 1;
    assert(calculate_arc_points(0, 0, 100, 0, 90, &arc) == -1 && !arc);
    assert(!CalculateEllipsePoints(100, 100, 4, 0));
    createdCount = 0; DrawCircleByRouteDistance(50, 1, 8, routePoints[0], objects); assert(!createdCount);
    failAllocation = 0; assert(allocations == 0);
    pickChoice = 2; assert(ChooseBoundaryLayer((LFile)1) == (LLayer)3);
    assert(ChooseConversionLayer((LFile)1) == (LLayer)3);
    pickChoice = -1; assert(!ChooseBoundaryLayer((LFile)1));
    assert(!ChooseConversionLayer((LFile)1));
    failAllocation = 1; assert(!ChooseBoundaryLayer((LFile)1)); failAllocation = 0;
    assert(CreateHelperLayer((LFile)1) == (LLayer)4 && !strcmp(layerNames[4], "Boundary_Helpers_2"));
    assert(CreateHelperLayer((LFile)1) == (LLayer)5 && !strcmp(layerNames[5], "Boundary_Helpers_4"));
    failLayerCreate = 1; assert(!CreateHelperLayer((LFile)1) && layerCount == 5);
    pickChoice = 1; polygonCount = 0;
    Conversion_To_Polygon(); assert(polygonCount == 2 && lastCreatedLayer == (LLayer)2);
    polygonCount = 0;
    Conversion_To_Rounded_Hexagon(); assert(polygonCount == 2 && lastCreatedLayer == (LLayer)2);
    dialogOverride = "1"; createdCount = 0;
    Conversion_To_Rounded_Hexagon(); assert(createdCount == 2 && lastCreatedLayer == (LLayer)2);
    dialogOverride = NULL; pickChoice = -1; polygonCount = createdCount = 0;
    Conversion_To_Polygon(); Conversion_To_Rounded_Hexagon();
    assert(polygonCount == 0 && createdCount == 0);
    assert(allocations == 0);
    puts("PASS: target layer selection/cancellation, unique helper layer names, layer creation and allocation failure");
    puts("PASS: geometry, numeric validation, alternate units, failed allocation/creation, centroid, circle fit, unique hex array, route spacing, duplicate deletion, allocation balance");
    return 0;
}
