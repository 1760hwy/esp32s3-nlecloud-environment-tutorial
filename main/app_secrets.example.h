#pragma once

/* Copy to app_secrets.h. Fill locally. Never commit app_secrets.h or binaries.
 * Double quotes and backslashes inside C strings must be escaped.
 * This example preserves the original plain MQTT / ordinary authentication.
 */
#define APP_WIFI_SSID "YOUR_2_4_GHZ_WIFI_SSID"
#define APP_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define APP_MQTT_URI "mqtt://mqtt.nlecloud.com:1883"
#define APP_MQTT_DEVICE_ID "YOUR_NUMERIC_DEVICE_ID"
#define APP_MQTT_CLIENT_ID "YOUR_DEVICE_TAG"
#define APP_MQTT_SECRET_KEY "YOUR_DEVICE_SECRET_KEY"
