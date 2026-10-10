# WebSocket API

The device exposes a two-way WebSocket at:

```text
ws://<device-ip>/ws
```

All messages are JSON text frames. The device sends **status updates** to every connected client, and clients send **commands** to control the timer. Both the knob and the WebSocket drive the same state machine, so either can be used at any time.

Implementation: `src/web_server.cpp` (parsing, queue, broadcasts) and `Timer::handleCommand()` in `src/timer.cpp` (what each command does).

---

## Device → client

Every message has a `type` field.

### `presets`

Sent once when a client connects. It lists the presets configured in `PRESETS[]` (`src/config.h`), in order. A preset's position in this list is the index used by the `select` command.

```json
{
  "type": "presets",
  "presets": [
    { "name": "Coding",   "durationMs": 900000,  "pauseMs": 300000, "longPauseMs": 600000,  "longPauseAfter": 2 },
    { "name": "Workshop", "durationMs": 1800000, "pauseMs": 600000, "longPauseMs": 1200000, "longPauseAfter": 4 },
    { "name": "Focus",    "durationMs": 480000,  "pauseMs": 300000, "longPauseMs": 360000,  "longPauseAfter": 4 }
  ]
}
```

| Field | Type | Meaning |
| --- | --- | --- |
| `name` | string | Preset name |
| `durationMs` | number | Work interval length, in ms |
| `pauseMs` | number | Short break length, in ms |
| `longPauseMs` | number | Long break length, in ms |
| `longPauseAfter` | number | Completed work cycles between long breaks |

### `status`

Sent:

- when a client connects (right after `presets`)
- immediately whenever the state or selected preset changes, whether from the knob or a command
- otherwise once a second, for the countdown

```json
{
  "type": "status",
  "state": "Running",
  "presetIndex": 0,
  "presetName": "Coding",
  "remainingMs": 895565,
  "durationMs": 900000,
  "isLongBreak": false,
  "cycles": 0,
  "deviceTime": 1791624325120,
  "endsAt": 1791625220685
}
```

| Field | Type | Meaning |
| --- | --- | --- |
| `state` | string | Current timer state, see [States](#states) |
| `presetIndex` | number | Index of the selected preset in the `presets` list |
| `presetName` | string \| null | Name of the selected preset |
| `remainingMs` | number | Time left in the current work or break interval. `0` in any other state. |
| `durationMs` | number | Full length of the current work or break interval. `0` in any other state. Progress = `1 - remainingMs / durationMs`. |
| `isLongBreak` | boolean | Whether the current (or upcoming) break is a long break |
| `cycles` | number | Work cycles completed since power-on. Resets on reboot. |
| `deviceTime` | number \| null | The device's clock when it sent this message, as Unix epoch ms (UTC). `null` until the device has synced its time over SNTP. |
| `endsAt` | number \| null | When the current interval will end, as Unix epoch ms (UTC). `null` unless an interval is actively counting down (`Running` or `RunningBreak`) **and** the clock is synced. |

#### Showing the countdown

Use `endsAt` so the countdown keeps running on its own between messages, and through a dropped connection. The next message after a reconnect simply corrects it.

```js
// On every status message:
clockOffset = msg.deviceTime - Date.now();   // device clock minus browser clock

// Every frame / tick:
const remaining = msg.endsAt !== null
    ? Math.max(0, msg.endsAt - (Date.now() + clockOffset))
    : msg.remainingMs;                          // paused, idle, or clock not synced yet
```

`clockOffset` corrects for the browser's clock differing from the device's. Network latency adds a few ms of error to it, which is invisible at one-second resolution. When `endsAt` is `null`, show `remainingMs` as-is: the timer is paused or idle, or the device hasn't synced its time yet. In that last case, the once-a-second `status` messages still keep the display current.

### `error`

Sent **only to the client that sent the bad message**, when it can't be parsed or queued.

```json
{ "type": "error", "error": "unknown cmd" }
```

| `error` | Cause |
| --- | --- |
| `invalid JSON` | The message isn't valid JSON |
| `unknown cmd` | `cmd` is missing or not one of the [commands](#client--device) |
| `select needs a preset index` | `select` was sent without a `preset` field |
| `not ready` | The device is still showing its boot splash screen (the first few seconds after starting) and isn't accepting commands yet |
| `busy` | The command queue (8 entries) is full |
| `unsupported frame` | Binary, fragmented or multi-frame message |

### `shutdown`

Sent **to every client** when the device is about to switch off (Settings → Shut down on the device). No other fields.

```json
{ "type": "shutdown" }
```

After it, the device clears its screen, closes every connection and goes into deep sleep. Pressing the knob switches it back on, which restarts it from the beginning, so a client should treat the next connection like a fresh one: `presets`, then `status`. The web page shows "switched off" and keeps trying to reconnect.

---

## Client → device

```json
{ "cmd": "select", "preset": 2 }
{ "cmd": "start" }
```

| `cmd` | Extra fields | What it does |
| --- | --- | --- |
| `select` | `preset`: index | Selects a preset without starting it |
| `start` | | Starts the selected preset. After a finished break, starts the next work interval. |
| `pause` | | Pauses a running work interval or break |
| `resume` | | Resumes a paused work interval or break |
| `restart` | | Restarts the current work interval from zero. Keeps the cycle count; time already worked is still added to the saved stats. After a finished break, same as `start`. |
| `stop` | | Cancels the current interval and returns to preset selection |
| `break` | | Starts a break now, or confirms the break once a work interval has finished |

### Which commands work in which state

| State | Valid commands |
| --- | --- |
| `SelectingPreset` | `select`, `start` |
| `Running` | `pause`, `restart`, `break`, `stop` |
| `UserInitiatedPause` | `resume`, `restart`, `break`, `stop` |
| `WaitingConfirmStartOfBreak` | `break` |
| `RunningBreak` | `pause`, `stop` |
| `UserInitiatedBreakPause` | `resume`, `stop` |
| `WaitingConfirmEndOfBreak` | `start` / `restart` (next work interval), `stop` (back to presets) |

A command that isn't valid in the current state is **ignored, not answered with an error**. The check happens later, in the main loop, so the device just logs a warning on the serial console. The client can tell from the next `status` message, because the state won't have changed.

### Timing

Commands are queued and applied by the main loop. They usually take effect within **1–2.5 s**, because an e-paper redraw blocks the loop while it runs. If the display was blanked, the command wakes it first, which takes a few seconds more. While the device shows its Status or Settings screen, commands wait in the queue and are applied once you leave it.

---

## States

| `state` | Meaning |
| --- | --- |
| `SelectingPreset` | Idle, choosing a preset |
| `Running` | Work interval counting down |
| `UserInitiatedPause` | Work interval paused |
| `WaitingConfirmStartOfBreak` | Work interval finished; waiting for the break to be confirmed |
| `RunningBreak` | Break counting down |
| `UserInitiatedBreakPause` | Break paused |
| `WaitingConfirmEndOfBreak` | Break finished; waiting to start work again or go back to presets |

`Stopped` also exists in the code, but it isn't used: stopping returns to `SelectingPreset`.

---

## MQTT (planned)

> **Not implemented yet.** The MQTT client connects to the broker but doesn't publish or subscribe to anything. See `roadmap.MD` → Future → *MQTT publish/subscribe*.

The plan is to reuse the messages in this document unchanged, with the **topic** taking the place of the `type` field:

| Topic | Direction | Retained | Payload |
| --- | --- | --- | --- |
| `pomodoro/<id>/status` | device → broker | yes | [`status`](#status) |
| `pomodoro/<id>/presets` | device → broker | yes | [`presets`](#presets) |
| `pomodoro/<id>/availability` | device → broker | yes | `online` / `offline` (last will) |
| `pomodoro/<id>/cmd` | broker → device | no | [command](#client--device) |
| `pomodoro/<id>/error` | device → broker | no | [`error`](#error) |

Planned differences from the WebSocket:

- **`status` is published on change only,** not once a second, with an `endsAt` timestamp so subscribers can count down themselves.
- **Errors go to the `error` topic,** since MQTT has no way to reply to only the sender.
- **`<id>` is a per-device ID,** so several units can share one broker.

---

## Trying it from the browser console

On the device's web page, the page's own connection is available as `websocket`:

```js
websocket.send(JSON.stringify({ cmd: "select", preset: 2 }));
websocket.send(JSON.stringify({ cmd: "start" }));
```

Or from any page, with a new connection:

```js
const ws = new WebSocket("ws://<device-ip>/ws");
ws.onmessage = (e) => console.log(JSON.parse(e.data));
ws.onopen = () => ws.send(JSON.stringify({ cmd: "pause" }));
```
