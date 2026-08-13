/**
 * @file msgs_base.h
 * @brief X-macro listing the keys every language's `Msgs` struct is meant to define.
 *
 * @note Not currently included or expanded anywhere: en.h/de.h define their `Msgs` struct
 * members directly rather than by expanding ::MSGS_KEYS through an `X()` macro, and this list
 * (4 keys) is far smaller than what `Msgs` actually defines (~20+ keys and several message
 * lists). Treat this file as an aspirational scaffold rather than the source of truth for
 * which keys a language file must provide.
 */
#pragma once

// Every language file MUST define all of these keys.
// Add new keys here first, then to each language file.
/// Unused X-macro key list; see the file-level @note.
#define MSGS_KEYS \
    X(OPEN) \
    X(CLOSE) \
    X(STOP) \
    X(STATISTICS)
