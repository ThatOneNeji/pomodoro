# Development Roadmap — Pomodoro ESP32

The project has a solid foundation: a state machine in `src/timer.cpp`, an i18n system in `src/i18n/`, `.clang-format` and `.clang-tidy` configured, and a `test/` directory ready to populate. The work splits into four phases, plus future features.

## Status (updated 2026-10-10)

| Item | Status | Notes |
| --- | --- | --- |
| 1.1 Add languages | 🟡 Partial | The `LANG_AF`/`LANG_DE` switch works, but both files are mostly English placeholders (`af.h` holds the German facts). Needs real translations. |
| 1.2 Blank display when idle | ✅ Done | `IDLE_BLANK_TIMEOUT` / `PAUSE_BLANK_TIMEOUT`; works from boot since the splash auto-advances (PR #5) |
| 2.1 Host tests (`env:native`) | 🟡 Started | `pio test -e native` runs the timer tests (TMR-01 to TMR-10); the rest are listed in `test/README.md` |
| 2.2 On-device smoke tests | ❌ Not started | `STRINGS_TEST` is a manual debug build, not an automated test |
| 3.1 WiFi + web server | ✅ Done | Credentials in Preferences; timer, settings and status pages |
| 3.2 Control endpoints | ✅ Done differently | WebSocket commands instead of REST (PR #1); `/wifi` and `/system` stay REST |
| 3.3 WebSocket live state | ✅ Done | Status on change + `endsAt`; documented in `websocket.md` |
| 4.1 Doxygen comments | ✅ Mostly | `Doxyfile` exists; public headers are documented |
| 4.2 GitHub Actions | 🟡 2 of 4 | `build.yml` (build + flash size budget) and `test.yml` (host tests) done; `lint.yml`, `docs.yml` not |
| MQTT publish/subscribe | ❌ Not started | Plan ready below |
| Image redo | ✅ Done | SVG icons, screens drawn in code |
| Drop LPE entirely | 🟡 Step 1 of 2 | `INCLUDE_MEME_ASSETS` switch done; removal pending |
| mDNS hostname | ✅ Done | `http://pomodoro.local/` (PR #14) |
| Refresh trivia facts | 📋 Undecided | Options A/B/C below |
| Battery power | 🟡 Measuring | UM34C meter + `scripts/um34c_log.py`: 88.5 mA running, 19.4 mA shut down (at 5 V); the power path decides the case |
| Case changes | 🟡 Pipeline ready | Edits are code in `cad/build_case.py`, `npm run case` rebuilds `stl/`; needs part choices and sizes |
| Branding | ❌ Not started | Name in `config.h`; nice to have: language stub script, web colour theme |

Flash: 83.6% at the start of 2026-10-10 → **56.4%** (~892 KB free) after it. See `caturday.md` for the day's details.

---

## Phase 1 — Extend Existing Systems (lowest risk, quick wins)

### 1.1 — Add languages to `src/i18n/`

> **Partial.** The switch works, but `af.h` and `de.h` are mostly English placeholders (`af.h` holds the German facts). Translating them is what's left; messages wrap automatically, so translations need no manual line breaks, but must stay plain 7-bit ASCII (the fonts have no accents or ß).

The infrastructure is in place. Each language is a header (e.g. `src/i18n/af.h`) defining a `Msgs` struct of strings and `std::array` message lists (their size is counted by the compiler, and `messages.cpp` static-asserts none has an empty slot); `src/config.h` selects the active one via `#define LANG_XX`.

Steps:

- Copy an existing language file as the template
- Translate the macro values
- Add a `#ifdef LANG_XX` block in `src/messages.h` matching the existing pattern

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

> **Started (2026-10-10).** `[env:native]` builds the real timer sources against stand-ins for the hardware in `test/fakes/` (a display that draws nothing, fake time, settings in memory), so no firmware code had to change for testing. `test/test_timer/` covers the state machine (TMR-01 to TMR-10), and CI runs it on every PR (`test.yml`). The full list of tests, with why each one exists, is `test/README.md`; next up there: messages (MSG), `wrapText()` (WRP) and formatting (FMT).
>
> Getting it to build turned up one real problem: `src/strings.h` had the same name as the C library's `<strings.h>`, so the system's own `string.h` included ours by mistake. It's now `src/messages.h` (no flash change).

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

- Add `///` doc comments to all public headers (start with `src/timer.h`, `src/config.h`, `src/messages.h`)
- Add a `Doxyfile` at the repo root

### 4.2 — GitHub Actions workflows (create `.github/workflows/`)

> **2 of 4 done.** `build.yml` builds the firmware and the web files and enforces the flash/RAM/LittleFS budget (`scripts/check_size.py`); it's the required `firmware` check in the branch ruleset for `main` and `develop`. `test.yml` runs the host tests (job `host-tests`); it can be added to the ruleset as a required check too. Next cheapest: `lint.yml` (`clang-format --dry-run`, already enforced locally by the pre-commit hook).

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
- **One "state changed" notification for both transports.** Today the status push is wired to the WebSocket. Before adding MQTT, give the timer a single point that says "state changed" and let the WebSocket and MQTT both publish from it, rather than calling each transport from the timer code. This is the logic rework expected for MQTT; do it once host tests (2.1) cover the timer.
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
- **Publish `offline` before a shutdown.** Settings → Shut down (since 2026-10-10, `src/power.cpp`) already sends a WebSocket `shutdown` message and marks the spot for MQTT. The broker only sends the last will when a connection drops unexpectedly, so a clean shutdown must publish `offline` to the availability topic itself, then disconnect.
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

**Battery power** — ❌ not started. Run the timer from a LiPo, through a switch and a voltage booster.

- **Measure before changing anything.** Power numbers come from a USB power meter (on USB now) or an INA219 inline with the battery, not from unit tests. Measure:
  - on USB, as a baseline, before the battery hardware goes in;
  - with WiFi connected;
  - with WiFi off or not configured;
  - a long session, e.g. a 24-hour preset.
- **First measurements (2026-10-10, at USB 5.2 V, see PWR-xx in `test/README.md`):** **88.5 mA** (0.46 W) running with WiFi, and **19.4 mA** (0.10 W) after Shut down. Rough battery estimates from that, for a 3.7 V LiPo: about **145 mA** through a 5 V booster (0.46 W at ~85% efficiency), or about **88 mA** through a 3.3 V regulator straight to the board's 3.3 V pin (88.5 mA × 3.3 V at ~90%). With a 2000 mAh cell, that's roughly 14 h against 23 h, before any firmware changes: the direct regulator is worth ~60% more runtime.
- **The CPU, not WiFi, is most of the draw (2026-10-10).** Idle on the blanked selection screen: 83.8 mA with WiFi on, 79.3 mA with it off. The ESP32 already sleeps the radio between transmissions; the flat ~80 mA is the main loop running flat out at 240 MHz. Next firmware levers, cheapest first: a lower CPU clock (e.g. `setCpuFrequencyMhz(80)`), then light sleep while idle. Measure each with `scripts/um34c_log.py`.
- **"Off" isn't off yet.** 19 mA in deep sleep is mostly the dev board (its regulator and USB chip) and whatever stays powered on the display and LED. A hardware power switch makes it irrelevant; otherwise it's worth chasing, e.g. holding the display's power pin low during sleep.
- **Expect the 24-hour run to fail as things stand.** An ESP32 with WiFi on averages roughly 80–120 mA; a 1000 mAh LiPo would last about 10 hours, a 2000 mAh one about 20. The e-paper uses almost nothing between refreshes, so the radio and CPU are the cost. Firmware options, in order of effort: WiFi modem sleep, light sleep between screen updates, a setting to turn WiFi off.
- **The timer copes with 24 hours.** 86.4 M ms is well inside `millis()` (wraps after ~49 days); check the settings allow entering a preset that long.
- **Choose the power path before designing the case around it.** LiPo (3.7 V) → booster (5 V) → the board's regulator (3.3 V) converts twice and wastes power both times. A 3.3 V low-dropout or buck-boost regulator straight from the LiPo usually lasts noticeably longer. Settle this before adding mounting points for the booster.
- **Battery level.** A voltage divider to an **ADC1** pin (ADC2 doesn't work while WiFi is on) lets the Status screen and status page show the charge. Cheap to wire in now, awkward to add after the case is printed.
- **Charging.** If the LiPo charges over USB, the charger board needs a place in the case too.

**Case changes** — 🟡 the editing pipeline is ready (2026-10-10); no edits yet. The original design is a public Onshape document, but exporting from it needs an Onshape account with working WebGL (or API keys, which a new account can't create), and its design history can't leave Onshape anyway. So the original STLs are the source instead: they're in `cad/original/` and never edited. `cad/build_case.py` turns them into solids with FreeCAD, applies our edits as code, and writes the printable files to `stl/` (`npm run case`). The round trip is lossless (`npm run case:check`). See `cad/README.md`.

Changes, each needing the part's sizes (calipers) and product page or datasheet:

- [ ] Change the USB cut-out.
- [ ] Add a cut-out for a power switch.
- [ ] Double the thickness of the bottom.
- [ ] Mounting points for the voltage booster (or the regulator that replaces it, see "Battery power").
- [ ] Mounting for the LiPo battery.
- [ ] A place for the charger board, if there is one.

**Branding** — ❌ not started.

- **The device name in `config.h`**, so a rebuilt unit doesn't have to say "Pomodoro". Two values: a display name (splash, web page titles, the WiFi setup network name) and a hostname (lowercase, no spaces; `MDNS_HOSTNAME` already exists). Cheap.
- *Nice to have:* **a language stub script** that creates a new `src/i18n/<lang>.h` from `en.h` when it doesn't exist yet. More useful still if it also lists which strings in each language are still identical to English: the build's `allPopulated` check only catches empty strings, not untranslated ones.
- *Nice to have:* **a colour theme for the web pages**: CSS variables, plus automatic dark mode via `prefers-color-scheme`. Tiny flash cost, as the pages are gzipped.

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

1. **More tests (2.1).** The timer tests are in; next the messages, `wrapText()` and formatting tests listed in `test/README.md`. They're the safety net for the MQTT rework.
2. **`lint.yml` (4.2).** Cheap: CI repeats the clang-format check every PR already gets locally.
3. **The shared "state changed" notification, then MQTT.** The larger feature; the plan above is ready.
4. **Choose the power path**: booster or a 3.3 V regulator, plus the battery-level divider. Doesn't need measurements, and it decides the case changes.
5. **Measure power** (PWR-01 to PWR-05 in `test/README.md`), once there's a USB power meter or an INA219 to measure with.
6. **Case changes**, once the parts are chosen.
7. **Branding**, **translations (1.1)**, **dropping LPE**, and **refreshing the facts**, as and when wanted.
