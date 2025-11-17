#!/usr/bin/env python3
import socket

# Configuración del servidor
HOST = "127.0.0.1"   # Dirección IP local (localhost)
PORT = 8080          # Puerto de escucha

# Crear el socket (IPv4, TCP)
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Permitir reutilizar la dirección en caso de reinicio rápido
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Asignar IP y puerto al socket
server_socket.bind((HOST, PORT))

# Poner el socket en modo escucha
server_socket.listen(5)  # hasta 5 conexiones en cola
print(f"Servidor escuchando en {HOST}:{PORT}...")

while True:
    # Esperar conexión de un cliente
    client_socket, client_address = server_socket.accept()
    print(f"Cliente conectado desde {client_address}")

    # Recibir datos (máx 1024 bytes)
    data = client_socket.recv(1024).decode()
    print(f"Mensaje recibido: {data}")

    # Responder al cliente
    client_socket.sendall("Hola desde el servidor!".encode())

    # Cerrar la conexión con el cliente
    client_socket.close()
