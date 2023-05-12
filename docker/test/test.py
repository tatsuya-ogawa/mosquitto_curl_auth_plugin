import pytest
import paho.mqtt.client as mqtt
from concurrent.futures import Future, ThreadPoolExecutor

USERNAME = "hoge"
PASSWORD = "fuga"

def test_mqtt_acl():
    pub_future = Future()
    sub_future = Future()

    def on_pub_connect(client, userdata, flags, rc):
        print("on_pub_connect start",flush=True)
        if rc == 0:
            client.publish("hoge/topic/fuga", "Hello, MQTT!", qos=1)
        else:
            pub_future.set_exception(Exception(f"Connection failed with code {rc}"))

    def on_publish(client, userdata, mid):
        print("on_publish start",flush=True)
        pub_future.set_result(True)

    def on_sub_connect(client, userdata, flags, rc):
        print("on_sub_connect start",flush=True)
        if rc == 0:
            client.subscribe("hoge/topic/fuga")
        else:
            sub_future.set_exception(Exception(f"Connection failed with code {rc}"))

    def on_message(client, userdata, msg):
        sub_future.set_result(True)

    pub_client = mqtt.Client()
    pub_client.username_pw_set(USERNAME, PASSWORD)
    pub_client.on_connect = on_pub_connect
    pub_client.on_publish = on_publish

    sub_client = mqtt.Client()
    sub_client.username_pw_set(USERNAME, PASSWORD)
    sub_client.on_connect = on_sub_connect
    sub_client.on_message = on_message

    pub_client.connect('mosquitto', 1883, 60)
    sub_client.connect('mosquitto', 1883, 60)

    pub_client.loop_start()
    sub_client.loop_start()

    with ThreadPoolExecutor(max_workers=2) as executor:
        try:
            executor.submit(pub_future.result(timeout=10))
            executor.submit(sub_future.result(timeout=10))
        except Exception as e:
            pytest.fail(str(e))
    pub_client.loop_stop()
    sub_client.loop_stop()