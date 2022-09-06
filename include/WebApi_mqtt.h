// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <ESPAsyncWebServer.h>

#define MQTT_JSON_DOC_SIZE 3072

class WebApiMqttClass {
public:
    void init(AsyncWebServer* server);
    void loop();

private:
    void onMqttStatus(AsyncWebServerRequest* request);
    void onMqttAdminGet(AsyncWebServerRequest* request);
    void onMqttAdminPost(AsyncWebServerRequest* request);
    String getRootCaCertInfo(const char* cert);

    AsyncWebServer* _server;
};