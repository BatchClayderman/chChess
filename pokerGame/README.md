## pokerGame

该文件夹提供扑克牌游戏可能的实现。

### pokerGame.cpp

这是一个你能打赢，同时支持多种扑克牌类型的 C++ 程序，但暂未完善。

该代码支持命令行启动，在 Windows 下应当使用 Visual Studio 进行编译，在其它平台下可使用 g++ 直接编译。

前有代码即证明（Code as proof），现有代码即规则（Code as rules）。

基类成员：

```
protected:
	mt19937 seed{}; // 用于存储随机数种子
	string pokerType = "扑克牌"; // 用于存储扑克牌类型
	Value values[14] = { 0 }; // 用于存储大小
	vector<vector<Card>> players{}; // 用于存储玩家手上的扑克牌
	vector<Card> deck{}; // 用于存储地主牌或牌堆
	vector<vector<Token>> records{}; // 用于存储游戏记录
	Player currentPlayer = (Player)(-1); // 用于指示当前需要进行操作的玩家
	Token* pLastToken = nullptr; // 用于存储指向最后一次非空 token 的指针
	Status status = Status::Ready; // 用于存储状态
```

基类主要函数（返回值均为 bool）：

1) 构造函数：初始化随机数种子，派生类中应当声明扑克牌类型，状态缺省值为 Ready。
2) initialize：初始化 values（从 1 开始）和 players，该过程将清空 deck 和 record；该过程允许在任何状态下被调用，调用成功后，状态将变更为 Initialized；
3) deal：发牌，该过程将利用随机数种子 seed 向 players 和 deck 随机分配扑克牌并清空 records，随后将预备回合信息写入 records[0]；该过程允许在不低于 Initialized 的状态下被调用，调用成功后，状态将变更为 Dealt（涉及地主信息）或 Assigned（不涉及地主信息）；
4) 一些为获取当前玩家和地主信息设定设立的额外函数：地主信息设定仅允许在 Dealt 状态下涉及地主信息的扑克牌类型中进行，地主信息被写入 records[0] 后，状态将变更为 Assigned。
5) start：开牌，该过程将依照 token 读写 records，部分扑克牌类型对开牌的 token 有特定要求；该过程仅允许在 Assigned 状态下进行，调用成功后，状态将变更为 Started。
6) play：出牌，该过程将依照 token 读写 records；该过程仅允许在 Started 状态下进行，游戏结束后，状态将变更为 Over。

编程风格：

1) 代码块排序：头文件 -> 宏 -> 简易类型定义 -> 命名空间 -> 枚举类 -> 结构体 -> 类（成员 -> 方法）-> 非 main 函数 -> main 函数；
2) 变量排序：从 main 函数开始从上往下进行扫描，每出现新符号时依照代码块排序放置在其所属代码块中、当前符号前的最后的位置，随后递归地从上往下扫描新符号中的新符号执行相应的操作以完成排序；
3) 虚函数：虚函数在重写时必须带有 override 修饰符，在最后实现时必须带有 final 修饰符。

### 参阅

斗地主规则（出自《象棋百科全书》）：[https://www.xqbase.com/other/landlord.htm](https://www.xqbase.com/other/landlord.htm)

