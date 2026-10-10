/**
 * @file web_server.h
 * @brief Optional WiFi webserver: connects in the background and, once connected, serves a
 * WebSocket status feed, WiFi settings endpoints, and static files (e.g. data/index.html) from
 * LittleFS. Never blocks startup — the device works the same fully offline.
 *
 * Routes:
 *   - WS    /ws: two-way. Server → client messages all carry a `type`:
 *       - `presets`: `{presets: [{name, durationMs, pauseMs, longPauseMs, longPauseAfter}]}`, on connect.
 *       - `status`: `{state, presetIndex, presetName, remainingMs, durationMs, isLongBreak, cycles}`,
 *         on connect, immediately on any state/preset change, and otherwise once a second.
 *       - `error`: `{error}`, in reply to a malformed or unknown command.
 *     Client → server: `{"cmd": "select", "preset": <index>}`, or `{"cmd": "<name>"}` for
 *     start/pause/resume/restart/stop/break (see TimerCommand). Commands are queued and applied
 *     from loop(); a command that isn't valid in the current state is logged and ignored.
 *   - GET  /wifi: the currently configured SSID (never the password).
 *   - POST /wifi: save new WiFi credentials ({"ssid": "...", "password": "..."}, password
 *     optional to keep the existing one) and restart to apply them.
 *   - GET  /system: read-only device status (network, time, MQTT, hardware, firmware, storage,
 *     statistics); see system_status.h. Nothing here is editable.
 *   - static files from data/ (e.g. /, /settings.html) once LittleFS is mounted.
 */
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

enum class TimerCommand;  // timer.h; forward-declared so this header doesn't pull in the display stack.

/**
 * @brief Mount LittleFS, start connecting to WiFi (see wifi_manager.h), and register the
 * webserver's routes.
 *
 * The LittleFS mount happens immediately (it's local, no network needed) and is logged either
 * way; if it fails (e.g. `pio run -t uploadfs` was never run), static file serving is simply
 * skipped rather than blocking anything. The WiFi connection itself is non-blocking: this fires
 * WiFi.begin() and returns immediately. The actual webserver only starts once a WiFi.onEvent()
 * callback confirms an IP address was obtained, so a misconfigured or unreachable network never
 * delays setup() or blocks normal (offline) use of the device.
 */
void setupWebServer();

/**
 * @brief Broadcast the current status to all connected /ws clients (throttled to once a second)
 * and prune stale WebSocket clients.
 *
 * Call this once per main loop() iteration; it's a cheap no-op whenever the webserver hasn't
 * started yet, no clients are connected, or a second hasn't passed since the last broadcast.
 * A newly connecting client is sent its first status immediately (see the WS_EVT_CONNECT
 * handler in web_server.cpp), so it doesn't have to wait out the throttle to see current state.
 */
void webServerLoop();

/**
 * @brief Take the next queued /ws control command, if any.
 *
 * Call from loop() and apply each one with Timer::handleCommand(), before Timer::loop() so any
 * redraw it requests happens in the same iteration.
 * @param[out] command The command.
 * @param[out] arg Its argument (preset index for TimerCommand::Select, otherwise -1).
 * @return true if a command was dequeued, false if the queue is empty.
 */
bool webServerNextCommand(TimerCommand &command, int &arg);

#endif
