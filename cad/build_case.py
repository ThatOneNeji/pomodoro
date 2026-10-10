"""Regenerate the printable case parts in stl/ from the originals in cad/original/, plus our edits.

Run it with FreeCAD's command-line interpreter (see cad/README.md):

    FREECAD=/path/to/FreeCAD.AppImage npm run case        # rebuild stl/
    FREECAD=/path/to/FreeCAD.AppImage npm run case:check  # prove the conversion is lossless

cad/original/ holds the STL files from the original design (Rukenshia's Onshape document) and is
never edited: every change lives in this file, as an edit function listed in EDITS. A part with no
edits is copied through byte for byte. A part with edits is converted from a mesh to a solid, edited
with ordinary CAD operations (cut, fuse), and meshed again.

Coordinates are millimetres, in each original file's own coordinate system (print `describe()` of a
part to see its bounding box).
"""

import os
import shutil
import sys
import time

import FreeCAD  # noqa: F401  (provided by freecadcmd)
import Mesh
import MeshPart
import Part

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ORIGINAL_DIR = os.path.join(ROOT, "cad", "original")
OUTPUT_DIR = os.path.join(ROOT, "stl")

PARTS = ["Cover", "Housing", "Knob", "LEDCase"]

# --- Edits -----------------------------------------------------------------------------------------
# Each edit takes the part as a solid and returns the changed solid. Keep measurements as named
# constants next to the edit, with where they came from (datasheet, product page, calipers).

#: Edits per part, applied in order. A part missing here, or with an empty list, is copied unchanged.
EDITS = {
    "Cover": [],
    "Housing": [],
    "Knob": [],
    "LEDCase": [],
}

# --- Pipeline --------------------------------------------------------------------------------------

#: When meshing an edited solid again: the largest allowed deviation from the true surface, in mm.
#: The originals are already triangles, so this only matters for curved shapes the edits add.
LINEAR_DEFLECTION = 0.05
#: The same, as an angle in radians, for small curved features.
ANGULAR_DEFLECTION = 0.2

#: How far a converted part may differ from its original before the check fails.
VOLUME_TOLERANCE = 0.001  # fraction of the volume
BOUNDS_TOLERANCE = 0.01  # mm


def stl_name(part):
    return f"Pomodoro - {part}.stl"


def load_solid(part):
    """Read an original STL and turn its triangles into one closed solid."""
    mesh = Mesh.Mesh(os.path.join(ORIGINAL_DIR, stl_name(part)))
    shape = Part.Shape()
    shape.makeShapeFromMesh(mesh.Topology, 0.05, False)
    # removeSplitter() merges neighbouring triangles that lie in the same plane into one face.
    solid = Part.Solid(Part.Shell(shape.Faces)).removeSplitter()
    if not (solid.isValid() and solid.isClosed()):
        raise RuntimeError(f"{part}: the original mesh did not make a valid, closed solid")
    return solid


def to_mesh(solid):
    return MeshPart.meshFromShape(
        Shape=solid, LinearDeflection=LINEAR_DEFLECTION, AngularDeflection=ANGULAR_DEFLECTION
    )


def describe(shape):
    box = shape.BoundBox
    return (
        f"x {box.XMin:.2f}..{box.XMax:.2f}, y {box.YMin:.2f}..{box.YMax:.2f}, "
        f"z {box.ZMin:.2f}..{box.ZMax:.2f} mm, {shape.Volume / 1000:.2f} cm3"
    )


def build():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    for part in PARTS:
        source = os.path.join(ORIGINAL_DIR, stl_name(part))
        target = os.path.join(OUTPUT_DIR, stl_name(part))
        edits = EDITS.get(part, [])
        if not edits:
            shutil.copyfile(source, target)
            print(f"{part}: no edits, copied")
            continue

        started = time.time()
        solid = load_solid(part)
        for edit in edits:
            solid = edit(solid).removeSplitter()
            if not solid.isValid():
                raise RuntimeError(f"{part}: {edit.__name__} produced an invalid solid")
            print(f"{part}: {edit.__name__}")
        mesh = to_mesh(solid)
        if not mesh.isSolid() or mesh.hasNonManifolds():
            raise RuntimeError(f"{part}: the edited mesh isn't watertight")
        mesh.write(target)
        print(f"{part}: written ({describe(solid)}, {time.time() - started:.1f}s)")


def check():
    """Convert every part to a solid and back, unedited, and compare it with the original."""
    failed = False
    for part in PARTS:
        started = time.time()
        original = Mesh.Mesh(os.path.join(ORIGINAL_DIR, stl_name(part)))
        solid = load_solid(part)
        meshed = to_mesh(solid)

        volume_error = abs(meshed.Volume - original.Volume) / original.Volume
        a, b = original.BoundBox, meshed.BoundBox
        bounds_error = max(
            abs(a.XMin - b.XMin), abs(a.XMax - b.XMax), abs(a.YMin - b.YMin),
            abs(a.YMax - b.YMax), abs(a.ZMin - b.ZMin), abs(a.ZMax - b.ZMax),
        )
        ok = volume_error <= VOLUME_TOLERANCE and bounds_error <= BOUNDS_TOLERANCE and meshed.isSolid()
        failed |= not ok
        print(
            f"{'OK  ' if ok else 'FAIL'} {part}: {describe(solid)}; volume off by {volume_error:.4%}, "
            f"bounds by {bounds_error:.4f} mm, {len(solid.Faces)} faces ({time.time() - started:.1f}s)"
        )
    if failed:
        sys.exit(1)


# freecadcmd runs this file as a module (not as __main__), so there is no __name__ guard.
if os.environ.get("CASE_CHECK"):
    check()
else:
    build()
