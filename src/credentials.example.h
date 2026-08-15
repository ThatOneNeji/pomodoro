/**
 * @file credentials.example.h
 * @brief Template for credentials.h — copy this file to credentials.h and fill in your
 * network details. credentials.h is gitignored so real credentials never get committed.
 *
 * These are only the compiled-in defaults used until a user overrides them at runtime; see
 * wifi_manager.h.
 */
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#define WIFI_SSID_DEFAULT "YourNetworkName"          ///< Compiled-in default SSID.
#define WIFI_PASSWORD_DEFAULT "YourNetworkPassword"  ///< Compiled-in default password.

#define MQTT_USERNAME ""  ///< Leave empty if the broker allows anonymous connections.
#define MQTT_PASSWORD ""  ///< Leave empty if the broker allows anonymous connections.

#endif
