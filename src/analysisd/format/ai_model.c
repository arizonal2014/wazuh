#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Temporary AI model function for testing
char* call_ai_model(const char* log_data)
{

    (void)log_data;
    // For now, we return a static string for testing purposes
    const char* explanation = "This is a temporary AI explanation based on the log data.";

    // Allocate memory for the explanation string
    char* result = (char*)malloc(strlen(explanation) + 1);
    if (result)
    {
        strcpy(result, explanation); // Copy explanation to result
    }

    return result; // Return the explanation string
}
