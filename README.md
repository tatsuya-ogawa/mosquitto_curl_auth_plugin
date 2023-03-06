# mosquitto_curl_auth_plugin
* simple auth plugin for mosquitto.
* when mosquitto basic auth was called, this plugin calls MOSQUITTO_PLUGIN_AUTH_BACKEND_URL with post("username=%s&password=%s"). 
* authentication is success if response status code is 200, otherwise fail.

# getting started

# docker environment
1. if you use docker-compose, you can easily try out.
    ```
    docker-compose -f docker/docker-compose.yml up
    ```
2. plugin installed mosquitto and mock server is startup.
3. you can use user(hoge/fuga).sample pub/sub is follwing
    ```
    docker-compose exec -f docker/docker-compose.yml exec mosquitto /bin/ash
    mosquitto_pub -d -t orz -m "test payload" \
        -u hoge -P fuga \
        --topic test/topic
    ```

## manual configuration

1. set environment variable
    * BACKEND_AUTH_URL
        request url for authenticate
2. set mosquitto config
    ```
    plugin path to libmosquitto_curl_auth_plugin.so  
    ```
3. start mosquitto

