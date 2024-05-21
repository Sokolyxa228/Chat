#include "Server.h"



sf::Packet& operator<<(sf::Packet& inp, const ClientData& msg)
{
	inp << msg.checking_password << msg.message << msg.clientname << msg.error;
	return inp;
}

sf::Packet& operator>>(sf::Packet& out, ClientData& msg)
{
	out >> msg.clientname >> msg.clientpassword >> msg.checking_password >> msg.service_information >> msg.message >> msg.to_another_client;
	return out;
}



void Server::TcpListen()
{
	if (listener_.listen(3000) != sf::Socket::Done) {
		std::cout << "Error!\n";
	}
	
	
}

void Server::AcceptClient()
{
	while (true) {
		ClientData * new_user = new ClientData();

		if (listener_.accept(new_user->socket) != sf::Socket::Done) {			
			std::cout << "Error!\n";
		}
		else {
			
			new_user->socket.setBlocking(false);
			std::cout << "Hi client! "  << '\n';
			new_user->client_port = new_user->socket.getRemotePort();
			new_user->online = true;
			///new_user->checking_password = false; ????
			clients_list_.push_back(new_user);
			
		}
		
	}
}

void Server::ReceiveClientInformation()
{
	while (true) {
		for (int i = 0; i < clients_list_.size();++i) {
			sf::Packet packet_from;
			sf::Socket::Status recived_status = (clients_list_[i]->socket).receive(packet_from);
			if (recived_status == sf::Socket::Disconnected && clients_list_[i]->online) {
				clients_list_[i]->client_port = -1;
				std::cout << "Client went away: " << clients_list_[i]->client_port << ' ' << clients_list_[i]->client_port << "\n";
				clients_list_[i]->online = false;
			}
			
			
			if (packet_from.getDataSize() > 0)
			{
				packet_from >> (*clients_list_[i]);
				if (clients_list_[i]->checking_password == false) {
					WorkWithLogin(*clients_list_[i]);
					SendToClient(*clients_list_[i]);
					
				}
				else {
					SendToClient(*clients_list_[i]);
				}
				
			}
			else {

				//std::cout << "Error received\n";
			}
			
		}
	}
}

void Server::WorkWithLogin(ClientData& user)
{
	std::ifstream in("source/database.txt");
	bool record_base = true;
	std::string check_name, check_password;
	while (in >> check_name >> check_password) {
		//std::cout << check_name << ' ' << check_password << '\n';
		if (check_name == user.clientname && check_password == user.clientpassword) {
			record_base = false;
			user.checking_password = true;
			user.error = 0;
			break;
		}
		else if ((check_name == user.clientname && check_password != user.clientpassword)) {
			std::cout << "Entrance error\n";
			record_base = false;
			user.error = 1;
			break;
		}
	}
	in.close();
	if (record_base) {
		std::ofstream out("source/database.txt",std::iostream::out | std::iostream::app);
		out << user.clientname << ' ' << user.clientpassword << std::endl;
		out.close();
		user.checking_password = true;
		std::ofstream out2("", std::iostream::out | std::iostream::app);
		out2 << user.clientname << std::endl;
		out2.close();
	
	}


}

void Server::SendToClient(ClientData& user)
{

	sf::Packet packet;
		
	if (user.service_information) {
		user.message = "Server message";
		packet << user;
		if (user.socket.send(packet) != sf::Socket::Done)
		{
			std::cout << "Could not send packet\n";
		}
		else {
			std::cout << "Server: I sent the packet to client " << user.clientname << '\n';
		}
	}
	else {

		for (int i = 0; i < clients_list_.size(); ++i) {
			if ((user.to_another_client == clients_list_[i]->clientname) && (clients_list_[i]->online) && clients_list_[i]->checking_password) {
				user.error = 2;
				packet << user;
				if (clients_list_[i]->socket.send(packet) == sf::Socket::Done) {
					std::cout << "Server: I sent the packet to client " << clients_list_[i]->clientname << '\n';
					return;
				}
			}
		}
		user.error = 3;
		packet << user;
		if (user.socket.send(packet) == sf::Socket::Done) {
			std::cout << "I send error num = 2\n";
		}
	}
	
}

void Server::PrintToConsole()
{
	for (int i = 0; i < clients_list_.size(); ++i) {
		std::cout << clients_list_[i]->clientname << " " << clients_list_[i]->clientpassword
			<< " " << clients_list_[i]->checking_password << " " << clients_list_[i]->service_information << ' ' << clients_list_[i]->message
			<< ' ' << clients_list_[i]->to_another_client << '\n';
	}
}

void Server::Run() {

	
	TcpListen();
	std::thread a (&Server::AcceptClient,this);
	std::thread b(&Server::ReceiveClientInformation,this);
	a.join();
	b.join();
	
}

