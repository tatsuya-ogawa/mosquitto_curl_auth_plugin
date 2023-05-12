install: lib
	cp -a libmosquitto_curl_auth_plugin.so /usr/local/lib
lib: plugin.c
	g++ -Wall -fPIC -c topic_manager.c
	g++ -fanalyzer -Wall -fPIC -shared plugin.c topic_manager.o -o libmosquitto_curl_auth_plugin.so -lcurl -lmosquittopp -lmosquitto