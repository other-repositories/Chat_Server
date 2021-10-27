
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <experimental/filesystem>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <fstream>
#include <locale>
#include <vector>
#include <map>

using namespace std;

#pragma warning(disable: 4996)

SOCKET Connections[100];
int Counter = 0;

string IP = "127.0.0.1";

string Path = "Data.json";

// Конвертация в Json
vector <pair <string, string> > Parsing_Json_In(string adress)
{
	string str, name, value;
	int k;
	bool key1 = false, key2 = false, key3 = false;
	vector < pair < string, string> > data;
	ifstream in(adress);
	while (str[0] != '}')
	{
		k = 0;
		name = "";
		value = "";
		key1 = false, key2 = false, key3 = false;
		getline(in, str);
		for (int i = 0; i != str.size(); i++)
		{
			if (str[i] == '"') { k++; }
			if (k % 2 == 1)key1 = true; else key1 = false;
			if (str[i] == ':')
			{
				key3 = true;
				if (str[i + 1] != '"')key2 = true;
			}
			if (str[i] == ',')break;
			if (!key3)
			{
				if (key1 == true) {
					if (str[i] != '"')
						name += str[i];
				}
			}
			else
			{
				if (key2 == true) {
					if (str[i] != '<' && str[i] != '>' && str[i] != '"' && str[i] != ':')
						value += str[i];
				}
				if (key1 == true) {

					if (str[i] != '<' && str[i] != '>' && str[i] != '"' && str[i] != ':')
						value += str[i];
				}
			}
		}

		if (!(name.size() == 0 || value.size() == 0))
		{
			data.push_back(make_pair(name, value));
		}

	}
	in.close();
	return data;
}

// Парсинг из Json
void Parsing_Json_Out(vector <pair <string, string> > data, string adress)
{

	ofstream  out(adress);
	out << "{" << endl;
	bool key;
	for (int i = 0; i != data.size(); i++)
	{
		out << " ";
		if (data[i].first == "id")
		{
			out << '"' << data[i].first << '"' << ':' << data[i].second;
		}
		else
		{
			out << '"' << data[i].first << '"' << ':' << '"' << data[i].second << '"';
		}
		out << "," << endl;
	}
	out << "}";
}

// Отправка файла Json сокетом
void send_file(SOCKET* sock, const string& file_name)
{
	fstream file;
	file.open(file_name, ios_base::in | ios_base::binary);
	if (file.is_open()) {
		int file_size = experimental::filesystem::file_size(file_name) +1 ;

		char* bytes = new char[file_size];
		file.read((char*)bytes, file_size);
		cout << "size: " << file_size << endl;
		cout << "name: " << file_name << endl;
		cout << "data: " << bytes << endl;

		send(*sock, to_string(file_size).c_str(), 16, 0);
		send(*sock, file_name.c_str(), 32, 0);
		send(*sock, bytes, file_size, 0);


	}
	else {
		cout << "ERORR With files" << endl;
	}
	file.close();

}

// Принятие файла Json сокетом
bool recv_file(SOCKET* sock)
{
	char file_size_str[16];
	char file_name[32];

	recv(*sock, file_size_str, 16, 0);
	int file_size = atoi(file_size_str);
	if (file_size == 0) return true;
	char* bytes = new char[file_size];

	recv(*sock, file_name, 32, 0);
	fstream file;
	file.open(file_name, ios_base::out | ios_base::binary);


	if (file.is_open()) {
		cout << "size: " << file_size << endl;
		cout << "name: " << file_name << endl;
		recv(*sock, bytes, file_size, 0);

		cout << "data: " << bytes << endl;

		file.write(bytes, file_size);
		cout << "ok save" << endl;


	}
	else
	{
		cout << "Extern_Exit" << endl;
	}

	delete[] bytes;
	file.close();
	return false;
}

// Список авторизированных пользователей
vector <int> Users_accept;

// Проверка пароля
bool check_pass = false;

// Замена id -> Ником
map < int, string > name_id;

// Проверка на возможность регистрации
string Check_login_base(string login_check, string pass_check)
{
	string login, pass, name;
	ifstream in("DataBase.txt");
	while (true)
	{
		
		in >> login;
		if (login == "$") break;
		in >> pass >> name;
		if (login == login_check && pass == pass_check) 
		{
			return name;
		}
		
	}
	in.close();
	return "$NONE";
}

// Регистрация в базе
bool Registration_login_base(string login, string pass, string nick)
{
	ifstream in("DataBase.txt");
	string login_t, pass_t, nick_t;
	vector < pair < string, pair < string,string> > > dates;
	while (true)
	{
		in >> login_t >> pass_t >> nick_t;
		if (login_t == "$") break;
		dates.push_back(make_pair(login_t, make_pair(pass_t, nick_t)));
	}
	ofstream out("DataBase.txt");
	bool check=true;

	for (int i = 0; i != dates.size(); i++) 
	{
		if (dates[i].first == login || dates[i].second.second == nick)check = false;
		out << dates[i].first << " " << dates[i].second.first << " " << dates[i].second.second << endl;

	}
	if (check) {
		out << login<<" "<<pass<<" "<< nick << endl;
	}
	out << "$";
	in.close();
	out.close();
	return check;

}

// Проверка логина
map < int, int > check_login;

// Центр сервера
void ClientHandler(int index) {
	int msg_size=-1;
	while (true) {
		
		int Errors_sock = send(Connections[index], "s ", 0, 1);
		Errors_sock = send(Connections[index], "s ", 0, 1);
		if (Errors_sock == -1)
		{
			cout << "User " << index << ": "<< name_id.find(index)->second << " leave" << endl;
			break;
		}
		else {

			if (!recv_file(&Connections[index])) {
				vector < pair <string, string > > data;
				data = Parsing_Json_In(Path);
				if (data.size() > 0)
				{
					if (data[0].first == "command")
					{
						if (data[0].second == "login")
						{
							check_pass = false;
							string login, pass;
							if (data[1].first == "login") {
								login = data[1].second;
								if (data[2].first == "password") {
									pass = data[2].second;

									string name = Check_login_base(login, pass);

									if (name !="$NONE" ) {

										check_login[index] = 1;
										name_id[index] = name;

										cout << "user " << index << "  login accept" << endl;
										Users_accept.push_back(index);
										data.clear();
										data.push_back(make_pair("command", "login"));
										data.push_back(make_pair("status", "ok"));
										data.push_back(make_pair("session", IP));
										Parsing_Json_Out(data, Path);
										data.clear();  Sleep(30);
										send_file(&Connections[index], Path);
									}
									else {
										check_pass = true;
									}
								}
								else {
									check_pass = true;
								}
							}
							else {
								check_pass = true;
							}

							if (check_pass == true) {

								data.clear();
								data.push_back(make_pair("command", "login"));
								data.push_back(make_pair("status", "failed"));
								data.push_back(make_pair("message", "Wrong login or password"));
								Parsing_Json_Out(data, Path);
								data.clear();  Sleep(30);
								send_file(&Connections[index], Path);
							}

						}
						
						if (data[0].second == "message")
						{
							if (data[1].first == "body") 
							{
								string message = data[1].second;
								if (check_login.find(index)->second == 1)
								{

									if (data[2].first == "session")
									{
										if (data[2].second == IP)
										{
										
											data.clear();
										    data.push_back(make_pair("command", "message_reply"));
										    data.push_back(make_pair("status", "ok"));
										    data.push_back(make_pair("message", "last"));
										    Parsing_Json_Out(data, Path);
									    	data.clear();  Sleep(30);
								     		send_file(&Connections[index], Path);

											data.clear();
											
											data.push_back(make_pair("command", "message"));
											data.push_back(make_pair("body", message ));
											data.push_back(make_pair("sender_login", name_id.find(index)->second));
											Parsing_Json_Out(data, Path);
											data.clear();
											for (int i = 0; i != Users_accept.size(); i++)
											{
												if (Users_accept[i] != index) {
													send_file(&Connections[Users_accept[i]], Path);
												}
											}

										}
									}
								}
								else
								{
									data.clear();
									data.push_back(make_pair("command", "message_reply"));
									data.push_back(make_pair("status", "failed"));
									data.push_back(make_pair("message", "User not login"));
									Parsing_Json_Out(data, Path);
									data.clear();  Sleep(30);
									send_file(&Connections[index], Path);
								}
							}
						}

						if (data[0].second == "register")
						{
							if(data[1].first=="login")
							   if(data[2].first=="password")
								   if (data[3].first == "nickname")
								   {
									   if (Registration_login_base(data[1].second, data[2].second, data[3].second))
									   {
										   data.clear();
										   data.push_back(make_pair("command", "register_reply"));
										   data.push_back(make_pair("status", "ok"));
										   data.push_back(make_pair("message", "Account created"));
										   Parsing_Json_Out(data, Path);
										   data.clear();  Sleep(30);
										   send_file(&Connections[index], Path);
									   }
									   else 
									   {
										   data.clear();
										   data.push_back(make_pair("command", "register_reply"));
										   data.push_back(make_pair("status", "failed"));
										   data.push_back(make_pair("message", "Warning -> Login or Nickname uses another person"));
										   Parsing_Json_Out(data, Path);
										   data.clear();  Sleep(30);
										   send_file(&Connections[index], Path);

									   }
								   }

						}
						
						if (data[0].second == "HELLO")
						{
							data.clear();
							data.push_back(make_pair("command", "HELLO"));
							data.push_back(make_pair("authmethod", "Connected"));
	
							Parsing_Json_Out(data, Path);
							data.clear();  Sleep(30);
							send_file(&Connections[index], Path);
						}

					}
					else {

					cout << "Client error-> "<< index<<endl;

					}

					


				}


				// Парсинг Json
				//send_file(&Connections[index], "1.jpg");

			}

		}
	}
}

// Входящие подключения
int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");
	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error with version" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	SOCKET newConnection;
	for (int i = 0; i < 100; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == 0) {
			std::cout << "Error with connect"<<endl;
		}
		else {
			
			Connections[i] = newConnection;
			Counter++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
		}
	}


	system("pause");
	return 0;
}
