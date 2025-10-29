#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 4096

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;
    FILE *file;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int total_bytes = 0;
    int headers_processed = 0;
    
    printf("NAV\n");
    
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        printf("Erro ao inicializar Winsock\n");
        return 1;
    }
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Erro ao criar socket\n");
        WSACleanup();
        return 1;
    }
    
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Erro: Nao foi possivel conectar ao servidor\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    
    printf("Conectado ao servidor!\n");
    
    char *request = 
        "GET /imagem.jpg HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Connection: close\r\n"
        "\r\n";
    
    printf("Solicitando imagem.jpg...\n");
    
    if (send(sock, request, strlen(request), 0) < 0) {
        printf("Erro ao enviar requisicao\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    
    file = fopen("imagem_baixada.jpg", "wb");
    if (!file) {
        printf("Erro ao criar arquivo\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
        
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        if (!headers_processed) {
            char *body_start = NULL;
            for (int i = 0; i < bytes_received - 3; i++) {
                if (buffer[i] == '\r' && buffer[i+1] == '\n' && 
                    buffer[i+2] == '\r' && buffer[i+3] == '\n') {
                    body_start = buffer + i + 4;
                    break;
                }
            }
            
            if (body_start) {
                int body_size = bytes_received - (body_start - buffer);
                if (body_size > 0) {
                    fwrite(body_start, 1, body_size, file);
                    total_bytes += body_size;
                }
                headers_processed = 1;
            }
        } else {
            fwrite(buffer, 1, bytes_received, file);
            total_bytes += bytes_received;
        }
        
        printf("\rRecebidos: %d bytes", total_bytes);
        fflush(stdout);
    }
    
    fclose(file);
    closesocket(sock);
    WSACleanup();
    
    printf("\n");
    if (total_bytes > 100) { 
        printf("SUCESSO: Imagem salva como 'imagem_baixada.jpg' (%d bytes)\n", total_bytes);
    } else {
        
        remove("imagem_baixada.jpg"); 
        return 1;
    }
    
    return 0;
}