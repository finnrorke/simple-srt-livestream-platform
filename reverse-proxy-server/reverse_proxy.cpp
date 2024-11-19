#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

int get_request_endpoint(char* response, char** route){
    char temp = response[3];
    response[3] = '\0';
    
    if (strcmp(response, "GET"))
        return -1;
    response[3] = temp;

    response = strtok(response, "\r\n");
    
    char* start_path = strchr(response, ' ') + 1;
    char* end_path = strchr(start_path, ' ');
    end_path[0] = '\0';
    
    *route = start_path;
    
    return 0;
}

int main(){
    struct addrinfo     hints;
    struct addrinfo     *result, *rp;
    int                 err, sock;
    
    int                 connected_socket;

    struct sockaddr_in  peer_addr;
    socklen_t           peer_addrlen;

    struct sockaddr     sock_addr_info;
    socklen_t           sock_addr_info_len;


    memset(&hints, 0, sizeof(hints));
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    err = getaddrinfo("0.0.0.0", "443", &hints, &result);
    if (err != 0){
        std::cout << "getaddrinfo error: " << strerror(errno) << std:: endl;
        exit(EXIT_FAILURE);
	}

    for (rp = result; rp != NULL; rp = rp->ai_next){
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        int optval = 1;
        err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (err == -1){
            std::cout << "setsockopt error: " << strerror(errno) << std:: endl;
            close(sock);
            exit(EXIT_FAILURE);
	    }
        
        if(sock == -1)
            continue;
        
        if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0){
            break;
        }
        
        close(sock);
    }
    freeaddrinfo(result);

    if (rp == NULL){
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    err = listen(sock, 5);
    
    if (err != 0){
        std::cout << "listen error: " << strerror(errno) << std:: endl;
        close(sock);
        exit(EXIT_FAILURE);
	}

    peer_addrlen = sizeof(peer_addr);
    connected_socket = accept(sock, (sockaddr*)&peer_addr, &peer_addrlen);

    if (connected_socket == -1){
        std::cout << "accept error: " << strerror(errno) << std:: endl;
        close(sock);
        close(connected_socket);
        exit(EXIT_FAILURE);
    }

    char buf[1024];
    memset(&buf, 0, sizeof(buf));
    ssize_t bytes_recieved = recv(connected_socket, buf, 1024, 0);

    if(bytes_recieved > 0){
        buf[bytes_recieved] = '\0';
        char* route;
        err = get_request_endpoint(buf, &route);
        if(err == -1){
            std::cout << "error" << std::endl;
        }
        std::cout << route << std::endl;
    }


    const char* simple_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    send(connected_socket, simple_response, strlen(simple_response), 0);
    
    close(sock);
    close(connected_socket);
}
