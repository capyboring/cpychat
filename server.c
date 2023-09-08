#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

// Array para armazenar os sockets dos clientes
int client_sockets[2] = {0, 0};

// Mutex para sincronização entre threads
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Estrutura para armazenar informações sobre o cliente
typedef struct {
    int sockfd;
    int client_id;
} ClientData;

void *handle_client(void *arg);

int main() {
    int server_sockfd;
    struct sockaddr_in server_address;
    socklen_t client_len;
    pthread_t thread;

    // Criação do socket do servidor
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1) {
        perror("socket");
        exit(1);
    }

    // Configuração das informações do servidor
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(50000);

    // Associação do socket a um endereço e porta
    if (bind(server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("bind");
        exit(1);
    }

    // Escuta por conexões de clientes
    if (listen(server_sockfd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server is running...\n");

    // Loop para aceitar conexões de clientes
    while (1) {
        int client_sockfd;
        struct sockaddr_in client_address;
        client_len = sizeof(client_address);

        // Aceita uma nova conexão de cliente
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
        if (client_sockfd == -1) {
            perror("accept");
            continue;
        }

        // Seção crítica: apenas uma thread pode modificar client_sockets[] de cada vez
        pthread_mutex_lock(&lock);

        if (client_sockets[0] == 0) {
            client_sockets[0] = client_sockfd;
        } else if (client_sockets[1] == 0) {
            client_sockets[1] = client_sockfd;
        } else {
            // Se já houver dois clientes, fecha a conexão adicional
            close(client_sockfd);
            pthread_mutex_unlock(&lock);
            continue;
        }

        pthread_mutex_unlock(&lock);

        printf("New connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Alocando e definindo informações do cliente
        ClientData *data = malloc(sizeof(ClientData));
        data->sockfd = client_sockfd;
        data->client_id = (client_sockfd == client_sockets[0]) ? 1 : 2;

        // Cria uma nova thread para tratar o cliente
        if (pthread_create(&thread, NULL, handle_client, data) != 0) {
            perror("pthread_create");
            continue;
        }
    }

    return 0;
}

// Função para tratar a comunicação com o cliente
void *handle_client(void *arg) {
    ClientData *data = (ClientData *)arg;
    int client_sockfd = data->sockfd;
    int client_id = data->client_id;
    char buffer[1024];
    ssize_t bytes_read;

    while (1) {
        // Recebe mensagem do cliente
        bytes_read = recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            printf("Client %d disconnected\n", client_id);
            break;
        }

        buffer[bytes_read] = '\0';
        printf("Received message from Client %d: %s\n", client_id, buffer);

        // Seção crítica para enviar a mensagem para o outro cliente
        pthread_mutex_lock(&lock);

        if (client_sockfd == client_sockets[0] && client_sockets[1] != 0) {
            if (send(client_sockets[1], buffer, bytes_read, 0) == -1) {
                perror("send");
            }
        } else if (client_sockfd == client_sockets[1] && client_sockets[0] != 0) {
            if (send(client_sockets[0], buffer, bytes_read, 0) == -1) {
                perror("send");
            }
        }

        pthread_mutex_unlock(&lock);
    }

    // Limpar e fechar conexão
    pthread_mutex_lock(&lock);
    if (client_sockfd == client_sockets[0]) {
        client_sockets[0] = 0;
    } else if (client_sockfd == client_sockets[1]) {
        client_sockets[1] = 0;
    }
    pthread_mutex_unlock(&lock);

    free(data);
    close(client_sockfd);
    return NULL;
}