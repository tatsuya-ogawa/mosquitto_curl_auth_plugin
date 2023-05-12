import pytest
import paho.mqtt.client as mqtt
from concurrent.futures import Future, ThreadPoolExecutor

USERNAME = "hoge"
PASSWORD = "fuga"
MOSQUITTO_HOST = "mosquitto"
def check_mqtt_acl(pub_topic,sub_topic,timeout=10):
    pub_future = Future()
    sub_future = Future()
    sub_connect_future = Future()

    def on_pub_connect(client, userdata, flags, rc):
        print("on_pub_connect start",flush=True)
        if rc == 0:
            client.publish(pub_topic, "Hello, MQTT!", qos=1)
        else:
            pub_future.set_exception(Exception(f"Connection failed with code {rc}"))

    def on_publish(client, userdata, mid):
        print("on_publish start",flush=True)
        pub_future.set_result(True)

    def on_sub_connect(client, userdata, flags, rc):
        print("on_sub_connect start",flush=True)
        sub_connect_future.set_result(True)
        if rc == 0:
            client.subscribe(sub_topic)
        else:
            sub_future.set_exception(Exception(f"Connection failed with code {rc}"))

    def on_message(client, userdata, msg):
        print("on_message start",flush=True)
        sub_future.set_result(True)

    pub_client = mqtt.Client()
    pub_client.username_pw_set(USERNAME, PASSWORD)
    pub_client.on_connect = on_pub_connect
    pub_client.on_publish = on_publish

    sub_client = mqtt.Client()
    sub_client.username_pw_set(USERNAME, PASSWORD)
    sub_client.on_connect = on_sub_connect
    sub_client.on_message = on_message

    sub_client.connect(MOSQUITTO_HOST, 1883, 60)
    sub_client.loop_start()

    sub_connect_future.result(timeout=timeout)

    pub_client.connect(MOSQUITTO_HOST, 1883, 60)
    pub_client.loop_start()


    with ThreadPoolExecutor(max_workers=2) as executor:
        executor.submit(pub_future.result(timeout=timeout))
        executor.submit(sub_future.result(timeout=timeout))
    pub_client.loop_stop()
    sub_client.loop_stop()

def test_pass_plus():
    check_mqtt_acl("hoge/topic/fuga","hoge/+/fuga",timeout=1)

def test_pass_sharp():
    check_mqtt_acl("abc/topic/fuga","abc/#",timeout=1)

def test_acl_error():
    with pytest.raises(Exception) as e_info:
        check_mqtt_acl("fuga/topic/fuga","fuga/#",timeout=3)