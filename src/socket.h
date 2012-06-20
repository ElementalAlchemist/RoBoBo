#pragma once
#include "main.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define SOCKET_SPAWN(modName) extern "C" Socket* spawn() {\
	return new modName;\
}

class Socket {
	public:
		/** Socket constructor
		 * Initializes the class variables.
		 */
		Socket();
		
		/** Socket destructor
		 * It's virtual to allow modules to extend this.  It's otherwise unremarkable.
		 */
		virtual ~Socket();
		
		/** API version
		 * This function checks that the socket module API for which the socket module was written matches
		 * the current socket module API.
		 * @return API version
		 */
		virtual unsigned int apiVersion() = 0;
		
		/** Connect to server
		 * This function is called when a protocol module is ready to begin a connection to the IRC server.
		 * @param server The server to which to connect
		 * @param port The port number to connect to, as a string
		 * @param bindAddr The address to bind this socket to, optional
		 */
		virtual void connectServer(std::string server, std::string port, std::string bindAddr = "");
		
		/** Receive data
		 * This function returns a line of data from the server, or an empty string if the connection is broken.
		 * @return An IRC line from the server
		 */
		virtual std::string receive();
		
		/** Send data
		 * This function is used to send data to the server.  It must append the line end (\r\n) to any IRC lines before sending.
		 * @param line Data to send
		 */
		virtual void sendData(std::string line);
		
		/** Close connection
		 * This function closes the connection.  Any quit messages should be sent via sendData() before this, as this will simply
		 * close the socket.
		 */
		virtual void closeConnection();
		
		/** Connection check
		 * Checks whether the connection is still active.
		 * @return true if the connection is still active, false otherwise
		 */
		bool isConnected();
	protected:
		/// The socket fd used to reference this socket, as returned by socket()
		int socketfd;
		/// Holds whether the bot is connected or not, set by the socket module
		bool connected;
};