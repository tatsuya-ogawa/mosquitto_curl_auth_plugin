#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"

#define UNUSED(A) (void)(A)
static mosquitto_plugin_id_t *mosq_pid = NULL;
char *backend_auth_url = NULL;

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

static int auth_callback(int event, void *event_data, void *userdata)
{
	struct mosquitto_evt_basic_auth *ed = event_data;
	UNUSED(event);
	UNUSED(userdata);
	CURL *curl;
	CURLcode res;
	if(ed->username==NULL || ed->password==NULL){
		mosquitto_log_printf(MOSQ_LOG_ERR, "username or password is not specified");
		return -1;
	}

	curl = curl_easy_init();
	int result = 0;
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, backend_auth_url);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		char *username = curl_easy_escape(curl, ed->username, strlen(ed->username));
		char *password = curl_easy_escape(curl, ed->password, strlen(ed->password));

		const char *param_format = "username=%s&password=%s";
		int param_size = snprintf(NULL, 0, param_format, username, password);
		char *param = malloc(param_size + 1);
		snprintf(param, param_size + 1, param_format, username, password);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, param);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(param));
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
		free(param);
		curl_easy_cleanup(curl);
	}
	if (result == 0)
	{
		return MOSQ_ERR_SUCCESS;
	}
	else
	{
		return MOSQ_ERR_AUTH;
	}
}
int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);
	curl_global_init(CURL_GLOBAL_ALL);
	const char *_backend_auth_url = getenv("BACKEND_AUTH_URL");
	if (_backend_auth_url == NULL)
	{
		mosquitto_log_printf(MOSQ_LOG_ERR, "The environment variable:BACKEND_AUTH_URL is not defined.");
		return -1;
	}
	else
	{
		backend_auth_url = (char *)malloc(strlen(_backend_auth_url) + 1);
		if (backend_auth_url == NULL)
		{
			mosquitto_log_printf(MOSQ_LOG_ERR, "backend_auth_url allocation failed.");
			return -1;
		}
		else
		{
			strcpy(backend_auth_url, _backend_auth_url);
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
	if (backend_auth_url != NULL)
	{
		free(backend_auth_url);
		backend_auth_url = NULL;
	}
	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, auth_callback, NULL);
}