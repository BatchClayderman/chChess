#include <iostream>
#include <string>
#include <vector>
#include <map>
#define WINSOCK_DEPRECATED_NO_WARNINGS 1
#include <WinSock2.h>
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
#ifndef EOF
#define EOF (-1)
#endif
#ifdef INFINITE
#define INFINITE 0xFFFFFFFF
#endif
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 0x10000
#endif
using namespace std;


enum class Description
{
	optionN, 
	optionE
};


class Board
{
private: // https://www.xqbase.com/protocol/pgnfen2.htm
	string piecePlacementData = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR";
	char activeColor = 'w';
	string castlingAvailability = "-";
	string enPassantTargetSquare = "-";
	unsigned int halfmoveClock = 0;
	unsigned int fullmoveNumber = 1;

public:
	Board(string FEN)
	{

	}
	Board(string piecePlacementData, char activeColor, string castlingAvailability, string enPassantTargetSquare, unsigned int halfmoveClock, unsigned int fullmoveNumber)
	{
		this->piecePlacementData = piecePlacementData;
		this->activeColor = activeColor;
		this->castlingAvailability = castlingAvailability;
		this->enPassantTargetSquare = enPassantTargetSquare;
		this->halfmoveClock = halfmoveClock;
		this->fullmoveNumber = fullmoveNumber;
	}
	bool isBoardValid() const
	{

	}
	Board generateNextBoard(string movement) const
	{
		return Board(this->piecePlacementData, 'w' == this->activeColor ? 'b' : 'w', "-", "-", this->halfmoveClock + 1, this->fullmoveNumber + 1);
	}
	string getFEN() const
	{
		return this->piecePlacementData + " " + this->activeColor + " " + this->castlingAvailability + " " + this->enPassantTargetSquare + " " + to_string(this->halfmoveClock) + " " + to_string(this->fullmoveNumber);
	}
};

class Record
{
private:
	vector<Board> boards{};
	
public:
	Record(Board board)
	{
		this->boards.push_back(board);
	}
	Record(string FEN)
	{
		this->boards.push_back(Board(FEN));
	}
	Record(string piecePlacementData, char activeColor, string castlingAvailability, string enPassantTargetSquare, unsigned int halfmoveClock, unsigned int fullmoveNumber)
	{
		this->boards.push_back(Board(piecePlacementData, activeColor, castlingAvailability, enPassantTargetSquare, halfmoveClock, fullmoveNumber));
	}
};

class Requests
{
public:
	bool get(string url, string* website_HTML)
	{
		string getMethodRequest = "GET / HTTP/1.1\r\nHost: " + url + "\r\nConnection: close\r\n\r\n";

		WSADATA wsaData{};
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
			return false;

		SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		hostent* host = gethostbyname(url.c_str()); // https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-gethostbyname

		SOCKADDR_IN SockAddr{};
		SockAddr.sin_port = htons(80);
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

		if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
			return false;

		send(Socket, getMethodRequest.c_str(), (int)strlen(getMethodRequest.c_str()), 0);

		char buffer[BUFFER_SIZE];
		website_HTML->clear();
		while (recv(Socket, buffer, BUFFER_SIZE, 0) > 0) {
			int i = 0;
			while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {

				website_HTML += buffer[i];
				i += 1;
			}
		}
		closesocket(Socket);
		WSACleanup();

		// display HTML source 
		cout << website_HTML;
	}
};

class Algorithm
{
private:
	int depth = 0;
	bool isTheCloudDatabaseEnabled = true;
	unsigned int halfmoveNumberToLeaveTheCloudDatabase = INFINITE;
	
	bool getTheOptimalMovementFromTheCloudDatabase(Record& record, string& bannedMovements, string& buffer) const
	{
		Requests requests = Requests();
		if (requests.get((string)"http://www.chessdb.cn/chessdb.php?action=queryall&endgame=" + (record.getTheCurrent this->halfmoveNumberToLeaveTheCloudDatabase ? "0" : "1") + "&board=", &buffer))
		{

		}
	}
	
public:
	Algorithm(int depth)
	{
		this->depth = depth;
	}
	Algorithm(int depth, bool isTheCloudDatabaseEnabled, unsigned int halfmoveNumberToLeaveTheCloudDatabase)
	{
		this->depth = depth;
		this->isTheCloudDatabaseEnabled;
		this->halfmoveNumberToLeaveTheCloudDatabase = halfmoveNumberToLeaveTheCloudDatabase;
	}
	string analyze(Record record) const
	{
		if (this->isTheCloudDatabaseEnabled)
		{
			string bannedMovements = "", buffer = "";
			if (this->getTheOptimalMovementFromTheCloudDatabase(record, bannedMovements, buffer))
				return buffer;
		}
		// TODO: Algorithm
	}
};

class Interface
{
public:
	Interface()
	{
		
	}
	void display(Board board)
	{
		return;
	}
};

class StringPool
{
private:
	const string defaultLanguage = "en";
	map<Description, map<string, string>> mappings = {
		{ Description::optionN, { { "en", "New Game" } } },
		{ Description::optionE, { { "en", "Exit" } } }
	};
	
public:
	StringPool()
	{

	}
	string getDescription(Description description, string language)
	{
		if (this->mappings.find(description) != this->mappings.end())
			if (this->mappings[description].find(language) != this->mappings[description].end())
				return this->mappings[description][language];
			else if (this->mappings[description].find(this->defaultLanguage) != this->mappings[description].end())
				return this->mappings[description][defaultLanguage];
			else
				return "";
		else
			return "";
	}
};

class Interaction
{
private:
	string language = "zh-CN";
	const map<unsigned char, Description> options = { { 'N', Description::optionN }, { 'E', Description::optionE } };
	StringPool stringPool{};
	const string normalBoard = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1";
	const string darkBoard = "xxxxkxxxx/9/1x5x1/x1x1x1x1x/9/9/X1X1X1X1X/1X5X1/9/XXXXKXXXX w - - 0 1";

public:
	Interaction()
	{
		
	}
	void printHelp() const
	{
		cout << "";
		return;
	}
	bool interact(Record record, )
	{

	}
	void printOptions()
	{
		cout << "Available options are as follows. " << endl;
		for (map<unsigned char, Description>::iterator it = this->options.begin(); it != this->options.end(); it++)
			cout << "\t" << it->first << " = " << stringPool.getDescription(it->second, this->language) << endl;
		cout << endl;
		return;
	}
	void rfStdin()
	{
		rewind(stdin);
		fflush(stdin);
	}
	unsigned char getAnOptionFromUsers()
	{
		cout << "Please select an option to continue: ";
		this->rfStdin();
		unsigned char option = 0;
		cin >> option;
		for (map<unsigned char, Description>::iterator it = this->options.begin(); it != this->options.end(); ++it)
			if (option == it->first)
				return option;
		return 0;
	}
	bool doInteraction(vector<string> arguments)
	{
		// TODO: Parse Parameters from main. 
		for (;;)
		{
			this->printOptions();
			unsigned char option = this->getAnOptionFromUsers();
			Record record = Record(normalBoard);
			Algorithm algorithmW = Algorithm(), algorithmB = Algorithm();
			interact(record, );
		}
		return EXIT_SUCCESS;
	}
};



int main(int argc, char* argv[])
{
	vector<string> arguments(argc - 1);
	for (int i = 0; i < argc - 1;)
		arguments[i] = argv[++i];
	Interaction interaction = Interaction();
	const int status = interaction.doInteraction(arguments);
	return status;
}