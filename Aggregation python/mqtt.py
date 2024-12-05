# import paho.mqtt.client as mqtt

# # Configuración del broker y tema
# broker = "broker.hivemq.com"  # Broker público de prueba
# port = 1883
# topic = "weights/iiottest"

# # Callback cuando se conecta al broker
# def on_connect(client, userdata, flags, rc):
#     if rc == 0:
#         print("Conectado al broker!")
#         client.subscribe(topic)
#     else:
#         print(f"Error de conexión, código: {rc}")

# # Callback cuando se recibe un mensaje
# def on_message(client, userdata, msg):
#     print(f"Mensaje recibido en '{msg.topic}': {msg.payload.decode()}")

# # Crear cliente MQTT y configurar callbacks
# client = mqtt.Client()
# client.on_connect = on_connect
# client.on_message = on_message

# # Conectar al broker
# client.connect(broker, port, 60)

# # Publicar un mensaje
# client.publish(topic, "¡Hola desde Python!")

# # Iniciar bucle de red para mantener la conexión y procesar mensajes
# client.loop_start()

# try:
#     while True:
#         # Mantener el programa en ejecución
#         pass
# except KeyboardInterrupt:
#     print("Desconexión del broker")
#     client.disconnect()
#     client.loop_stop()


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
#BROKER_PORT = 1606
#BROKER_USERNAME = 'smart_lock'
#BROKER_PASSWORD = "123456789"

weights = {}
weightsFinal = []

if os.path.isfile('data.txt'):
    weightsFinal = np.loadtxt('data.txt', dtype=float)


#with open('json_data.json') as json_file:
#    weightsFinal = json.load(json_file)

# The callback for when the client receives a CONNACK response from the server.
# def on_connect(client, userdata, flags, rc):
#     print("Connected with result code "+str(rc))
#     subs = []
#     #for model in ["health","car"]:
#     subs.append(('/get-weights',1))
#     subs.append(('/add-participant',1))
#     client.subscribe(subs)

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

# The callback for when a PUBLISH message is received from the server.

def sendWeights(client):
    if len(weightsFinal) > 0:
        for i in range(0,len(weightsFinal)):
            # print(len(weightsFinal))
            print(f"SENT WEIGHT: {i}/{len(weightsFinal)}")
            client.publish('/receive-weights', weightsFinal[i])
            time.sleep(0.01)
            client.loop()
        print("Weights sent")
    else: print("Can't send the final model. Not all the nodes ended the training process")

  # Variable global para guardar el índice del último peso enviado

 # Variable global para guardar el índice del último peso enviado

# def sendWeights(client):
#     global last_sent_index
#     if len(weightsFinal) > 0:
#         for i in range(last_sent_index, len(weightsFinal)):
#             try:
#                 # Verificar conexión antes de enviar
#                 if not client.is_connected():
#                     print("Cliente desconectado, intentando reconectar...")
#                     client.reconnect()

#                 # Publicar el peso
#                 print(f"Enviando peso {i + 1}/{len(weightsFinal)}...")
#                 client.publish('/receive-weights', weightsFinal[i], qos=1)
                
#                 # Tiempo de espera para evitar saturación del broker
#                 time.sleep(0.1)

#                 # Actualizar índice del último peso enviado
#                 last_sent_index = i + 1
            
#             except Exception as e:
#                 # Manejo de excepciones en caso de error
#                 print(f"Error al enviar peso {i}: {e}")
#                 print("Reintentando en 1 segundo...")
#                 time.sleep(1)
#                 client.reconnect()  # Intentar reconectar al broker

#         # Si todos los pesos fueron enviados exitosamente
#         print("Todos los pesos han sido enviados.")
#         last_sent_index = 0  # Reiniciar índice para futuros envíos

#     else:
#         print("No se puede enviar el modelo final. No todos los nodos completaron el proceso de entrenamiento.")

# Activar loop en un hilo separado




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
    topic = msg.topic.replace("/","")
    payload = msg.payload.decode("utf-8").strip()
    # print("llego message")

    if topic == 'add-participant':
        # print("llego")

        if payload not in list(weights.keys()):
            print(f"New training node added: {payload}")
            client.subscribe((f'/{payload}',1))
        #else: 
        #    print(f"!!!! WARNING: THERE IS ALREADY A NODE RECORDED WITH nodeNumber={payload} !!!!")
        weights[payload] = {"done": False, "data": []}
        return

    if topic == 'get-weights':
        sendWeights(client)
        client.loop()
        return

    nodeId = topic

    if weights[nodeId]['done'] == True: return

    if payload == "END":
        weights[nodeId]['done'] = True
        print(f"COMPLETED node:  {nodeId}")

        for node in list(weights.values()):
            if node["done"] != True:
                return
            
        print(f"WEIGHTS AGGREGATION")
        weightsFinal = aggregateWeights(list(weights.values()))
        np.savetxt('data.txt', weightsFinal, fmt='%f')

        print(f"SENDING WEIGHTS:")
        sendWeights(client)
        return
    
    print(f"Receiving weights node {nodeId}:  {payload}\t{len(weights[nodeId][f'data'])}")
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
# client.loop_start()