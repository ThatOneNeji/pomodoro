# Caturday Prep — Web control, live countdown, preset graphics

Prep notes written before Caturday (2026-10-08), then kept up to date as the work happened.

**Flash:** 83.6% (~336 KB free) at the 2026-10-08 baseline → **64.6% (724 KB free)** after Caturday (2026-10-10), with the WebSocket control, status screens and web pages added. CI enforces the budget, see [§6](#6-cicd--size-budget-added-2026-10-08).

## Status (updated 2026-10-10)

| § | Item | Status |
| --- | --- | --- |
| 0 | Before Saturday | ✅ Done |
| 1 | Backend: WebSocket commands | ✅ Done (PR #1) |
| 2 | Frontend: controls and countdown | ✅ Done (PR #1) |
| 3 | Preset graphics / Workshop | ✅ Done: icon (PR #2), running message (PR #7) |
| 4 | Decisions | ✅ All recorded |
| 5 | Suggested order | ✅ 6 of 7 steps done, 1 dropped (the long press) |
| 6 | CI size budget | ✅ Done; runs on every PR |
| 7 | Long press | ❌ Dropped (2026-10-10): Settings is now on the selection screen |
| 8 | Status screen | ✅ Done (PR #3); some checks still to test, see below |
| 9 | LPE switch + SVG icons | ✅ Done (PR #2), plus the drawn running screen (PR #4) |
| 10 | MQTT direction | 📋 Later (`roadmap.MD`); the `endsAt` part is done |
| 11 | Splash + selection menu | ✅ Done (PR #4, #5); break icons decided and done (PR #8) |

**Still to do:** nothing from the Caturday plan. **Later** (`roadmap.MD`): MQTT publishing, an mDNS hostname (`pomodoro.local`), fully removing LPE, tests (Phase 2).

**Also fixed along the way (PR #7):** the trivia list had 42 slots but 41 facts, so ~1 in 50 running messages could be a null pointer and crash the device. The message arrays now size themselves, and the build fails if a slot is ever empty.

**Still to test**

- [ ] The "time for a break" screen, after a work interval actually runs out (device and web page)
- [ ] A real dropped WebSocket connection: the web countdown keeps running and resyncs on reconnect
- [ ] A real WiFi save from `settings.html`: the page shows "Saved" before the device restarts
- [ ] Boot with WiFi unavailable (wrong SSID): Status says not connected, with no hang (§8)
- [ ] Status with WiFi connecting after boot: "Connecting..." changes to the IP without a button press (§8)
- [ ] MQTT shown as connected, which needs a broker (§8)
- [ ] A new commit recompiles only `version.cpp` (§8)
- [ ] The af/de translations, which are English placeholders for now (§8)

---

## 0. Before Saturday (5 minutes, no code)

- [x] Commit the current WIP (`config.h` presets + MQTT disabled, README heading fix, `.vscode/extensions.json`) so Saturday starts from a clean tree
- [x] `git switch -c feature/web-control` off `develop`
- [x] Have the replacement artwork ready as PNGs (see [§3](#3-backend--preset-graphics))

---

## 1. Backend — accept control commands

### What exists

- `src/web_server.cpp` has `AsyncWebSocket ws("/ws")`. It only **pushes** status, once a second.
- `onWsEvent()` handles `WS_EVT_CONNECT` / `WS_EVT_DISCONNECT`, but ignores `WS_EVT_DATA`.
- `Timer` already has every action needed: `selectPreset(i)`, `start()`, `pause()`, `resume()`, `startBreak()`, `stop()`.

### Gotchas (these will bite you)

1. **Threading.** ESPAsyncWebServer callbacks run on the AsyncTCP task, not in `loop()`. `Timer` isn't thread-safe and drives the SPI display. **Never call `timer.start()` directly from `onWsEvent()`.**
   Have the handler enqueue a command (a FreeRTOS queue, or a single `std::atomic` pending-command slot) and drain it in `loop()` before or inside `timer.loop()`.
2. **Redraw.** `start()` doesn't request a redraw by itself. The encoder path sets `needsFullRedraw = true` right after calling it (`src/states/timer_selecting_preset.cpp:24-26`). A web-triggered start must do the same, or the e-paper keeps showing the old screen.
3. **Blanked display.** After 10 s idle in `SelectingPreset` (or 5 min while paused), the panel is hibernated (`displayBlanked`). A web command arriving then must wake it with a full `display.init()`. `wakeDisplayIfTriggered()` is private and only checks the button/encoder, so this needs a small public hook on `Timer`.
4. **Encoder sync.** The state methods already call `topMenu->setEncoderCount(lastEncoderCount)`, so physical and web input shouldn't fight. Still worth testing: turn the knob after a web start.

### Command set (client → server, over the existing `/ws`, no new REST endpoints)

| Command | Payload | Maps to | Valid in |
| --- | --- | --- | --- |
| select | `{"cmd":"select","preset":1}` | `selectPreset(1)` + redraw, so the e-paper shows the selection too | SelectingPreset |
| start | `{"cmd":"start"}` | `start()` on the currently selected preset (the "confirm") | SelectingPreset, Stopped |
| pause | `{"cmd":"pause"}` | `pause()` | Running, RunningBreak |
| resume | `{"cmd":"resume"}` | `resume()` | UserInitiated*Pause |
| restart | `{"cmd":"restart"}` | `start()` (same preset, interval reset, **cycle count kept**) | Running, UserInitiatedPause |
| stop | `{"cmd":"stop"}` | `stop()` | anything but SelectingPreset |
| break | `{"cmd":"break"}` | `startBreak()` | Running, WaitingConfirmStartOfBreak |

Reject invalid-for-state commands, and log them with `ESP_LOGW`.

**Restart keeping cycles comes for free:** `start()` only calls `reset()`, which clears per-interval timekeeping and never touches `cycles`/`minutesWorked`. Still to decide on the day: should restart during a break restart the *break*? `start()` would jump back to a work interval.

**Why WebSocket:** the connection is already open and two-way, and status replies come back on the same channel. The trade-off is that you can't poke it with `curl`. Use the browser console instead: `websocket.send(JSON.stringify({cmd:"start"}))`.

### Status JSON additions (server → client)

Today it sends `{state, presetName, remainingMs}`. For a good countdown UI, add:

- `presetIndex`, so the frontend can highlight the active tab
- `durationMs`, the length of the current interval, for a progress bar/ring (needs a small getter on `Timer`)
- `isLongBreak` and `cycles` (both already members of `Timer`; they just need getters)
- **On connect only:** a `presets` array (`[{name, durationMs, pauseMs, longPauseMs, longPauseAfter}]`) built from `PRESETS[]` in `config.h`, so the frontend stops hard-coding 25/5/15
- Optionally, broadcast immediately on a state change instead of waiting for the next 1 s tick

---

## 2. Frontend — controls + countdown (`html/index.html`)

### What exists

- The WebSocket connects and reconnects with backoff, but `onmessage` only does `console.log`.
- The visible timer is a **fake local timer** (`MODES` hard-coded to 25/5/15 min, driven by `setInterval`). This all gets replaced.

### Tasks

All done in PR #1. The countdown ended up running locally from `endsAt` (see §10), rather than from `remainingMs`.

- [x] Build the mode tabs from the server's `presets` list. Clicking a tab sends `select` only; the Start button sends `start` (the confirm). Lock the tabs while a timer runs.
- [x] Render `remainingMs` from the WS message. Between the 1 s broadcasts, either show the last value or interpolate locally and re-sync on each message.
- [x] Map `state` to button labels: Start / Pause / Resume / Skip break / Stop.
- [x] Add a "disconnected" visual state while the WS is reconnecting.
- [x] Use `session-count` for `cycles`.

### Workflow reminder

```bash
npm run minify     # html/ → data/ (inlines + purges CSS)
npm run datasize   # check against the ~1.9 MB LittleFS budget
npm run uploadfs   # gzip + flash data/ to LittleFS
```

Quicker iteration: open `html/index.html` locally and point `gateway` at the device's IP temporarily.

---

## 3. Backend — preset graphics

### Bug to fix first: the new "Workshop" preset is half-wired

Presets are matched **by name string** in three places, and none of them knows about "Workshop":

| Where | What happens for "Workshop" today |
| --- | --- |
| `IconProvider::getPresetIcon()` (`src/icon_provider.h`) | Falls back to `icon_lpehacker`, **the LPE meme icon, even with LPE mode off** |
| `Timer::getRunningMessage()` (`src/states/timer_running.cpp:170`) | Shows the literal text **"Unknown preset"** in the speech bubble |
| `Messages` enum (`src/strings.h`) + `src/i18n/{en,af,de}.h` | No `Preset_Workshop_Message` |

"Emails" is gone from `PRESETS[]`, but its branches are still in all three places (harmless, just dead code).

**Suggestion:** stop matching by name. Add the icon (or an icon key) and a message ID to `PresetConfig`, so adding a preset means changing one row in `config.h`.

### Also noticed

- `enterPresetSelection()` always selects **index 1**. With the new list, that's now "Workshop" (it used to be "Coding"). Is that intended?
- Long-break images are hard-coded in `Timer::startBreak()`: `cat`, `stonks`, `pablo`, `what_a_week`.

### Asset pipeline

> **See [§9](#9-graphics-direction--svg-icons-added-2026-10-08)** for the proposed SVG-based icon pipeline and the neutral/LPE image split. The notes below describe how it works today.


- Icons: 512×512 PNG → `assets/icons/`. The script outputs 192/128/64/48 px, dithered to 1-bit.
- Backgrounds: **800×480** PNG → `assets/images/`. Each one costs **48,000 bytes of flash**.
- Run `python scripts/gen_assets.py` (needs Pillow; see `pyproject.toml`). It regenerates `src/icons.h`, `src/icons/icons.cpp`, `src/images/*.h` and `src/images.h`.
- **Flash budget:** about 336 KB free, which is about **7 more backgrounds at most**, and that's before Saturday's web code. Replace images rather than add them, or drop unused ones. For example, `src/images/image_bg_lpehacker_bubble.h` isn't included in `images.h`, and `image_bg_splash` is worth checking.
- Direction from earlier notes: make non-meme images the default, and offer memes / user images ("Mah images") as an option. The existing `lpe` checkbox + `IconProvider` is the natural switch point.

---

## 4. Decisions (2026-10-08)

| Question | Decision |
| --- | --- |
| Commands over WebSocket or REST? | **WebSocket**, so no new REST endpoints |
| Clicking a preset tab | **Select only**, then confirm with Start |
| Restart | **Keep the cycle count** |
| Long-press time | ~~**3 s** (`LONG_PRESS_MS 3000`)~~ superseded: long press dropped |
| Long-press behaviour | ~~**`ESP.restart()`** back to the start/settings screen~~ superseded: long press dropped |
| Short presses | **Act on release**, and only if held for less than `LONG_PRESS_MS` |
| Status screen contents | **All items** from §8; sizing and layout reviewed during development |
| Status screen exit | **Press** → back to the start screen |
| Status screen firmware version | **Yes**: git-derived `FW_VERSION`, see §8 |
| LPE / meme assets | **Drop eventually.** For now, a build switch `INCLUDE_MEME_ASSETS`, **off by default** (see §9) |
| Neutral screens | **Drawn in code** (shapes + Bootstrap icons), not stored as bitmaps |
| Start screen | **"Pomodoro" + Bootstrap `stopwatch` icon**, drawn in code |
| Long press (§7) | **Dropped** (2026-10-10): Settings is on the preset selection screen now |
| Status screen QR code | ~~Future feature~~ **Dropped** (2026-10-10): Status already shows the IP; an mDNS hostname (`roadmap.MD`) replaces it |

---

## 5. Suggested Saturday order

1. ✅ Fix Workshop wiring (or move to config-driven presets). Small, and it unblocks the graphics work.
2. ✅ Status JSON additions + `presets` on connect. Check with the browser console, since `console.log` is already there.
3. ✅ Command queue + `select` / `start` / `stop` only, end-to-end with minimal frontend buttons.
4. ✅ The rest of the commands + full frontend render.
5. ✅ Graphics swap, within the flash budget.
6. ❌ *(dropped, see §7)* Long-press → back to the splash/settings screen ([§7](#7-long-press-encoder--back-to-the-startsettings-screen-added-2026-10-08)). It's independent of the web work, so it can also be the warm-up.
7. ✅ Status screen on the start screen ([§8](#8-status-screen-on-the-start-screen-added-2026-10-08)). Once §7 is done, a long press gets you straight to it.

---

## 6. CI/CD — size budget (added 2026-10-08)

`.github/workflows/build.yml` runs on pushes to `main`/`develop` and on every PR:

1. Builds the firmware with placeholder credentials (copied from `credentials.example.h`)
2. Rebuilds `data/` (`npm run minify` + gzip)
3. Runs `scripts/check_size.py`, which adds warning/error annotations and a size table to the run summary

| Budget | Warn at | Fail at | Today |
| --- | --- | --- | --- |
| Flash (firmware) | 85% | 95% | 83.6%, **only ~30 KB below the warning** |
| RAM (static) | 70% | 85% | 30.7% |
| LittleFS (`data/`) | 80% | 95% | 1.8% |

Thresholds are constants at the top of `scripts/check_size.py`. Run it locally with `npm run sizecheck`.
Expect Saturday's web code plus any new images to set off the flash warning. That's the point: it's a prompt to swap images out instead of adding them.

---

## 7. Long-press encoder → back to the start/settings screen (added 2026-10-08)

> **Dropped (2026-10-10).** Its purpose was reaching Settings without a power cycle, and Settings is now on the preset selection screen (§11). The plan below is kept for reference only.

**Goal:** holding the encoder button for more than *X* seconds, from any state, stops everything and returns to the boot "Start / Settings" screen. Today the only way back to it is a power cycle.

### How it works today

- The splash screen (`SplashScreen::draw()` + `loop()`) runs **once, inside `setup()`**, in its own blocking `while (true)` loop (`src/main.cpp`, `src/splashscreen.cpp`).
- Saving any setting already ends in `ESP.restart()` (`src/splashscreen.cpp:110-136`). The settings screen never hands control back.
- `Button` (`src/button.cpp`) only knows about **presses**: the ISR fires on the `FALLING` edge with a 1000 ms debounce and sets `pressed`. It has no idea how long the button is held.

### Decided: long-press (3 s) = `ESP.restart()`

Settings already restart the device, so a restart is what "go back to the start screen" means in practice. It's a few lines of code, and it can't leave the `Timer`, display or web server in a half-torn-down state.

| | Restart (chosen) | Return in place, no reboot |
| --- | --- | --- |
| Effort | Small | Large: the splash loops are blocking and built for `setup()` only |
| Side effects | ~2-3 s reboot, WiFi/MQTT reconnect, in-RAM cycle count lost | Web status and MQTT stall while the blocking splash loop runs |
| Persisted stats (`tcs`, `tt`, `tbt`) | Kept (they live in Preferences) | Kept |

### Implementation notes / gotchas

1. **Rework `Button` so short presses act on release (decided).** Keep all the timing inside `Button`, so callers don't change:
   - Attach the ISR on `CHANGE` instead of `FALLING`. On a falling edge, record `pressStartTime`. On a rising edge, if the hold was shorter than `LONG_PRESS_MS` and no long press fired, set `pressed = true`.
   - Add `bool isLongPress()` (or similar) that `loop()` polls. It returns true **once**, as soon as the button has been held `LONG_PRESS_MS`, *while still held*. Don't wait for release: the restart should happen at 3 s, not when the user lets go. It also flags the current press, so the release doesn't then count as a short press.
   - `checkAndClearButtonPress()` keeps its name and signature, so the call sites in `Timer`, `src/states/*.cpp` and `SplashScreen` don't change. They just see presses a moment later, on release.
   - **Debounce:** the current 1000 ms debounce (`DEBOUNCE_DELAY` in `src/button.cpp`) is far too long once both edges matter. Use ~30-50 ms per edge, and ignore edges that don't match the last known pin state, so contact bounce can't fake a release.
   - Put the threshold in `config.h` as `#define LONG_PRESS_MS 3000`.
2. **Side effect to expect:** every press will feel slightly later, because it now acts on release. Do a quick check that it still feels fine in the settings screen and on the "wake the display" press.
3. **Feedback while holding.** E-paper is too slow to show a hold progress indicator. Use the LED instead: add a `LedMode` (e.g. a fast flash) once the hold passes ~1 s, so the user knows to keep holding.
4. **Check it from anywhere.** It has to work in every `TimerState`, including while the display is blanked. Check it in `main.cpp`'s `loop()` *before* `timer.loop()`, so a blanked or paused state can't swallow it.
5. **Not in the splash/settings loops.** Those have their own `while (true)` loops and don't run `main.cpp`'s `loop()`, so the long-press check won't fire there. That's fine, since you're already on the target screen.
6. **Clean shutdown before the restart (optional):** `timer.stop()`, then a short `delay()` so any in-flight web response finishes.
7. **Web freebie:** the same function can back a `{"cmd":"menu"}` WebSocket command (see §1), so the web UI can send the device back to the start screen too.

### Test checklist (not applicable: dropped)

- Long-press in each state: SelectingPreset, Running, paused, RunningBreak, WaitingConfirm*, display blanked
- A short press still works normally in each state, and acts on release
- A long press does **not** also trigger the short-press action (e.g. no extra cycle counted on "Break now")
- Fast double presses aren't lost now that the debounce is shorter, and contact bounce doesn't register as two presses
- A hold just under `LONG_PRESS_MS` does *not* restart
- After the restart, the splash screen appears, and Settings is reachable and saves

---

## 8. Status screen on the start screen (added 2026-10-08)

**Goal:** after a power cycle or restart, the start screen offers **Start · Settings · Status**. Status shows the unit's IP address and other basic device info.

> **Done (2026-10-10, `feature/status-screen`).** How it ended up:
>
> - **Device:** five pages turned with the knob (Network, Time & MQTT, Device, Firmware, Storage & stats); a press returns to the start screen menu. It redraws immediately when WiFi, the IP, time sync or MQTT change, otherwise every 30 s. It calls `mqttLoop()` while open, so MQTT shows its real state.
> - **Shared data:** `collectSystemStatus()` (`src/system_status.h`) feeds both the device screen and `GET /system`.
> - **Web:** `status.html`, reached from an info-circle icon on the timer page. The read-only System card moved there from `settings.html`, and Back is now an arrow icon.
> - **Firmware version:** `git describe` via `scripts/fw_version.py`, as planned below.
> - **Firmware size** is measured against the running app partition: with `no_ota.csv`, `ESP.getFreeSketchSpace()` reports an OTA partition that doesn't exist.
> - **Found on the way: fonts were duplicated per source file** (`const` arrays in headers, included from `config.h`): 34,418 B of duplicate font data, and the status screen would have added ~21 KB more. Fonts are now defined once in `src/fonts.cpp`. Net flash with the whole status screen: **68.1% → 67.0%**.
> - **Not done:** `fetch-depth: 0` in CI (only needed once the repo has tags). The QR code was later dropped in favour of an mDNS hostname (`roadmap.MD` → Future).

### How it works today

- The start screen menu is built in `src/splashscreen.h:29`, with 2 items: `"Start"` (hard-coded English) and `Msgs::SETTINGS`.
- `SplashScreen::loop()` (`src/splashscreen.cpp:53`) handles index 0 → `timer.start()` and index 1 → settings. **Any** button press then `return`s, so there's no "go back to the start screen" path yet.
- WiFi starts in `setup()` **before** the start screen (`setupWebServer()` at `src/main.cpp:139`). But it connects asynchronously, so the IP often won't exist yet when the start screen first appears.
- `GET /system` (`src/web_server.cpp`) already gathers some of this info (SNTP, MQTT) for `settings.html`.

### Gotchas

1. **The IP arrives late.** Show "Connecting to <SSID>..." until `WiFi.status() == WL_CONNECTED`, then redraw with the IP. Re-check about once a second and only redraw **when something changed**. E-paper refreshes are slow and wear the panel, so don't redraw on a timer.
2. **MQTT will always look disconnected here.** The start screen's `while (true)` loop never calls `mqttLoop()` (or `webServerLoop()`). PubSubClient only connects from `mqttLoop()`, so the status screen would show "MQTT: not connected" even when the broker is fine. Either call `mqttLoop()` from inside the status loop, or show "MQTT: <broker> (connects after Start)". The first is better, and cheap.
3. **SNTP only starts once WiFi is up** (`onWiFiGotIP` → `setupSntp()`), so the time may also show "not synced" for the first few seconds. That's handled by the same redraw-on-change.
4. **The menu loop needs a "back" path.** Settings always ends in `ESP.restart()`, so it never had to return. Status has to go back to the start screen on a press. Restructure `SplashScreen::loop()` so that after the status screen exits it redraws the start screen and keeps looping, instead of `return`ing.
5. **Menu width.** The 3 items share the 800 px width: (800 − 2×20 padding − 2×20 gaps) / 3 ≈ 240 px each. That fits "Settings"/"Status" in `SUB_FONT`. Check the af/de translations fit too.
6. **i18n.** Add `Msgs::STATUS` to `src/i18n/{en,af,de}.h`. While you're in there, `"Start"` is still hard-coded; give it a `Msgs::START` too.
7. **Share the data with the web.** Write one `collectSystemStatus()` (a struct, or filling a `JsonDocument`) used by both the e-paper screen and `GET /system`, so the two can't drift apart.

### Contents (decided: all of them)

| Group | Item | Source |
| --- | --- | --- |
| Network | IP address, SSID, signal (RSSI dBm), MAC | `WiFi.localIP()`, `getWifiSSID()`, `WiFi.RSSI()`, `WiFi.macAddress()` |
| Network | Web UI URL (`http://<ip>/`) | derived |
| Time | Synced yes/no, current time, timezone | `isTimeSynced()`, `getCurrentTimeString()`, `SNTP_TIMEZONE` |
| MQTT | Broker:port, connected yes/no (or "disabled") | `MQTT_BROKER_ADDRESS`, `isMqttConnected()` |
| Device | Chip model/revision, CPU MHz, uptime, free heap | `ESP.getChipModel()`, `ESP.getChipRevision()`, `ESP.getCpuFreqMHz()`, `millis()`, `ESP.getFreeHeap()` |
| Firmware | **Version (git)**, build date/time, firmware size / free app space | `FW_VERSION` (see below), `__DATE__ __TIME__`, `ESP.getSketchSize()`, `ESP.getFreeSketchSpace()` |
| Storage | LittleFS used / total | `LittleFS.usedBytes()`, `LittleFS.totalBytes()` |
| Stats | Total cycles, total work time, total break time | `getStatistics()` (`src/statistics.h`) |

### Optional extras

- ~~QR code~~: dropped (2026-10-10). Status already shows the IP; an mDNS hostname (`pomodoro.local`, `roadmap.MD` → Future) is the cheaper fix for not having to type it.
- **Same screen in the web UI:** `settings.html` already shows `/system`. With the shared `collectSystemStatus()`, it gets the extra fields for free.

### Firmware version (decided: yes)

- Add `scripts/fw_version.py` as a PlatformIO pre-build script (`extra_scripts = pre:scripts/fw_version.py` under `[env]` in `platformio.ini`). It runs `git describe --tags --always --dirty` and writes `src/version_generated.h` containing `#define FW_VERSION "<result>"`. Add that file to `.gitignore`.
- **Use a generated header, not a `-D` build flag.** A `-DFW_VERSION=...` flag changes the compile command for *every* file, so each new commit would trigger a full rebuild. Instead, the script only rewrites the header when the value actually changed, and only one small `src/version.cpp` (`const char *getFirmwareVersion()`) includes it. A version change then recompiles just that one file.
- **No tags exist yet**, so today it outputs just the short hash: `4dfefc1-dirty`. Tagging releases (`git tag v0.1.0`) changes that to `v0.1.0-3-g4dfefc1-dirty` automatically. Worth tagging a first version once Caturday's work is merged.
- **Fallback:** if `git` isn't available or it isn't a git checkout (e.g. a downloaded zip), define `FW_VERSION` as `"unknown"` rather than failing the build. Have `version.cpp` use `__has_include("version_generated.h")` with an `"unknown"` fallback, so editors, linters and `cppcheck` that don't run the script still compile.
- **The `-dirty` suffix is a feature:** it shows at a glance when the unit is running uncommitted code.
- **Use it everywhere:** the status screen, `GET /system` (and so `settings.html`), the boot log (`ESP_LOGI` in `setup()`), and later the MQTT state payload.
- **CI:** `actions/checkout` fetches with depth 1 and no tags, so `git describe` only gives the hash. Once you start tagging, set `fetch-depth: 0` in `.github/workflows/build.yml` so CI builds report the proper version.
- **Call `getFirmwareVersion()` everywhere else.** Nothing outside `version.cpp` should include the generated header, or those files will recompile on every commit too.

### Decided

- **Show everything** in the table. That's roughly 20+ lines, more than the ~10-12 that fit in `SMALL_FONT`, so plan for **two columns or knob-paged groups** (Network / Time & MQTT / Device & Firmware / Storage & Stats). Settle on one on Saturday, once it's on the panel.
- **A press exits** back to the start screen. If you page with the knob, turning it pages and pressing still exits.

### Test checklist

- [ ] Boot with WiFi available: "Connecting..." changes to the IP without any button input *(still to test)*
- [ ] Boot with WiFi unavailable (wrong SSID): the screen still works and says not connected, with no hang *(still to test)*
- [x] MQTT shows the correct state when disabled (`MQTT_BROKER_ADDRESS` empty). *Connected: still to test.*
- [x] The firmware version matches `git describe` and shows `-dirty` with uncommitted changes. *Only `version.cpp` recompiling: still to test.*
- [x] Every item is readable (no clipping or overlap), on every page
- [x] A press returns to the menu, and everything still works after visiting Status (since §11: back to preset selection)
- [ ] Menu text fits in en/af/de *(still to test; af/de are English placeholders)*

---

## 9. Graphics direction + SVG icons (added 2026-10-08)

### What's in the build today

| Asset | Content | Meme / third-party? | Flash |
| --- | --- | --- | --- |
| `bg_bubble` | Neutral person + speech bubble | No: **the neutral template** | 48,000 B |
| `bg_lpe_bubble` | LPE character + speech bubble | Yes | 48,000 B |
| `bg_splash` | "Zenlina v1.0" + LPE character | Yes (character) | 48,000 B |
| `bg_cat`, `bg_stonks`, `bg_pablo`, `bg_what_a_week` | Long-break images: cat photo, Stonks meme, *Narcos* still, *Tintin* panel | Yes. *Narcos* and *Tintin* are copyrighted works, not just memes | 4 × 48,000 B |
| 5 × `lpe*` icons | LPE character poses | Yes | 5 × 7,456 B |
| 6 × plain icons | Bootstrap Icons (MIT), already credited in `THIRD_PARTY_LICENSES.md` | No | 6 × 7,456 B |

So **6 of the 7 backgrounds (288 KB) and ~37 KB of icons are meme/third-party content.** That's most of the image flash.

### Problems found while looking

1. **LPE mode defaults are inconsistent.** `src/main.cpp:151` reads `pref_getCheckbox("lpe", true)`, and the settings checkbox (`src/splashscreen.h:32`) also defaults to `true`. But `MessageCache::isLpeModeEnabled()` (`src/strings.cpp:7`) defaults to `false`. On a fresh device, the icons are LPE but the messages aren't.
2. **LPE images are used even with LPE mode off:**
   - The confirmation screen always draws `icon_lpetantrum` (`src/states/timer_waiting_for_confirmation.cpp:66`)
   - The settings checkboxes use `icon_lpehacker` / `icon_lpenote` (`src/splashscreen.h:32-33`)
   - The splash background always has the LPE character
   - The long-break images are picked regardless of mode (`Timer::startBreak()`)
3. **A fresh clone can't regenerate the plain icons.** `.gitignore` excludes `assets/icons/*.png` except `lpe*.png`, so `checkmark.png`, `coding.png` etc. only exist on your machine. The generated `src/icons/icons.cpp` is committed, so the build works, but nobody else can change an icon.

### SVG icons from Bootstrap Icons: yes, as the *source*

**The 16 KB PNG size doesn't reach the device.** `gen_assets.py` turns every icon into 1-bit bitmaps at 192, 128, 64 and 48 px: **7,456 bytes per icon**, whatever the source file is. What matters for flash is how many icons there are and at which sizes.

**The ESP32 can't draw SVG directly.** GxEPD2 / Adafruit GFX have no SVG renderer, and adding one would cost far more flash and RAM than the bitmaps. So the plan is:

- **Commit Bootstrap SVGs as the icon source** (`assets/icons/*.svg`), and rasterize them **at build time** in `scripts/gen_assets.py`. `html/icons/` already holds the same Bootstrap SVGs, so the device and the web UI share **one standard icon library**, and fresh clones can regenerate everything (fixes problem 3).
- **Rasterizer:** `resvg-py` (pip wheels, no system libraries) is the simplest fit. `cairosvg` also works, but it needs the system Cairo library. Add whichever you pick to `pyproject.toml`.
- **Sharper icons for free:** render each size straight from the vector, and use a plain **threshold instead of Floyd-Steinberg dithering**. Dithering suits photos but gives line icons fuzzy, speckled edges. Keep dithering for photo-like backgrounds only.
- **Only generate the sizes actually used.** Today every icon gets all 4 sizes. Real usage: preset icons → 192, confirmation icon → 128, checkbox icons → 64, checkmark/coffee → 48, and `icon_warning` / `icon_lpesip` aren't used in release builds at all. A small manifest (e.g. `assets/icons/icons.json`: name → SVG file + sizes) plus a `nullptr`-safe `Icon::scaled()` would save **~50 KB of flash**.
- **Adding a new icon then means:** download the SVG from icons.getbootstrap.com → add it to `assets/icons/` → add one manifest line → run `gen_assets.py`. Keep `THIRD_PARTY_LICENSES.md` as is (the MIT notice covers the whole set) and list which icons are used.
- The LPE icons are drawings, not SVGs, so the PNG path stays for them (and for any "Mah images" user art).

### Icon library: `assets/svg/` (added 2026-10-08)

The full Bootstrap Icons set is now in `assets/svg/`: **2,078 icons, ~1.2 MB** plus the sprite file. It's untracked, not yet committed. Every icon named in this plan is there (`stopwatch`, `tools`, `hammer`, `bell`, `chat-quote`, `cup-hot(-fill)`, `tree`, `moon-stars`, `emoji-smile`, `person-circle`/`person-fill`, `check-lg`, `code-slash`, `bullseye`, ...).

- **`assets/svg/bootstrap-icons.svg` is not an icon.** It's the 1.1 MB sprite sheet with every icon in one file (half the folder's size). Delete it before committing, or have the script skip it, or `gen_assets.py` will try to treat it as an icon.
- **The library stays as-is; a manifest picks from it.** The device only gets the icons listed in the manifest (e.g. `assets/icons.json`: `{"coding": {"svg": "code-slash", "sizes": [192]}, ...}`), so the 2,000 unused icons cost no flash. This replaces the earlier idea of copying SVGs into `assets/icons/`.
- **Format notes for the rasterizer:** every icon uses `viewBox="0 0 16 16"` and `fill="currentColor"`. Render at the target pixel size (16 → 192 is a clean 12×) and make `currentColor` black. resvg defaults it to black, but check this on the first render.
- **Licence + version:** keep the MIT notice in `THIRD_PARTY_LICENSES.md` (already there). It's worth adding the Bootstrap Icons **version** these came from, e.g. in a short `assets/svg/README.md`, so a later refresh knows what it's replacing.
- **Web UI:** `html/icons/*.svg` can later be copied from `assets/svg/` by the `minify` script instead of being kept by hand, so there's one source for both.
- **Commit it:** ~1.2 MB of text is fine for git and makes the pipeline reproducible on any clone. Committing only the used icons is the alternative if you'd rather keep the repo lean.

### Images: proposed direction

- **Neutral is the default, and LPE/memes are opt-in.** Keep the existing `lpe` setting as the switch, but default it to `false` everywhere (fixes problem 1), and route *every* image choice through `IconProvider` (fixes problem 2). That includes the confirmation icon, settings icons, splash and break images.
- **`bg_bubble` is the template for the neutral running screen.** Better still, it could be **drawn at runtime** instead of stored as a 48 KB bitmap: a rounded-rect bubble from GFX primitives plus the Bootstrap `person-circle`/`person-fill` icon at 128 px. That's about 2 KB instead of 48 KB.
- **Neutral long breaks:** instead of photos, use a large Bootstrap icon (`cup-hot`, `tree`, `moon-stars`, `emoji-smile`...) plus a line of text, also drawn at runtime. That costs almost nothing in flash, and the variety comes from the icon and the message.
- **Neutral splash:** replace "Zenlina" + character with "Pomodoro" (from i18n) and a Bootstrap icon such as `stopwatch` or `hourglass-split`, drawn at runtime.
- **Optional build flag `INCLUDE_MEME_ASSETS`** in `config.h`. When off, the LPE icons and meme backgrounds aren't compiled in at all, freeing **~275 KB** (5 backgrounds + 5 LPE icons; the splash is replaced separately). That covers forks that don't want the memes anywhere in the binary, and it makes a lot of room for the web work and the QR code.
- **"Mah images" (user images)** fits on top of this later: same 800×480 / icon PNG pipeline, behind the same `IconProvider` switch.

### Decided (2026-10-08)

1. **LPE goes away eventually.** It was the original author's taste, and dropping it removes a flash cost. Step one is the build switch (point 3); full removal comes later (see `roadmap.MD` → Future).
2. **Neutral screens are drawn in code.** Running bubble, long breaks and splash use GFX primitives + Bootstrap icons, not 48 KB bitmaps.
3. **Build switch `INCLUDE_MEME_ASSETS`** in `config.h`, **default `false`**. When it's `false`:
   - None of the LPE icons, `bg_lpe_bubble`, `bg_cat`, `bg_stonks`, `bg_pablo` or `bg_what_a_week` are compiled in
   - The "LPE Mode" checkbox is hidden from Settings (`src/splashscreen.h:32`), and `IconProvider` always returns neutral assets
   - The LPE message branches in `src/strings.cpp` and the `RESTARTTIMERMSGLPE` arrays in `src/i18n/*.h` are compiled out
   - This also fixes the LPE default mismatch from problem 1, since nobody can turn LPE on in a default build
4. **Start screen:** "Pomodoro" + Bootstrap `stopwatch`, drawn in code. `bg_splash` (48 KB) goes.

### Graphics changes and their flash cost (done 2026-10-10, `feature/lpe-switch-svg-icons`)

*Current Size* is the source file on disk. *ESP Size* is what the asset costs in the firmware's flash: an icon costs `size × size / 8` bytes per generated size, and an 800×480 image costs 48,000 B. Before this change every icon was generated at all four sizes (192/128/64/48 = 7,456 B). "Meme only" assets cost 0 B in a default build and the shown amount with `INCLUDE_MEME_ASSETS`.

| File | What | Current Size | ESP Size | Replaced with | Current Size | ESP Size | Size diff |
| --- | --- | ---: | ---: | --- | ---: | ---: | ---: |
| `assets/icons/coding.png` | Coding preset icon | 20,974 B | 7,456 B | `assets/svg/code-slash.svg` @192 | 455 B | 4,608 B | −2,848 B |
| `assets/icons/focus.png` | Focus preset icon | 64,873 B | 7,456 B | `assets/svg/bullseye.svg` @192 | 434 B | 4,608 B | −2,848 B |
| — | Workshop preset icon (was the LPE fallback) | — | 0 B | `assets/svg/tools.svg` @192 | 922 B | 4,608 B | +4,608 B |
| `assets/icons/checkmark.png` | Checked checkbox | 16,136 B | 7,456 B | `assets/svg/check-lg.svg` @48 | 321 B | 288 B | −7,168 B |
| `assets/icons/coffee.png` | Break marker, preset screen | 20,191 B | 7,456 B | `assets/svg/cup-hot-fill.svg` @48 | 2,113 B | 288 B | −7,168 B |
| `assets/icons/email.png` | Emails preset icon (preset removed) | 31,219 B | 7,456 B | removed | — | 0 B | −7,456 B |
| `assets/icons/warning.png` | Unused | 21,722 B | 7,456 B | removed | — | 0 B | −7,456 B |
| `assets/icons/lpesip.png` | Debug icon-scaling test only | 160,734 B | 7,456 B | removed (test uses `stopwatch`) | — | 0 B | −7,456 B |
| `assets/icons/lpenote.png` | "Messages" setting icon | 192,499 B | 7,456 B | `assets/svg/chat-quote-fill.svg` @64 | 941 B | 512 B | −6,944 B |
| `assets/icons/lpetantrum.png` | "Time's up" confirmation icon | 147,454 B | 7,456 B | `assets/svg/bell-fill.svg` @128 (+ `lpetantrum.png` @128, meme only) | 295 B | 2,048 B (+2,048) | −5,408 B |
| `assets/icons/lpehacker.png` | LPE Coding icon, LPE Mode setting, fallback icon | 108,030 B | 7,456 B | meme only, @192 + @64 | 108,030 B | 0 B (5,120) | −7,456 B |
| `assets/icons/lpethink.png` | LPE Focus icon | 155,422 B | 7,456 B | meme only, @192 | 155,422 B | 0 B (4,608) | −7,456 B |
| `assets/images/bg_splash.png` | Start screen ("Zenlina" + LPE character) | 74,075 B | 48,000 B | drawn in code: "Pomodoro" + `assets/svg/stopwatch.svg` @192 | 487 B | 4,608 B | −43,392 B |
| `assets/images/bg_bubble.png` | Running screen background | 78,274 B | 48,000 B | unchanged (neutral; drawing it in code is a follow-up) | 78,274 B | 48,000 B | 0 B |
| `assets/images/bg_lpe_bubble.png` | LPE running background | 105,480 B | 48,000 B | meme only | 105,480 B | 0 B (48,000) | −48,000 B |
| `assets/images/bg_cat.png` | Long break image | 316,368 B | 48,000 B | meme only; neutral long breaks use the statistics layout | 316,368 B | 0 B (48,000) | −48,000 B |
| `assets/images/bg_stonks.png` | Long break image | 558,703 B | 48,000 B | meme only | 558,703 B | 0 B (48,000) | −48,000 B |
| `assets/images/bg_pablo.png` | Long break image | 676,518 B | 48,000 B | meme only | 676,518 B | 0 B (48,000) | −48,000 B |
| `assets/images/bg_what_a_week.png` | Long break image | 491,292 B | 48,000 B | meme only | 491,292 B | 0 B (48,000) | −48,000 B |
| `src/lpe.h` | Old LPE bitmaps, not included anywhere | 339,659 B | 0 B | deleted | — | 0 B | 0 B |
| `src/images/image_bg_lpehacker_bubble.h` | Not included in `images.h` | 298,277 B | 0 B | deleted | — | 0 B | 0 B |
| **Total assets** | | | **418,016 B** | | | **69,568 B** (+251,776 meme) | **−348,448 B** |

**Measured firmware size** (`pio run`, whole binary): **1,762,641 B (84.0%) → 1,428,629 B (68.1%), −334,012 B.** That's ~14 KB less than the asset total, because of the new code (the start screen drawing, `IconProvider` changes, the icon size fallback). With `INCLUDE_MEME_ASSETS=1` the firmware is 1,681,321 B (80.2%).

**Code that still mentions LPE** (all behind `INCLUDE_MEME_ASSETS` now; for the later full removal):

- `src/icon_provider.h`: LPE branches for the running background, long-break images, confirmation icon and preset icons
- `src/splashscreen.h`: the "LPE Mode" checkbox
- `src/strings.cpp` / `src/strings.h`: `isLpeModeEnabled()` (constant `false` when the switch is off), `lpe*Messages` vectors, `if (lpeModeEnabled)` branches
- `src/i18n/{en,af,de}.h`: `MODE = "LPE Mode"`, `RESTARTTIMERMSGLPE`
- `src/main.cpp`: `setLpeMode(...)` and the `IMAGE_CYCLE_TEST` debug test
- `assets/manifest.json`: the `"meme": true` entries, plus the source art in `assets/icons/lpe*.png` and `assets/images/bg_*.png`

### Expected flash result

With the switch off, the meme backgrounds and the splash replaced by code, and per-size icons: roughly **288 KB (backgrounds) + ~37 KB (LPE icons) + ~50 KB (unused icon sizes)**, so somewhere around **350 KB freed**. That would take flash from 83.6% to roughly 66%, leaving plenty of room for the web work, the status screen and the QR code. CI (§6) will confirm the real number.

### Suggested Saturday slice

This is bigger than one day, together with the web work. A realistic first slice:
1. Add `INCLUDE_MEME_ASSETS` (default off): hide the LPE checkbox, route all image choices through `IconProvider`, and swap the two hard-coded LPE icons for Bootstrap ones. That's the biggest flash win for the least code.
   - **Needs stopgaps on the same day,** or a default build has holes: until the code-drawn screens exist, keep `bg_bubble` (neutral) for the running screen, show the **short-break layout** for long breaks (no image), and give the splash a minimal text-only "Pomodoro" + `stopwatch` version. Those are a few lines each, and step 5 replaces them.
2. Switch `gen_assets.py` to read SVGs from `assets/svg/` through a manifest with per-icon sizes; commit the library (minus the sprite file)
3. Add a Workshop icon (fixes the §3 fallback). Bootstrap has `tools` and `hammer`.
4. Delete the dead `src/lpe.h` and `image_bg_lpehacker_bubble.h`
5. Follow-up session: the code-drawn running bubble, long-break and splash screens

---

## 10. MQTT: decided direction, not for Caturday (added 2026-10-10)

The full plan is in `roadmap.MD` → Future → **MQTT publish/subscribe**. In short: the same JSON as `websocket.md`, topics instead of the `type` field, retained status/presets, a last-will availability topic, and status published on change only.

**Decided (2026-10-10): yes, `status` includes `endsAt`** (plus `deviceTime`, to correct for the browser's clock differing from the device's). The web countdown keeps running through a dropped connection, and the next message re-syncs it. Implemented; see `websocket.md` → *Showing the countdown*.

~~One open decision affects today's frontend work:~~ whether `status` gets an **`endsAt`** field (the epoch ms when the interval ends, from SNTP time).

- **With it:** the browser counts down smoothly by itself, the WebSocket can send `status` on changes only, and MQTT later gets it for free.
- **Without it:** the browser shows `remainingMs` from the once-a-second broadcast, as it works today.

It's a small addition to `buildStatusJson()`, but decide before building the countdown in §2, because it changes how the frontend draws the timer.

---

## 11. Splash, selection menu and drawn running screen (added 2026-10-10)

**Done** on `feature/drawn-backgrounds` and `feature/selection-menu`:

- **Running screen:** the person and speech bubble are drawn in code (Bootstrap `person-fill` + `drawSpeechBubble()`) instead of the 48 KB `bg_bubble` bitmap. A default build now has no full-screen bitmaps.
- **Splash:** no menu any more. It shows the logo for `SPLASH_DURATION_MS` (`config.h`, 3 s), then goes to preset selection by itself, so idle blanking works from boot and testing needs no button press after flashing.
- **Preset selection:** a **Status | Settings** row at the top; the knob cycles presets → Status → Settings. Preset icons went from 192 to 128 px to make room.
- **Settings:** its own screen with a **Back** row, since it's now reachable during normal use. Fixed: every redraw used to rewrite every setting to flash.
- Flash: 67.0% → **64.6%**.

**Decided:**

- **Break screen decorations: done (PR #8).** A 128 px cup (`cup-hot-fill`) for short breaks and a moon (`moon-stars-fill`) for long ones, left of the statistics box. The original idea: Breaks are already fully drawn in code (statistics box, header, menu). The idea on the table: a large Bootstrap icon in the free space left of the statistics box, e.g. `cup-hot` for short breaks and `moon-stars` for long ones, ~2 KB each at 128 px.
- ~~**The long press (§7).**~~ **Decided: dropped.** Its purpose was reaching Settings without a power cycle, and Settings is now on the selection screen.
