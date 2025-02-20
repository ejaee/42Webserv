#include "Server.hpp"
#include <fcntl.h>
#include <unistd.h>

Server::Server(const ServerConfig &config): config(config) {
	this->initSocket(config);
}

Server::~Server()
{
}

ListenSd Server::getListenSd() const
{
    return listenSd;
}

SocketAddrV6 &Server::getAddress()
{
    return sockAddr;
}

int &Server::getAddrLen()
{
    return sockAddrLen;
}

void Server::initSocket(const ServerConfig &config) {
	//create a master socket
	if ((listenSd = socket(AF_INET6, SOCK_STREAM, 0)) == 0)
		throw SocketCreationException();
	if (fcntl(listenSd, F_SETFL, O_NONBLOCK) < 0)
		throw SocketOptionException();
	int opt = 1;
	//set master socket to allow multiple connections,
	//this is just a good habit, it will work without this
	if(setsockopt(listenSd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0)
		throw SocketOptionException();

	int port = *config.at("listen").data;
	//type of socket created
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin6_family = AF_INET6;
	sockAddr.sin6_addr = in6addr_any;
	sockAddr.sin6_port = htons(port);
	
	//bind the socket to localhost port
	if (bind(listenSd, (struct sockaddr *)&sockAddr, sizeof(sockAddr))<0) {
		close(listenSd);
		throw SocketBindingException();
	}

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(listenSd, 10000) < 0)
		throw SocketListeningException();

	//accept the incoming connection
	sockAddrLen = sizeof(sockAddr);
}

const char* Server::SocketCreationException::what() const throw()
{
	return "cannot create socket";
}

const char* Server::SocketOptionException::what() const throw()
{
	return "cannot set socket option";
}

const char* Server::SocketBindingException::what() const throw()
{
	return "cannot bind socket";
}

const char* Server::SocketListeningException::what() const throw()
{
	return "cannot listen socket";
}

