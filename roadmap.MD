# Development Roadmap — Pomodoro ESP32

The project has a solid foundation: a state machine in `src/timer.cpp`, a working i18n system in `src/i18n/`, `.clang-format` and `.clang-tidy` already configured, and an empty `test/` directory ready to populate. The work splits naturally into four phases.

---

## Phase 1 — Extend Existing Systems (lowest risk, quick wins)

### 1.1 — Add languages to `src/i18n/`

The infrastructure is fully in place. Each language is a header (e.g. `src/i18n/af.h`) that defines macros matching `src/i18n/msgs_base.h`, then `src/config.h` selects the active one via `#define LANG_XX`.

Steps:

- Copy an existing language file as the template
- Translate the macro values
- Add a `#ifdef LANG_XX` block in `src/strings.h` matching the existing pattern

### 1.2 — Display timeout / blank on idle

The encoder is on pins 32/21/14 (defined in `src/config.h`). The timer state machine in `src/timer.cpp` already knows whether the timer is running. The `Stopped` and `SelectingPreset` states are the idle candidates.

Steps:

- Track last-input timestamp in `src/main.cpp` (encoder ISR already fires events)
- In the main loop, if `TimerState::Stopped` or `SelectingPreset` and elapsed > threshold, call `display.hibernate()` (GxEPD2 API)
- Any encoder event wakes the display and redraws

---

## Phase 2 — Testing Infrastructure

PlatformIO supports Unity-based tests. Tests that don't touch hardware can run natively on the host (`env:native` in `platformio.ini`).

### 2.1 — Set up host test environment

- Add a `[env:native]` section to `platformio.ini`
- Write tests for pure-logic units: timer state transitions, i18n macro resolution, statistics calculations
- Target files: `src/timer.h`/`src/timer.cpp`, `src/statistics.h`, `src/i18n/`

### 2.2 — On-device smoke tests (optional, later)

- These require the actual ESP32; leave as a stretch goal until the webserver is in place (it makes triggering test runs easier)

---

## Phase 3 — Webserver + WebSocket

This is the largest new feature. The ESP32 Arduino framework has `WebServer.h` in-tree; for async support `ESPAsyncWebServer` is a common PlatformIO library.

### 3.1 — WiFi and webserver scaffold

- Add WiFi credentials handling (ideally via `src/preferences_manager.h` so they survive reboots)
- Start the webserver in `src/main.cpp` `setup()`
- Serve a minimal status page (current `TimerState`, remaining time, active preset)

### 3.2 — REST control endpoints

Mirror the encoder actions: start/pause/stop, select preset, confirm break. These map directly onto existing `timer.cpp` methods.

### 3.3 — WebSocket for live state

- Broadcast a JSON payload whenever `TimerState` changes or the remaining-time tick fires
- Clients subscribe and update the UI without polling

> **Dependency note:** Phases 1 and 2 should be complete before starting Phase 3 — a stable, tested core makes the webserver integration far less risky.

---

## Phase 4 — CI/CD

With tests passing and Doxygen comments in place, automation is straightforward.

### 4.1 — Doxygen comments pass

- Add `///` doc comments to all public headers (start with `src/timer.h`, `src/config.h`, `src/strings.h`)
- Add a `Doxyfile` at the repo root

### 4.2 — GitHub Actions workflows (create `.github/workflows/`)

| Workflow | Trigger | Steps |
| --- | --- | --- |
| `lint.yml` | push / PR | `clang-tidy` + `clang-format --dry-run` (configs already exist) |
| `build.yml` | push / PR | `pio run -e esp32dev` |
| `test.yml` | push / PR | `pio test -e native` |
| `docs.yml` | push to main | `doxygen Doxyfile`, publish to GitHub Pages |

> The lint and build workflows can land before tests exist; just gate the test job on Phase 2 being done.

---

## Future (Post Near-Term)

**MQTT client** — straightforward once the webserver WiFi stack is in place. `PubSubClient` is the standard PlatformIO library. Publishes the same state JSON the WebSocket uses, so most of the serialisation work is already done.

**Image redo** — `scripts/gen_assets.py` already handles the source → C header pipeline. The blocker is sourcing or redrawing the original artwork, which is a design task independent of the code.

---

**Drop LPE entirely** — LPE/meme art was the original author's choice. Step one (Caturday) is the `INCLUDE_MEME_ASSETS` build switch, off by default. Once the neutral, code-drawn screens are settled, remove the switch and every LPE touchpoint (listed in `caturday.md` §9), the `lpe` preference key and the source art.

**QR code on the status screen** — show a QR code of `http://<ip>/` on the start-screen Status page (see `caturday.md` §8), so a phone can open the web UI without typing the IP. Needs a small QR library (e.g. `ricmoo/QRCode`) plus some flash. Revisit once the image redo is done and the flash budget is clearer (CI reports it, see `scripts/check_size.py`).

---

## Suggested Sequencing

```text
Phase 1a (i18n)    ──┐
Phase 1b (timeout) ──┤── independent, can run in parallel
Phase 2 (tests)    ──┘
         │
         ▼
Phase 4a (Doxygen) ────── can start before Phase 3
         │
Phase 3 (webserver) ───── needs stable tested core
         │
Phase 4b (CI/CD)   ────── gates on tests + docs both existing
         │
Future (MQTT, images)
```

The i18n and timeout work are good warm-up tasks that touch the real codebase without risk. Tests and Doxygen comments should be built up alongside each feature rather than saved for the end.
