#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <curl/curl.h>

#define MAX_CODE_SIZE 100000
#define MAX_JSON_SIZE 150000

// Helper function to escape special characters for a JSON string literal
void json_escape(const char *input, char *output) {
    while (*input) {
        switch (*input) {
            case '\\': *output++ = '\\'; *output++ = '\\'; break;
            case '"':  *output++ = '\\'; *output++ = '"';  break;
            case '\n': *output++ = '\\'; *output++ = 'n';  break;
            case '\r': *output++ = '\\'; *output++ = 'r';  break;
            case '\t': *output++ = '\\'; *output++ = 't';  break;
            default:   *output++ = *input;                 break;
        }
        input++;
    }
    *output = '\0';
}

// Scans the current directory for .c and .h files and collects their content
void gather_codebase(char *buffer, size_t max_size) {
    DIR *dir = opendir(".");
    struct dirent *entry;
    if (!dir) return;

    size_t current_len = 0;
    buffer[0] = '\0';

    while ((entry = readdir(dir)) != NULL) {
        // Check for .c or .h files
        char *ext = strrchr(entry->d_name, '.');
        if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0)) {
            FILE *f = fopen(entry->d_name, "r");
            if (!f) continue;

            current_len += snprintf(buffer + current_len, max_size - current_len, "\n--- File: %s ---\n", entry->d_name);
            
            char ch;
            while ((ch = fgetc(f)) != EOF && current_len < max_size - 10) {
                buffer[current_len++] = ch;
            }
            buffer[current_len] = '\0';
            fclose(f);
        }
    }
    closedir(dir);
}

// Callback function for curl to handle the API response streaming
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char *response = (char *)userp;
    strncat(response, (char *)contents, realsize);
    return realsize;
}

// Primitive JSON extractor to pull the text out of Gemini's response structure
void extract_markdown_content(const char *json_response) {
    // Gemini structures text under: "candidates" -> "content" -> "parts" -> "text"
    const char *text_key = "\"text\": \"";
    char *start = strstr(json_response, text_key);
    if (!start) {
        printf("Failed to find text block in API response.\n");
        return;
    }
    start += strlen(text_key);

    FILE *readme = fopen("README.md", "w");
    if (!readme) return;

    // Process characters, unescaping \n and \" back into clean markdown
    while (*start && !(*start == '"' && *(start - 1) != '\\')) {
        if (*start == '\\' && *(start + 1) == 'n') {
            fputc('\n', readme);
            start += 2;
        } else if (*start == '\\' && *(start + 1) == '"') {
            fputc('"', readme);
            start += 2;
        } else if (*start == '\\' && *(start + 1) == '\\') {
            fputc('\\', readme);
            start += 2;
        } else {
            fputc(*start, readme);
            start++;
        }
    }
    fclose(readme);
    printf("README.md successfully updated via C script!\n");
}

int main() {
    char *api_key = getenv("GEMINI_API_KEY");
    if (!api_key) {
        fprintf(stderr, "Error: GEMINI_API_KEY environment variable not set.\n");
        return 1;
    }

    // 1. Gather and escape code
    char *raw_code = malloc(MAX_CODE_SIZE);
    char *escaped_code = malloc(MAX_CODE_SIZE * 2);
    gather_codebase(raw_code, MAX_CODE_SIZE);
    json_escape(raw_code, escaped_code);
    free(raw_code);

    // 2. Build the JSON Payload
    char *payload = malloc(MAX_JSON_SIZE);
    snprintf(payload, MAX_JSON_SIZE, 
        "{\"contents\": [{\"parts\": [{\"text\": \""
        "You are an expert technical writer. Review this codebase and generate a high-quality README.md.\\n\\n"
        "The README must strictly use these exact headings:\\n"
        "## Purpose\\nExplain what it does and why it exists.\\n"
        "## Getting Started\\nProvide clear compilation instructions using gcc and local execution examples.\\n"
        "## Usage\\nShow concrete code snippets or terminal usage patterns.\\n"
        "## Collaboration\\nStandard contribution guidelines.\\n"
        "## Support\\nWhere to report bugs.\\n\\n"
        "Return ONLY raw markdown. Do not wrap in backticks.\\n\\n"
        "Codebase:\\n%s\"}]}]}", escaped_code);
    free(escaped_code);

    // 3. Setup Curl and Dispatch API Request
    CURL *curl = curl_easy_init();
    char *response_buffer = calloc(MAX_JSON_SIZE, 1);
    
    if (curl) {
        char url[512];
        snprintf(url, sizeof(url), "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=%s", api_key);
        
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_buffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            extract_markdown_content(response_buffer);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    free(payload);
    free(response_buffer);
    return 0;
}