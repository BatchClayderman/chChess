#include <iostream>
#include <fstream>
#include <vector>
#if !defined _WIN32 && !defined _WIN64
#include <algorithm>
#endif
#include <random>
#include <thread>
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
#ifndef EOF
#define EOF (-1)
#endif
#ifndef UNREFERENCED_PARAMETER
#if defined _WIN32 || defined _WIN64
#define UNREFERENCED_PARAMETER(P) (P)
#else
#define UNREFERENCED_PARAMETER(P)
#endif
#endif
using namespace std;
typedef unsigned char HelpKey;
typedef unsigned char Value;
typedef unsigned char Point;
typedef unsigned char Player;
typedef unsigned char Count;
constexpr long long int TIME_FOR_SLEEP = 3;
constexpr size_t BUFFER_SIZE = 1024;
constexpr Point JOKER_POINT = 0;
constexpr Player INVALID_PLAYER = (Player)(-1);


enum class SortingMethod
{
	FromBigToSmall = 0,
	FromSmallToBig = 1,
	FromManyToFew = 2,
	FromFewToMany = 3
};

enum class Suit
{
	Diamond = 0, 
	Club = 1, 
	Heart = 2, 
	Spade = 3, 
	Black = 4, 
	Red = 5, 
	Cover = 6
};

enum class TokenType
{
	Empty = 0b00000000, 
	
	Single = 0b00010000, 
	Straight = 0b00010001, 
	Flush = 0b00010010, 
	FlushStraight = 0b00010011, 
	
	Pair = 0b00100000, 
	PairStraight = 0b00100001, 
	PairStraightWithSingle = 0b00100010, 
	PairJokers = 0b00100011, 
	
	Triple = 0b00110000, 
	TripleWithSingle = 0b00110001, 
	TripleWithPair = 0b00110010, 
	TripleWithPairSingle = 0b00110011, 
	TripleStraight = 0b00110100, 
	TripleStraightWithSingle = 0b00110101, 
	TripleStraightWithSingles = 0b00110110, 
	TripleStraightWithPairs = 0b00110111, 
	
	Quadruple‌ = 0b01000000, 
	Quadruple‌WithSingle = 0b01000001, 
	Quadruple‌WithSingleSingle = 0b01000010, 
	Quadruple‌WithPairPair = 0b01000011, 
	QuadrupleJokers = 0b01000100, 
	
	Quintuple = 0b01010000, 
	Sextuple = 0b01100000, 
	Septuple = 0b01110000, 
	Octuple = 0b10000000, 
	
	Invalid = 0b11111111
};

enum class Status
{
	Ready = 0, 
	Initialized = 1, 
	Dealt = 2, 
	Assigned = 3, 
	Started = 4, 
	Over = 5
};

enum class LandlordScore
{
	None = 0, 
	One = 1, 
	Two = 2, 
	Three = 3
};


struct Card
{
	Point point = JOKER_POINT; // JOKER_POINT (0) is for the Jokers, the Cover Card, and the default value. 
	Suit suit = Suit::Cover;

	friend bool operator==(const Card& a, const Card& b)
	{
		return a.point == b.point && a.suit == b.suit;
	}
	operator string() const
	{
		string stringBuffer = "";
		switch (this->suit)
		{
		case Suit::Diamond:
			stringBuffer += "方块";
			break;
		case Suit::Club:
			stringBuffer += "梅花";
			break;
		case Suit::Heart:
			stringBuffer += "红桃";
			break;
		case Suit::Spade:
			stringBuffer += "黑桃";
			break;
		case Suit::Black:
			stringBuffer += "小";
			break;
		case Suit::Red:
			stringBuffer += "大";
			break;
		default:
			return "广告牌";
		}
		switch (this->point)
		{
		case 1:
			stringBuffer += "A";
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			stringBuffer += this->point + '0';
			break;
		case 10:
			stringBuffer += "10";
			break;
		case 11:
			stringBuffer += "J";
			break;
		case 12:
			stringBuffer += "Q";
			break;
		case 13:
			stringBuffer += "K";
			break;
		case JOKER_POINT:
			stringBuffer += "王";
			break;
		default:
			break;
		}
		return stringBuffer;
	}
};

struct Token
{
	Player player = INVALID_PLAYER;
	vector<Card> cards{};
	TokenType tokenType = TokenType::Invalid;
};


class Values
{
private:
	Value values[14] = { 0 };

public:
	Values()
	{
		
	}
	bool set(const Point point, const Value value)
	{
		if (0 <= point && point <= 13 && 1 <= value && value <= 14)
		{
			this->values[point] = value;
			return true;
		}
		else
			return false;
	}
	bool get(const Point point, Value& value) const
	{
		if (0 <= point && point <= 13)
		{
			value = this->values[point];
			return true;
		}
		else
			return false;
	}
	Value operator[](const Point point) const
	{
		return 0 <= point && point <= 13 ? this->values[point] : 0;
	}
};

class PokerGame
{
protected:
	mt19937 seed{};
	string pokerType = "扑克牌";
	Values values{};
	vector<vector<Card>> players{};
	vector<Card> deck{};
	vector<vector<Token>> records{};
	Player currentPlayer = INVALID_PLAYER;
	Token* pLastToken = nullptr;
	Status status = Status::Ready;
	
private:
	virtual void add52CardsToDeck(vector<Card>& _deck) const final
	{
		for (Point point = 1; point <= 13; ++point)
		{
			_deck.push_back(Card{ point, Suit::Diamond });
			_deck.push_back(Card{ point, Suit::Club });
			_deck.push_back(Card{ point, Suit::Heart });
			_deck.push_back(Card{ point, Suit::Spade });
		}
		return;
	}
	virtual void add54CardsToDeck(vector<Card>& _deck) const final
	{
		this->add52CardsToDeck(_deck);
		_deck.push_back(Card{ JOKER_POINT, Suit::Black });
		_deck.push_back(Card{ JOKER_POINT, Suit::Red });
		return;
	}
	
protected:
	/* PokerGame::deal */
	virtual void add52CardsToDeck() final
	{
		this->add52CardsToDeck(this->deck);
		return;
	}
	virtual void add54CardsToDeck() final
	{
		this->add54CardsToDeck(this->deck);
		return;
	}
	virtual void sortCards(vector<Card>& cards, SortingMethod sortingMethod) const final
	{
		switch (sortingMethod)
		{
		case SortingMethod::FromSmallToBig:
			sort(cards.begin(), cards.end(), [this](Card a, Card b) { return this->values[a.point] < this->values[b.point] || (this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
			break;
		case SortingMethod::FromManyToFew:
		{
			Count counts[14] = { 0 };
			for (const Card& card : cards)
				++counts[card.point];
			sort(cards.begin(), cards.end(), [&counts, this](Card a, Card b) { return counts[a.point] > counts[b.point] || (counts[a.point] == counts[b.point] && this->values[a.point] > this->values[b.point]) || (counts[a.point] == counts[b.point] && this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
			break;
		}
		case SortingMethod::FromFewToMany:
		{
			Count counts[14] = { 0 };
			for (const Card& card : cards)
				++counts[card.point];
			sort(cards.begin(), cards.end(), [&counts, this](Card a, Card b) { return counts[a.point] < counts[b.point] || (counts[a.point] == counts[b.point] && this->values[a.point] > this->values[b.point]) || (counts[a.point] == counts[b.point] && this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
			break;
		}
		case SortingMethod::FromBigToSmall:
		default:
			sort(cards.begin(), cards.end(), [this](Card a, Card b) { return this->values[a.point] > this->values[b.point] || (this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
			break;
		}
		return;
	}
	virtual void sortCards(vector<Card>& cards) const final
	{
		this->sortCards(cards, SortingMethod::FromBigToSmall);
		return;
	}
	virtual bool assignDealer()
	{
		if (Status::Dealt == this->status && this->records.empty())
		{
			this->records.push_back(vector<Token>{});
			const size_t playerCount = this->players.size();
			for (Player player = 0; player < playerCount; ++player)
				this->records[0].push_back(Token{ player, vector<Card>{ this->players[player].back() } });
			sort(this->records[0].begin(), this->records[0].end(), [this](Token a, Token b) { return this->values[a.cards.back().point] > this->values[b.cards.back().point] || (this->values[a.cards.back().point] == this->values[b.cards.back().point] && a.cards.back().suit > b.cards.back().suit); });
			this->currentPlayer = this->records[0].back().player;
			this->pLastToken = &this->records[0].back();
			this->status = Status::Assigned;
			return true;
		}
		else
			return false;
	}
	
	/* PokerGame::setLandlord */
	virtual bool nextPlayer()
	{
		const size_t playerCount = this->players.size();
		if (this->currentPlayer < playerCount)
		{
			if (++this->currentPlayer >= playerCount) // This works correctly even though player is 255. 
				this->currentPlayer = 0;
			return true;
		}
		else
			return false;
	}
	
	/* PokerGame::start and PokerGame::play */
	virtual bool description2cards(const string& description, vector<Card>& cards) const final
	{
		if ("/" == description || "\\" == description || "-" == description || "--" == description || "要不起" == description || "不出" == description || "不打" == description)
		{
			cards.clear();
			return true;
		}
		else if (0 <= this->currentPlayer && this->currentPlayer < this->players.size() && !this->players[this->currentPlayer].empty())
		{
			vector<size_t> selected{};
			vector<Card> exactCards{};
			vector<Point> fuzzyPoints{};
			vector<Suit> fuzzySuits{};
			bool waitingForAPoint = false;
			Suit suit = Suit::Diamond;
			const size_t descriptionLength = description.length();
			for (size_t idx = 0; idx < descriptionLength; ++idx)
			{
				switch (description.at(idx))
				{
				case 'A':
				case 'a':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 1, suit });
					else
						fuzzyPoints.push_back(1);
					break;
				case '1':
					if (idx + 1 < descriptionLength && '0' == description.at(idx + 1))
					{
						if (waitingForAPoint)
							exactCards.push_back(Card{ 10, suit });
						else
							fuzzyPoints.push_back(10);
						++idx;
					}
					else if (waitingForAPoint)
						exactCards.push_back(Card{ 1, suit });
					else
						fuzzyPoints.push_back(1);
					break;
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					if (waitingForAPoint)
						exactCards.push_back(Card{ (Point)(description.at(idx) - '0'), suit });
					else
						fuzzyPoints.push_back(description.at(idx) - '0');
					break;
				case 'T':
				case 't':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 10, suit });
					else
						fuzzyPoints.push_back(10);
				case 'J':
				case 'j':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 11, suit });
					else
						fuzzyPoints.push_back(11);
					break;
				case 'Q':
				case 'q':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 12, suit });
					else
						fuzzyPoints.push_back(12);
					break;
				case 'K':
				case 'k':
					if (waitingForAPoint)
						exactCards.push_back(Card{ 13, suit });
					else
						fuzzyPoints.push_back(13);
					break;
				case 'L':
				case 'l':
					if (waitingForAPoint)
						fuzzySuits.push_back(suit);
					else
						exactCards.push_back(Card{ JOKER_POINT, Suit::Black });
					break;
				case 'B':
				case 'b':
					if (waitingForAPoint)
						fuzzySuits.push_back(suit);
					else
						exactCards.push_back(Card{ JOKER_POINT, Suit::Black });
					break;
				default:
				{
					if (waitingForAPoint)
					{
						fuzzySuits.push_back(suit);
						waitingForAPoint = false;
					}
					const string str = description.substr(idx, 4);
					if ("方块" == str)
					{
						suit = Suit::Diamond;
						waitingForAPoint = true;
						idx += 3;
					}
					else if ("梅花" == str)
					{
						suit = Suit::Club;
						waitingForAPoint = true;
						idx += 3;
					}
					else if ("红桃" == str || "红心" == str)
					{
						suit = Suit::Heart;
						waitingForAPoint = true;
						idx += 3;
					}
					else if ("黑桃" == str)
					{
						suit = Suit::Spade;
						waitingForAPoint = true;
						idx += 3;
					}
					else if ("小王" == str || "小鬼" == str)
					{
						suit = Suit::Black;
						exactCards.push_back(Card{ JOKER_POINT, Suit::Black });
						idx += 3;
					}
					else if ("大王" == str || "大鬼" == str)
					{
						suit = Suit::Red;
						exactCards.push_back(Card{ JOKER_POINT, Suit::Red });
						idx += 3;
					}
					break;
				}
				}
			}
			if (waitingForAPoint)
				fuzzySuits.push_back(suit);
			const size_t length = this->players[this->currentPlayer].size();
			size_t position = 0;
			for (const Card& card : exactCards) // select the rightmost one
			{
				bool flag = false;
				for (size_t idx = 0; idx < length; ++idx)
					if (this->values[this->players[this->currentPlayer][idx].point] > this->values[card.point])
						continue;
					else if (this->players[this->currentPlayer][idx] == card && find(selected.begin(), selected.end(), idx) == selected.end())
					{
						position = idx;
						flag = true;
					}
					else
						break;
				if (flag)
					selected.push_back(position);
				else
					return false;
			}
			for (const Point& point : fuzzyPoints) // search for the smallest suit that is not selected for each point to select
			{
				bool flag = false;
				for (size_t idx = 0; idx < length; ++idx)
					if (this->values[this->players[this->currentPlayer][idx].point] > this->values[point])
						continue;
					else if (this->players[this->currentPlayer][idx].point == point)
					{
						if (find(selected.begin(), selected.end(), idx) == selected.end())
						{
							position = idx;
							flag = true;
						}
					}
					else
						break;
				if (flag)
					selected.push_back(position);
				else
					return false;
			}
			for (const Suit& s : fuzzySuits) // select from right to left
			{
				bool flag = false;
				for (size_t idx = length - 1; idx > 0; --idx)
					if (this->players[this->currentPlayer][idx].suit == s && find(selected.begin(), selected.end(), idx) == selected.end())
					{
						position = idx;
						flag = true;
						break;
					}
				if (flag)
					selected.push_back(position);
				else if (this->players[this->currentPlayer][0].suit == s) // avoid (size_t)(-1)
					selected.push_back(0);
				else
					return false;
			}
			cards.clear();
			for (const size_t& p : selected)
				cards.push_back(this->players[this->currentPlayer][p]);
			return true;
		}
		else
			return false;
	}
	virtual bool checkStarting(const vector<Card>& cards) const
	{
		return !cards.empty() && (this->records[0].back().cards.size() == 1 && find(cards.begin(), cards.end(), this->records[0].back().cards[0]) != cards.end());
	}
	virtual bool processToken(Token& token) const = 0;
	virtual bool removeCards(const vector<Card>& _smallerCards, vector<Card>& largerCards) const final // The vector ``largerCards`` must have been sorted according to the default sorting method. 
	{
		vector<Card> smallerCards = vector<Card>(_smallerCards);
		this->sortCards(smallerCards);
		const size_t smallerLength = smallerCards.size(), largerLength = largerCards.size();
		if (smallerLength > largerLength || 0 == largerLength)
			return false;
		else if (smallerLength >= 1)
		{
			vector<size_t> selected{};
			for (size_t smallerIndex = 0, largerIndex = 0; smallerIndex < smallerLength && largerIndex < largerLength; ++largerIndex)
				if (smallerCards[smallerIndex] == largerCards[largerIndex])
				{
					selected.push_back(largerIndex);
					++smallerIndex;
				}
			if (selected.size() == smallerLength)
			{
				for (size_t idx = smallerLength - 1; idx > 0; --idx)
					largerCards.erase(largerCards.begin() + selected[idx]);
				largerCards.erase(largerCards.begin() + selected[0]);
				return true;
			}
			else
				return false;
		}
		else
			return true;
	}
	virtual bool isOver() const
	{
		if (this->status >= Status::Started)
			for (const vector<Card>& cards : this->players)
				if (cards.empty())
					return true;
		return false;
	}
	virtual bool coverLastToken(const Token& currentToken) const = 0;
	
	/* PokerGame::display */
	virtual string cards2string(const vector<Card>& cards, const string& prefix, const string& separator, const string& suffix, const string& returnIfEmpty) const final
	{
		if (cards.empty())
			return returnIfEmpty;
		else
		{
			string stringBuffer = prefix + (string)cards[0];
			size_t length = cards.size();
			for (size_t cardID = 1; cardID < length; ++cardID)
				stringBuffer += separator + (string)cards[cardID];
			stringBuffer += suffix;
			return stringBuffer;
		}
	}
	virtual string getPreRoundString() const = 0;
	virtual void display(const string& dealerRemark, const string& deckDescription) const final
	{
		switch (this->status)
		{
		case Status::Ready:
			cout << "牌局未初始化，请先初始化牌局。" << endl << endl;
			break;
		case Status::Initialized:
			cout << "当前牌局（" << this->pokerType << "）已初始化，但暂未开局，请发牌或录入残局数据。" << endl << endl;
			break;
		case Status::Dealt:
		case Status::Assigned:
		case Status::Started:
		case Status::Over:
		{
			cout << "牌局：" << this->pokerType << "（";
			switch (this->status)
			{
			case Status::Dealt:
				cout << "已发牌";
				break;
			case Status::Assigned:
				cout << "等待开牌";
				break;
			case Status::Started:
				cout << "正在游戏";
				break;
			case Status::Over:
				cout << "已结束";
				break;
			default:
				break;
			}
			cout << "）" << endl << endl;
			const size_t playerCount = this->players.size();
			for (Player player = 0; player < playerCount; ++player)
				cout << "玩家 " << (player + 1) << (!this->records.empty() && !this->records[0].empty() && this->records[0].back().player == player ? "（" + dealerRemark + "）" : "：") << endl << this->cards2string(this->players[player], "", " | ", "", "（空）") << endl << endl;
			cout << deckDescription;
			if (this->status >= Status::Assigned && !this->records.empty())
			{
				cout << "出牌记录：" << endl;
				cout << "预备回合：" << this->getPreRoundString() << endl;
				const size_t roundCount = this->records.size();
				for (size_t round = 1; round < roundCount; ++round)
				{
					cout << "第 " << round << " 回合：";
					if (this->records[round].empty())
						cout << "（无）" << endl;
					else
					{
						cout << this->cards2string(this->records[round][0].cards, "", "+", "", "要不起") << "（玩家 " << (this->records[round][0].player + 1) << "）";
						const size_t tokenCount = this->records[round].size();
						for (size_t tokenID = 1; tokenID < tokenCount; ++tokenID)
							cout << " -> " << this->cards2string(this->records[round][tokenID].cards, "", "+", "", "要不起") << "（玩家 " << (this->records[round][tokenID].player + 1) << "）";
						cout << endl;
					}
				}
				cout << endl;
			}
			break;
		}
		default:
			cout << "当前牌局状态未知，无法显示牌局状况。" << endl << endl;
			break;
		}
		return;
	}
	
public:
	PokerGame() // seed, pokerType, and status = Status::Ready
	{
		random_device rd;
		mt19937 g(rd());
		this->seed = g;
	}
	virtual ~PokerGame()
	{
		
	}
	virtual bool initialize() = 0; // values, players (= vector<vector<Card>>(n)), deck (clear), records (clear), currentPlayer (reset), pLastToken = nullptr, and status = Status::Initialized
	virtual bool initialize(const size_t playerCount) = 0; // values, players (= vector<vector<Card>>(n)), deck (clear), records (clear), currentPlayer (reset), pLastToken = nullptr, and status = Status::Initialized
	virtual bool deal() = 0; // players, deck, records (clear) -> records[0] (clear), currentPlayer, pLastToken, and status = Status::Dealt | Status::Assigned
	virtual bool getCurrentPlayer(Player& player) const final // const
	{
		if (Status::Dealt <= this->status && this->status <= Status::Started && 0 <= this->currentPlayer && this->currentPlayer < this->players.size())
		{
			player = this->currentPlayer;
			return true;
		}
		else
			return false;
	}
	virtual bool setLandlord(const bool b) { UNREFERENCED_PARAMETER(b); return false; } // records[0], currentPlayer, pLastToken, and status = Status::Assigned
	virtual bool setLandlord(const LandlordScore landlordScore) { UNREFERENCED_PARAMETER(landlordScore); return false; } // records[0], currentPlayer, pLastToken, and status = Status::Assigned
	virtual bool start(const vector<Card>& cards) final // records, currentPlayer, pLastToken, and status = Status::Started
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty() && 0 <= this->currentPlayer && this->currentPlayer < this->players.size() && this->checkStarting(cards))
		{
			Token token{ this->currentPlayer, cards };
			if (this->processToken(token) && this->removeCards(cards, this->players[this->currentPlayer]))
			{
				this->records.push_back(vector<Token>{ token });
				if (this->isOver())
				{
					this->currentPlayer = INVALID_PLAYER;
					this->pLastToken = nullptr;
					this->status = Status::Over;
				}
				else
				{
					this->nextPlayer();
					this->pLastToken = &this->records[1][0];
					this->status = Status::Started;
				}
				return true;
			}
			else
				return false;
		}
		else
			return false;

	}
	virtual bool start(const string& description) final // records
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty())
		{
			vector<Card> cards{};
			return this->description2cards(description, cards) && this->start(cards);
		}
		else
			return false;
	}
	virtual bool play(const vector<Card>& cards) final // records
	{
		if (Status::Started == this->status && this->records.size() >= 2 && !this->records.back().empty() && 0 <= this->currentPlayer && this->currentPlayer < this->players.size() && this->pLastToken != nullptr)
		{
			Token token{ this->currentPlayer, cards };
			if (this->processToken(token))
			{
				if (token.player == this->pLastToken->player)
					if (TokenType::Empty != token.tokenType && this->removeCards(cards, this->players[this->currentPlayer]))
					{
						if (this->coverLastToken(token))
							this->records.back().push_back(token);
						else
							this->records.push_back(vector<Token>{ token });
						this->nextPlayer();
						this->pLastToken = &this->records.back().back();
						return true;
					}
					else
						return false;
				else if (TokenType::Empty == token.tokenType)
				{
					this->records.back().push_back(token);
					this->nextPlayer();
					return true;
				}
				else if (this->coverLastToken(token) && this->removeCards(token.cards, this->players[this->currentPlayer]))
				{
					this->records.back().push_back(token);
					this->nextPlayer();
					this->pLastToken = &this->records.back().back();
					return true;
				}
				else
					return false;
			}
			else
				return false;
		}
		else
			return false;
	}
	virtual bool play(const string& description) final // records
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty())
		{
			vector<Card> cards{};
			return this->description2cards(description, cards) && this->play(cards);
		}
		else
			return false;
	}
	virtual bool set(const char binaryChars[]) final // values, players, deck, records, currentPlayer, pLastToken, and status
	{
		const size_t length = sizeof(binaryChars), playerCount = this->players.size();
		vector<vector<Card>> newPlayers(playerCount);
		for (size_t idx = 0; idx < length; ++idx)
		{
			char keyChar = 0, valueBuffer[BUFFER_SIZE] = { 0 };
			size_t valueIdx = 0;
			while (idx < length && '\t' != binaryChars[idx])
				keyChar = binaryChars[idx++];
			if (length >= idx)
				return false;
			while (idx < length && '\n' != binaryChars[idx])
				if (valueIdx + 1 >= BUFFER_SIZE)
					return false;
				else
					valueBuffer[valueIdx] = binaryChars[idx++];
			switch (keyChar)
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				const char playerIdx = keyChar - '0';
				if (0 <= playerIdx && playerIdx < playerCount && this->description2cards(valueBuffer, this->players[0]))
					break;
				else
					return false;
			}
			case 'D':
			case 'd':
				if (this->deal())
					break;
				else
					return false;
			case 'A':
			case 'a':
				if (strlen(valueBuffer) >= 1)
					if ('0' == valueBuffer[0])
						if (this->setLandlord(false))
							break;
						else
							return false;
					else if (this->setLandlord(true))
						break;
					else
						return false;
				else
					return false;
			case 'L':
			case 'l':
				if (strlen(valueBuffer) >= 1)
				{
					LandlordScore landlordScore = LandlordScore::None;
					switch (valueBuffer[0])
					{
					case '1':
						landlordScore = LandlordScore::One;
						break;
					case '2':
						landlordScore = LandlordScore::Two;
						break;
					case '3':
						landlordScore = LandlordScore::Three;
						break;
					default:
						break;
					}
					if (this->setLandlord(landlordScore))
						break;
					else
						return false;
				}
				else
					return false;
			case 'S':
			case 's':
				if (this->start(valueBuffer))
					break;
				else
					return false;
			case 'P':
			case 'p':
				if (this->play(valueBuffer))
					break;
				else
					return false;
			}
		}
		return true;
	}
	virtual void display() const = 0; // const
};

class Landlords : public PokerGame /*  Next: Landlords4P */
{
private:
	bool assignDealer() override final
	{
		if (Status::Dealt == this->status && this->records.empty())
		{
			this->records.push_back(vector<Token>{});
			uniform_int_distribution<size_t> distribution(0, this->players.size() - 1);
			this->currentPlayer = (Player)(distribution(this->seed));
			this->pLastToken = nullptr;
			return true;
		}
		else
			return false;
	}
	bool checkStarting(const vector<Card>& cards) const override final
	{
		return !cards.empty();
	}
	/*bool processToken(Token& token) const override final
	{
		return true;
	}
	bool coverLastToken(const Token& currentToken) const override final
	{
		return true;
	}*/
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			Count callerAndRobberCount = 0;
			const size_t length = this->records[0].size();
			for (size_t idx = 0; idx < length; ++idx)
				if (!this->records[0][idx].cards.empty())
					++callerAndRobberCount;
			char playerBuffer[4] = { 0 };
			switch (callerAndRobberCount)
			{
			case 0:
				snprintf(playerBuffer, 4, "%d", (this->records[0][0].player + 1));
				return "无人叫地主，强制玩家 " + (string)playerBuffer + " 为地主。";
			case 1:
			case 2:
			case 3:
			case 4:
			{
				string preRoundString = "";
				bool isRobbing = false;
				for (size_t idx = 0; idx < length; ++idx)
				{
					snprintf(playerBuffer, 4, "%d", this->records[0][idx].player + 1);
					if (this->records[0][idx].cards.empty())
						preRoundString += (isRobbing ? "不抢（玩家 " : "不叫（玩家 ") + (string)playerBuffer + "） -> ";
					else if (isRobbing)
						preRoundString += "抢地主（玩家 " + (string)playerBuffer + "） -> ";
					else
					{
						preRoundString += "叫地主（玩家 " + (string)playerBuffer + "） -> ";
						isRobbing = true;
					}
				}
				preRoundString.erase(preRoundString.length() - 4, 4);
				return preRoundString;
			}
			default:
				return "预备回合信息检验异常。";
			}
		}
	}
	
public:
	Landlords() : PokerGame()
	{
		this->pokerType = "斗地主";
	}
	bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->values.set(JOKER_POINT, value++);
			this->players = vector<vector<Card>>(3);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->pLastToken = nullptr;
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	bool initialize(const size_t playerCount) override final { return 3 == playerCount && this->initialize(); }
	bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < 3; ++player)
			{
				this->players[player] = vector<Card>(17);
				for (size_t idx = 0; idx < 17; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	bool setLandlord(const bool b) override final
	{
		if (Status::Dealt == this->status && this->records.size() == 1 && 0 <= this->currentPlayer && this->currentPlayer < this->players.size())
			switch (this->records[0].size())
			{
			case 0:
				if (b)
				{
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{} } });
					this->pLastToken = &this->records[0][0];
				}
				else
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
				this->nextPlayer();
				return true;
			case 1:
				if (b)
				{
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{} } });
					if (nullptr == this->pLastToken)
						this->pLastToken = &this->records[0][1];
				}
				else
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
				this->nextPlayer();
				return true;
			case 2:
			{
				this->records[0].push_back(Token{ this->currentPlayer, b ? vector<Card>{ Card{} } : vector<Card>{} });
				Count callerAndRobberCount = 0;
				for (size_t idx = 0; idx < 3; ++idx)
					if (!this->records[0][idx].cards.empty())
						++callerAndRobberCount;
				switch (callerAndRobberCount)
				{
				case 0:
					this->currentPlayer = this->records[0][0].player;
					this->pLastToken = &this->records[0][0];
					this->status = Status::Assigned;
					return true;
				case 1:
					this->currentPlayer = this->pLastToken->player;
					this->status = Status::Assigned;
					return true;
				case 2:
				case 3:
					this->currentPlayer = this->pLastToken->player;
					return true;
				default:
					return false;
				}
			}
			case 3:
				this->records[0].push_back(Token{ this->currentPlayer, b ? vector<Card>{ Card{} } : vector<Card>{} });
				this->pLastToken = nullptr;
				for (size_t idx = 3; idx > 0; --idx)
					if (!this->records[0][idx].cards.empty())
					{
						this->currentPlayer = this->records[0][idx].player;
						this->pLastToken = &this->records[0][idx];
					}
				if (this->pLastToken)
				{
					this->currentPlayer = this->records[0][0].player;
					this->pLastToken = &this->records[0][0];
				}
				this->status = Status::Assigned;
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	void display() const override final
	{
		if (this->status >= Status::Assigned)
			PokerGame::display("地主", "地主牌：" + this->cards2string(this->deck, "", " | ", "（已公开）", "（空）") + "\n\n");
		else
			PokerGame::display("拥有明牌", "地主牌：" + this->cards2string(this->deck, "", " | ", "（未公开）", "（空）") + "\n\n");
		return;
	}
};

class Landlords4P : public PokerGame /* Previous: Landlords | Next: BigTwo */
{
private:
	bool assignDealer() override final
	{
		if (Status::Dealt == this->status && this->records.empty())
		{
			this->records.push_back(vector<Token>{});
			uniform_int_distribution<size_t> distribution(0, this->players.size() - 1);
			this->currentPlayer = (Player)(distribution(this->seed));
			this->pLastToken = nullptr;
			return true;
		}
		else
			return false;
	}
	bool checkStarting(const vector<Card>& cards) const override final
	{
		return !cards.empty();
	}
	/*bool processToken(Token& token) const override final
	{
		return true;
	}
	bool coverLastToken(const Token& currentToken) const override final
	{
		return true;
	}*/
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			Count callerAndRobberCount = 0;
			const size_t length = this->records[0].size();
			for (size_t idx = 0; idx < length; ++idx)
				if (this->records[0][idx].cards.size() == 1 && this->records[0][idx].cards[0].point)
					++callerAndRobberCount;
			char playerBuffer[4] = { 0 };
			switch (callerAndRobberCount)
			{
			case 0:
				snprintf(playerBuffer, 4, "%d", (this->records[0][0].player + 1));
				return "无人叫地主，强制玩家 " + (string)playerBuffer + " 为地主。";
			case 1:
			case 2:
			case 3:
			{
				string preRoundString = "";
				for (const Token& token : this->records[0])
				{
					snprintf(playerBuffer, 4, "%d", token.player + 1);
					switch (token.cards.size())
					{
					case 0:
						preRoundString += "不叫（玩家 " + (string)playerBuffer + "） -> ";
						break;
					case 1:
						switch (token.cards[0].point)
						{
						case 0:
							preRoundString += "不叫（玩家 " + (string)playerBuffer + "） -> ";
							break;
						case 1:
						case 2:
						case 3:
						{
							char landlordScoreBuffer[4] = { 0 };
							snprintf(landlordScoreBuffer, 4, "%d", token.cards[0].point);
							preRoundString += (string)landlordScoreBuffer + "分（玩家 " + playerBuffer + "） -> ";
							break;
						}
						default:
							return "预备回合信息检验异常。";
						}
						break;
					default:
						return "预备回合信息检验异常。";
					}
				}
				preRoundString.erase(preRoundString.length() - 4, 4);
				return preRoundString;
			}
			default:
				return "预备回合信息检验异常。";
			}
		}
	}
	
public:
	Landlords4P() : PokerGame()
	{
		this->pokerType = "四人斗地主";
	}
	bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->values.set(JOKER_POINT, value++);
			this->players = vector<vector<Card>>(4);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->pLastToken = nullptr;
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	bool initialize(const size_t playerCount) override final { return 3 == playerCount && this->initialize(); }
	bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add54CardsToDeck();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < 4; ++player)
			{
				this->players[player] = vector<Card>(25);
				for (size_t idx = 0; idx < 25; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	bool setLandlord(const LandlordScore landlordScore) override final
	{
		const Point point = static_cast<Point>(landlordScore);
		if (Status::Dealt == this->status && this->records.size() == 1 && 0 <= this->currentPlayer && this->currentPlayer < this->players.size())
			switch (this->records[0].size())
			{
			case 0:
				switch (landlordScore)
				{
				case LandlordScore::None:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
					this->nextPlayer();
					this->pLastToken = &this->records[0][0];
					return true;
				case LandlordScore::One:
				case LandlordScore::Two:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->nextPlayer();
					this->pLastToken = &this->records[0][0];
					return true;
				case LandlordScore::Three:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->pLastToken = &this->records[0][0];
					this->status = Status::Assigned;
					return true;
				default:
					return false;
				}
			case 1:
			case 2:
				switch (landlordScore)
				{
				case LandlordScore::None:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
					this->nextPlayer();
					return true;
				case LandlordScore::One:
				case LandlordScore::Two:
					if (nullptr == this->pLastToken || (this->pLastToken->cards.size() == 1 && point > this->pLastToken->cards[0].point))
					{
						this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
						this->nextPlayer();
						this->pLastToken = &this->records[0].back();
						return true;
					}
					else
						return false;
				case LandlordScore::Three:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->pLastToken = &this->records[0].back();
					this->status = Status::Assigned;
					return true;
				default:
					return false;
				}
			case 3:
				switch (landlordScore)
				{
				case LandlordScore::None:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
					break;
				case LandlordScore::One:
				case LandlordScore::Two:
					if (nullptr == this->pLastToken || (this->pLastToken->cards.size() == 1 && point > this->pLastToken->cards[0].point))
					{
						this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
						break;
					}
					else
						return false;
				case LandlordScore::Three:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
					break;
				default:
					return false;
				}
				for (size_t idx = 3; idx > 0; --idx)
					if (!this->records[0][idx].cards.empty())
					{
						this->currentPlayer = this->records[0][idx].player;
						this->pLastToken = &this->records[0][idx];
						this->status = Status::Assigned;
						return true;
					}
				this->currentPlayer = this->records[0][0].player;
				this->pLastToken = &this->records[0][0];
				this->status = Status::Assigned;
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	void display() const override final
	{
		if (this->status >= Status::Assigned)
			PokerGame::display("地主", "地主牌：" + this->cards2string(this->deck, "", " | ", "（已公开）", "（空）") + "\n\n");
		else
			PokerGame::display("拥有明牌", "地主牌：" + this->cards2string(this->deck, "", " | ", "（未公开）", "（空）") + "\n\n");
		return;
	}
};

class BigTwo : public PokerGame /* Previous: Landlords4P | Next: ThreeTwoOne */
{
private:
	/*bool processToken(Token& token) const override final
	{
		return true;
	}*/
	bool coverLastToken(const Token& currentToken) const override final
	{
		if (this->pLastToken != nullptr && TokenType::Empty < this->pLastToken->tokenType && this->pLastToken->tokenType < TokenType::Invalid && !this->pLastToken->cards.empty() && TokenType::Empty < currentToken.tokenType && currentToken.tokenType < TokenType::Invalid && !currentToken.cards.empty())
		{
			switch (this->pLastToken->tokenType)
			{
			case TokenType::Single: // 单牌
				return TokenType::Single == currentToken.tokenType && this->values[currentToken.cards[0].point] > this->values[this->pLastToken->cards[0].point];
			case TokenType::Straight: // 顺子（长度只能为 5）：可被一条龙/同花顺、金刚、葫芦/俘虏、同花以及比自己大的顺子盖过
				return (TokenType::FlushStraight == currentToken.tokenType || TokenType::Quadruple‌WithSingle == currentToken.tokenType || TokenType::TripleWithPair == currentToken.tokenType || TokenType::Flush == currentToken.tokenType) || (TokenType::Straight == currentToken.tokenType && this->values[currentToken.cards[0].point] > this->values[this->pLastToken->cards[0].point]);
			case TokenType::Flush: // 同花（长度只能为 5）：可被一条龙/同花顺、金刚、葫芦/俘虏以及比自己大的同花盖过
				return (TokenType::FlushStraight == currentToken.tokenType || TokenType::Quadruple‌WithSingle == currentToken.tokenType || TokenType::TripleWithPair == currentToken.tokenType) || (TokenType::Flush == currentToken.tokenType && this->values[currentToken.cards[0].point] > this->values[this->pLastToken->cards[0].point]);
			case TokenType::FlushStraight: // 一条龙/同花顺（长度只能为 5）
				return TokenType::FlushStraight == currentToken.tokenType && this->values[currentToken.cards[0].point] > this->values[this->pLastToken->cards[0].point];
			case TokenType::Pair: // 对子
				return TokenType::Pair == currentToken.tokenType && this->values[currentToken.cards[0].point] > this->values[this->pLastToken->cards[0].point];
			}
			return currentToken.tokenType == this->pLastToken->tokenType && this->values[currentToken.cards[0].point] > this->values[this->pLastToken->cards[0].point];
		}
		else
			return false;
	}
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString = "";
			char playerBuffer[4] = { 0 };
			if (this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit)
			{
				snprintf(playerBuffer, 4, "%d", this->records[0].back().player + 1);
				preRoundString = "玩家 " + (string)playerBuffer + " 拥有最小的牌（" + (string)this->records[0].back().cards[0] + "），拥有发牌权。";
			}
			else
				preRoundString = "预备回合信息检验异常。";
			return preRoundString;
		}
	}
	
public:
	BigTwo() : PokerGame()
	{
		this->pokerType = "锄大地";
	}
	bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->players = vector<vector<Card>>(4);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->pLastToken = nullptr;
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	bool initialize(const size_t playerCount) override final { return 3 == playerCount && this->initialize(); }
	bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add52CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < 4; ++player)
			{
				this->players[player] = vector<Card>(13);
				for (size_t idx = 0; idx < 13; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	void display() const override final
	{
		PokerGame::display("方块 3 先出", "");
		return;
	}
};

class ThreeTwoOne : public PokerGame /* Previous: BigTwo | Next: Wuguapi */
{
private:
	/*bool processToken(Token& token) const override final
	{
		return true;
	}*/
	bool coverLastToken(const Token& currentToken) const override final
	{
		if (this->pLastToken != nullptr && TokenType::Empty < this->pLastToken->tokenType && this->pLastToken->tokenType < TokenType::Invalid && !this->pLastToken->cards.empty() && TokenType::Empty < currentToken.tokenType && currentToken.tokenType < TokenType::Invalid && !currentToken.cards.empty())
			return currentToken.tokenType == this->pLastToken->tokenType && currentToken.cards.size() == this->pLastToken->cards.size() && this->values[currentToken.cards[0].point] > this->values[this->pLastToken->cards[0].point];
		else
			return false;
	}
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString = "";
			char playerBuffer[4] = { 0 };
			if (this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit)
			{
				snprintf(playerBuffer, 4, "%d", this->records[0].back().player + 1);
				preRoundString = "玩家 " + (string)playerBuffer + " 拥有最小的牌（" + (string)this->records[0].back().cards[0] + "），拥有发牌权。";
			}
			else
				preRoundString = "预备回合信息检验异常。";
			return preRoundString;
		}
	}
	
public:
	ThreeTwoOne() : PokerGame()
	{
		this->pokerType = "三两一";
	}
	bool initialize() override final
	{
		if (this->status >= Status::Ready)
		{
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->players = vector<vector<Card>>(4);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->pLastToken = nullptr;
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	bool initialize(const size_t playerCount) override final { return 3 == playerCount && this->initialize(); }
	bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add52CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			for (Player player = 0; player < 4; ++player)
			{
				this->players[player] = vector<Card>(13);
				for (size_t idx = 0; idx < 13; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	bool nextPlayer() override final
	{
		if (this->records.empty() || this->currentPlayer >= 4)
			return false;
		else
		{
			bool flags[4] = { true, true, true, true };
			for (const Token& token : this->records.back())
				if (token.player >= 4)
					return false;
				else if (TokenType::Empty == token.tokenType)
					flags[token.player] = false;
			Player offsetPlayer = this->currentPlayer;
			for (Count count = 0; count < 4; ++count)
			{
				offsetPlayer = (offsetPlayer + 1) % 4;
				if (flags[offsetPlayer])
				{
					this->currentPlayer = offsetPlayer;
					return true;
				}
			}
			return false;
		}
	}
	void display() const override final
	{
		PokerGame::display("方块 3 先出", "");
		return;
	}
};

class Wuguapi : public PokerGame /* Previous: ThreeTwoOne | Next: Qiguiwueryi */
{
private:
	/*bool processToken(Token& token) const override final
	{
		return true;
	}*/
	bool isOver() const override final
	{
		if (this->status >= Status::Started && this->deck.empty())
			for (const vector<Card>& cards : this->players)
				if (cards.empty())
					return true;
		return false;
	}
	/*bool coverLastToken(const Token& currentToken) const override final
	{
		return true;
	}*/
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString = "";
			char playerBuffer[4] = { 0 };
			for (const Token& token : this->records[0])
				if (token.cards.size() == 1)
				{
					snprintf(playerBuffer, 4, "%d", token.player + 1);
					preRoundString += (string)token.cards[0] + "（玩家 " + playerBuffer + "） > ";
				}
				else
					return "预备回合信息检验异常。";
			preRoundString.erase(preRoundString.length() - 3, 3);
			return preRoundString;
		}
	}
	
public:
	Wuguapi() : PokerGame()
	{
		this->pokerType = "五瓜皮";
	}
	bool initialize() override final { return this->initialize(2); }
	bool initialize(const size_t playerCount) override final
	{
		if (this->status >= Status::Ready && 2 <= playerCount && playerCount <= 10)
		{
			Value value = 1;
			for (Point point = 6; point <= 13; ++point)
				this->values.set(point, value++);
			for (Point point = 1; point <= 5; ++point)
				this->values.set(point, value++);
			this->values.set(JOKER_POINT, value++);
			this->players = vector<vector<Card>>(playerCount);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->pLastToken = nullptr;
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			const size_t playerCount = this->players.size();
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = vector<Card>(5);
				for (size_t idx = 0; idx < 5; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	void display() const override final
	{
		PokerGame::display("最小先出", "牌堆（自下往上）：" + this->cards2string(this->deck, "", " | ", "", "（空）") + "\n\n");
		return;
	}
};

class Qiguiwueryi : public PokerGame /* Previous: ThreeTwoOne | Next: Qiguiwuersan */
{
private:
	/*bool processToken(Token& token) const override final
	{
		return true;
	}*/
	bool isOver() const override final
	{
		if (this->status >= Status::Started && this->deck.empty())
			for (const vector<Card>& cards : this->players)
				if (cards.empty())
					return true;
		return false;
	}
	bool coverLastToken(const Token& currentToken) const override final
	{
		if (this->pLastToken != nullptr && TokenType::Empty < this->pLastToken->tokenType && this->pLastToken->tokenType < TokenType::Invalid && !this->pLastToken->cards.empty() && TokenType::Empty < currentToken.tokenType && currentToken.tokenType < TokenType::Invalid && !currentToken.cards.empty())
			return currentToken.tokenType == this->pLastToken->tokenType && currentToken.cards.size() == this->pLastToken->cards.size() && this->values[currentToken.cards[0].point] > this->values[this->pLastToken->cards[0].point];
		else
			return false;
	}
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString = "";
			char playerBuffer[4] = { 0 };
			for (const Token& token : this->records[0])
				if (token.cards.size() == 1)
				{
					snprintf(playerBuffer, 4, "%d", token.player + 1);
					preRoundString += (string)token.cards[0] + "（玩家 " + playerBuffer + "） > ";
				}
				else
					return "预备回合信息检验异常。";
			preRoundString.erase(preRoundString.length() - 3, 3);
			return preRoundString;
		}
	}
	
public:
	Qiguiwueryi() : PokerGame()
	{
		this->pokerType = "七鬼五二一";
	}
	bool initialize() override { return this->initialize(2); }
	bool initialize(const size_t playerCount) override
	{
		if (this->status >= Status::Ready && 2 <= playerCount && playerCount <= 7)
		{
			Value value = 1;
			this->values.set(3, value++);
			this->values.set(4, value++);
			this->values.set(6, value++);
			for (Point point = 8; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(1, value++);
			this->values.set(2, value++);
			this->values.set(5, value++);
			this->values.set(JOKER_POINT, value++);
			this->values.set(7, value++);
			this->players = vector<vector<Card>>(playerCount);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->pLastToken = nullptr;
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
	bool deal() override final
	{
		if (this->status >= Status::Initialized)
		{
			this->deck.clear();
			this->add54CardsToDeck();
			shuffle(this->deck.begin(), this->deck.end(), this->seed);
			const size_t playerCount = this->players.size();
			for (Player player = 0; player < playerCount; ++player)
			{
				this->players[player] = vector<Card>(7);
				for (size_t idx = 0; idx < 7; ++idx)
				{
					this->players[player][idx] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[player]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	void display() const override final
	{
		PokerGame::display("最小先出", "牌堆（自下往上）：" + this->cards2string(this->deck, "", " | ", "", "（空）") + "\n\n");
		return;
	}
};

class Qiguiwuersan : public Qiguiwueryi /* Previous: Qiguiwueryi */
{
public:
	Qiguiwuersan() : Qiguiwueryi()
	{
		this->pokerType = "七鬼五二三";
	}
	bool initialize() override final { return this->initialize(2); }
	bool initialize(const size_t playerCount) override final
	{
		if (this->status >= Status::Ready && 2 <= playerCount && playerCount <= 7)
		{
			Value value = 1;
			this->values.set(1, value++);
			this->values.set(4, value++);
			this->values.set(6, value++);
			for (Point point = 8; point <= 13; ++point)
				this->values.set(point, value++);
			this->values.set(3, value++);
			this->values.set(2, value++);
			this->values.set(5, value++);
			this->values.set(JOKER_POINT, value++);
			this->values.set(7, value++);
			this->players = vector<vector<Card>>(playerCount);
			this->deck.clear();
			this->records.clear();
			this->currentPlayer = INVALID_PLAYER;
			this->pLastToken = nullptr;
			this->status = Status::Initialized;
			return true;
		}
		else
			return false;
	}
};

class Interaction
{
private:
	const vector<string> helpOptions = { "?", "/?", "-?", "h", "/h", "-h", "help", "/help", "--help" };
	HelpKey helpKey = 0;
	const vector<string> pokerTypeOptions = { "t", "/t", "-t", "pokerType", "/pokerType", "--pokerType" };
	const vector<string> pokerTypesC = { "斗地主", "四人斗地主", "锄大地", "三两一", "五瓜皮", "七鬼五二一", "七鬼五二三" };
	const vector<string> pokerTypesE = { "Landlords", "Landlords4P", "BigTwo", "ThreeTwoOne", "Wuguapi", "Qiguiwueryi", "Qiguiwuersan" };
	string pokerType = "";
	const vector<string> playerCountOptions = { "p", "/p", "-p", "playerCount", "/playerCount", "--playerCount" };
	size_t playerCount = 0;
	const vector<string> sortingMethodOptions = { "s", "/s", "-s", "sortingMethod", "/sortingMethod", "--sortingMethod" };
	vector<SortingMethod> sortingMethods{};
	const vector<string> landlordStatements = { "Y", "yes", "1", "T", "true", "是", "叫", "叫地主", "叫牌", "抢", "抢地主", "抢牌" };

	bool isEqual(const string& s1, const string& s2) const // Please use == directly if cases cannot be ignored
	{
		if (s1.length() == s2.length())
		{
			const size_t length = s1.length();
			for (size_t idx = 0; idx < length; ++idx)
			{
				char c1 = s1.at(idx), c2 = s2.at(idx);
				if ('A' <= c1 && c1 <= 'Z')
					c1 ^= 0x20;
				if ('A' <= c2 && c2 <= 'Z')
					c2 ^= 0x20;
				if (c1 != c2)
					return false;
			}
			return true;
		}
		else
			return false;
	}
	bool isIn(const string& s, const vector<string>& strings) const
	{
		for (const string& str : strings)
			if (this->isEqual(s, str))
				return true;
		return false;
	}
	void replaceAll(string& str, const string& oldSubString, const string& newSubString) const
	{
		size_t pos = 0;
		while ((pos = str.find(oldSubString, pos)) != string::npos)
		{
			str.replace(pos, oldSubString.length(), newSubString);
			pos += newSubString.length();
		}
		return;
	}
	void optimizePokerType(string& _pokerType) const
	{
		if (!this->isIn(_pokerType, this->pokerTypesE))
		{
			_pokerType.erase(remove_if(_pokerType.begin(), _pokerType.end(), [](char ch) { return ' ' == ch || '\t' == ch || '\r' == ch || '\n' == ch; }), _pokerType.end());
			this->replaceAll(_pokerType, "D", "地");
			this->replaceAll(_pokerType, "7", "七");
			this->replaceAll(_pokerType, "5", "五");
			this->replaceAll(_pokerType, "3", "三");
			this->replaceAll(_pokerType, "2", "二");
			this->replaceAll(_pokerType, "1", "一");
			this->replaceAll(_pokerType, "三人斗地主", "斗地主");
			this->replaceAll(_pokerType, "欢乐斗地主", "斗地主");
			this->replaceAll(_pokerType, "大老二", "锄大地");
		}
		return;
	}
	string vector2string(const vector<string>& strings, const string& prefix, const string& separator, const string& suffix) const
	{
		string stringBuffer = prefix;
		const size_t length = strings.size();
		if (length >= 1)
		{
			stringBuffer += strings[0];
			for (size_t stringID = 1; stringID < length; ++stringID)
				stringBuffer += separator + strings[stringID];
		}
		stringBuffer += suffix;
		return stringBuffer;
	}
	bool printHelp()
	{
		cout << "通用扑克牌实现与解杀程序。" << endl << endl << endl;
		switch (this->helpKey)
		{
		case 'T':
		case 't':
		{
			cout << "目前支持以下扑克牌类型：" << endl;
			const size_t length = this->pokerTypesC.size();
			for (size_t idx = 0; idx < length;)
				cout << "\t（" << ++idx << "）" << this->pokerTypesC[idx] << "；" << endl;
			cout << endl << endl << endl;
			return true;
		}
		case 'P':
		case 'p':
			cout << "用于指定玩家人数的参数目前仅对以下扑克牌类型生效：" << endl;
			cout << "\t（1）五瓜皮：最少 2 人，最多 10 人，默认 2 人；" << endl;
			cout << "\t（2）七鬼五二一：最少 2 人，最多 7 人，默认 2 人；" << endl;
			cout << "\t（3）七鬼五二三：最少 2 人，最多 7 人，默认 2 人。" << endl << endl;
			cout << "否则，该参数将会被自动忽略。" << endl << endl << endl << endl;
			return true;
		case 'S':
		case 's':
			cout << "目前支持以下排序显示方式：" << endl;
			cout << "\t（1）从大到小排序（默认）：FromBigToSmall (0)；" << endl;
			cout << "\t（2）从小到大排序：FromSmallToBig (1)；" << endl;
			cout << "\t（3）从多到少排序：FromManyToFew (2)；" << endl;
			cout << "\t（4）从少到多排序：FromFewToMany (3)。" << endl << endl << endl << endl;
			return true;
		default:
			cout << "参数（不区分顺序和大小写）：" << endl;
			cout << "\t" << this->vector2string(this->pokerTypeOptions, "[", "|", "]") << " [扑克牌类型]\t\t\t设置扑克牌类型" << endl;
			cout << "\t" << this->vector2string(this->playerCountOptions, "[", "|", "]") << " [玩家人数]\t\t设置玩家人数" << endl;
			cout << "\t" << this->vector2string(this->sortingMethodOptions, "[", "|", "]") << " [排序显示方式]\t设置排序显示方式" << endl;
			cout << "\t" << this->vector2string(this->helpOptions, "[", "|", "]") << " 或 [其它参数] " << this->vector2string(this->helpOptions, "[", "|", "]") << "\t显示帮助" << endl << endl << endl;
			cout << "注意：" << endl;
			cout << "\t（1）键和值应当成对出现，即每一个表示键的参数后均应紧接着其对应的值（含帮助参数）；" << endl;
			cout << "\t（2）当同一键出现多次时，其值以该键最后一次以合法键值对的形式出现时的值为准；" << endl;
			cout << "\t（3）出现多个帮助参数时，以最后一次出现帮助参数时的上下文进行显示帮助。" << endl << endl << endl << endl;
			return 1 == this->helpKey;
		}
	}
	int clearScreen() const
	{
#if defined _WIN32 || defined _WIN64
		return system("cls");
#else
		return system("clear");
#endif
	}
	void getDescription(string& description) const
	{
		rewind(stdin);
		fflush(stdin);
		char buffer[BUFFER_SIZE] = { 0 };
		fgets(buffer, BUFFER_SIZE, stdin);
		description = buffer;
		while (!description.empty() && description.back() == '\n')
			description.pop_back();
		return;
	}
	size_t fetchPlayerCount(size_t lowerBound, size_t upperBound) const // lowerBound cannot be 0
	{
		if (lowerBound <= this->playerCount && this->playerCount <= upperBound)
			return this->playerCount;
		else
		{
			this->clearScreen();
			cout << "已选定扑克牌类型：" << this->pokerType << endl;
			cout << "该扑克牌类型支持最少 " << lowerBound << " 人，最多 " << upperBound << " 人。" << endl;
			for (;;)
			{
				string playerCountString = "";
				cout << "请输入玩家人数：";
				this->getDescription(playerCountString);
				if (playerCountString.empty())
					return lowerBound;
				else
				{
					const unsigned long int playerCountUL = strtoul(playerCountString.c_str(), NULL, 0);
					if (lowerBound <= playerCountUL && playerCountUL <= upperBound)
						return (size_t)playerCountUL;
				}
			}
		}
	}
	bool fetchPokerType()
	{
		this->optimizePokerType(this->pokerType);
		if (!this->isIn(this->pokerType, this->pokerTypesC) && !this->isIn(this->pokerType, this->pokerTypesE))
			for (;;)
			{
				this->clearScreen();
				cout << "可选的扑克牌类型如下：" << endl;
				const size_t length = this->pokerTypesC.size();
				for (size_t idx = 0; idx < length; ++idx)
					cout << "\t" << (idx + 1) << " = " << this->pokerTypesC[idx] << endl;
				cout << "\t0 = 退出程序" << endl << endl << "请选择或输入一种扑克牌以继续：";
				string _pokerType = "";
				this->getDescription(_pokerType);
				if (_pokerType.size() == 1)
				{
					const char choice = _pokerType.at(0) - '1';
					if (-1 == choice)
						return false;
					else if (0 <= choice && choice < this->pokerTypesC.size())
					{
						this->pokerType = this->pokerTypesC[choice];
						break;
					}
				}
				else
				{
					this->optimizePokerType(_pokerType);
					if (this->isIn(_pokerType, this->pokerTypesC) || this->isIn(_pokerType, this->pokerTypesE))
					{
						this->pokerType = _pokerType;
						break;
					}
					else if ("退出程序" == _pokerType || this->isEqual("Exit", _pokerType))
						return false;
				}
			}
		if ("五瓜皮" == this->pokerType || this->isEqual("Wuguapi", this->pokerType))
			this->playerCount = fetchPlayerCount(2, 10);
		else if ("七鬼五二一" == this->pokerType || this->isEqual("Qiguiwueryi", this->pokerType) || "七鬼五二三" == this->pokerType || this->isEqual("Qiguiwuersan", this->pokerType))
			this->playerCount = fetchPlayerCount(2, 7);
		else
			this->playerCount = 0;
		return true;
	}
	bool fetchBinaryChars(const string& filePath, char binaryChars[]) const // return true if the parameter is a good file to be read
	{
		ifstream ifs(filePath, ios::binary);
		if (ifs.is_open())
		{
			ifs.read(binaryChars, sizeof(binaryChars));
			ifs.close();
			return true;
		}
		else
			return false;
	}
	
public:
	Interaction()
	{

	}
	Interaction(const vector<string>& arguments)
	{
		if (!arguments.empty())
		{
			vector<size_t> invalidArgumentIndexes{};
			const size_t argumentCount = arguments.size() - 1;
			size_t argumentID = 0;
			for (; argumentID < argumentCount; ++argumentID)
				if (this->isIn(arguments[argumentID], this->helpOptions))
					this->helpKey = 1;
				else if (this->isIn(arguments[argumentID], this->pokerTypeOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 't';
					else
					{
						string _pokerType = arguments[argumentID];
						this->optimizePokerType(_pokerType);
						if (this->isIn(_pokerType, this->pokerTypesC) || this->isIn(_pokerType, this->pokerTypesE))
							this->pokerType = _pokerType;
						else
							invalidArgumentIndexes.push_back(argumentID);
					}
				else if (this->isIn(arguments[argumentID], this->playerCountOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 'p';
					else
					{
						const size_t _playerCount = (size_t)strtoul(arguments[argumentID].c_str(), NULL, 0);
						if (2 <= _playerCount && _playerCount <= 10)
							this->playerCount = _playerCount;
						else
							invalidArgumentIndexes.push_back(argumentID);
					}
				else if (this->isIn(arguments[argumentID], this->sortingMethodOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 's';
					else if (this->isEqual("FromBigToSmall", arguments[argumentID]) || this->isEqual("0", arguments[argumentID]))
						this->sortingMethods = vector<SortingMethod>{ SortingMethod::FromBigToSmall };
					else if (this->isEqual("FromSmallToBig", arguments[argumentID]) || this->isEqual("1", arguments[argumentID]))
						this->sortingMethods = vector<SortingMethod>{ SortingMethod::FromSmallToBig };
					else if (this->isEqual("FromManyToFew", arguments[argumentID]) || this->isEqual("2", arguments[argumentID]))
						this->sortingMethods = vector<SortingMethod>{ SortingMethod::FromManyToFew };
					else if (this->isEqual("FromFewToMany", arguments[argumentID]) || this->isEqual("3", arguments[argumentID]))
						this->sortingMethods = vector<SortingMethod>{ SortingMethod::FromFewToMany };
					else
						invalidArgumentIndexes.push_back(argumentID);
				else
					invalidArgumentIndexes.push_back(argumentID);
			if (argumentID == argumentCount)
				if (this->isIn(arguments[argumentID], this->helpOptions))
					this->helpKey = 1;
				else
					invalidArgumentIndexes.push_back(argumentID);
			if (!invalidArgumentIndexes.empty())
			{
				const size_t length = invalidArgumentIndexes.size();
				cout << "警告：以下 " << length << " 个参数无效。" << endl;
				for (size_t idx = 0; idx < length; ++idx)
					cout << "（" << (idx + 1) << "）参数 " << (invalidArgumentIndexes[idx] + 1) << " 无效——“" << arguments[invalidArgumentIndexes[idx]] << "”，其键值对（如有）已被自动跳过。" << endl;
				cout << endl;
				this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP * length));
			}
		}
	}
	bool interact()
	{
		if (this->helpKey)
			return this->printHelp();
		else
		{
			PokerGame* pokerGame = nullptr;
			while (nullptr == pokerGame)
			{
				if ("斗地主" == this->pokerType || this->isEqual("Landlords", this->pokerType)) // "三人斗地主"
					pokerGame = new Landlords;
				else if ("四人斗地主" == this->pokerType || this->isEqual("Landlords4P", this->pokerType)) // "四人斗地主"
					pokerGame = new Landlords4P;
				else if ("锄大地" == this->pokerType || this->isEqual("BigTwo", this->pokerType)) // "锄大地"
					pokerGame = new BigTwo;
				else if ("三两一" == this->pokerType || this->isEqual("ThreeTwoOne", this->pokerType)) // "三两一"
					pokerGame = new ThreeTwoOne;
				else if ("五瓜皮" == this->pokerType || this->isEqual("Wuguapi", this->pokerType)) // "五瓜皮"
					pokerGame = new Wuguapi;
				else if ("七鬼五二一" == this->pokerType || this->isEqual("Qiguiwueryi", this->pokerType)) // "七鬼五二一"
					pokerGame = new Qiguiwueryi;
				else if ("七鬼五二三" == this->pokerType || this->isEqual("Qiguiwuersan", this->pokerType)) // "七鬼五二三"
					pokerGame = new Qiguiwueryi;
				else if (!this->fetchPokerType())
					return false;
			}
			this->fetchPokerType();
			if (this->playerCount ? pokerGame->initialize(this->playerCount) : pokerGame->initialize())
			{
				string buffer = "";
				this->clearScreen();
				cout << "请按回车键开局，或录入残局库数据：";
				for (;;)
				{
					this->getDescription(buffer);
					if (buffer.empty() || "/" == buffer)
						pokerGame->deal();
					else
					{
						char binaryChars[BUFFER_SIZE] = { 0 };
						this->fetchBinaryChars(buffer, binaryChars);
						if (pokerGame->set(binaryChars))
							break;
						else
							cout << "录入失败！请按回车键开局，或再次尝试录入残局库数据：";
					}
				}
				Player player = 0;
				if ("斗地主" == this->pokerType)
				{
					Count retryCount = 0;
					for (;;)
					{
						bool isRobbing = false;
						Count callerAndRobberCount = 0;
						for (Count count = 1; count <= 3; ++count)
						{
							pokerGame->getCurrentPlayer(player);
							this->clearScreen();
							pokerGame->display();
							cout << "请玩家 " << (player + 1) << " 选择是否" << (isRobbing ? "抢" : "叫") << "地主：";
							this->getDescription(buffer);
							if (this->isIn(buffer, this->landlordStatements))
								if (pokerGame->setLandlord(true))
								{
									isRobbing = true;
									++callerAndRobberCount;
								}
								else
									--count;
							else if (!pokerGame->setLandlord(false))
								--count;
						}
						if (callerAndRobberCount >= 2)
							for (;;)
							{
								this->clearScreen();
								pokerGame->display();
								cout << "请玩家 " << (player + 1) << " 选择是否抢地主：";
								this->getDescription(buffer);
								if (this->isIn(buffer, this->landlordStatements))
								{
									if (pokerGame->setLandlord(true))
										break;
								}
								else if (!pokerGame->setLandlord(false))
									break;
							}
						else if (0 == callerAndRobberCount && ++retryCount < 3)
						{
							cout << "无人叫地主，即将重新发牌。" << endl;
							this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
							pokerGame->deal();
							continue;
						}
						break;
					}
				}
				else if ("四人斗地主" == this->pokerType)
				{
					Count retryCount = 0;
					for (;;)
					{
						bool isCalled = false;
						LandlordScore currentHighestScore = LandlordScore::None;
						vector<string> scoreDescriptions{ "不叫", "3分", "2分", "1分" };
						for (Count count = 1; count <= 4 && currentHighestScore < LandlordScore::Three;)
						{
							pokerGame->getCurrentPlayer(player);
							this->clearScreen();
							pokerGame->display();
							cout << "请玩家 " << (player + 1) << " 选择（" << this->vector2string(scoreDescriptions, "", " | ", "") << "）：";
							this->getDescription(buffer);
							if (!buffer.empty())
							{
								const char scoreChar = buffer.at(0) - '0';
								LandlordScore landlordScore = LandlordScore::None;
								switch (scoreChar)
								{
								case 1:
									landlordScore = LandlordScore::One;
									break;
								case 2:
									landlordScore = LandlordScore::Two;
									break;
								case 3:
									landlordScore = LandlordScore::Three;
									break;
								default:
									break;
								}
								if (pokerGame->setLandlord(landlordScore))
								{
									if (landlordScore > currentHighestScore)
									{
										for (Count removalCount = static_cast<Count>(landlordScore) - static_cast<Count>(currentHighestScore); removalCount > 0; --removalCount)
											scoreDescriptions.pop_back();
										currentHighestScore = landlordScore;
									}
									++count;
								}
							}
						}
						if (isCalled || retryCount >= 2)
							break;
						else
						{
							++retryCount;
							cout << "无人叫地主，即将重新发牌。" << endl;
							this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
							pokerGame->deal();
						}
					}
				}
				pokerGame->getCurrentPlayer(player);
				for (;;)
				{
					this->clearScreen();
					pokerGame->display();
					cout << "请玩家 " << (player + 1) << " 开牌：";
					this->getDescription(buffer);
					if (pokerGame->start(buffer))
						break;
				}
				pokerGame->getCurrentPlayer(player);
				while (player != INVALID_PLAYER)
				{
					for (;;)
					{
						this->clearScreen();
						pokerGame->display();
						cout << "请玩家 " << (player + 1) << " 出牌：";
						this->getDescription(buffer);
						if (pokerGame->play(buffer))
							break;
					}
					pokerGame->getCurrentPlayer(player);
				}
				cout << "此局已终，程序即将退出。" << endl;
				this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
				if (pokerGame != nullptr)
				{
					delete pokerGame;
					pokerGame = nullptr;
				}
				return true;
			}
			else
			{
				cout << "错误：初始化实例失败。" << endl << endl << endl << endl;
				if (pokerGame != nullptr)
				{
					delete pokerGame;
					pokerGame = nullptr;
				}
				return false;
			}
		}
	}
};



int main(int argc, char* argv[])
{
	vector<string> arguments(argc - 1);
	for (int i = 0; i < argc - 1; ++i)
		arguments[i] = argv[i + 1];
	Interaction interaction = Interaction(arguments);
	return interaction.interact() ? EXIT_SUCCESS : EXIT_FAILURE;
}