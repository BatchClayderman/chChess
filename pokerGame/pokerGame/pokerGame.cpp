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

enum class SortingMethod
{
	FromBigToSmall = 0,
	FromSmallToBig = 1,
	FromManyToFew = 2,
	FromFewToMany = 3
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

	Unspecified = 0b11110000, 
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
	Point point = 0; // JOKER_POINT (0) for jokers
	Suit suit = Suit::Diamond;

	friend bool operator==(const Card& a, const Card& b)
	{
		return a.point == b.point && a.suit == b.suit;
	}
};

struct Displayer
{
	vector<Card> cards{};
	SortingMethod sortingMethod = SortingMethod::FromBigToSmall;
};

struct Token
{
	PlayerID playerID = 0;
	vector<Card> cards{};
	TokenType tokenType = TokenType::Unspecified;
	Card indicator = Card{};
};


class PokerGame
{
private:
	mt19937 seed{};
	string pokerType = "";
	Value values[14] = { 0 };
	vector<vector<Card>> players{};
	vector<Displayer> displayers{};
	vector<Card> deck{};
	vector<vector<Token>> records{};
	Status status = Status::Ready;

	void replaceAll(string& str, const string& oldSubString, const string& newSubString) const // only used in PokerGame::initialize
	{
		size_t pos = 0;
		while ((pos = str.find(oldSubString, pos)) != string::npos)
		{
			str.replace(pos, oldSubString.length(), newSubString);
			pos += newSubString.length();
		}
		return;
	}
	void add52CardsToDeck(vector<Card>& _deck) const // first used in PokerGame::deal
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
	void add54CardsToDeck(vector<Card>& _deck) const // first used in PokerGame::deal
	{
		this->add52CardsToDeck(_deck);
		_deck.push_back(Card{ JOKER_POINT, Suit::Black });
		_deck.push_back(Card{ JOKER_POINT, Suit::Red });
		return;
	}
	void add52CardsToDeck() // first used in PokerGame::deal
	{
		this->add52CardsToDeck(this->deck);
		return;
	}
	void add54CardsToDeck() // first used in PokerGame::deal
	{
		this->add54CardsToDeck(this->deck);
		return;
	}
	void sortCards(vector<Card>& cards, SortingMethod sortingMethod) const // first used in PokerGame::deal
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
	void sortCards(vector<Card>& cards) const // first used in PokerGame::deal
	{
		this->sortCards(cards, SortingMethod::FromBigToSmall);
		return;
	}
	void sortCards(Displayer& displayer) const // first used in PokerGame::deal
	{
		this->sortCards(displayer.cards, displayer.sortingMethod);
		return;
	}
	bool removeCards(const vector<Card>& _smallerCards, vector<Card>& largerCards, const SortingMethod sortingMethod) const // first used in PokerGame::set
	{
		vector<Card> smallerCards = vector<Card>(_smallerCards);
		this->sortCards(smallerCards, sortingMethod);
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
	bool removeCards(const vector<Card>& smallerCards, vector<Card>& largerCards) const // first used in PokerGame::set
	{
		return this->removeCards(smallerCards, largerCards, SortingMethod::FromBigToSmall);
	}
	bool removeCards(const vector<Card>& smallerCards, Displayer& displayer, const vector<Card>& alignedCards) const // first used in PokerGame::set
	{
		if (this->removeCards(smallerCards, displayer.cards, displayer.sortingMethod))
			return true;
		else
		{
			displayer.cards = vector<Card>(alignedCards);
			this->sortCards(displayer.cards, displayer.sortingMethod);
			return false; // return false if aligned
		}
	}
	bool getNextPlayerID(PlayerID& playerID) const // first used in PokerGame::setLandlord
	{
		if (++playerID >= this->players.size())
			playerID = 0;
		return playerID < this->players.size();
	}
	bool isStraight(const vector<Point>& points, const size_t cardCount, size_t& indicatorPointer) const // first used in PokerGame::start->PokerGame::processToken
	{
		if (points.size() == cardCount && cardCount >= 3)
		{
			size_t smallestPointer = 0, largestPointer = points.size() - 1;
			if (JOKER_POINT == points[smallestPointer] || JOKER_POINT == points[largestPointer])
				return false;
			else
			{
				while (smallestPointer < largestPointer)
					if (JOKER_POINT == points[smallestPointer + 1])
						return false;
					else if (this->values[points[smallestPointer]] - this->values[points[smallestPointer + 1]] == 1)
						++smallestPointer;
					else
						break;
				if (smallestPointer == largestPointer)
				{
					indicatorPointer = 0;
					return true;
				}
				while (largestPointer > smallestPointer)
					if (JOKER_POINT == points[largestPointer - 1])
						return false;
					else if (this->values[points[largestPointer - 1]] - this->values[points[largestPointer]] == 1)
						--largestPointer;
					else
						break;
				if (smallestPointer + 1 == largestPointer && points[0] + 1 == points.back())
				{
					indicatorPointer = largestPointer;
					return true;
				}
				else
					return false;
			}
		}
		else
			return false;
	}
	bool processToken(Token& token) const // first used in PokerGame::start
	{
		const PlayerID playerID = token.playerID;
		if (0 <= playerID && playerID < this->players.size() && token.cards.size() <= this->players[playerID].size())
		{
			this->sortCards(token.cards, SortingMethod::FromManyToFew);
			vector<Point> points{};
			vector<Count> counts{};
			for (const Card& card : token.cards)
				if (points.empty() || counts.empty())
				{
					points.push_back(card.point);
					counts.push_back(1);
				}
				else if (points.back() == card.point)
					counts.back() += 1;
			switch (token.cards.size())
			{
			case 0:
				token.tokenType = TokenType::Empty;
				break;
			case 1:
				token.tokenType = TokenType::Single;
				token.indicator = token.cards[0];
				break;
			case 2:
				if (points.size() == 1 && counts.size() == 1 && 2 == counts[0])
				{
					token.tokenType = TokenType::Pair;
					token.indicator = token.cards[0];
				}
				else
					token.tokenType = TokenType::Invalid;
				break;
			case 3:
				if (points.size() == 1 && counts.size() == 1 && 3 == counts[0])
				{
					token.tokenType = TokenType::Triple;
					token.indicator = token.cards[0];
				}
				else if (points.size() == 3 && counts.size() == 3 && 1 == counts[0] && 1 == counts[1] && 1 == counts[2])
				{
					size_t indicatorPointer = 0;
					if (this->isStraight(points, 3, indicatorPointer))
						if ("三两一" == this->pokerType)
						{
							token.tokenType = TokenType::Straight;
							token.indicator = token.cards[indicatorPointer];
							return false;
						}
						else if ("五瓜皮" == this->pokerType)
						{
							token.tokenType = TokenType::Straight;
							token.indicator = token.cards[0];
							return true;
						}
						else
						{
							token.tokenType = TokenType::Invalid;
							return false;
						}
					else
					{
						token.tokenType = TokenType::Invalid;
						return false;
					}
				}
				break;
			case 4:
				if (points.size() == 1 && counts.size() == 1 && 4 == counts[0])
				{
					token.tokenType = TokenType::Quadruple‌;
					token.indicator = token.cards[0];
				}
				else if (points.size() == 2 && counts.size() == 2 && 3 == counts[0] && 1 == counts[1])
				{
					token.tokenType = TokenType::TripleWithSingle;
					token.indicator = token.cards[0];
				}
			default:
				token.tokenType = TokenType::Invalid;
			}
			return token.tokenType <= TokenType::Octuple;
		}
		else
			return false;
	}
	bool canCover(const Token& currentToken, const Token& previousToken) const // first used in PokerGame::start
	{
		if (currentToken.tokenType < TokenType::Unspecified && previousToken.tokenType < TokenType::Unspecified)
		{
			if ("三人斗地主" == this->pokerType || "四人斗地主" == this->pokerType)
				return ((TokenType::Quadruple‌ == currentToken.tokenType || currentToken.tokenType >= TokenType::Quintuple) && TokenType::Quadruple‌ != previousToken.tokenType && previousToken.tokenType < TokenType::Quintuple) || (currentToken.tokenType == previousToken.tokenType && currentToken.cards.size() == previousToken.cards.size() && this->values[currentToken.indicator.point] > this->values[previousToken.indicator.point]);
			else if ("三两一" == this->pokerType)
				return currentToken.tokenType == previousToken.tokenType && currentToken.cards.size() == previousToken.cards.size() && this->values[currentToken.indicator.point] > this->values[previousToken.indicator.point];
			else//else if ("锄大地" == this->pokerType || "五瓜皮" == this->pokerType || "七鬼五二一" == this->pokerType || "七鬼五二三" == this->pokerType)
				return currentToken.tokenType == previousToken.tokenType && currentToken.cards.size() == previousToken.cards.size() && (this->values[currentToken.indicator.point] > this->values[previousToken.indicator.point] || (this->values[currentToken.indicator.point] == this->values[previousToken.indicator.point] && currentToken.indicator.suit > previousToken.indicator.suit));
			//else
			//	return false;
		}
		else
			return false;
	}
	bool description2token(const string& description, Token& token) const // first used in PokerGame::start
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
	int clearScreen() const // first used PokerGame::display
	{
#if defined _WIN32 || defined _WIN64
		return system("cls");
#else
		return system("clear");
#endif
	}
	string convertASingleCardToString(const Card card) const // first used in PokerGame::display
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
	string cards2string(const vector<Card>& cards, const string& prefix, const string& separator, const string& suffix, const string& returnIfEmpty) const // first used in PokerGame::display
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
	string getPreRoundString() const // first used in PokerGame::display
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
								snprintf(playerIDBuffer, 4, "%d", playerID + 1);
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
					snprintf(playerIDBuffer, 4, "%d", (this->records[0].back().playerID + 1));
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
								char landlordScoreBuffer[4] = { 0 };
								snprintf(landlordScoreBuffer, 4, "%d", token.cards[0].point);
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
					snprintf(playerIDBuffer, 4, "%d", (this->records[0].back().playerID + 1));
					preRoundString += "无人叫地主，强制玩家 " + (string)playerIDBuffer + " 为地主。";
				}
			else if (("锄大地" == this->pokerType || "三两一" == this->pokerType) && this->records[0].back().cards.size() == 1 && 1 == this->values[this->records[0].back().cards[0].point] && Suit::Diamond == this->records[0].back().cards[0].suit)
			{
				snprintf(playerIDBuffer, 4, "%d", this->records[0].back().playerID + 1);
				preRoundString = "玩家 " + (string)playerIDBuffer + " 拥有最小的牌（" + this->convertASingleCardToString(this->records[0].back().cards[0]) + "），拥有发牌权。";
			}
			else if ("五瓜皮" == this->pokerType || "七鬼五二一" == this->pokerType || "七鬼五二三" == this->pokerType)
			{
				for (const Token& token : this->records[0])
					if (token.cards.size() == 1)
					{
						snprintf(playerIDBuffer, 4, "%d", token.playerID + 1);
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
	bool initialize(string __pokerType, size_t playerCount) // pokerType, values, players (= vector<vector<Card>>(n)), displayers (= vector<vector<Card>>(n)), deck (clear), and records (clear)
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
				this->displayers = vector<Displayer>(4);
			}
			else
			{
				this->players = vector<vector<Card>>(3);
				this->displayers = vector<Displayer>(3);
			}
			this->deck.clear();
			this->records.clear();
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
			this->displayers = vector<Displayer>(4);
			this->deck.clear();
			this->records.clear();
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
			this->displayers = vector<Displayer>(playerCount);
			this->deck.clear();
			this->records.clear();
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
			this->displayers = vector<Displayer>(playerCount);
			this->deck.clear();
			this->records.clear();
			this->status = Status::Initialized;
			return true;
		}
		return false;
	}
	bool initialize(string __pokerType) // pokerType, values, players (= vector<vector<Card>>(n)), displayers (= vector<vector<Card>>(n)), deck (clear), and records (clear)
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
	bool deal() // players, displayers, deck, and records (clear)
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
					this->displayers[playerID] = Displayer{ this->players[playerID] };
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
					this->displayers[playerID] = Displayer{ this->players[playerID] };
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
					this->displayers[playerID] = Displayer{ this->players[playerID] };
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
					this->displayers[playerID] = Displayer{ this->players[playerID] };
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
					this->displayers[playerID] = Displayer{ this->players[playerID] };
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
	bool set(const vector<vector<Card>>& _players, const vector<Card> _deck, const vector<vector<Token>>& _records) // players, displayers, deck, and records
	{
		if (this->status >= Status::Initialized)
		{
			/* Checking */
			vector<Card> universalDeck{};
			if (("三人斗地主" == this->pokerType && this->players.size() == 3 && _players.size() == 3) || ("五瓜皮" == this->pokerType || "七鬼五二一" == this->pokerType || "七鬼五二三" == this->pokerType) && this->players.size() >= 2 && _players.size() >= 2)
				this->add54CardsToDeck(universalDeck);
			else if ("四人斗地主" == this->pokerType && this->players.size() == 4 && _players.size() == 4)
			{
				this->add54CardsToDeck(universalDeck);
				this->add54CardsToDeck(universalDeck);
			}
			else if (("锄大地" == this->pokerType || "三两一" == this->pokerType) && this->players.size() == 4 && _players.size() == 4)
				this->add52CardsToDeck(universalDeck);
			else
				return false;
			for (const vector<Card>& cards : _players)
				if (!this->removeCards(cards, universalDeck))
					return false;
			if (!this->removeCards(_deck, universalDeck))
				return false;
			const size_t playerCount = _players.size(), roundCount = _records.size();
			Status _status = Status::Dealt;
			if (roundCount >= 2)
			{
				if (_records[0].empty() || _records[1].empty() || _records[0].back().playerID != _records[1][0].playerID)
					return false;
				for (size_t round = 1; roundCount; ++round)
					for (const Token& token : _records[round])
						if (token.playerID >= playerCount || token.cards.empty() || !this->removeCards(token.cards, universalDeck))
							return false;
				_status = Status::Started;
			}
			else if (1 == roundCount && !_records[0].empty())
				_status = Status::Assigned;

			/* Importing */
			if (universalDeck.empty())
			{
				this->players = vector<vector<Card>>(playerCount);
				for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
				{
					this->players[playerID] = vector<Card>(_players[playerID]);
					this->displayers[playerID] = Displayer{ vector<Card>(_players[playerID]) };
					this->sortCards(this->players[playerID], SortingMethod::FromBigToSmall);
					this->sortCards(this->displayers[playerID]);
				}
				this->deck = vector<Card>(_deck);
				if (1 == roundCount && !_records[0].empty())
					this->records.clear();
				else
				{
					this->records = vector<vector<Token>>(roundCount);
					for (size_t round = 0; round < roundCount; ++round)
						this->records[round] = vector<Token>(_records[round]);
				}
				this->status = _status;
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}
	bool setSortingMethod(PlayerID playerID, SortingMethod sortingMethod) // displayers
	{
		if (0 <= playerID && playerID < this->displayers.size())
		{
			this->displayers[playerID].sortingMethod = sortingMethod;
			this->sortCards(this->displayers[playerID]);
			return true;
		}
		else
			return false;
	}
	void setSortingMethod(SortingMethod sortingMethod) // displayers
	{
		const size_t playerCount = this->displayers.size();
		for (PlayerID playerID = 0; playerID < playerCount; ++playerID)
		{
			this->displayers[playerID].sortingMethod = sortingMethod;
			this->sortCards(this->displayers[playerID]);
		}
		return;
	}
	PlayerID getLandlord() // records
	{
		switch (this->status)
		{
		case Status::Dealt:
			if (this->players.empty())
				return (PlayerID)(-1);
			else if ("三人斗地主" == this->pokerType || "四人斗地主" == this->pokerType)
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
					this->records[0].push_back(Token{ playerID, vector<Card>{ this->players[playerID].back() } });
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
		default:
			return (PlayerID)(-1);
		}
	}
	bool setLandlord(bool b0, bool b1, bool b2, bool b3) // records
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
				this->displayers[this->records[0].back().playerID].cards.push_back(this->deck[idx]);
			}
			this->sortCards(this->players[this->records[0].back().playerID]);
			return true;
		}
		else
			return false;
	}
	bool setLandlord(const vector<PlayerID>& playerIDs) // records
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
	bool setLandlord(LandlordScore s0, LandlordScore s1, LandlordScore s2, LandlordScore s3) // records
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
				this->displayers[this->records[0].back().playerID].cards.push_back(this->deck[idx]);
			}
			return true;
		}
		else
			return false;
	}
	bool start(Token& token, PlayerID& playerID) // records
	{
		if (Status::Assigned == this->status && this->records.size() == 1 && !this->records[0].empty() && token.playerID == this->records[0].back().playerID && !token.cards.empty() && ("三人斗地主" == this->pokerType || "四人斗地主" == this->pokerType || (this->records[0].back().cards.size() == 1 && find(token.cards.begin(), token.cards.end(), this->records[0].back().cards[0]) != token.cards.end())) && this->processToken(token) && this->removeCards(token.cards, this->players[token.playerID]))
		{
			this->removeCards(token.cards, this->displayers[token.playerID], this->players[playerID]);
			this->records.push_back(vector<Token>{ token });
			if (this->players[playerID].empty())
			{
				playerID = (PlayerID)(-1);
				this->status = Status::Over;
			}
			else
			{
				playerID = token.playerID;
				this->getNextPlayerID(playerID);
				this->status = Status::Started;
			}
			return true;
		}
		else
			return false;
	}
	bool start(const string& description, PlayerID& playerID) // records
	{
		if (Status::Assigned == this->status)
		{
			Token token{ this->records[0].back().playerID, vector<Card>{} };
			return this->description2token(description, token) ? this->start(token, playerID) : false;
		}
		else
			return false;
	}
	bool play(Token& token, PlayerID& playerID) // records
	{
		if (Status::Started == this->status && this->records.size() >= 2 && !this->records.back().empty() && this->processToken(token))
			if (TokenType::Empty == token.tokenType)
				if (token.playerID == this->records.back().back().playerID) // the starter for the new round
					return false;
				else
				{
					playerID = token.playerID;
					this->getNextPlayerID(playerID);
					return true;
				}
			else if (this->canCover(token, this->records.back().back()))
				if (this->removeCards(token.cards, this->players[token.playerID]))
				{
					this->removeCards(token.cards, this->displayers[token.playerID], this->players[playerID]);
					this->records.back().push_back(token);
					if (this->players[playerID].empty())
					{
						playerID = (PlayerID)(-1);
						this->status = Status::Over;
					}
					else
					{
						playerID = token.playerID;
						this->getNextPlayerID(playerID);
					}
					return true;
				}
				else
					return false;
			else if (token.playerID == this->records.back().back().playerID)
				if (this->removeCards(token.cards, this->players[token.playerID]))
				{
					this->removeCards(token.cards, this->displayers[token.playerID], this->players[playerID]);
					this->records.push_back(vector<Token>{ token }); // new round
					if (this->players[playerID].empty())
					{
						playerID = (PlayerID)(-1);
						this->status = Status::Over;
					}
					else
					{
						playerID = token.playerID;
						this->getNextPlayerID(playerID);
					}
					return true;
				}
				else
					return false;
			else
				return false;
		else
			return false;
	}
	bool play(const string& description, const PlayerID inputPlayerID, PlayerID& outputPlayerID) // records
	{
		if (Status::Started == this->status)
		{
			Token token{ inputPlayerID, vector<Card>{} };
			return this->description2token(description, token) ? this->play(token, outputPlayerID) : false;
		}
		else
			return false;
	}
	//void display(PlayerID playerID, bool doesClearScreen) const // const
	//{
	//	return;
	//}
	void display(bool doesClearScreen) const // const
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
				cout << "玩家 " << (playerID + 1) << (!this->records.empty() && !this->records[0].empty() && this->records[0].back().playerID == playerID ? (this->pokerType == "三人斗地主" || this->pokerType == "四人斗地主" ? "（地主）：" : "（最小先出）：") : "：") << endl << this->cards2string(this->displayers[playerID].cards, "", " | ", "", "（空）") << endl << endl;
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
	const vector<string> helpOptions = { "?", "/?", "-?", "h", "/h", "-h", "help", "/help", "--help" };
	HelpKey helpKey = 0;

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
	void getDescription(string& description) const
	{
		rewind(stdin);
		fflush(stdin);
		char buffer[256] = { 0 };
		fgets(buffer, 255, stdin);
		description = buffer;
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
					else if (this->isEqual("FromBigToSmall", arguments[argumentID]) || this->isEqual("0", arguments[argumentID]))
						this->sortingMethod = SortingMethod::FromBigToSmall;
					else if (this->isEqual("FromSmallToBig", arguments[argumentID]) || this->isEqual("1", arguments[argumentID]))
						this->sortingMethod = SortingMethod::FromSmallToBig;
					else if (this->isEqual("FromManyToFew", arguments[argumentID]) || this->isEqual("2", arguments[argumentID]))
						this->sortingMethod = SortingMethod::FromManyToFew;
					else if (this->isEqual("FromFewToMany", arguments[argumentID]) || this->isEqual("3", arguments[argumentID]))
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
				cardGame.setSortingMethod(this->sortingMethod);
				PlayerID playerID = cardGame.getLandlord();
				if ("三人斗地主" == this->pokerType)
					cardGame.setLandlord(true, true, true, false);
				else if ("四人斗地主" == this->pokerType)
					cardGame.setLandlord(LandlordScore::None, LandlordScore::One, LandlordScore::Three, LandlordScore::None);
				playerID = cardGame.getLandlord();
				for (;;)
				{
					cardGame.display(true);
					cout << "请玩家 " << (playerID + 1) << " 开牌：";
					string buffer = "";
					this->getDescription(buffer);
					if (cardGame.start(buffer, playerID))
						break;
				}
				while (playerID != (PlayerID)(-1))
				{
					for (;;)
					{
						cardGame.display(true);
						cout << "请玩家 " << (playerID + 1) << " 出牌：";
						string buffer = "";
						this->getDescription(buffer);
						if (cardGame.play(buffer, playerID, playerID))
							break;
					}
				}
				cout << "此局已终，程序即将退出。" << endl;
				this_thread::sleep_for(chrono::seconds(TIME_FOR_SLEEP));
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