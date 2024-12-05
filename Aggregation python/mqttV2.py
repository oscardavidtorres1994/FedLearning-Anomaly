import paho.mqtt.client as mqtt
# from aggragate_weights import *
import numpy as np
import time
import os
import threading



BROKER_HOST = "localhost"
port = 1883
topic = "weights/iiottest"

last_sent_index = 0
weights = {}
weightsFinal = []

if os.path.isfile('data.txt'):
    weightsFinal = np.loadtxt('data.txt', dtype=float)




def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado al broker!")
        client.subscribe(topic)
        subs = []
    #for model in ["health","car"]:
        subs.append(('/get-weights',1))
        subs.append(('/add-participant',1))
        client.subscribe(subs)
    else:
        print(f"Error de conexión, código: {rc}")
        #for d in weightsFinal:
        #    client.publish('/receive-weights', d)


def sendWeights(client):
    if len(weightsFinal) > 0:
        pending_indices = list(range(len(weightsFinal)))  # Índices de pesos pendientes

        while pending_indices:  # Reintenta mientras haya pesos pendientes
            for i in pending_indices[:]:  # Itera sobre una copia de la lista
                try:
                    print(f"Trying to send weight: {i}/{len(weightsFinal)}")
                    result = client.publish('/receive-weights', weightsFinal[i])

                    if result.rc == mqtt.MQTT_ERR_SUCCESS:
                        print(f"Successfully sent weight: {i}")
                        pending_indices.remove(i)  # Remueve el índice si se envió correctamente
                    else:
                        print(f"Failed to send weight {i}, requeuing...")
                except Exception as e:
                    print(f"Error sending weight {i}: {e}")
                
                time.sleep(0.01)  # Retraso entre envíos

            if pending_indices:
                print("Retrying pending weights...")
                time.sleep(1)  # Espera antes de reintentar
        print("All weights sent successfully.")
    else:
        print("Can't send the final model. Not all the nodes ended the training process")


def parseData(lines = []):
    return np.asarray([float(x.strip()) for x in lines])

def aggregateWeights(weights=[]):
    weight = parseData(weights[0]["data"])
    
    for index in range(len(weights)):
        if index > 0:
            weight += parseData(weights[index]["data"])
    weights[index]["done"] = False
    weights[index]["data"] = []
    
    weight /= len(weights)
    
    return weight


def on_message(client, userdata, msg):
    global weightsFinal
    topic = msg.topic.replace("/", "")
    payload = msg.payload.decode("utf-8").strip()

    if topic == 'add-participant':
        if payload not in list(weights.keys()):
            print(f"New training node added: {payload}")
            client.subscribe((f'/{payload}', 1))
        weights[payload] = {"done": False, "data": []}
        return

    if topic == 'get-weights':
        sendWeights(client)
        return

    nodeId = topic

    if weights[nodeId]['done'] == True:
        return

    if payload == "END":
        weights[nodeId]['done'] = True
        print(f"COMPLETED node: {nodeId}")

        for node in list(weights.values()):
            if not node["done"]:
                return
            
        print("WEIGHTS AGGREGATION")
        weightsFinal = aggregateWeights(list(weights.values()))
        np.savetxt('data.txt', weightsFinal, fmt='%f')

        print("SENDING WEIGHTS:")
        sendWeights(client)
        return
    
    print(f"Receiving weights node {nodeId}: {payload}\t{len(weights[nodeId]['data'])}")
    weights[nodeId]['data'].append(payload)


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
#client.username_pw_set(BROKER_USERNAME, BROKER_PASSWORD)
client.connect(BROKER_HOST, port ,60)#, BROKER_PORT, 60)
client.reconnect_delay_set(min_delay=1, max_delay=120)
# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()