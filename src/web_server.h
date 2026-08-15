/**
 * @file web_server.h
 * @brief Optional WiFi status webserver: connects in the background and serves a JSON status
 * endpoint once connected. Never blocks startup — the device works the same fully offline.
 */
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

/**
 * @brief Start connecting to WiFi (see wifi_manager.h) and register the webserver's routes.
 *
 * Non-blocking: this fires WiFi.begin() and returns immediately. The actual webserver only
 * starts once a WiFi.onEvent() callback confirms an IP address was obtained, so a misconfigured
 * or unreachable network never delays setup() or blocks normal (offline) use of the device.
 */
void setupWebServer();

#endif
