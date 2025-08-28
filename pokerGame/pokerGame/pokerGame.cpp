#include <iostream>
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
#define UNREFERENCED_PARAMETER(P) (P)
#endif
#ifndef JOKER_POINT
#define JOKER_POINT 0
#endif
#ifndef TIME_FOR_SLEEP
#define TIME_FOR_SLEEP 3
#endif
using namespace std;
typedef unsigned char Value;
typedef unsigned char Point;
typedef unsigned char PlayerID;
typedef unsigned char Count;
typedef unsigned char CardID;
typedef unsigned char HelpKey;


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
	flush = 0b00010010, 
	flushStraight = 0b00010011, 
	
	Pair = 0b00100000, 
	PairStraight = 0b00100001, 
	PairStraightWithSingle = 0b00100010, 
	Rocket = 0b00100011, 
	
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

enum class SortingMethod
{
	FromBigToSmall = 0, 
	FromSmallToBig = 1, 
	FromManyToFew = 2, 
	FromFewToMany = 3
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
	Point point = 0; // JOKER_POINT (0) for the Jokers and the Cover Card
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
	PlayerID playerID = (PlayerID)(-1);
	vector<Card> cards{};
	TokenType tokenType = TokenType::Invalid;
	Card indicator = Card{};
};


class PokerGame
{
protected:
	mt19937 seed{};
	string pokerType = "扑克牌";
	Value values[14] = { 0 };
	vector<vector<Card>> players{};
	vector<Card> deck{};
	vector<vector<Token>> records{};
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
			for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
				this->records[0].push_back(Token{ playerID, vector<Card>{ this->players[playerID].back() } });
			sort(this->records[0].begin(), this->records[0].end(), [this](Token a, Token b) { return this->values[a.cards.back().point] > this->values[b.cards.back().point] || (this->values[a.cards.back().point] == this->values[b.cards.back().point] && a.cards.back().suit > b.cards.back().suit); });
			this->status = Status::Assigned;
			return true;
		}
		else
			return false;
	}
	
	/* PokerGame::start and PokerGame::play */
	virtual bool description2token(const string& description, Token& token) const final
	{
		const PlayerID playerID = token.playerID;
		if (0 <= playerID && playerID < this->players.size() && !this->players[playerID].empty())
			if ("" == description || "/" == description || "-" == description || "--" == description || "要不起" == description || "不出" == description || "不打" == description)
			{
				token.cards.clear();
				return true;
			}
			else
			{
				vector<size_t> selected{};
				vector<Card> exactCards{};
				vector<Point> fuzzyPoints{};
				vector<Suit> fuzzySuits{};
				bool waitForAPoint = false;
				Suit suit = Suit::Diamond;
				const size_t descriptionLength = description.length();
				for (size_t idx = 0; idx < descriptionLength; ++idx)
				{
					switch (description.at(idx))
					{
					case 'A':
					case 'a':
						if (waitForAPoint)
							exactCards.push_back(Card{ 1, suit });
						else
							fuzzyPoints.push_back(1);
						break;
					case '1':
						if (idx + 1 < descriptionLength && '0' == description.at(idx + 1))
						{
							if (waitForAPoint)
								exactCards.push_back(Card{ 10, suit });
							else
								fuzzyPoints.push_back(10);
							++idx;
						}
						else if (waitForAPoint)
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
						if (waitForAPoint)
							exactCards.push_back(Card{ (Point)(description.at(idx) - '0'), suit });
						else
							fuzzyPoints.push_back(description.at(idx) - '0');
						break;
					case 'T':
					case 't':
						if (waitForAPoint)
							exactCards.push_back(Card{ 10, suit });
						else
							fuzzyPoints.push_back(10);
					case 'J':
					case 'j':
						if (waitForAPoint)
							exactCards.push_back(Card{ 11, suit });
						else
							fuzzyPoints.push_back(11);
						break;
					case 'Q':
					case 'q':
						if (waitForAPoint)
							exactCards.push_back(Card{ 12, suit });
						else
							fuzzyPoints.push_back(12);
						break;
					case 'K':
					case 'k':
						if (waitForAPoint)
							exactCards.push_back(Card{ 13, suit });
						else
							fuzzyPoints.push_back(13);
						break;
					case 'L':
					case 'l':
						if (waitForAPoint)
							fuzzySuits.push_back(suit);
						else
							exactCards.push_back(Card{ JOKER_POINT, Suit::Black });
						break;
					case 'B':
					case 'b':
						if (waitForAPoint)
							fuzzySuits.push_back(suit);
						else
							exactCards.push_back(Card{ JOKER_POINT, Suit::Black });
						break;
					default:
					{
						if (waitForAPoint)
						{
							fuzzySuits.push_back(suit);
							waitForAPoint = false;
						}
						const string str = description.substr(idx, 4);
						if ("方块" == str)
						{
							suit = Suit::Diamond;
							waitForAPoint = true;
							idx += 3;
						}
						else if ("梅花" == str)
						{
							suit = Suit::Club;
							waitForAPoint = true;
							idx += 3;
						}
						else if ("红桃" == str || "红心" == str)
						{
							suit = Suit::Heart;
							waitForAPoint = true;
							idx += 3;
						}
						else if ("黑桃" == str)
						{
							suit = Suit::Spade;
							waitForAPoint = true;
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
				if (waitForAPoint)
					fuzzySuits.push_back(suit);
				const size_t length = this->players[playerID].size();
				size_t position = 0;
				for (const Card& card : exactCards) // select the rightmost one
				{
					bool flag = false;
					for (size_t idx = 0; idx < length; ++idx)
						if (this->values[this->players[playerID][idx].point] > this->values[card.point])
							continue;
						else if (this->players[playerID][idx] == card && find(selected.begin(), selected.end(), idx) == selected.end())
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
						if (this->values[this->players[playerID][idx].point] > this->values[point])
							continue;
						else if (this->players[playerID][idx].point == point)
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
						if (this->players[playerID][idx].suit == s && find(selected.begin(), selected.end(), idx) == selected.end())
						{
							position = idx;
							flag = true;
							break;
						}
					if (flag)
						selected.push_back(position);
					else if (this->players[playerID][0].suit == s) // avoid (size_t)(-1)
						selected.push_back(0);
					else
						return false;
				}
				token.cards.clear();
				for (const size_t& p : selected)
					token.cards.push_back(this->players[playerID][p]);
				return true;
			}
		else
			return false;
	}
	
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
			for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
				cout << "玩家 " << (playerID + 1) << (!this->records.empty() && !this->records[0].empty() && this->records[0].back().playerID == playerID ? "（" + dealerRemark + "）" : "：") << endl << this->cards2string(this->players[playerID], "", " | ", "", "（空）") << endl << endl;
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
						cout << this->cards2string(this->records[round][0].cards, "", "+", "", "（空）") << "（玩家 " << (this->records[round][0].playerID + 1) << "）";
						const size_t tokenCount = this->records[round].size();
						for (size_t tokenID = 1; tokenID < tokenCount; ++tokenID)
							cout << " -> " << this->cards2string(this->records[round][tokenID].cards, "", "+", "", "（空）") << "（玩家 " << (this->records[round][tokenID].playerID + 1) << "）";
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
	PokerGame() // seed and pokerType
	{
		random_device rd;
		mt19937 g(rd());
		this->seed = g;
	}
	virtual ~PokerGame()
	{

	}
	virtual bool initialize() = 0; // values, players (= vector<vector<Card>>(n)), deck (clear), and records (clear)
	virtual bool initialize(const size_t playerCount) = 0; // values, players (= vector<vector<Card>>(n)), deck (clear), and records (clear)
	virtual bool deal() = 0; // players, deck, and records (clear) -> records[0]
	virtual bool getCurrentPlayer(PlayerID& playerID) const final // const
	{
		if (this->records.empty() || this->records.back().empty())
			return false;
		else
		{
			playerID = this->records.back().back().playerID;
			return true;
		}
	}
	virtual bool getNextPlayer(PlayerID& playerID) const // const
	{
		if (this->players.empty())
			return false;
		else
		{
			if (++playerID >= this->players.size())// This works correctly even though playerID is 255. 
				playerID = 0;
			return true;
		}
	}
	virtual bool setLandlord(bool b0, bool b1, bool b2, bool b3) // records
	{
		UNREFERENCED_PARAMETER(b0);
		UNREFERENCED_PARAMETER(b1);
		UNREFERENCED_PARAMETER(b2);
		UNREFERENCED_PARAMETER(b3);
		return false;
	}
	virtual bool setLandlord(const vector<PlayerID>& playerIDs) // records
	{
		UNREFERENCED_PARAMETER(playerIDs);
		return false;
	}
	virtual bool setLandlord(LandlordScore s0, LandlordScore s1, LandlordScore s2, LandlordScore s3) // records
	{
		UNREFERENCED_PARAMETER(s0);
		UNREFERENCED_PARAMETER(s1);
		UNREFERENCED_PARAMETER(s2);
		UNREFERENCED_PARAMETER(s3);
		return false;
	}
	//virtual bool set() = 0; // values, players, deck, and records
	virtual void display() const = 0; // const
};

class Landlords : public PokerGame
{
private:
	bool assignDealer() override final
	{
		if (Status::Dealt == this->status && this->records.empty())
		{
			uniform_int_distribution<size_t> distribution(0, this->players.size() - 1);
			const PlayerID playerID = (PlayerID)(distribution(this->seed));
			this->records.push_back(vector<Token>{ Token{ playerID, vector<Card>{} }});
			return true;
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
			char playerIDBuffer[4] = { 0 };
			const size_t length = this->records[0].size();
			if (length >= 2)
			{
				bool isRobbing = false;
				PlayerID playerID = this->records[0][0].playerID;
				for (PlayerID idxInner = 1, idxOuter = 1; idxOuter <= 4; ++idxOuter)
				{
					if (idxInner < length && this->records[0][idxInner].playerID == playerID)
					{
						snprintf(playerIDBuffer, 4, "%d", playerID + 1);
						if (isRobbing)
							preRoundString += "抢地主（玩家 " + (string)playerIDBuffer + "） -> ";
						else
						{
							preRoundString += "叫地主（玩家 " + (string)playerIDBuffer + "） -> ";
							isRobbing = true;
						}
						++idxInner;
					}
					else
						preRoundString += (isRobbing ? "不抢（玩家 " : "不叫（玩家 ") + (string)playerIDBuffer + "） -> ";
					this->getNextPlayer(playerID);
				}
				preRoundString.erase(preRoundString.length() - 4, 4);
			}
			else if (1 == length)
			{
				snprintf(playerIDBuffer, 4, "%d", (this->records[0].back().playerID + 1));
				preRoundString += "无人叫地主，强制玩家 " + (string)playerIDBuffer + " 为地主。";
			}
			else
				preRoundString += "地主信息不详。";
			return preRoundString;
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
				this->values[point] = value++;
			this->values[1] = value++;
			this->values[2] = value++;
			this->values[JOKER_POINT] = value++;
			this->players = vector<vector<Card>>(3);
			this->deck.clear();
			this->records.clear();
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
			for (PlayerID playerID = 0; playerID < 3; ++playerID)
			{
				this->players[playerID] = vector<Card>(17);
				for (CardID cardID = 0; cardID < 17; ++cardID)
				{
					this->players[playerID][cardID] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[playerID]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	bool setLandlord(bool b0, bool b1, bool b2, bool b3) override final
	{
		if (Status::Dealt == this->status && this->records.size() == 1 && this->records[0].size() == 1 && this->players.size() == 3)
		{
			PlayerID playerID = this->records[0].back().playerID;
			if (b0)
				this->records[0].push_back(Token{ playerID, vector<Card>{} });
			this->getNextPlayer(playerID);
			if (b1)
				this->records[0].push_back(Token{ playerID, vector<Card>{} });
			this->getNextPlayer(playerID);
			if (b2)
				this->records[0].push_back(Token{ playerID, vector<Card>{} });
			this->getNextPlayer(playerID);
			if (b3)
				this->records[0].push_back(Token{ playerID, vector<Card>{} });
			for (size_t idx = 0; idx < 3; ++idx)
				this->players[this->records[0].back().playerID].push_back(this->deck[idx]);
			this->sortCards(this->players[this->records[0].back().playerID]);
			this->status = Status::Assigned;
			return true;
		}
		else
			return false;
	}
	bool setLandlord(const vector<PlayerID>& playerIDs) override final
	{
		const size_t playerCount = this->players.size(), length = playerIDs.size();
		if (Status::Dealt == this->status && this->records.size() == 1 && this->records[0].size() == 1 && 3 == playerCount && length <= 4)
		{
			PlayerID playerID = this->records[0].back().playerID;
			bool booleans[4] = { false };
			for (size_t idxInner = 0, idxOuter = 0; idxInner < playerCount && idxOuter < length; ++idxInner)
			{
				if (playerIDs[idxOuter] == playerID)
				{
					booleans[idxInner] = true;
					++idxOuter;
				}
				this->getNextPlayer(playerID);
			}
			return this->setLandlord(booleans[0], booleans[1], booleans[2], booleans[3]);
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

class Landlords4P : public PokerGame
{
private:
	bool assignDealer() override final
	{
		if (Status::Dealt == this->status && this->records.empty())
		{
			uniform_int_distribution<size_t> distribution(0, this->players.size() - 1);
			const PlayerID playerID = (PlayerID)(distribution(this->seed));
			this->records.push_back(vector<Token>{ Token{ playerID, vector<Card>{} }});
			return true;
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
			char playerIDBuffer[4] = { 0 }, landlordScoreBuffer[4] = { 0 };
			const size_t length = this->records[0].size();
			if (length >= 2)
			{
				for (const Token& token : this->records[0])
					if (token.cards.size() == 1)
					{
						snprintf(playerIDBuffer, 4, "%d", token.playerID + 1);
						switch (token.cards[0].point)
						{
						case 0:
							preRoundString += "不叫（玩家 " + (string)playerIDBuffer + "） -> ";
							break;
						case 1:
						case 2:
						case 3:
						{
							snprintf(landlordScoreBuffer, 4, "%d", token.cards[0].point);
							preRoundString += (string)landlordScoreBuffer + "分（玩家 " + playerIDBuffer + "） -> ";
							break;
						}
						default:
							break;
						}
					}
				preRoundString.erase(preRoundString.length() - 4, 4);
			}
			else
			{
				snprintf(playerIDBuffer, 4, "%d", (this->records[0].back().playerID + 1));
				if (this->records[0].back().cards[0].point)
				{
					snprintf(landlordScoreBuffer, 4, "%d", this->records[0].back().cards[0].point);
					preRoundString += (string)landlordScoreBuffer + "分（玩家 " + playerIDBuffer + "）";
				}
				else
					preRoundString += "无人叫地主，强制玩家 " + (string)playerIDBuffer + " 为地主。";
			}
			return preRoundString;
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
				this->values[point] = value++;
			this->values[1] = value++;
			this->values[2] = value++;
			this->values[JOKER_POINT] = value++;
			this->players = vector<vector<Card>>(4);
			this->deck.clear();
			this->records.clear();
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
			for (PlayerID playerID = 0; playerID < 4; ++playerID)
			{
				this->players[playerID] = vector<Card>(25);
				for (CardID cardID = 0; cardID < 25; ++cardID)
				{
					this->players[playerID][cardID] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[playerID]);
			}
			this->records.clear();
			this->status = Status::Dealt;
			this->assignDealer();
			return true;
		}
		else
			return false;
	}
	bool setLandlord(LandlordScore s0, LandlordScore s1, LandlordScore s2, LandlordScore s3) override final
	{
		if (Status::Dealt == this->status && this->records.size() == 1 && this->records[0].size() == 1 && this->records[0][0].cards.empty() && this->players.size() == 4)
		{
			PlayerID playerID = this->records[0].back().playerID;
			LandlordScore currentHighestScore = s0, landlordScores[4] = { s0, s1, s2, s3 };
			this->records[0].back().cards = vector<Card>{ Card{ static_cast<Point>(s0) } };
			this->getNextPlayer(playerID);
			for (size_t idx = 1; idx <= 3; ++idx)
				if (LandlordScore::Three == currentHighestScore)
					break;
				else
				{
					if (currentHighestScore < landlordScores[idx])
					{
						currentHighestScore = landlordScores[idx];
						this->records[0].push_back(Token{ playerID, vector<Card>{ Card{ static_cast<Point>(landlordScores[idx]) } } });
					}
					this->getNextPlayer(playerID);
				}
			for (size_t idx = 0; idx < 8; ++idx)
				this->players[this->records[0].back().playerID].push_back(this->deck[idx]);
			this->sortCards(this->players[this->records[0].back().playerID]);
			this->status = Status::Assigned;
			return true;
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

class BigTwo : public PokerGame
{
private:
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString = "";
			char playerIDBuffer[4] = { 0 };
			if (this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit)
			{
				snprintf(playerIDBuffer, 4, "%d", this->records[0].back().playerID + 1);
				preRoundString = "玩家 " + (string)playerIDBuffer + " 拥有最小的牌（" + (string)this->records[0].back().cards[0] + "），拥有发牌权。";
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
				this->values[point] = value++;
			this->values[1] = value++;
			this->values[2] = value++;
			this->players = vector<vector<Card>>(4);
			this->deck.clear();
			this->records.clear();
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
			for (PlayerID playerID = 0; playerID < 4; ++playerID)
			{
				this->players[playerID] = vector<Card>(13);
				for (CardID cardID = 0; cardID < 13; ++cardID)
				{
					this->players[playerID][cardID] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[playerID]);
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

class ThreeTwoOne : public PokerGame
{
private:
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString = "";
			char playerIDBuffer[4] = { 0 };
			if (this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit)
			{
				snprintf(playerIDBuffer, 4, "%d", this->records[0].back().playerID + 1);
				preRoundString = "玩家 " + (string)playerIDBuffer + " 拥有最小的牌（" + (string)this->records[0].back().cards[0] + "），拥有发牌权。";
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
				this->values[point] = value++;
			this->values[1] = value++;
			this->values[2] = value++;
			this->players = vector<vector<Card>>(4);
			this->deck.clear();
			this->records.clear();
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
			for (PlayerID playerID = 0; playerID < 4; ++playerID)
			{
				this->players[playerID] = vector<Card>(13);
				for (CardID cardID = 0; cardID < 13; ++cardID)
				{
					this->players[playerID][cardID] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[playerID]);
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

class Wuguapi : public PokerGame
{
private:
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString = "";
			char playerIDBuffer[4] = { 0 };
			for (const Token& token : this->records[0])
				if (token.cards.size() == 1)
				{
					snprintf(playerIDBuffer, 4, "%d", token.playerID + 1);
					preRoundString += (string)token.cards[0] + "（玩家 " + playerIDBuffer + "） > ";
				}
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
				this->values[point] = value++;
			for (Point point = 1; point <= 5; ++point)
				this->values[point] = value++;
			this->values[JOKER_POINT] = value++;
			this->players = vector<vector<Card>>(playerCount);
			this->deck.clear();
			this->records.clear();
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
			for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
			{
				this->players[playerID] = vector<Card>(5);
				for (CardID cardID = 0; cardID < 5; ++cardID)
				{
					this->players[playerID][cardID] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[playerID]);
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

class Qiguiwueryi : public PokerGame
{
private:
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString = "";
			char playerIDBuffer[4] = { 0 };
			for (const Token& token : this->records[0])
				if (token.cards.size() == 1)
				{
					snprintf(playerIDBuffer, 4, "%d", token.playerID + 1);
					preRoundString += (string)token.cards[0] + "（玩家 " + playerIDBuffer + "） > ";
				}
			preRoundString.erase(preRoundString.length() - 3, 3);
			return preRoundString;
		}
	}
	
public:
	Qiguiwueryi() : PokerGame()
	{
		this->pokerType = "七鬼五二一";
	}
	bool initialize() override final { return this->initialize(2); }
	bool initialize(const size_t playerCount) override final
	{
		if (this->status >= Status::Ready && 2 <= playerCount && playerCount <= 7)
		{
			Value value = 1;
			this->values[3] = value++;
			this->values[4] = value++;
			this->values[6] = value++;
			for (Point point = 8; point <= 13; ++point)
				this->values[point] = value++;
			this->values[1] = value++;
			this->values[2] = value++;
			this->values[5] = value++;
			this->values[JOKER_POINT] = value++;
			this->values[7] = value++;
			this->players = vector<vector<Card>>(playerCount);
			this->deck.clear();
			this->records.clear();
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
			for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
			{
				this->players[playerID] = vector<Card>(7);
				for (CardID cardID = 0; cardID < 7; ++cardID)
				{
					this->players[playerID][cardID] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[playerID]);
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

class Qiguiwuersan : public PokerGame
{
private:
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString = "";
			char playerIDBuffer[4] = { 0 };
			for (const Token& token : this->records[0])
				if (token.cards.size() == 1)
				{
					snprintf(playerIDBuffer, 4, "%d", token.playerID + 1);
					preRoundString += (string)token.cards[0] + "（玩家 " + playerIDBuffer + "） > ";
				}
			preRoundString.erase(preRoundString.length() - 3, 3);
			return preRoundString;
		}
	}
	
public:
	Qiguiwuersan() : PokerGame()
	{
		this->pokerType = "七鬼五二三";
	}
	bool initialize() override final { return this->initialize(2); }
	bool initialize(const size_t playerCount) override final
	{
		if (this->status >= Status::Ready && 2 <= playerCount && playerCount <= 7)
		{
			Value value = 1;
			this->values[1] = value++;
			this->values[4] = value++;
			this->values[6] = value++;
			for (Point point = 8; point <= 13; ++point)
				this->values[point] = value++;
			this->values[3] = value++;
			this->values[2] = value++;
			this->values[5] = value++;
			this->values[JOKER_POINT] = value++;
			this->values[7] = value++;
			this->players = vector<vector<Card>>(playerCount);
			this->deck.clear();
			this->records.clear();
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
			for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
			{
				this->players[playerID] = vector<Card>(7);
				for (CardID cardID = 0; cardID < 7; ++cardID)
				{
					this->players[playerID][cardID] = this->deck.back();
					this->deck.pop_back();
				}
				this->sortCards(this->players[playerID]);
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

class Interaction
{
private:
	string pokerType = "";
	size_t playerCount = 0;
	SortingMethod sortingMethod = SortingMethod::FromBigToSmall;
	const vector<string> pokerTypeOptions = { "t", "/t", "-t", "pokerType", "/pokerType", "--pokerType" };
	const vector<string> pokerTypesC = { "斗地主", "四人斗地主", "锄大地", "三两一", "五瓜皮", "七鬼五二一", "七鬼五二三" };
	const vector<string> pokerTypesE = { "Landlords", "Landlords4P", "BigTwo", "ThreeTwoOne", "Wuguapi", "Qiguiwueryi", "Qiguiwuersan" };
	const vector<string> playerCountOptions = { "p", "/p", "-p", "playerCount", "/playerCount", "--playerCount" };
	const vector<string> sortingMethodOptions = { "s", "/s", "-s", "sortingMethod", "/sortingMethod", "--sortingMethod" };
	const vector<string> helpOptions = { "?", "/?", "-?", "h", "/h", "-h", "help", "/help", "--help" };
	HelpKey helpKey = 0;
	const vector<string> landlordStatements = { "叫地主", "抢地主", "叫牌", "抢牌", "是", "Y", "Yes", "true", "1" };

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
			cout << "\t（1）键和值应当成对出现，每一个表示键的参数后面应当紧接着其对应的值；" << endl;
			cout << "\t（2）当同一键出现多次时，其值以该键最后一次出现时对应的值为准；" << endl;
			cout << "\t（3）出现多个帮助参数时，以最后一个出现帮助参数（前的参数）进行显示帮助。" << endl << endl << endl << endl;
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
		char buffer[256] = { 0 };
		fgets(buffer, 256, stdin);
		if ('\n' == buffer[strlen(buffer) - 1])
			buffer[strlen(buffer) - 1] = 0;
		description = buffer;
		return;
	}
	size_t fetchPlayerCount(size_t lowerBound, size_t upperBound) const // lowerBound cannot be 0
	{
		if (lowerBound <= this->playerCount && this->playerCount <= upperBound)
			return this->playerCount;
		else
			for (;;)
			{
				string playerCountString = "";
				cout << "该扑克牌类型支持最少 " << lowerBound << " 人，最多 " << upperBound << " 人；请输入玩家人数：";
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
		{
			this->clearScreen();
			cout << "已选定扑克牌类型：" << this->pokerType << endl;
			this->playerCount = fetchPlayerCount(2, 10);
		}
		else if ("七鬼五二一" == this->pokerType || this->isEqual("Qiguiwueryi", this->pokerType) || "七鬼五二三" == this->pokerType || this->isEqual("Qiguiwuersan", this->pokerType))
		{
			this->clearScreen();
			cout << "已选定扑克牌类型：" << this->pokerType << endl;
			this->playerCount = fetchPlayerCount(2, 7);
		}
		else
			this->playerCount = 0;
		return true;
	}
	
public:
	Interaction(const vector<string>& arguments)
	{
		if (!arguments.empty())
		{
			vector<size_t> invalidArgumentIndexes{};
			const size_t argumentCount = arguments.size() - 1;
			size_t argumentID = 0;
			for (; argumentID < argumentCount; ++argumentID)
				if (this->isIn(arguments[argumentID], this->pokerTypeOptions))
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
						this->sortingMethod = SortingMethod::FromBigToSmall;
					else if (this->isEqual("FromSmallToBig", arguments[argumentID]) || this->isEqual("1", arguments[argumentID]))
						this->sortingMethod = SortingMethod::FromSmallToBig;
					else if (this->isEqual("FromManyToFew", arguments[argumentID]) || this->isEqual("2", arguments[argumentID]))
						this->sortingMethod = SortingMethod::FromManyToFew;
					else if (this->isEqual("FromFewToMany", arguments[argumentID]) || this->isEqual("3", arguments[argumentID]))
						this->sortingMethod = SortingMethod::FromFewToMany;
					else
						invalidArgumentIndexes.push_back(argumentID);
				else if (this->isIn(arguments[argumentID], this->helpOptions))
					this->helpKey = 1;
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
				pokerGame->deal();
				PlayerID playerID = 0;
				pokerGame->getCurrentPlayer(playerID);
				if ("斗地主" == this->pokerType)
				{
					Count retryCount = 0;
					for (;;)
					{
						bool booleans[4] = { false, false, false, false }, isRobbing = false;
						for (size_t idx = 0; idx < 3; ++idx)
						{
							this->clearScreen();
							pokerGame->display();
							cout << "请玩家 " << (playerID + 1) << " 选择是否" << (isRobbing ? "抢" : "叫") << "地主：";
							string buffer = "";
							this->getDescription(buffer);
							if (this->isIn(buffer, this->landlordStatements))
							{
								booleans[idx] = true;
								isRobbing = true;
							}
							pokerGame->getNextPlayer(playerID);
						}
						if (booleans[0] && (booleans[1] || booleans[2]))
						{
							this->clearScreen();
							pokerGame->display();
							cout << "请玩家 " << (playerID + 1) << " 选择是否抢地主：";
							string buffer = "";
							this->getDescription(buffer);
							if (this->isIn(buffer, this->landlordStatements))
								booleans[3] = true;
							pokerGame->getNextPlayer(playerID);
						}
						if (booleans[0] || booleans[1] || booleans[2] || booleans[3] || retryCount >= 2)
						{
							pokerGame->setLandlord(booleans[0], booleans[1], booleans[2], booleans[3]);
							break;
						}
						else
						{
							++retryCount;
							cout << "无人叫地主，即将重新发牌。" << endl;
							this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
							pokerGame->deal();
						}
					}
				}
				else if ("四人斗地主" == this->pokerType)
				{
					Count retryCount = 0;
					for (;;)
					{
						LandlordScore landlordScores[4] = { LandlordScore::None, LandlordScore::None, LandlordScore::None, LandlordScore::None }, currentHighestScore = LandlordScore::None;
						vector<string> scoreDescriptions{ "不叫", "3分", "2分", "1分" };
						for (size_t idx = 0; idx < 4 && currentHighestScore < LandlordScore::Three; ++idx)
						{
							this->clearScreen();
							pokerGame->display();
							cout << "请玩家 " << (playerID + 1) << " 选择（" << this->vector2string(scoreDescriptions, "", " | ", "") << "）：";
							string buffer = "";
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
								if (landlordScore > currentHighestScore)
								{
									landlordScores[idx] = landlordScore;
									switch (landlordScore)
									{
									case LandlordScore::Three:
										switch (currentHighestScore)
										{
										case LandlordScore::None:
											scoreDescriptions.pop_back();
										case LandlordScore::One:
											scoreDescriptions.pop_back();
										case LandlordScore::Two:
											scoreDescriptions.pop_back();
										default:
											break;
										}
										break;
									case LandlordScore::Two:
										switch (currentHighestScore)
										{
										case LandlordScore::None:
											scoreDescriptions.pop_back();
										case LandlordScore::One:
											scoreDescriptions.pop_back();
										default:
											break;
										}
										break;
									case LandlordScore::One:
										scoreDescriptions.pop_back();
										break;
									default:
										break;
									}
									currentHighestScore = landlordScore;
								}
							}
							pokerGame->getNextPlayer(playerID);
						}
						if (landlordScores[0] >= LandlordScore::One || landlordScores[1] >= LandlordScore::One || landlordScores[2] >= LandlordScore::One || landlordScores[3] >= LandlordScore::One || retryCount >= 2)
						{
							pokerGame->setLandlord(landlordScores[0], landlordScores[1], landlordScores[2], landlordScores[3]);
							break;
						}
						else
						{
							++retryCount;
							cout << "无人叫地主，即将重新发牌。" << endl;
							this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
							pokerGame->deal();
						}
					}
				}
				pokerGame->getCurrentPlayer(playerID);
				for (;;)
				{
					this->clearScreen();
					pokerGame->display();
					cout << "请玩家 " << (playerID + 1) << " 开牌：";
					string buffer = "";
					this->getDescription(buffer);
					//if (pokerGame->start(buffer, playerID))
						break;
				}
				while (playerID != (PlayerID)(-1))
				{
					for (;;)
					{
						this->clearScreen();
						pokerGame->display();
						cout << "请玩家 " << (playerID + 1) << " 出牌：";
						string buffer = "";
						this->getDescription(buffer);
						//if (pokerGame->play(buffer, playerID, playerID))
							break;
					}
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
				cout << "错误：不支持的扑克牌类型——" << this->pokerType << "，或指定的玩家人数超出该扑克牌类型所承受的范围。" << endl << endl;
				this->printHelp();
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