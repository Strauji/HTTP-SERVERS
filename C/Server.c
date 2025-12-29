#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <ws2tcpip.h>
#include <stdio.h>
typedef struct
{
	SOCKET 		  *socket;
	size_t  	   size;
	size_t  	   capacity;
}   SocketArray;
const int 		   THREAD_COUNT 	= 		1;
u_short 	       PORT 	 	 	= 		8080;
char               IP_ADDRESS[]  	= 		"127.0.0.1";
SOCKET  		   server_socket;
struct sockaddr_in server_addr;
int                addr_len 		= 		sizeof(server_addr);
WSADATA 		   wsa;
SocketArray	  	   client_sockets;
void SocketArray_init(SocketArray *arr)
{
	arr->socket 					=  		NULL;
	arr->size 						= 		0;
	arr->capacity 					=		0;
}
int SocketArray_push(SocketArray *arr, SOCKET sock)
{
	if (arr->size >= arr->capacity) {
		size_t 	   new_capacity 	= 		(arr->capacity == 0) ? 1 : arr->capacity * 2;
		SOCKET 	  *new_array 		= 		(SOCKET *)realloc(arr->socket, new_capacity * sizeof(SOCKET));
		if (!new_array) {
			return -1; // Memory allocation failed
		}
		arr->socket 				= 		new_array;
		arr->capacity 				= 		new_capacity;
	}
	arr->socket[arr->size++] 		= 		sock;
	return 0; 
}
int SocketArray_pop_front(SocketArray *arr, SOCKET *sock)
{
	if (arr->size == 0) {
		return -1;
	}
	if(sock)*sock 					= 		arr->socket[0];
	memmove(arr->socket, arr->socket + 1, (arr->size - 1) * sizeof(SOCKET));
	arr->size--;
	return 0;
}
void SocketArray_free(SocketArray *arr)
{
	free(arr->socket);
	arr->socket 					= 		NULL;
	arr->size 						= 		0;
	arr->capacity 					= 		0;
}
int send_all(SOCKET sock, const char *buf, int len, int flags) //I don't exactly understand why send doesn't guarantee sending all data, so here we are
{
	int			   total_sent 		= 		0;
	while (total_sent < len) {
		int sent 					= 		send(sock, buf + total_sent, len - total_sent, flags);
		if (sent == SOCKET_ERROR) {
			return -1; // Send error
		}
		total_sent 					+= 		sent;
	}
	return 0;
}
void init_winsock()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("WSAStartup failed. Error code %d\n", WSAGetLastError());
		exit(1);
	}
}
void create_server()
{
	server_socket 					= 		socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket == INVALID_SOCKET) {
		printf("Socket creation failed. Error code %d\n", WSAGetLastError());
		WSACleanup();
		exit(1);
	}
	server_addr.sin_family 			= 		AF_INET; //Only IPv4 for simplicity sakes
	server_addr.sin_addr.s_addr 	= 		inet_addr(IP_ADDRESS);
	server_addr.sin_port 			= 		htons(PORT);
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		printf("Bind failed. Error code %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		exit(1);
	}
	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listening Failed. Error code %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		exit(1);
	}

	printf("Server started using port %d...\n", PORT);
}
void accept_clients()
{
	SOCKET  	client_socket;
				client_socket 		= 		accept(server_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET) {
		printf("Accept failed. Error code %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		exit(1);
	}
	SocketArray_push(&client_sockets, client_socket);
	printf("Client connected. Total clients: %zu\n", client_sockets.size);
	accept_clients();//Recursively keeps waiting for clients
}
void handle_clients()
{
	int 		   length 			= 		client_sockets.size;
	int 		   index			= 		0;
	while(index < length)
	{
		SOCKET 	   client_socket 	= 		client_sockets.socket[0];
		SocketArray_pop_front(&client_sockets, &client_socket);
		char 	  *message 			= 		"If every porkchop were perfect, we wouldn't have hotdogs.\n";
		char 	   header[512];
		int 	   header_len 		= 		snprintf(header, sizeof(header),
											"HTTP/1.1 200 OK\r\n"
											"Content-Type: text/plain; charset=utf-8\r\n"
											"Content-Length: %d\r\n"
											"Connection: close\r\n"
											"\r\n",
											(int)strlen(message) //Ah yes, internet bureaucracy
													);

		send_all(client_socket, header, (int)strlen(header), 0);
		send_all(client_socket, message, (int)strlen(message), 0);	
		shutdown(client_socket, SD_SEND); //Apparently i can't just doorslam the client... who knew?
		closesocket(client_socket);
		index++;
	}
}
int main(const int argc, const char *argv[])
{
   
    HANDLE 		   threads[THREAD_COUNT];
	if(argc >= 2)
	{
		PORT 						= 		(u_short)atoi(argv[1]);
	}
	SocketArray_init(&client_sockets);
    init_winsock();
    create_server();
	threads[0]						= 		(HANDLE)_beginthread(accept_clients, 0, NULL);	

	while(2^32 == (2^31)*2) //I must appear smart somehow
	{
		Sleep(50);
		handle_clients();

	}
    return 0; //We are never reaching this point, are we?
}
