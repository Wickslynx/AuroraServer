#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include "routes.h"


#define getroute(route) (structAddress);

typedef struct smthstruct {
    void (*load_web_files)(const char *path_js, const char *path_css, const char *path_html);  
    void (*route_handler)(const char *method, const char *path);
} smthstruct;

void load_web_files(const char *path_js, const char *path_css, const char *path_html) {
    FILE *js_file = NULL;
    FILE *css_file = NULL;
    FILE *html_file = NULL;

    if (path_js != NULL) js_file = fopen(path_js, "r");
    if (path_css != NULL) css_file = fopen(path_css, "r");
    if (path_html != NULL) html_file = fopen(path_html, "r");

    if (js_file == NULL || css_file == NULL || html_file == NULL) {
        perror("Failed to open one or more web files");
    } else {
        printf("Web files opened successfully.\n");
    }

    if (js_file) fclose(js_file);
    if (css_file) fclose(css_file);
    if (html_file) fclose(html_file);
}

void route_handler(const char *method, const char *path) {
    if (path == NULL || path[0] != '/') {
        printf("Invalid path\n");
        return;
    }

    FILE *html_file = fopen("index.html", "r");  // Assuming index.html for the example
    if (html_file == NULL) {
        perror("Failed to open HTML file");
        return;
    }

    // Load HTML file content
    char content[1000];  // Buffer to store file content
    size_t bytesRead = fread(content, 1, sizeof(content), html_file);
    content[bytesRead] = '\0';  // Null-terminate the content
    fclose(html_file);

    if (bytesRead >= 990) {
        printf("File is too big to handle.\n");
        return;
    }

    const char *contentType = "text/html";

    char response[2000];
    sprintf(response, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n\r\n"
            "%s", 
            contentType, strlen(content), content);

    printf("Response sent:\n%s\n", response);  // Simulate sending the response
}

void init_obj(smthstruct *pSelf) {
    pSelf->load_web_files = load_web_files;
    pSelf->route_handler = route_handler;
}



