#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "http server";

// Handler for GET /
static esp_err_t root_get_handler(httpd_req_t *req) {
    const char *response = "ESP32 HTTP Server Running!";
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

// Handler for GET /health
static esp_err_t health_get_handler(httpd_req_t *req) {
    char response[128];
    snprintf(response, sizeof(response),
             "{ \"uptime\": %lu, \"free_heap\": %lu }",
             esp_log_timestamp() / 1000,
             esp_get_free_heap_size());

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

static void register_routes(httpd_handle_t server) {
    httpd_uri_t root_uri = {.uri = "/",
                            .method = HTTP_GET,
                            .handler = root_get_handler,
                            .user_ctx = NULL};

    httpd_uri_t health_uri = {.uri = "/health",
                              .method = HTTP_GET,
                              .handler = health_get_handler,
                              .user_ctx = NULL};

    httpd_register_uri_handler(server, &root_uri);
    httpd_register_uri_handler(server, &health_uri);
}

httpd_handle_t start_http_server() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "HTTP server started on port %d", config.server_port);
        register_routes(server);
    } else {
        ESP_LOGE(TAG, "Failed to start HTTP server");
    }

    return server;
}

