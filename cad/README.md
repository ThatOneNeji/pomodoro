# Case

The 3D-printed case: where it comes from, how to change it, and how to rebuild the files you print.

| Folder | What's in it |
| --- | --- |
| `cad/original/` | The STL files of the original design, exported from Rukenshia's [Onshape document](https://cad.onshape.com/documents/06055e629740267835bb7660/w/df56eb93ab74e2f4d61e5097/e/21a7853695e4900200750891). **Never edited.** |
| `cad/build_case.py` | Our changes to the case, as code, and the script that applies them. |
| `stl/` | The files to print. **Generated:** don't edit them by hand, run `npm run case`. |

The parts are the Housing (back and bottom), the Cover (front, around the display), the Knob and the LEDCase.

## Rebuilding `stl/`

You need [FreeCAD](https://www.freecad.org/) 1.x. Its AppImage is fine, and nothing needs installing.

1. Tell the scripts where FreeCAD is, e.g. in `~/.bashrc`:

   ```bash
   export FREECAD="$HOME/Desktop/AppImage/FreeCAD_1.1.4-Linux-x86_64-py311.AppImage"
   ```

   Without `FREECAD`, `freecadcmd` must be on your `PATH` (e.g. `sudo apt install freecad`).
2. Run `npm run case` to rebuild `stl/`.
3. Optionally, run `npm run case:check` to prove the conversion is lossless (below). It takes about 1.5 minutes, mostly for the Knob.

Commit the rebuilt `stl/` files together with the change to `build_case.py`, so people without FreeCAD can print straight from the repo.

## How a change works

The originals are STL files, which are meshes of triangles, not CAD models. The Onshape design history can't be exported, and Onshape exports need an account. So `build_case.py`:

1. Copies a part unchanged when it has no edits, byte for byte.
2. Otherwise turns the part's triangles into a solid. Flat triangles that touch merge into one face.
3. Applies that part's edits, listed in `EDITS`: ordinary CAD operations such as cutting a box out (`solid.cut(...)`) or adding one (`solid.fuse(...)`).
4. Meshes the result again and writes it to `stl/`, after checking it's a valid, watertight solid.

`npm run case:check` converts every part to a solid and back without edits, and compares it with the original. On 2026-10-10 every part matched: volume within 0.0003%, outer bounds exact.

**Adding an edit:** write a function that takes the solid and returns the changed one, and add it to the part's list in `EDITS`. Keep each measurement as a named constant, with where it came from (datasheet, product page, calipers). Coordinates are millimetres, in each original file's own coordinate system; `npm run case:check` prints every part's bounds.

**Limits:** curved surfaces stay as many small flat facets. That doesn't matter for printing, or for adding and cutting shapes. Reshaping an existing feature, though, such as widening the current USB opening, means filling the old one and cutting the new one.

## Viewing

Open any file from `stl/` or `cad/original/` in FreeCAD (File → Open, or drag it in) to look at it and measure it. The `.FCStd` files FreeCAD saves are just viewing copies: the source is `cad/original/` plus `build_case.py`.
