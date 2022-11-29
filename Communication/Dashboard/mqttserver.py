# python3.6

import random
import time
from paho.mqtt import client as mqtt_client


broker = '127.0.0.1'
port = 1883
topic = "msp"
# generate client ID with pub prefix randomly
client_id = f'python-mqtt-{random.randint(0, 100)}'
username = ''
password = ''


def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

reliabilityCount = 0
msgLength = 0
start = None
def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        #print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        global reliabilityCount, start, msgLength
        # Check throughput
        if msgLength <20:
            start = time.time()
        elif msgLength >= 2000:
            print(str(time.time() - start))
        msgLength += len(msg.payload.decode(encoding="UTF-8"))
        print(msgLength)

        

        # Check latency

        # if reliabilityCount == 0:
        #     start = time.time()
        #     print(start)
        # elif reliabilityCount == 30:
        #     end = time.time()
        #     print(str(end-start))
        # reliabilityCount += 1
        # print(f"Msg: {msg.payload.decode('utf-8')}")
        # print(f"Count: {reliabilityCount}")
        
    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()


if __name__ == '__main__':
    run()