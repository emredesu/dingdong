#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <string>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_PORT 27015
#define DEFAULT_BUFFER_LENGTH 64

class ConnectionManager {
	private:
		fd_set fdset;
		struct timeval client_wait_timeout;
		struct timeval server_wait_timeout;

		SOCKET sock = INVALID_SOCKET;

		// This is where we'll be setting up connection parameters or where we'll be storing the parameters for a connection that's made.
		SOCKADDR_IN connection_data;
		int connection_data_len = sizeof(connection_data);

		char receive_buffer[DEFAULT_BUFFER_LENGTH] = { 0 }; // The object where the recieved data will be placed on.
	public:
		std::wstring server_ipv4;

		bool is_connected = false;
		std::string type = "uwu";

		ConnectionManager();
		int init(std::string connection_type);
		void reset();
		bool establish_first_connection();
		bool await_first_connection();
		std::string receive_data();
		std::string send_data(std::string data);
};