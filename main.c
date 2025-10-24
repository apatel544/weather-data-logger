#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include "cJSON.h"
#include <unistd.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

int main(void) {
    char *api_key = "7addccf61f8d4dd5b76225325250202"; // API KEY
    char *location = "Los_Angeles"; // Location API will pull data for
    char *response = NULL;
    float temp;
    int humidity;

    for (int i = 0; i < 3; i++) { // Run 3 times for testing
        if (fetch_weather_data(api_key, location, &response) == 0) {
            if (parse_weather_data(response, &temp, &humidity) == 0) {
                if (log_weather_data(temp, humidity) == 0) {
                    printf("Logged: Temp=%.1fC, Humidity=%d%%\n", temp, humidity);
                } else {
                    fprintf(stderr, "Logging failed\n");
                }
            } else {
                fprintf(stderr, "Parsing failed\n");
            }
            free(response);
        } else {
            fprintf(stderr, "Fetch failed\n");
        }
        sleep(2); // Wait 2 seconds
    }

    return 0;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);

    if (!ptr) {
        fprintf(stderr, "realloc() failed\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int fetch_weather_data(char *api_key, char *location, char **response) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk = {0};

    chunk.memory = malloc(1);
    if (!chunk.memory) {
        fprintf(stderr, "malloc() failed\n");
        return 1;
    }
    chunk.memory[0] = 0;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl_easy_init() failed\n");
        free(chunk.memory);
        return 1;
    }

    char url[256];
    snprintf(url, sizeof(url), "http://api.weatherapi.com/v1/current.json?key=%s&q=%s", api_key, location);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        curl_easy_cleanup(curl);
        return 1;
    }

    *response = chunk.memory;
    curl_easy_cleanup(curl);
    return 0;
}

int parse_weather_data(char *json_string, float *temp, int *humidity) {
    cJSON *root = cJSON_Parse(json_string);
    if (!root) {
        fprintf(stderr, "JSON parse failed\n");
        return 1;
    }

    cJSON *current = cJSON_GetObjectItem(root, "current");
    if (!current) {
        fprintf(stderr, "No 'current' object in JSON\n");
        cJSON_Delete(root);
        return 1;
    }

    cJSON *temp_item = cJSON_GetObjectItem(current, "temp_c");
    cJSON *humidity_item = cJSON_GetObjectItem(current, "humidity");
    if (!temp_item || !humidity_item) {
        fprintf(stderr, "Missing temp_c or humidity\n");
        cJSON_Delete(root);
        return 1;
    }

    *temp = (float)cJSON_GetNumberValue(temp_item);
    *humidity = (int)cJSON_GetNumberValue(humidity_item);

    cJSON_Delete(root);
    return 0;
}

int log_weather_data(float temp, int humidity) {
    FILE *fp = fopen("logs/weather_log.csv", "a");
    if (!fp) {
        fprintf(stderr, "Failed to open log file\n");
        return 1;
    }

    time_t now;
    time(&now);
    char timestamp[26];
    ctime_r(&now, timestamp);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline

    fprintf(fp, "%s,%.1f,%d\n", timestamp, temp, humidity);
    fflush(fp);
    fclose(fp);
    return 0;
}
