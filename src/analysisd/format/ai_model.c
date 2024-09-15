#include "ai_model.h"
#include "cJSON.h"  // JSON parsing library
#include "shared.h" // Wazuh shared utilities
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Your OpenAI API key
const char* api_key = "sk-your-openai-api-key";

// Func to all AI model using Wazuh's HTTP request system
char* call_ai_model(const char* log_data)
{
    const char* url = "https://api.openai.com/v1/chat/completions";
    const long timeout = 10;        
    size_t max_size = 4096;       
    curl_response* response = NULL;
    char* headers[] = {"Content-Type: application/json", NULL};

    // Authorization header
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers[1] = auth_header;

    // Prepare payload (input log data for the AI model)
    char json_payload[2048];
    snprintf(json_payload,
             sizeof(json_payload),
             "{\"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"system\", \"content\": \"You are an AI that "
             "provides insights based on log data.\"}, {\"role\": \"user\", \"content\": \"Analyze the following log "
             "data and provide insights: %s\"}], \"max_tokens\": 100}",
             log_data);

    // Make HTTP POST request to the OpenAI API using Wazuh's built-in function
    response = wurl_http_request("POST", headers, url, json_payload, max_size, timeout);

    if (response && response->status_code == 200)
    {
        char* ai_explanation = strdup(response->body);
        wurl_free_response(response); // Clean up response
        return ai_explanation;        // Return the explanation
    }
    else
    {
        // Handle errors
        printf("Error: Unable to call AI model. HTTP status: %ld\n", response ? response->status_code : -1);
        if (response)
        {
            wurl_free_response(response); // Clean up response in case of failure
        }
        return strdup("No AI explanation available");
    }
}
