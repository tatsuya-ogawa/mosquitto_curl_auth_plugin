#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"
#include "topic_manager.h"
#define UNUSED(A) (void)(A)
static mosquitto_plugin_id_t *mosq_pid = NULL;
char *auth_backend_url = NULL;
TopicManager topic_manager;

int mosquitto_plugin_version(int supported_version_count, const int *supported_versions)
{
	int i;

	for (i = 0; i < supported_version_count; i++)
	{
		if (supported_versions[i] == 5)
		{
			return 5;
		}
	}
	return -1;
}
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

static int auth_callback(int event, void *event_data, void *userdata)
{
	struct mosquitto_evt_basic_auth *ed = (mosquitto_evt_basic_auth*)event_data;
	UNUSED(event);
	UNUSED(userdata);

	CURL *curl;
	CURLcode res;
	if(ed->username==NULL || ed->password==NULL){
		mosquitto_log_printf(MOSQ_LOG_ERR, "username or password is not specified");
		return -1;
	}
	const char *ip_address = mosquitto_client_address(ed->client);
	if(ip_address==NULL){
        mosquitto_log_printf(MOSQ_LOG_ERR, "connection already closed");
        return -1;
    }


	curl = curl_easy_init();
	int result = 0;
	std::string read_buffer;
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, auth_backend_url);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		char *username = curl_easy_escape(curl, ed->username, strlen(ed->username));
		char *password = curl_easy_escape(curl, ed->password, strlen(ed->password));
		char *ip = curl_easy_escape(curl, ip_address, strlen(ip_address));

		const char *param_format = "username=%s&password=%s&ip=%";
		int param_size = snprintf(NULL, 0, param_format, username, password);
		char *param = (char*)malloc(param_size + 1);
		snprintf(param, param_size + 1, param_format, username, password);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, param);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(param));
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
		res = curl_easy_perform(curl);
		if (res == CURLE_OK)
		{
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			if (response_code != 200)
			{
				mosquitto_log_printf(MOSQ_LOG_ERR, "curl response code error: %d\n", response_code);
				result = -1;
			}
		}
		else
		{
			mosquitto_log_printf(MOSQ_LOG_ERR, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			result = -1;
		}
		curl_free(username);
		curl_free(password);
		curl_free(ip);
		free(param);
		curl_easy_cleanup(curl);
	}
	if (result == 0)
	{
	    topic_manager.cache_topic(read_buffer);
		return MOSQ_ERR_SUCCESS;
	}
	else
	{
		return MOSQ_ERR_PLUGIN_DEFER;
	}
}
int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);
	curl_global_init(CURL_GLOBAL_ALL);
	const char *_auth_backend_url = getenv("MOSQUITTO_PLUGIN_AUTH_BACKEND_URL");
	if (_auth_backend_url == NULL)
	{
		mosquitto_log_printf(MOSQ_LOG_ERR, "The environment variable:MOSQUITTO_PLUGIN_AUTH_BACKEND_URL is not defined.");
		return -1;
	}
	else
	{
		auth_backend_url = (char *)malloc(strlen(_auth_backend_url) + 1);
		if (auth_backend_url == NULL)
		{
			mosquitto_log_printf(MOSQ_LOG_ERR, "auth_backend_url allocation failed.");
			return -1;
		}
		else
		{
			strcpy(auth_backend_url, _auth_backend_url);
		}
	}
	mosq_pid = identifier;
	return mosquitto_callback_register(mosq_pid, MOSQ_EVT_BASIC_AUTH, auth_callback, NULL, NULL);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);
	curl_global_cleanup();
	if (auth_backend_url != NULL)
	{
		free(auth_backend_url);
		auth_backend_url = NULL;
	}
	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, auth_callback, NULL);
}