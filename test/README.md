# Tests

The list of tests for the Pomodoro timer: what each one checks, and **why it exists**. Many rows trace back to a real bug, so the reason column records which one, and the test keeps it from coming back.

The timer tests run; most others are still planned (roadmap §2.1 and §2.2). Update the **Status** column as they land.

| Status | Meaning |
| --- | --- |
| ✅ | In place and passing |
| 🔒 | Covered at build time (the build fails instead) |
| ❌ | Not written yet |
| 👀 | Manual check, not yet done |

**How the tests run:**

- **Host tests** run on the laptop with `pio test -e native` (or `npm run piotests`), and in CI on every PR (`.github/workflows/test.yml`). They build the real firmware sources (the list is `build_src_filter` in the `[env:native]` section of `platformio.ini`) against stand-ins for the hardware in `test/fakes/`:
  - **Time** only moves when a test moves it: `fake::now` is what `millis()` returns.
  - **The display** draws nothing, and measures text as 10 px per character.
  - **Settings** go to an in-memory map instead of flash, and `fake::prefWrites` counts the writes.
  - **The button** is pressed by setting `button.pressed = true`. The LED does nothing, and the Status and Settings screens close straight away.
- Each suite is a folder named `test/test_<name>/`. A firmware source a new suite needs goes into `build_src_filter`; a hardware library it needs gets a stand-in in `test/fakes/`.
- **On-device tests** will drive the ESP32 over the WebSocket (`websocket.md`) from a script on the laptop.

**Adding a test:** add a row with the next free ID in its section. If the test comes from a bug, say so in **Why**, with the date or PR.

---

## Host tests

### Timer state machine (`src/timer.cpp`, `src/states/`)

| ID | What it checks | Why | Status |
| --- | --- | --- | --- |
| TMR-01 | Each state accepts exactly the commands in the `websocket.md` table and rejects the rest, leaving the state unchanged | That table is the contract the web page (and later MQTT) relies on. The encoder and remote commands share the same transition code, so this covers both. | ✅ |
| TMR-02 | Stop returns to `SelectingPreset` with the **same** preset still selected | Bug fixed 2026-10-10: stopping always jumped to preset index 1 | ✅ |
| TMR-03 | Pausing and resuming a break: the break still runs for its full length, with the pause time left out | Bug fixed 2026-10-10: a paused break ended early | ✅ |
| TMR-04 | Pausing and resuming work: the pause time is left out of elapsed | Same timekeeping as TMR-03, for work intervals | ✅ |
| TMR-05 | A long break comes after every `longPauseAfter` cycles, and a short break otherwise (e.g. Coding: long after 2) | The long-break rule is easy to get off by one, and is slow to check by hand | ✅ |
| TMR-06 | Restart sets the interval back to zero but keeps the cycle count | Decided in Caturday planning: restarting shouldn't cost you your long break | ✅ |
| TMR-07 | Remaining and interval time are 0 outside the running states, and remaining never wraps around to a huge number after the end | The values are unsigned, and the status JSON and web countdown read them | ✅ |
| TMR-08 | A `select` command with an out-of-range index is rejected | Remote input is untrusted, and the index goes straight into the preset list | ✅ |
| TMR-09 | The selection cursor steps through the presets, then Status, then Settings, wrapping both ways; landing on a preset selects it | Status and Settings were added to the selection screen on 2026-10-10 (PR #5) | ✅ |
| TMR-10 | A 24-hour preset runs and finishes correctly | The planned battery tests use a 24-hour session; check that the timer copes before measuring power | ✅ |
| TMR-11 | An interval running across the `millis()` wraparound (~49.7 days of uptime) doesn't end early or late | The unsigned subtraction should handle it, but nothing proves it, and a wrong answer only shows after weeks of uptime. **Can't run as a host test:** `unsigned long` is 64 bits on the laptop but 32 on the ESP32, so the laptop never wraps. Needs a 32-bit host build (`-m32`) or the device. | ❌ |

### Messages and presets (`src/i18n/`, `src/messages.cpp`)

| ID | What it checks | Why | Status |
| --- | --- | --- | --- |
| MSG-01 | No message array has an empty slot | Bug fixed 2026-10-10: `CHAT_GPT_FACTS` declared 42 slots but listed 41, so the device could pick the empty one and crash. Now a `static_assert(allPopulated(...))`. | 🔒 |
| MSG-02 | Every preset gets a running message and an icon; none falls through to "Unknown preset" | Bug fixed 2026-10-10 (PR #7): the Workshop preset showed "Unknown preset" | ❌ |
| MSG-03 | Every message, in every language, uses only characters the fonts can draw (7-bit ASCII) | Bug fixed 2026-10-10: "ß" in af/de was outside the font. Fetched facts (roadmap) and real translations make this more likely. | ❌ |
| MSG-04 | Every message fits the speech bubble (at most 4 lines after wrapping) | `STRINGS_TEST` checks this by eye, as a debug build. New facts and translations need an automatic check. | ❌ |

### Text wrapping (`wrapText()` in `src/gfx_utils.cpp`)

| ID | What it checks | Why | Status |
| --- | --- | --- | --- |
| WRP-01 | Lines break at spaces and are never wider than the maximum width | Messages wrap automatically since PR #10, which replaced the hand-placed line breaks | ❌ |
| WRP-02 | A single word wider than the line is handled, with no endless loop or lost text | The awkward case for any wrapping code; a long word in a fact or translation would hit it | ❌ |
| WRP-03 | An empty string, a single word, and extra or trailing spaces | Edge cases that tend to give empty lines or crashes | ❌ |

### Formatting and status (`src/statistics.cpp`, `src/system_status.cpp`)

| ID | What it checks | Why | Status |
| --- | --- | --- | --- |
| FMT-01 | `formatDuration()`: under an hour shows `Ym`, otherwise `Xh Ym` (0, 59, 60, 61 and a multi-day value) | Shown on the Status screen and page; the hour boundary is where it goes wrong | ❌ |
| FMT-02 | `formatBytes()` at the unit boundaries: 1023 B, 1024 B, just under 1 MB, 1 MB, 4 MB | It rounds to the nearest KB, so a value just under 1 MB may show as "1024 KB" rather than "1.0 MB"; pin down which | ❌ |
| FMT-03 | `systemStatusToJson()` includes every field the status page reads (e.g. `totalHeapBytes`, `minFreeHeapBytes`, `hostname`) | The firmware and the web page are built and uploaded separately, so a renamed field breaks the page without a build error | ❌ |
| STA-01 | Lifetime statistics add up correctly in whole minutes, including how part-minutes are rounded | The totals are stored as minutes but recorded in ms; decide whether short intervals are rounded or dropped | ❌ |

### WebSocket protocol (`src/web_server.cpp`)

These need the JSON building and command parsing moved out of `web_server.cpp`. The MQTT plan in the roadmap moves them into a shared file anyway.

| ID | What it checks | Why | Status |
| --- | --- | --- | --- |
| WS-01 | Command parsing: each valid command maps to the right `TimerCommand`; an unknown command, missing field or broken JSON gets an error reply and queues nothing | Network input is untrusted; MQTT will use the same parser | ❌ |
| WS-02 | The `status` message matches `websocket.md`: `endsAt` when the time is synced, `remainingMs` otherwise | Clients (and later MQTT subscribers) count down from it | ❌ |
| WS-03 | `endsAt` stays the same between status messages while the timer runs | Bug fixed 2026-10-10: `endsAt` jittered by about a second because it was built from a stale elapsed value | ❌ |
| WS-04 | The local-dev CORS check allows only `localhost` and `127.0.0.1` origins | It's a security boundary: no other site's pages should be able to control the timer | ❌ |

### Settings (`src/settings_screen.cpp`, `src/checkbox.cpp`)

| ID | What it checks | Why | Status |
| --- | --- | --- | --- |
| SET-01 | Drawing the settings screen writes nothing to flash; only changing a setting does | Bug fixed 2026-10-10: a copied `Checkbox` saved itself when destroyed, so every redraw rewrote every setting, wearing the flash | ❌ |

---

## On-device tests (roadmap §2.2)

Run against the real ESP32 over the WebSocket, from a script on the laptop.

| ID | What it checks | Why | Status |
| --- | --- | --- | --- |
| DEV-01 | After a reboot the device reaches preset selection with no button press, and answers on `/ws` | The splash advances by itself since PR #5, which is what makes automated device tests possible | ❌ |
| DEV-02 | A status request sent while the device is still booting gets an answer, without a crash | Bug fixed 2026-10-10: the network task read the `Timer` during start-up (a race) | ❌ |
| DEV-03 | A full cycle via commands: start → break → start, with the state and cycle count reported at each step | The end-to-end path a web user takes | ❌ |
| DEV-04 | Saving WiFi settings replies "Saved" before the device restarts | Bug fixed 2026-10-10: the restart came before the reply was sent | ❌ |
| DEV-05 | `http://pomodoro.local/` resolves and serves the timer page | mDNS added in PR #14; Android support varies, so check from the laptop | ❌ |

---

## Manual checks (by eye)

Things only a person looking at the e-paper or the browser can confirm. Most come from the "Still to test" list in `caturday.md`.

| ID | What it checks | Why | Status |
| --- | --- | --- | --- |
| MAN-01 | The "time for a break" screen after a work interval actually runs out, on the device and on the web page | Only tested by skipping ahead so far, never with a real interval ending | 👀 |
| MAN-02 | A real dropped WebSocket connection: the web countdown keeps running, then resyncs on reconnect | The reason `endsAt` exists; so far tested only by simulation | 👀 |
| MAN-03 | Booting with WiFi unavailable (wrong SSID): the Status screen says not connected, with no hang | The timer must work without a network | 👀 |
| MAN-04 | Status while WiFi connects after boot: "Connecting..." changes to the IP without a button press | The screen should update by itself | 👀 |
| MAN-05 | MQTT shows as connected | Needs a broker; worth doing with the MQTT work | 👀 |
| MAN-06 | After a new commit, only `version.cpp` recompiles | The firmware-version script must not trigger full rebuilds | 👀 |
| MAN-07 | Menus and messages fit on screen in af and de | Both are English placeholders for now; check once they're translated | 👀 |
| MAN-08 | Settings → Shut down: the web page says "switched off", the screen clears to white, the LED goes off; a knob press switches the device back on, and the web page reconnects by itself | Added 2026-10-10. Deep sleep and waking can only be checked on the device. Passed 2026-10-10, twice, with the web page watched in a browser | ✅ |

---

## Power measurements

For the planned LiPo battery (roadmap: Battery power). These are readings from a USB power meter or an INA219, not pass/fail code tests. Record the result in the **Result** column.

| ID | What to measure | Why | Result |
| --- | --- | --- | --- |
| PWR-01 | Average current on USB, timer running, WiFi connected | The baseline, before any battery hardware goes in | — |
| PWR-02 | On battery with WiFi connected | Expected to be the main drain (roughly 80–120 mA) | — |
| PWR-03 | On battery with WiFi off or not configured | Shows how much turning WiFi off would buy | — |
| PWR-04 | A 24-hour preset on a full battery: does it finish, and how much charge is left? | The target use case; needs TMR-10 passing first | — |
| PWR-05 | Current while the display is blanked (idle and paused) | Blanking exists to save power; this shows how much it saves | — |

---

## Build checks (already in place)

| ID | What it checks | Why | Status |
| --- | --- | --- | --- |
| BLD-01 | Flash use: CI warns at 85% and fails at 95% (`scripts/check_size.py`) | Flash was at 83.6% before Caturday; one stream include alone cost 200 KB | ✅ |
| BLD-02 | Source formatting, by `clang-format` in the pre-commit hook | Keeps diffs clean. Local only for now; `lint.yml` (roadmap §4.2) would repeat it in CI. | ✅ |
| BLD-03 | The host tests pass, in CI on every PR (`.github/workflows/test.yml`) | Tests only protect what they're run against | ✅ |
