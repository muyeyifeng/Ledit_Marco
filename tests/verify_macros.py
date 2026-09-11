"""Run in an MSVC developer terminal: python tests/verify_macros.py.

L-Edit's module wrapper and top-level registration call are removed only in
temporary files. The actual repository UPI header is used for type checking.
This is not a replacement for loading the macros in L-Edit.
"""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def function(source, name):
    match = re.search(r"\b(?:void|int|double|bool|Point|LPoint|LObject)\s+\*?\s*" + name + r"\([^;{}]*\)\s*(?://[^\n]*)?\s*\{", source)
    if not match:
        raise ValueError(name)
    pos = source.index("{", match.start()) + 1
    depth = 1
    end = pos
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[match.start():end]


def run():
    with tempfile.TemporaryDirectory(prefix="ledit-check-") as directory:
        temp = Path(directory)
        for source in sorted(ROOT.glob("*.c")):
            code = source.read_text(encoding="utf-8-sig")
            code = re.sub(r"\bmodule\s+\w+\s*\{", "", code, count=1)
            code = re.sub(r"\}\s*/\* end of module[^\n]*\*/\s*\w+\(\);\s*$", "", code)
            target = temp / source.name
            target.write_text(code, encoding="utf-8")
            subprocess.run(["cl", "/nologo", "/TC", "/std:c11", "/utf-8", "/W3",
                            "/we4024", "/we4047", "/we4715", "/we4716", "/we4090", "/we4033", "/we4045",
                            "/wd4244", "/wd4267", "/wd4996", "/c", str(target),
                            "/I" + str(ROOT), "/Fo" + str(temp / (source.stem + ".obj"))], check=True)
        print("PASS: 7 macro bodies compiled against repository ldata.h")
        test = ROOT / "tests" / "geometry_tests.c"
        if test.exists():
            pieces = [test.read_text(encoding="utf-8")]
            groups = {
                "Ledit_Array_Module.c": ["MirrorCoordinates", "CalculateOffset", "MirrorObject", "RotateObject", "LSelection_GetNumber", "_ArrayInObjectByDistanceHexagon"],
                "Ledit_Modifiy_Module.c": ["calculateIntersection", "calculateCircleSegmentIntersection", "GetPointsDistance", "DrawCircleByRouteDistance", "Compare_Objects", "Modifiy_Delete_Duplicates"],
                "Ledit_Special_Polygon.c": ["Calculate_Outside_Circle", "normalize_angle", "get_short_arc_interpolated_angle", "calculate_arc_points"],
                "Ledit_Conversion.c": ["computeCentroid", "computeInertiaMatrix", "computeEccentricity", "fitCircle"],
                "Ledit_ShapeCenter_Module.c": ["swap", "compare", "CalculateCentroid"],
                "Ledit_Regular_Polygon.c": ["CalculateEllipsePoints"],
            }
            for name, names in groups.items():
                code = (ROOT / name).read_text(encoding="utf-8")
                pieces.extend(function(code, name) for name in names)
            target = temp / "geometry_tests.c"
            target.write_text("\n".join(pieces), encoding="utf-8")
            exe = temp / "geometry_tests.exe"
            subprocess.run(["cl", "/nologo", "/TC", "/std:c11", "/utf-8", "/wd4244", "/wd4267", str(target), "/I" + str(ROOT), "/Fo" + str(temp / "test.obj"), "/Fe" + str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == "__main__":
    run()
