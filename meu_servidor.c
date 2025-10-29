#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <sys/stat.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shell32.lib")

#define BUFFER_SIZE 4096
#define DEFAULT_PORT 8080
#define MAX_PATH_LENGTH 1024

void print_error(const char *msg) {
    fprintf(stderr, "ERRO: %s (Código: %d)\n", msg, GetLastError());
}

int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return 0;
    return S_ISDIR(statbuf.st_mode);
}

const char* get_mime_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext == NULL) return "application/octet-stream";
    
    ext++; 
    
    if (_stricmp(ext, "html") == 0 || _stricmp(ext, "htm") == 0) return "text/html";
    if (_stricmp(ext, "txt") == 0) return "text/plain";
    if (_stricmp(ext, "pdf") == 0) return "application/pdf";
    if (_stricmp(ext, "jpg") == 0 || _stricmp(ext, "jpeg") == 0) return "image/jpeg";
    if (_stricmp(ext, "png") == 0) return "image/png";
    if (_stricmp(ext, "gif") == 0) return "image/gif";
    if (_stricmp(ext, "js") == 0) return "application/javascript";
    if (_stricmp(ext, "css") == 0) return "text/css";
    if (_stricmp(ext, "json") == 0) return "application/json";
    if (_stricmp(ext, "ico") == 0) return "image/x-icon";
    
    return "application/octet-stream";
}


char* create_directory_listing(const char *dirpath, const char *request_path) {
    char *html = malloc(65536); 
    if (!html) return NULL;
    
    snprintf(html, 65536,
             "<!DOCTYPE html>\n"
             "<html>\n"
             "<head>\n"
             "    <title>Index of %s</title>\n"
             "    <style>\n"
             "        body { font-family: Arial, sans-serif; margin: 40px; }\n"
             "        h1 { color: #333; }\n"
             "        ul { list-style-type: none; padding: 0; }\n"
             "        li { margin: 5px 0; }\n"
             "        a { text-decoration: none; color: #0066cc; }\n"
             "        a:hover { text-decoration: underline; }\n"
             "        .dir { font-weight: bold; }\n"
             "    </style>\n"
             "</head>\n"
             "<body>\n"
             "    <h1>Index of %s</h1>\n"
             "    <ul>\n", 
             request_path, request_path);
    
    size_t current_len = strlen(html);
    
    
    if (strcmp(request_path, "/") != 0) {
        char parent_path[MAX_PATH_LENGTH];
        strcpy(parent_path, request_path);
        char *last_slash = strrchr(parent_path, '/');
        if (last_slash && last_slash != parent_path) {
            *last_slash = '\0';
        } else {
            strcpy(parent_path, "/");
        }
        
        current_len += snprintf(html + current_len, 65536 - current_len,
                               "        <li><a href=\"%s\"> ../</a></li>\n", parent_path);
    }
    
    
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char search_path[MAX_PATH_LENGTH];
    
    snprintf(search_path, sizeof(search_path), "%s\\*", dirpath);
    
    hFind = FindFirstFileA(search_path, &findFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            
            if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
                continue;
            }
            
           
            char file_url[MAX_PATH_LENGTH * 2];
            if (strcmp(request_path, "/") == 0) {
                snprintf(file_url, sizeof(file_url), "/%s", findFileData.cFileName);
            } else {
                snprintf(file_url, sizeof(file_url), "%s/%s", request_path, findFileData.cFileName);
            }
            
           
            char escaped_url[MAX_PATH_LENGTH * 3];
            char *dest = escaped_url;
            const char *src = file_url;
            while (*src && dest < escaped_url + sizeof(escaped_url) - 1) {
                if (*src == ' ') {
                    *dest++ = '%';
                    *dest++ = '2';
                    *dest++ = '0';
                } else {
                    *dest++ = *src;
                }
                src++;
            }
            *dest = '\0';
            
        
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                
                current_len += snprintf(html + current_len, 65536 - current_len,
                                       "        <li><a class=\"dir\" href=\"%s\"> %s/</a></li>\n",
                                       escaped_url, findFileData.cFileName);
            } else {
                
                const char *icon = "";
                const char *ext = strrchr(findFileData.cFileName, '.');
                if (ext) {
                    if (_stricmp(ext, ".jpg") == 0 || _stricmp(ext, ".jpeg") == 0 || 
                        _stricmp(ext, ".png") == 0 || _stricmp(ext, ".gif") == 0) {
                        
                    } else if (_stricmp(ext, ".pdf") == 0) {
                        
                    } else if (_stricmp(ext, ".html") == 0 || _stricmp(ext, ".htm") == 0) {
                        
                    } else if (_stricmp(ext, ".txt") == 0) {
                        
                    } else if (_stricmp(ext, ".exe") == 0 || _stricmp(ext, ".msi") == 0) {
                        
                    } else if (_stricmp(ext, ".zip") == 0 || _stricmp(ext, ".rar") == 0) {
                        
                    }
                }
                
                
                char size_str[32];
                double size = (double)findFileData.nFileSizeLow;
                const char *unit = "bytes";
                
                if (size > 1024 * 1024) {
                    size /= (1024 * 1024);
                    unit = "MB";
                } else if (size > 1024) {
                    size /= 1024;
                    unit = "KB";
                }
                
                if (strcmp(unit, "bytes") == 0) {
                    snprintf(size_str, sizeof(size_str), "%.0f %s", size, unit);
                } else {
                    snprintf(size_str, sizeof(size_str), "%.2f %s", size, unit);
                }
                
                current_len += snprintf(html + current_len, 65536 - current_len,
                                       "        <li><a href=\"%s\">%s %s</a> <span style=\"color: #666; font-size: 0.9em;\">(%s)</span></li>\n",
                                       escaped_url, icon, findFileData.cFileName, size_str);
            }
        } while (FindNextFileA(hFind, &findFileData) != 0);
        
        FindClose(hFind);
    }
    
    
    current_len += snprintf(html + current_len, 65536 - current_len,
                           "    </ul>\n"
                           "    <hr>\n"
                           "    <p><em>Servidor HTTP em C - %s</em></p>\n"
                           "</body>\n"
                           "</html>", dirpath);
    
    return html;
}


int serve_file(SOCKET client_socket, const char *filepath, const char *mime_type) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        return 0; 
    }
    
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    
    char header[BUFFER_SIZE];
    int header_len = snprintf(header, sizeof(header),
                             "HTTP/1.1 200 OK\r\n"
                             "Content-Type: %s\r\n"
                             "Content-Length: %ld\r\n"
                             "Connection: close\r\n"
                             "Server: MeuServidor/1.0\r\n"
                             "\r\n",
                             mime_type, file_size);
    
    if (send(client_socket, header, header_len, 0) == SOCKET_ERROR) {
        fclose(file);
        return -1;
    }
    
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) == SOCKET_ERROR) {
            fclose(file);
            return -1;
        }
    }
    
    fclose(file);
    return 1;
}


void handle_request(SOCKET client_socket, const char *base_dir) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        return;
    }
    buffer[bytes_received] = '\0';
    
    
    char method[16], path[MAX_PATH_LENGTH];
    if (sscanf(buffer, "%15s %1023s", method, path) != 2) {
        return;
    }
    
    printf("Requisicao: %s %s\n", method, path);
    
    
    if (strcmp(method, "GET") != 0) {
        const char *response = 
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";
        send(client_socket, response, strlen(response), 0);
        return;
    }
    
    
    char physical_path[MAX_PATH_LENGTH];
    
    
    if (strcmp(path, "/") == 0) {
        strcpy(physical_path, base_dir);
    } else {
        
        
        const char *url_path = path + 1; 
        
        
        char decoded_path[MAX_PATH_LENGTH];
        char *dest = decoded_path;
        const char *src = url_path;
        while (*src && dest < decoded_path + MAX_PATH_LENGTH - 1) {
            if (*src == '%' && src[1] == '2' && src[2] == '0') {
                *dest++ = ' ';
                src += 3;
            } else {
                *dest++ = *src++;
            }
        }
        *dest = '\0';
        
        snprintf(physical_path, sizeof(physical_path), "%s\\%s", base_dir, decoded_path);
    }
    
    
    DWORD attrib = GetFileAttributesA(physical_path);
    if (attrib == INVALID_FILE_ATTRIBUTES) {
        
        const char *response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><h1>404 Not Found</h1><p>O arquivo solicitado nao existe.</p></body></html>";
        send(client_socket, response, strlen(response), 0);
        return;
    }
    
   
    if (attrib & FILE_ATTRIBUTE_DIRECTORY) {
        char index_path[MAX_PATH_LENGTH];
        
        
        snprintf(index_path, sizeof(index_path), "%s\\index.html", physical_path);
        if (GetFileAttributesA(index_path) != INVALID_FILE_ATTRIBUTES) {
            
            serve_file(client_socket, index_path, "text/html");
        } else {
            
            char *html = create_directory_listing(physical_path, path);
            if (html) {
                char header[BUFFER_SIZE];
                int header_len = snprintf(header, sizeof(header),
                                         "HTTP/1.1 200 OK\r\n"
                                         "Content-Type: text/html\r\n"
                                         "Content-Length: %zu\r\n"
                                         "Connection: close\r\n"
                                         "Server: MeuServidor/1.0\r\n"
                                         "\r\n",
                                         strlen(html));
                
                send(client_socket, header, header_len, 0);
                send(client_socket, html, strlen(html), 0);
                free(html);
            } else {
                const char *response = 
                    "HTTP/1.1 500 Internal Server Error\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                send(client_socket, response, strlen(response), 0);
            }
        }
    } else {
        
        const char *mime_type = get_mime_type(physical_path);
        if (!serve_file(client_socket, physical_path, mime_type)) {
            
            const char *response = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><body><h1>404 Not Found</h1><p>O arquivo solicitado nao existe.</p></body></html>";
            send(client_socket, response, strlen(response), 0);
        }
    }
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);
    int port = DEFAULT_PORT;
    char base_dir[MAX_PATH_LENGTH];
    
    printf("Servidor\n");
    
    
    if (argc >= 2) {
        strncpy(base_dir, argv[1], sizeof(base_dir) - 1);
        base_dir[sizeof(base_dir) - 1] = '\0';
    } else {
        if (_getcwd(base_dir, sizeof(base_dir)) == NULL) {
            strcpy(base_dir, ".");
        }
    }
    
    
    if (argc >= 3) {
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            port = DEFAULT_PORT;
        }
    }
    
    
    DWORD attrib = GetFileAttributesA(base_dir);
    if (attrib == INVALID_FILE_ATTRIBUTES || !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
        fprintf(stderr, "Erro: Diretorio '%s' nao existe.\n", base_dir);
        return 1;
    }
    
    printf("Diretorio: %s\n", base_dir);
    printf("Porta: %d\n", port);
    printf("URL: http://localhost:%d\n", port);
    
   
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        print_error("WSAStartup falhou");
        return 1;
    }
    
   
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        print_error("Nao foi possivel criar socket");
        WSACleanup();
        return 1;
    }
    
    
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        print_error("setsockopt falhou");
    }
    
   
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        print_error("Bind falhou");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        print_error("Listen falhou");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
        
    
    while (1) {
        
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET) {
            print_error("Accept falhou");
            continue;
        }
        
        printf("Cliente conectado: %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        
        handle_request(client_socket, base_dir);
        
        
        closesocket(client_socket);
        printf("Requisicao concluida\n\n");
    }
    
    
    closesocket(server_socket);
    WSACleanup();
    return 0;
}