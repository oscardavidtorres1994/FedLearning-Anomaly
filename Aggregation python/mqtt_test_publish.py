import paho.mqtt.client as mqtt
import time

# Configuración del broker y tema
broker = "broker.hivemq.com"  # Broker público de prueba
port = 1883
topic = "alexandrade_chat_app/user1"

# Callback cuando se conecta al broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado al broker!")
        client.subscribe(topic)
    else:
        print(f"Error de conexión, código: {rc}")

# Callback cuando se recibe un mensaje
def on_message(client, userdata, msg):
    print(f"Mensaje recibido en '{msg.topic}': {msg.payload.decode()}")

# Crear cliente MQTT y configurar callbacks
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Conectar al broker
client.connect(broker, port, 60)

# Iniciar bucle de red para mantener la conexión y procesar mensajes
client.loop_start()

try:
    while True:
        # Publicar un mensaje cada segundo
        mensaje = "Message: {1}"
        # client.publish(topic, mensaje)
        print(f"Mensaje enviado: {mensaje}")
        time.sleep(10)
        mensaje = "Message: {2}"
        # client.publish(topic, mensaje)
        print(f"Mensaje enviado: {mensaje}")
        time.sleep(10)
except KeyboardInterrupt:
    print("Desconexión del broker")
    client.disconnect()
    client.loop_stop()
