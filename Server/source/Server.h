#pragma once
#include <SFML/Network.hpp>
#include <iostream>
#include <set>
#include <fstream>
#include <thread>

struct ClientData {
	std::string clientname;
	std::string clientpassword;
	sf::TcpSocket socket;
	int client_port;
	bool online;
	bool checking_password;
	bool service_information;
	std::string message;
	std::string to_another_client;
};

class Server
{
private:
	sf::TcpListener listener_;
	std::vector<ClientData *> clients_list_;

private:
	void TcpListen();
	void AcceptClient();
	void ReceiveClientInformation();
	void WorkWithLogin(ClientData & user);
	void SendToClient(ClientData& user);
	void PrintToConsole();

public:
	void Run();
};

