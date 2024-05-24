#include "Client.h"


#include <iostream>

sf::Packet& operator<<(sf::Packet& inp, const UserData& msg)
{
	inp << msg.clientname << msg.clientpassword << msg.checking_password <<msg.service_information << msg.message_to_another_client << msg.to_client_name;
	return inp;
}

sf::Packet& operator>>(sf::Packet& out,UserData& msg)
{
	out >> msg.checking_password >> msg.message_from_another_client >> msg.from_client_name >> msg.error >> msg.to_client_name;
	return out;
}



void Client::InitSFMLWindow()
{
	video_mode_ = sf::VideoMode(1200, 800);
	window_ = new sf::RenderWindow(video_mode_, "SuperChat", sf::Style::Close | sf::Style::Titlebar);
	window_->setFramerateLimit(60);
}

void Client::InitImGui()
{
	ImGui::SFML::Init(*window_);
}

void Client::PollEvents()
{
	while (window_->pollEvent(sfml_evnt_))
	{
		ImGui::SFML::ProcessEvent(sfml_evnt_);
		switch (sfml_evnt_.type)
		{
		case sf::Event::Closed:
			window_->close();
			break;
		}
	}

}

void Client::RunSFML()
{
	while (Running()) {
		
		Update();
		Render();
	}

	ImGui::SFML::Shutdown();
}

void Client::Run() {
	
	std::thread new_thred(&Client::ReceiveMessage, this);
	RunSFML();
	new_thred.join();
	
}


Client::Client() : greeting_flag_(true), checking_password_(false), error_code_(0), sending_yourself_(false), choosen_name_chat_(""), selected(-1)
{
	name_[0] = '\0';
	password_[0] = '\0';
	another_name_[0] = '\0';
	message_[0] = '\0';
	InitSFMLWindow();
	InitImGui();
}

Client::~Client()
{
	socket_.disconnect();

}



bool Client::Running() const
{
	return window_->isOpen();
}

void Client::Update()
{
	//Update PollEvents
	PollEvents();
	ImGui::SFML::Update(*window_, delta_clock.restart());


}

void Client::RenderImGui()
{
	if (greeting_flag_) {
		GreetingWindow();
	}
	else if (checking_password_) {
		CommunicationWindow();
		
	}
	else {
		LoginWindow();
	}
	
	
}



void Client::Render()
{
	RenderImGui();
	window_->clear();
	ImGui::SFML::Render(*window_);
	window_->display();
}





void Client::GreetingWindow() {
	ImGui::SetNextWindowSize(ImVec2(1200, 800));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	auto& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImColor(83, 145, 189);
	style.Colors[ImGuiCol_Button] = ImColor(11, 72, 115);
	
	if (ImGui::Begin("name",nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {
		ImGui::SetWindowFontScale(3);
		ImGui::SetCursorPos(ImVec2(370, 200));
		ImGui::Text("Welcome to SuperChat :)");
		ImGui::SetCursorPos(ImVec2(470, 280));
		if (ImGui::Button("Start", ImVec2(250, 120))) {
			greeting_flag_ = false;
			ConnectToServer("127.0.0.1", 3000);
		}
	}ImGui::End();
}


void Client::LoginWindow()
{
	ImGui::SetNextWindowSize(ImVec2(1200, 800));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	if (ImGui::Begin("Input Block", nullptr, ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoCollapse)) {
		ImGui::SetWindowFontScale(2);
		//auto& style = ImGui::GetStyle();
		//style.Colors[ImGuiCol_WindowBg] = ImColor(25, 123, 255);
		ImGui::Text("User name: ");
		ImGui::InputText("##user name", name_, 100);
		ImGui::Text("Password: ");
		ImGui::InputText("##password", password_, IM_ARRAYSIZE(password_), ImGuiInputTextFlags_Password);
		ImGui::SetCursorPos(ImVec2(10, 150));
		if (ImGui::Button("Login", ImVec2(200, 100))) {
			if (name_[0] != '\0' && password_[0] != '\0') {
				UserData mes;
				mes.clientname = name_;
				mes.clientpassword = password_;
				mes.checking_password = false;
				mes.service_information = true;
				mes.message_to_another_client = "Hello server!";
				mes.to_client_name = "Server";
				sf::Packet packet;
				packet << mes;
				std::cout << "My name is " << name_ << '\n';
				SendPacketToServer(packet);
				



			}
		}
		if (error_code_ == 1) {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			ImGui::Text("Invalid username or password");
			ImGui::PopStyleColor();
			
		}

		
	}
	ImGui::End();

}

void Client::CommunicationWindow()
{

	ImGui::SetNextWindowSize(ImVec2(1200, 800));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	if (ImGui::Begin(name_, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
		ImGui::SetWindowFontScale(2);

		ImGui::Text("Send message to: ");
		ImGui::InputText("##clossed", another_name_, 100);

		ImGui::Text("Your message: ");
		ImGui::InputText("##clossed2", message_, 100);

		if (ImGui::Button("Send", ImVec2(200, 100))) {
			if (std::string(another_name_,sizeof(another_name_)) == std::string(name_, sizeof(name_))) {
				sending_yourself_ = true;
			}
			else {
				sending_yourself_ = false;
				UserData mes;
				mes.clientname = name_;
				mes.clientpassword = password_;
				mes.checking_password = true;
				mes.service_information = false;
				mes.message_to_another_client = message_;
				mes.to_client_name = another_name_;
				sf::Packet packet;
				packet << mes;
				SendPacketToServer(packet);
			}
		}
		SelectChatMenu();
		if (sending_yourself_) {
			ImGui::SetCursorPos(ImVec2(300, 200));
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			ImGui::Text("You can not send message to yourself!");
			ImGui::PopStyleColor();
		}
		
	
		if (error_code_ == 3) {
			ImGui::SetCursorPos(ImVec2(300, 200));
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			ImGui::Text("User was not found");
			ImGui::PopStyleColor();

			
		}
		else if (error_code_ == 5) {
			ImGui::SetCursorPos(ImVec2(300, 200));
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			ImGui::Text("User is not online now");
			ImGui::PopStyleColor();
		}



	}ImGui::End();

	
}

void Client::SelectChatMenu()
{
	
	if (ImGui::TreeNode("All chats:"))
	{
		int n = 1;
		
		for (std::string name: list_clients_names_)
		{
			const char* name_for_print= name.c_str();
			
			if (ImGui::Selectable(name_for_print, selected == n)) {
				selected = n;
				choosen_name_chat_ = name;
			}
				
				
			n++;
		}
		if (choosen_name_chat_ != "") {
			HistoryMessages();
		}
		ImGui::TreePop();
	}
	

}

void Client::HistoryMessages()
{
	
	ImGui::SetNextWindowSize(ImVec2(600, 600));
	if (ImGui::Begin("##Chat with", nullptr, ImGuiWindowFlags_NoResize)) {
		ImGui::SetWindowFontScale(2.5);
		for (auto element : list_messages_) {
			std::string first = element.first;
			std::string second = element.second;
			std::string subfirst = first.substr(1);
			if (first == choosen_name_chat_) {
				std::string print_inf = first + ": " + second;
				//std::cout << print_inf << '\n';
				const char* new_mes = print_inf.c_str();
				ImGui::Text(new_mes);
			}
			else if (subfirst == choosen_name_chat_) {
				//std::cout << first << ' ' << second << '\n';
				std::string print_inf = "me: " + second;
				//std::cout << print_inf << '\n';
				const char* new_mes = print_inf.c_str();
				ImGui::Text(new_mes);
			}
		}
	} ImGui::End();
	
}



void Client::ConnectToServer(const char* ip_adress, unsigned short port)
{
	
	if (socket_.connect(ip_adress, port) != sf::Socket::Done) {
		std::cout << "Could not connect to the server\n";
	}
	else {
		std::cout <<  "I am connected\n";
	}
	socket_.setBlocking(false);
}

void Client::SendPacketToServer(sf::Packet& packet)
{
	if (socket_.send(packet) != sf::Socket::Done)
	{
		std::cout << "Could not send packet\n";
	}
	else {
		std::cout << "I sent the packet\n";
	}
}

void Client::ReceiveMessage()
{
	sf::Packet packet_from;

	while (true) {
		sf::Socket::Status recived_status = socket_.receive(packet_from);
		if (checking_password_) {


			if (packet_from.getDataSize() > 0) {
				UserData new_sms;
				packet_from >> new_sms;
				CheckNewMessage(new_sms);
				//PrintToConsole(new_sms);
			}
		}
		else {
			if (packet_from.getDataSize() > 0) {
				UserData new_sms;
				packet_from >> new_sms;
				CheckPassword(new_sms);

			}

		}
	}
}

void Client::PrintToConsole(UserData & data)
{

	std::cout << "!!!!!!: " << data.message_from_another_client << ' ' << data.from_client_name
		<< " " << data.checking_password  << '\n';
	std::cout << "Error code " << data.error << '\n';

}

void Client::CheckPassword(UserData& data)
{
	if (data.checking_password) {

		PrintToConsole(data);
		checking_password_ = true;
	}
	error_code_ = data.error;
}

void Client::CheckNewMessage(UserData& data)
{
	error_code_ = data.error;
	if (error_code_ == 2) {
		message_for_me_ = data.message_from_another_client;
		name_message_for_me_ = data.from_client_name;
		list_messages_.push_back(std::make_pair(name_message_for_me_, message_for_me_));
		list_clients_names_.insert(name_message_for_me_);
	}
	else if (error_code_ == 4) {
		std::string mes = data.message_from_another_client;
		std::string new_name = data.to_client_name;
		list_messages_.push_back(std::make_pair("@"+new_name, mes));
		list_clients_names_.insert(another_name_);
	}
}


