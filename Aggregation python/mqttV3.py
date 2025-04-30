import paho.mqtt.client as mqtt
# from aggragate_weights import *
import numpy as np
import time
import os
import threading




BROKER_HOST = "192.168.1.101"
# BROKER_HOST = "192.168.47.203"
port = 1883
topic = "weights/iiottest"

last_sent_index = 0
weights = {}
weightsFinal = []
# expected_nodes = ["1","2","3"]
expected_nodes = ["1","2","3"]

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
                    result = client.publish('/receive-weights', weightsFinal[i],qos=1)

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

# def sendWeights(client):
#     if len(weightsFinal) > 0:
#         block_size = 1000  # Tamaño máximo del bloque
#         total_weights = len(weightsFinal)
#         pending_indices = list(range(total_weights))  # Índices de pesos pendientes

#         while pending_indices:  # Reintenta mientras haya pesos pendientes
#             for i in range(0, len(pending_indices), block_size):  # Dividir en bloques
#                 block = pending_indices[i:i + block_size]  # Obtener bloque actual
#                 print(f"Sending block {i // block_size + 1}/{-(-total_weights // block_size)}")
                
#                 for index in block[:]:  # Iterar sobre los índices del bloque
#                     try:
#                         print(f"Trying to send weight: {index}/{total_weights}")
#                         result = client.publish('/receive-weights', weightsFinal[index], qos=1)

#                         if result.rc == mqtt.MQTT_ERR_SUCCESS:
#                             print(f"Successfully sent weight: {index}")
#                             pending_indices.remove(index)  # Remueve el índice si se envió correctamente
#                         else:
#                             print(f"Failed to send weight {index}, requeuing...")
#                     except Exception as e:
#                         print(f"Error sending weight {index}: {e}")
                    
#                     time.sleep(0.01)  # Retraso entre envíos individuales dentro del bloque
                
#                 if pending_indices:  # Esperar antes del próximo bloque si aún hay pendientes
#                     print("Waiting 30 seconds before sending the next block...")
#                     time.sleep(30)

#         print("All weights sent successfully.")
#     else:
#         print("Can't send the final model. Not all the nodes ended the training process")


def parseData(lines):
    # Verificar cada elemento de la lista para convertirlo a float
    processed_lines = []
    for x in lines:
        if isinstance(x, str):  # Si es una cadena, hacer strip() y convertir a float
            processed_lines.append(float(x.strip()))
        else:  # Si no es una cadena (ya es un número), convertir a float directamente
            processed_lines.append(float(x))
    return np.asarray(processed_lines, dtype=np.float32)


def aggregateWeights(weights=[]):
    # Verificar que los datos sean procesables antes de agregarlos
    try:
        weight = np.array(parseData(weights[0]["data"]), dtype=np.float32)
    except ValueError as e:
        print(f"Error procesando datos: {e}")
        return np.array([])  # Retornar array vacío en caso de error
    
    for index in range(len(weights)):
        if index > 0:
            try:
                weight += np.array(parseData(weights[index]["data"]), dtype=np.float32)
            except ValueError as e:
                print(f"Error procesando datos del nodo {index}: {e}")
                continue

    # Limpiar datos para el próximo entrenamiento
    for node in weights:
        node["done"] = False
        node["data"] = []

    # Promedio de los pesos
    weight /= len(weights)

    # Limitar el rango al de float32 para evitar valores fuera de rango
    weight = np.clip(weight, -3.4e38, 3.4e38)
    
    return weight





# Lista de nodos esperados (debe configurarse de antemano)
  # IDs de los nodos esperados

def on_message(client, userdata, msg):
    global weightsFinal
    topic = msg.topic.replace("/", "")
    payload = msg.payload.decode("utf-8").strip()

    # Registrar nuevos participantes
    if topic == 'add-participant':
        if payload not in weights:
            print(f"New training node added: {payload}")
            client.subscribe((f'/{payload}', 1))
            weights[payload] = {"done": False, "data": []}
        return

    # Enviar los pesos a los clientes
    # if topic == 'get-weights':
    #     sendWeights(client)
    #     return

    # Asegurar que estamos procesando solo nodos esperados
    if topic not in expected_nodes:
        print(f"Unexpected node ID: {topic}")
        return

    # Si el nodo ya completó, ignorar más mensajes
    if weights.get(topic, {}).get("done"):
        return

    # Si se recibe "END", marcar el nodo como completado
    if payload == "END":
        weights[topic]["done"] = True
        print(f"COMPLETED node: {topic}")

        # Verificar si todos los nodos esperados han completado
        all_done = all(weights.get(node, {}).get("done", False) for node in expected_nodes)
        if all_done:
            print("ALL EXPECTED NODES READY. PERFORMING WEIGHTS AGGREGATION.")
            weightsFinal = aggregateWeights([weights[node] for node in expected_nodes])
            np.savetxt('data.txt', weightsFinal, fmt='%.7f')

            print("SENDING WEIGHTS:")
            # sendWeights(client)
            threading.Thread(target=sendWeights, args=(client,), daemon=True).start()

            # Reiniciar el estado de los nodos para el próximo ciclo
            for node in weights.values():
                node["done"] = False
                node["data"] = []
        return

    # Procesar y validar los datos recibidos
    try:
        value = np.float32(float(payload))  # Convertir a float32
        value = np.clip(value, -3.4e38, 3.4e38)  # Limitar el rango
        print(f"Receiving weights node {topic}: {value}\t{len(weights[topic]['data'])}")
        weights[topic]['data'].append(value)
    except ValueError:
        print(f"Invalid payload received from node {topic}: {payload}")




client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
#client.username_pw_set(BROKER_USERNAME, BROKER_PASSWORD)
client.connect(BROKER_HOST, port ,60)#, BROKER_PORT, 60)
client.reconnect_delay_set(min_delay=1, max_delay=240)
# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()