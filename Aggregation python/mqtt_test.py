import paho.mqtt.client as mqtt

broker_address ="192.168.154.203"  # Usa "localhost" para el broker local
port = 1883
topic = "test/topic"

# Definir la función de callback
def on_message(client, userdata, message):
    print(f"Mensaje recibido en {message.topic}: {message.payload.decode()}")

# Crear cliente MQTT
client = mqtt.Client()

# Asignar la función de callback para recibir mensajes
client.on_message = on_message

# Conectar al broker y suscribirse al tema
client.connect(broker_address, port)
client.subscribe(topic)

# Iniciar el bucle para recibir mensajes
client.loop_start()
print("Suscrito al tema, esperando mensajes...")

try:
    # Mantener el script en ejecución
    while True:
        pass
except KeyboardInterrupt:
    # Parar el bucle y desconectar al terminar
    client.loop_stop()
    client.disconnect()
    print("Desconectado del broker MQTT")
