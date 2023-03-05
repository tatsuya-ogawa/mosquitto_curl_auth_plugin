install: lib
	cp -a libmosquitto_curl_auth_plugin.so /usr/local/lib
lib: plugin.c
	gcc -fanalyzer -Wall -fPIC -shared plugin.c -o libmosquitto_curl_auth_plugin.so -lcurl -lmosquittopp -lmosquitto