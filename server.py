import socket
import threading

# Definindo o endereço IP e a porta que o servidor estará ouvindo
HOST = "127.0.0.1"
PORT = 50000

# Função para tratar a conexão de um cliente específico, sendo conn o objeto de socket e addr o endereço
def handle_client(conn, addr):    
    print(f"Connected by {addr}")

    # Loop infinito para receber e responder mensagens do cliente
    while True:
        data = conn.recv(1024)
        # Se não receber dados, encerra a conexão com o cliente
        if not data:
            print("Closing the connection")
            conn.close()
            break
        
        # Imprime a mensagem recebida do cliente
        print(f"Client: {data.decode()}")
        
        # Lê a mensagem do servidor para enviar ao cliente
        server_message = input("Server: ")
        conn.sendall(server_message.encode())

# Criação do socket do servidor
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Vincula o socket ao endereço IP e porta especificados
s.bind((HOST, PORT))

# Coloca o socket em modo de escuta, aguardando conexões de clientes
s.listen()
print("Waiting for a client connection")

# Loop infinito para aceitar múltiplas conexões de clientes
while True:
    # Aceita uma nova conexão de cliente
    conn, addr = s.accept()

    # Cria uma nova thread para tratar a comunicação com o cliente
    client_thread = threading.Thread(target=handle_client, args=(conn, addr))
    client_thread.start()