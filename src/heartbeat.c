/*
    Obsidian, a lightweight Classicube server
    Copyright (C) 2018-2019 Sean Baggaley.

    Obsidian is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Obsidian is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with Obsidian.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <curl/curl.h>
#include "heartbeat.h"
#include "defs.h"
#include "player.h"
#include "config.h"
#include "rng.h"

#define HEARTBEAT_URL "https://www.classicube.net/server/heartbeat"
static const char *SALT_CHARACTERS = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void make_salt();
static size_t heartbeat_curl_write_stub(void *buffer, size_t size, size_t nmemb, void *userp);

extern int current_tick;
extern int num_players;
extern rng_t *global_rng;

static int last_heartbeat = -HEARTBEAT_RATE;
static pthread_t hb_thread;
static CURL *curl;
static char *salt = NULL;

typedef struct heartbeat_data_s {
    int num_players;
    int num_online;
} heartbeat_data_t;

void heartbeat_tick() {
    if (current_tick - last_heartbeat < HEARTBEAT_RATE || !configuration->should_heartbeat) {
        return;
    }

    last_heartbeat = current_tick;

    heartbeat_data_t *data = malloc(sizeof(heartbeat_data_t));
    data->num_online = playerman_get_num_online();
    data->num_players = num_players;

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&hb_thread, &attr, heartbeat_run, data);
}

void heartbeat_init() {
    make_salt();

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
}

void *heartbeat_run(void *parm) {
    heartbeat_data_t *data = (heartbeat_data_t *) parm;

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, HEARTBEAT_URL);

    char *esc_name = curl_easy_escape(curl, configuration->name, strlen(configuration->name));
    char *esc_app_name = curl_easy_escape(curl, app_get_full_name(), strlen(app_get_full_name()));

    char fields[512];
    snprintf(fields, sizeof(fields), "name=%s&port=%d&users=%d&max=%d&public=true&salt=%s&software=%s",
        esc_name, 25565, data->num_online, data->num_players, salt, esc_app_name
    );

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, heartbeat_curl_write_stub);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "failed to send heartbeat: %s\n", curl_easy_strerror(res));
    }

    free(data);

    pthread_exit(NULL);
    return NULL;
}

void heartbeat_shutdown() {
    free(salt);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void make_salt() {
    salt = calloc(SALT_LENGTH, 1);
    int nchars = strlen(SALT_CHARACTERS);

    for (int i = 0; i < SALT_LENGTH; i++) {
        int c = rng_next2(global_rng, 0, nchars);
        salt[i] = SALT_CHARACTERS[c];
    }
}

char *heartbeat_get_salt() {
    return salt;
}

size_t heartbeat_curl_write_stub(void *buffer, size_t size, size_t nmemb, void *userp) {
    char *text = (char *) buffer;

    if (strncmp("http", text, 4) != 0) {
        fprintf(stderr, "Message from heartbeat service: %s\n", text);
    }

    return size * nmemb;
}