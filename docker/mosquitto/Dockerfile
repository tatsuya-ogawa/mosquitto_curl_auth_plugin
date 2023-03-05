FROM eclipse-mosquitto:latest as build
RUN apk add --no-cache alpine-sdk make mosquitto-dev curl-dev
COPY ./plugin /opt/build
WORKDIR /opt/build
RUN make install

FROM eclipse-mosquitto:latest
RUN apk add --no-cache curl-dev
COPY --from=0 /usr/local/lib/libmosquitto_curl_auth_plugin.so  /usr/local/lib/