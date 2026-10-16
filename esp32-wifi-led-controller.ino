/*
В коде установлен фиксированный IP-адрес для устройства: 192.168.1.129
адрес должен быть свободен и зарезервирован за ESP32 в роутере либо корректно исключён из DHCP-пула
фиксированное имя сети: AAA-56GH17
и пароль доступа к сети Wi-Fi.

Работа в браузере по адресу http://192.168.1.129

*/
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_err.h"

// ==================== WI-FI ====================

const char* WIFI_SSID     = "AAA-56GH17";
const char* WIFI_PASSWORD = "ВСТАВЬ_СВОЙ_ПАРОЛЬ";

// Статические сетевые настройки
IPAddress localIP(192, 168, 1, 129);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 1);
IPAddress secondaryDNS(8, 8, 8, 8);

// ==================== WS2812B ====================

#define LED_PIN   27
#define LED_COUNT 3

Adafruit_NeoPixel strip(
  LED_COUNT,
  LED_PIN,
  NEO_GRB + NEO_KHZ800
);

const uint32_t FADE_IN_MS  = 3000;
const uint32_t COLORS_MS   = 9000;
const uint32_t FADE_OUT_MS = 3000;
const uint32_t CYCLE_MS    = FADE_IN_MS + COLORS_MS + FADE_OUT_MS;

// ==================== STATE ====================

volatile bool ledEnabled = true;

bool wifiConnected = false;
bool serverStarted = false;

httpd_handle_t server = nullptr;

// ==================== LED COLORS ====================

const uint32_t rainbowColors[] = {
  0xFF0000, // Red
  0xFF7F00, // Orange
  0xFFFF00, // Yellow
  0x00FF00, // Green
  0x00FFFF, // Cyan
  0x0000FF, // Blue
  0x8B00FF  // Violet
};

const uint8_t COLOR_COUNT =
  sizeof(rainbowColors) / sizeof(rainbowColors[0]);

// ==================== LED CONTROL ====================

void ledsOff() {
  strip.clear();
  strip.show();
}

void updateRainbow() {
  if (!ledEnabled || !wifiConnected) {
    ledsOff();
    return;
  }

  uint32_t now = millis();
  uint32_t t = now % CYCLE_MS;

  uint8_t brightness = 255;

  if (t < FADE_IN_MS) {
    brightness = (uint32_t)t * 255 / FADE_IN_MS;
  } else if (t >= FADE_IN_MS + COLORS_MS) {
    uint32_t fadeTime = t - FADE_IN_MS - COLORS_MS;
    brightness = 255 - (uint32_t)fadeTime * 255 / FADE_OUT_MS;
  }

  uint32_t colorTime = 0;

  if (t >= FADE_IN_MS) {
    colorTime = t - FADE_IN_MS;
    if (colorTime > COLORS_MS) {
      colorTime = COLORS_MS;
    }
  }

  uint32_t scaledTime = colorTime * COLOR_COUNT;
  uint8_t colorIndex = scaledTime / COLORS_MS;
  uint32_t remainder = scaledTime % COLORS_MS;

  if (colorIndex >= COLOR_COUNT) {
    colorIndex = COLOR_COUNT - 1;
    remainder = COLORS_MS;
  }

  uint32_t c1 = rainbowColors[colorIndex];
  uint32_t c2 = rainbowColors[(colorIndex + 1) % COLOR_COUNT];

  uint8_t r1 = (c1 >> 16) & 0xFF;
  uint8_t g1 = (c1 >> 8) & 0xFF;
  uint8_t b1 = c1 & 0xFF;

  uint8_t r2 = (c2 >> 16) & 0xFF;
  uint8_t g2 = (c2 >> 8) & 0xFF;
  uint8_t b2 = c2 & 0xFF;

  uint8_t r = r1 + ((int16_t)r2 - r1) * remainder / COLORS_MS;
  uint8_t g = g1 + ((int16_t)g2 - g1) * remainder / COLORS_MS;
  uint8_t b = b1 + ((int16_t)b2 - b1) * remainder / COLORS_MS;

  r = (uint16_t)r * brightness / 255;
  g = (uint16_t)g * brightness / 255;
  b = (uint16_t)b * brightness / 255;

  uint32_t color = strip.Color(r, g, b);

  for (uint8_t i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }

  strip.show();
}

// ==================== WEB PAGE ====================

static const char INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 LED</title>
<style>
body {
  font-family: Arial, sans-serif;
  max-width: 400px;
  margin: 40px auto;
  padding: 0 20px;
  text-align: center;
  background: #151515;
  color: #f0f0f0;
}
h1 { font-size: 25px; }
button {
  width: 100%;
  padding: 18px;
  margin: 10px 0;
  font-size: 18px;
  border: 0;
  border-radius: 10px;
  cursor: pointer;
}
.on { background: #21a366; color: white; }
.off { background: #d94b4b; color: white; }
#status { margin: 22px 0; font-size: 18px; }
</style>
</head>
<body>
<h1>Управление подсветкой</h1>
<div id="status">Проверка состояния...</div>
<button class="on" onclick="setLed(true)">Включить</button>
<button class="off" onclick="setLed(false)">Выключить</button>

<script>
async function setLed(state) {
  try {
    await fetch(state ? '/on' : '/off', {cache: 'no-store'});
    await refresh();
  } catch (e) {
    document.getElementById('status').textContent = 'Нет связи с устройством';
  }
}

async function refresh() {
  try {
    const response = await fetch('/state', {cache: 'no-store'});
    const data = await response.json();
    document.getElementById('status').textContent =
      data.on ? 'Подсветка включена' : 'Подсветка выключена';
  } catch (e) {
    document.getElementById('status').textContent = 'Нет связи с устройством';
  }
}

refresh();
setInterval(refresh, 3000);
</script>
</body>
</html>
)rawliteral";

// ==================== HTTP HANDLERS ====================

esp_err_t rootHandler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html; charset=utf-8");
  return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

esp_err_t onHandler(httpd_req_t *req) {
  ledEnabled = true;
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_sendstr(req, "ON");
}

esp_err_t offHandler(httpd_req_t *req) {
  ledEnabled = false;
  ledsOff();
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_sendstr(req, "OFF");
}

esp_err_t stateHandler(httpd_req_t *req) {
  char response[24];

  snprintf(
    response,
    sizeof(response),
    "{\"on\":%s}",
    ledEnabled ? "true" : "false"
  );

  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");

  return httpd_resp_sendstr(req, response);
}

// ==================== WEB SERVER ====================

void startWebServer() {
  if (serverStarted) {
    return;
  }

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 8;

  if (httpd_start(&server, &config) != ESP_OK) {
    server = nullptr;
    return;
  }

  httpd_uri_t rootUri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = rootHandler,
    .user_ctx = nullptr
  };

  httpd_uri_t onUri = {
    .uri = "/on",
    .method = HTTP_GET,
    .handler = onHandler,
    .user_ctx = nullptr
  };

  httpd_uri_t offUri = {
    .uri = "/off",
    .method = HTTP_GET,
    .handler = offHandler,
    .user_ctx = nullptr
  };

  httpd_uri_t stateUri = {
    .uri = "/state",
    .method = HTTP_GET,
    .handler = stateHandler,
    .user_ctx = nullptr
  };

  httpd_register_uri_handler(server, &rootUri);
  httpd_register_uri_handler(server, &onUri);
  httpd_register_uri_handler(server, &offUri);
  httpd_register_uri_handler(server, &stateUri);

  serverStarted = true;
}

void stopWebServer() {
  if (server != nullptr) {
    httpd_stop(server);
    server = nullptr;
  }

  serverStarted = false;
}

// ==================== WI-FI EVENTS ====================

void wifiEventHandler(
  void* arg,
  esp_event_base_t eventBase,
  int32_t eventId,
  void* eventData
) {
  if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
    wifiConnected = false;
    stopWebServer();
    ledsOff();

    esp_wifi_connect();
  }

  if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
    wifiConnected = true;
    startWebServer();
  }
}

// ==================== SETUP ====================

void setup() {
  strip.begin();
  strip.setBrightness(255);
  ledsOff();

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();

  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    nvs_flash_init();
  }

  // Initialize network stack
  esp_netif_init();
  esp_event_loop_create_default();

  esp_netif_t* netif = esp_netif_create_default_wifi_sta();

  // Configure static IP before connecting
  esp_netif_dhcpc_stop(netif);

  esp_netif_ip_info_t ipInfo;
  IP4_ADDR(&ipInfo.ip,      192, 168, 1, 129);
  IP4_ADDR(&ipInfo.gw,      192, 168, 1, 1);
  IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);

  esp_netif_set_ip_info(netif, &ipInfo);

  // Initialize Wi-Fi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  esp_event_handler_register(
    WIFI_EVENT,
    ESP_EVENT_ANY_ID,
    &wifiEventHandler,
    nullptr
  );

  esp_event_handler_register(
    IP_EVENT,
    IP_EVENT_STA_GOT_IP,
    &wifiEventHandler,
    nullptr
  );

  wifi_config_t wifiConfig = {};
  strncpy(
    (char*)wifiConfig.sta.ssid,
    WIFI_SSID,
    sizeof(wifiConfig.sta.ssid)
  );
  strncpy(
    (char*)wifiConfig.sta.password,
    WIFI_PASSWORD,
    sizeof(wifiConfig.sta.password)
  );

  wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);

  // Maximum configured Wi-Fi transmit power: 20 dBm
  esp_wifi_set_max_tx_power(80);

  // Disable Wi-Fi power saving
  esp_wifi_set_ps(WIFI_PS_NONE);

  esp_wifi_start();
  esp_wifi_connect();
}

// ==================== MAIN LOOP ====================

void loop() {
  updateRainbow();
  delay(10);
}
