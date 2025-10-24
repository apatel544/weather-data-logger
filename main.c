#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <unistd.h>
#include "cJSON.h"

// Constants to avoid magic numbers/strings (easier to change in one place)
#define API_URL_BASE "http://api.weatherapi.com/v1/current.json?key=%s&q=%s"
#define URL_BUFFER_SIZE 256 // Size for URL string buffer
#define API_TIMEOUT_SECONDS 10 // Timeout for API call (in seconds, to avoid hanging on slow networks)
// #define LOG_FILE_PATH "logs/weather_log.csv" // Path to log file
#define LOG_FILE_PATH "/home/ankurpatel/Desktop/projects/WeatherLogger/logs/weather_log.csv"
#define LOG_INTERVAL_SECONDS 2 // Desired interval between logs (in seconds)
#define TEST_LOOP_COUNT 15 // Number of test loops
#define TIMESTAMP_BUFFER_SIZE 26 // Size for timestamp string
#define JSON_CURRENT_KEY "current" // JSON key for current weather data
#define JSON_TEMP_KEY "temp_f" // JSON key for temperature
#define JSON_HUMIDITY_KEY "humidity" // JSON key for humidity

// Structure to hold response data dynamically (like a resizable notebook for JSON text)
struct MemoryStruct {
    char *memory; // Pointer to the data buffer (the notebook's pages)
    size_t size; // Number of bytes in the buffer (how many pages are filled)
};

<<<<<<< HEAD
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
=======
// Callback function called by libcurl for each chunk of data received
static size_t WriteCallback(void *contents, size_t size, size_t numElmBytes, void *userp) {
    size_t realsize = size * numElmBytes; // Calculate total bytes in this chunk (size per piece * number of pieces)
    struct MemoryStruct *mem = (struct MemoryStruct *)userp; // Cast userp to our MemoryStruct pointer
    char *ptr = realloc(mem->memory, mem->size + realsize + 1); // Resize the buffer to fit old data + new chunk + 1 byte for null terminator
>>>>>>> 53d4253debf638e069688e29afac1ecc96df206d

    if (ptr == NULL) { // Check if realloc failed (e.g., out of memory)
        fprintf(stderr, "realloc() failed\n"); // Print error to terminal
        return 0; // Tell libcurl to stop the transfer
    }

    mem->memory = ptr; // Update the buffer pointer (realloc might move it to a new location)
    memcpy(&(mem->memory[mem->size]), contents, realsize); // Copy the new chunk to the end of the buffer
    mem->size += realsize; // Update the total size of the buffer
    mem->memory[mem->size] = 0; // Add null terminator to make it a valid string

    return realsize; // Tell libcurl we handled all bytes in the chunk
}

// Function to fetch weather data from API
int fetch_weather_data(char *api_key, char *location, char **response) {
    CURL *curl; // Handle for libcurl session (like a messenger)
    CURLcode res; // Result code from libcurl operations
    struct MemoryStruct chunk = {0}; // Local notebook to store response data

    chunk.memory = malloc(1); // Start with a 1-byte buffer (initial empty notebook)
    if (chunk.memory == NULL) { // Check if malloc failed
        fprintf(stderr, "malloc() failed\n"); // Print error
        return 1; // Return failure
    }
    chunk.memory[0] = 0; // Set to empty string

    curl = curl_easy_init(); // Initialize libcurl session
    if (curl == NULL) { // Check if init failed
        fprintf(stderr, "curl_easy_init() failed\n"); // Print error
        free(chunk.memory); // Clean up buffer
        return 1; // Return failure
    }

    char url[URL_BUFFER_SIZE]; // Buffer for API URL
    snprintf(url, sizeof(url), API_URL_BASE, api_key, location); // Build URL safely with key and location

    curl_easy_setopt(curl, CURLOPT_URL, url); // Set API URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Use our callback for data
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk); // Pass notebook to callback
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, API_TIMEOUT_SECONDS); // Set timeout to avoid hanging

    res = curl_easy_perform(curl); // Perform the request (send messenger)
    if (res != CURLE_OK) { // Check if request failed
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res)); // Print error
        free(chunk.memory); // Clean up buffer
        curl_easy_cleanup(curl); // Clean up session
        return 1; // Return failure
    }

    *response = chunk.memory; // Give the buffer's address to the caller
    curl_easy_cleanup(curl); // Clean up session
    return 0; // Return success
}

// Function to parse JSON for temperature and humidity
int parse_weather_data(char *json_string, float *temp, int *humidity) {
    cJSON *root = cJSON_Parse(json_string); // Parse JSON string into a tree
    if (root == NULL) { // Check if parsing failed
        fprintf(stderr, "JSON parse failed\n"); // Print error
        return 1; // Return failure
    }

    cJSON *current = cJSON_GetObjectItem(root, JSON_CURRENT_KEY); // Find "current" section
    if (current == NULL) { // Check if missing
        fprintf(stderr, "No 'current' object in JSON\n"); // Print error
        cJSON_Delete(root); 
        return 1; // Return failure
    }

    cJSON *temp_item = cJSON_GetObjectItem(current, JSON_TEMP_KEY); // Find "temp_c"
    cJSON *humidity_item = cJSON_GetObjectItem(current, JSON_HUMIDITY_KEY); // Find "humidity"
    if (temp_item == NULL || humidity_item == NULL) { // Check if missing
        fprintf(stderr, "Missing temp_c or humidity\n"); // Print error
        cJSON_Delete(root); 
        return 1; // Return failure
    }

    *temp = (float)cJSON_GetNumberValue(temp_item); // Save temperature
    *humidity = (int)cJSON_GetNumberValue(humidity_item); // Save humidity

    cJSON_Delete(root); 
    return 0; // Return success
}

// Function to log data to CSV file
int log_weather_data(float temp, int humidity) {
    FILE *fp = fopen(LOG_FILE_PATH, "a"); // Open file to append data
    if (fp == NULL) { // Check if open failed
        fprintf(stderr, "Failed to open log file\n"); // Print error
        return 1; // Return failure
    }

    time_t now; // Current time in seconds
    time(&now); // Get current time
    char timestamp[TIMESTAMP_BUFFER_SIZE]; // Buffer for time string
    ctime_r(&now, timestamp); // Convert time to string
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline at end

    fprintf(fp, "%s,%.1f,%d\n", timestamp, temp, humidity); // Write line to file
    fflush(fp); // Force save to SD card
    fclose(fp); // Close file
    return 0; // Return success
}
<<<<<<< HEAD
=======

// Main program entry
int main(void) {
    char *api_key = "7addccf61f8d4dd5b76225325250202";
    char *location = "El_Segundo"; 
    char *response = NULL; // Pointer for JSON string
    float temp; // Storage for temperature
    int humidity; // Storage for humidity

    for (int i = 0; i < TEST_LOOP_COUNT; i++) { // Test loop n times
        if (fetch_weather_data(api_key, location, &response) == 0) { // Fetch data
            printf("Raw JSON: %s\n", response); // Debug print
            if (parse_weather_data(response, &temp, &humidity) == 0) { // Parse data
                if (log_weather_data(temp, humidity) == 0) { // Log data
                    printf("Logged: Temp=%.1fF, Humidity=%d%%\n", temp, humidity); // Confirm log
                } else {
                    fprintf(stderr, "Logging failed\n"); // Log error
                }
            } else {
                fprintf(stderr, "Parsing failed\n"); // Parse error
            }
            free(response); // Free JSON memory
        } else {
            fprintf(stderr, "Fetch failed\n"); // Fetch error
        }
        sleep(LOG_INTERVAL_SECONDS); // Pause for next loop
    }

    return 0; // End program successfully
}
>>>>>>> 53d4253debf638e069688e29afac1ecc96df206d
