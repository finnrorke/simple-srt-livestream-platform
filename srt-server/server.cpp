#include <iostream>
#include <srt/srt.h>
#include <sys/socket.h>

struct USERINFO {
	std::string stream_key;
	std::string username;
};

int SrtListenCallback(void* opaq, SRTSOCKET ns, int hsversion, 
	const struct sockaddr* peeraddr, const char* streamid){
	
	USERINFO* userinfo = static_cast<USERINFO*>(opaq);
	
}

int main() {
	SRTSOCKET 					socket;
	// Variable that holds the addr info criteria
	struct addrinfo 			hints;
	// Result of the addr info
	struct addrinfo 			*result, *rp;
	// Holds the sockaddr of the peer 
	struct sockaddr_in 			peer_addr;
	int                			peer_addrlen;
	// New socket bound to the opposite socket at the peer
	SRTSOCKET 					new_sock;
	int 						srtread;
	// For storing function status'
	int 						err;

	//Setting configuration for getaddrinfo
	memset(&hints, 0, sizeof(hints));
	// Only allow IPv4
	hints.ai_family = AF_INET;
	// SOCK_DGRAM as SRT is based on UDP
	hints.ai_socktype = SOCK_DGRAM;
	// Used for binding a sicket that will accept connections
	hints.ai_flags = AI_PASSIVE;

	// Get addr info (use 0.0.0.0 for everything on the containers network), port number greater than 5000
	err = getaddrinfo("0.0.0.0", "7500", &hints, &result);  
	if (err != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
	}

	srt_startup();
	
	socket = srt_create_socket();
	if (socket == SRT_INVALID_SOCK){
		fprintf(stderr, "Count not create socket");
		exit(EXIT_FAILURE);
	}

	// Loop through each addrinfo structures until we successfully bind 
	for (rp = result; rp != NULL; rp = rp->ai_next){
		if(srt_bind(socket, rp->ai_addr, rp->ai_addrlen) == 0 )
			break;
	}

	freeaddrinfo(result);

	if(rp == NULL){
		fprintf(stderr, "Could not bind");
		exit(EXIT_FAILURE);
	}
	
	// Start listening for incoming connections.
	srt_listen(socket, 5);
	//Using srt_listen_callback to get the streamid
	srt_listen_callback(socket, &SrtListenCallback, NULL);
	// Create new socket by accepting incoming connection.
	new_sock = srt_accept(socket, (sockaddr*)&peer_addr, &peer_addrlen);
	
	

	if (new_sock == SRT_INVALID_SOCK){
		fprintf(stderr, "Count not create socket");
		exit(EXIT_FAILURE);
	}

	char msg[1316];
	memset(&msg, 0, 1316);

	for (;;){
		err = srt_recv(new_sock, msg, 1316);
		if(err == SRT_ERROR){
			fprintf(stderr, "Error recieving data\n");
			continue;
		}
		std::cout << msg << std::endl;
	}

	srt_close(socket);
	srt_close(new_sock);
	srt_cleanup();
}
