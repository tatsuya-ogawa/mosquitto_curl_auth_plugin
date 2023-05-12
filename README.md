# mosquitto_curl_auth_plugin

This is a simple authentication plugin for Mosquitto. When Mosquitto's basic authentication is called, this plugin sends a POST request to the `BACKEND_AUTH_URL` with the parameters "username=%s&password=%s". If the response status code is 200, the authentication is considered successful. Otherwise, the authentication fails.

Additionally, this plugin retrieves an array of `topic_patterns` from the JSON response of the HTTP request. These topic patterns are used to perform ACL checks for subsequent publish and subscribe operations.

## Getting Started

### Docker Environment

1. If you use docker-compose, you can easily try this out.
    ```
    docker-compose -f docker/docker-compose.yml up
    ```
2. A Mosquitto instance with the plugin installed and a mock server will start up.
3. You can use the user credentials "hoge/fuga". Here is a sample publish operation:
    ```
    docker-compose exec -f docker/docker-compose.yml exec mosquitto /bin/ash
    mosquitto_pub -d -t orz -m "test payload" \
        -u hoge -P fuga \
        --topic hoge/sample/fuga
    ```

### Manual Configuration

1. Set the environment variable
    * `MOSQUITTO_PLUGIN_AUTH_BACKEND_URL`
      This should be the request URL for authentication.
2. Set the Mosquitto config
    ```
    plugin path to libmosquitto_curl_auth_plugin.so  
    ```
3. Start Mosquitto
