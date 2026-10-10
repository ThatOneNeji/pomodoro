# Bootstrap Icons

The complete SVG set from [Bootstrap Icons](https://github.com/twbs/icons), MIT-licensed (see
`THIRD_PARTY_LICENSES.md`).

**Version: v1.13.2** (released 2026-10-08). Its icons are identical to v1.13.1 (2025-05-09); that
release only changed the project's docs site and dependencies.

Only the icons listed in `assets/manifest.json` are compiled into the firmware, so the rest of this
folder costs no flash. Browse them at <https://icons.getbootstrap.com/>.

## Updating

1. Download the release's `bootstrap-icons-<version>.zip` from
   <https://github.com/twbs/icons/releases>.
2. Replace the `.svg` files here with the ones from the zip. Skip `bootstrap-icons.svg`: that's
   the sprite sheet with every icon in one file, not an icon.
3. Update the version above, then run `uv run python scripts/gen_assets.py`. If an icon the
   manifest uses was renamed or removed, the script stops and names it.
