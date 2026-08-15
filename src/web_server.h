/**
 * @file web_server.h
 * @brief Optional WiFi webserver: connects in the background and, once connected, serves a JSON
 * status endpoint, WiFi settings endpoints, and static files (e.g. data/index.html) from
 * LittleFS. Never blocks startup — the device works the same fully offline.
 *
 * Routes:
 *   - GET  /status: current TimerState, active preset name, milliseconds remaining.
 *   - GET  /wifi: the currently configured SSID (never the password).
 *   - POST /wifi: save new WiFi credentials ({"ssid": "...", "password": "..."}, password
 *     optional to keep the existing one) and restart to apply them.
 *   - static files from data/ (e.g. /, /settings.html) once LittleFS is mounted.
 */
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

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

#endif
