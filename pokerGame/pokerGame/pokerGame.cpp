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
typedef long long int Amount;
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
	SingleStraight = 0b00010001, 
	SingleFlush = 0b00010010, 
	SingleFlushStraight = 0b00010011, 
	
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
	
	Quadruple = 0b01000000, 
	QuadrupleWithSingle = 0b01000001, 
	QuadrupleWithSingleSingle = 0b01000010, 
	QuadrupleWithPairPair = 0b01000011, 
	QuadrupleStraight = 0b01000100, 
	QuadrupleStraightWithSingle = 0b01000101, 
	QuadrupleJokers = 0b01000110, 
	
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
		string stringBuffer{};
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

	operator bool() const
	{
		return this->player != INVALID_PLAYER || this->tokenType != TokenType::Invalid;
	}
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
	Player currentPlayer = INVALID_PLAYER, dealer = INVALID_PLAYER;
	Token lastToken{};
	vector<Amount> amounts{};
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
	virtual void sortCards(vector<Card>& cards, const SortingMethod sortingMethod) const final
	{
		switch (sortingMethod)
		{
		case SortingMethod::FromSmallToBig:
			sort(cards.begin(), cards.end(), [this](const Card a, const Card b) { return this->values[a.point] < this->values[b.point] || (this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
			break;
		case SortingMethod::FromManyToFew:
		{
			Count counts[14] = { 0 };
			for (const Card& card : cards)
				if (this->values[card.point])
					++counts[card.point];
			sort(cards.begin(), cards.end(), [&counts, this](const Card a, const Card b) { return counts[a.point] > counts[b.point] || (counts[a.point] == counts[b.point] && this->values[a.point] > this->values[b.point]) || (counts[a.point] == counts[b.point] && this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
			break;
		}
		case SortingMethod::FromFewToMany:
		{
			Count counts[14] = { 0 };
			for (const Card& card : cards)
				if (this->values[card.point])
					++counts[card.point];
			sort(cards.begin(), cards.end(), [&counts, this](const Card a, const Card b) { return counts[a.point] < counts[b.point] || (counts[a.point] == counts[b.point] && this->values[a.point] > this->values[b.point]) || (counts[a.point] == counts[b.point] && this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
			break;
		}
		case SortingMethod::FromBigToSmall:
		default:
			sort(cards.begin(), cards.end(), [this](const Card a, const Card b) { return this->values[a.point] > this->values[b.point] || (this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
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
			this->dealer = this->records[0].back().player;
			this->lastToken = Token{};
			this->amounts.clear();
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
	virtual Count judgeStraight(vector<Card>& cards) const final // The vector ``cards`` must be sorted according to ``SortingMethod::FromManyToFew``. 
	{
		if (cards.empty())
			return 0;
		vector<vector<Card>> items{};
		Point lastPoint = JOKER_POINT;
		for (const Card& card : cards)
			if (JOKER_POINT == card.point)
				return false;
			else if (lastPoint == card.point)
				items.back().push_back(card);
			else
			{
				items.push_back(vector<Card>{ card });
				lastPoint = card.point;
			}
		const Count repeatedCount = static_cast<Count>(items[0].size());
		for (size_t idx = items.size() - 1; idx > 0; --idx)
			if (items[idx].size() != repeatedCount)
				return false;
		size_t smallestPointer = 0, largestPointer = items.size() - 1;
		while (smallestPointer < largestPointer)
			if (this->values[items[smallestPointer][0].point] - this->values[items[smallestPointer + 1][0].point] == 1)
				++smallestPointer;
			else
				break;
		if (smallestPointer == largestPointer)
			return true;
		while (largestPointer > smallestPointer)
			if (this->values[items[largestPointer - 1][0].point] - this->values[items[largestPointer][0].point] == 1)
				--largestPointer;
			else
				break;
		if (smallestPointer + 1 == largestPointer && items[0][0].point + 1 == items.back()[0].point)
		{
			for (size_t idx = items.size() - largestPointer; idx > 0; --idx)
			{
				items.insert(items.begin(), items.back());
				items.pop_back();
			}
			cards = vector<Card>(cards.size());
			size_t idx = 0;
			for (const vector<Card>& item : items)
				for (const Card& card : item)
					cards[idx++] = card;
			return repeatedCount;
		}
		else
			return 0;
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
	virtual bool processAmounts(const Token& token) { UNREFERENCED_PARAMETER(token); return false; }
	virtual bool isAbsolutelyLargest(const Token& token) const = 0;
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
	virtual string getBasisString() const { return ""; }
	virtual string getPreRoundString() const = 0;
	virtual bool display(const vector<Player>& selectedPlayers, const string& dealerRemark, const string& deckDescription) const final
	{
		switch (this->status)
		{
		case Status::Ready:
			cout << "牌局未初始化，请先初始化牌局。" << endl << endl;
			return true;
		case Status::Initialized:
			cout << "当前牌局（" << this->pokerType << "）已初始化，但暂未开局，请发牌或录入残局数据。" << endl << endl;
			return true;
		case Status::Dealt:
		case Status::Assigned:
		case Status::Started:
		case Status::Over:
		{
			bool flag = true;
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
			cout << "）" << endl << this->getBasisString() << endl;
			const size_t playerCount = this->players.size();
			for (const Player& player : selectedPlayers)
				if (0 <= player && player < playerCount)
					cout << "玩家 " << (player + 1) << (!this->records.empty() && !this->records[0].empty() && this->dealer == player ? "（" + dealerRemark + "）：" : "：") << endl << this->cards2string(this->players[player], "", " | ", "", "（空）") << endl << endl;
				else
					flag = false;
			cout << deckDescription;
			if (this->status >= Status::Dealt && !this->records.empty())
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
			return flag;
		}
		default:
			cout << "当前牌局状态未知，无法显示牌局状况。" << endl << endl;
			return false;
		}
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
	virtual bool initialize() = 0; // values, players (= vector<vector<Card>>(n)), deck (clear), records (clear), currentPlayer (reset), dealer (reset), lastToken (reset), amounts (clear), and status = Status::Initialized
	virtual bool initialize(const size_t playerCount) = 0; // values, players (= vector<vector<Card>>(n)), deck (clear), records (clear), currentPlayer (reset), dealer (reset), lastToken (reset), amounts (clear), and status = Status::Initialized
	virtual bool deal() = 0; // players, deck, records (clear) -> records[0], currentPlayer, dealer, lastToken (reset), amounts (clear) | amounts = vector<Amount>{ 0 }, and status = Status::Dealt | Status::Assigned
	virtual bool getCurrentPlayer(Player& player) const final // const
	{
		if ((Status::Dealt <= this->status && this->status <= Status::Started && 0 <= this->currentPlayer && this->currentPlayer < this->players.size()) || Status::Over == this->status)
		{
			player = this->currentPlayer;
			return true;
		}
		else
			return false;
	}
	virtual bool setLandlord(const bool b) { UNREFERENCED_PARAMETER(b); return false; } // records[0], currentPlayer, dealer (const) -> dealer, lastToken -> lastToken (reset), amounts[0], and status (const) -> status = Status::Assigned
	virtual bool setLandlord(const LandlordScore landlordScore) { UNREFERENCED_PARAMETER(landlordScore); return false; } // records[0], currentPlayer, dealer (const) -> dealer, lastToken -> lastToken (reset), amounts[0], and status (const) -> status = Status::Assigned
	virtual bool start(const vector<Card>& cards) final // records[1], currentPlayer, lastToken, amounts (const) | amounts[0], and status = Status::Started | Status::Over
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty() && 0 <= this->currentPlayer && this->currentPlayer < this->players.size() && this->checkStarting(cards))
		{
			Token token{ this->currentPlayer, cards };
			if (this->processToken(token) && this->removeCards(cards, this->players[this->currentPlayer]))
			{
				this->records.push_back(vector<Token>{ token });
				this->processAmounts(token);
				if (this->isOver())
				{
					this->currentPlayer = INVALID_PLAYER;
					this->lastToken = Token{};
					this->status = Status::Over;
				}
				else
				{
					if (!this->isAbsolutelyLargest(token))
						this->nextPlayer();
					this->lastToken = this->records[1][0];
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
	virtual bool start(const string& description) final // records[1], currentPlayer, lastToken, amounts (const) | amounts[0], and status = Status::Started | Status::Over
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty())
		{
			vector<Card> cards{};
			return this->description2cards(description, cards) && this->start(cards);
		}
		else
			return false;
	}
	virtual bool play(const vector<Card>& cards) final // records, currentPlayer, lastToken, amounts (const) | amounts[0], and status (const) -> status = Status::Over
	{
		if (Status::Started == this->status && this->records.size() >= 2 && !this->records.back().empty() && 0 <= this->currentPlayer && this->currentPlayer < this->players.size() && this->lastToken)
		{
			Token token{ this->currentPlayer, cards };
			if (this->processToken(token))
				if (token.player == this->lastToken.player)
					if (TokenType::Empty != token.tokenType && this->removeCards(cards, this->players[this->currentPlayer]))
						if (this->coverLastToken(token))
							this->records.back().push_back(token);
						else
							this->records.push_back(vector<Token>{ token });
					else
						return false;
				else if (TokenType::Empty == token.tokenType)
				{
					this->records.back().push_back(token);
					this->nextPlayer();
					return true;
				}
				else if (this->coverLastToken(token) && this->removeCards(token.cards, this->players[this->currentPlayer]))
					this->records.back().push_back(token);
				else
					return false;
			else
				return false;
			this->processAmounts(token);
			if (this->isOver())
			{
				this->currentPlayer = INVALID_PLAYER;
				this->lastToken = Token{};
				this->status = Status::Over;
			}
			else
			{
				if (!this->isAbsolutelyLargest(token))
					this->nextPlayer();
				this->lastToken = this->records.back().back();
			}
			return true;
		}
		else
			return false;
	}
	virtual bool play(const string& description) final // records, currentPlayer, lastToken, amounts (const) | amounts[0], and status (const) -> status = Status::Over
	{
		if (Status::Started == this->status && this->records.size() >= 2 && !this->records.back().empty())
		{
			vector<Card> cards{};
			return this->description2cards(description, cards) && this->play(cards);
		}
		else
			return false;
	}
	virtual bool getAmounts(vector<Amount>& _amounts) = 0; // amounts
	virtual bool set(const char binaryChars[]) final // values, players, deck, records, currentPlayer, dealer, lastToken, amounts, and status
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
	virtual bool display(const vector<Player>& selectedPlayers) const = 0; // const
	virtual bool display() const final // const
	{
		const size_t playerCount = this->players.size();
		vector<Player> selectedPlayers(playerCount);
		for (Player player = 0; player < playerCount; ++player)
			selectedPlayers[player] = player;
		return this->display(selectedPlayers);
	}
	virtual bool display(const Player player) const final { return INVALID_PLAYER == player ? this->display() : this->display(vector<Player>{ player }); } // const
};

class Landlords : public PokerGame /* Next: Landlords4P */
{
private:
	bool assignDealer() override final
	{
		if (Status::Dealt == this->status && this->records.empty())
		{
			this->records.push_back(vector<Token>{});
			uniform_int_distribution<size_t> distribution(0, this->players.size() - 1);
			this->currentPlayer = (Player)(distribution(this->seed));
			this->dealer = INVALID_PLAYER;
			this->lastToken = Token{};
			this->amounts = vector<Amount>{ 0b0 };
			return true;
		}
		else
			return false;
	}
	bool checkStarting(const vector<Card>& cards) const override final
	{
		return !cards.empty();
	}
	bool processToken(Token& token) const override final
	{
		return true;
	}
	virtual bool processAmounts(const Token& token) override final
	{
		if (Status::Assigned <= this->status && this->status <= Status::Started && !this->records.empty() && !this->records.back().empty() && this->amounts.size() == 1)
		{
			if (TokenType::Quadruple == token.tokenType || TokenType::PairJokers == token.tokenType)
				this->amounts[0] += token.player == this->lastToken.player ? 0b10000000 : 0b1000;
			return true;
		}
		else
			return false;
	}
	virtual bool isAbsolutelyLargest(const Token& token) const override final
	{
		return TokenType::PairJokers == token.tokenType;
	}
	bool coverLastToken(const Token& currentToken) const override final
	{
		if (this->lastToken && TokenType::Single <= this->lastToken.tokenType && this->lastToken.tokenType <= TokenType::QuadrupleWithPairPair && !this->lastToken.cards.empty() && TokenType::Single <= currentToken.tokenType && currentToken.tokenType <= TokenType::QuadrupleWithPairPair && !currentToken.cards.empty())
			switch (this->lastToken.tokenType)
			{
			case TokenType::Single: // 单张
			case TokenType::SingleStraight: // 顺子
			case TokenType::Pair: // 对子
			case TokenType::PairStraight: // 连对
			case TokenType::Triple: // 三条
			case TokenType::TripleWithSingle: // 三带一
			case TokenType::TripleWithPair: // 三带一对
			case TokenType::TripleStraight: // 飞机（不拖不带）
			case TokenType::TripleStraightWithSingles: // 飞机（带单张）
			case TokenType::TripleStraightWithPairs: // 飞机（带对子）
			case TokenType::QuadrupleWithSingleSingle: // 四带二（单张）
			case TokenType::QuadrupleWithPairPair: // 四带二（对子）
				return (TokenType::PairJokers == currentToken.tokenType || TokenType::Quadruple == currentToken.tokenType) || (currentToken.tokenType == this->lastToken.tokenType && currentToken.cards.size() == this->lastToken.cards.size() && this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point]);
			case TokenType::PairJokers: // 王炸
				return false;
			case TokenType::Quadruple: // 炸弹
				return TokenType::PairJokers == currentToken.tokenType || (TokenType::Quadruple == currentToken.tokenType && this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point]);
			default:
				return false;
			}
		else
			return false;
	}
	string getBasisString() const
	{
		if (this->amounts.size() == 1)
		{
			char buffer1[21] = { 0 }, buffer2[21] = { 0 }, buffer3[21] = { 0 };
			snprintf(buffer1, 21, "%lld", this->amounts[0] & 0b0111);
			snprintf(buffer2, 21, "%lld", (this->amounts[0] & 0b01111111) >> 3);
			snprintf(buffer3, 21, "%lld", this->amounts[0] >> 7);
			return (string)"倍数信息：当前共叫地主或抢地主 " + buffer1 + " 次；共出实炸 " + buffer2 + " 个，空炸 " + buffer3 + " 个。\n";
		}
		else
			return "";
	}
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
			if (0 == callerAndRobberCount && length >= 3)
			{
				snprintf(playerBuffer, 4, "%d", (this->records[0][0].player + 1));
				return "无人叫地主，强制玩家 " + (string)playerBuffer + " 为地主。";
			}
			else
			{
				string preRoundString{};
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
			this->dealer = INVALID_PLAYER;
			this->lastToken = Token{};
			this->amounts.clear();
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
		if (Status::Dealt == this->status && this->records.size() == 1 && 0 <= this->currentPlayer && this->currentPlayer < this->players.size() && this->amounts.size() == 1)
			switch (this->records[0].size())
			{
			case 0:
				if (b)
				{
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{} } });
					this->lastToken = this->records[0][0];
					this->amounts[0] += 0b1;
				}
				else
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
				this->nextPlayer();
				return true;
			case 1:
				if (b)
				{
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{} } });
					if (!this->lastToken)
						this->lastToken = this->records[0][1];
					this->amounts[0] += 0b1;
				}
				else
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
				this->nextPlayer();
				return true;
			case 2:
			{
				if (b)
				{
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{} } });
					this->amounts[0] += 0b1;
				}
				else
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
				Count callerAndRobberCount = 0;
				for (size_t idx = 0; idx < 3; ++idx)
					if (!this->records[0][idx].cards.empty())
						++callerAndRobberCount;
				switch (callerAndRobberCount)
				{
				case 0:
					this->currentPlayer = this->records[0][0].player;
					this->dealer = this->records[0][0].player;
					this->lastToken = Token{};
					this->status = Status::Assigned;
					return true;
				case 1:
					this->currentPlayer = this->lastToken.player;
					this->dealer = this->lastToken.player;
					this->lastToken = Token{};
					this->status = Status::Assigned;
					return true;
				case 2:
				case 3:
					this->currentPlayer = this->lastToken.player;
					return true;
				default:
					return false;
				}
			}
			case 3:
				if (b)
				{
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{} } });
					this->amounts[0] += 0b1;
				}
				else
				{
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
					this->currentPlayer = this->records[0][2].cards.empty() ? this->records[0][1].player : this->records[0][2].player;
				}
				this->dealer = this->currentPlayer;
				this->lastToken = Token{};
				this->status = Status::Assigned;
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	bool getAmounts(vector<Amount>& _amounts, const bool connectedByMultiplication, const Amount basis, const char16_t callingAndRobbing, const char16_t realBooms, const char16_t emptyBooms, const char16_t spring) override final
	{
		if (Status::Over == this->status)
		{
			switch (this->amounts.size())
			{
			case 1: // at most (9 ** (4 + 14) | 9 ** (4 + 13 + 1)) * 2 = 9 ** 18 * 2 < 1 << 59
			{
				const Amount callingAndRobbingCount = this->amounts[0] & 0b0111, realBoomCount = (this->amounts[0] & 0b01111111) >> 3, emptyBoomCount = this->amounts[0] >> 7;

				/////
			}
			case 3:
				_amounts = vector<Amount>(this->amounts);
				return true;
			default:
				return false;
			}
		}
		else
			return false;
	}
	bool getAmounts(vector<Amount>& _amounts) override final
	{
		return this->getAmounts(_amounts, true, 10, '*2', '*2', '*2', '*2');
	}
	bool display(const vector<Player>& selectedPlayers) const override final
	{
		return this->status >= Status::Assigned ? PokerGame::display(selectedPlayers, "地主", "地主牌：" + this->cards2string(this->deck, "", " | ", "（已公开）", "（空）") + "\n\n") : PokerGame::display(selectedPlayers, "拥有明牌", "地主牌：" + this->cards2string(this->deck, "", " | ", "（未公开）", "（空）") + "\n\n");
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
			this->dealer = INVALID_PLAYER;
			this->lastToken = Token{};
			this->amounts.clear();
			return true;
		}
		else
			return false;
	}
	bool checkStarting(const vector<Card>& cards) const override final
	{
		return !cards.empty();
	}
	bool processToken(Token& token) const override final
	{
		return true;
	}
	virtual bool processAmounts(const Token& token) override final
	{
		if (Status::Assigned <= this->status && this->status <= Status::Started && this->amounts.size() == 1)
		{
			switch (token.tokenType)
			{
			case TokenType::Sextuple:
			case TokenType::Septuple:
				this->amounts[0] <<= 1;
				break;
			case TokenType::Octuple:
			case TokenType::QuadrupleJokers:
				this->amounts[0] += this->amounts[0] << 1;
				break;
			default:
				break;
			}
			return true;
		}
		else
			return false;
	}
	virtual bool isAbsolutelyLargest(const Token& token) const override final
	{
		return TokenType::QuadrupleJokers == token.tokenType;
	}
	bool coverLastToken(const Token& currentToken) const override final
	{
		if (this->lastToken && TokenType::Single <= this->lastToken.tokenType && this->lastToken.tokenType <= TokenType::Octuple && !this->lastToken.cards.empty() && TokenType::Single <= currentToken.tokenType && currentToken.tokenType <= TokenType::Octuple && !currentToken.cards.empty())
			switch (this->lastToken.tokenType)
			{
			case TokenType::Single: // 单张
			case TokenType::SingleStraight: // 顺子
			case TokenType::Pair: // 对子
			case TokenType::PairStraight: // 连对
			case TokenType::Triple: // 三条
			case TokenType::TripleWithPair: // 三带一对
			case TokenType::TripleStraight: // 飞机（不拖不带）
			case TokenType::TripleStraightWithPairs: // 飞机（带对子）
				return currentToken.tokenType >= TokenType::Quintuple || TokenType::Quadruple == currentToken.tokenType || (currentToken.tokenType == this->lastToken.tokenType && currentToken.cards.size() == this->lastToken.cards.size() && this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point]);
			case TokenType::Quadruple: // 四条炸弹
				return currentToken.tokenType >= TokenType::Quintuple || (TokenType::Quadruple == currentToken.tokenType && this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point]);
			case TokenType::QuadrupleJokers: // 天王炸弹
			case TokenType::Quintuple: // 五张炸弹
			case TokenType::Sextuple: // 六张炸弹
			case TokenType::Septuple: // 七张炸弹
			case TokenType::Octuple: // 八张炸弹
				return currentToken.tokenType > this->lastToken.tokenType || (currentToken.tokenType == this->lastToken.tokenType && this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point]);
			default:
				return false;
			}
		else
			return false;
	}
	string getBasisString() const
	{
		if (this->amounts.size() == 1)
		{
			char buffer[21] = { 0 };
			snprintf(buffer, 21, "%lld", this->amounts[0]);
			return (string)"当前倍数：" + buffer;
		}
		else
			return "";
	}
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
			if (0 == callerAndRobberCount && this->records[0].size() == 4)
			{
				snprintf(playerBuffer, 4, "%d", (this->records[0][0].player + 1));
				return "无人叫地主，强制玩家 " + (string)playerBuffer + " 为地主。";
			}
			else
			{
				string preRoundString{};
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
			this->dealer = INVALID_PLAYER;
			this->lastToken = Token{};
			this->amounts.clear();
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
					return true;
				case LandlordScore::One:
				case LandlordScore::Two:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->nextPlayer();
					this->lastToken = this->records[0][0];
					return true;
				case LandlordScore::Three:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->dealer = this->currentPlayer;
					this->lastToken = Token{};
					this->amounts = vector<Amount>{ 3 };
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
					if (!this->lastToken || (this->lastToken.cards.size() == 1 && point > this->lastToken.cards[0].point))
					{
						this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
						this->nextPlayer();
						this->lastToken = this->records[0].back();
						return true;
					}
					else
						return false;
				case LandlordScore::Three:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->dealer = this->currentPlayer;
					this->lastToken = Token{};
					this->amounts = vector<Amount>{ 3 };
					this->status = Status::Assigned;
					return true;
				default:
					return false;
				}
			case 3:
				switch (landlordScore)
				{
				case LandlordScore::None:
					if (this->lastToken)
					{
						if (this->lastToken.cards.size() == 1)
						{
							this->currentPlayer = this->lastToken.player;
							this->amounts = vector<Amount>{ this->lastToken.cards[0].point };
						}
						else
							return false;
					}
					else
						this->currentPlayer = this->records[0][0].player;
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{} });
					break;
				case LandlordScore::One:
				case LandlordScore::Two:
					if (!this->lastToken || (this->lastToken.cards.size() == 1 && point > this->lastToken.cards[0].point))
					{
						this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
						this->amounts = vector<Amount>{ this->lastToken.cards[0].point };
						break;
					}
					else
						return false;
				case LandlordScore::Three:
					this->records[0].push_back(Token{ this->currentPlayer, vector<Card>{ Card{ point } } });
					this->amounts = vector<Amount>{ 3 };
					break;
				default:
					return false;
				}
				this->dealer = this->currentPlayer;
				this->lastToken = Token{};
				this->status = Status::Assigned;
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	virtual bool getAmounts(vector<Amount>& _amounts) override final
	{
		if (Status::Over == this->status)
			switch (this->amounts.size())
			{
			case 1:
			{
				if (this->players.size() != 4)
					return false;
				const Amount base = this->amounts[0];
				this->amounts = vector<Amount>(4);
				/////
			}
			case 4:
				_amounts = vector<Amount>(this->amounts);
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	bool display(const vector<Player>& selectedPlayers) const override final
	{
		return this->status >= Status::Assigned ? PokerGame::display(selectedPlayers, "地主", "地主牌：" + this->cards2string(this->deck, "", " | ", "（已公开）", "（空）") + "\n\n") : PokerGame::display(selectedPlayers, "拥有明牌", "地主牌：" + this->cards2string(this->deck, "", " | ", "（未公开）", "（空）") + "\n\n");
	}
};

class BigTwo : public PokerGame /* Previous: Landlords4P | Next: ThreeTwoOne */
{
private:
	bool processToken(Token& token) const override final
	{
		return true;
	}
	virtual bool isAbsolutelyLargest(const Token& token) const override final
	{
		return (TokenType::Single == token.tokenType || TokenType::Pair == token.tokenType || TokenType::Triple == token.tokenType || TokenType::Quadruple == token.tokenType || TokenType::SingleFlushStraight == token.tokenType) && (!token.cards.empty() && Card { 2, Suit::Spade } == token.cards[0]);
	}
	bool coverLastToken(const Token& currentToken) const override final
	{
		if (this->lastToken && TokenType::Single <= this->lastToken.tokenType && this->lastToken.tokenType <= TokenType::QuadrupleWithSingle && !this->lastToken.cards.empty() && TokenType::Single <= currentToken.tokenType && currentToken.tokenType <= TokenType::QuadrupleWithSingle && !currentToken.cards.empty())
			switch (this->lastToken.tokenType)
			{
			case TokenType::Single: // 单张
			case TokenType::SingleFlushStraight: // 一条龙/同花顺（长度只能为 5）
			case TokenType::Pair: // 对子
			case TokenType::Triple: // 三条
			case TokenType::Quadruple: // 四个
				return currentToken.tokenType == this->lastToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit));
			case TokenType::SingleStraight: // 顺子（长度只能为 5）：可被一条龙/同花顺、金刚、葫芦/俘虏、同花以及比自己大的顺子盖过
				return TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::QuadrupleWithSingle == currentToken.tokenType || TokenType::TripleWithPair == currentToken.tokenType || TokenType::SingleFlush == currentToken.tokenType || (TokenType::SingleStraight == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit)));
			case TokenType::SingleFlush: // 同花（长度只能为 5）：可被一条龙/同花顺、金刚、葫芦/俘虏以及比自己大的同花盖过
				return TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::QuadrupleWithSingle == currentToken.tokenType || TokenType::TripleWithPair == currentToken.tokenType || (TokenType::SingleFlush == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit)));
			case TokenType::TripleWithPair: // 葫芦/俘虏：可被一条龙/同花顺、金刚、以及比自己大的葫芦/俘虏盖过
				return TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::QuadrupleWithSingle == currentToken.tokenType || (TokenType::TripleWithPair == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit)));
			case TokenType::QuadrupleWithSingle: // 金刚：可被一条龙/同花顺和比自己大的金刚盖过
				return TokenType::SingleFlushStraight == currentToken.tokenType || (TokenType::QuadrupleWithSingle == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit)));
			default:
				return false;
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
			string preRoundString{};
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
			this->dealer = INVALID_PLAYER;
			this->lastToken = Token{};
			this->amounts.clear();
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
	virtual bool getAmounts(vector<Amount>& _amounts) override final
	{
		if (Status::Over == this->status)
			switch (this->amounts.size())
			{
			case 0:
			{
				if (this->players.size() != 4)
					return false;
				Count winnerCount = 0;
				this->amounts = vector<Amount>(4);
				for (size_t idx = 0; idx < 4; ++idx)
				{
					const size_t n = this->players[idx].size();
					if (n <= 0)
						++winnerCount;
					else if (n < 8)
						this->amounts[idx] = n;
					else if (8 <= n && n < 10)
						this->amounts[idx] = n << 1 << static_cast<size_t>(find(this->players[idx].begin(), this->players[idx].end(), Card{ 2, Suit::Spade }) != this->players[idx].end());
					else if (10 <= n && n < 13)
						this->amounts[idx] = (n * 3) << static_cast<size_t>(find(this->players[idx].begin(), this->players[idx].end(), Card{ 2, Suit::Spade }) != this->players[idx].end());
					else if (13 == n)
						this->amounts[idx] = n << 2 << static_cast<size_t>(find(this->players[idx].begin(), this->players[idx].end(), Card{ 2, Suit::Spade }) != this->players[idx].end());
					else
					{
						this->amounts.clear();
						return false;
					}
				}
				if (1 == winnerCount)
				{
					const Amount s = this->amounts[0] + this->amounts[1] + this->amounts[2] + this->amounts[3];
					for (size_t idx = 0; idx < 4; ++idx)
						this->amounts[idx] = s - (this->amounts[idx] << 2);
				}
				else
				{
					this->amounts.clear();
					return false;
				}
			}
			case 4:
				_amounts = vector<Amount>(this->amounts);
				return true;
			default:
				return false;
			}
		else
			return false;
	}
	bool display(const vector<Player>& selectedPlayers) const override final
	{
		return PokerGame::display(selectedPlayers, "方块3 先出", "");
	}
};

class ThreeTwoOne : public PokerGame /* Previous: BigTwo | Next: Wuguapi */
{
private:
	bool processToken(Token& token) const override final
	{
		vector<Count> counts(14);
		for (const Card& card : token.cards)
			if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(token.cards.begin(), token.cards.end(), [&counts, this](const Card a, const Card b) { return counts[a.point] > counts[b.point] || (counts[a.point] == counts[b.point] && this->values[a.point] > this->values[b.point]) || (counts[a.point] == counts[b.point] && this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		switch (token.cards.size())
		{
		case 0:
			token.tokenType = TokenType::Empty;
			return true;
		case 1:
			token.tokenType = TokenType::Single;
			return true;
		case 2:
			if (2 == counts[0])
			{
				token.tokenType = TokenType::Pair;
				return true;
			}
			else
				return false;
		case 3:
			if (3 == counts[0])
			{
				token.tokenType = TokenType::Triple;
				return true;
			}
			else if (this->judgeStraight(token.cards) == 1)
			{
				token.tokenType = TokenType::SingleStraight;
				return true;
			}
			else
				return false;
		case 4:
			if (4 == counts[0])
			{
				token.tokenType = TokenType::Quadruple;
				return true;
			}
			else
				switch (this->judgeStraight(token.cards))
				{
				case 1:
					token.tokenType = TokenType::SingleStraight;
					return true;
				case 2:
					token.tokenType = TokenType::PairStraight;
					return true;
				case 4:
					token.tokenType = TokenType::Quadruple;
					return true;
				default:
					return false;
				}
		case 5:
			if (4 == counts[0])
			{
				token.tokenType = TokenType::QuadrupleWithSingle;
				return true;
			}
			else if (3 == counts[0] && 2 == counts[1])
			{
				token.tokenType = TokenType::TripleStraightWithPairs;
				return true;
			}
			else if (this->judgeStraight(token.cards) == 1)
			{
				token.tokenType = TokenType::SingleStraight;
				return true;
			}
			else
				return false;
		case 6:
			if (4 == counts[0] && 2 == counts[1])
			{
				swap(token.cards[3], token.cards[4]);
				swap(token.cards[4], token.cards[5]);
				token.tokenType = TokenType::TripleWithPairSingle;
				return true;
			}
			else if (3 == counts[0] && 3 == counts[1])
			{
				if (this->values[token.cards[0].point] - this->values[token.cards[3].point] == 1)
					token.tokenType = TokenType::TripleStraight;
				else if (2 == token.cards[0].point && 3 == token.cards[3].point)
				{
					for (size_t idx = 0; idx < 3; ++idx)
						swap(token.cards[idx], token.cards[idx + 3]);
					token.tokenType = TokenType::TripleStraight;
				}
				else
					token.tokenType = TokenType::TripleWithPairSingle;
				return true;
			}
			else if (3 == counts[0] && 2 == counts[1]) // && 1 == counts[2]
			{
				token.tokenType = TokenType::TripleWithPairSingle;
				return true;
			}
			else
				switch (this->judgeStraight(token.cards))
				{
				case 1:
					token.tokenType = TokenType::SingleStraight;
					return true;
				case 2:
					token.tokenType = TokenType::PairStraight;
					return true;
				case 3:
					token.tokenType = TokenType::TripleStraight;
					return true;
				default:
					return false;
				}
		case 7:
			if (4 == counts[0] && 3 == counts[1])
			{
				vector<Card> bodyCards(token.cards);
				bodyCards.erase(bodyCards.begin() + 3);
				this->sortCards(bodyCards, SortingMethod::FromManyToFew);
				if (3 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards[3]);
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::TripleStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (3 == counts[0] && 3 == counts[1]) // && 1 == counts[2]
			{
				vector<Card> bodyCards(token.cards.begin(), token.cards.end() - 1);
				if (3 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards.back());
					token.cards = bodyCards;
					token.tokenType = TokenType::TripleStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (3 == counts[0] && 2 == counts[1] && 2 == counts[2])
			{
				vector<Card> bodyCards(token.cards);
				bodyCards.erase(bodyCards.begin() + 2);
				this->sortCards(bodyCards, SortingMethod::FromManyToFew);
				if (2 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards[2]);
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::PairStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (2 == counts[0] && 2 == counts[1] && 2 == counts[2]) // && 1 == counts[3]
			{
				vector<Card> bodyCards(token.cards.begin(), token.cards.end() - 1);
				if (2 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards.back());
					token.cards = bodyCards;
					token.tokenType = TokenType::PairStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (1 == this->judgeStraight(token.cards))
			{
				token.tokenType = TokenType::SingleStraight;
				return true;
			}
			else
				return false;
		case 8:
			switch (this->judgeStraight(token.cards))
			{
			case 1:
				token.tokenType = TokenType::SingleStraight;
				return true;
			case 2:
				token.tokenType = TokenType::PairStraight;
				return true;
			case 4:
				token.tokenType = TokenType::QuadrupleStraight;
				return true;
			default:
				return false;
			}
		case 9:
			if (4 == counts[0] && 4 == counts[1]) // && 1 == counts[2]
			{
				vector<Card> bodyCards(token.cards.begin(), token.cards.end() - 1);
				if (4 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards.back());
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::QuadrupleStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (3 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3])
			{
				vector<Card> bodyCards(token.cards);
				bodyCards.erase(bodyCards.begin() + 2);
				this->sortCards(bodyCards, SortingMethod::FromManyToFew);
				if (2 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards[2]);
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::PairStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (2 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3]) // && 1 == counts[4]
			{
				vector<Card> bodyCards(token.cards.begin(), token.cards.end() - 1);
				if (2 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards.back());
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::PairStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else
				switch (this->judgeStraight(token.cards))
				{
				case 1:
					token.tokenType = TokenType::SingleStraight;
					return true;
				case 3:
					token.tokenType = TokenType::TripleStraight;
					return true;
				default:
					return false;
				}
		case 10:
			if (4 == counts[0] && 3 == counts[1] && 3 == counts[2])
			{
				vector<Card> bodyCards(token.cards);
				bodyCards.erase(bodyCards.begin() + 3);
				this->sortCards(bodyCards, SortingMethod::FromManyToFew);
				if (3 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards[3]);
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::TripleStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (3 == counts[0] && 3 == counts[1] && 3 == counts[2]) // && 1 == counts[3]
			{
				vector<Card> bodyCards(token.cards.begin(), token.cards.end() - 1);
				if (3 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards.back());
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::TripleStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else
				switch (this->judgeStraight(token.cards))
				{
				case 1:
					token.tokenType = TokenType::SingleStraight;
					return true;
				case 2:
					token.tokenType = TokenType::TripleStraight;
					return true;
				default:
					return false;
				}
		case 11:
			if (3 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4])
			{
				vector<Card> bodyCards(token.cards);
				bodyCards.erase(bodyCards.begin() + 2);
				this->sortCards(bodyCards, SortingMethod::FromManyToFew);
				if (2 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards[2]);
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::PairStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (2 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4]) // && 1 == counts[5]
			{
				vector<Card> bodyCards(token.cards.begin(), token.cards.end() - 1);
				if (2 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards.back());
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::PairStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (1 == this->judgeStraight(token.cards))
			{
				token.tokenType = TokenType::SingleStraight;
				return true;
			}
			else
				return false;
		case 12:
			switch (this->judgeStraight(token.cards))
			{
			case 1:
				token.tokenType = TokenType::SingleStraight;
				return true;
			case 2:
				token.tokenType = TokenType::PairStraight;
				return true;
			case 3:
				token.tokenType = TokenType::TripleStraight;
				return true;
			case 4:
				token.tokenType = TokenType::QuadrupleStraight;
				return true;
			default:
				return false;
			}
		case 13:
			if (4 == counts[0] && 3 == counts[1] && 3 == counts[2] && 3 == counts[3])
			{
				vector<Card> bodyCards(token.cards);
				bodyCards.erase(bodyCards.begin() + 3);
				this->sortCards(bodyCards, SortingMethod::FromManyToFew);
				if (3 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards[3]);
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::TripleStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (3 == counts[0] && 3 == counts[1] && 3 == counts[2] && 3 == counts[3]) // && 1 == counts[4]
			{
				vector<Card> bodyCards(token.cards.begin(), token.cards.end() - 1);
				if (3 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards.back());
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::TripleStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (3 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5])
			{
				vector<Card> bodyCards(token.cards);
				bodyCards.erase(bodyCards.begin() + 2);
				this->sortCards(bodyCards, SortingMethod::FromManyToFew);
				if (2 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards[2]);
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::PairStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (2 == counts[0] && 2 == counts[1] && 2 == counts[2] && 2 == counts[3] && 2 == counts[4] && 2 == counts[5]) // && 1 == counts[6]
			{
				vector<Card> bodyCards(token.cards.begin(), token.cards.end() - 1);
				if (2 == this->judgeStraight(bodyCards))
				{
					bodyCards.push_back(token.cards.back());
					token.cards = vector<Card>(bodyCards);
					token.tokenType = TokenType::PairStraightWithSingle;
					return true;
				}
				else
					return false;
			}
			else if (1 == this->judgeStraight(token.cards))
			{
				token.tokenType = TokenType::SingleStraight;
				return true;
			}
			else
				return false;
		default:
			return false;
		}
	}
	virtual bool isAbsolutelyLargest(const Token& token) const override final
	{
		return TokenType::Single <= token.tokenType && token.tokenType <= TokenType::QuadrupleStraightWithSingle && !token.cards.empty() && 2 == token.cards[0].point;
	}
	bool coverLastToken(const Token& currentToken) const override final
	{
		if (this->lastToken && TokenType::Single <= this->lastToken.tokenType && this->lastToken.tokenType <= TokenType::QuadrupleStraightWithSingle && !this->lastToken.cards.empty() && TokenType::Single <= currentToken.tokenType && currentToken.tokenType <= TokenType::QuadrupleStraightWithSingle && !currentToken.cards.empty())
			return (currentToken.tokenType == this->lastToken.tokenType || (TokenType::TripleWithPairSingle == this->lastToken.tokenType && TokenType::TripleStraight == currentToken.tokenType)) && currentToken.cards.size() == this->lastToken.cards.size() && this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point];
		else
			return false;
	}
	string getPreRoundString() const override final
	{
		if (this->records.empty() || this->records[0].empty())
			return "暂无预备回合信息。";
		else
		{
			string preRoundString{};
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
			this->dealer = INVALID_PLAYER;
			this->lastToken = Token{};
			this->amounts.clear();
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
	virtual bool getAmounts(vector<Amount>& _amounts) override final
	{
		if (Status::Over == this->status)
			switch (this->amounts.size())
			{
			case 0:
			{
				if (this->players.size() != 4)
					return false;
				Player winner = INVALID_PLAYER;
				this->amounts = vector<Amount>(4);
				Amount s = 0;
				for (size_t idx = 0; idx < 4; ++idx)
				{
					switch (this->players[idx].size())
					{
					case 0:
						if (INVALID_PLAYER == winner)
							winner = static_cast<Player>(idx);
						else
						{
							this->amounts.clear();
							return false;
						}
						break;
					case 1:
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						this->amounts[idx] = -1;
						break;
					case 6:
					case 7:
					case 8:
					case 9:
						this->amounts[idx] = -2;
						break;
					case 10:
					case 11:
					case 12:
						this->amounts[idx] = -3;
						break;
					case 13:
						this->amounts[idx] = -5;
						break;
					default:
						this->amounts.clear();
						return false;
					}
					s -= this->amounts[idx];
				}
				if (INVALID_PLAYER == winner)
				{
					this->amounts.clear();
					return false;
				}
				else
					this->amounts[winner] = s;
			}
			case 4:
				_amounts = vector<Amount>(this->amounts);
				return true;
			default:
				this->amounts.clear();
				return false;
			}
		else
			return false;
	}
	bool display(const vector<Player>& selectedPlayers) const override final
	{
		return PokerGame::display(selectedPlayers, "方块3 先出", "");
	}
};

class Wuguapi : public PokerGame /* Previous: ThreeTwoOne | Next: Qiguiwueryi */
{
private:
	bool processToken(Token& token) const override final
	{
		vector<Count> counts(14);
		for (const Card& card : token.cards)
			if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(token.cards.begin(), token.cards.end(), [&counts, this](const Card a, const Card b) { return counts[a.point] > counts[b.point] || (counts[a.point] == counts[b.point] && this->values[a.point] > this->values[b.point]) || (counts[a.point] == counts[b.point] && this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		switch (token.cards.size())
		{
		case 0:
			token.tokenType = TokenType::Empty;
			return true;
		case 1:
			token.tokenType = TokenType::Single;
			return true;
		case 2:
			if (2 == counts[0])
			{
				token.tokenType = JOKER_POINT == token.cards[0].point ? TokenType::PairJokers : TokenType::Pair;
				return true;
			}
			else
				return false;
		case 3:
			if (3 == counts[0])
				if (JOKER_POINT == token.cards[0].point)
					return false;
				else
				{
					token.tokenType = TokenType::Triple;
					return true;
				}
			else
			{
				const bool isSingleStraight = this->judgeStraight(token.cards) == 1;
				bool isFlush = true;
				const Suit suit = token.cards[0].suit;
				for (size_t idx = 1; idx < 3; ++idx)
					if (suit != token.cards[idx].suit)
					{
						isFlush = false;
						break;
					}
				if (isSingleStraight)
					token.tokenType = isFlush ? TokenType::SingleFlushStraight : TokenType::SingleStraight;
				else if (isFlush)
					token.tokenType = TokenType::SingleFlush;
				else
					return false;
				return true;
			}
		case 4:
			if (4 == counts[0])
				if (JOKER_POINT == token.cards[0].point)
					return false;
				else
				{
					token.tokenType = TokenType::Quadruple;
					return true;
				}
			else
			{
				const bool isSingleStraight = this->judgeStraight(token.cards) == 1;
				bool isFlush = true;
				const Suit suit = token.cards[0].suit;
				for (size_t idx = 1; idx < 4; ++idx)
					if (suit != token.cards[idx].suit)
					{
						isFlush = false;
						break;
					}
				if (isSingleStraight)
					token.tokenType = isFlush ? TokenType::SingleFlushStraight : TokenType::SingleStraight;
				else if (isFlush)
					token.tokenType = TokenType::SingleFlush;
				else
					return false;
				return true;
			}
		case 5:
			if (4 == counts[0]) // && 1 == counts[1]
			{
				token.tokenType = TokenType::QuadrupleWithSingle;
				return true;
			}
			else if (3 == counts[0] && 2 == counts[1])
			{
				token.tokenType = TokenType::TripleStraightWithPairs;
				return true;
			}
			else
			{
				const bool isSingleStraight = this->judgeStraight(token.cards) == 1;
				bool isFlush = true;
				const Suit suit = token.cards[0].suit;
				for (size_t idx = 1; idx < 5; ++idx)
					if (suit != token.cards[idx].suit)
					{
						isFlush = false;
						break;
					}
				if (isSingleStraight)
					token.tokenType = isFlush ? TokenType::SingleFlushStraight : TokenType::SingleStraight;
				else if (isFlush)
					token.tokenType = TokenType::SingleFlush;
				else
					return false;
				return true;
			}
		default:
			return false;
		}
	}
	bool isOver() const override final
	{
		if (this->status >= Status::Started && this->deck.empty())
		{
			bool hasCards = false;
			for (const vector<Card>& cards : this->players)
				if (!cards.empty())
					if (hasCards)
						return false;
					else
						hasCards = true;
			return true;
		}
		return false;
	}
	virtual bool isAbsolutelyLargest(const Token& token) const override final
	{
		return false;
	}
	bool coverLastToken(const Token& currentToken) const override final
	{
		if (this->lastToken && TokenType::Single <= this->lastToken.tokenType && this->lastToken.tokenType <= TokenType::QuadrupleWithSingle && !this->lastToken.cards.empty() && TokenType::Single <= currentToken.tokenType && currentToken.tokenType <= TokenType::QuadrupleWithSingle && !currentToken.cards.empty())
			switch (this->lastToken.tokenType)
			{
			case TokenType::Single: // 单张
				return currentToken.tokenType == this->lastToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit));
			case TokenType::SingleStraight: // 顺子
				switch (this->lastToken.cards.size())
				{
				case 3: // 可被一条龙/同花顺、三条、同花以及比自己大的顺子盖过
					return currentToken.cards.size() == this->lastToken.cards.size() && (TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::Triple == currentToken.tokenType || TokenType::SingleFlush == currentToken.tokenType || (TokenType::SingleStraight == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit))));
				case 4: // 可被一条龙/同花顺、四条、同花以及比自己大的顺子盖过
					return currentToken.cards.size() == this->lastToken.cards.size() && (TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::Quadruple == currentToken.tokenType || TokenType::SingleFlush == currentToken.tokenType || (TokenType::SingleStraight == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit))));
				case 5: // 可被一条龙/同花顺、金刚、葫芦/俘虏、同花以及比自己大的顺子盖过
					return currentToken.cards.size() == this->lastToken.cards.size() && (TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::QuadrupleWithSingle == currentToken.tokenType || TokenType::TripleWithPair == currentToken.tokenType || TokenType::SingleFlush == currentToken.tokenType || (TokenType::SingleStraight == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit))));
				default:
					return false;
				}
			case TokenType::SingleFlush: // 同花
				switch (this->lastToken.cards.size())
				{
				case 3: // 可被一条龙/同花顺、三条、以及比自己大的同花盖过
					return currentToken.cards.size() == this->lastToken.cards.size() && (TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::Triple == currentToken.tokenType || (TokenType::SingleFlush == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit))));
				case 4: // 可被一条龙/同花顺、四条、以及比自己大的同花盖过
					return currentToken.cards.size() == this->lastToken.cards.size() && (TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::Quadruple == currentToken.tokenType || (TokenType::SingleFlush == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit))));
				case 5: // 可被一条龙/同花顺、金刚、葫芦/俘虏、以及比自己大的同花盖过
					return currentToken.cards.size() == this->lastToken.cards.size() && (TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::QuadrupleWithSingle == currentToken.tokenType || TokenType::TripleWithPair == currentToken.tokenType || (TokenType::SingleFlush == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit))));
				default:
					return false;
				}
			case TokenType::SingleFlushStraight: // 一条龙/同花顺
				return currentToken.tokenType == this->lastToken.tokenType && currentToken.cards.size() == this->lastToken.cards.size() && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit));
			case TokenType::Pair: // 对子
				return TokenType::PairJokers == currentToken.tokenType || (currentToken.tokenType == this->lastToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit)));
			case TokenType::PairJokers: // 对鬼
				return false;
			case TokenType::Triple: // 三条：可被一条龙/同花顺和比自己大的三条盖过
				return (TokenType::SingleFlushStraight == currentToken.tokenType && currentToken.cards.size() == 3) || (TokenType::Triple == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit)));
			case TokenType::TripleWithPair: // 葫芦/俘虏：可被一条龙/同花顺、金刚、以及比自己大的葫芦/俘虏盖过
				return TokenType::SingleFlushStraight == currentToken.tokenType || TokenType::QuadrupleWithSingle == currentToken.tokenType || (TokenType::TripleWithPair == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit)));
			case TokenType::Quadruple: // 四条：可被一条龙/同花顺和比自己大的四条盖过
				return (TokenType::SingleFlushStraight == currentToken.tokenType && currentToken.cards.size() == 4) || (TokenType::Quadruple == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit)));
			case TokenType::QuadrupleWithSingle: // 金刚：可被一条龙/同花顺和比自己大的金刚盖过
				return TokenType::SingleFlushStraight == currentToken.tokenType || (TokenType::QuadrupleWithSingle == currentToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit)));
			default:
				return false;
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
			string preRoundString{};
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
			this->dealer = INVALID_PLAYER;
			this->lastToken = Token{};
			this->amounts.clear();
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
	bool getAmounts(vector<Amount>& _amounts) override final
	{
		if (Status::Over == this->status)
		{
			if (this->amounts.size() != this->players.size())
			{

				/////
			}
			_amounts = vector<Amount>(this->amounts);
			return true;
		}
		else
			return false;
	}
	bool display(const vector<Player>& selectedPlayers) const override final
	{
		return PokerGame::display(selectedPlayers, "最小先出", "牌堆（自下往上）：" + this->cards2string(this->deck, "", " | ", "", "（空）") + "\n\n");
	}
};

class Qiguiwueryi : public PokerGame /* Previous: ThreeTwoOne | Next: Qiguiwuersan */
{
private:
	bool processToken(Token& token) const override final
	{
		vector<Count> counts(14);
		for (const Card& card : token.cards)
			if (this->values[card.point])
				++counts[card.point];
			else
				return false;
		sort(token.cards.begin(), token.cards.end(), [&counts, this](const Card a, const Card b) { return counts[a.point] > counts[b.point] || (counts[a.point] == counts[b.point] && this->values[a.point] > this->values[b.point]) || (counts[a.point] == counts[b.point] && this->values[a.point] == this->values[b.point] && a.suit > b.suit); });
		sort(counts.begin(), counts.end(), [](const Count a, const Count b) { return a > b; });
		switch (token.cards.size())
		{
		case 0:
			token.tokenType = TokenType::Empty;
			return true;
		case 1:
			token.tokenType = TokenType::Single;
			return true;
		case 2:
			if (2 == counts[0])
			{
				token.tokenType = JOKER_POINT == token.cards[0].point ? TokenType::PairJokers : TokenType::Pair;
				return true;
			}
			else
				return false;
		case 3:
			if (3 == counts[0])
			{
				token.tokenType = TokenType::Triple;
				return true;
			}
			else
				return false;
		case 4:
			if (4 == counts[0])
			{
				token.tokenType = TokenType::Quadruple;
				return true;
			}
			else
				return false;
		default:
			return false;
		}
	}
	bool isOver() const override final
	{
		if (this->status >= Status::Started && this->deck.empty())
		{
			bool hasCards = false;
			for (const vector<Card>& cards : this->players)
				if (!cards.empty())
					if (hasCards)
						return false;
					else
						hasCards = true;
			return true;
		}
		return false;
	}
	virtual bool isAbsolutelyLargest(const Token& token) const override final
	{
		return (TokenType::Single == token.tokenType || TokenType::Pair == token.tokenType || TokenType::Triple == token.tokenType || TokenType::Quadruple == token.tokenType) && (!token.cards.empty() && Card { 7, Suit::Spade } == token.cards[0]);
	}
	bool coverLastToken(const Token& currentToken) const override final
	{
		if (this->lastToken && TokenType::Single <= this->lastToken.tokenType && this->lastToken.tokenType <= TokenType::Quadruple && !this->lastToken.cards.empty() && TokenType::Single <= currentToken.tokenType && currentToken.tokenType <= TokenType::Quadruple && !currentToken.cards.empty())
			switch (this->lastToken.tokenType)
			{
			case TokenType::Single:
			case TokenType::Triple:
			case TokenType::Quadruple:
				return currentToken.tokenType == this->lastToken.tokenType && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit));
			case TokenType::Pair:
			case TokenType::PairJokers:
				return (TokenType::Pair == currentToken.tokenType || TokenType::PairJokers == currentToken.tokenType) && (this->values[currentToken.cards[0].point] > this->values[this->lastToken.cards[0].point] || (currentToken.cards[0].point == this->lastToken.cards[0].point && currentToken.cards[0].suit > this->lastToken.cards[0].suit));
			default:
				return false;
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
			string preRoundString{};
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
			this->dealer = INVALID_PLAYER;
			this->lastToken = Token{};
			this->amounts.clear();
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
	bool getAmounts(vector<Amount>& _amounts) override final
	{
		if (Status::Over == this->status)
		{
			if (this->amounts.size() != this->players.size())
			{

				/////
			}
			_amounts = vector<Amount>(this->amounts);
			return true;
		}
		else
			return false;
	}
	bool display(const vector<Player>& selectedPlayers) const override final
	{
		return PokerGame::display(selectedPlayers, "最小先出", "牌堆（自下往上）：" + this->cards2string(this->deck, "", " | ", "", "（空）") + "\n\n");
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
			this->dealer = INVALID_PLAYER;
			this->lastToken = Token{};
			this->amounts.clear();
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
	string pokerType = "扑克牌";
	const vector<string> playerCountOptions = { "p", "/p", "-p", "playerCount", "/playerCount", "--playerCount" };
	size_t playerCount = 0;
	const vector<string> sortingMethodOptions = { "s", "/s", "-s", "sortingMethod", "/sortingMethod", "--sortingMethod" };
	vector<SortingMethod> sortingMethods{};
	PokerGame* pokerGame = nullptr;
	const vector<string> landlordStatements = { "Y", "yes", "1", "T", "true", "是", "叫", "叫地主", "叫牌", "抢", "抢地主", "抢牌" };
	const vector<string> returnStatements = { "R", "Return", "返回", "返回主面板", "返回主界面" };
	const vector<string> exitStatements = { "Exit", "Ctrl+C", "Ctrl + C", "退出", "退出程序" };

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
				string playerCountString{};
				cout << "请输入玩家人数（输入“/”并按下回车键将使用默认值）：";
				this->getDescription(playerCountString);
				if ("/" == playerCountString)
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
				string _pokerType{};
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
	bool ensureAction(const string& buffer, const string& actionDescriptioin) const
	{
		string repeatedBuffer{};
		cout << "您确定要" << actionDescriptioin << "吗？请再次输入以上指令以确认：";
		this->getDescription(repeatedBuffer);
		return buffer == repeatedBuffer;
	}
	bool setLandlord(bool& exitFlag) const
	{
		Player player = INVALID_PLAYER;
		string buffer{};
		Count retryCount = 0;
		for (;;)
		{
			bool isRobbing = false;
			Count callerAndRobberCount = 0;
			for (Count count = 0; count < 3;)
			{
				this->pokerGame->getCurrentPlayer(player);
				this->clearScreen();
				this->pokerGame->display(this->sortingMethods.empty() ? INVALID_PLAYER : player);
				cout << "请玩家 " << (player + 1) << " 选择是否" << (isRobbing ? "抢" : "叫") << "地主：";
				this->getDescription(buffer);
				if (this->isIn(buffer, returnStatements))
				{
					if (this->ensureAction(buffer, "返回"))
						return false;
				}
				else if (this->isIn(buffer, exitStatements))
				{
					if (this->ensureAction(buffer, "退出程序"))
					{
						exitFlag = true;
						return false;
					}
				}
				else if (!buffer.empty())
					if (this->isIn(buffer, this->landlordStatements))
					{
						if (this->pokerGame->setLandlord(true))
						{
							isRobbing = true;
							++callerAndRobberCount;
							++count;
						}
					}
					else if (this->pokerGame->setLandlord(false))
						++count;
			}
			if (callerAndRobberCount >= 2)
				for (;;)
				{
					this->pokerGame->getCurrentPlayer(player);
					this->clearScreen();
					this->pokerGame->display(this->sortingMethods.empty() ? INVALID_PLAYER : player);
					cout << "请玩家 " << (player + 1) << " 选择是否抢地主：";
					this->getDescription(buffer);
					if (this->isIn(buffer, returnStatements))
					{
						if (this->ensureAction(buffer, "返回"))
							return false;
					}
					else if (this->isIn(buffer, exitStatements))
					{
						if (this->ensureAction(buffer, "退出程序"))
						{
							exitFlag = true;
							return false;
						}
					}
					else if (this->pokerGame->setLandlord(this->isIn(buffer, this->landlordStatements)))
						break;
				}
			else if (0 == callerAndRobberCount && ++retryCount < 3)
			{
				cout << "无人叫地主，即将重新发牌。" << endl;
				this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
				this->pokerGame->deal();
				continue;
			}
			break;
		}
		return true;
	}
	bool setLandlord4P(bool& exitFlag) const
	{
		Player player = INVALID_PLAYER;
		string buffer{};
		Count retryCount = 0;
		for (;;)
		{
			LandlordScore currentHighestLandlordScore = LandlordScore::None;
			vector<string> scoreDescriptions{ "不叫", "3分", "2分", "1分" };
			for (Count count = 1; count <= 4 && currentHighestLandlordScore < LandlordScore::Three;)
			{
				this->pokerGame->getCurrentPlayer(player);
				this->clearScreen();
				this->pokerGame->display(this->sortingMethods.empty() ? INVALID_PLAYER : player);
				cout << "请玩家 " << (player + 1) << " 选择（" << this->vector2string(scoreDescriptions, "", " | ", "") << "）：";
				this->getDescription(buffer);
				if (this->isIn(buffer, returnStatements))
				{
					if (this->ensureAction(buffer, "返回"))
						return false;
				}
				else if (this->isIn(buffer, exitStatements))
				{
					if (this->ensureAction(buffer, "退出程序"))
					{
						exitFlag = true;
						return false;
					}
				}
				else if (!buffer.empty())
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
					if (this->pokerGame->setLandlord(landlordScore))
					{
						if (landlordScore > currentHighestLandlordScore)
						{
							for (Count removalCount = static_cast<Count>(landlordScore) - static_cast<Count>(currentHighestLandlordScore); removalCount > 0; --removalCount)
								scoreDescriptions.pop_back();
							currentHighestLandlordScore = landlordScore;
						}
						++count;
					}
				}
			}
			if (currentHighestLandlordScore >= LandlordScore::One || retryCount >= 2)
				break;
			else
			{
				++retryCount;
				cout << "无人叫地主，即将重新发牌。" << endl;
				this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
				this->pokerGame->deal();
			}
		}
		return true;
	}
	void resetAll()
	{
		this->helpKey = 0;
		this->pokerType = "扑克牌";
		this->playerCount = 0;
		this->sortingMethods = vector<SortingMethod>{};
		if (this->pokerGame != nullptr)
		{
			delete this->pokerGame;
			this->pokerGame = nullptr;
		}
		return;
	}
	bool start(bool& exitFlag) const
	{
		Player player = INVALID_PLAYER;
		string buffer{};
		this->pokerGame->getCurrentPlayer(player);
		for (;;)
		{
			this->clearScreen();
			this->pokerGame->display(this->sortingMethods.empty() ? INVALID_PLAYER : player);
			cout << "请玩家 " << (player + 1) << " 开牌：";
			this->getDescription(buffer);
			if (this->isIn(buffer, returnStatements))
			{
				if (this->ensureAction(buffer, "返回"))
					return false;
			}
			else if (this->isIn(buffer, exitStatements))
			{
				if (this->ensureAction(buffer, "退出程序"))
				{
					exitFlag = true;
					return false;
				}
			}
			else if (this->pokerGame->start(buffer))
				break;
		}
		return true;
	}
	bool play(bool& exitFlag) const
	{
		Player player = INVALID_PLAYER;
		string buffer{};
		this->pokerGame->getCurrentPlayer(player);
		while (player != INVALID_PLAYER)
		{
			for (;;)
			{
				this->clearScreen();
				this->pokerGame->display(this->sortingMethods.empty() ? INVALID_PLAYER : player);
				cout << "请玩家 " << (player + 1) << " 出牌：";
				this->getDescription(buffer);
				if (this->isIn(buffer, returnStatements))
				{
					if (this->ensureAction(buffer, "返回"))
						return false;
				}
				else if (this->isIn(buffer, exitStatements))
				{
					if (this->ensureAction(buffer, "退出程序"))
					{
						exitFlag = true;
						return false;
					}
				}
				else if (this->pokerGame->play(buffer))
					break;
			}
			this->pokerGame->getCurrentPlayer(player);
		}
		return true;
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
			for (;;)
			{
				this->pokerGame = nullptr;
				while (nullptr == this->pokerGame)
				{
					if ("斗地主" == this->pokerType || this->isEqual("Landlords", this->pokerType)) // "三人斗地主"
						this->pokerGame = new Landlords;
					else if ("四人斗地主" == this->pokerType || this->isEqual("Landlords4P", this->pokerType)) // "四人斗地主"
						this->pokerGame = new Landlords4P;
					else if ("锄大地" == this->pokerType || this->isEqual("BigTwo", this->pokerType)) // "锄大地"
						this->pokerGame = new BigTwo;
					else if ("三两一" == this->pokerType || this->isEqual("ThreeTwoOne", this->pokerType)) // "三两一"
						this->pokerGame = new ThreeTwoOne;
					else if ("五瓜皮" == this->pokerType || this->isEqual("Wuguapi", this->pokerType)) // "五瓜皮"
						this->pokerGame = new Wuguapi;
					else if ("七鬼五二一" == this->pokerType || this->isEqual("Qiguiwueryi", this->pokerType)) // "七鬼五二一"
						this->pokerGame = new Qiguiwueryi;
					else if ("七鬼五二三" == this->pokerType || this->isEqual("Qiguiwuersan", this->pokerType)) // "七鬼五二三"
						this->pokerGame = new Qiguiwueryi;
					else if (!this->fetchPokerType())
						return true;
				}
				this->fetchPokerType();
				if (this->playerCount ? this->pokerGame->initialize(this->playerCount) : this->pokerGame->initialize())
				{
					this->clearScreen();
					cout << "请输入“/”并按下回车键开局，或录入残局库数据：";
					for (;;)
					{
						string buffer{};
						this->getDescription(buffer);
						if ("/" == buffer)
							if (this->pokerGame->deal())
								break;
							else
								cout << "开局失败！请再次尝试输入“/”并按下回车键开局，或录入残局库数据：";
						else
						{
							char binaryChars[BUFFER_SIZE] = { 0 };
							this->fetchBinaryChars(buffer, binaryChars);
							if (this->pokerGame->set(binaryChars))
								break;
							else
								cout << "录入失败！请输入“/”并按下回车键开局，或再次尝试录入残局库数据：";
						}
					}
					bool exitFlag = false;
					if ("斗地主" == this->pokerType)
					{
						if (!this->setLandlord(exitFlag))
							if (exitFlag)
								return true;
							else
							{
								this->resetAll();
								continue;
							}
					}
					else if ("四人斗地主" == this->pokerType)
					{
						if (!this->setLandlord4P(exitFlag))
							if (exitFlag)
								return true;
							else
							{
								this->resetAll();
								continue;
							}
					}
					if (!this->start(exitFlag))
					{
						if (exitFlag)
							return true;
						else
						{
							this->resetAll();
							continue;
						}
					}
					if (!this->play(exitFlag))
						if (exitFlag)
							return true;
						else
						{
							this->resetAll();
							continue;
						}
					cout << "此局已终，程序即将返回。" << endl;
					this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
					this->resetAll();
				}
				else
				{
					cout << "错误：初始化实例失败。" << endl << endl << endl << endl;
					this->resetAll();
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