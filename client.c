#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"      // Definição do IP do servidor
#define SERVER_PORT 50000          // Definição da porta do servidor

int client_socket;                  // Socket do cliente
pthread_t receive_thread;           // Thread para receber mensagens do servidor

// Função executada pela thread para receber mensagens do servidor
void *receive_messages(void *arg) {
    char server_message[2000];
    memset(server_message, '\0', sizeof(server_message));

    while (1) {
        if (recv(client_socket, server_message, sizeof(server_message), 0) < 0) {
            printf("Server disconnected\n");
            break;
        }

        printf("Received from server: %s\n", server_message);
        memset(server_message, '\0', sizeof(server_message)); // Limpa o buffer
    }

    pthread_exit(NULL);  // Encerra a thread
}

int main(void) {
    struct sockaddr_in server_addr;  // Estrutura para armazenar endereço do servidor

    // Criação do socket do cliente
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0) {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Configuração das informações do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Tenta conectar ao servidor
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't connect to the server\n");
        return -1;
    }
    printf("Connected to the server\n");

    // Cria uma thread para receber mensagens do servidor
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) < 0) {
        printf("Error creating thread\n");
        return -1;
    }

    char client_message[2000];   // Buffer para armazenar mensagens do cliente

    while (1) {
        printf("Enter your message: ");
        fgets(client_message, sizeof(client_message), stdin);  // Lê a mensagem do usuário

        // Envia a mensagem para o servidor
        if (send(client_socket, client_message, strlen(client_message), 0) < 0) {
            printf("Can't send message\n");
            break;
        }

        memset(client_message, '\0', sizeof(client_message));  // Limpa o buffer
    }

    close(client_socket);  // Encerra o socket do
}