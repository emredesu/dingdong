#include "connection_manager.hpp"

ConnectionManager::ConnectionManager() {
	WSADATA wsadata;
	int result;

	// Initialize Windows Sockets library, version 2.2.
	result = WSAStartup(MAKEWORD(2, 2), &wsadata);

	if (result != 0)
		std::cerr << "WSAStartup failed, error: " << result << "\n";
}

// Returns last WSA error if there was an error, or "9999" if server timed out and "8888" if the client timed out.
int ConnectionManager::init(std::string connection_type) {
	connection_data.sin_family = AF_INET; // Using IPv4
	connection_data.sin_port = htons(DEFAULT_PORT);

	int result = 0;

	if (connection_type == "server") {
		connection_data.sin_addr.s_addr = INADDR_ANY; // Bind the socket to all available interfaces - or in other words, accept connections from any IPv4 address. We'll change this after we establish our first connection with the client.

		// Create a socket for the server to listen from client for data / send data to client.
		sock = socket(connection_data.sin_family, SOCK_DGRAM, 0);
		if (sock == INVALID_SOCKET) {
			std::cerr << "Error occured while creating server socket: " << WSAGetLastError() << "\n";
			return WSAGetLastError();
		}

		// Bind the listening socket.
		result = bind(sock, (SOCKADDR*)&connection_data, connection_data_len);
		if (result == SOCKET_ERROR) {
			std::cerr << "Listening socket bind failed with error: " << WSAGetLastError() << "\n";
			closesocket(sock);
			return WSAGetLastError();
		}

		std::cout << "Awaiting connection..." << "\n";
		if (!await_first_connection()) {
			std::cerr << "Either no one connnected during the 60 second period, or there was a problem with the server. Last WSA error:" << WSAGetLastError() << "\n";
			if (WSAGetLastError() == 0)
				return 9999;
			else
				return WSAGetLastError();
		}
		else {
			std::cout << "Connected successfully!" << "\n";
			is_connected = true;
		}
	}
	else if (connection_type == "client") {
		InetPton(connection_data.sin_family, (PCWSTR)(server_ipv4.c_str()), &connection_data.sin_addr.s_addr); // Set the IP address to connect to on the connection_data structure.

		// Create a socket for sending data to server.
		sock = socket(connection_data.sin_family, SOCK_DGRAM, IPPROTO_UDP);
		if (sock == INVALID_SOCKET) {
			std::cerr << "Error occured while creating client socket: " << WSAGetLastError() << "\n";
			return WSAGetLastError();
		}

		std::wcout << "Attempting to connect to " << server_ipv4 << "..." << "\n";
		if (!establish_first_connection()) {
			std::cerr << "There was a problem connecting the server. Last WSA error: " << WSAGetLastError() << "\n";
			if (WSAGetLastError() == 0)
				return 8888;
			else
				return WSAGetLastError();
		}
		else {
			std::wcout << "Successfully connected to " << server_ipv4 << "!" << "\n";
			is_connected = true;
		}
	}

	// Put the socket in non-blocking mode.
	unsigned long mode = 1;
	if (ioctlsocket(sock, FIONBIO, (unsigned long*)&mode) == SOCKET_ERROR) {
		std::cerr << "Error while putting the socket into non-blocking mode: " << WSAGetLastError() << "\n";
	}

	return 0;
}

void ConnectionManager::reset() {
	puts("Connection manager reset!");
	is_connected = false;
	closesocket(sock);
	sock = INVALID_SOCKET;
	memset(&connection_data, 0, sizeof(connection_data)); // Get rid of the data from the previous connection.
	memset(&receive_buffer, 0, sizeof(receive_buffer));
}

/*
Functions "establish_first_connection" and "await_first_connection" do something that's quite similar to the three-way handshake method of a TCP connection.
*/

bool ConnectionManager::establish_first_connection() { // This will be used by the client.
	// Set up the file descriptor set.
	FD_ZERO(&fdset);
	FD_SET(sock, &fdset);
	
	int send_result = INT32_MAX;
	std::string syn_message = "hi ily <3";

	send_result = sendto(sock, syn_message.c_str(), syn_message.length(), 0, (SOCKADDR*)&connection_data, connection_data_len);
	if (send_result == SOCKET_ERROR) {
		std::cerr << "Error occured while attempting to send SYN to server: " << WSAGetLastError() << "\n";
	}
	else {
		int result = 0;
		int receive_result = 0;

		// Set up the timeval struct for the timeout.
		// We'll wait for 10 seconds for the server to respond, or else we'll call the connection off.
		client_wait_timeout.tv_sec = 10; // seconds
		client_wait_timeout.tv_usec = 0; // microseconds

		// Wait until the timeout or until we receive data.
		result = select(sock, &fdset, NULL, NULL, &client_wait_timeout);
		if (result == 0) {
			std::cout << "Client timeout." << "\n";
			return false;
		}
		else if (result == -1)
			std::cerr << "Error occured while awaiting first connection data from server. Last WSA error:" << WSAGetLastError() << "\n";

		receive_result = recvfrom(sock, receive_buffer, DEFAULT_BUFFER_LENGTH, 0, (SOCKADDR*)&connection_data, &connection_data_len);
		if (receive_result > 0) { // If we received any data before the timeout, return true.
			std::string client_ack_message = ";-;";
			std::cout << receive_buffer << "\n";
			sendto(sock, client_ack_message.c_str(), client_ack_message.length(), 0, (SOCKADDR*)&connection_data, connection_data_len);

			return true;
		}
	}
	return false;
}

bool ConnectionManager::await_first_connection() { // This will be used by the server.
	int result = 0;
	int receive_result = 0;
	int send_result = 0;

	// Set up the file descriptor set.
	FD_ZERO(&fdset);
	FD_SET(sock, &fdset);

	// Set up the timeval struct for the timeout.
	// We'll wait for 60 seconds for someone to connect and if someone doesn't connect, we'll cancel the server.
	server_wait_timeout.tv_sec = 60; // seconds
	server_wait_timeout.tv_usec = 0; // microseconds

	// Wait until the timeout or until we receive data.
	result = select(sock, &fdset, NULL, NULL, &server_wait_timeout);
	if (result == 0) {
		std::cout << "Timeout." << "\n";
		return false;
	}
	else if (result == -1)
		std::cerr << "Error occured while awaiting first connection data from client. Last WSA error: " << WSAGetLastError() << "\n";

	receive_result = recvfrom(sock, receive_buffer, DEFAULT_BUFFER_LENGTH, 0, (SOCKADDR*)&connection_data, &connection_data_len); // We set the first connected client as the only suitable connector from now on here.
	if (receive_result > 0) { // If we received any data before the timeout, let the client know that we acknowledge their request and return true.
		std::string ack_message = "ok";

		send_result = sendto(sock, ack_message.c_str(), ack_message.length(), 0, (SOCKADDR*)&connection_data, connection_data_len); // Let the client know that we received their message.
		if (send_result != SOCKET_ERROR)
			return true;
	}
	return false;
}

std::string ConnectionManager::receive_data() {
	ZeroMemory(receive_buffer, sizeof(receive_buffer)); // Clean the receive buffer of any possibly remaining data.

	int receive_result = 9999;
	u_long ioctl_result = 123;

	while (true) { // When ioctl with FIONREAD results 0, that means there's no datagram pending in the receive queue. We'll use this to grab only the last received package.
		receive_result = recvfrom(sock, receive_buffer, DEFAULT_BUFFER_LENGTH, 0, (SOCKADDR*)&connection_data, &connection_data_len);

		// Handle errors.
		if (receive_result == SOCKET_ERROR) {
			switch (WSAGetLastError()) {
				case WSAEWOULDBLOCK:
					break;
				case WSAECONNRESET:
					return "CONNRESET";
					break;
				default:
					std::cerr << "Unhandled error while receiving data: " << WSAGetLastError() << "\n";
			}
		}

		ioctlsocket(sock, FIONREAD, &ioctl_result);
		if (ioctl_result == 0)
			break;
	}

	if (receive_result > 0) {
		return std::string(receive_buffer, receive_result); // Using the built-in method of casting char to std::string.
	}

	return "NONE";
}
	
std::string ConnectionManager::send_data(std::string data) {
	int send_result = 666;
	send_result = sendto(sock, data.c_str(), data.length(), 0, (SOCKADDR*)&connection_data, connection_data_len);

	// Handle errors.
	if (send_result == SOCKET_ERROR) {
		std::cerr << "Error while sending data: " << WSAGetLastError() << "\n";
		return std::string("FAIL");
	}
	else
		return std::string("OK");
}