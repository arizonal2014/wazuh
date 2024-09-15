#include "ai_model.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Your OpenAI API key (for testing purposes, include it directly)
const char* api_key =
    "sk-proj-SScj8qbhxerVYNVq_DJzPqh8hHqFifr097iLZvNNqPcaaj2yjU-"
    "NwFjQCu4azZT5QAfhOnsPYZT3BlbkFJYK4N1q1aWAKcxdj0a2GCEw5kzHbYgCV96vOKGgBW1wXm2iaEoSUaQVgInGM8N-Qok1JZtf7PwA";

// Callback function to handle the response from cURL
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    char** response_ptr = (char**)userp;

    // Allocate memory for the response
    *response_ptr = (char*)realloc(*response_ptr, strlen(*response_ptr) + realsize + 1);
    if (*response_ptr == NULL)
    {
        printf("Failed to allocate memory for response\n");
        return 0;
    }

    // Append received data into the response
    strncat(*response_ptr, contents, realsize);

    return realsize;
}

// Extract AI explanation from JSON response
char* extract_ai_explanation(const char* json_response)
{
    cJSON* json = cJSON_Parse(json_response);
    if (json == NULL)
    {
        printf("Error parsing JSON response\n");
        return NULL;
    }

    // Navigate the JSON to get the AI's response content
    cJSON* choices = cJSON_GetObjectItem(json, "choices");
    if (choices)
    {
        cJSON* first_choice = cJSON_GetArrayItem(choices, 0);
        if (first_choice)
        {
            cJSON* message = cJSON_GetObjectItem(first_choice, "message");
            if (message)
            {
                cJSON* content = cJSON_GetObjectItem(message, "content");
                if (content && cJSON_IsString(content))
                {
                    char* explanation = strdup(content->valuestring);
                    cJSON_Delete(json);
                    return explanation;
                }
            }
        }
    }

    cJSON_Delete(json);
    return NULL;
}

// Function to call the OpenAI API
char* call_ai_model(const char* log_data)
{
    CURL* curl;
    CURLcode res;
    struct curl_slist* headers = NULL;
    char* response = (char*)calloc(1, sizeof(char)); // Initialize response buffer

    // OpenAI API endpoint for chat-based models
    const char* api_url = "https://api.openai.com/v1/chat/completions";

    // Initialize cURL
    curl = curl_easy_init();
    if (!curl)
    {
        printf("Failed to initialize curl\n");
        return NULL;
    }

    // Set headers
    headers = curl_slist_append(headers, "Content-Type: application/json");
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, auth_header);

    // Set up JSON payload for the chat model with gpt-3.5-turbo
    char json_data[2048];
    snprintf(json_data,
             sizeof(json_data),
             "{\"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"system\", \"content\": \"You are an AI that "
             "provides insights based on log data.\"}, {\"role\": \"user\", \"content\": \"Analyze the following log "
             "data and provide insights: %s\"}], \"max_tokens\": 100}",
             log_data);

    // Set cURL options
    curl_easy_setopt(curl, CURLOPT_URL, api_url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(response);
        response = NULL;
    }
    else
    {
        // Extract AI explanation from the response
        char* ai_explanation = extract_ai_explanation(response);
        if (ai_explanation)
        {
            free(response); // Free the original response memory
            return ai_explanation;
        }
    }

    // Cleanup
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    return response;
}
