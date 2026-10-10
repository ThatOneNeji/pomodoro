# Development Roadmap — Pomodoro ESP32

The project has a solid foundation: a state machine in `src/timer.cpp`, an i18n system in `src/i18n/`, `.clang-format` and `.clang-tidy` configured, and a `test/` directory ready to populate. The work splits into four phases, plus future features.

## Status (updated 2026-10-10)

| Item | Status | Notes |
| --- | --- | --- |
| 1.1 Add languages | 🟡 Partial | The `LANG_AF`/`LANG_DE` switch works, but both files are mostly English placeholders (`af.h` holds the German facts). Needs real translations. |
| 1.2 Blank display when idle | ✅ Done | `IDLE_BLANK_TIMEOUT` / `PAUSE_BLANK_TIMEOUT`; works from boot since the splash auto-advances (PR #5) |
| 2.1 Host tests (`env:native`) | ❌ Not started | **The biggest gap**, see "What's next" |
| 2.2 On-device smoke tests | ❌ Not started | `STRINGS_TEST` is a manual debug build, not an automated test |
| 3.1 WiFi + web server | ✅ Done | Credentials in Preferences; timer, settings and status pages |
| 3.2 Control endpoints | ✅ Done differently | WebSocket commands instead of REST (PR #1); `/wifi` and `/system` stay REST |
| 3.3 WebSocket live state | ✅ Done | Status on change + `endsAt`; documented in `websocket.md` |
| 4.1 Doxygen comments | ✅ Mostly | `Doxyfile` exists; public headers are documented |
| 4.2 GitHub Actions | 🟡 1 of 4 | `build.yml` (build + flash size budget) done; `lint.yml`, `test.yml`, `docs.yml` not |
| MQTT publish/subscribe | ❌ Not started | Plan ready below |
| Image redo | ✅ Done | SVG icons, screens drawn in code |
| Drop LPE entirely | 🟡 Step 1 of 2 | `INCLUDE_MEME_ASSETS` switch done; removal pending |
| mDNS hostname | ✅ Done | `http://pomodoro.local/` (PR #14) |
| Refresh trivia facts | 📋 Undecided | Options A/B/C below |

Flash: 83.6% at the start of 2026-10-10 → **56.4%** (~892 KB free) after it. See `caturday.md` for the day's details.

---

## Phase 1 — Extend Existing Systems (lowest risk, quick wins)

### 1.1 — Add languages to `src/i18n/`

> **Partial.** The switch works, but `af.h` and `de.h` are mostly English placeholders (`af.h` holds the German facts). Translating them is what's left; messages wrap automatically, so translations need no manual line breaks, but must stay plain 7-bit ASCII (the fonts have no accents or ß).

The infrastructure is in place. Each language is a header (e.g. `src/i18n/af.h`) defining a `Msgs` struct of strings and `std::array` message lists (their size is counted by the compiler, and `strings.cpp` static-asserts none has an empty slot); `src/config.h` selects the active one via `#define LANG_XX`.

Steps:

- Copy an existing language file as the template
- Translate the macro values
- Add a `#ifdef LANG_XX` block in `src/strings.h` matching the existing pattern

### 1.2 — Display timeout / blank on idle

> **Done.** Implemented in `Timer` (`IDLE_BLANK_TIMEOUT`, `PAUSE_BLANK_TIMEOUT`, `blankDisplay()` / `wakeDisplay()`); the original plan is kept below for reference.

The encoder is on pins 32/21/14 (defined in `src/config.h`). The timer state machine in `src/timer.cpp` already knows whether the timer is running. The `Stopped` and `SelectingPreset` states are the idle candidates.

Steps:

- Track last-input timestamp in `src/main.cpp` (encoder ISR already fires events)
- In the main loop, if `TimerState::Stopped` or `SelectingPreset` and elapsed > threshold, call `display.hibernate()` (GxEPD2 API)
- Any encoder event wakes the display and redraws

---

## Phase 2 — Testing Infrastructure

PlatformIO supports Unity-based tests. Tests that don't touch hardware can run natively on the host (`env:native` in `platformio.ini`).

### 2.1 — Set up host test environment

> **Not started, and the most valuable next step.** Two bugs fixed on 2026-10-10 would have been caught by simple tests: a message array with an empty slot (a crash risk) and "Unknown preset" for new presets. Good first candidates: `Timer::handleCommand()` state transitions, `wrapText()` line breaking, `formatDuration()`, and the per-state command rules in `websocket.md`. `wrapText()` and `Timer` use the display class, so they need a small fake display or a pure-logic seam for host builds.

- Add a `[env:native]` section to `platformio.ini`
- Write tests for pure-logic units: timer state transitions, i18n macro resolution, statistics calculations
- Target files: `src/timer.h`/`src/timer.cpp`, `src/statistics.h`, `src/i18n/`

### 2.2 — On-device smoke tests (optional, later)

- These require the actual ESP32; leave as a stretch goal until the webserver is in place (it makes triggering test runs easier)

---

## Phase 3 — Webserver + WebSocket

This is the largest new feature. The ESP32 Arduino framework has `WebServer.h` in-tree; for async support `ESPAsyncWebServer` is a common PlatformIO library.

> **Phase 3 is done** (PRs #1, #3, #14), ahead of Phases 1 and 2 in practice.

### 3.1 — WiFi and webserver scaffold

- Add WiFi credentials handling (ideally via `src/preferences_manager.h` so they survive reboots)
- Start the webserver in `src/main.cpp` `setup()`
- Serve a minimal status page (current `TimerState`, remaining time, active preset)

### 3.2 — REST control endpoints

> **Done differently:** the controls are WebSocket commands (`select`, `start`, `pause`, `resume`, `restart`, `stop`, `break`) through `Timer::handleCommand()`, since the page already holds a WebSocket open; see `websocket.md`. `GET/POST /wifi` and `GET /system` remain REST.

Mirror the encoder actions: start/pause/stop, select preset, confirm break. These map directly onto existing `timer.cpp` methods.

### 3.3 — WebSocket for live state

- Broadcast a JSON payload whenever `TimerState` changes or the remaining-time tick fires
- Clients subscribe and update the UI without polling

> **Dependency note (original):** Phases 1 and 2 should be complete before starting Phase 3. In practice Phase 3 came first; tests (Phase 2) are now the catch-up item.

---

## Phase 4 — CI/CD

With tests passing and Doxygen comments in place, automation is straightforward.

### 4.1 — Doxygen comments pass

> **Mostly done.** `Doxyfile` exists (`npm run doxygen`) and the public headers carry `///` / `@brief` docs. Publishing them is `docs.yml` in 4.2.

- Add `///` doc comments to all public headers (start with `src/timer.h`, `src/config.h`, `src/strings.h`)
- Add a `Doxyfile` at the repo root

### 4.2 — GitHub Actions workflows (create `.github/workflows/`)

> **1 of 4 done.** `build.yml` builds the firmware and the web files and enforces the flash/RAM/LittleFS budget (`scripts/check_size.py`); it's the required `firmware` check in the branch ruleset for `main` and `develop`. Next cheapest: `lint.yml` (`clang-format --dry-run`, already enforced locally by the pre-commit hook).

| Workflow | Trigger | Steps |
| --- | --- | --- |
| `lint.yml` | push / PR | `clang-tidy` + `clang-format --dry-run` (configs already exist) |
| `build.yml` | push / PR | `pio run -e esp32dev` |
| `test.yml` | push / PR | `pio test -e native` |
| `docs.yml` | push to main | `doxygen Doxyfile`, publish to GitHub Pages |

> The lint and build workflows can land before tests exist; just gate the test job on Phase 2 being done.

---

## Future (Post Near-Term)

**MQTT publish/subscribe** — ❌ not started. The client already connects (`src/mqtt_manager.cpp`) but publishes and subscribes to nothing. Plan:

- **Reuse the WebSocket payloads** documented in `websocket.md`. First move `buildStatusJson()` / `buildPresetsJson()` out of `src/web_server.cpp` into a shared file (e.g. `src/status_json.cpp`), so both transports always send identical JSON.
- **Reuse the command path.** Incoming commands use the same JSON (`{"cmd": "start"}`) and go through the same parser and queue into `Timer::handleCommand()`. The command parsing in `web_server.cpp` moves to the shared file too. The rules about which commands are valid in which state then stay identical.
- **Topics** (`<id>` = a per-device ID, e.g. derived from the MAC address, so several units can share one broker):

  | Topic | Direction | Retained | Payload |
  | --- | --- | --- | --- |
  | `pomodoro/<id>/status` | device → broker | yes | `status` JSON |
  | `pomodoro/<id>/presets` | device → broker | yes | `presets` JSON |
  | `pomodoro/<id>/availability` | device → broker | yes | `online` / `offline` (last will) |
  | `pomodoro/<id>/cmd` | broker → device | no | command JSON |
  | `pomodoro/<id>/error` | device → broker | no | `error` JSON (MQTT has no reply channel) |

  The topic says what a message is, so the `type` field is redundant on MQTT, but harmless to keep.
- **Publish status on change, not every second.** A 1 Hz countdown is a broker write every second, all day. Publish on a state or preset change, plus an `endsAt` field (epoch ms, from SNTP time) so subscribers can count down themselves. Fall back to `remainingMs` when time isn't synced.
- **Buffer size:** PubSubClient's default maximum message is **256 bytes**. The `presets` message is already ~340 bytes and would be silently dropped. Call `mqttClient.setBufferSize(1024)` (or larger) in `setupMqtt()`.
- **Later, optional:** Home Assistant MQTT discovery, so the timer appears in HA as entities without manual YAML.

**Image redo** — ✅ done (2026-10-10). Icons come from the Bootstrap SVGs via `assets/manifest.json` and `scripts/gen_assets.py`; the splash, running and break screens are drawn in code. A default build has no full-screen bitmaps. See `caturday.md` §9 and §11.

---

**Drop LPE entirely** — 🟡 step 1 of 2 done. LPE/meme art was the original author's choice. Step one (Caturday) is the `INCLUDE_MEME_ASSETS` build switch, off by default. Once the neutral, code-drawn screens are settled, remove the switch and every LPE touchpoint (listed in `caturday.md` §9), the `lpe` preference key and the source art.

**mDNS hostname (`pomodoro.local`)** — ✅ done (2026-10-10; `MDNS_HOSTNAME` in `config.h`, +27.8 KB flash). Announce the device on the local network so the web UI is at `http://pomodoro.local/` instead of an IP that can change when the router reassigns it. Uses `ESPmDNS` from the ESP32 Arduino core: a few lines in `onWiFiGotIP()` (`src/web_server.cpp`) and very little flash. Show the name on the Status screen and status page next to the IP. Laptops (Linux, macOS, Windows) and iPhones resolve `.local` names; Android support varies by version and browser, so keep showing the IP too.

*Replaces the earlier QR-code idea (dropped 2026-10-10): the Status screen already shows the IP and web address, so a QR code would only have saved typing it, for a library and drawing code.*

**Refresh the trivia facts** — 📋 undecided. Replace `Msgs::CHAT_GPT_FACTS` with new facts from the internet now and then. Messages wrap automatically since 2026-10-10, so facts are plain sentences. Options (decide later; the user leans towards A):

- **A.** A script (`scripts/fetch_facts.py`) fetches facts from an API and rewrites the list in `src/i18n/`; review the diff, commit, reflash.
- **B.** Facts move to `data/facts.txt` on LittleFS, refreshed by the same script and uploaded with `npm run uploadfs`: no firmware rebuild for new facts, and ~3.5 KB of flash freed.
- **C.** The device fetches a fact live over HTTPS, with the built-in list as the offline fallback: always new, but unreviewed, and needs TLS (affordable now).

Whatever the option, the script must: convert or skip characters outside 7-bit ASCII (the GFX fonts can't draw them, e.g. curly quotes, accents, ß); cap the length so a fact fits the bubble's 4 lines (`STRINGS_TEST` checks this); check the source's accuracy, tone and licence terms; and keep the af/de lists as they are (fetched facts would be English only).

---

## What happened, and what's next

The original plan was Phases 1 → 2 → 4a → 3 → 4b. In practice (prep on 2026-10-08, the work on 2026-10-10):

```text
Phase 4b (build.yml + size budget) ─── first, to watch flash while working
Phase 3  (web server + WebSocket)  ─── done
Phase 1b (idle blanking)           ─── already existed; fixed to work from boot
Future   (images, mDNS)            ─── done
```

**Suggested next, by value:**

1. **Tests (2.1).** The biggest gap; see the note there for first candidates.
2. **`lint.yml` (4.2).** Cheap: CI repeats the clang-format check every PR already gets locally.
3. **MQTT publish/subscribe.** The larger feature; the plan above is ready.
4. **Translations (1.1)**, **dropping LPE**, and **refreshing the facts**, as and when wanted.
