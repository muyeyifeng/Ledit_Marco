//*************************************************************************
//* IDENT: ldata.h
//* Copyright © Tanner Research, Inc. 2000-2009 All rights reserved
//* 
//* DESCRIPTION: UPI Link Library Header file 
//*************************************************************************

#if !defined(__LDATA_H)
#define __LDATA_H

// Version constants
#define UpiMajorVersion 11 /* Even though UPI code is forward compatible, programmer is free to use these constants */
#define UpiMinorVersion 0  /* to somehow make code more efficient when used with older versions of L-Edit.          */

// Automatic linking (for DLL's only)
#if defined(MAKE_DLL)
#	ifdef UPI_MACRO_USING_MFC
#		define RUNTIME_SUFFIX "s"
#	else
#		define RUNTIME_SUFFIX ""
#  endif
#	ifndef FILE_X64_SUFFIX
#		ifdef _WIN64
#			define FILE_X64_SUFFIX "64"
#		else
#			define FILE_X64_SUFFIX ""
#		endif
#	endif
#	ifndef FILE_DEBUG_SUFFIX
#		ifdef _DEBUG
#			define FILE_DEBUG_SUFFIX "d"
#		else
#			define FILE_DEBUG_SUFFIX ""
#		endif
#	endif
#	if defined(COMPILE_AS_V6)
#		define UPILINK_FILE "upilink" FILE_X64_SUFFIX FILE_DEBUG_SUFFIX RUNTIME_SUFFIX "_MSVC6.lib"
#	elif _MSC_VER == 1700
#		define UPILINK_FILE "upilink" FILE_X64_SUFFIX FILE_DEBUG_SUFFIX RUNTIME_SUFFIX "_MSVC2012.lib"
#	elif _MSC_VER == 1600
#		define UPILINK_FILE "upilink" FILE_X64_SUFFIX FILE_DEBUG_SUFFIX RUNTIME_SUFFIX "_MSVC2010.lib"
#	elif defined(COMPILE_AS_V2008) || (_MSC_VER == 1500)
#		define UPILINK_FILE "upilink" FILE_X64_SUFFIX FILE_DEBUG_SUFFIX RUNTIME_SUFFIX "_MSVC2008.lib"
#	elif defined(COMPILE_AS_V2005) || (_MSC_VER == 1400)
#		define UPILINK_FILE "upilink" FILE_X64_SUFFIX FILE_DEBUG_SUFFIX RUNTIME_SUFFIX "_MSVC2005.lib"
#	else
#		define UPILINK_FILE "upilink" FILE_X64_SUFFIX FILE_DEBUG_SUFFIX RUNTIME_SUFFIX "_MSVC2003.lib"
#	endif
#	if !defined(UPILINK_NOAUTOLINK)
#		pragma comment( lib, UPILINK_FILE )
#	endif
#	define LEDITAPI _stdcall
#	ifndef _UPI_PACKING
#		define _UPI_PACKING    8 
#	endif
#else
#	define LEDITAPI
#endif

// For GCC macro compiler
#if !defined(_SIZE_T_DEFINED)
#	ifdef _WIN64
		typedef unsigned long long size_t; // unsigned __int64 size_t; 
#	else
		typedef unsigned int size_t; 
#	endif
#	define _SIZE_T_DEFINED
#endif

// If needed, turn on special packing for UPI
#ifdef _UPI_PACKING
#	ifndef __GNUC__
#		pragma pack(push, _UPI_PACKING)
#	else
#		pragma pack(push, 8)
#	endif
#endif

/****************************************************************************************************************************************************************/
/*** TYPE AND CONSTANT DEFINITIONS                                                                                                                              */
/****************************************************************************************************************************************************************/

typedef struct _LFile*      LFile;
typedef struct _LCell*      LCell;
typedef union  _LObject*    LObject;
typedef struct _LInstance*  LInstance;
typedef struct _objListRec* LPortObjects;
typedef struct _LEntity*    LEntity;
typedef struct _LVertex*    LVertex;
typedef struct _LPort*      LPort;
typedef struct _LLabel*     LLabel;
typedef struct _LSelection* LSelection;
typedef struct _LLayer*     LLayer;
typedef struct _LPass*      LPass; 
typedef struct _LCore*      LCore;
typedef struct _LWindow*    LWindow;
typedef struct _LDrcRule*   LDrcRule;

typedef enum {LFALSE=0,LTRUE=1} LBoolean;

#define WORLD_MAX ((LCoord) 0x1fffffffL)

#define MAX_LAYER_NAME 256
#define MAX_CELL_NAME 256
#define MAX_TDBFILE_NAME 256

#define LOK 1
#define LCANCEL 0

#define LYES 1
#define LNO 0

#define LUPI_ERROR_MISSING_PARAMETER -101
#define LUPI_ERROR_INVALID_PARAMETER_TYPE -102
#define LUPI_ERROR_INVALID_PARAMETER_VALUE -103

typedef enum 
{
   LStatusOK=0,
   LTooManyInits=1,
   LOpenError=2,
   LCloseError=3,
   LCreateError=4,
   LSaveError=5,
   LBadFile=6,
   LBadCell=7,
   LBadLayer=8,
   LBadParameters=9,
   LBadObject=10,
   LBadHierarchy=11,
   LTmError=12,
   LUserDataError=13,
   LCellOverWritten=14,
   LLayerMapsDifferent=15,
   LNamedCellExists=16,
   LCopyProtViolation=17, 
   LNoSelection=18,
   LVertexNotFound=19,
   LCantDeleteVertex=20,
   LPropertyNotFound=21,
   LPropertyHasNoValue=22,
   LPropertyTypeMismatch=23,
   LBufferTooSmall=24,
   LSystemError=25,              // System errors like cannot load DLL.
   LPropertyConversionError=26,  // Error occurred during the export or import of properties to/from a file.
   LUserAbort=27,                // User aborted the operation such as DRC, Extract, etc.
   LExists=28,                   // Item added already exists (LDrcRule, LLayer, etc.)
   LLayerNotEmpty=29,            // Layer has objects on it.
   LParameterOutOfRange=30,
	LMaxRunTimeExceeded=31			// For DRC Automation
} LStatus;

typedef enum 
{
	LCellInMemory,		// Cell in memory that will not be saved to disk (such as AutoGen cell, for example).
	LCellNormal,			// Normal cell that is on disk as well as in memory
	LCellNew,				// New cell created in memory, but not yet explicitly saved to disk by the user (In OA it is saved implicitly after creation in order to reserve the rights to its name in multi-user environment. Such cell will be deleted, if design is closed before any explicit save.)
	LCellDeleted,			// Cell is flagged for deletion, but deletion is still not committed to disk.
	LCellUnresolved,    // Cell that is instanced, but cannot be read from disk.
	LCellDelayRead,		// Cell that is found on disk, but we chose to delay reading its content.
	LCellCycleBreaker,	// Cell that is used in place of another cell when instanced in order to break a dependency cycle.
	LCellReadInProgress	// Delay read in progress, object list is not valid
} LCellCacheStatus;

typedef struct 
{
   char prompt[40];
   char value[21];
} LDialogItem;

typedef enum 
{
   LSelectionTool = 0, 
   LBoxTool, 
   LPolygon90Tool, 
   LPolygon45Tool, 
   LPolygonAATool,
   LWire90Tool, 
   LWire45Tool, 
   LWireAATool,
   LCircleTool, 
   LPieWedgeTool, 
   LTorusTool,
   LPortTool, 
   LRuler90Tool, 
   LRuler45Tool, 
   LRulerAATool, 
   LInstanceTool,
	LPortPoly90Tool,
	LPortPoly45Tool,
	LPortPolyAATool,
	LLabelTool
} UPIDrawingToolType;

// Horizontal placement
#define PORT_TEXT_LEFT         0
#define PORT_TEXT_MIDDLE       1
#define PORT_TEXT_RIGHT        2
// Vertical placement
#define PORT_TEXT_TOP          0
#define PORT_TEXT_CENTER       4
#define PORT_TEXT_BOTTOM       8
// Orientation
#define PORT_TEXT_HORIZONTAL   0
#define PORT_TEXT_VERTICAL    16

// Horizontal placement
#define LABEL_TEXT_LEFT			0
#define LABEL_TEXT_MIDDLE		1
#define LABEL_TEXT_RIGHT		2
// Vertical placement
#define LABEL_TEXT_TOP			8
#define LABEL_TEXT_CENTER		4
#define LABEL_TEXT_BOTTOM		0
// Orientation
#define LABEL_TEXT_HORIZONTAL	0
#define LABEL_TEXT_VERTICAL	16

typedef enum 
{
	TT_In = 0,
	TT_Out,
	TT_Inout, 
	TT_Switch,
	TT_Jumper,
	TT_Unused,
	TT_Tristate
} LTerminalType;

#define PORT_ACCESS_DIRECTION_TOP		1
#define PORT_ACCESS_DIRECTION_LEFT		2
#define PORT_ACCESS_DIRECTION_RIGHT		4
#define PORT_ACCESS_DIRECTION_BOTTOM	8

typedef enum
{
   GridLayer,
   OriginLayer,
   CellOutlineLayer,
   ErrorLayer,
   IconLayer,
   FirstMaskLayer,
   DragBoxLayer
} LSpecialLayer;

typedef long LCoord;
typedef unsigned long LLen;
typedef struct
{
   LCoord y;
   LCoord x;
} LPoint;

typedef struct
{
   LCoord y0;
   LCoord x0;
   LCoord y1;
   LCoord x1;
} LRect;

typedef struct
{
   double x;
   double y;
} DPoint;

typedef float LOrientation_Ex99;
#define LNormalOrientation        0
#define LRotate0                  0
#define LRotate90                90
#define LRotate180              180
#define LRotate270              270
#define LRotate0MirrorX       (-360)
#define LRotate90MirrorX      ( -90)
#define LRotate180MirrorX     (-180)
#define LRotate270MirrorX     (-270)

typedef struct LMagnification 
{
   LLen num;
   LLen denom;
} LMagnification;

typedef struct
{
   LPoint translation;
   LOrientation_Ex99 orientation;
   LMagnification magnification;
} LTransform_Ex99;

typedef struct
{
   LPoint ptCenter;
   LCoord nInnerRadius;
   LCoord nOuterRadius;
   double dStartAngle;
   double dStopAngle;
} LTorusParams;

typedef struct
{
   LPoint ptCenter;
   LCoord nRadius;
   double dStartAngle;
   double dStopAngle;
} LPieParams;

typedef enum
{
   MICRONS,
   MILLIMETERS,
   CENTIMETERS,
   MILS,
   INCHES,
   LAMBDA,
   OTHER
} tech_unit_type;

typedef struct _LTechnologyEx840 
{
   char szName[128];
   tech_unit_type eUnitType;
   char szUnitName[128];
   long nNum;
   long nDenom;
   long nOtherNum;
   long nOtherDenom;
} LTechnologyEx840;

typedef enum
{
   LTdbFile,
   LCifFile,
   LGdsFile,
   LV6TdbFile
} LFileType;

typedef enum
{
   LBox, 
   LCircle,
   LWire,
   LPolygon,
   LTorus,
   LPie,
   LOtherObject,
   LObjInstance,
   LObjPort,
   LObjRuler,
	LObjLabel,
	LVia
} LShapeType;

typedef enum
{
   LOrthogonal,
   LFortyFive,
   LAllAngle,
   LCurved,
   LNonGeometric,
   LManhattan = LOrthogonal,
   LBoston = LFortyFive
} LGeomType;

typedef enum 
{
	CW, 
	CCW,
	INVALID // returned by LVertex_GetCurveExactCenter
} LArcDirection;

typedef enum
{
   LCapButt=0,
   LCapRound,
   LCapExtend
} LCapType;

typedef enum
{
   LJoinMiter=0,
   LJoinRound,
   LJoinBevel,
   LJoinLayout
} LJoinType;

typedef struct
{
   LCoord width;
   LJoinType join;
   LCapType cap;
   short miter_angle;
} LWireConfig;

typedef int LWireConfigBits;
const int LSetWireWidth	= 1 << 0;
const int LSetWireJoin	= 1 << 1;
const int LSetWireCap	= 1 << 2;
const int LSetWireMiterAngle	= 1 << 3;
const int LSetWireAll	= -1;

typedef struct _LExtractOptionsEx840
{
   // Data
   // General Options
   char szExtDefnFile[256];            // Extract definition file - Input.
   char szExtOutFile[256];             // Extract SPICE output file - Output.
   double dExtractBinSize;             // Bin size.  (Locator Units)
   LBoolean bLabelAllDevices;          // Create ports for all devices.
   char szDeviceLabelLayer[256];       // Place device labels on this layer.  Blank for the recognition layer.

   // Output Options
   LBoolean bWriteNodeNames;           // Write Node names in comments. 
   LBoolean bWriteDeviceCoord;         // Write device coordinates in comments.
   LBoolean bWriteShortedDevices;      // Write shorted devices in comments.
   LBoolean bWriteCapResWarnings;      // Write layer capacitance and resistance warnings.
   LBoolean bWriteParasiticCap;        // Write parasitic capacitances.
   double dParasiticCutoff;            // Cutoff value for parasitic capacitors.
   LBoolean bWriteNodesAsNames;        // Write nodes as (integers or names).
   LBoolean bWriteSciNotation;         // Write values in scientific notation.
   LBoolean bWriteVerboseSPICE;        // Write R, L, C with verbose style (R=, L=, C=).
   LBoolean bWriteSubCktDefs;          // Write empty subcircuit definitions.
   LBoolean bWriteENDStatement;        // Write the .END statement
   char* pszExtIncludeStmt;            // SPICE include statement.

   // Subcircuit Options
   LBoolean bSubCktRecognition;        // Recognize subcircuit instances.
   char szSubCktRecogLayer[256];       // Subcircuit recognition layer.
   LBoolean bUseSubCktNetlistFmt;      // Write netlist as a subcircuit.
   LBoolean bFlagImproperOverlaps;     // Flag improper overlaps.
   char szIgnoreConnPortLayer[256];    // Ignore connection ports on this layer.
   char szIgnoreConnPort[256];         // Ignore connection ports with this name.
   char szIgnoreCrossPort[256];        // Ignore cross ports with this name.

   // Miscellaneous
   long nMaxIncludeStmtLen;            // Length of the Include Statement string.
} LExtractOptionsEx840;

typedef struct _LEnvironment
{
   short MenuBackgroundColor;  // obsolete
   short MenuForegroundColor;  // obsolete
   short MenuSelectColor;      // obsolete
   short AlertBackgroundColor; // obsolete
   long  DefaultPortTextSize;
   int DropDownMenus;          // obsolete
   int ActivePushRubberbanding;
   int AutoPanning;
   int StatusBar;
   int HideInsides;
   short HorizontalPixels;
   short VerticalPixels;
} LEnvironment;

typedef struct _LSelectionParam 
{
   long selection_range;
   long deselect_distance_2;
   long deselect_distance_1;
   long lambda_edit_range;
   long pixel_edit_range;
   int select_draws;
} LSelectionParam;

typedef enum 
{
   LSnapping,
   LSmooth
} LCursorType;

typedef struct {
	long displayed_grid_size; /* In internal units */
	long min_grid_pixels;
	long displayed_majorgrid_size; /* In internal units */
	long min_majorgrid_pixels;
	long mouse_snap_grid_size_x; /* In internal units */
	long mouse_snap_grid_size_y; /* In internal units */
	long mouse_snap_grid_offset_x; /* In internal units */
	long mouse_snap_grid_offset_y; /* In internal units */
	LCursorType cursor_type;
	long locator_scaling;
	long manufacturing_grid_size; /* In internal units */
	LBoolean display_curves_using_manufacturing_grid;
} LGrid_v16_30;

typedef struct
{
   int poly_to_rect;
   int port_rect;
} LCIFParam;

typedef enum
{
   LHidden,
   LVisible
} LLayerViewStatus;

typedef struct
{
   long defaultWireWidth;
   short defaultWireMiterAngle;
   LCapType capType;
   LJoinType joinType;
} LWireParam;

typedef struct
{
   short LRed;
   short LBlue;
   short LGreen;
} LColor;

typedef enum _LOutlineStyle
{
   osUnknown = -1,
   osFirstOutlineStyle = 0,
   osSolid = 0, 
   osDotted = 1,
   osShort = 2, 
   osShortDot = 3,
   osLongDot = 4, 
   osLong = 5, 
   osLongDotDot = 6,
   osLongShortShort = 7,
   osLongLongShort = 8,
   osLastOutlineStyle = 8
}
LOutlineStyle;

#define NumberOfOutlineStyles (osLastOutlineStyle+1)

typedef enum _LRenderingMode
{
   rmPaint = 0,
   rmAdd = 1,
   rmSubtract = 2
}
LRenderingMode;

typedef enum _LOutlineUnitType
{
   utPixels = 0,
   utLocatorUnits = 1
}
LOutlineUnitType;

typedef unsigned char LStipple[8];

typedef struct _LRenderingAttribute
{
   LRenderingMode mMode;
   unsigned int   mPass;
   LStipple       mFillPattern;
   unsigned int   mFillColorIndex;
   LStipple       mOutlinePattern;
   unsigned int   mOutlineColorIndex;
   LOutlineStyle  mOutlineStyle;
   LOutlineUnitType  mOutlineWidthUnits;
   unsigned int   mOutlineWidth;
   unsigned char  mbBGprevPass;
}
LRenderingAttribute;

typedef LRenderingAttribute *LLRenderingAttribute;

typedef enum _LRenderingAttributeIndex
{
   raiFirstRenderingAttribute = 0,
   raiObject = 0,
   raiSelectedObject = 1,
   raiPortBox = 2,
   raiSelectedPortBox = 3,
   raiPortText = 4,
   raiSelectedPortText = 5,
   raiWireCenterline = 6,
   raiSelectedWireCenterline = 7,
   raiLastRenderingAttribute = 7
}
LRenderingAttributeIndex;

#define NumberOfRenderingAttributes  (raiLastRenderingAttribute+1)

typedef enum 
{
   LMIN_WIDTH,
   LEXACT_WIDTH,
   LOVERLAP,
   LEXTENSION,
   LNOT_EXISTS,
   LSPACING,
   LSURROUND,
   LDENSITY
} LDrcRuleType;

typedef struct _LDesignRuleParam
{
   int enable;
   char *name;
   LDrcRuleType rule_type;
   int ignore_coincidences;
   int ignore_intersections;
   int ignore_enclosures;
   int ignore_acute_angles;
   char *layer1;
   char *layer2;
   long distance;
   int use_internal_units;
} LDesignRuleParam;

typedef enum 
{
   LDOT_Bool=0,
   LDOT_Area=1,
   LDOT_Select=2,
   LDOT_Density=3,
   LDOT_External=4
} LDerivationType;

typedef struct _LDerivedLayerBoolOperation 
{
   char *src_layer1;
   char *src_layer2;
   char *src_layer3;
   int layer1_not_op;
   long layer1_grow_amount;
   int layer2_not_op; 
   long layer2_grow_amount;
   int layer3_not_op; 
   long layer3_grow_amount;
   int layer1_bool_layer2;
   int layer2_bool_layer3;
} LDerivedLayerBoolOperation;

typedef enum
{
   LDOST_Inside=0,
   LDOST_Outside,
   LDOST_Hole,
   LDOST_Cut,
   LDOST_Touch,
   LDOST_Enclose,
   LDOST_Overlap,
   LDOST_Vertex,
   LDOST_Density
} LSelectOperationRelationType;

typedef struct _LDerivedLayerSelectOperation 
{
   char *layer1;
   char *layer2;
   int not_flag; 
   int range_enabled_flag;
   int n1;
   int n2;
   double d1;
   double d2;
   LSelectOperationRelationType relation_type;   
} LDerivedLayerSelectOperation;

typedef struct _LDerivedLayerDensityOperation 
{
   char *layer1;
   char *layer2;
   int not_flag; 
   double d1;
   double d2;
} LDerivedLayerDensityOperation;

typedef enum
{
  LDOAT_Range=0,
  LDOAT_EQ=1
} LAreaCheckType;

typedef enum
{
  LDOUT_LocatorUnits=0,
  LDOUT_TechnologyUnits=1
} LAreaUnitType;

typedef struct _LDerivedLayerAreaOperation 
{
   char *layer1;
   int not_flag; 
   double n1;
   double n2;
   double area;
   LAreaCheckType area_check_type;   
   LAreaUnitType area_unit_type;
} LDerivedLayerAreaOperation;

typedef union _LDerivedLayerOperation 
{
   LDerivedLayerBoolOperation boolean;
   LDerivedLayerSelectOperation select;
   LDerivedLayerAreaOperation area;  
   LDerivedLayerDensityOperation density;
} LDerivedLayerOperation;

typedef struct _LDerivedLayerParamEx830 
{
   char *name;
   int enable_evaluation;  
   LDerivationType  derivation_type;
   LDerivedLayerOperation operation; 
} LDerivedLayerParamEx830;

typedef enum {
    UNKNOWN = 0,
    CELL_BROWSER,
    TEXT,
    LAYOUT,
    CROSS_SECTION,
    CODE,
	 LW_SPICE,
	 LW_LOG,
	 LW_COMMAND,
	 LW_HTML
} LWindowType;

typedef enum
{
   L_unassigned=0,
   L_none, 
   L_int, 
   L_real, 
   L_bool, 
   L_string, 
   L_enum, 
   L_byte,
   L_ptr,   
   L_blob
} LPropertyType;

typedef enum
{
   LTC_notexist=-1,
   LTC_bool,
   LTC_int,
   LTC_float,
   LTC_string,
   LTC_layer,
   LTC_hstretch,
   LTC_vstretch
} LTCellParameterType;

typedef struct
{
	char CIFName[7];           // CIF layer name
   short GDSNumber;           // GDSII layer number. -32,768 <= GDSNumber <= 32,767.  The GDSII standard requires 0 <= GDSNumber <= 255.     
   short GDSDataType;         // GDSII data type. -32,768 <= GDSDataType <= 32,767.  The GDSII standard requires 0 <= GDSNumber <= 255. A value of -1 indicates an unassigned GDSII data type.
   double AreaCapacitance;    // Area capacitance in aF/Sq. micron from the layer to the substrate.  0 <= AreaCapacitance or AreaCapacitance = -1.  A value of -1 indicates an unassigned area capacitance.
   double FringeCapacitance;  // Fringe capacitance in fF/micron from the layer to the substrate.  0 <= FringeCapacitance or FringeCapacitance = -1.  A value of -1 indicates an unassigned fringe capacitance.
   double Resistivity;        // Sheet resistance of the layer in ohms/square. 0 <= Resistivity.
   LBoolean Locked;           // Indicates whether the layer is locked.
   LBoolean Hidden;           // Indicates whether the layer is hidden.
   LWireParam WireParam;      // Default wire parameters of the layer.
	LBoolean Protected;			// Indicates whether the layer is protected.
} LLayerParamEx1512;

typedef struct 
{
   char CIFName[7];           // CIF layer name
   short GDSNumber;           // GDSII layer number. -32,768 <= GDSNumber <= 32,767.  The GDSII standard requires 0 <= GDSNumber <= 255.     
   short GDSDataType;         // GDSII data type. -32,768 <= GDSDataType <= 32,767.  The GDSII standard requires 0 <= GDSNumber <= 255. A value of -1 indicates an unassigned GDSII data type.
   double AreaCapacitance;    // Area capacitance in aF/Sq. micron from the layer to the substrate.  0 <= AreaCapacitance or AreaCapacitance = -1.  A value of -1 indicates an unassigned area capacitance.
   double FringeCapacitance;  // Fringe capacitance in fF/micron from the layer to the substrate.  0 <= FringeCapacitance or FringeCapacitance = -1.  A value of -1 indicates an unassigned fringe capacitance.
   double Resistivity;        // Sheet resistance of the layer in ohms/square. 0 <= Resistivity.
   LBoolean Locked;           // Indicates whether the layer is locked.
   LBoolean Hidden;           // Indicates whether the layer is hidden.
   LWireParam WireParam;      // Default wire parameters of the layer.
} LLayerParamEx830;

typedef unsigned int LMarker;

// Please note that there is also more complete structure LMarkerParam_V11.
// That structure gives finer control over parameters of a marker than LMarkerParam.
// Either structure can be used in marker functions.
typedef struct _LMarkerParam
{
	int StructureSize;		// Use sizeof(LMarkerParam) to assign value to this member
	int EdgeThickness;		// In pixels.  Default is 3.
	int EdgeStyle;				// PS_... values for Windows API for Pen GUI objects.  Default is 0 for solid.
	int CircleThickness;		// In pixels.  Default is 3.  0 means don't show the circle.
	int CrosshairThickness; // In pixels.  Default is 3.  0 means don't show the crosshair.
} LMarkerParam;

typedef enum
{
	LMarkerEdges,
	LMarkerPolygon,
	LMarkerOrthogonalCross,
	LMarkerDiagonalCross,
	LMarkerTriangleNorth,
	LMarkerTriangleEast,
	LMarkerTriangleSouth,
	LMarkerTriangleWest,
	LMarkerDiamond,
	LMarkerSquare,
	LMarkerCircle,
	LMarkerHorizontalCenterline,
	LMarkerVerticalCenterline,
	LMarkerCrossedSquare,
	LMarkerCrossedCircle,
	LMarkerMultiPolygon
} LMarkerStyle;

// Note that in order to use LMarkerParam_V11 structure in marker functions
// one must explicitly typecast its pointer to LMarkerParam*.
typedef struct _LMarkerParam_V11 
{
	int StructureSize;		// Use sizeof(LMarkerParam) to assign value to this member
	int EdgeThickness;		// In pixels.  Default is 3.
	int EdgeStyle;				// PS_... values for Windows API for Pen GUI objects.  Default is 0 for solid.
	int CircleThickness;		// In pixels.  Default is 3.  0 means don't show the circle.
	int CrosshairThickness; // In pixels.  Default is 3.  0 means don't show the crosshair.
	unsigned int EdgeColor;	// This is the edge color.  Windows COLORREF, returned by RGB() macro.
	LBoolean	Global;			// Toggled/removed by "universal" buttons. Used for DRCEN, SDL, and Node Highlighting.
	unsigned int FillColor;	// This is the fill color.  Windows COLORREF, returned by RGB() macro.
	unsigned short FillPattern[8]; // Fill pattern (make it all 0's for no-fill or all 0xff's for solid fill).
	LMarkerStyle Style;
} LMarkerParam_V11;

typedef struct 
{
   char     szDispUnitName[128];       
   double   dScaleFactor;              // Internal units divided by dScaleFactor equals Display units.
   int      nEditDecimalDigits;        // Number of decimal digits to use when formatting string for editing.
   int      nMouseGridDecimalDigits;   // Number of decimal digits to use when formatting string for display of mouse position.
} LDisplayUnitInfo;

typedef struct
{
   LBoolean bFlagAcuteAngles;
   LBoolean bFlagAllAngleEdges;
   LBoolean bFlagOffGridObjects;
} LDrcFlags;

typedef struct
{
	int nIndex;
	int nLowerLayerNumber;
	int nUpperLayerNumber;
} LCellViaInfo;

typedef enum
{
   LBoolOp_OR,
   LBoolOp_AND,
   LBoolOp_XOR,
   LBoolOp_NOT,
   LBoolOp_GROW,
   LBoolOp_SHRINK,
   LBoolOp_SUBTRACT
} LBooleanOperation;

typedef enum
{
   LDrcStatus_Needed,
   LDrcStatus_Passed,
   LDrcStatus_Failed
} LDrcStatus;

typedef struct 
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} LGUID;

/* GDS Export scope options */
typedef enum
{
	gdsExportAllCells,
	gdsExportActiveCell,
	gdsExportFabricationCell,
	gdsExportSpecifiedCell,
	gdsExportCellsFromLibraries,
} LGDSExportScope;

/* GDS Export options */
typedef struct 
{
	// Destination file name
	const char* cszDestFileName;

	//
	LBoolean bZipOutputFile;

	// Export scope value enumaration. See below for values.
	LGDSExportScope ExportScope;

	// Contains Cell Name if ExportScope is gdsExportSpecifiedCell
	const char* cszSpecifiedCell;
	
	// Libraries to include ExportScope is gdsExportCellsFromLibraries
	const char* const * cpszIncludeLibraries;

	// LTRUE if hierarchy of specified cells need to be included in the export
	// or LFALSE.
	LBoolean bIncludeHierarchy;

	// List of libraries to exclude (empty or NULL if none)
	const char * const * cpszExcludeLibraries;

	// Use default units
	LBoolean bUseDefaultUnits;

	// Following two values specify custom Units if bUserDefaultUnits is LFALSE
	double dMicrons;
	double dUserUnits;

	// 0 for PFile::GDS_CasePreserve, 1 for PFile::GDS_CaseUpper,
	// 2 for PFile::GDS_CaseLower
	int nUpcaseCellName;

	// Restrict cell name character count
	int nCellNameLength; // 32 - Standard

	// Map file name
	const char* cszMapFileName;

	//
	LBoolean bDoNotExportHiddenObjects; // Do not export hidden objects and layers
	LBoolean bOverwriteGDSIIDataType; // Overwrite object data type with layer data type
	LBoolean bCalcChecksum; // Calculate MOSIS checksum
	LBoolean bCheckSelfIntersections; // Check self-intersecting polygons and wires

	LBoolean bFracture; // Fracture polygons with more than nFractureLimit vertices
	int nFractureLimit;
} LGDSParamEx;

/* GDS Export logging options */
// Pass NULL instead of pointer to this structure to turn logging off
typedef struct
{
	const char* szLogFileName;
	LBoolean bOpenLogInWindow; // if not in QuietMode
} LGDSExportLogParams;

/* GDS and CIF Import */
typedef enum {
	cOverwriteAllCells = 0,
	cOverwriteCellsInTopDesignOnly,
	cDontOverwriteCells
} LOverwriteCellsScopeOnImport;

typedef enum {
	AUTOVIEWPORTCHANGE_NONE,
	AUTOVIEWPORTCHANGE_PAN,
	AUTOVIEWPORTCHANGE_ZOOM
} LAutoViewportChangeStyle;

typedef enum {
	cAcceptAllZ,
	cAcceptZeroZ,
	cAcceptZRange
} LProcessNonZeroElevation;

typedef enum {
	cRS274X,
	cWindowTrace,
	cRasterTrace
} LGerberFormat;

/****************************************************************************************************************************************************************/
/*** FUNCTION DECLARATIONS                                                                                                                                      */
/****************************************************************************************************************************************************************/

#ifdef __cplusplus
	extern "C" {
#endif 

// Dialog functions
void LEDITAPI LDialog_MsgBox( const char* msg );
int  LEDITAPI LDialog_MultiLineMsgBox( const char* ibuf[], int total_entries );
void LEDITAPI LDialog_AlertBox( const char* msg );
int  LEDITAPI LDialog_YesNoBox( const char* msg );
int  LEDITAPI LDialog_InputBox( const char* title, const char *s, char *ibuf );
int  LEDITAPI LDialog_MultiLineInputBox( const char *title, LDialogItem ibuf[], int total_entries );
int  LEDITAPI LDialog_PickList( const char* title, const char* picklist[], int total_entries, int default_choice );
void LEDITAPI LDialog_File( const char* szDefaultName, const char* szTitle, char* szFileNameBuffer,
                            const char* szFiltersForBrowser, int iBrowseType, const char* szMessage,
                            const char* szOkText, const char* szDefaultExt, const char* szTypeList,
                            const LFile pFile );

// Cursor and display functions
LPoint  LEDITAPI LCursor_GetPositionEx99( int iSnapped, int iPauseForInput, const char* szMessage );
LRect	LEDITAPI LCursor_GetRect( const char* szMessage );
void    LEDITAPI LDisplay_Refresh( void );
void    LEDITAPI LStatusBar_SetMsg( const char* msg );
void		LEDITAPI LUpi_LogMessage( const char* msg );
LStatus LEDITAPI LCell_HomeView( LCell cell );
LCell   LEDITAPI LCell_GetVisible( void );
LCell   LEDITAPI LCell_GetLastVisible( LFile file );
LStatus LEDITAPI LCell_MakeVisible( LCell cell );
LStatus LEDITAPI LCell_MakeVisibleNoRefresh( LCell cell );
void		LEDITAPI LCell_RefreshMarkers( LCell cell );

// UPI macro functions
void LEDITAPI LMacro_Register( const char* macro_desc, const char* function_name );
unsigned int LEDITAPI LMacro_BindToMenuAndHotKey_v9_30(const char* szMenu, const char* szHotKey, const char* szMacroDescription, const char* szFunctionName, const char* szHotKeyCategory);
int                LEDITAPI LMacro_IsLoaded( const char* dll_path );
int                LEDITAPI LMacro_Load(const char* szDLL_Path);
int                LEDITAPI LMacro_LoadEx1200(const char* szDLL_Path, LBoolean bLoadAtStartUp);
void               LEDITAPI LMacro_UnLoad( const char* dll_path );
LCell              LEDITAPI LMacro_GetNewTCell( void );
long               LEDITAPI LUpi_GetSerialNumber( void );
LStatus            LEDITAPI LUpi_SetQuietMode( int val );
int                LEDITAPI LUpi_InQuietMode( void );
void               LEDITAPI LUpi_SetSelectionTool( void );
UPIDrawingToolType LEDITAPI LUpi_SetDrawingTool( UPIDrawingToolType eTool );
void               LEDITAPI LUpi_InsertMenuItemSeparator( const char* menu );
void               LEDITAPI LUpi_SetReturnCode( int nCode );
int                LEDITAPI LUpi_GetReturnCode( void );
LStatus            LEDITAPI LUpi_SetUpdateDisplayMode( int val );
int                LEDITAPI LUpi_GetUpdateDisplayMode( void );
LBoolean			  LEDITAPI LUpi_SetByPassCheckingMode(LBoolean bByPass);
LBoolean			  LEDITAPI LUpi_GetByPassCheckingMode(void);
const char*        LEDITAPI LFormat( const char* lpszFormat, ...);
const char*        LEDITAPI LFormatV( const char* lpszFormat, char** argList);
double				  LEDITAPI LAtoF( const char* szString );

// Window functions
LWindow      LEDITAPI LWindow_GetVisible( void );
LWindow      LEDITAPI LWindow_GetNext( LWindow pWindow );
LWindow      LEDITAPI LWindow_GetList( void );
LStatus      LEDITAPI LWindow_MakeVisible( LWindow pWindow );
LStatus      LEDITAPI LWindow_Close( LWindow pWindow );
LStatus      LEDITAPI LWindow_CloseAll( void );
LWindowType  LEDITAPI LWindow_GetType( LWindow pWindow );
LCell        LEDITAPI LWindow_GetCell( LWindow pWindow );
LStatus      LEDITAPI LWindow_GetEditTransform( LWindow pWindow, LTransform_Ex99* pEditTransform );
LCell        LEDITAPI LWindow_GetTopCell( LWindow pWindow );
LFile        LEDITAPI LWindow_GetFile( LWindow pWindow );
int          LEDITAPI LWindow_IsLast( LWindow pWindow );
LStatus      LEDITAPI LWindow_SaveToFile( LWindow pWindow, const char* cszFileName );
unsigned int LEDITAPI LWindow_GetText( LWindow pWindow, char* szBuffer, unsigned int nBufferLength );
LStatus      LEDITAPI LWindow_SetText( LWindow pWindow, const char* cszText );
LWindow      LEDITAPI LWindow_LoadTextFile( const char* cszFileName, LWindowType eWindowType );
LWindow      LEDITAPI LWindow_NewTextWindow( const char* cszFileName, LWindowType eWindowType );
unsigned int LEDITAPI LWindow_GetName( LWindow pWindow, char* szFileName, unsigned int nBufferLength );
LStatus      LEDITAPI LWindow_SetName( LWindow pWindow, const char* cszFileName );
LStatus		  LEDITAPI LWindow_SetTextCursorLocationAt(LWindow pWindow, const char* cszString);
LStatus		  LEDITAPI LWindow_SetTextCursorLocation(LWindow pWindow, long nLine, long nColumn);
LStatus      LEDITAPI LWindow_SetTextSelection(LWindow pWindow, LPoint ptStartPos, LPoint ptEndPos);
LStatus      LEDITAPI LWindow_EditInPlacePushIn( LWindow pWindow, LInstance pInstance );
LStatus      LEDITAPI LWindow_EditInPlacePopOut( LWindow pWindow );
LStatus	     LEDITAPI LWindow_EditInPlacePopToTop( LWindow pWindow );
LStatus		  LEDITAPI LWindow_CopyToClipboard(LWindow pWindow);
LStatus      LEDITAPI LWindow_SaveImageToFile(LWindow pWindow, const char* cszFileName);
LStatus      LEDITAPI LCell_SaveImageToFile(LCell pCell, LRect* pCellRect, const char* cszFileName, int nImageWidthInPixels, int nImageHeightInPixels, int nDpi);

void         LEDITAPI LFile_DisplayCellBrowser( LFile pTDBFile );
LWindow      LEDITAPI LFile_OpenCell( LFile pTDBFile, const char* cszCellName );
#if defined(MAKE_DLL)
   void  LEDITAPI LWindow_GetParameters( void **phInst, void **phWnd, void **phLib );
   void* LEDITAPI LWindow_GetWindowHandle( LWindow pWindow );
#endif
LAutoViewportChangeStyle LEDITAPI LWindow_GetAutoViewportChangeStyle();
void LEDITAPI LWindow_SetAutoViewportChangeStyle( LAutoViewportChangeStyle nStyle );

// Cross-section viewer
void LEDITAPI LCSV_Run( LCell inCell );

// Application functions (accessors)
LBoolean LEDITAPI LApp_GetCacheInstances( void );
int      LEDITAPI LApp_GetCacheInstancesSmallerThanNumOfPixels( void );
LBoolean LEDITAPI LApp_GetFillObjectsDuringDrawing( void );
LStatus  LEDITAPI LApp_GetHideInstanceInsidesIfLessThanNumOfPixels( LPoint* pptPixelSize );
int      LEDITAPI LApp_GetHideObjectsSmallerThanNumOfPixels( void );
LBoolean LEDITAPI LApp_GetHideSmallInstanceInsides( void );
LBoolean LEDITAPI LApp_GetHideSmallObjects( void );
LBoolean LEDITAPI LApp_GetInterruptableRendering( void );
LBoolean LEDITAPI LApp_GetRedrawAllWindows( void );
LStatus  LEDITAPI LApp_GetVersion( char* szVersionString, int nBufferLen );
LStatus  LEDITAPI LApp_GetVersionDateTime( char* szVersionString, int nBufferLen );
LStatus  LEDITAPI LApp_GetFullVersion( char* szVersionString, int nBufferLen );
LStatus  LEDITAPI LApp_GetShortVersion( char* szVersionString, int nBufferLen );
LBoolean LEDITAPI LApp_GetShowDesignWhileRendering( void );
int      LEDITAPI LApp_GetShowDesignFirstTimeIncrement( void );
int      LEDITAPI LApp_GetShowDesignNextTimeIncrement( void );
LBoolean LEDITAPI LApp_GetRenderingUseCPUForColorMixing( void );
LBoolean LEDITAPI LApp_GetRenderingUseMMX( void );
LBoolean LEDITAPI LApp_GetRenderingUsePatBltForPatterns( void );
LBoolean LEDITAPI LApp_GetAllowSelectionOnLockedLayers( void );
// Application functions (modifiers)
LStatus  LEDITAPI LApp_SetCacheInstances( LBoolean bCacheInstances );
LStatus  LEDITAPI LApp_SetCacheInstancesSmallerThanNumOfPixels( int iPixelSize );
LStatus  LEDITAPI LApp_SetFillObjectsDuringDrawing( LBoolean bFillObjectDuringDrawing );
LStatus  LEDITAPI LApp_SetHideInstanceInsidesIfLessThanNumOfPixels( const LPoint* pptPixelSize );
LStatus  LEDITAPI LApp_SetHideObjectsSmallerThanNumOfPixels( int iPixelSize );
LStatus  LEDITAPI LApp_SetHideSmallInstanceInsides( LBoolean bHideSmallInstanceInsides );
LStatus  LEDITAPI LApp_SetHideSmallObjects( LBoolean bHideSmallObjects );
LStatus  LEDITAPI LApp_SetInterruptableRendering( LBoolean bInterruptRendering );
LStatus  LEDITAPI LApp_SetRedrawAllWindows( LBoolean bRedrawAllWindows );
LStatus  LEDITAPI LApp_SetShowDesignWhileRendering( LBoolean bShowDesignWhileRendering);
LStatus  LEDITAPI LApp_SetShowDesignTimeIncrement(int nFirstIncrement, int nNextIncrement );
LStatus  LEDITAPI LApp_SetRenderingUseCPUForColorMixing( LBoolean bUseCPUForColorMixing );
LStatus  LEDITAPI LApp_SetRenderingUseMMX( LBoolean bUseMMX );
LStatus  LEDITAPI LApp_SetRenderingUsePatBltForPatterns( LBoolean bUsePatBltForPatterns );
LStatus  LEDITAPI LApp_SetAllowSelectionOnLockedLayers( LBoolean bInterruptRendering );
void     LEDITAPI LApp_ExitAfterCompletion( LBoolean bDiscardUnsavedFiles );

// File functions
LFile            LEDITAPI LFile_New( LFile setup_file, const char* name );
LFile            LEDITAPI LFile_Open( const char* filename, LFileType type );
LStatus          LEDITAPI LFile_Save( LFile file );
LStatus          LEDITAPI LFile_SaveAs( LFile file, const char* name, LFileType type );
LStatus          LEDITAPI LFile_Close( LFile file );
LFile            LEDITAPI LFile_Find( const char* name );
LFile            LEDITAPI LFile_GetList( void );
LFile            LEDITAPI LFile_GetNext( LFile file );
int              LEDITAPI LFile_GetLock( LFile file );
int              LEDITAPI LFile_SetLock( LFile file, int set );
int              LEDITAPI LFile_IsChanged( LFile file );
char*            LEDITAPI LFile_GetName( LFile file, char* name, const int maxlen );
char*            LEDITAPI LFile_GetAuthor( LFile file, char* author, const int maxlen );
const char*      LEDITAPI LFile_SetAuthor( LFile file, const char* author );
LCell            LEDITAPI LFile_GetFabricationCell( LFile file );
LStatus          LEDITAPI LFile_SetFabricationCell( LFile file, LCell cell );
char*            LEDITAPI LFile_GetOrganization( LFile file, char* org, const int maxlen );
const char*      LEDITAPI LFile_SetOrganization( LFile file, const char* org );
void             LEDITAPI LFile_GetLayoutVersion( LFile file, long* major, long* minor );
void             LEDITAPI LFile_SetLayoutVersion( LFile file, long major, long minor );
void             LEDITAPI LFile_GetSetupVersion( LFile file, long* major, long* minor );
void             LEDITAPI LFile_SetSetupVersion( LFile file, long major, long minor );
char*            LEDITAPI LFile_GetInfoText( LFile file, char* info, const int maxlen );
const char*      LEDITAPI LFile_SetInfoText( LFile file, const char* info );
LEnvironment*    LEDITAPI LFile_GetEnvironment( LFile file, LEnvironment *lenvironment );
LStatus          LEDITAPI LFile_SetEnvironment( LFile file, LEnvironment *lenvironment );
LStatus          LEDITAPI LFile_GetGrid_v16_30( LFile file, LGrid_v16_30 *grid );
LStatus          LEDITAPI LFile_SetGrid_v16_30( LFile file, LGrid_v16_30 *grid );
LSelectionParam* LEDITAPI LFile_GetSelectionParam( LFile file, LSelectionParam *param );
LStatus          LEDITAPI LFile_SetSelectionParam( LFile file, LSelectionParam *param );
LStatus          LEDITAPI LFile_SetUserData( LFile file, void* dataPointer );
void*            LEDITAPI LFile_GetUserData( LFile file );
LStatus          LEDITAPI LFile_DeleteUserData( LFile file );
LStatus          LEDITAPI LFile_ClearUserData( LFile pTDBFile );
LStatus          LEDITAPI LFile_SetLastCurrent( LFile file, LCell cell );
LStatus          LEDITAPI LFile_GetResolvedFileName( LFile pTDBFile, const char* szRelativeFileName, char* szAbsoluteFileNameBuffer, int iBufferSize );
LFile            LEDITAPI LFile_GetVisible( void );
void             LEDITAPI LFile_SetChanged( LFile pTDBFile );
LStatus          LEDITAPI LFile_GetDisplayUnitInfo( LFile file, LDisplayUnitInfo *pDispUnitInfo );
LStatus          LEDITAPI LFile_SetDisplayUnit( LFile file, const char* szDispUnitName );
double           LEDITAPI LFile_IntUtoDispU(LFile pFile, LCoord lcValue);
LCoord           LEDITAPI LFile_DispUtoIntU(LFile pFile, double dValue);
double           LEDITAPI LFile_IntUtoMicrons(LFile pFile, LCoord lcValue);
LCoord           LEDITAPI LFile_MicronsToIntU(LFile pFile, double dValue);
double           LEDITAPI LFile_IntUtoTechU(LFile pFile, LCoord lcValue);
LCoord           LEDITAPI LFile_TechUtoIntU(LFile pFile, double dValue);
LCoord				LEDITAPI LFile_GetNudgeAmount(LFile pTDBFile);
void					LEDITAPI LFile_SetNudgeAmount(LFile pTDBFile, LCoord nNudgeAmount);
LStatus				LEDITAPI LFile_ImportOnlineDRCRules(LFile pTDBFile);
LStatus				LEDITAPI LFile_GetGUID(LFile pTDBFile, LGUID* pGuid);
int					LEDITAPI LFile_IsOpenAccess(LFile pTDBFile);
LStatus				LEDITAPI LFile_ExportGDSII(LFile file, LGDSParamEx* pGdsParam, LGDSExportLogParams* pGdsExportLogParam);
LStatus				LEDITAPI LFile_ImportGDSII(LFile pFile, const char* pcszSrcFile, LBoolean bUseGDSDataType, LOverwriteCellsScopeOnImport OverwriteCells, 
															LBoolean bGdsFileResolution, double dCustomResolution, const char* pcszLogFileName);
LStatus				LEDITAPI	LFile_ImportCIF(LFile pFile, const char* pcszSrcFile, LBoolean bPolygonAsRect, 
														 LOverwriteCellsScopeOnImport OverwriteCells, const char* pcszLogFileName);
LStatus				LEDITAPI LFile_ImportDXF( LFile pFile, 
                                             const char* pcszSrcFile,
                                             double dScale,
                                             LProcessNonZeroElevation NonZeroElevation,
                                             double dMinElevation,
                                             double dMaxElevation,
                                             LBoolean bMergeOpenPolylines,
                                             LCoord nMergeTolerance,
                                             LBoolean bIgnoreFills,
                                             LBoolean bIgnoreAttributes,
                                             LOverwriteCellsScopeOnImport OverwriteCells,
                                             const char* pcszLogFileName); // can be NULL
LStatus				LEDITAPI LFile_ImportGerberFolder( LFile pFile, 
                                                const char* pcszSrcFolder,
																const char* pcszSrcExtn,
																int	nDigitsLeftOfDecimal,
																int	nDigitsRightOfDecimal,
                                                LOverwriteCellsScopeOnImport OverwriteCells,
                                                const char* pcszLogFileName); // can be NULL
LStatus				LEDITAPI LFile_ImportGerberFile( LFile pFile, 
                                                const char* pcszSrcFile,
																LLayer pLayer,		// default layer
																int	nDigitsLeftOfDecimal,
																int	nDigitsRightOfDecimal,
                                                LOverwriteCellsScopeOnImport OverwriteCells,
                                                const char* pcszLogFileName); // can be NULL
LStatus				LEDITAPI LFile_ImportTechnology_GDSLayerMap( LFile pFile, 
                                                const char* pcszSrcFile );
LStatus				LEDITAPI LFile_ImportTechnology_LEF( LFile pFile, 
                                                const char* pcszSrcFile,
																const char* pcszTargetLibrary,
																LBoolean bImportLayerResistancesAndCapacitances );

// Cell functions
LCell         LEDITAPI LCell_New( LFile file, const char* name );
LCell         LEDITAPI LCell_NewEx( LFile file, const char* cellname, const char* viewname, const char* libname );
LCell         LEDITAPI LCell_NewTemp( LFile file, LCell cell );
LStatus       LEDITAPI LCell_Delete( LCell cell );
LStatus       LEDITAPI LCell_Copy( LFile sourceFile, LCell sourceCell, LFile destFile, const char* destCellName );
LStatus       LEDITAPI LCell_CopyEx( LCell sourceCell, LFile destFile, const char* destCellName, const char* destViewName, const char* destLibName );
LStatus       LEDITAPI LCell_Copy_XRef( LFile sourceFile, LCell sourceCell, LFile destFile, const char* destCellName );
LCell         LEDITAPI LCell_Find( LFile file, const char* name );
LCell         LEDITAPI LCell_FindEx( LFile file, const char* name );
LCell         LEDITAPI LCell_FindEx2( LFile file, const char* cellname, const char* viewname, const char* libname );
LFile         LEDITAPI LCell_GetFile( LCell cell );
LCell         LEDITAPI LCell_GetList( LFile file );
LCell         LEDITAPI LCell_GetNext( LCell cell );
int           LEDITAPI LCell_GetLock( LCell cell );
int           LEDITAPI LCell_SetLock( LCell, int set );
int           LEDITAPI LCell_Reserve( LCell cell, char* szErrMsg, const int cnErrMsgMaxLen );
LStatus		  LEDITAPI LCell_Unreserve( LCell cell, LBoolean bSaveIfChanged );
char*         LEDITAPI LCell_GetName( LCell cell, char* name, const int maxlen );		// cell0
char*			  LEDITAPI LCell_GetFullName( LCell cell, char* name, const int maxname );	// cell0*viewifnotlayout:lib or cell0:lib
char*			  LEDITAPI LCell_GetCellName( LCell cell, char* name, const int maxname );	// cell0
char*			  LEDITAPI LCell_GetViewName( LCell cell, char* name, const int maxname );	// viewifnotlayout or layout
char*			  LEDITAPI LCell_GetLibName( LCell cell, char* name, const int maxname );	// lib
char*			  LEDITAPI LCell_GetPresentationName( LCell cell, char* name, const int maxname, LBoolean bCapitalize );	// cell: cell0 library: lib
char*			  LEDITAPI LCell_GetCanonicalName( LCell cell, char* name, const int maxname );	// minimum unambiguous name
LStatus		  LEDITAPI LCell_SetLibName( LCell cell, const char* newname );
LStatus       LEDITAPI LCell_SetName( LFile file, LCell cell, const char* newname );
char*         LEDITAPI LCell_GetAuthor( LCell cell, char* author, const int maxlen );
const char*   LEDITAPI LCell_SetAuthor( LCell cell, const char* author );
char*         LEDITAPI LCell_GetOrganization( LCell cell, char* org, const int maxlen );
const char*   LEDITAPI LCell_SetOrganization( LCell cell, const char* org );
char*         LEDITAPI LCell_GetInfoText( LCell cell, char* info, const int maxlen );
const char*   LEDITAPI LCell_SetInfoText( LCell cell, const char* info );
int           LEDITAPI LCell_IsChanged( LCell cell );
long				LEDITAPI LCell_GetModifiedTime( LCell pCell );
long				LEDITAPI LCell_GetCreatedTime( LCell pCell ); 
LStatus			LEDITAPI LCell_GetVersion( LCell pCell, long * major, long *minor );
LStatus			LEDITAPI LCell_SetVersion( LCell pCell, long major, long minor );  
LRect         LEDITAPI LCell_GetMbb( LCell cell );
LRect         LEDITAPI LCell_GetMbbAll( LCell cell );
LCell         LEDITAPI LCell_Flatten( LCell cell );
LStatus       LEDITAPI LCell_ClearUserData( LCell pCell );
LStatus       LEDITAPI LCell_SetUserData( LCell cell, void* dataPointer );
void*         LEDITAPI LCell_GetUserData( LCell cell );
LStatus       LEDITAPI LCell_DeleteUserData( LCell cell );
void          LEDITAPI LCell_SetChanged( LCell pCell );
LStatus       LEDITAPI LCell_ClearUndoLists( LCell cell );
LStatus       LEDITAPI LCell_SetView( LCell cell, LRect view );
LRect         LEDITAPI LCell_GetView( LCell cell );
void          LEDITAPI LCell_CalcMBB( LCell pCell );
LMarker       LEDITAPI LCell_AddMarker( LCell pCell, const char* cszText, int nNumOfVertices, LBoolean bPolygon, const LPoint* cpnVertices, const LMarkerParam* cpParam );
LStatus		  LEDITAPI LCell_GetMarkerParam(LCell pCell, LMarker Bullseye, LMarkerParam* pParam);
LStatus		  LEDITAPI LCell_SetMarkerParam(LCell pCell, LMarker Bullseye, const LMarkerParam* cpParam);
void          LEDITAPI LCell_RemoveMarker( LCell pCell, LMarker MarkerHandle );
void          LEDITAPI LCell_RemoveAllMarkers( LCell pCell );
void			  LEDITAPI LMarker_SetShowGlobal( LBoolean bShow );
LBoolean		LEDITAPI LMarker_GetShowGlobal();
void				LEDITAPI LCell_RemoveGlobalMarkers(LCell pCell);
LBoolean		LEDITAPI LCell_GlobalMarkerExists(LCell pCell);
LBoolean      LEDITAPI LCell_GetShowInLists( LCell cell );
LStatus       LEDITAPI LCell_SetShowInLists( LCell cell, LBoolean show );
LStatus       LEDITAPI LCell_BooleanOperation( LCell pCell, LBooleanOperation nOp, LCoord Amount, LObject* ArrayOfObjectsA, unsigned int nNumOfObjectsA, LObject* ArrayOfObjectsB, unsigned int nNumOfObjectsB, LLayer pResultLayer, LBoolean bDeleteInputs );
LStatus       LEDITAPI LCell_Slice( LCell pCell, LObject* pObjectArray, unsigned int nNumOfObjects, const LPoint* cpPoint1, const LPoint* cpPoint2 );
LBoolean      LEDITAPI LCell_IsFromTdbLib( LCell cell );
LCellCacheStatus LEDITAPI LCell_GetCacheStatus(LCell pCell);
LStatus		  LEDITAPI LCell_ClearContents(LCell pCell);
LStatus       LEDITAPI LCell_MakeLogo( LCell pCell, const char* szLogoString, LCoord nTextSize, LLayer pLayer, LBoolean bPrintDate, LBoolean bPrintMaskCopyright, LBoolean bUnderline,
													LCoord CenterPointX, LCoord CenterPointY, LBoolean bPrintCompanyLogo, LBoolean bSnapToGrid,
													LBoolean bCreateAsInstance, const char* cszLogoLibrary, const char* cszLogoCell, const char* cszLogoView,
													const char* cszCharacterSetFileName );
LStatus       LEDITAPI LCell_MakeLogoEx1630( LCell pCell, const char* szLogoString, LCoord nTextSize, LLayer pLayer, LBoolean bPrintDate, LBoolean bPrintMaskCopyright, LBoolean bUnderline,
													LCoord CenterPointX, LCoord CenterPointY, LBoolean bPrintCompanyLogo, LBoolean bSnapToGrid,
													LBoolean bCreateAsInstance, const char* cszLogoLibrary, const char* cszLogoCell, const char* cszLogoView,
													const char* cszCharacterSetFileName, int nAlignment );
LStatus LEDITAPI LCell_ExportDXF( LCell pCell, const char* pcszOutputFile, LBoolean bFlatten, 
											LBoolean bExportWiresAsPolylines, LBoolean bRasterizeCurvedObjects );
LStatus LEDITAPI LCell_ExportGerber( LCell pCell, const char* pcszOutput, LGerberFormat nFormat, LBoolean bOneFilePerLayer, 
													 LBoolean bUseInches, int nDigitsLeftOfDecimal, int nDigitsRightOfDecimal, LBoolean bWriteWiresAsPolygons );
LStatus LEDITAPI LCell_ExportGerberDrill( LCell pCell, const char* pcszOutput, LLayer pLayer );


// T-Cell functions
LBoolean LEDITAPI LCell_IsTCell(LCell cell);
LBoolean LEDITAPI LCell_IsGenerator(LCell cell); // T-Cell or via-def
LCell LEDITAPI LCell_GetGenerator(LCell cell); // Returns non-null, if and only if 'cell' is an auto-gen
LStatus			LEDITAPI LCell_GetTCellDefaultValue(LCell pCell, const char* cszParameterName, char* szBuffer, int nBufferLength);
LStatus			LEDITAPI LCell_SetTCellDefaultValue(LCell pCell, const char* cszParameterName, const char* cszDefaultValue);
LStatus			LEDITAPI LCell_GetTCellPreviousValue(LCell pCell, const char* cszParameterName, char* szBuffer, int nBufferLength);
LStatus			LEDITAPI LCell_AddTCellParameter(LCell pCell, const char* cszParameterName, LTCellParameterType nType, const char* cszDefaultValue);
const char *         LEDITAPI LCell_GetParameterValue( LCell cell, const char* lpszParamName );
LTCellParameterType  LEDITAPI LCell_GetParameterType( LCell cell, const char* lpszParamName );
const char *         LEDITAPI LCell_GetParameterName( LCell cell, const char * lpszParamName );

LLayer   LEDITAPI LCell_GetParameterAsLayer( LCell pCell, const char * lpszParamName );
LCoord   LEDITAPI LCell_GetParameterAsCoord( LCell pCell, const char * lpszParamName, double scale );
LCoord   LEDITAPI LCell_GetParameterAsCoordOnGrid( LCell pCell, const char * lpszParamName, double scale );
double   LEDITAPI LCell_GetParameterAsDouble( LCell pCell, const char * lpszParamName );
LBoolean LEDITAPI LCell_GetParameterAsBoolean( LCell pCell, const char * lpszParamName );
int      LEDITAPI LCell_GetParameterAsInt( LCell pCell, const char * lpszParamName );

LStatus  LEDITAPI LCell_MoveOrigin( LCell pCell, LCoord NewOriginPointX, LCoord NewOriginPointY );

// Instance functions
LInstance       LEDITAPI LInstance_GetList( LCell cell );
LInstance       LEDITAPI LInstance_GetNext( LInstance instance );
LInstance       LEDITAPI LInstance_Find( LCell cell, const char* name );
LInstance       LEDITAPI LInstance_FindNext( LInstance instance, const char* name );
LRect           LEDITAPI LInstance_GetMbb( LInstance instance );
LCell           LEDITAPI LInstance_GetCell( LInstance instance );
LTransform_Ex99 LEDITAPI LInstance_GetTransform_Ex99( LInstance instance );
LPoint          LEDITAPI LInstance_GetRepeatCount( LInstance instance );
LPoint          LEDITAPI LInstance_GetDelta( LInstance instance );
char*           LEDITAPI LInstance_GetName( LInstance instance, char* name, const int maxlen );
LInstance       LEDITAPI LInstance_New_Ex99( LCell cell, LCell instance_cell, LTransform_Ex99 transform, LPoint repeat_cnt, LPoint delta );
LStatus         LEDITAPI LInstance_SetName( LCell cell, LInstance instance, const char* name );
LStatus         LEDITAPI LInstance_Set_Ex99( LCell cell, LInstance instance, LTransform_Ex99 transform, LPoint repeat_cnt, LPoint delta );
LStatus         LEDITAPI LInstance_Delete( LCell cell, LInstance instance );
LInstance       LEDITAPI LInstance_Generate( LCell cell, LCell instance_cell, ...);
LInstance       LEDITAPI LInstance_GenerateV( LCell cell, LCell instance_cell, const char* const* argList );
int				 LEDITAPI LInstance_GetLock( LInstance instance );
int				 LEDITAPI LInstance_SetLock( LInstance instance, int set );

// Property functions
LStatus      LEDITAPI LEntity_PropertyExists( const LEntity entity, const char* name );
LStatus      LEDITAPI LEntity_GetPropertyType( const LEntity entity, const char* name, LPropertyType* type );
unsigned int LEDITAPI LEntity_GetPropertyValueSize( const LEntity entity, const char* name );
LStatus      LEDITAPI LEntity_GetPropertyValue( const LEntity entity, const char* name, void* value, unsigned int max_size );
LStatus      LEDITAPI LEntity_AssignProperty( LEntity entity, const char* name, LPropertyType type, const void* value );
LStatus      LEDITAPI LEntity_AssignBlobProperty( LEntity entity, const char* name, const void* value, unsigned int size );
LStatus      LEDITAPI LEntity_DeleteProperty( LEntity entity, const char* name );
LStatus      LEDITAPI LEntity_DeleteAllProperties( LEntity entity );
LStatus      LEDITAPI LEntity_CopyAllProperties( LEntity target_entity, LEntity source_entity );
const char*  LEDITAPI LEntity_GetFirstProperty( const LEntity entity );
const char*  LEDITAPI LEntity_GetNextProperty( void );
void         LEDITAPI LEntity_SetCurrentProperty( const char* name );
LStatus      LEDITAPI LEntity_BrowseProperties( LEntity entity );
LStatus      LEDITAPI LEntity_LoadBlobProperty( LEntity entity, const char* name, const char* file_name );
LStatus      LEDITAPI LEntity_SaveBlobProperty( const LEntity entity, const char* name, const char* file_name );
LStatus      LEDITAPI LEntity_StoreAsCompressedBlob( LEntity pEntity, const char* szFileName, const char* szBlobPropName, const char* szUncompressedSizePropName );
LStatus      LEDITAPI LEntity_DecompressBlobToFile( LEntity pEntity, const char* szFileName, const char* szBlobPropName, const char* szUncompressedSizePropName );
LStatus      LEDITAPI LEntity_StringToValidPropertyName( const char* szString, char* szPropertyName, int iBufferSize );
LStatus      LEDITAPI LEntity_ValidPropertyNameToString( const char* szPropertyName, char* szString, int iBufferSize );
LStatus      LEDITAPI LEntity_WritePropertiesToFile( const LEntity pEntity, const char* szPath, const char* szFilename );
LStatus      LEDITAPI LEntity_ReadPropertiesFromFile( LEntity pEntity, const char* szPath, const char* szFilename );

// Object functions
LObject    LEDITAPI LObject_GetList( LCell cell, LLayer layer );
LObject    LEDITAPI LObject_GetNext( LObject object );
LRect      LEDITAPI LObject_GetMbb( LObject object );
LShapeType LEDITAPI LObject_GetShape( LObject object );
LGeomType  LEDITAPI LObject_GetGeometry( LObject object );
LStatus    LEDITAPI LObject_Delete( LCell cell, LObject object );
LLayer     LEDITAPI LObject_GetLayer( LCell cell, LObject object );
LStatus    LEDITAPI LObject_ChangeLayer( LCell pCell, LObject pObject, LLayer pNewLayer );
double     LEDITAPI LObject_Area( LObject object );
double     LEDITAPI LObject_Perimeter( LObject object );
void       LEDITAPI LObject_Transform_Ex99( LObject object, LTransform_Ex99 transform );
LInstance  LEDITAPI LObject_GetInstance( LObject pObject );
int		  LEDITAPI LObject_GetGDSIIDataType( LObject pObject );
LStatus    LEDITAPI LObject_SetGDSIIDataType( LObject pObject, short iGDSIIDataType );
LStatus    LEDITAPI LObject_SetGDSIIDataTypeEx( LObject pObject, int iGDSIIDataType );
const char* LEDITAPI LObject_GetNetName( LObject pObject ); // Return value freed automatically when macro ends
LStatus	  LEDITAPI LObject_SetNetName( LObject pObject, const char* cszNetName );
LStatus    LEDITAPI LObject_ChangeLayer( LCell pCell, LObject pObject, LLayer pNewLayer );
LStatus    LEDITAPI LObject_ConvertToPolygon( LCell pCell, LObject* ArrayOfObjects, unsigned int nNumOfObjects );
LObject    LEDITAPI LObject_Copy(LCell pCell, LLayer pLayer, LObject pObject);
LCoord     LEDITAPI LObject_DistanceToPoint(LObject pObject, LPoint ptPoint, LFile pTDBFile);
LBoolean	LEDITAPI LObject_SnapToGrid(LObject pObject, LCoord nGridSize);
LBoolean	LEDITAPI LObject_SnapToMfgGrid(LObject pObject, LFile pTDBFile);


// Polygon functions
LObject LEDITAPI LPolygon_New( LCell cell, LLayer layer, LPoint point_arr[], const int npoints );
LObject LEDITAPI LPolygon_Create( LCell cell, LLayer layer, LPoint* pPoints, double* pBulges, const int npoints );
LStatus LEDITAPI LPolygon_StraightenAllCurves( LCell cell, LObject object );
LStatus LEDITAPI LPolygon_RemoveAllCurves( LObject object );
int     LEDITAPI LPolygon_HasCurve( LObject object );

// Vertex functions
LVertex LEDITAPI LObject_GetVertexList( LObject object );
LVertex LEDITAPI LVertex_GetNext( LVertex vertex );
LPoint  LEDITAPI LVertex_GetPoint( LVertex vertex);
LStatus LEDITAPI LVertex_SetPoint( LVertex vertex, LPoint point );
LStatus LEDITAPI LVertex_Add( LObject object, LVertex prev_vertex, LPoint point );
LStatus LEDITAPI LVertex_Delete( LObject object, LVertex vertex );
long    LEDITAPI LVertex_GetCount( LObject object );
long    LEDITAPI LVertex_GetArray( LObject object, LPoint point_arr[], const int maxpoints );
long    LEDITAPI LVertex_GetBulgeArray( LObject object, double bulge_arr[], const int maxbulges );
int     LEDITAPI LVertex_HasCurve( LVertex vertex );
LStatus LEDITAPI LVertex_AddCurve( LObject object, LVertex vertex, LPoint ptCenter, LArcDirection dir );
LStatus LEDITAPI LVertex_RemoveCurve( LObject object, LVertex vertex );
LStatus LEDITAPI LVertex_GetCurve( LVertex vertex, LPoint *ptCenter, LArcDirection *dir );
LStatus LEDITAPI LVertex_SetCurve( LVertex vertex, LPoint ptCenter, LArcDirection dir );
LStatus LEDITAPI LVertex_GetCurveEX( LObject object, LVertex vertex, LPoint *ptCenter, LCoord *radius, LPoint *start, LPoint *end, LArcDirection *dir );
DPoint  LEDITAPI LVertex_GetCurveExactCenter( LObject object, LVertex vertex, LCoord radius, LArcDirection *dir );

// Box functions
LObject LEDITAPI LBox_New( LCell cell, LLayer layer, LCoord x0, LCoord y0, LCoord x1, LCoord y1 );
LRect   LEDITAPI LBox_GetRect( LObject object );
LStatus LEDITAPI LBox_Set( LCell cell, LObject object, LRect box );

// Wire functions
LObject   LEDITAPI LWire_New( LCell cell, LLayer layer, LWireConfig* config, LWireConfigBits bits, LPoint point_arr[], const int npoints );
LCoord    LEDITAPI LWire_GetWidth( LObject object );
LJoinType LEDITAPI LWire_GetJoinType( LObject object );
LCapType  LEDITAPI LWire_GetCapType( LObject object );
short     LEDITAPI LWire_GetMiterAngle( LObject object );
LStatus   LEDITAPI LWire_SetWidth(LCell pCell, LObject pObject, LCoord nWidth);
LStatus   LEDITAPI LWire_SetJoinType(LCell pCell, LObject pObject, LJoinType eJoinType);
LStatus   LEDITAPI LWire_SetCapType(LCell pCell, LObject pObject, LCapType eCapType);
LStatus   LEDITAPI LWire_SetMiterAngle(LCell pCell, LObject pObject, short nAngle);
double    LEDITAPI LWire_GetLength(LObject pObject);
double    LEDITAPI LWire_GetSquares(LObject pObject);
double    LEDITAPI LWire_GetResistanceEx(LLayer pLayer, LObject pObject);


// Port functions
LPort		LEDITAPI LPort_New( LCell cell, LLayer layer, const char* text, LCoord x0, LCoord y0, LCoord x1, LCoord y1 );
LPort		LEDITAPI LPort_GetList( LCell cell );
LPort		LEDITAPI LPort_GetNext( LPort port );
LLayer	LEDITAPI LPort_GetLayer( LPort port );
LRect		LEDITAPI LPort_GetMbb( LPort port );
LRect		LEDITAPI LPort_GetRect( LPort port );
char*		LEDITAPI LPort_GetText( LPort port, char* text, const int maxlen );
LCoord	LEDITAPI LPort_GetTextSize( LPort pPort );
LStatus	LEDITAPI LPort_SetTextSize( LPort pPort, LCoord lcTextSize );
int		LEDITAPI LPort_GetTextAlignment( LPort pPort );
LStatus	LEDITAPI LPort_SetTextAlignment( LPort pPort, int nAlignment );
LStatus	LEDITAPI LPort_Set( LCell cell, LPort port, LRect rect );
LStatus	LEDITAPI LPort_SetText( LCell cell, LPort port, const char* text, LCoord textSize );
LStatus	LEDITAPI LPort_Delete( LCell cell, LPort port );
LPort		LEDITAPI LPort_Find( LCell cell, const char* text );
LPort		LEDITAPI LPort_FindNext( LPort port, const char* text );
LPort		LEDITAPI LPort_New_With_Object( LCell cell, const char* text, LObject object );
LStatus	LEDITAPI LPort_AddObject( LCell cell, LPort port, LObject object );
LStatus	LEDITAPI LPort_RemoveObject( LCell cell, LPort port, LObject object );
LPort		LEDITAPI LObject_GetPort( LCell cell, LObject object );
LBoolean LEDITAPI LPort_IsLegacy( LPort port );
LStatus	LEDITAPI LPort_DeletePortOnly( LCell cell, LPort port );
LStatus	LEDITAPI LPort_SetTerminalType( LCell cell, LPort port, LTerminalType nTerminalType );
LTerminalType LEDITAPI LPort_GetTerminalType( LPort port );
LStatus	LEDITAPI LPort_SetAccessDirection( LCell cell, LPort port, int nAccessDirection );
int		LEDITAPI LPort_GetAccessDirection( LPort port );
void		LEDITAPI LPort_SetDefaultTextAlignment( int nDefaultTextAlignment );
int		LEDITAPI LPort_GetDefaultTextAlignment( void );
char*		LEDITAPI LPort_GetMustConnect(LPort pPort, char* szMustConnect, int iMaxLen);
LStatus	LEDITAPI LPort_SetMustConnect(LCell pCell, LPort pPort, const char* cszMustConnect);
char*		LEDITAPI LPort_GetPinName(LPort pPort, char* szPinName, const int cnMaxLen);

// Iterate object list of a port (slower iterator, but simpler to use)
LObject	LEDITAPI LPort_GetObjectsList( LPort port );
LObject	LEDITAPI LPort_GetObjectsNext( LPort port, LObject object );

// Iterate object list of a port (faster iterator and gives access to objects' layer)
LPortObjects LEDITAPI LPort_GetObjectsListEx(LPort pPort);
LPortObjects LEDITAPI LPort_GetObjectsNextEx(LPortObjects pPortObjects);
LObject	LEDITAPI LPort_GetObject(LPortObjects pPortObjects);
LLayer	LEDITAPI LPort_GetObjectLayer(LPortObjects pPortObjects);



// Label functions
LLabel LEDITAPI LLabel_GetList( LCell cell );
LLabel LEDITAPI LLabel_GetNext( LLabel label );
LLayer LEDITAPI LLabel_GetLayer( LLabel label );
LLabel LEDITAPI LLabel_New( LCell cell, LLayer layer, const char* name, LCoord x0, LCoord y0 );
char* LEDITAPI LLabel_GetName( LLabel label, char* name, const int maxname );
LStatus LEDITAPI LLabel_SetName( LCell cell, LLabel label, const char* name, LCoord textSize );
LPoint LEDITAPI LLabel_GetPosition( LLabel label );
LCoord LEDITAPI LLabel_GetTextSize( LLabel label );
LStatus LEDITAPI LLabel_SetTextSize(LCell cell, LLabel label, LCoord lcTextSize);
int LEDITAPI LLabel_GetTextAlignment(LLabel label);
LStatus LEDITAPI LLabel_SetTextAlignment(LCell cell, LLabel label, int nAlignment);

// Circle functions
LObject LEDITAPI LCircle_New( LCell cell, LLayer layer, LPoint ptCenter, LCoord radius );
LRect   LEDITAPI LCircle_GetRect( LObject object );
LPoint  LEDITAPI LCircle_GetCenter( LObject object );
LCoord  LEDITAPI LCircle_GetRadius( LObject object );
LStatus LEDITAPI LCircle_Set( LCell cell, LObject object, LPoint ptCenter, LCoord radius );

// Torus functions
LObject LEDITAPI LTorus_CreateNew( LCell pCell, LLayer pLayer, LTorusParams *pParams );
LStatus LEDITAPI LTorus_SetParams( LCell pCell, LObject pTorus, LTorusParams *pParams );
LStatus LEDITAPI LTorus_GetParams( LObject pTorus, LTorusParams *pParams );

// Pie functions
LObject LEDITAPI LPie_CreateNew( LCell pCell, LLayer pLayer, LPieParams *pParams );
LStatus LEDITAPI LPie_SetParams( LCell pCell, LObject pPie, LPieParams *pParams );
LStatus LEDITAPI LPie_GetParams( LObject pPie, LPieParams *pParams );

// Selection functions
LSelection LEDITAPI LSelection_GetList( void );
LObject    LEDITAPI LSelection_GetObject( LSelection selection );
LSelection LEDITAPI LSelection_GetNext( LSelection selection );
LSelection LEDITAPI LSelection_SelectAll( void );
void       LEDITAPI LSelection_DeselectAll( void );
LStatus    LEDITAPI LSelection_AddObject( LObject obj );
LStatus    LEDITAPI LSelection_RemoveObject( LObject obj );
LStatus    LEDITAPI LSelection_AddAllObjectsOnLayer( LLayer layer );
LStatus    LEDITAPI LSelection_RemoveAllObjectsOnLayer( LLayer layer );
LStatus    LEDITAPI LSelection_AddAllObjectsInRect( const LRect *box );
LStatus    LEDITAPI LSelection_RemoveAllObjectsInRect( const LRect *box );
LStatus    LEDITAPI LSelection_RotateAroundPoint( double angle, LCoord x, LCoord y, LBoolean bRelativeToCenter );
LStatus    LEDITAPI LSelection_Move( long dx, long dy );
LStatus    LEDITAPI LSelection_Cut( void );
LStatus    LEDITAPI LSelection_Copy( void );
LStatus    LEDITAPI LSelection_Clear( void );
LStatus    LEDITAPI LSelection_Paste( void );
LStatus    LEDITAPI LSelection_PasteToLayer( LLayer layer);
LStatus    LEDITAPI LSelection_Duplicate( void );
LStatus    LEDITAPI LSelection_Group( const char* group_cell_name );
LStatus    LEDITAPI LSelection_UnGroup( void );
LStatus    LEDITAPI LSelection_ChangeLayer( LLayer srcLayer, LLayer dstLayer );
LStatus    LEDITAPI LSelection_Merge( void );
LStatus    LEDITAPI LSelection_SliceHorizontal( const LPoint *point );
LStatus    LEDITAPI LSelection_SliceVertical( const LPoint *point );
LStatus    LEDITAPI LSelection_Rotate( void );
LStatus    LEDITAPI LSelection_FlipHorizontal( void );
LStatus    LEDITAPI LSelection_FlipVertical( void );
LStatus    LEDITAPI LSelection_Flatten( void );
LLayer     LEDITAPI LSelection_GetLayer( LSelection pSelection );
LStatus		LEDITAPI LSelection_SnapToMfgGrid(void);
LStatus		LEDITAPI LSelection_ConvertToCurvedPolygon(void);
LStatus		LEDITAPI LSelection_ConnectSegments( LCoord nTolerance );

// Layer functions
#define INVALID_LAYER_NUMBER 0x50badbad
#define INVALID_PURPOSE_NUMBER 0x50badbad
#define TANNER_PURPOSE_NUMBER 1111
LStatus      LEDITAPI LLayer_New( LFile file, LLayer preceding_layer, const char *name );
LLayer       LEDITAPI LLayer_GetList( LFile file );
LLayer       LEDITAPI LLayer_GetNext( LLayer layer );
LLayer       LEDITAPI LLayer_Find( LFile file, const char* name );
LLayer       LEDITAPI LLayer_FindByNames( LFile file, const char* name, const char* purpose );
LLayer       LEDITAPI LLayer_FindByNumbers( LFile file, int layer_number, int purpose_number );
LLayer		 LEDITAPI LLayer_FindGDS( LFile file, long gdsnum, long datatype );
int			 LEDITAPI LLayer_FindLayerNumber( LFile pFile, const char* cszLayerName );
int			 LEDITAPI LLayer_FindPurposeNumber( LFile pFile, const char* cszPurposeName );
char*			 LEDITAPI LLayer_FindLayerName( LFile pFile, const int cnLayerNumber, char* szName, const int cnMaxLen );
char*			 LEDITAPI LLayer_FindPurposeName( LFile pFile, const int cnPurposeNumber, char* szPurposeName, const int cnMaxLen );
char*        LEDITAPI LLayer_GetName( LLayer layer, const char* name, const int maxlen );
LStatus      LEDITAPI LLayer_SetName( LLayer layer, const char *name );
int          LEDITAPI LLayer_GetLayerNumber( LLayer layer );
int          LEDITAPI LLayer_GetPurposeNumber( LLayer layer );
LLayer       LEDITAPI LLayer_GetSpecial( LFile file, LSpecialLayer specialLayer );
LStatus      LEDITAPI LLayer_SetSpecial( LFile file, LSpecialLayer specialLayer, LLayer layer );
double       LEDITAPI LLayer_GetCap( LLayer layer );
double       LEDITAPI LLayer_GetFringeCap( LLayer layer );
double       LEDITAPI LLayer_GetRho( LLayer layer );
LStatus      LEDITAPI LLayer_SetCap( LLayer layer, double cap );
LStatus      LEDITAPI LLayer_SetFringeCap( LLayer layer, double cap );
LStatus      LEDITAPI LLayer_SetRho( LLayer layer, double rho );
LLayer       LEDITAPI LLayer_PrecedingLayerEx99( LFile file, LLayer layer );
LStatus      LEDITAPI LLayer_Delete( LFile file, LLayer layer );
LStatus      LEDITAPI LLayer_SetCurrent( LFile file, LLayer layer );
LLayer       LEDITAPI LLayer_GetCurrent( LFile file );
LStatus      LEDITAPI LLayer_GetParametersEx1512(LLayer pLayer, LLayerParamEx1512* pLayerParam);
LStatus      LEDITAPI LLayer_SetParametersEx1512(LLayer pLayer, const LLayerParamEx1512* pLayerParam);
LStatus      LEDITAPI LLayer_MoveLayer(LFile pTDBFile, LLayer pLayerToMove, LLayer pLayerNewLocation);
LLayer       LEDITAPI LLayer_Copy(LLayer pLayer);

// Generated (derived) layers functions
LStatus	 LEDITAPI LCell_GenerateLayers_v11_10(LCell pCell, const char* szCommandFile, const char** pszArrayOfLayerNames, 
									unsigned int nNumberOfLayers, LBoolean bMergeObjectsAfterGeneration);
LStatus                  LEDITAPI LCell_GenerateLayers_v10_00( LCell pCell, LLayer* ArrayOfLayers, unsigned int nNumberOfLayers, LBoolean bClearAllGeneratedLayers, LBoolean bMergeObjectsAfterGeneration );
LStatus                  LEDITAPI LCell_ClearGenerateLayers( LCell cell );
LDerivedLayerParamEx830* LEDITAPI LLayer_GetDerivedParametersEx830( LLayer  layer, LDerivedLayerParamEx830* param );
LStatus                  LEDITAPI LLayer_SetDerivedParametersEx830( LFile file, LLayer layer, LDerivedLayerParamEx830* param );
LStatus                  LEDITAPI LLayer_DestroyDerivedParameterEx840(LDerivedLayerParamEx830* pDerivedLayerParam);
LLayer                   LEDITAPI LLayer_GetDerivedList( LFile file );
LLayer                   LEDITAPI LLayer_GetDerivedNext( LLayer layer );
int                      LEDITAPI LLayer_IsDerived( LLayer layer );
LStatus                  LEDITAPI LLayer_EnableAllDerived( LFile file );
LStatus                  LEDITAPI LLayer_DisableAllDerived( LFile file );

// Layer rendering setup functions
LStatus     LEDITAPI LLayer_GetRenderingObjectName( LLayer layer, LRenderingAttributeIndex index, char *nameBuf, int nameBufSize );
LStatus     LEDITAPI LLayer_GetRenderingAttribute( LLayer layer, LRenderingAttributeIndex index, LLRenderingAttribute pRA );
LStatus     LEDITAPI LLayer_SetRenderingAttribute( LLayer layer, LRenderingAttributeIndex index, LLRenderingAttribute pRA );
LStatus     LEDITAPI LFile_GetColorPalette( LFile file, LColor *pColor, int index );
int         LEDITAPI LFile_GetColorPaletteNumColors( LFile file );
const char* LEDITAPI LFile_GetColorPaletteSortBy( LFile file );
LStatus     LEDITAPI LFile_SetColorPalette( LFile file, const LColor *pColor, int index );
LStatus     LEDITAPI LFile_SetColorPaletteNumColors( LFile file, int numcolors );
LStatus     LEDITAPI LFile_SetColorPaletteSortBy( LFile file, const char *sortby );

// Technology setup functions
LStatus     LEDITAPI LFile_GetTechnologyEx840( LFile pTDBFile, LTechnologyEx840* pTechnology );
LStatus     LEDITAPI LFile_SetTechnologyEx840( LFile pTDBFile, LTechnologyEx840* pTechnology );
const char* LEDITAPI LFile_SetTechnologyName( LFile file, const char* name );
LStatus     LEDITAPI LFile_SetTechnologyUnitName( LFile file, const char* name );
LStatus     LEDITAPI LFile_SetTechnologyUnitNum( LFile file, long num );
LStatus     LEDITAPI LFile_SetTechnologyUnitDenom( LFile file, long denom );
LStatus     LEDITAPI LFile_SetTechnologyLambdaNum( LFile file, long num );
LStatus     LEDITAPI LFile_SetTechnologyLambdaDenom( LFile file, long denom );

// CIF file import/export setup functions
LCIFParam* LEDITAPI LFile_GetCIFParameters( LFile file, LCIFParam *cifparam );
LStatus    LEDITAPI LFile_SetCIFParameters( LFile file, LCIFParam *cifparam );

// Geometrical transformation functions
LTransform_Ex99 LEDITAPI LTransform_Set_Ex99( LCoord xtrans, LCoord ytrans, LOrientation_Ex99 orient, LMagnification mag );
LTransform_Ex99 LEDITAPI LTransform_Zero_Ex99( void );
LTransform_Ex99 LEDITAPI LTransform_Add_Ex99( LTransform_Ex99 transform_to_be_added, LTransform_Ex99 current_transform );
LTransform_Ex99 LEDITAPI LTransform_Subtract_Ex99( LTransform_Ex99 transform_to_be_subtracted, LTransform_Ex99 current_transform );
LTransform_Ex99 LEDITAPI LTransform_GetInverse(LTransform_Ex99 oTransform);
LPoint          LEDITAPI LPoint_Transform_Ex99( LPoint point, LTransform_Ex99 transform );
LRect           LEDITAPI LRect_Transform_Ex99( LRect rect, LTransform_Ex99 transform );

// Point functions
LPoint LEDITAPI LPoint_Set( LCoord lX, LCoord lY );
LPoint LEDITAPI LPoint_Add( LPoint ptA, LPoint ptB );
LPoint LEDITAPI LPoint_Subtract( LPoint ptA, LPoint ptB );

// Rectangle functions
LRect LEDITAPI LRect_Set( LCoord x0, LCoord y0, LCoord x1, LCoord y1 );

// DRC functions
LStatus           LEDITAPI LCell_RunDRC( LCell pCell, const LRect* pDRCArea, unsigned int* pnNumErrors );
LStatus           LEDITAPI LCell_RunDRCCommandFile( LCell pCell, const char* szCommandFile, const LRect* pDRCArea, unsigned int* pnNumErrors );
LStatus           LEDITAPI LDrcRule_SetRuleSet( LFile file, const char *rule_set );
LStatus           LEDITAPI LDrcRule_SetTolerance( LFile file, long tolerance );
LDrcRule          LEDITAPI LDrcRule_GetList( LFile file );
LDrcRule          LEDITAPI LDrcRule_GetNext( LDrcRule rule );
LDrcRule          LEDITAPI LDrcRule_Find( LFile file, LDrcRuleType rule_type, const char *layer1, const char *layer2 );
LDesignRuleParam* LEDITAPI LDrcRule_GetParameters( LDrcRule rule, LDesignRuleParam *param );
LStatus           LEDITAPI LDrcRule_DestroyParameter(LDesignRuleParam* pDesignRuleParam);
LStatus           LEDITAPI LDrcRule_SetParameters( LFile file, LDrcRule rule, LDesignRuleParam *param );
LStatus           LEDITAPI LDrcRule_Delete( LFile file, LDrcRule rule );
LDrcRule          LEDITAPI LDrcRule_Add( LFile file, LDrcRule preceding_rule, LDesignRuleParam *param );
LStatus           LEDITAPI LFile_GetDrcFlags( LFile file, LDrcFlags* pDrcFlags );
LStatus           LEDITAPI LFile_SetDrcFlags( LFile file, const LDrcFlags* pDrcFlags );
LWindow           LEDITAPI LCell_OpenDRCSummary(LCell pCell);
LWindow           LEDITAPI LCell_OpenDRCStatistics(LCell pCell);
unsigned int      LEDITAPI LCell_GetDRCNumErrors(LCell pCell);
LDrcStatus        LEDITAPI LCell_GetDRCStatus(LCell pCell);
LStatus           LEDITAPI LCell_LoadResultsIntoDRCErrorNavigator(LCell pCell, const char* szFileName, LBoolean bShowBrowserEvenIfNoErrors);
LWindow           LEDITAPI LCell_OpenRouterLog(LCell pCell);

// Extract functions
LStatus LEDITAPI LExtract_Run( LCell topCell, const char *extDefFile, const char *spiceOutFile, int writeNodeName, int writeNodeCapacitance );
LStatus LEDITAPI LExtract_Run_Dialog( LCell cell );
LStatus LEDITAPI LExtract_RunEx840( LCell pTopCell, LExtractOptionsEx840* pExtOptions );
LStatus LEDITAPI LExtract_GetOptionsEx840( LCell pCell, LExtractOptionsEx840* pExtOptions );
LStatus LEDITAPI LExtract_SetOptionsEx840( LCell pCell, LExtractOptionsEx840* pExtOptions );
LWindow LEDITAPI LCell_OpenExtractSummary(LCell pCell);
LWindow LEDITAPI LCell_OpenExtractStatistics(LCell pCell);
LStatus LEDITAPI LExtract_RunCommandFile( LCell topCell, const char *szCommandFile, const char *szSpiceOutFile );
LStatus LEDITAPI LExtract_RunHiPer( LCell topCell );

// SPR functions
LCore   LEDITAPI LCore_GetCore( LFile file );
double  LEDITAPI LCore_GetLLHCap( LCore core );
LStatus LEDITAPI LCore_SetLLHCap( LCore core, double LLHCap );
double  LEDITAPI LCore_GetLLVCap( LCore core );
LStatus LEDITAPI LCore_SetLLVCap( LCore core, double LLVCap );

// Base point functions
LBoolean LEDITAPI LApp_GetBasePointMode();
LStatus  LEDITAPI LApp_SetBasePointMode( LBoolean bOn );
LStatus  LEDITAPI LCell_SetBasePoint(LCell pCell, LCoord lX, LCoord lY);
LPoint	LEDITAPI LCell_GetBasePoint(LCell pCell);

int LEDITAPI LSelection_Fracture(int nMaxVertices);
int LEDITAPI LCell_Fracture(LCell pCell, int nMaxVertices);

// Via functions
int LEDITAPI LFile_GetViaCount( LFile pTDBFile );
LStatus LEDITAPI LFile_DeleteAllVias( LFile pTDBFile );
LStatus LEDITAPI LFile_AddVia( LFile pTDBFile, const char * cszLowerLayer, 
														const char * cszUpperLayer, const char* csViaCell, LCoord nPitchX, LCoord nPitchY );
// First via has nIndex of 1.
LStatus LEDITAPI LFile_GetVia( LFile pTDBFile, int nIndex, char* pszLowerLayer, const int cnMaxLower, char* pszUpperLayer, const int cnMaxUpper, 
									char* pszViaCell, const int cnMaxVia, LCoord* pnPitchX, LCoord* pnPitchY );
LObject	LEDITAPI LVia_Fill( LCell cell, LCell viaDef, LRect area, LBoolean bFillArea );
LCell		LEDITAPI LVia_Find( LFile pFile, const char* cszViaDefName );
LCell		LEDITAPI LVia_FindByLayer( LFile pFile, const char* cszLayer1Name, const char* cszLayer2Name );

LStatus LEDITAPI LCell_GetViaInfo( LCell pCell, LCellViaInfo* pInfo, size_t nInfoSize );

//Library functions
int LEDITAPI LFile_GetLibraryCount( LFile pTDBFile ); 
LStatus LEDITAPI LFile_DeleteAllLibraries( LFile pTDBFile );
/* 
LStatus LEDITAPI LFile_AddLibrary( LFile pTDBFile, const char* cszLibrary );
LStatus LEDITAPI LFile_RemoveLibrary( LFile pTDBFile, const char* cszLibrary );
*/
// First Library has nIndex of 1.
LStatus LEDITAPI LFile_GetLibraryName( LFile pTDBFile, int nIndex, char* szName, const int cnMaxname );
LStatus LEDITAPI LFile_GetLibraryPath( LFile pTDBFile, const char* cszName, char* szPath, const int cnMaxpath );
LStatus LEDITAPI LFile_GetTopLevelLibraryName( LFile pTDBFile, char* szName, const int cnMaxname );

// Layer Palette functions
const char*		LEDITAPI LFile_GetCurrentLayerPalette( LFile pTDBFile );
LStatus			LEDITAPI LFile_SetCurrentLayerPalette( LFile pTDBFile, const char *cszPalette );
// LFile_GetNextLayerPalette: pass NULL to get first; returns NULL after last
const char*		LEDITAPI LFile_GetNextLayerPalette( LFile pTDBFile, const char *cszPalette ); 
LStatus			LEDITAPI LFile_DefineLayerPalette(LFile pTDBFile, const char* cszPalette );

#ifdef __cplusplus
	} // extern "C" 
#endif 

/****************************************************************************************************************************************************************/
/*** UTILITY MACROS                                                                                                                                             */
/****************************************************************************************************************************************************************/

#if !defined(NULL)
#  define NULL 0
#endif

#if defined(__cplusplus)
#  if !defined(_TANNER_H) && !defined(TLIB_TANNER_H)
#     include <assert.h>
      template <class T> inline bool Assigned(T* Ptr) {return(Ptr != NULL);};
      template <class T> inline bool NotAssigned(T* Ptr) {return(Ptr == NULL);};
#  endif
#else
#  define Assigned(Ptr) ((Ptr) != NULL)
#  define NotAssigned(Ptr) ((Ptr) == NULL)
#  define DisplayLine(lLineNumber)  LDialog_MsgBox(LFormat("At Line Number %ld.", lLineNumber));
#  define assert(x) (x)
   typedef int bool;
#  define false 0
#  define true  1
#endif

#ifndef NOMINMAX
#	ifndef max
#		define max(a,b)    (((a) > (b)) ? (a) : (b))
#	endif

#	ifndef min
#		define min(a,b)    (((a) < (b)) ? (a) : (b))
#	endif
#endif

/****************************************************************************************************************************************************************/
/*** OBSOLETE FUNCTIONS                                                                                                                                         */
/****************************************************************************************************************************************************************/

// If you write new code then you should not use these functions.
// It is ok to continue using these functions in the code that was written before these functions became obsolete.
#ifndef EXCLUDE_LEDIT_LEGACY_UPI

#ifdef __cplusplus
	extern "C" {
#endif 

   double LEDITAPI LFile_IntUtoLocU( LFile pFile, LCoord lcValue ); // Superseded by LFile_IntUtoDispU and by LFile_IntUtoMicrons
   LCoord LEDITAPI LFile_LocUtoIntU( LFile pFile, double dValue ); // Superseded by LFile_DispUtoIntU and by LFile_MicronsToIntU

   LStatus LEDITAPI LCell_GenerateLayers( LCell pCell, int iBinSize ); // Superseded by LCell_GenerateLayers_v10_00
   LStatus LEDITAPI LCell_GenerateLayersEx99( LCell pCell, int iBinSize, LLayer pLayer ); // Superseded by LCell_GenerateLayers_v10_00
   LStatus LEDITAPI LCell_GenerateLayersEx00( LCell pCell, int iBinSize, LLayer pLayer, LBoolean bDeletePreviousDerivedLayers, LBoolean bMergeObjectsAfterGeneration ); // Superseded by LCell_GenerateLayers_v10_00
   LStatus LEDITAPI LCell_GenerateLayersEx830( LCell pCell, int iBinSize, LLayer pLayer, LBoolean bDeletePreviousDerivedLayers, LBoolean bMergeObjectsAfterGeneration ); // Superseded by LCell_GenerateLayers_v10_00

	// XRef functions
	int LEDITAPI LFile_GetXRefCount( LFile pTDBFile ); 
	LStatus LEDITAPI LFile_DeleteAllXRefs( LFile pTDBFile );
	LStatus LEDITAPI LFile_AddXRef( LFile pTDBFile, const char * cszXRef );
	// First XRef has nIndex of 1.
	LStatus LEDITAPI LFile_GetXRef( LFile pTDBFile, int nIndex, char* pszName, const int cnMaxname );


#  define LDrcFlagWriteErrors             1
#  define LDrcFlagWriteErrorPorts         2
#  define LDrcFlagWriteErrorObjects       4
#  define LDrcFlagWriteTimingStatistics   8

   LStatus LEDITAPI LCell_RunDRCEx00( LCell pCell, LRect* pDRCArea, LCoord lcBinSize, const char* szErrorFile, LBoolean bWriteErrorPorts, LBoolean bWriteErrorObjects ); // Superseded by LCell_RunDRC
   LStatus LEDITAPI LCell_RunDRCEx01( LCell pCell, LRect* pDRCArea, LCoord lcBinSize, const char* szErrorFile, int flags ); // Superseded by LCell_RunDRC
   LStatus LEDITAPI LCell_RunDRCEx830( LCell pCell, LRect* pDRCArea, LCoord lcBinSize, const char* szErrorFile, int flags ); // Superseded by LCell_RunDRC
   void    LEDITAPI LDRC_Run( LCell inCell, LRect* onArea, const char* errfile, int writeErrorPorts, int writeErrorObjects ); // Superseded by LCell_RunDRC

   typedef long int LOrientation;
   typedef struct 
   {
       LPoint           translation;
       LOrientation     orientation;
       LMagnification   magnification;
   } LTransform;

   LTransform LEDITAPI LInstance_GetTransform( LInstance instance ); // Superseded by LInstance_GetTransform_Ex99
   LInstance  LEDITAPI LInstance_New( LCell cell, LCell instance_cell, LTransform transform, LPoint repeat_cnt, LPoint delta ); // Superseded by LInstance_New_Ex99
   LStatus    LEDITAPI LInstance_Set( LCell cell, LInstance instance, LTransform transform, LPoint repeat_cnt, LPoint delta ); // Superseded by LInstance_Set_Ex99
   void       LEDITAPI LObject_Transform( LObject object, LTransform transform ); // Superseded by LObject_Transform_Ex99

	typedef struct
	{
		int max_segment_per_curve;
		long max_length_of_segment;
		int display_as_approx;
	} LCurve;

	LCurve*          LEDITAPI LFile_GetCurveSetup( LFile file, LCurve *curve );
	LStatus          LEDITAPI LFile_SetCurveSetup( LFile file, LCurve *curve );

   LObject LEDITAPI LTorus_New(LCell cell, LLayer layer, LPoint ptCenter, double startangle, double stopangle, LCoord radius1, LCoord radius2); // Superseded by LTorus_CreateNew
   LStatus LEDITAPI LTorus_Set(LCell pCell, LObject pObject, LPoint ptCenter, double dStartAngle, double dStopAngle, LCoord nInnerRadius, LCoord nOuterRadius); // Superseded by LTorus_SetParams
   LPoint LEDITAPI LTorus_GetCenter(LObject object); // Superseded by LTorus_GetParams
   LCoord LEDITAPI LTorus_GetStartAngle(LObject object); // Superseded by LTorus_GetParams
   LCoord LEDITAPI LTorus_GetStopAngle(LObject object); // Superseded by LTorus_GetParams
   LCoord LEDITAPI LTorus_GetInnerRadius(LObject object); // Superseded by LTorus_GetParams
   LCoord LEDITAPI LTorus_GetOuterRadius(LObject object); // Superseded by LTorus_GetParams
   LObject LEDITAPI LPie_New(LCell cell, LLayer layer, LPoint ptCenter, double startangle, double stopangle, LCoord radius); // Superseded by LPie_CreateNew
   LStatus LEDITAPI LPie_Set(LCell cell, LObject object, LPoint ptCenter, double startangle, double stopangle, LCoord radius); // Superseded by LPie_SetParams
   LPoint LEDITAPI LPie_GetCenter(LObject object); // Superseded by LPie_GetParams
   LCoord LEDITAPI LPie_GetStartAngle(LObject object); // Superseded by LPie_GetParams
   LCoord LEDITAPI LPie_GetStopAngle(LObject object); // Superseded by LPie_GetParams
   LCoord LEDITAPI LPie_GetRadius(LObject object); // Superseded by LPie_GetParams

	LLayer       LEDITAPI LLayer_PrecedingLayer( LFile file, const char *name, LLayer precReserved );

   typedef struct _LDerivedLayerParam 
   {
      int enable_evaluation;  
      char *name;
      char *src_layer1;
      char *src_layer2;
      char *src_layer3;
      int layer1_not_op; 
      long layer1_grow_amount;
      int layer2_not_op; 
      long layer2_grow_amount;
      int layer3_not_op; 
      long layer3_grow_amount;
      int layer1_bool_layer2;
      int layer2_bool_layer3; 
   } LDerivedLayerParam;

   typedef struct 
   {
      char CIFName[7];
      short GDSNumber;
      double cap;
      double rho;
      int lock;
      LLayerViewStatus viewStatus;
      LWireParam wireParam;
   } LLayerParam;

   typedef LDerivedLayerParamEx830 LDerivedLayerParamEx00;

   LDerivedLayerParamEx00* LEDITAPI LLayer_GetDerivedParametersEx00(LLayer  layer, LDerivedLayerParamEx00 *param); // Superseded by LLayer_GetDerivedParametersEx830
   LStatus LEDITAPI LLayer_DestroyDerivedParameter(LDerivedLayerParam* pDerivedLayerParam); // Superseded by LLayer_DestroyDerivedParameterEx840
   LStatus LEDITAPI LLayer_SetDerivedParametersEx00(LFile file, LLayer layer, LDerivedLayerParamEx00 *param); // Superseded by LLayer_SetDerivedParametersEx830
   LDerivedLayerParam* LEDITAPI LLayer_GetDerivedParameters( LLayer layer, LDerivedLayerParam *param); // Superseded by LLayer_GetDerivedParametersEx830
   LStatus LEDITAPI  LLayer_SetDerivedParameters( LFile file, LLayer layer, LDerivedLayerParam *param); // Superseded by LLayer_SetDerivedParametersEx830
   typedef LLayerParamEx830 LLayerParam_Ex00;
   LStatus LEDITAPI LLayer_GetParametersEx830(LLayer pLayer, LLayerParamEx830* pLayerParam);	// Superseded by LLayer_GetParametersEx1512
	LStatus LEDITAPI LLayer_SetParametersEx830(LLayer pLayer, const LLayerParamEx830* pLayerParam);  // Superseded by LLayer_SetParametersEx1512
	LStatus LEDITAPI LLayer_GetParameters_Ex00(LLayer pLayer, LLayerParam_Ex00* pLayerParam); // Superseded by LLayer_GetParametersEx830
   LStatus LEDITAPI LLayer_SetParameters_Ex00(LLayer pLayer, LLayerParam_Ex00* pLayerParam); // Superseded by LLayer_SetParametersEx830
   LLayerParam* LEDITAPI LLayer_GetParameters( LLayer layer, LLayerParam *param ); // Superseded by LLayer_GetParametersEx830
   LStatus      LEDITAPI LLayer_SetParameters( LLayer layer, LLayerParam *param ); // Superseded by LLayer_SetParametersEx830

   typedef LColor LPalette[16];
   LColor* LEDITAPI LFile_GetPalette( LFile file, LColor *lpalette ); // Superseded by LFile_GetColorPalette
   LStatus LEDITAPI LFile_SetPalette( LFile file, LColor *lpalette, short index ); // Superseded by LFile_SetColorPalette

   LTransform LEDITAPI LTransform_Set( LCoord xtrans, LCoord ytrans, LOrientation orient, LMagnification mag ); // Superseded by LTransform_Set_Ex99
   LTransform LEDITAPI LTransform_Zero( void ); // Superseded by LTransform_Zero_Ex99
   LTransform LEDITAPI LTransform_Add( LTransform transform_to_be_added, LTransform current_transform ); // Superseded by LTransform_Add_Ex99
   LTransform LEDITAPI LTransform_Subtract( LTransform transform_to_be_subtracted, LTransform current_transform ); // Superseded by LTransform_Subtract_Ex99
   LPoint     LEDITAPI LPoint_Transform( LPoint point, LTransform transform ); // Superseded by LPoint_Transform_Ex99
   LRect      LEDITAPI LRect_Transform( LRect rect, LTransform transform ); // Superseded by LRect_Transform_Ex99

   LCoord  LEDITAPI LDRC_GetSelectBinSize(void); // Obsolete function
   LStatus LEDITAPI LDRC_SetSelectBinSize(LCoord nBinSize);  // Obsolete function

   typedef struct _LTechnology 
   {
      const char* name;
      tech_unit_type unit_type;
      const char* unit_name;
      long num;
      long denom;
      long lambda_num;
      long lambda_denom;
   } LTechnology;
   LTechnology LEDITAPI LFile_GetTechnology( LFile file ); // Superseded by LFile_GetTechnologyEx840
   LStatus     LEDITAPI LFile_SetTechnology( LFile file, LTechnology *ltechnology );  // Superseded by LFile_SetTechnologyEx840

   typedef struct _LExtractOptions
   {
      // Data
      // General Options
      char      szExtDefnFile[256];      // Extract definition file - Input.
      char      szExtOutFile[256];       // Extract SPICE output file - Output.
      double    dExtractBinSize;         // Bin size.  (Locator Units)
      int       iNodeHighlight;          // Enable Node highlighting capability. (0 - False, Otherwise True).

      // Output Options
      int       iWriteNodeNames;         // Write Node names in comments. (0 - False, Otherwise True).
      int       iWriteDeviceCoord;       // Write device coordinates in comments. (0 - False, Otherwise True).
      int       iWriteShortedDevices;    // Write shorted devices in comments. (0 - False, Otherwise True).
      int       iWriteParasiticCap;      // Write parasitic capacitances. (0 - False, Otherwise True).
      double    dParasiticCutoff;        // Cutoff value for parasitic capacitors. (0 - False, Otherwise True).
      int       iWriteNodesAs;           // Write nodes as (integers or names). (0 - Integers, Otherwise Names).
      int       iWriteSciNotation;       // Write values in scientific notation. (0 - False, Otherwise True).
      int       iWriteVerboseSPICE;      // Write R, L, C with verbose style (R=, L=, C=). (0 - False, Otherwise True).
      int       iWriteSubCktDefs;        // Write empty subcircuit definitions. (0 - False, Otherwise True).
      char     *szExtIncludeStmt;        // SPICE include statement.
      int       iLabelAllDevices;        // Create ports for all devices. (0 - False, Otherwise True).
      LLayer    oDeviceLabelLayer;       // Place device labels on this layer.

      // Subcircuit Options
      int       iSubCktRecognition;      // Recognize subcircuit instances. (0 - False, Otherwise True).
      LLayer    oSubCktRecogLayer;       // Subcircuit recognition layer.
      int       iUseSubCktNetlistFmt;    // Write netlist as a subcircuit. (0 - False, Otherwise True).
      int       iFlagImproperOverlaps;   // Flag improper overlaps. (0 - False, Otherwise True).
      LLayer    oIgnoreConnPortLayer;    // Ignore connection ports on this layer.
      char      szIgnoreConnPort[256];   // Ignore connection ports with this name.
      char      szIgnoreCrossPort[256];  // Ignore cross ports with this name.

      // Miscellaneous
      long      lMaxIncludeStmtLen;      // Length of the Include Statement string.
   } LExtractOptions;

   LStatus LEDITAPI LExtract_Run_Ex98( LCell topCell, LExtractOptions* ExtOptions ); // Superseded by LExtract_RunEx840
   LStatus LEDITAPI LExtract_GetOptions_Ex98( LCell oCell, LExtractOptions *ExtOptions ); // Superseded by LExtract_GetOptionsEx840
   LStatus LEDITAPI LExtract_SetOptions_Ex98( LCell oCell, LExtractOptions *ExtOptions ); // Superseded by LExtract_SetOptionsEx840

   LStatus LEDITAPI LUPI_InitSystem( int* argc, char** argv ); // Obsolete

   typedef enum 
   {
      LSet=16,
      LClear=8
   } LPassMode;

   typedef enum 
   {
      LObjectPass, LPortPass,
      LTextPass
   } LPassType; 

   typedef struct _LPassParam 
   {
      unsigned char ColorIndex; 
      LPassMode WriteMode;
      LStipple Stipple;
   } LPassParam;

   LPass       LEDITAPI LPass_GetList( LLayer layer, LPassType passType ); // Superseded by LLayer_GetRenderingAttribute
   LPass       LEDITAPI LPass_GetNext( LPass pass ); // Superseded by LLayer_GetRenderingAttribute
   LPass       LEDITAPI LPass_New( LPass precedingPass, LPass pass ); // Superseded by LLayer_SetRenderingAttribute
   LPassParam* LEDITAPI LPass_GetParameters( LPass pass, LPassParam *param ); // Superseded by LLayer_GetRenderingAttribute
   LStatus     LEDITAPI LPass_SetParameters( LPass pass, LPassParam *param ); // Superseded by LLayer_SetRenderingAttribute

   typedef enum
   {
      LDo_Not_Flag = 0,
      LFlag = 1,
      LFix = 2
   } LAmbiguousFillType;

   typedef struct
   {
      int FlagSelfIntersection;
      LAmbiguousFillType PolygonsWithAmbiguousFills;
      int FlagIgnoredObjects;
      int FlagOffGridObjects;
      double GridSize;
      int UseLocatorUnits;
   } LDesignRuleFlags;

   LCoord  LEDITAPI LFile_GetBinSize( LFile file ); // Obsolete
   void    LEDITAPI LFile_SetBinSize( LFile file, LCoord size ); // Obsolete
   LStatus LEDITAPI LFile_GetDesignRuleFlags( LFile file, LDesignRuleFlags *pDrcFlags ); // Superseded by LFile_GetDrcFlags
   LStatus LEDITAPI LFile_SetDesignRuleFlags( LFile file, LDesignRuleFlags *pDrcFlags ); // Superseded by LFile_SetDrcFlags
#  define ignore_45_acute_angles ignore_acute_angles

#  define FixNonConstForUPI(x) ((char*)(x))

   LObject LEDITAPI LPolygon_WireToPolygon( LCell cell, LLayer layer, LObject object ); // Superseded by LObject_ConvertToPolygon
   LObject LEDITAPI LPolygon_CircleToPolygon( LCell cell, LLayer layer, LObject object, int NumSides ); // Superseded by LObject_ConvertToPolygon

   typedef struct 
   {
      long displayed_grid_size; 
      long min_grid_pixels;
      long mouse_snap_grid_size;
      LCursorType cursor_type;
      long locator_scaling;
   } LGrid;

   typedef struct 
   {
      long displayed_grid_size; /* In internal units */
      long min_grid_pixels;
      long displayed_majorgrid_size; /* In internal units */
      long min_majorgrid_pixels;
      long mouse_snap_grid_size; /* In internal units */
      LCursorType cursor_type;
      long locator_scaling;
   } LGridEx840;

	typedef struct
	{
		long displayed_grid_size; /* In internal units */
		long min_grid_pixels;
		long displayed_majorgrid_size; /* In internal units */
		long min_majorgrid_pixels;
		long mouse_snap_grid_size; /* In internal units */
		LCursorType cursor_type;
		long locator_scaling;
		long manufacturing_grid_size; /* In internal units */
		LBoolean display_curves_using_manufacturing_grid;
	} LGrid_v10_00;

   LGrid*  LEDITAPI LFile_GetGrid( LFile file, LGrid *grid );  // Superseded by LFile_GetGrid_v10_00
   LStatus LEDITAPI LFile_SetGrid( LFile file, LGrid *grid );  // Superseded by LFile_SetGrid_v10_00
   LStatus LEDITAPI LFile_GetGridEx840( LFile file, LGridEx840 *grid ); // Superseded by LFile_GetGrid_v10_00
   LStatus LEDITAPI LFile_SetGridEx840( LFile file, LGridEx840 *grid ); // Superseded by LFile_SetGrid_v10_00	
	LStatus LEDITAPI LFile_GetGrid_v10_00( LFile file, LGrid_v10_00 *grid ); // Superseded by LFile_GetGrid_v16_30	
	LStatus LEDITAPI LFile_SetGrid_v10_00( LFile file, LGrid_v10_00 *grid ); // Superseded by LFile_SetGrid_v16_30	
	
	typedef struct _LProp*      LProp;
	typedef struct _LPropItem*  LPropItem;
	typedef enum 
	{
		LPROP,
		LPROP_ITEM,
		LPROP_NONE
	} LPropItemType; // Old style property, only for use with LProp_AddExtractProp(...) function
	typedef enum 
	{
		LPT_INT,
		LPT_FLOAT,
		LPT_BOOL,
		LPT_STRING,
		LPT_EQN, 
		LPT_LINK,
		LPT_BLOB,
		LPT_GROUP,
		LPT_UNKNOWN
	} LPropType; // Old style property, only for use with LProp_AddExtractProp(...) function
	typedef short LPropCount;
	typedef long LPropAttrib;
	#define Lboolean LBoolean
	typedef union LpropertyVal
	{
		int LintVal;
		float LfloatVal;
		Lboolean LboolVal;
		char* LstrVal;
	} LPropVal; // Old style property, only for use with LProp_AddExtractProp(...) function

	extern LStatus LEDITAPI LProp_AddExtractProp( LCell Cell_Now, const char *propName, LPropType type, LPropVal value, LPropCount count, LPropAttrib attrib );

	typedef struct _LNet *LNet;
	typedef struct _LNetVertex *LNetVertex;
	typedef enum
	{
		LUndefinedCellType, 
		LDieCellType,
		LDiscreteCellType, 
		LIORingCellType,
		LViaCellType,
		LArtificialPinType,
		LBlockCellType,
		LSPRCoreCellType
	} LCellType;

	// BPR functions
	char*      LEDITAPI LFile_BPR_GetCellName( LFile file,char* name, const int maxname );
	LCellType  LEDITAPI LCell_GetType( LCell cell );
	LNet       LEDITAPI LCell_GetNetList( LCell pCell );
	LNet       LEDITAPI LNet_GetNext( LNet pNet );
	char*      LEDITAPI LNet_GetName( LNet pNet, char* name, const int maxname );
	LNetVertex LEDITAPI LNet_GetNetVertexList( LNet pNet );
	LNetVertex LEDITAPI LNetVertex_GetNext( LNetVertex pNetVertex );
	LPort      LEDITAPI LNetVertex_GetPort( LNetVertex pNetVertex );
	LPort      LEDITAPI LInstance_GetNetVertexPort( LInstance pInstance );
	char*      LEDITAPI LPort_GetNetName( LPort pPort, char* name, const int maxname );

	typedef struct
	{
		int upcase_cell_name;
		short circle_to_polygon_sides; // obsolete
		int use_default_units;  
	} LGDSParam;

	// GDS file import/export setup functions
	LGDSParam* LEDITAPI LFile_GetGDSParameters( LFile file, LGDSParam* gdsparam );
	LStatus    LEDITAPI LFile_SetGDSParameters( LFile file, LGDSParam* gdsparam );

	unsigned int LEDITAPI LMacro_BindToHotKey( int keycode, const char* macro_desc, const char* function_name );
	unsigned int LEDITAPI LMacro_BindToMenu( const char* menu, const char* macro_desc, const char* function_name );
	unsigned int LEDITAPI LMacro_BindToMenuAndHotKey( const char* menu, int keycode, const char* macro_desc, const char* function_name );

	// Keyboard constants
#	if defined( UNIX_DLL )
#		define KEYGROUP_NUM    (48)
#		define KEYGROUP_ALPHA  (97)
#		define KEYGROUP_FKEYS  (187)
#	else
#		define KEYGROUP_NUM    ((unsigned short)0x0100)
#		define KEYGROUP_ALPHA  ((unsigned short)0x0200)
#		define KEYGROUP_FKEYS  ((unsigned short)0x0300)
#		define KEYGROUP_CURSOR ((unsigned short)0x0400)
#		define KEYGROUP_MISC   ((unsigned short)0x0500)
#	endif

#	define KEY_0           ((unsigned short)(KEYGROUP_NUM+0))
#	define KEY_1           ((unsigned short)(KEYGROUP_NUM+1))
#	define KEY_2           ((unsigned short)(KEYGROUP_NUM+2))
#	define KEY_3           ((unsigned short)(KEYGROUP_NUM+3))
#	define KEY_4           ((unsigned short)(KEYGROUP_NUM+4))
#	define KEY_5           ((unsigned short)(KEYGROUP_NUM+5))
#	define KEY_6           ((unsigned short)(KEYGROUP_NUM+6))
#	define KEY_7           ((unsigned short)(KEYGROUP_NUM+7))
#	define KEY_8           ((unsigned short)(KEYGROUP_NUM+8))
#	define KEY_9           ((unsigned short)(KEYGROUP_NUM+9))

#	define KEY_A           ((unsigned short)(KEYGROUP_ALPHA+0))
#	define KEY_B           ((unsigned short)(KEYGROUP_ALPHA+1))
#	define KEY_C           ((unsigned short)(KEYGROUP_ALPHA+2))
#	define KEY_D           ((unsigned short)(KEYGROUP_ALPHA+3))
#	define KEY_E           ((unsigned short)(KEYGROUP_ALPHA+4))
#	define KEY_F           ((unsigned short)(KEYGROUP_ALPHA+5))
#	define KEY_G           ((unsigned short)(KEYGROUP_ALPHA+6))
#	define KEY_H           ((unsigned short)(KEYGROUP_ALPHA+7))
#	define KEY_I           ((unsigned short)(KEYGROUP_ALPHA+8))
#	define KEY_J           ((unsigned short)(KEYGROUP_ALPHA+9))
#	define KEY_K           ((unsigned short)(KEYGROUP_ALPHA+10))
#	define KEY_L           ((unsigned short)(KEYGROUP_ALPHA+11))
#	define KEY_M           ((unsigned short)(KEYGROUP_ALPHA+12))
#	define KEY_N           ((unsigned short)(KEYGROUP_ALPHA+13))
#	define KEY_O           ((unsigned short)(KEYGROUP_ALPHA+14))
#	define KEY_P           ((unsigned short)(KEYGROUP_ALPHA+15))
#	define KEY_Q           ((unsigned short)(KEYGROUP_ALPHA+16))
#	define KEY_R           ((unsigned short)(KEYGROUP_ALPHA+17))
#	define KEY_S           ((unsigned short)(KEYGROUP_ALPHA+18))
#	define KEY_T           ((unsigned short)(KEYGROUP_ALPHA+19))
#	define KEY_U           ((unsigned short)(KEYGROUP_ALPHA+20))
#	define KEY_V           ((unsigned short)(KEYGROUP_ALPHA+21))
#	define KEY_W           ((unsigned short)(KEYGROUP_ALPHA+22))
#	define KEY_X           ((unsigned short)(KEYGROUP_ALPHA+23))
#	define KEY_Y           ((unsigned short)(KEYGROUP_ALPHA+24))
#	define KEY_Z           ((unsigned short)(KEYGROUP_ALPHA+25))

#	define KEY_F1          ((unsigned short)(KEYGROUP_FKEYS+0))
#	define KEY_F2          ((unsigned short)(KEYGROUP_FKEYS+1))
#	define KEY_F3          ((unsigned short)(KEYGROUP_FKEYS+2))
#	define KEY_F4          ((unsigned short)(KEYGROUP_FKEYS+3))
#	define KEY_F5          ((unsigned short)(KEYGROUP_FKEYS+4))
#	define KEY_F6          ((unsigned short)(KEYGROUP_FKEYS+5))
#	define KEY_F7          ((unsigned short)(KEYGROUP_FKEYS+6))
#	define KEY_F8          ((unsigned short)(KEYGROUP_FKEYS+7))
#	define KEY_F9          ((unsigned short)(KEYGROUP_FKEYS+8))
#	define KEY_F10         ((unsigned short)(KEYGROUP_FKEYS+9))
#	define KEY_F11         ((unsigned short)(KEYGROUP_FKEYS+10))
#	define KEY_F12         ((unsigned short)(KEYGROUP_FKEYS+11))
#	define KEY_F13         ((unsigned short)(KEYGROUP_FKEYS+12))
#	define KEY_F14         ((unsigned short)(KEYGROUP_FKEYS+13))
#	define KEY_F15         ((unsigned short)(KEYGROUP_FKEYS+14))
#	define KEY_F16         ((unsigned short)(KEYGROUP_FKEYS+15))
#	define KEY_F17         ((unsigned short)(KEYGROUP_FKEYS+16))
#	define KEY_F18         ((unsigned short)(KEYGROUP_FKEYS+17))
#	define KEY_F19         ((unsigned short)(KEYGROUP_FKEYS+18))
#	define KEY_F20         ((unsigned short)(KEYGROUP_FKEYS+19))
#	define KEY_F21         ((unsigned short)(KEYGROUP_FKEYS+20))
#	define KEY_F22         ((unsigned short)(KEYGROUP_FKEYS+21))
#	define KEY_F23         ((unsigned short)(KEYGROUP_FKEYS+22))
#	define KEY_F24         ((unsigned short)(KEYGROUP_FKEYS+23))
#	define KEY_F25         ((unsigned short)(KEYGROUP_FKEYS+24))
#	define KEY_F26         ((unsigned short)(KEYGROUP_FKEYS+25))
	
#	if !defined( UNIX_DLL )
#		define KEY_DOWN        ((unsigned short)(KEYGROUP_CURSOR+0))
#		define KEY_UP          ((unsigned short)(KEYGROUP_CURSOR+1))
#		define KEY_LEFT        ((unsigned short)(KEYGROUP_CURSOR+2))
#		define KEY_RIGHT       ((unsigned short)(KEYGROUP_CURSOR+3))
#		define KEY_HOME        ((unsigned short)(KEYGROUP_CURSOR+4))
#		define KEY_END         ((unsigned short)(KEYGROUP_CURSOR+5))
#		define KEY_PAGEUP      ((unsigned short)(KEYGROUP_CURSOR+6))
#		define KEY_PAGEDOWN    ((unsigned short)(KEYGROUP_CURSOR+7))

#		define KEY_RETURN      ((unsigned short)(KEYGROUP_MISC+0))
#		define KEY_ESCAPE      ((unsigned short)(KEYGROUP_MISC+1))
#		define KEY_TAB         ((unsigned short)(KEYGROUP_MISC+2))
#		define KEY_BACKSPACE   ((unsigned short)(KEYGROUP_MISC+3))
#		define KEY_SPACE       ((unsigned short)(KEYGROUP_MISC+4))
#		define KEY_INSERT      ((unsigned short)(KEYGROUP_MISC+5))
#		define KEY_DELETE      ((unsigned short)(KEYGROUP_MISC+6))

#		define KEY_ADD         ((unsigned short)(KEYGROUP_MISC+7))
#		define KEY_SUBTRACT    ((unsigned short)(KEYGROUP_MISC+8))
#		define KEY_MULTIPLY    ((unsigned short)(KEYGROUP_MISC+9))
#		define KEY_DIVIDE      ((unsigned short)(KEYGROUP_MISC+10))
#		define KEY_POINT       ((unsigned short)(KEYGROUP_MISC+11))
#		define KEY_COMMA       ((unsigned short)(KEYGROUP_MISC+12))
#		define KEY_LESS        ((unsigned short)(KEYGROUP_MISC+13))
#		define KEY_GREATER     ((unsigned short)(KEYGROUP_MISC+14))
#		define KEY_EQUAL       ((unsigned short)(KEYGROUP_MISC+15))
#	else /* Unix */
#		define KEY_DOWN        (208)
#		define KEY_UP          (200)
#		define KEY_LEFT        (203)
#		define KEY_RIGHT       (205)
#		define KEY_HOME        (199)
#		define KEY_END         (207)
#		define KEY_PAGEUP      (201)
#		define KEY_PAGEDOWN    (209)

#		define KEY_RETURN      (13)
#		define KEY_ESCAPE      (27)
#		define KEY_TAB         (9)
#		define KEY_BACKSPACE   (8)
#		define KEY_SPACE       (32)
#		define KEY_DELETE      (211)

#		define KEY_ADD         (43)
#		define KEY_SUBTRACT    (45)
#		define KEY_MULTIPLY    (42)
#		define KEY_DIVIDE      (47)
#		define KEY_POINT       (46)
#		define KEY_COMMA       (44)
#		define KEY_LESS        (60)
#		define KEY_GREATER     (62)
#		define KEY_EQUAL       (61)
#	endif /* of if !UNIX_DLL */

	LPoint  LEDITAPI LCursor_GetPosition( void );
	LPoint  LEDITAPI LCursor_GetSnappedPosition( void );

	void              LEDITAPI LDrc_OverrideGrowComputationValue( LCoord l );

	//use LCell_GetParameterValue() instead
	size_t   LEDITAPI LCell_GetParameter( LCell cell, const char* lpszParamName );

	double	LEDITAPI LWire_GetResistance(LObject pObject);

#ifdef __cplusplus
	} // extern "C" 
#endif 

#endif // End of legacy functions

// Clean up
#ifdef _UPI_PACKING
#pragma pack(pop)
#endif

#endif // #ifndef __LDATA_H
