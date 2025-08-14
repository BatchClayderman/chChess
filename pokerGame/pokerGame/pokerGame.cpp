#include <iostream>
#include <vector>
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
#ifndef JOKER_POINT
#define JOKER_POINT 0
#endif
#ifndef TIME_FOR_SLEEP
#define TIME_FOR_SLEEP 3
#endif
using namespace std;
typedef unsigned char Point;
typedef unsigned char PlayerID;
typedef unsigned char Value;
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
};

enum class Status
{
	Ready = 0,
	Initialized = 1,
	Dealt = 2,
	Set = 3,
	Assigned = 4, 
	Started = 5, 
	Over = 6
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
	Point point = 0; // JOKER_POINT (0) for jokers
	Suit suit = Suit::Diamond;

	friend bool operator==(const Card& a, const Card& b)
	{
		return a.point == b.point && a.suit == b.suit;
	}
};

struct Token
{
	PlayerID playerID = 0;
	vector<Card> cards{};
};


class PokerGame
{
private:
	mt19937 seed{};
	string pokerType = "";
	Value values[14] = { 0 };
	vector<vector<Card>> players{};
	vector<vector<Card>> aliases{};
	vector<Card> deck{};
	vector<vector<Token>> records{};
	vector<Card> played{};
	Status status = Status::Ready;

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
	void add52CardsToDeck()
	{
		for (Point point = 1; point <= 13; ++point)
		{
			this->deck.push_back(Card{ point, Suit::Diamond });
			this->deck.push_back(Card{ point, Suit::Club });
			this->deck.push_back(Card{ point, Suit::Heart });
			this->deck.push_back(Card{ point, Suit::Spade });
		}
		return;
	}
	void add54CardsToDeck()
	{
		this->add52CardsToDeck();
		this->deck.push_back(Card{ JOKER_POINT, Suit::Black });
		this->deck.push_back(Card{ JOKER_POINT, Suit::Red });
		return;
	}
	void sortCards(vector<Card>& cards, SortingMethod sortingMethod) const
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
	void sortCards(vector<Card>& cards) const
	{
		this->sortCards(cards, SortingMethod::FromBigToSmall);
		return;
	}
	bool getNextPlayerID(PlayerID& playerID) const
	{
		if (++playerID >= this->players.size())
			playerID = 0;
		return playerID < this->players.size();
	}
	int clearScreen() const
	{
#if defined _WIN32 || defined _WIN64
		return system("cls");
#else
		return system("clear");
#endif
	}
	string convertASingleCardToString(const Card card) const
	{
		string stringBuffer = "";
		switch (card.suit)
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
			break;
		}
		switch (card.point)
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
			stringBuffer += card.point + '0';
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
	string cards2string(const vector<Card>& cards, const string& prefix, const string& separator, const string& suffix, const string& returnIfEmpty) const
	{
		if (cards.empty())
			return returnIfEmpty;
		else
		{
			string stringBuffer = prefix + this->convertASingleCardToString(cards[0]);
			size_t length = cards.size();
			for (size_t cardID = 1; cardID < length; ++cardID)
				stringBuffer += separator + this->convertASingleCardToString(cards[cardID]);
			stringBuffer += suffix;
			return stringBuffer;
		}
	}
	string getPreRoundString() const
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备轮信息。";
		else
		{
			string preRoundString = "";
			char playerIDBuffer[4] = { 0 };
			const size_t length = this->records[0].size();
			if ("三人斗地主" == this->pokerType)
				if (length >= 2)
				{
					bool isCalling = true;
					PlayerID playerID = this->records[0][0].playerID;
					for (PlayerID idxInner = 1, idxOuter = 1; idxOuter <= 4; ++idxOuter)
					{
						if (idxInner < length && this->records[0][idxInner].playerID == playerID)
						{
							if (isCalling)
							{
								sprintf_s(playerIDBuffer, "%d", playerID + 1);
								preRoundString += "叫地主（玩家 " + (string)playerIDBuffer + "） -> ";
								isCalling = false;
							}
							else
								preRoundString += "抢地主（玩家 " + (string)playerIDBuffer + "） -> ";
							++idxInner;
						}
						else
							preRoundString += (isCalling ? "不叫（玩家 " : "不抢（玩家 ") + (string)playerIDBuffer + "） -> ";
						this->getNextPlayerID(playerID);
					}
					preRoundString.erase(preRoundString.length() - 4, 4);
				}
				else if (1 == length)
				{
					sprintf_s(playerIDBuffer, "%d", (this->records[0].back().playerID + 1));
					preRoundString += "无人叫地主，强制玩家 " + (string)playerIDBuffer + " 为地主。";
				}
				else
					preRoundString += "地主信息不详。";
			else if ("四人斗地主" == this->pokerType)
				if (length >= 2)
				{
					for (const Token& token : this->records[0])
						if (token.cards.size() == 1)
						{
							sprintf_s(playerIDBuffer, "%d", token.playerID + 1);
							switch (token.cards[0].point)
							{
							case 0:
								preRoundString += "不叫（玩家 " + (string)playerIDBuffer + "） -> ";
								break;
							case 1:
							case 2:
							case 3:
							{
								char landlordScoreBuffer[2] = { 0 };
								sprintf_s(landlordScoreBuffer, "%d", token.cards[0].point);
								preRoundString += (string)landlordScoreBuffer + " 分（玩家 " + playerIDBuffer + "） -> ";
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
					sprintf_s(playerIDBuffer, "%d", (this->records[0].back().playerID + 1));
					preRoundString += "无人叫地主，强制玩家 " + (string)playerIDBuffer + " 为地主。";
				}
			else if (("锄大地" == this->pokerType || "三两一" == this->pokerType) && this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit)
			{
				sprintf_s(playerIDBuffer, "%d", this->records[0].back().playerID + 1);
				preRoundString = "玩家 " + (string)playerIDBuffer + " 拥有最小的牌（" + this->convertASingleCardToString(this->records[0].back().cards[0]) + "），拥有发牌权。";
			}
			else if ("五瓜皮" == this->pokerType || "七鬼五二一" == this->pokerType || "七鬼五二三" == this->pokerType)
			{
				for (const Token& token : this->records[0])
					if (token.cards.size() == 1)
					{
						sprintf_s(playerIDBuffer, "%d", token.playerID + 1);
						preRoundString += this->convertASingleCardToString(token.cards[0]) + "（玩家 " + playerIDBuffer + "） > ";
					}
				preRoundString.erase(preRoundString.length() - 3, 3);
			}
			else
				preRoundString = "不详";
			return preRoundString;
		}
	}

public:
	PokerGame()
	{
		random_device rd;
		mt19937 g(rd());
		this->seed = g;
	}
	bool initialize(string __pokerType, size_t playerCount) // pokerType, values, players, and aliases
	{
		string _pokerType(__pokerType);
		_pokerType.erase(remove_if(_pokerType.begin(), _pokerType.end(), [](char ch) { return ' ' == ch || '\t' == ch || '\r' == ch || '\n' == ch; }), _pokerType.end());
		this->replaceAll(_pokerType, "D", "地");
		this->replaceAll(_pokerType, "7", "七");
		this->replaceAll(_pokerType, "5", "五");
		this->replaceAll(_pokerType, "3", "三");
		this->replaceAll(_pokerType, "2", "二");
		this->replaceAll(_pokerType, "1", "一");
		if ("斗地主" == _pokerType)
		{
			if (3 == playerCount)
				return this->initialize("三人斗地主", 3);
			else if (4 == playerCount)
				return this->initialize("四人斗地主", 4);
			else
				return false;
		}
		else if ("三人斗地主" == _pokerType || "欢乐斗地主" == _pokerType || "四人斗地主" == _pokerType) // "三人斗地主" and "四人斗地主"
		{
			this->pokerType = "四人斗地主" == _pokerType ? "四人斗地主" : "三人斗地主";
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values[point] = value++;
			this->values[1] = value++;
			this->values[2] = value++;
			this->values[JOKER_POINT] = value++;
			if ("四人斗地主" == _pokerType)
			{
				this->players = vector<vector<Card>>(4);
				this->aliases = vector<vector<Card>>(4);
			}
			else
			{
				this->players = vector<vector<Card>>(3);
				this->aliases = vector<vector<Card>>(3);
			}
			this->status = Status::Initialized;
			return true;
		}
		else if ("锄大地" == _pokerType || "大老二" == _pokerType || "三两一" == _pokerType) // "锄大地" and "三两一"
		{
			this->pokerType = "三两一" == _pokerType ? "三两一" : "锄大地";
			Value value = 1;
			for (Point point = 3; point <= 13; ++point)
				this->values[point] = value++;
			this->values[1] = value++;
			this->values[2] = value++;
			this->players = vector<vector<Card>>(4);
			this->aliases = vector<vector<Card>>(4);
			this->status = Status::Initialized;
			return true;
		}
		else if ("五瓜皮" == _pokerType && 2 <= playerCount && playerCount <= 10) // "五瓜皮"
		{
			this->pokerType = "五瓜皮";
			Value value = 1;
			for (Point point = 6; point <= 13; ++point)
				this->values[point] = value++;
			for (Point point = 1; point <= 5; ++point)
				this->values[point] = value++;
			this->values[JOKER_POINT] = value++;
			this->players = vector<vector<Card>>(playerCount);
			this->aliases = vector<vector<Card>>(playerCount);
			this->status = Status::Initialized;
			return true;
		}
		else if (("七鬼五二一" == _pokerType || "七鬼五二三" == _pokerType) && 2 <= playerCount && playerCount <= 7) // "七鬼五二一" and  "七鬼五二三"
		{
			this->pokerType = _pokerType;
			Value value = 1;
			this->values["七鬼五二一" == _pokerType ? 3 : 1] = value++;
			this->values[4] = value++;
			this->values[6] = value++;
			for (Point point = 8; point <= 13; ++point)
				this->values[point] = value++;
			this->values["七鬼五二一" == _pokerType ? 1 : 3] = value++;
			this->values[2] = value++;
			this->values[5] = value++;
			this->values[JOKER_POINT] = value++;
			this->values[7] = value++;
			this->players = vector<vector<Card>>(playerCount);
			this->aliases = vector<vector<Card>>(playerCount);
			this->status = Status::Initialized;
			return true;
		}
		return false;
	}
	bool initialize(string __pokerType) // pokerType, values, players, and aliases
	{
		string _pokerType(__pokerType);
		_pokerType.erase(remove_if(_pokerType.begin(), _pokerType.end(), [](char ch) { return ' ' == ch || '\t' == ch || '\r' == ch || '\n' == ch; }), _pokerType.end());
		this->replaceAll(_pokerType, "D", "地");
		this->replaceAll(_pokerType, "7", "七");
		this->replaceAll(_pokerType, "5", "五");
		this->replaceAll(_pokerType, "3", "三");
		this->replaceAll(_pokerType, "2", "二");
		this->replaceAll(_pokerType, "1", "一");
		if ("斗地主" == _pokerType || "三人斗地主" == _pokerType || "欢乐斗地主" == _pokerType) // "三人斗地主"
			return this->initialize("三人斗地主", 3);
		else if ("四人斗地主" == _pokerType) // "四人斗地主"
			return this->initialize("四人斗地主", 4);
		else if ("锄大地" == _pokerType || "大老二" == _pokerType || "三两一" == _pokerType) // "锄大地" and "三两一"
			return this->initialize("三两一" == _pokerType ? "三两一" : "锄大地", 4);
		else if ("五瓜皮" == _pokerType || "七鬼五二一" == _pokerType || "七鬼五二三" == _pokerType) // "五瓜皮", "七鬼五二一" and  "七鬼五二三"
			return this->initialize(_pokerType, 2);
		else
			return false;
	}
	bool deal() // players, aliases, deck, and records
	{
		if (this->status >= Status::Initialized)
		{
			if ("三人斗地主" == this->pokerType && this->players.size() == 3)
			{
				this->deck.clear();
				this->add54CardsToDeck();
				shuffle(this->deck.begin(), this->deck.end(), this->seed);
				const size_t playerCount = this->players.size();
				for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
				{
					this->players[playerID] = vector<Card>(17);
					for (CardID cardID = 0; cardID < 17; ++cardID)
					{
						this->players[playerID][cardID] = this->deck.back();
						this->deck.pop_back();
					}
					this->sortCards(this->players[playerID]);
					this->aliases[playerID] = vector<Card>(this->players[playerID]);
				}
				this->records.clear();
				this->status = Status::Dealt;
				return true;
			}
			else if ("四人斗地主" == this->pokerType && this->players.size() == 4)
			{
				this->deck.clear();
				this->add54CardsToDeck();
				this->add54CardsToDeck();
				shuffle(this->deck.begin(), this->deck.end(), this->seed);
				const size_t playerCount = this->players.size();
				for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
				{
					this->players[playerID] = vector<Card>(25);
					for (CardID cardID = 0; cardID < 25; ++cardID)
					{
						this->players[playerID][cardID] = this->deck.back();
						this->deck.pop_back();
					}
					this->sortCards(this->players[playerID]);
					this->aliases[playerID] = vector<Card>(this->players[playerID]);
				}
				this->records.clear();
				this->status = Status::Dealt;
				return true;
			}
			else if (("锄大地" == this->pokerType || "三两一" == this->pokerType) && this->players.size() == 4)
			{
				this->deck.clear();
				this->add52CardsToDeck();
				shuffle(this->deck.begin(), this->deck.end(), this->seed);
				const size_t playerCount = this->players.size();
				for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
				{
					this->players[playerID] = vector<Card>(13);
					for (CardID cardID = 0; cardID < 13; ++cardID)
					{
						this->players[playerID][cardID] = this->deck.back();
						this->deck.pop_back();
					}
					this->sortCards(this->players[playerID]);
					this->aliases[playerID] = vector<Card>(this->players[playerID]);
				}
				this->records.clear();
				this->status = Status::Dealt;
				return true;
			}
			else if ("五瓜皮" == this->pokerType)
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
					this->aliases[playerID] = vector<Card>(this->players[playerID]);
				}
				this->records.clear();
				this->status = Status::Dealt;
				return true;
			}
			else if ("七鬼五二一" == this->pokerType || "七鬼五二三" == this->pokerType)
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
					this->aliases[playerID] = vector<Card>(this->players[playerID]);
				}
				this->records.clear();
				this->status = Status::Dealt;
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}
	bool set()
	{
		if (this->status >= Status::Initialized)
		{
			this->status = Status::Set;
			return true;
		}
		else
			return false;
	}
	PlayerID getLandlord()
	{
		switch (this->status)
		{
		case Status::Dealt:
			if ("三人斗地主" == this->pokerType || "四人斗地主" == this->pokerType)
			{
				uniform_int_distribution<size_t> distribution(0, this->players.size() - 1);
				const PlayerID playerID = (PlayerID)(distribution(this->seed));
				this->records.push_back(vector<Token>{});
				this->records[0].push_back(Token{ playerID, vector<Card>{} });
				this->status = Status::Assigned;
				return playerID;
			}
			else
			{
				this->records.push_back(vector<Token>{});
				const size_t playerCount = this->players.size();
				for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
					this->records[0].push_back(Token{ playerID, vector<Card>{ this->players[playerID].back() }});
				sort(this->records[0].begin(), this->records[0].end(), [this](Token a, Token b) {return this->values[a.cards.back().point] > this->values[b.cards.back().point] || (this->values[a.cards.back().point] == this->values[b.cards.back().point] && a.cards.back().suit > b.cards.back().suit); });
				this->status = Status::Assigned;
 				return this->records[0].back().playerID;
			}
		case Status::Assigned:
		case Status::Started:
		case Status::Over:
			return !this->records.empty() && !this->records[0].empty() ? this->records[0].back().playerID : (PlayerID)(-1);
		case Status::Ready:
		case Status::Initialized:
		case Status::Set:
		default:
			return (PlayerID)(-1);
		}
	}
	bool setLandlord(bool b0, bool b1, bool b2, bool b3)
	{
		if (Status::Assigned == this->status && !this->records.empty() && this->records[0].size() == 1 && "三人斗地主" == this->pokerType && this->players.size() == 3)
		{
			PlayerID playerID = this->records[0].back().playerID;
			if (b0)
				this->records[0].push_back(Token{ playerID, vector<Card>{} });
			this->getNextPlayerID(playerID);
			if (b1)
				this->records[0].push_back(Token{ playerID, vector<Card>{} });
			this->getNextPlayerID(playerID);
			if (b2)
				this->records[0].push_back(Token{ playerID, vector<Card>{} });
			this->getNextPlayerID(playerID);
			if (b3)
				this->records[0].push_back(Token{ playerID, vector<Card>{} });
			for (size_t idx = 0; idx < 3; ++idx)
			{
				this->players[this->records[0].back().playerID].push_back(this->deck[idx]);
				this->aliases[this->records[0].back().playerID].push_back(this->deck[idx]);
			}
			this->sortCards(this->players[this->records[0].back().playerID]);
			return true;
		}
		else
			return false;
	}
	bool setLandlord(const vector<PlayerID>& playerIDs)
	{
		const size_t playerCount = this->players.size(), length = playerIDs.size();
		if (Status::Assigned == this->status && !this->records.empty() && this->records[0].size() == 1 && "三人斗地主" == this->pokerType && this->players.size() == 3 && length <= 4)
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
				this->getNextPlayerID(playerID);
			}
			return this->setLandlord(booleans[0], booleans[1], booleans[2], booleans[3]);
		}
		else
			return false;
	}
	bool setLandlord(LandlordScore s0, LandlordScore s1, LandlordScore s2, LandlordScore s3)
	{
		if (Status::Assigned == this->status && !this->records.empty() && this->records[0].size() == 1 && this->records[0][0].cards.empty() && "四人斗地主" == this->pokerType && this->players.size() == 4)
		{
			PlayerID playerID = this->records[0].back().playerID;
			LandlordScore currentHighestScore = s0, landlordScores[4] = { s0, s1, s2, s3 };
			this->records[0].back().cards = vector<Card>{ Card{ static_cast<Point>(s0) } };
			this->getNextPlayerID(playerID);
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
					this->getNextPlayerID(playerID);
				}
			for (size_t idx = 0; idx < 8; ++idx)
			{
				this->players[this->records[0].back().playerID].push_back(this->deck[idx]);
				this->aliases[this->records[0].back().playerID].push_back(this->deck[idx]);
			}
			return true;
		}
		else
			return false;
	}
	bool start(const vector<Card>& cards, PlayerID& playerID) // records
	{
		if (Status::Assigned == this->status && !cards.empty())
			if ("三人斗地主" == this->pokerType || "四人斗地主" == this->pokerType)
			{
				this->records.push_back(vector<Token>{Token{ this->records[0].back().playerID, cards }});
				playerID = this->records[0].back().playerID;
				this->getNextPlayerID(playerID);
				this->status = Status::Started;
				return true;
			}
			else if (this->records[0].back().cards.size() == 1 && find(cards.begin(), cards.end(), this->records[0].back().cards[0]) != cards.end())
			{
				this->records.push_back(vector<Token>{Token{ this->records[0].back().playerID, cards }});
				playerID = this->records[0].back().playerID;
				this->getNextPlayerID(playerID);
				this->status = Status::Started;
				return true;
			}
			else
				return false;
		else
			return false;
	}
	bool play(const Token& token, PlayerID& playerID) // records
	{
		if (Status::Started == this->status)
		{
			this->records.back().push_back(token);
			this->getNextPlayerID(playerID);
			return true;
		}
		else
			return false;
	}
	bool isOver() const
	{
		return Status::Over == this->status;
	}
	bool setSortingMethod(PlayerID playerID, SortingMethod sortingMethod)
	{
		if (0 <= playerID && playerID < this->aliases.size())
		{
			this->sortCards(this->aliases[playerID], sortingMethod);
			return true;
		}
		else
			return false;
	}
	void setSortingMethod(SortingMethod sortingMethod)
	{
		const size_t playerCount = this->aliases.size();
		for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
			this->sortCards(this->aliases[playerID], sortingMethod);
		return;
	}
	//void display(PlayerID playerID, bool doesClearScreen) const
	//{
	//	return;
	//}
	void display(bool doesClearScreen) const
	{
		if (doesClearScreen)
			this->clearScreen();
		switch (this->status)
		{
		case Status::Ready:
			cout << "牌局未初始化，请先初始化牌局。" << endl << endl;
			break;
		case Status::Initialized:
			cout << "当前牌局（" << this->pokerType << "）已初始化，但暂未开局，请发牌或录入残局数据。" << endl << endl;
			break;
		case Status::Dealt:
		case Status::Set:
		case Status::Assigned:
		case Status::Started:
		case Status::Over:
		{
			cout << "牌局：" << this->pokerType << "（";
			switch (this->status)
			{
			case Status::Dealt:
				cout << "正常发牌";
				break;
			case Status::Set:
				cout << "残局";
				break;
			case Status::Assigned:
				cout << "即将开局";
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
				cout << "玩家 " << (playerID + 1) << (!this->records.empty() && !this->records[0].empty() && this->records[0].back().playerID == playerID ? (this->pokerType == "三人斗地主" || this->pokerType == "四人斗地主" ? "（地主）：" : "（最小先出）：") : "：") << endl << this->cards2string(this->aliases[playerID], "", " | ", "", "（空）") << endl << endl;
			cout << ("三人斗地主" == this->pokerType || "四人斗地主" == this->pokerType ? "地主牌：" : "牌堆（自下往上）：") << this->cards2string(this->deck, "", " | ", "", "（空）") << endl << endl;
			if (!this->records.empty())
			{
				cout << "出牌记录：" << endl;
				cout << "预备轮：" << this->getPreRoundString() << endl;
				const size_t roundCount = this->records.size();
				for (size_t round = 1; round < roundCount; ++round)
				{
					cout << "第 " << round << " 轮：";
					if (this->records[round].empty())
						cout << "（无）" << endl;
					else
					{
						cout << this->cards2string(this->records[round][0].cards, "", " | ", "", "（空）") << "（玩家 " << (this->records[round][0].playerID + 1) << "）";
						const size_t tokenCount = this->records[round].size();
						for (size_t tokenID = 1; tokenID < tokenCount; ++tokenID)
							cout << " -> " << this->cards2string(this->records[round][tokenID].cards, "", " | ", "", "（空）") << "（玩家 " << (this->records[round][tokenID].playerID + 1) << "）";
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
};

class Algorithm
{
private:
	PokerGame* pokerGame = nullptr;
	PlayerID playerID = 0;
	
public:
	Algorithm(PokerGame& _pokerGame, PlayerID _playerID)
	{
		this->pokerGame = &_pokerGame;
		this->playerID = _playerID;
	}
	bool alanysis(vector<Card>& cards) const
	{
		cards.clear();
		return true;
	}
};

class Interaction
{
private:
	string pokerType = "三人斗地主";
	size_t playerCount = 0;
	SortingMethod sortingMethod = SortingMethod::FromBigToSmall;
	const vector<string> pokerTypeOptions = { "t", "/t", "-t", "pokerType", "/pokerType", "--pokerType" };
	const vector<string> playerCountOptions = { "p", "/p", "-p", "playerCount", "/playerCount", "--playerCount" };
	const vector<string> sortingMethodOptions = { "s", "/s", "-s", "sortingMethod", "/sortingMethod", "--sortingMethod" };
	const vector<string> helpOptions = { "h", "/h", "-h", "help", "/help", "--help" };
	HelpKey helpKey = 0;

	bool isIn(const string& s, const vector<string>& strings) const
	{
		for (const string& str : strings)
			if (0 == _stricmp(s.c_str(), str.c_str()))
				return true;
		return false;
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
			cout << "目前支持以下扑克牌类型：" << endl;
			cout << "\t（1）三人斗地主（默认）；" << endl;
			cout << "\t（2）四人斗地主；" << endl;
			cout << "\t（3）锄大地；" << endl;
			cout << "\t（4）斗地主；" << endl;
			cout << "\t（5）五瓜皮；" << endl;
			cout << "\t（6）七鬼五二一；" << endl;
			cout << "\t（7）七鬼五二三。" << endl << endl << endl << endl;
			return true;
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
	void rfstdin()
	{
		rewind(stdin);
		fflush(stdin);
		return;
	}

public:
	Interaction(const vector<string>& arguments)
	{
		if (!arguments.empty())
		{
			const size_t argumentCount = arguments.size() - 1;
			vector<size_t> invalidArgumentIndexes{};
			size_t argumentID = 0;
			for (; argumentID < argumentCount; ++argumentID)
				if (this->isIn(arguments[argumentID], this->pokerTypeOptions))
					if (this->isIn(arguments[++argumentID], this->helpOptions))
						this->helpKey = 't';
					else
						this->pokerType = arguments[argumentID];
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
					else if (0 == _stricmp("FromBigToSmall", arguments[argumentID].c_str()) || 0 == _stricmp("0", arguments[argumentID].c_str()))
						this->sortingMethod = SortingMethod::FromBigToSmall;
					else if (0 == _stricmp("FromSmallToBig", arguments[argumentID].c_str()) || 0 == _stricmp("1", arguments[argumentID].c_str()))
						this->sortingMethod = SortingMethod::FromSmallToBig;
					else if (0 == _stricmp("FromManyToFew", arguments[argumentID].c_str()) || 0 == _stricmp("2", arguments[argumentID].c_str()))
						this->sortingMethod = SortingMethod::FromManyToFew;
					else if (0 == _stricmp("FromFewToMany", arguments[argumentID].c_str()) || 0 == _stricmp("3", arguments[argumentID].c_str()))
						this->sortingMethod = SortingMethod::FromFewToMany;
					else
					{
						cout << "警告：不支持的排序显示方式——“" << arguments[argumentID] << "”，已重置为默认排序方式。" << endl;
						this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
					}
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
			PokerGame cardGame = PokerGame();
			if (this->playerCount ? cardGame.initialize(this->pokerType, this->playerCount) : cardGame.initialize(this->pokerType))
			{
				cardGame.deal();
				PlayerID playerID = cardGame.getLandlord();
				if ("三人斗地主" == this->pokerType)
					cardGame.setLandlord(true, true, true, false);
				else if ("四人斗地主" == this->pokerType)
					cardGame.setLandlord(LandlordScore::None, LandlordScore::One, LandlordScore::Three, LandlordScore::None);
				playerID = cardGame.getLandlord();
				cardGame.setSortingMethod(this->sortingMethod);
				for (;;)
				{
					cardGame.display(true);
					cout << "请玩家 " << (playerID + 1) << " 开牌：";
					string buffer = "";
					this->rfstdin();
					cin >> buffer;
					vector<Card> cards{};
					//this->string2cards(buffer, cards);
					if (cardGame.start(cards, playerID))
						break;
				}
				while (!cardGame.isOver())
				{
					for (;;)
					{
						cardGame.display(true);
						cout << "请玩家 " << (playerID + 1) << " 出牌：";
						string buffer = "";
						this->rfstdin();
						cin >> buffer;
						Token token{};
						//this->string2token(buffer, token);
						if (cardGame.play(token, playerID))
							break;
					}
				}
				return true;
			}
			else
			{
				cout << "错误：不支持的扑克牌类型——" << this->pokerType << "，或指定的玩家人数超出该扑克牌类型所承受的范围。" << endl << endl;
				this->printHelp();
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
	arguments.push_back("t");
	arguments.push_back("三两一");
	Interaction interaction = Interaction(arguments);
	return interaction.interact() ? EXIT_SUCCESS : EXIT_FAILURE;
}