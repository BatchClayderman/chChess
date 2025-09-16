## pokerGame

该文件夹提供扑克牌游戏可能的实现。

### pokerGame.cpp

这是一个你能打赢，同时支持多种扑克牌类型的 C++ 程序，但暂未完善。

该代码支持命令行启动，在 Windows 下应当使用 Visual Studio 进行编译，在其它平台下可使用 g++ 直接编译，要求编译器支持 C++11 或更高标准。

前有代码即证明（Code as proof），现有代码即规则（Code as rules）。

基类成员变量：

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

基类中主要的函数（所有成员函数的返回值类型均为 bool）：

1) 构造函数：初始化随机数种子，派生类的构造函数中应当调用该过程并声明扑克牌类型；该过程仅在实例创建时被调用；调用成功后，状态将变更为缺省值 Ready。
2) 析构函数：该过程仅在实例的销毁阶段被调用；由于实例即将被销毁，状态变更没有意义，故不会发生状态变更。
3) initialize：初始化 values（从 1 开始）和 players，该过程将清空 deck 和 record；该过程允许在不低于 Ready 的状态（可理解为任意状态）下被调用；调用成功后，状态将变更为 Initialized。
4) deal：发牌，该过程将利用随机数种子 seed 向 players 和 deck 随机分配扑克牌并清空 records，随后将预备回合信息写入 records[0]；该过程允许在不低于 Initialized 的状态下被调用；调用成功后，状态将变更为 Dealt（涉及地主信息）或 Assigned（不涉及地主信息）；
5) 一些为获取当前玩家和地主信息设定设立的额外函数：地主信息设定仅允许在 Dealt 状态下涉及地主信息的扑克牌类型中进行；地主信息被写入 records[0] 后，状态将变更为 Assigned。
6) start：开牌，该过程将依照 token 读写 records，部分扑克牌类型对开牌的 token 有特定要求；该过程仅允许在 Assigned 状态下进行；调用成功后，状态将变更为 Started 或 Over（开局直接结束游戏）。
7) play：出牌，该过程将依照 token 读写 records；该过程仅允许在 Started 状态下进行；游戏结束后，状态将变更为 Over。
8) set：导入牌局，该过程将先尝试将参数作为文件路径进行处理，处理失败则将参数作为数据进行处理；该过程仅允许不低于 Initialized 的状态下被调用；调用成功后，状态将根据导入牌局的情况进行自适应变更。
9) display：显示牌局；该过程允许在任何状态下被调用；由于该过程是只读的，故不会发生状态变更。

编程风格：

1) 代码块排序：头文件 -> 宏 -> 简易类型定义 -> constexpr -> 命名空间 -> 枚举类 -> 结构体 -> 类（成员 -> 私有方法 -> 受保护方法 -> 公有方法）-> 非 main 函数 -> main 函数；
2) 变量排序：从 main 函数开始从上往下进行扫描符号，每出现新符号时依照代码块排序放置在其所属代码块中最后的位置，如新符号与当前符号属于同一代码块，则放置在代码块内当前符号前的最后位置，随后递归地从上往下扫描新符号中的新符号执行相应的操作以完成排序；
3) 最小权限原则：对方法在满足设计需求范围的前提下使用最受限的修饰符，例如使用 const 修饰不对成员做出修改的方法，使用 private 声明私有函数等；
4) 虚函数：虚函数在重写时必须带有 override 修饰符，在最后实现时必须带有 final 修饰符。

### 规则

#### 斗地主

### 参阅

斗地主规则（出自《象棋百科全书》）：[https://www.xqbase.com/other/landlord.htm](https://www.xqbase.com/other/landlord.htm)

