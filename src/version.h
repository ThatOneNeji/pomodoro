/**
 * @file version.h
 * @brief Firmware version and build time, from `git describe` at build time (see scripts/fw_version.py).
 */
#ifndef VERSION_H
#define VERSION_H

/// @return The firmware version, e.g. "v0.1.0-3-g4dfefc1-dirty", the commit hash until the repo has
/// tags, or "unknown" when built outside a git checkout.
const char *getFirmwareVersion();

/// @return When this firmware was built, e.g. "2026-10-10 08:15 UTC", or "unknown".
const char *getFirmwareBuildTime();

#endif  // VERSION_H
