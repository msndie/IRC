#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <arpa/inet.h>
#include <iostream>
#include "Bot.hpp"

Bot::Bot() {}

Bot::~Bot() {}

void Bot::startBot() {
	int sockfd;
	int status;
	int numbytes;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *p;
	char s[INET6_ADDRSTRLEN];
	char buf[100];

	_config = new Configuration();
	_config->parseConfiguration("configuration.yaml");

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	status = getaddrinfo(_config->getConfig().find("server.ip")->second.c_str(),
						 _config->getConfig().find("server.port")->second.c_str(),
						 &hints, &servinfo);
	if (status != 0) {
		std::string err;
		err.append("getaddrinfo error: ");
		err.append(gai_strerror(status));
		std::cerr << err << std::endl;
		return;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if (p == nullptr) {
		fprintf(stderr, "client: failed to connect\n");
		return;
	}

	inet_ntop(p->ai_family, inet_ntoa(reinterpret_cast<sockaddr_in*>(p)->sin_addr),
			  s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);

	if ((numbytes = recv(sockfd, buf, 100-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);

	close(sockfd);
}
