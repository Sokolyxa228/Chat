#pragma once

#include "imgui.h"
#include "imgui-SFML.h"
#include <thread>
#include <map>
#include <vector>
#include <set>
#include <utility>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <fstream>


struct UserData {
	std::string clientname;
	std::string clientpassword;

	bool checking_password;
	bool service_information;

	std::string message_to_another_client;
	std::string to_client_name;

	std::string message_from_another_client;
	std::string from_client_name;
	int error;
};






class Client
{

private:
	/*-------------Start up Members----------------------*/
	sf::VideoMode video_mode_;
	sf::RenderWindow* window_;
	sf::Event sfml_evnt_;
	sf::Clock delta_clock;

	/*Input window*/
	char name_[100];
	char password_[100];
	bool greeting_flag_;
	bool checking_password_;

	/*Send message window*/
	char another_name_[100];
	char message_[100];
	int error_code_;
	std::string message_for_me_;
	std::string name_message_for_me_;
	std::set<std::string> list_clients_names_;
	std::vector < std::pair<std::string, std::string>> list_messages_;
	std::string choosen_name_chat_;
	bool sending_yourself_;
	int selected;
	/*Work with sockets*/
	sf::TcpSocket socket_;
	sf::Packet last_packet_;



private:
	/*-------------Start up Functions----------------------*/
	void InitSFMLWindow();
	void InitImGui();
	void PollEvents();
	bool Running() const;
	void Update();
	void Render();

	//For all ImGui windows
	void RenderImGui();
	void RunSFML();
	

	/*----------------ImGui Windows-----------------------*/

	void GreetingWindow();
	void LoginWindow();
	void CommunicationWindow();
	void SelectChatMenu();
	void HistoryMessages();

	/*With socket*/
	void ConnectToServer(const char* ip_adress, unsigned short port);
	void SendPacketToServer(sf::Packet& packet);
	void ReceiveMessage();

	/*Domestic comutations*/
	void PrintToConsole(UserData& data);
	void CheckPassword(UserData& data);
	void CheckNewMessage(UserData& data);

public:
	Client();
	void Run();
	~Client();
};

