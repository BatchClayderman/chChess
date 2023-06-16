import os
import sys
from re import findall
from secrets import randbelow
from threading import Lock, Semaphore, Thread
os.chdir(os.path.abspath(os.path.dirname(__file__)))#解析进入程序所在目录
PLATFORM = __import__("platform").system().upper()
if PLATFORM == "WINDOWS":
	from msvcrt import getch
else:
	os.system("setfont /usr/share/consolefonts/Lat2-Terminus16.psf.gz")
	import tty
	import termios
	def getch():
		fd = sys.stdin.fileno()
		old_settings = termios.tcgetattr(fd)
		try:
			tty.setraw(sys.stdin.fileno())
			ch = sys.stdin.read(1)
		finally:
			termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
		return bytes(ch, encoding = "utf-8")
EXIT_SUCCESS = 0
EXIT_FAILURE = 1
if "idlelib" in sys.modules or not sys.stdin.isatty(): # 未检测到终端
	print("未检测到终端，部分功能无法正常使用，请勿尝试在 IDE 或 IDLE 中启动本程序。")
	input("请按下回车键退出。")
	sys.exit(EXIT_FAILURE)
try:
	from colorama import Fore, Back, Style, init as initColor
	if PLATFORM == "WINDOWS" and sys.stdin.isatty(): # 在 Windows 终端
		initColor(wrap = True) # Windows 终端
	defaultFore = Fore.BLACK
	foreRed = Fore.RED # 红方
	foreBlack = Fore.GREEN # 黑方
	foreRound = Fore.BLUE # 环绕颜色
	print(defaultFore + Back.WHITE, end = Style.DIM)
except:
	print("未能调用 colorama 库，请尝试在合适的环境中运行“pip install colorama”进行安装。")
	print("程序将继续运行，但这可能会影响您的视觉效果，请按任意键继续。")
	defaultFore = foreRed = foreBlack = foreRound = "" # 关闭颜色
	getch()
try:
	import pyttsx3
	engine = pyttsx3.init()
	del engine
	lock = Lock() # 锁定 toSpeakList
	lock.acquire()
	toSpeakList = []
	sem = Semaphore(0) # 信号量
	lock.release()
	def speak():
		engine = pyttsx3.init()
		while True:
			sem.acquire() # 申请信号量（减少死循环）
			lock.acquire()
			if toSpeakList:
				toSpeak = toSpeakList.pop(0) # 弹出第一个
			lock.release()
			if toSpeak:
				engine.say(toSpeak)
				engine.runAndWait()
			else:
				engine.say("欢迎再次使用！")
				engine.runAndWait()
				break
	t = Thread(target = speak)
	t.start()
	def doSpeak(text):
		lock.acquire()
		global toSpeakList # 修改全局变量
		toSpeakList.append(text)
		lock.release()
		sem.release() # 释放信号量
except:
	def doSpeak(text): # 初始化失败，设置函数：什么也不干
		pass
	print("未能调用 pyttsx3 库，请尝试在合适的环境中运行“pip install pyttsx3”进行安装。")
	print("程序将继续运行，但这可能会影响您的听觉效果，请按任意键继续。")
	getch()


class Board:
	mode_statement = {					\
		0b0000:"中国象棋不用电脑", 			\
		0b0001:"中国象棋电脑执黑", 			\
		0b0010:"中国象棋电脑执红", 			\
		0b0011:"中国象棋电脑执双", 			\
		0b0100:"中国象棋残局不用电脑", 		\
		0b0101:"中国象棋残局电脑执黑", 		\
		0b0110:"中国象棋残局电脑执红", 		\
		0b0111:"中国象棋残局电脑执双", 		\
		0b1000:"中国揭棋随机生成不用电脑", 		\
		0b1001:"中国揭棋随机生成电脑执黑", 		\
		0b1010:"中国揭棋随机生成电脑执红", 		\
		0b1011:"中国揭棋随机生成电脑执双", 		\
		0b1100:"中国揭棋人工指派不用电脑", 		\
		0b1101:"中国揭棋人工指派电脑执黑", 		\
		0b1110:"中国揭棋人工指派电脑执红", 		\
		0b1111:"中国揭棋人工指派电脑执双"		\
	}
	FEN_statement = {																	\
		"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2C4/9/RNBAKABNR b":"中炮局（B00）", 								\
		"rnbakab1r/9/1c4nc1/p1p1p1p1p/9/9/P1P1P1P1P/1C2C4/9/RNBAKABNR w":"中炮对进左马（B05）", 								\
		"r1bakab1r/9/1cn3nc1/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/9/RNBAKAB1R w":"中炮对屏风马（C00）", 							\
		"r1bakabr1/9/1cn3nc1/p1p1p1p1p/9/9/P1P1P1P1P/NC2C1N2/9/R1BAKABR1 b":"中炮左边马对屏风马（C05）", 							\
		"r1bakabr1/9/1cn3nc1/p1p1p1p1p/9/9/P1P1P1P1P/1CN1C1N2/9/R1BAKABR1 b":"中炮七路马对屏风马（C01）", 						\
		"r1bakabr1/9/1cn3nc1/p1p1p1p1p/9/9/P1P1P1P1P/3CC1N2/9/RNBAKABR1 b":"五六炮对屏风马（C50）", 							\
		"r1bakabr1/9/1cn3nc1/p1p1p1p1p/9/7R1/P1P1P1P1P/1C2C1N2/9/RNBAKAB2 b":"中炮巡河车对屏风马——红不进左马（C15）", 					\
		"r1bakab1r/9/1cn3nc1/p1p1p3p/6p2/2P6/P3P1P1P/1CN1C1N2/9/R1BAKAB1R b":"中炮七路马对屏风马（C01）", 						\
		"r1bakab1r/9/1cn3nc1/p3p1p1p/2p6/6P2/P1P1P3P/1CN1C1N2/9/R1BAKAB1R b":"中炮七路马对屏风马（C01）", 						\
		"r1bakab1r/9/1cn3nc1/p1p1p1p1p/9/9/P1P1P1P1P/1CN1C1N2/9/R1BAKAB1R b":"中炮七路马对屏风马（C01）", 						\
		"rnbakab1r/9/1c4n1c/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/9/RNBAKAB1R w":"中炮右横车对左三步虎（B22）", 						\
		"rnbakab1r/9/1c4n1c/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/8R/RNBAKAB2 b":"中炮对左三步虎（B20）", 							\
		"r1bakabnr/9/1cn4c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2C4/9/RNBAKABNR w":"中炮对进右马（B01）", 								\
		"r1bakabnr/9/c1n4c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/9/RNBAKAB1R w":"中炮对右三步虎（B04）", 							\
		"rnbakabnr/9/1c2c4/p1p1p1p1p/9/9/P1P1P1P1P/1C2C4/9/RNBAKABNR w":"顺炮缓开车局（D00）", 								\
		"rnbakab1r/9/1c2c1n2/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/9/RNBAKABR1 b":"顺炮直车对缓开车（D10）", 							\
		"rnbakab2/8r/1c2c1n2/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/9/RNBAKABR1 w":"顺炮直车对横车（D20）", 							\
		"rnbakab2/8r/1c2c1n2/p1p1p1p1p/9/6P2/P1P1P3P/1C2C1N2/9/RNBAKABR1 b":"顺炮直车对横车——红进三兵（D26）", 						\
		"rnbakab2/8r/1c2c1n2/p1p1p1p1p/9/9/P1P1P1P1P/3CC1N2/9/RNBAKABR1 b":"顺炮直车对横车——红仕角炮（D25）", 						\
		"rnbakab2/8r/1c2c1n2/p1p1p1p1p/9/7R1/P1P1P1P1P/1C2C1N2/9/RNBAKAB2 b":"顺炮直车对横车——红巡河车（D23）", 						\
		"rnbakab2/8r/1c2c1n2/p1p1p1p1p/9/9/P1P1P1P1P/NC2C1N2/9/R1BAKABR1 b":"顺炮直车对横车——红左边马（D22）", 						\
		"rnbakab2/8r/1c2c1n2/p1p1p1p1p/9/2P6/P3P1P1P/1C2C1N2/9/RNBAKABR1 b":"顺炮直车对横车——红进七兵（D27）", 						\
		"rnbakab2/8r/1c2c1n2/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/4A4/RNBAK1BR1 b":"顺炮直车对横车——红先上仕（D21）", 					\
		"rnbakab1r/9/1c2c1n2/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/8R/RNBAKAB2 b":"顺炮横车对缓开车（D03）", 							\
		"rnbakabr1/9/1c2c1n2/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/8R/RNBAKAB2 w":"顺炮横车对直车（D04）", 							\
		"rnbakab2/9/1c2c1n2/p1p1p1p1p/7r1/9/P1P1P1P1P/1C2C1N2/3R5/RNBAKAB2 w":"顺炮横车对直车巡河（D05）", 						\
		"rnbakab2/8r/1c2c1n2/p1p1p1p1p/9/6P2/P1P1P3P/1C2C1N2/9/RNBAKAB1R w":"顺炮缓开车对横车（D01）", 							\
		"rnbakabr1/9/1c2c1n2/p1p1p1p1p/9/6P2/P1P1P3P/1C2C1N2/9/RNBAKAB1R w":"顺炮缓开车对直车（D02）", 							\
		"rnbakabr1/9/1c2c1n2/p1p1p1p1p/9/9/P1P1P1P1P/1CN1C1N2/9/R1BAKAB1R w":"顺炮缓开车对直车（D02）", 							\
		"rnbakab2/8r/1c2c1n2/p1p1p1p1p/9/9/P1P1P1P1P/1CN1C1N2/9/R1BAKAB1R w":"顺炮缓开车对横车（D01）", 							\
		"rnbakabn1/8r/1c2c4/p1p1p1p1p/9/9/P1P1P1P1P/1C2C1N2/9/RNBAKAB1R w":"顺炮缓开车对横车（D01）", 							\
		"rnbakabnr/9/1c2c4/p1p1p1p1p/9/9/P1P1P1P1P/1C2C4/8R/RNBAKABN1 b":"顺炮横车对缓开车（D03）", 							\
		"rnbakabnr/9/4c2c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2C4/9/RNBAKABNR w":"中炮对列炮（D50）", 								\
		
		"rnbakabnr/9/1c5c1/p1p1p1p1p/9/6P2/P1P1P3P/1C5C1/9/RNBAKABNR b":"仙人指路（E00）", 								\
		"rnbakabnr/9/1c4c2/p1p1p1p1p/9/6P2/P1P1P3P/1C5C1/9/RNBAKABNR w":"仙人指路对卒底炮（E10）", 							\
		"rnbakabnr/9/1c4c2/p1p1p1p1p/9/6P2/P1P1P3P/4C2C1/9/RNBAKABNR b":"仙人指路转左中炮对卒底炮（E13）", 						\
		"rnbaka1nr/9/1c2b1c2/p1p1p1p1p/9/6P2/P1P1P3P/4C2C1/9/RNBAKABNR w":"仙人指路转左中炮对卒底炮飞左象（E20）", 					\
		"rnbaka1nr/9/1c2b1c2/p1p1p1p1p/9/6P2/P1P1P3P/2N1C2C1/9/R1BAKABNR b":"仙人指路转左中炮对卒底炮飞左象——红进左马（E22）", 				\
		"rnbaka1nr/9/1c2b1c2/p1p1p3p/6p2/6P2/P1P1P3P/2N1C2C1/9/R1BAKABNR w":"仙人指路转左中炮对卒底炮飞左象——黑进7卒（E30）", 				\
		"rnbaka1nr/9/1c2b1c2/p1p1p3p/9/6p2/P1P1P3P/2N1C2C1/9/1RBAKABNR w":"仙人指路转左中炮对卒底炮飞左象——黑连进7卒（E31）", 				\
		"rnbaka1nr/9/1c2b1c2/p1p1p3p/9/6p2/P1P1P3P/2N1C2CN/9/R1BAKAB1R w":"仙人指路转左中炮对卒底炮飞左象——黑连进7卒（E31）", 				\
		"1nbaka1nr/r8/1c2b1c2/p1p1p1p1p/9/6P2/P1P1P3P/2N1C2C1/9/R1BAKABNR w":"仙人指路转左中炮对卒底炮飞左象——红进左马对黑右横车（E23）", 		\
		"rnbaka1nr/9/1c2b1c2/p1p1p1p1p/9/6P2/P1P1P3P/4C2CN/9/RNBAKAB1R b":"仙人指路转左中炮对卒底炮飞左象——红右边马（E37）", 				\
		"rnbaka1nr/9/1c2b1c2/p1p1p1p1p/9/6P2/P1P1P3P/4C2C1/4A4/RNBAK1BNR b":"仙人指路转左中炮对卒底炮飞左象——红先上仕（E21）", 				\
		"rnbaka1nr/9/1c2b1c2/p1p1C1p1p/9/6P2/P1P1P3P/7C1/9/RNBAKABNR b":"仙人指路转左中炮对卒底炮飞左象——红炮打中卒（E38）", 				\
		"rn1akabnr/9/1c2b1c2/p1p1p1p1p/9/6P2/P1P1P3P/4C2C1/9/RNBAKABNR w":"仙人指路转左中炮对卒底炮飞右象（E14）", 					\
		"rn1akabnr/9/1c2b1c2/p1p1p1p1p/9/6P2/P1P1P3P/4C2CN/9/RNBAKAB1R b":"仙人指路转左中炮对卒底炮飞右象——红右边马（E15）", 				\
		"rn1akab1r/9/1c2b1c1n/p1p1p1p1p/9/6P2/P1P1P3P/4C2CN/9/RNBAKAB1R w":"仙人指路转左中炮对卒底炮飞右象——互进边马（E16）", 				\
		"rnbakabnr/9/4c1c2/p1p1p1p1p/9/6P2/P1P1P3P/4C2C1/9/RNBAKABNR w":"仙人指路转左中炮对卒底炮转顺炮（E17）", 						\
		"rnbakabnr/9/1c4c2/p1p1p1p1p/9/6P2/P1P1P3P/1C2C4/9/RNBAKABNR b":"仙人指路转右中炮对卒底炮（E12）", 						\
		"rnbakabnr/9/1c4c2/p1p1p1p1p/9/6P2/P1P1P3P/1C2B2C1/9/RN1AKABNR b":"仙人指路飞相对卒底炮（E11）", 							\
		"rnbakabnr/9/1c4c2/p1p1p1p1p/9/6P2/P1P1P3P/1C2B2C1/9/RNBAKA1NR b":"仙人指路飞相对卒底炮（E11）", 							\
		"rnbakabnr/9/1c5c1/p3p1p1p/2p6/6P2/P1P1P3P/1C5C1/9/RNBAKABNR w":"对兵局（E40）", 								\
		"rnbakabnr/9/1c5c1/p3p1p1p/2p6/6P2/P1P1P3P/2C4C1/9/RNBAKABNR b":"对兵转兵底炮（E46）", 								\
		"rnbakabnr/9/4c2c1/p3p1p1p/2p6/6P2/P1P1P3P/2C4C1/9/RNBAKABNR w":"对兵转兵底炮对右中炮（E47）", 							\
		"rnbakabnr/9/1c2c4/p3p1p1p/2p6/6P2/P1P1P3P/2C4C1/9/RNBAKABNR w":"对兵转兵底炮对左中炮（E48）", 							\
		"rnbakabnr/9/1c5c1/p3p1p1p/2p6/6P2/P1P1P3P/1C4NC1/9/RNBAKAB1R b":"对兵进右马局（E41）", 								\
		"r1bakabnr/9/1cn4c1/p3p1p1p/2p6/6P2/P1P1P3P/1C4NC1/9/RNBAKAB1R w":"对兵互进右马局（E42）", 							\
		"r1bakabnr/9/1cn4c1/p3p1p1p/2p6/6P2/P1P1P3P/1C4NC1/8R/RNBAKAB2 b":"对兵互进右马局——红横车（E44）", 						\
		"r1bakabnr/9/1cn4c1/p3p1p1p/2p6/6P2/P1P1P3P/1C4N1C/9/RNBAKAB1R b":"对兵互进右马局——红边炮（E45）", 						\
		"r1bakabnr/9/1cn4c1/p3p1p1p/2p6/6P2/P1P1P3P/1C2B1NC1/9/RN1AKAB1R b":"对兵互进右马局——红飞相（E43）", 						\
		"r1bakabnr/9/1cn4c1/p3p1p1p/2p6/6P2/P1P1P3P/1C2B1NC1/9/RNBAKA2R b":"对兵互进右马局——红飞相（E43）", 						\
		"r1bakabnr/9/1cn4c1/p1p1p1p1p/9/6P2/P1P1P3P/1C5C1/9/RNBAKABNR w":"仙人指路对进右马（E06）", 							\
		"r1bakabnr/9/1cn4c1/p1p1p1p1p/9/6P2/P1P1P3P/1C4NC1/9/RNBAKAB1R b":"仙人指路互进右马局（E07）", 							\
		"r1bakabnr/9/1cn4c1/p1p1p1p1p/9/2P3P2/P3P3P/1C5C1/9/RNBAKABNR b":"两头蛇对进右马（E08）", 							\
		"r1bakabnr/9/1cn3c2/p1p1p1p1p/9/2P3P2/P3P3P/1C5C1/9/RNBAKABNR w":"两头蛇对进右马转卒底炮（E09）", 						\
		"rnbaka1nr/9/1c2b2c1/p1p1p1p1p/9/6P2/P1P1P3P/1C5C1/9/RNBAKABNR w":"仙人指路对飞象（E01）", 							\
		"rnbaka1nr/9/1c2b2c1/p1p1p1p1p/9/6P2/P1P1P3P/1C4NC1/9/RNBAKAB1R b":"仙人指路进右马对飞象（E02）", 						\
		"rnbakabnr/9/4c2c1/p1p1p1p1p/9/6P2/P1P1P3P/1C5C1/9/RNBAKABNR w":"仙人指路对中炮（E03）", 							\
		"rnbakabnr/9/6cc1/p1p1p1p1p/9/6P2/P1P1P3P/1C5C1/9/RNBAKABNR w":"仙人指路对金钩炮（E05）", 							\
		
		"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR b":"飞相局（A10）", 								\
		"rnbakabnr/9/1c1c5/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对左过宫炮（A30）", 							\
		"rnbakabnr/9/1c1c5/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR b":"飞相进右马对左过宫炮（A31）", 							\
		"rnbakabnr/9/3c3c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对右士角炮（A21）", 							\
		"rnbakabnr/9/3c3c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/R8/1NBAKA1NR b":"飞相横车对右士角炮（A24）", 							\
		"rnbakabnr/9/3c3c1/p1p1p1p1p/9/9/P1P1P1P1P/NC2B2C1/9/R1BAKA1NR b":"飞相左边马对右士角炮（A23）", 							\
		"rnbakabnr/9/3c3c1/p1p1p1p1p/9/9/P1P1P1P1P/1CN1B2C1/9/R1BAKA1NR b":"飞相进左马对右士角炮（A22）", 						\
		"rnbakabnr/9/3c3c1/p1p1p1p1p/9/2P6/P3P1P1P/1C2B2C1/9/RNBAKA1NR b":"飞相进七兵对右士角炮（A26）", 							\
		"rnbakabnr/9/3c3c1/p1p1p1p1p/9/6P2/P1P1P3P/1C2B2C1/9/RNBAKA1NR b":"飞相进三兵对右士角炮（A25）", 							\
		"rnbakabnr/9/1c5c1/p1p1p3p/6p2/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对进7卒（A36）", 								\
		"rnbakabnr/9/1c5c1/p1p1p3p/6p2/2P6/P3P1P1P/1C2B2C1/9/RNBAKA1NR b":"飞相互进七兵局（A38）", 							\
		"rnbakabnr/9/1c5c1/p1p1p3p/6p2/9/P1P1P1P1P/1CN1B2C1/9/R1BAKA1NR b":"飞相进左马对进7卒（A37）", 							\
		"r1bakabnr/9/1cn4c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对进右马（A14）", 							\
		"r1bakabnr/9/1cn4c1/p1p1p1p1p/9/6P2/P1P1P3P/1C2B2C1/9/RNBAKA1NR b":"飞相进三兵对进右马（A15）", 							\
		"r1bakabnr/9/1cn4c1/p1p1p1p1p/9/2P6/P3P1P1P/1C2B2C1/9/RNBAKA1NR b":"飞相进七兵对进右马（A16）", 							\
		"rnbakabnr/9/1c2c4/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对左中炮（A27）", 							\
		"rnbakabnr/9/5c1c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对右过宫炮（A35）", 							\
		"rnbakabnr/9/1c3c3/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对左士角炮（A20）", 							\
		"rnbaka1nr/9/1c2b2c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"顺相局（A11）", 								\
		"rnbakab1r/9/1c4nc1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对进左马（A13）", 							\
		"rnbakabnr/9/1c5c1/p3p1p1p/2p6/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对进3卒（A39）", 								\
		"rn1akabnr/9/1c2b2c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"列相局（A12）", 								\
		"rnbakabnr/9/4c2c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RNBAKA1NR w":"飞相对右中炮（A29）", 							\
		
		"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C4NC1/9/RNBAKAB1R b":"起马局（A40）", 								\
		"rnbakabnr/9/1c5c1/p1p1p3p/6p2/9/P1P1P1P1P/1C4NC1/9/RNBAKAB1R":"起马对进7卒（A41）", 								\
		"rnbakabnr/9/1c5c1/p1p1p3p/6p2/2P6/P3P1P1P/1C4NC1/9/RNBAKAB1R b":"起马互进七兵局（A45）", 							\
		"rnbakabnr/9/1c5c1/p1p1p3p/6p2/9/P1P1P1P1P/3C2NC1/9/RNBAKAB1R b":"起马转仕角炮对进7卒（A43）", 							\
		"rnbakabnr/9/1c5c1/p1p1p3p/6p2/9/P1P1P1P1P/1C4N1C/9/RNBAKAB1R b":"起马转边炮对进7卒（A42）", 							\
		"rnbakabnr/9/1c5c1/p1p1p3p/6p2/9/P1P1P1P1P/4C1NC1/9/RNBAKAB1R b":"起马转中炮对进7卒（A44）", 							\
		
		"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C1C5/9/RNBAKABNR b":"过宫炮局（A60）", 								\
		"rnbakab1r/9/1c4nc1/p1p1p1p1p/9/9/P1P1P1P1P/1C1C5/9/RNBAKABNR w":"过宫炮对进左马（A61）", 							\
		"rnbakabnr/9/1c2c4/p1p1p1p1p/9/9/P1P1P1P1P/1C1C5/9/RNBAKABNR w":"过宫炮对左中炮（A63）", 							\
		"rnbakabn1/8r/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C1C5/9/RNBAKABNR w":"过宫炮对横车（A62）", 								\
		
		"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/3C3C1/9/RNBAKABNR b":"仕角炮局（A50）", 								\
		"rnbakabnr/9/1c2c4/p1p1p1p1p/9/9/P1P1P1P1P/3C3C1/9/RNBAKABNR w":"仕角炮对右中炮（A52）", 							\
		"rnbakabnr/9/1c5c1/p3p1p1p/2p6/9/P1P1P1P1P/3C3C1/9/RNBAKABNR w":"仕角炮对进7卒（A54）", 								\
		
		"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/NC5C1/9/R1BAKABNR b":"边马局（A02）", 								\
		
		"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1CC6/9/RNBAKABNR b":"金钩炮局（A07）"								\
	}
	Initial_Board_Empty = "9/9/9/9/9/9/9/9/9/9 w - - 0 1"
	Initial_Board_Light = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1"
	Initial_Board_Dark = "xxxxkxxxx/9/1x5x1/x1x1x1x1x/9/9/X1X1X1X1X/1X5X1/9/XXXXKXXXX w - - 0 1"
	nodes = "RNHBEAKCPXrnhbeakcpx"
	uncovered = list("rnbaabnrccpppppPPPPPCCRNBAABNR") # 初始的未揭的棋
	natural_end = 120 # 无吃子步数作和
	longGeneral_end = 3 # 长将作负
	sameFEN_end = 4 # 不变着判和局面数
	def __init__(self, mode = 0b0000, FEN = Initial_Board_Light, prev = None, prevStep = None, next = [], nextStep = [], eaten = [], direction = True) -> tuple:
		self.mode = mode # 模式（棋类 | 机器辅助）
		self.FEN = FEN # 使用 FEN 记录局面
		self.FEN2lists() # 转 lists
		assert(self.checkFEN()) # 检查局面
		self.prev = prev # 上一个局面（只有一个局面）
		self.prevStep = prevStep # 上一步着法（只有一个局面）
		self.next = next[::] # 下一个局面（可能有多个局面）
		self.nextStep = nextStep[::] # 下一步着法（可能有多个局面）
		assert(len(self.next) == len(self.nextStep))
		if mode >> 3: # 揭棋
			if eaten is None: # 传递为 None 表示上一局面会处理
				self.eaten = None
			else:
				self.eaten = eaten[::] # 被吃子
				self.uncovered = Board.uncovered[::] # 初始化未揭子
				for line in self.lists[0]: # 去掉局面中已揭开的棋
					for piece in line:
						if piece == "H": # 马有两种表示
							self.uncovered.remove("N")
						elif piece == "h": # 马有两种表示
							self.uncovered.remove("n")
						elif piece == "E": # 相有两种表示
							self.uncovered.remove("B")
						elif piece == "e": # 象有两种表示
							self.uncovered.remove("b")
						elif piece not in "XxKk.": # 是其它非将帅明子
							self.uncovered.remove(piece)
				for piece in self.eaten: # 去掉被吃子
					if piece[-1] == "H": # 马有两种表示
						self.uncovered.remove("N")
					elif piece[-1] == "h": # 马有两种表示
						self.uncovered.remove("n")
					elif piece[-1] == "E": # 相有两种表示
						self.uncovered.remove("B")
					elif piece[-1] == "e": # 象有两种表示
						self.uncovered.remove("b")
					else:
						self.uncovered.remove(piece[-1]) # 第一个字符存明暗
				boardFEN = self.FEN.split(" ")[0]
				assert(												\
					len(findall("[A-Z]", boardFEN)) + len([piece for piece in self.eaten if "A" <= piece[-1] <= "Z"]) == 16		\
					and len(findall("[a-z]", boardFEN)) + len([piece for piece in self.eaten if "a" <= piece[-1] <= "z"]) == 16		\
					and boardFEN.count("X") == len([item for item in self.uncovered if "A" <= item <= "Z"])			\
					and boardFEN.count("x") == len([item for item in self.uncovered if "a" <= item <= "z"])			\
				) # 一方棋盘剩余子与被吃子和 == 16 and 暗子 == 未揭子
		else: # 象棋
			if eaten is None: # 传递为 None 表示上一局面会处理
				self.eaten = None
			else:
				self.eaten = Board.uncovered[::] # 初始化
				for line in self.lists[0]: # 逐行
					for piece in line: # 逐子
						if piece == "H": # 马有两种表示
							self.eaten.remove("N")
						elif piece == "h": # 马有两种表示
							self.eaten.remove("n")
						elif piece == "E": # 相有两种表示
							self.eaten.remove("B")
						elif piece == "e": # 象有两种表示
							self.eaten.remove("b")
						elif piece not in "Kk.": # 排除将帅空（遇到 Xx 报错）
							self.eaten.remove(piece)
			self.uncovered = [] # 未被揭开（象棋无暗子）
		self.direction = direction # 视角（红方为 True | 黑方为 False）
		tmpJudgement = self.judge() # 是否已终（防止残局无法走子）
		self.preMsg = tmpJudgement[2] if tmpJudgement[1] else None # 用于传递（构造函数无法传递返回值）
		self.startMsg = None
	def checkFEN(self) -> bool: # 局面检查
		boardFEN = self.FEN.split(" ")[0]
		return (													\
			(len(self.lists) == 6 and len(self.lists[0]) == 10 and not any([len(self.lists[0][i]) != 9 for i in range(10)]))			\
			and (boardFEN.count("R") <= 2 and boardFEN.count("r") <= 2)						\
			and (boardFEN.count("N") + boardFEN.count("H") <= 2 and boardFEN.count("n") + boardFEN.count("h") <= 2)		\
			and (boardFEN.count("B") + boardFEN.count("E") <= 2 and boardFEN.count("b") + boardFEN.count("e") <= 2)		\
			and (boardFEN.count("A") <= 2 and boardFEN.count("a") <= 2)						\
			and (boardFEN.count("K") == 1 and boardFEN.count("k") == 1)						\
			and (boardFEN.count("C") <= 2 and boardFEN.count("c") <= 2)						\
			and (boardFEN.count("P") <= 5 and boardFEN.count("p") <= 5)						\
			and (len(findall("[A-Z]", boardFEN)) <= 16 and len(findall("[a-z]", boardFEN)) <= 16)				\
			and self.lists[1] in "WwRrBbGg"									\
			and self.lists[-2] >> 1 <= self.lists[-1] - 1									\
			and (self.mode >> 3 or self.checkChess())									\
			and not self.checkIfIsEatingGeneral()									\
		)
	def checkChess(self) -> bool: # 象棋局面检查
		for i in range(7, 10): # 不应该出现红兵
			if "P" in self.lists[0][i]:
				return False
		for i in range(3): # 不应该出现黑卒
			if "p" in self.lists[0][i]:
				return False
		B_idx, b_idx, A_idx, a_idx, K_idx, k_idx = set(), set(), set(), set(), set(), set() # 相士将的位置
		for i in range(len(self.lists[0])):
			for j in range(len(self.lists[0][i])):
				if self.lists[0][i][j] in "BE":
					B_idx.add((i, j))
				elif self.lists[0][i][j] in "be":
					b_idx.add((i, j))
				elif self.lists[0][i][j] == "A":
					A_idx.add((i, j))
				elif self.lists[0][i][j] == "a":
					a_idx.add((i, j))
				elif self.lists[0][i][j] == "K":
					K_idx.add((i, j))
				elif self.lists[0][i][j] == "k":
					k_idx.add((i, j))
		return (									\
			B_idx < {(5, 2), (5, 6), (7, 0), (7, 4), (7, 8), (9, 2), (9, 6)}			\
			and b_idx < {(0, 2), (0, 6), (2, 0), (2, 4), (2, 8), (4, 2), (4, 6)}			\
			and A_idx < {(7, 3), (7, 5), (8, 4), (9, 3), (9, 5)}				\
			and a_idx < {(0, 3), (0, 5), (1, 4), (2, 3), (2, 5)}				\
			and K_idx < {(7, 3), (7, 4), (7, 5), (8, 3), (8, 4), (8, 5), (9, 3), (9, 4), (9, 5)}		\
			and k_idx < {(0, 3), (0, 4), (0, 5), (1, 3), (1, 4), (1, 5), (2, 3), (2, 4), (2, 5)}		\
		)
	def checkStart(self) -> None: # 检查开局局面
		if self.mode >> 3 == 0: # 只有象棋开局才有名称
			tmpFEN = self.FEN.split(" ")[0] + " " + self.lists[1]
			if tmpFEN in Board.FEN_statement:
				self.startMsg = Board.FEN_statement[tmpFEN]
			else:
				tmpFEN = "/".join([line[::-1] for line in self.FEN.split(" ")[0].split("/")]) + " " + self.lists[1]
				if tmpFEN in Board.FEN_statement:
					self.startMsg = Board.FEN_statement[tmpFEN]
	def checkIfIsEatingGeneral(self) -> False: # 当前局面先走方能否直接吃掉将军
		availCoor = Node.getAvailableCoordinates(self.mode, self.lists[1] in "WwRr", None, self.lists[0], useMethod = None) # 由于能够直接吃掉对方将军，不考虑是否走得开的问题
		for i in list(range(3)) + list(range(7, 10)):
			for j in range(3, 6):
				if self.lists[0][i][j] == "K":
					K_coor = (i, j)
				elif self.lists[0][i][j] == "k":
					k_coor = (i, j)
		for coor in availCoor:
			if coor[1] == (k_coor if self.lists[1] in "WwRr" else K_coor):
				return True
		return False
	def FEN2lists(self) -> None: # 转 lists
		self.lists = self.FEN.split(" ") # 按空格解剖 FEN
		self.lists[0] = self.lists[0].replace("H", "N").replace("E", "B").replace("h", "n").replace("e", "b") # 替换别号
		self.FEN = " ".join(self.lists)
		for ch in set(self.lists[0]):
			if "1" <= ch <= "9":
				self.lists[0] = self.lists[0].replace(ch, "." * int(ch)) # 拆解成若干个 .
		self.lists[0] = [list(line) for line in self.lists[0].split("/")] # 按行解剖棋盘
		self.lists[-1] = int(self.lists[-1])
		self.lists[-2] = int(self.lists[-2])
	def lists2FEN(self) -> None: # 转 FEN
		self.FEN = "/".join("".join(line) for line in self.lists[0]) + " " + " ".join([str(item) for item in self.lists[1:]])
		for i in range(9, 0, -1):
			self.FEN = self.FEN.replace("." * i, str(i)) # 将连续的 . 替换为数字
	def judgeGeneral(self, nextBoard) -> list: # 判断是否长将或将军
		availCoor = Node.getAvailableCoordinates(self.mode, self.lists[1] in "WwRr", None, nextBoard.lists[0], useMethod = None)
		for i in range(len(nextBoard.lists[0])):
			for j in range(len(nextBoard.lists[0][i])):
				if nextBoard.lists[0][i][j] == "K":
					K_coor = (i, j)
				elif nextBoard.lists[0][i][j] == "k":
					k_coor = (i, j)
		listRet = []
		for coor in availCoor:
			if coor[1] == (k_coor if self.lists[1] in "WwRr" else K_coor):
				listRet.append(nextBoard.lists[0][coor[0][0]][coor[0][1]])
		return listRet
	def judge(self) -> tuple: # 判断是否终局
		if self.lists[-2] >= Board.natural_end: # 无吃子步数达到 120 步
			return (True, True, "和棋！无吃子步数达到 {0} 步。".format(Board.natural_end))
		if self.mode >> 3: # 揭棋（只有两个将军才能判）
			naturalEnd = True
			for i in range(len(self.lists[0])): # 判断自然作和
				for j in range(len(self.lists[0][i])):
					if self.lists[0][i][j] not in "Kk.": # 有则不终止
						naturalEnd = False
						break
				if not naturalEnd: # 只是想要加速运行
					break
			if naturalEnd:
				return (True, True, "自然作和！")
			del naturalEnd # 释放内存			
		else: # 象棋
			naturalEnd = True
			for i in range(len(self.lists[0])): # 判断自然作和
				for j in range(len(self.lists[0][i])):
					if self.lists[0][i][j] in "RNHCPXrnhcpx": # 有则不终止
						naturalEnd = False
						break
				if not naturalEnd: # 只是想要加速运行
					break
			if naturalEnd:
				return (True, True, "自然作和！")
			del naturalEnd # 释放内存
		tmpBoard = self # 遍历链表用
		FEN_dicts = {" ".join(tmpBoard.FEN.split(" ")[:2]):1} # 统计数量
		while tmpBoard.prev: # 当含有前一个时
			tmpBoard = tmpBoard.prev
			FEN_dicts.setdefault(" ".join(tmpBoard.FEN.split(" ")[:2]), 0)
			FEN_dicts[" ".join(tmpBoard.FEN.split(" ")[:2])] += 1
			if FEN_dicts[" ".join(tmpBoard.FEN.split(" ")[:2])] >= Board.longGeneral_end and self.prev.judgeGeneral(self): # 一定有前一个
				return (True, True, "此局已终，{0}方长将作负！".format("黑" if self.lists[1] in "WwRr" else "红"))
			elif FEN_dicts[" ".join(tmpBoard.FEN.split(" ")[:2])] >= Board.sameFEN_end:
				return (True, True, "双方不变着作和！")
		else: # 循环自然结束释放内存
			FEN_dicts.clear()
			del FEN_dicts
			tmpBoard = None
			del tmpBoard
		availCoor = Node.getAvailableCoordinates(self.mode, self.lists[1] in "WwRr", None, self.lists[0], useMethod = None)
		for coor in availCoor: # 遍历所有可走的
			if Node.judgeWhetherCanGo(self.mode, coor[0], coor[1], self.lists[0], self.lists[1] in "WwRr"): # 只要能走得动
				return (True, False, None)
		else: # 走不动了
			return (True, True, "此局已终，{0}胜！".format("黑" if self.lists[1] in "WwRr" else "红"))
	def generateNextBoard(self, movement, isPrintMethod = True) -> tuple: # 根据着法生成下一个局面
		#results = (False, False, None) # 是否成功 | 是否终局 | 消息
		#p, q = (0, 0), (0, 0)		
		useMethod = None # 记录着法描述方式
		if type(movement) == tuple and len(movement) == 2 and len(movement[0]) == len(movement[1]) == 2 and 0 <= movement[0][0] <= 9 and 0 <= movement[0][1] <= 8 and 0 <= movement[1][0] <= 9 and 0 <= movement[1][1] <= 8:
			p, q = movement[0], movement[1]
			useMethod = 1
			method0 = True
			method1 = method2 = method3 = method4 = None # 跳过
		else:
			movement = str(movement) # 保险起见
			method0 = False
			method1 = findall("^[\\u4e00-\\u9fa5]?[\\u4e00-\\u9fa5].[进進退平].$", movement)
			method2 = findall("^[A-Za-z][1-9a-z+-][+-.][1-9]$", movement) # 匹配 c2-c5 等
			method3 = findall("^[A-Ia-i]\\d-?[A-Ia-i]\\d$", movement) # 匹配 c2-c5 等
			method4 = findall("\\d\\d-?\\d\\d$", movement) # 匹配 77-74 等
		if method4:
			method4 = method4[0] # 选取正则表达式数组中的第一个（实际上应该也是唯一一个）
			p = (int(method4[0]), int(method4[1]))
			q = (int(method4[2]), int(method4[3]))
			useMethod = 4
		elif method3:
			method3 = method3[0] # 选取正则表达式数组中的第一个（实际上应该也是唯一一个）
			p = (9 - int(method3[1]), ord(method3[0].upper()) - 65) # ord("A") = 65
			q = (9 - int(method3[-1]), ord(method3[-2].upper()) - 65)
			useMethod = 3
		elif method2:
			method2 = method2[0] # 选取正则表达式数组中的第一个（实际上应该也是唯一一个）
			if method2[0] not in Board.nodes:
				return (False, False, "未知棋子或未知移动方式，请检查。")
			elif method2[0] in "HEhe": # 相象和马的别号
				method2 = {"H":"N", "E":"B", "h":"n", "e":"b"}[method2[0]] + method2[1:] # 重构字符串
			if "1" <= method2[1] <= "9": # 某一路上唯一
				p1 = 9 - int(method2[1]) if "A" <= method2[0] <= "Z" else int(method2[1]) - 1 # 红用 9 减黑减 1
				if self.mode >> 3 == 0 and method2[0] in "BEAbea": # 象士特殊性
					if method2[1:-1] in ("3+", "3-", "7+", "7-"): # B3+1 等
						if method2[0] in "BE": # 相
							p = (9 if method2[2] == "+" else 5, 6 if method2[1] == "3" else 2)
						elif method2[0] in "be": # 象
							p = (0 if method2[2] == "+" else 4, 2 if method2[1] == "3" else 6)
						else:
							return (False, False, "仕士不存在 " + method2[1:] + " 说法，请检查。")
					elif method2[1:-1] in ("4+", "4-", "6+", "6-"): # A4+1 等
						if method2[0] == "A": # 仕
							p = (9 if method2[2] == "+" else 7, 5 if method2[1] == "4" else 3)
						elif method2[0] == "a": # 士
							p = (0 if method2[2] == "+" else 2, 3 if method2[1] == "4" else 5)
						else:
							return (False, False, "相象不存在 " + method[1:] + " 说法，请检查。")
					elif method2[1] in "159" and method2[2] in "+-": # 唯一
						if method2[0] == "A" and method2[1] == "5": # 仕
							p = (8, 4)
						elif method2[0] == "a" and method2[1] == "5": # 士
							p = (1, 4)
						elif method2[0] in "BE": # 相
							p = (7, 9 - int(method2[1]))
						elif method2[0] in "be": # 象
							p = (2, int(method2[1]) - 1)
						else:
							return (False, False, "一五九或159纵路上相象仕士指令不匹配，请检查。")
					else:
						return (False, False, "象棋模式下相象仕士指令不匹配，请检查。")
				else:
					cnt = 0
					for i in range(10):
						if self.lists[0][i][p1] == method2[0]:
							p0 = i 
							cnt += 1
					if cnt > 1:
						return (False, False, "指代不明，{0}路纵线上有 {1} 个{2}，请检查。".format("红方{0}".format("零一二三四五六七八九"[int(method2[1])]) if "A" <= method2[0] <= "Z" else "黑方 {0} ".format(method2[1]), cnt, Node.name_dict[method2[0]]))
					elif cnt == 0:
						return (False, False, "棋子不存在，{0}路纵线上没有{1}，请检查。".format("红方{0}".format("零一二三四五六七八九"[int(method2[1])]) if "A" <= method2[0] <= "Z" else "黑方 {0} ".format(method2[1]), Node.name_dict[method2[0]]))
					else:
						p = (p0, p1) # 保存坐标
			else: # 正则表达式已过滤
				multiCnt = 0 # 有多少列重复
				repeatIdx = [] # 存储坐标
				for j in (range(8, -1, -1) if "A" <= method2[0] <= "Z" else range(0, 9)): # 红方从右往左
					lineTmp = [] # 单列记录
					for i in (range(0, 10) if "A" <= method2[0] <= "Z" else range(9, -1, -1)): # 红方从上到下
						if self.lists[0][i][j] == method2[0]: # 相同兵种
							lineTmp.append((i, j))
					if len(lineTmp) > 1: # 有重复
						multiCnt += 1
						repeatIdx += lineTmp # 仅对有重复的列添加
				if multiCnt == 1 and len(repeatIdx) == 2 and method2[1] in "+-": # 一列重复且为前后
					p = repeatIdx[0 if method2[1] == "+" else -1] # 前为 0 后为 -1
				elif method2[1] not in "+-" and 0 <= ord(method2[1]) - 97 < len(repeatIdx): # 多重复（字母已被过滤小写无需容错）
					if len(repeatIdx) == 2:
						print("警告：写法不规范但依旧能识别，正确的写法应该为：“{0}”。\n指令已修正，请按任意键继续。".format(method2[0] + {"a":"+", "b":"-"}[method2[1]] + method2[2:]))
						getch()
					if ord(method2[1].lower()) - 97 < len(repeatIdx):
						p = repeatIdx[ord(method2[1].lower()) - 97] # 直接指示第几个
					else:
						return (False, False, "发生越界，请检查。")
				else:
					return (False, False, "未能识别该格式的第二个字符，请检查。")
			if "1" <= method2[3] <= "9": # 识别走法
				if method2[0] in "NHnh": # 马
					if method2[2] == ".":
						return (False, False, "马不能与平搭配，请检查。")
					q1 = 9 - int(method2[3]) if method2[0] in "NH" else int(method2[3]) - 1 # 确定路
					crossRoad = abs(p[1] - q1) # 确定跨过的路
					if crossRoad > 2 or crossRoad == 0: # 超过 2 路或直线报错
						return (False, False, "千里神马横跨超过两个纵路或走直线，请检查。")
					else:
						q0 = p[0] + (3 - crossRoad) * (1 if (method2[2] == "+") ^ (method2[0] in "NH") else -1) # 红马进为负
				elif method2[0] in "BEbe": # 相象
					if method2[2] == ".":
						return (False, False, "相和象不能与平搭配，请检查。")
					q1 = 9 - int(method2[3]) if method2[0] in "BE" else int(method2[3]) - 1 # 确定路
					if abs(p[1] - q1) != 2: # 检查跨过的路
						return (False, False, "小飞象乱飞，请检查。")
					else:
						q0 = p[0] + (2 if (method2[2] == "+") ^ (method2[0] in "BE") else -2) # 相进为负
				elif method2[0] in "Aa": # 仕士
					if method2[2] == ".":
						return (False, False, "仕和士不能与平搭配，请检查。")
					q1 = 9 - int(method2[3]) if method2[0] == "A" else int(method2[3]) - 1 # 确定路
					if abs(p[1] - q1) != 1: # 检查跨过的路
						return (False, False, "间谍士乱谋，请检查。")
					else:
						q0 = p[0] + (1 if (method2[2] == "+") ^ (method2[0] == "A") else -1) # 仕进为负
				elif method2[0] in "Xx": # 暗
					if p[0] == 6 and p[1] % 2 == 0: # 暗兵
						if method2[-2:] == "+1":
							q0, q1 = 5, p[1]
						else:
							return (False, False, "暗兵只有一种走法，即前进一步。")
					elif p[0] == 3 and p[1] % 2 == 0: # 暗卒
						if method2[-2:] == "+1":
							q0, q1 = 4, p[1]
						else:
							return (False, False, "暗卒只有一种走法，即前进一步。")
					elif p[0] == 7 and p[1] in (1, 7) or p[0] == 9 and p[1] in (0, 8): # 暗车炮
						if method2[-2] == "+": # 进
							q0, q1 = p[0] - int(method2[-1]), p[1]
						elif method2[-2] == "-": # 退
							q0, q1 = p[0] + int(method2[-1]), p[1]
						elif method2[-2] == ".": # 平
							q0, q1 = p[0], 9 - int(method2[-1])
					elif p[0] == 2 and p[1] in (1, 7) or p[0] == 0 and p[1] in (0, 8): # 暗車砲
						if method2[-2] == "+": # 进
							q0, q1 = p[0] + int(method2[-1]), p[1]
						elif method2[-2] == "-": # 退
							q0, q1 = p[0] - int(method2[-1]), p[1]
						elif method2[-2] == ".": # 平
							q0, q1 = p[0], int(method2[-1]) - 1
					elif p[0] == 9 and p[1] in (1, 7): # 暗马
						if method2[-2] == "+": # 进
							if method2[-1] in "1379":
								q0, q1 = 7, 9 - int(method2[-1])
							elif method2[-1] in "46":
								q0, q1 = 8, 9 - int(method2[-1])
							else:
								return (False, False, "走子失败，请正确使用暗马。")
						else:
							return (False, False, "走子失败，暗马开局唯有进。")
					elif p[0] == 0 and p[1] in (1, 7): # 暗馬
						if method2[-2] == "+": # 进
							if method2[-1] in "1379":
								q0, q1 = 2, int(method2[-1]) - 1
							elif method2[-1] in "46":
								q0, q1 = 1, int(method2[-1]) - 1
							else:
								return (False, False, "走子失败，请正确使用暗马。")
						else:
							return (False, False, "走子失败，暗马开局唯有进。")
					elif p[0] == 9 and p[1] in (2, 6): # 暗相
						if method2[-2] == "+": # 进
							if method2[-1] in "159":
								q0, q1 = 7, 9 - int(method2[-1])
							else:
								return (False, False, "走子失败，请正确使用暗相。")
						else:
							return (False, False, "走子失败，暗相开局唯有进。")
					elif p[0] == 0 and p[1] in (2, 6): # 暗象
						if method2[-2] == "+": # 进
							if method2[-1] in "159":
								q0, q1 = 2, int(method2[-1]) - 1
							else:
								return (False, False, "走子失败，请正确使用暗象。")
						else:
							return (False, False, "走子失败，暗象开局唯有进。")
					elif p[0] == 9 and p[1] in (3, 5): # 暗仕
						if method2[-2:] == "+5": # 进
							q0, q1 = 8, 4
						else:
							return (False, False, "走子失败，暗仕开局唯有进五。")
					elif p[0] == 0 and p[1] in (3, 5): # 暗士
						if method2[-2:] == "+5": # 进
							q0, q1 = 1, 4
						else:
							return (False, False, "走子失败，暗士开局唯有进5。")
					else:
						return (False, False, "暗子起始位置异常，走子失败。")
				else:
					if method2[2] == ".": # 平
						q0 = p[0]
						q1 = 9 - int(method2[3]) if "A" <= method2[0] <= "Z" else int(method2[3]) - 1 # 确定路
					else: # 进退（正则表达式已过滤）
						q0 = p[0] + (int(method2[3]) if (method2[2] == "+") ^ ("A" <= method2[0] <= "Z") else -int(method2[3])) # 红进为负
						q1 = p[1]
				q = (q0, q1)
			else:
				return (False, False, "未能识别该格式的最后一个数字字符，请检查。")
			useMethod = 2
		elif method1: # 很难（分类处理）
			method1 = method1[0] # 选取正则表达式数组中的第一个（实际上应该也是唯一一个）
			ch_num_list = "一二三四五六七八九壹贰叁肆伍陆柒捌玖"
			digit_num_list = "123456789１２３４５６７８９"
			if method1[-1] not in ch_num_list + digit_num_list:
				return (False, False, "未能识别该格式的最后一个汉字或数字字符，请检查。")
			if method1[-2] not in "进進退平":
				return (False, False, "未能识别该格式的倒数第二个汉字，请检查。")
			if method1[0] in list(Node.code_dict.keys()): # 炮二平五
				if method1[-1] in ch_num_list: # 中文数字
					if method1[1] in ch_num_list: # 是中文数字
						walker = True # 红方走子
					elif method1[1] in digit_num_list: # 是数字
						return (False, False, "数字和中文混合使用，请检查。")
					else: # 啥都不是
						return (False, False, "无法识别第二个字符，请检查。")
				else: # 数字（上文已过滤）
					if method1[1] in digit_num_list: # 是数字
						walker = False # 黑方走子
					elif method1[1] in ch_num_list: # 是汉字
						return (False, False, "中文和数字混合使用，请检查。")
					else: # 啥也不是
						return (False, False, "无法识别第二个字符，请检查。")
				node = Node.code_dict[method1[0]].upper() if walker else Node.code_dict[method1[0]].lower() # 棋子代号
				if self.mode >> 3 == 0 and method1[0] in "相象仕士": # 象士特殊性
					if method1[2] == "平":
						return (False, False, "相象仕士没有平的说法，请检查。")
					if method1[1] in "三叁七柒3３7７" and method1[2] in "进進退": # 相三进五 等
						if method1[0] in "相象" and walker: # 相
							if method1[0] != "相":
								print("警告：指令不规范但依旧可以识别，正确的写法为“相" + method1[1:] + "”。")
								print("请按任意键继续。")
								getch()
							p = (9 if method1[2] in "进進" else 5, 6 if method1[1] in "三叁" else 2)
						elif method1[0] in "相象" and not walker: # 象
							if method1[0] != "象":
								print("警告：指令不规范但依旧可以识别，正确的写法为“象" + method1[1:] + "”。")
								print("请按任意键继续。")
								getch()
							p = (0 if method1[2] in "进進" else 4, 2 if method1[1] in "3３" else 6)
						else:
							return (False, False, "仕士不存在“" + method1[1:] + "”说法，请检查。")
					elif method1[1] in "四肆六陆4４6６" and method1[2] in "进進退": # 仕四进五 等
						if method1[0] in "仕士" and walker: # 仕
							if method1[0] != "仕":
								print("警告：指令不规范但依旧可以识别，正确的写法为“仕" + method1[1:] + "”。")
								print("请按任意键继续。")
								getch()
							p = (9 if method1[2] in "进進" else 7, 5 if method1[1] in "四肆" else 3)
						elif method1[0] in "仕士" and not walker: # 士
							if method1[0] != "士":
								print("警告：指令不规范但依旧可以识别，正确的写法为“士" + method1[1:] + "”。")
								print("请按任意键继续。")
								getch()
							p = (0 if method1[2] in "进進" else 2, 3 if method1[1] in "4４" else 5)
						else:
							return (False, False, "相象不存在 " + method[1:] + " 说法，请检查。")
					elif method1[1] in "一五九壹伍玖1１5５9９" and method1[2] in "进進退": # 唯一
						if method1[0] in "仕士" and method1[1] == "五伍5５": # 仕士
							if walker: # 仕
								if method1[0] != "仕":
									print("警告：指令不规范但依旧可以识别，正确的写法为“仕" + method1[1:] + "”。")
									print("请按任意键继续。")
									getch()
								p = (8, 4)
							else: # 士
								if method1[0] != "士":
									print("警告：指令不规范但依旧可以识别，正确的写法为“士" + method1[1:] + "”。")
									print("请按任意键继续。")
									getch()
								p = (1, 4)
						elif method2[0] in "相象" and walker: # 相
							if method1[0] != "相":
								print("警告：指令不规范但依旧可以识别，正确的写法为“相" + method1[1:] + "”。")
								print("请按任意键继续。")
								getch()
							p = (7,  8 - ch_num_list.index(method1[1]) % 9)
						elif method2[0] in "相象" and not walker: # 象
							if method1[0] != "象":
								print("警告：指令不规范但依旧可以识别，正确的写法为“象" + method1[1:] + "”。")
								print("请按任意键继续。")
								getch()
							p = (2, ch_num_list.index(method1[1]))
						else:
							return (False, False, "一五九或159纵路上相象仕士指令不匹配，请检查。")
					else:
						return (False, False, "象棋模式下相象仕士指令不匹配，请检查。")
				else:
					p1 = 8 - ch_num_list.index(method1[1]) % 9 if walker else digit_num_list.index(method1[1]) % 9 # 定位列
					cnt = 0 # 计数
					for i in range(10):
						if self.lists[0][i][p1] == node: # 查找该纵列上的子
							p0 = i
							cnt += 1
					if cnt > 1:
						return (False, False, "指代不明，{0}路纵线上有 {1} 个{2}，请检查。".format("红方" + method1[1] if walker else "黑方" + method1[1], cnt, method1[0]))
					elif cnt == 0:
						return (False, False, "棋子不存在，{0}路纵线上没有{1}，请检查。".format("红方" + method1[1] if walker else "黑方" + method1[1], method1[0]))
					else:
						p = (p0, p1) # 保存坐标
			elif method1[1] in list(Node.code_dict.keys()) or len(method1) == 5 and method1[2] in list(Node.code_dict.keys()): # 前兵进一 或 前兵一进一 或 十二暗进一
				walker = method1[-1] in ch_num_list # 最后一个字符为中文数字即红方
				if method1[1] in list(Node.code_dict.keys()):
					node = Node.code_dict[method1[1]].upper() if walker else Node.code_dict[method1[1]].lower() # 棋子代号
				else:
					node = Node.code_dict[method1[2]].upper() if walker else Node.code_dict[method1[2]].lower() # 棋子代号
				multiCnt = 0 # 有多少列重复
				repeatIdx = [] # 存储坐标
				for j in (range(8, -1, -1) if walker else range(0, 9)): # 红方从右往左
					lineTmp = [] # 单列记录
					for i in (range(0, 10) if walker else range(9, -1, -1)): # 红方从上到下
						if self.lists[0][i][j] == node: # 相同兵种
							lineTmp.append((i, j))
					if len(lineTmp) > 1: # 有重复
						multiCnt += 1
						repeatIdx += lineTmp # 仅对有重复的列添加
				if multiCnt == 1: # 一列重复
					if len(method1) == 4 and method1[1] in list(Node.code_dict.keys()): # 前兵进一（单列）
						if len(repeatIdx) == 2: # 两个用前后
							if method1[0] == "前":
								if node in "BEAbea": # 象士特殊性
									print("警告：指令不规范但依旧可以识别。一般而言，象棋模式中相象仕士不使用前后进行描述。")
									print("请按任意键继续。")
									getch()
								p = repeatIdx[0]
							elif method1[0] in "后後":
								if node in "BEAbea": # 象士特殊性
									print("警告：指令不规范但依旧可以识别。一般而言，象棋模式中相象仕士不使用前后进行描述。")
									print("请按任意键继续。")
									getch()
								p = repeatIdx[-1]
							else:
								return (False, False, "仅查找到一列重复且重复计数为 2，应用“前后”描述，请检查。")
						elif len(repeatIdx) == 3: # 三个用前中后
							if method1[0] == "前":
								p = repeatIdx[0]
							elif method1[0] == "中":
								p = repeatIdx[1]
							elif method1[0] in "后後":
								p = repeatIdx[-1]
							else:
								return (False, False, "仅查找到一列重复且重复计数为 3，应用“前中后”描述，请检查。")
						else: # 前二三四
							if method1[0] in "前二三四五六七八九十前贰叁肆伍陆柒捌玖拾":
								if "前二三四五六七八九十前贰叁肆伍陆柒捌玖拾".index(method1[0]) % 10 < len(repeatIdx):
									p = repeatIdx["前二三四五六七八九十前贰叁肆伍陆柒捌玖拾".index(method1[0]) % 10]
								else:
									return (False, False, "发生越界，仅有一路有两个或两个以上的{0}，该路上仅有 {1} 个{0}，请检查。".format(method1[1], len(repeatIdx)))
							else:
								return (False, False, "仅查找到一列重复且重复计数超过 3，应用“前二三四……”描述，请检查。")
					else: # 表达冗余
						return (False, False, "仅查找到一列重复，但输入的指令长度为五，请检查。")
				else: # 多列重复
					if len(method1) == 4 and method1[1] in list(Node.code_dict.keys()): # 前兵进一（多列）
						if method1[0] in "一二三四五六七八九十壹贰叁肆伍陆柒捌玖拾":
							if "一二三四五六七八九十壹贰叁肆伍陆柒捌玖拾".index(method1[0]) % 10 < len(repeatIdx):
								p = repeatIdx["一二三四五六七八九十壹贰叁肆伍陆柒捌玖拾".index(method1[0]) % 10]
							else:
								return (False, False, "发生越界，有两个或两个以上的{0}的所有纵路中仅有 {1} 个{0}，请检查。".format(method1[1], len(repeatIdx)))
						else:
							return (False, False, "多列重复应用“一二三四……”描述，请检查。")
					elif len(method1) == 5 and method1[1] in list(Node.code_dict.keys()): # 前兵一进一
						if method1[-1] in ch_num_list: # 中文数字
							if method1[2] in digit_num_list: # 是数字
								return (False, False, "数字和中文混合使用，请检查。")
							elif method1[2] not in ch_num_list: # 啥都不是
								return (False, False, "无法识别第二个字符，请检查。")
						else: # 数字（上文已过滤）
							if method1[2] in ch_num_list: # 是汉字
								return (False, False, "中文和数字混合使用，请检查。")
							elif method1[2] not in digit_num_list: # 啥也不是
								return (False, False, "无法识别第二个字符，请检查。")
						p1 = 8 - ch_num_list.index(method1[2]) % 9 if walker else digit_num_list.index(method1[2]) % 9 # 定位列
						rpIdx = [] # 单列
						for item in repeatIdx:
							if item[1] == p1:
								rpIdx.append(item)
						if len(rpIdx) == 2:
							if method1[0] == "前":
								p = rpIdx[0]
							elif method1[0] in "后後":
								p = rpIdx[-1]
							else:
								return (False, False, "指定列中只有两个重复棋子应用“前后”描述，请检查。")
						elif len(rpIdx) == 3:
							if method1[0] == "前":
								p = rpIdx[0]
							elif method1[0] == "中":
								p = rpIdx[1]
							elif method1[0] in "后後":
								p = rpIdx[-1]
							else:
								return (False, False, "指定列中只有三个重复棋子应用“前中后”描述，请检查。")
						else:
							if method1[0] in "前二三四五六七八九十前贰叁肆伍陆柒捌玖拾":
								if "前二三四五六七八九十前贰叁肆伍陆柒捌玖拾".index(method1[0]) % 10 < len(rpIdx):
									p = rpIdx["前二三四五六七八九十前贰叁肆伍陆柒捌玖拾".index(method1[0]) % 10]
								else:
									return (False, False, "发生越界，手动指派的纵路中仅有 {0} 个{1}，请检查。".format(len(rpIdx), method1[1]))
							else:
								return (False, False, "指定列中超过三个重复棋子应用“前二三四……”描述，请检查。")
					elif len(method1) == 5 and method1[2] in list(Node.code_dict.keys()): # 十二暗进一
						if len(repeatIdx) != 12: # 不是十二一定越界
							return (False, False, "不存在 11 或 12 个多列重复的暗子，请检查。")	
						if method1[0] in "十拾":
							if method1[1] in "一壹":
								p = repeatIdx[10]
							elif method1[1] in "二贰":
								p = repeatIdx[11]
							else:
								return (False, False, "走子名称位于长度为五的指令中间时前两个字应该为十一或十二，请检查。")	
						else:
							return (False, False, "子名称位于长度为五的指令中间时前两个字应该为十一或十二，请检查。")	
			else:
				return (False, False, "未知棋子名称，请检查。")	
			if node in "NHnh": # 马
				if method1[-2] == "平":
					return (False, False, "马不能与平搭配，请检查。")
				q1 = 8 - ch_num_list.index(method1[-1]) % 9 if walker else digit_num_list.index(method1[-1]) % 9 # 定位列
				crossRoad = abs(p[1] - q1) # 确定跨过的路
				if crossRoad > 2 or crossRoad == 0: # 超过 2 路或直线报错
					return (False, False, "千里神马横跨超过两个纵路或走直线，请检查。")
				else:
					q0 = p[0] + (3 - crossRoad) * (1 if (method1[-2] in "进進") ^ walker else -1) # 红马进为负
			elif node in "BEbe": # 相象
				if method1[-2] == "平":
					return (False, False, "相象不能与平搭配，请检查。")
				q1 = 8 - ch_num_list.index(method1[-1]) % 9 if walker else digit_num_list.index(method1[-1]) % 9 # 定位列
				if abs(p[1] - q1) != 2: # 检查跨过的路
					return (False, False, "小飞象乱飞，请检查。")
				else:
					q0 = p[0] + (2 if (method1[-2] in "进進") ^ walker else -2) # 相进为负
			elif node in "Aa": # 仕士
				if method1[-2] == "平":
					return (False, False, "仕士不能与平搭配，请检查。")
				q1 = 8 - ch_num_list.index(method1[-1]) % 9 if walker else digit_num_list.index(method1[-1]) % 9 # 定位列
				if abs(p[1] - q1) != 1: # 检查跨过的路
					return (False, False, "间谍士乱谋，请检查。")
				else:
					q0 = p[0] + (1 if (method1[-2] in "进進") ^ walker else -1) # 仕进为负
			elif node in "Xx": # 暗
				if p[0] == 6 and p[1] % 2 == 0: # 暗兵
					if method1[-2] in "进進" and method1[-1] in "一壹":
						q0, q1 = 5, p[1]
					else:
						return (False, False, "暗兵只有一种走法，即前进一步。")
				elif p[0] == 3 and p[1] % 2 == 0: # 暗卒
					if method1[-2] in "进進" and method1[-1] in "1１":
						q0, q1 = 4, p[1]
					else:
						return (False, False, "暗卒只有一种走法，即前进一步。")
				elif p[0] == 7 and p[1] in (1, 7) or p[0] == 9 and p[1] in (0, 8): # 暗车炮
					if method1[-2] in "进進": # 进
						q0, q1 = p[0] - "〇一二三四五六七八九零壹贰叁肆伍陆柒捌玖".index(method1[-1]) % 10, p[1]
					elif method1[-2] == "退": # 退
						q0, q1 = p[0] + "〇一二三四五六七八九零壹贰叁肆伍陆柒捌玖".index(method1[-1]) % 10, p[1]
					elif method1[-2] == "平": # 平
						q0, q1 = p[0], 9 - "〇一二三四五六七八九零壹贰叁肆伍陆柒捌玖".index(method1[-1]) % 10
				elif p[0] == 2 and p[1] in (1, 7) or p[0] == 0 and p[1] in (0, 8): # 暗車砲
					if method1[-2] in "进進": # 进
						q0, q1 = p[0] + "0123456789０１２３４５６７８９".index(method1[-1]) % 10, p[1]
					elif method1[-2] == "退": # 退
						q0, q1 = p[0] - "0123456789０１２３４５６７８９".index(method1[-1]) % 10, p[1]
					elif method1[-2] == "平": # 平
						q0, q1 = p[0], "1234567890１２３４５６７８９０".index(method1[-1]) % 10
				elif p[0] == 9 and p[1] in (1, 7): # 暗马
					if method1[-2] in "进進": # 进
						if method1[-1] in "一三七九壹叁柒玖":
							q0, q1 = 7, 9 - "〇一二三四五六七八九零壹贰叁肆伍陆柒捌玖".index(method1[-1]) % 10
						elif method1[-1] in "四六肆陆":
							q0, q1 = 8, 9 - "〇一二三四五六七八九零壹贰叁肆伍陆柒捌玖".index(method1[-1]) % 10
						else:
							return (False, False, "走子失败，请正确使用暗马。")
					else:
						return (False, False, "走子失败，暗马开局唯有进。")
				elif p[0] == 0 and p[1] in (1, 7): # 暗馬
					if method1[-2] in "进進": # 进
						if method1[-1] in "1379１３７９":
							q0, q1 = 2, "1234567890１２３４５６７８９０".index(method1[-1]) % 10
						elif method1[-1] in "46４６":
							q0, q1 = 1, "1234567890１２３４５６７８９０".index(method1[-1]) % 10
						else:
							return (False, False, "走子失败，请正确使用暗马。")
					else:
						return (False, False, "走子失败，暗马开局唯有进。")
				elif p[0] == 9 and p[1] in (2, 6): # 暗相
					if method1[-2] in "进進": # 进
						if method1[-1] in "一五九壹伍玖":
							q0, q1 = 7, 9 - "〇一二三四五六七八九零壹贰叁肆伍陆柒捌玖".index(method1[-1]) % 10
						else:
							return (False, False, "走子失败，请正确使用暗相。")
					else:
						return (False, False, "走子失败，暗相开局唯有进。")
				elif p[0] == 0 and p[1] in (2, 6): # 暗象
					if method1[-2] in "进進": # 进
						if method1[-1] in "159１５９":
							q0, q1 = 2, "1234567890１２３４５６７８９０".index(method1[-1]) % 10
						else:
							return (False, False, "走子失败，请正确使用暗象。")
					else:
						return (False, False, "走子失败，暗象开局唯有进。")
				elif p[0] == 9 and p[1] in (3, 5): # 暗仕
					if method1[-2] in "进進" and method1[-1] in "五伍": # 进
						q0, q1 = 8, 4
					else:
						return (False, False, "走子失败，暗仕开局唯有进五。")
				elif p[0] == 0 and p[1] in (3, 5): # 暗士
					if method1[-2] in "进進" and method1[-1] in "5５": # 进
						q0, q1 = 1, 4
					else:
						return (False, False, "走子失败，暗士开局唯有进5。")
				else:
					return (False, False, "暗子起始位置异常，走子失败。")
			else:
				if method1[-2] == "平": # 平
					q0 = p[0]
					q1 = 8 - ch_num_list.index(method1[-1]) % 9 if walker else digit_num_list.index(method1[-1]) % 9 # 定位列
				else: # 进退（正则表达式已过滤）
					if walker: # 红棋
						q0 = p[0] + (-(ch_num_list.index(method1[-1]) + 1) if method1[-2] in "进進" else (ch_num_list.index(method1[-1]) + 1)) # 红进为负
					else:
						q0 = p[0] + ((digit_num_list.index(method1[-1]) + 1) if method1[-2] in "进進" else -(digit_num_list.index(method1[-1]) + 1)) # 黑进为正
					q1 = p[1]
			q = (q0, q1)
			useMethod = 1
		elif method0:
			pass
		else:
			return (False, False, "未能识别该格式，请检查。")
		for sid, statements in enumerate(self.nextStep):
			if "{0}{1}{2}{3}".format(p[0], p[1], q[0], q[1]) == statements[-1]:
				return (True, False, "当前局面已存在。")
		if 0 <= p[0] <= 9 and 0 <= p[1] <= 8 and 0 <= q[0] <= 9 and 0 <= q[1] <= 8:
			pNode = self.lists[0][p[0]][p[1]]
			qNode = self.lists[0][q[0]][q[1]]
			if self.lists[1] in "WwRr" and "A" <= pNode <= "Z" or self.lists[1] in "BbGg" and "a" <= pNode <= "z": # 红方走子选中红子或黑方走子选中黑子
				if (q[0], q[1]) in Node.getAvailableCoordinates(self.mode, pNode, p, self.lists[0], useMethod if isPrintMethod else None): # 走子规则检查
					if Node.judgeWhetherCanGo(self.mode, p, q, self.lists[0], self.lists[1] in "WwRr", useMethod if isPrintMethod else None): # 被将军或送将检查（走完后对方可打）
						statement = Node.getStatement(self.mode, p, q, self.lists[0], [1, 2, 3, 4])
						nextBoard = Board(self.mode, FEN = self.FEN[::], prev = self, prevStep = statement, eaten = None) # 创建新局面（同时使其上一个指向自己并传递吃子顺序）
						nextBoard.uncovered = self.uncovered[::]
						nextBoard.lists[0][p[0]][p[1]] = "." # 走完子后原位置为空
						if self.mode >> 3: # 揭棋
							if self.lists[0][p[0]][p[1]] in "Xx": # 是暗子才需要指定
								chooseField = [piece for piece in self.uncovered if ("A" <= piece <= "Z" if self.lists[1] in "WwRr" else "a" <= piece <= "z")] # 可选子
								if self.mode & 0b0100: # 人工指派
									print("字典：{0}".format(Node.name_dict))
									print("可选子列表：{0}".format(chooseField))
									newNode = None
									newNode = input("请输入所走暗子的明子：")
									while newNode not in chooseField:
										print("输入有误，请按任意键继续。")
										getch()
										newNode = input("请输入所走暗子的明子：")
								else:
									newNode = chooseField[randbelow(len(chooseField))]
								nextBoard.uncovered.remove(newNode) # 移除
								nextBoard.lists[0][q[0]][q[1]] = newNode # 走完子后变成新子
							else:
								nextBoard.lists[0][q[0]][q[1]] = self.lists[0][p[0]][p[1]] # 走完子后变成新子
						else: # 象棋
							nextBoard.lists[0][q[0]][q[1]] = pNode # 走完子后新位置为原位置的子
						nextBoard.eaten = self.eaten[::] # 新局面手动处理
						nextBoard.lists[1] = "b" if self.lists[1] in "WwRr" else "w" # 反转走子方
						if qNode == ".":
							nextBoard.lists[-2] = self.lists[-2] + 1
						else: # 发生吃子
							if qNode in "Xx": # 吃的是暗棋
								chooseField = [piece for piece in self.uncovered if ("a" <= piece <= "z" if self.lists[1] in "WwRr" else "A" <= piece <= "Z")] # 可选子
								if self.mode & 0b0100: # 人工指派
									print("字典：{0}".format(Node.name_dict))
									print("可选子列表：{0}".format(chooseField))
									newNode = None
									newNode = input("请输入被吃暗子的明子：")
									while newNode not in chooseField:
										print("输入有误，请按任意键继续。")
										getch()
										newNode = input("请输入被吃暗子的明子：")
								else:
									newNode = chooseField[randbelow(len(chooseField))]
								nextBoard.uncovered.remove(newNode) # 移除
								nextBoard.eaten.append(qNode + newNode)
							else:
								nextBoard.eaten.append(qNode)
							nextBoard.lists[-2] = 0 # 如果发生吃子重置为 0
							if isPrintMethod:
								doSpeak("吃" + Node.name_dict[qNode].replace("车", "拘").replace("車", "拘").replace("卒", "族").replace("相", "象")) # 音效
						nextBoard.lists[-1] = self.lists[-1] + int(self.lists[1] in "BbGg") # 回合数 + 1
						nextBoard.lists2FEN() # 转换
						self.nextStep.append(statement)
						self.next.append(nextBoard) # 加入链表
						if isPrintMethod: # 开启了音效
							generalResult = self.judgeGeneral(nextBoard)
							if len(generalResult) == 1: # 音效
								doSpeak("将军！")
							elif 2 <= len(generalResult) <= 10: # 音效（原则上最多五将）
								doSpeak("双三四五六七八九十"[len(generalResult) - 2] + "将！")
							elif len(generalResult) >= 11: # 音效
								doSpeak("多将！")
						if self.startMsg is not None: # 传达开局局面 startMsg
							nextBoard.startMsg = self.startMsg
						nextBoard.checkStart()
						return nextBoard.judge()
					else: # 走子违规
						return (False, False, "未正确应将或不能送将，请检查。")
				else: # 走子违规
					return (False, False, "走子违规，请检查。")
			else:
				return (False, False, "当前走子方为{0}方，所供坐标无子或不是{0}子。".format("红" if self.lists[1] in "WwRr" else "黑"))
		else:
			return (False, False, "数组越界，请检查。")
	def switchToNextBoard(self, cnt = None): # 切换到下一个局面（默认选择最后一个局面）
		if cnt is None:
			if len(self.next) <= 0: # 没有局面
				return None
			elif len(self.next) == 1: # 只有一个局面
				return self.next[0]
			else:
				try:
					print("当前局面有 {0} 个分支：".format(len(self.next)))
					for sid, statements in enumerate(self.nextStep):
						print("\t{0} = {1}".format(sid, statements))
					tmp = int(input("\n请输入一个分支以继续："))
					return self.next[tmp]
				except:
					print("切换未成功，请检查输入！\n请按任意键继续。")
					getch()
					return None
		elif -len(self.next) <= cnt < len(self.next):
			return self.next[cnt]
		else:
			return None
	def evaluate(self) -> int:
		if self.mode >> 3: # 暂不支持揭棋
			return Board.evaluateChessScore(self.FEN)
		else:
			return Board.evaluateChessScore(self.FEN)
	@staticmethod
	def evaluateChessScore(toEvaluate) -> int:
		if type(toEvaluate) == list:
			value = 0
			for i, line in enumerate(toEvaluate):
				for j, item in enumerate(line):
					if item in Node.score_dict:
						value += Node.score_dict[item]
					elif item == "X":
						value += abs(Node.score_dict[Node.place[i][j]]) // 10
					elif item == "x":
						value -= abs(Node.score_dict[Node.place[i][j]]) // 10
			return value
		elif type(toEvaluate) == str:
			transStr = toEvaluate.split(" ")[0].replace("H", "N").replace("E", "B").replace("h", "n").replace("e", "b")
			for ch in set(transStr):
				if "1" <= ch <= "9":
					transStr = transStr.replace(ch, "." * int(ch)) # 拆解成若干个 .
			transStr = [list(line) for line in transStr.split("/")] # 按行解剖棋盘
			return Board.evaluateChessScore(transStr)
		elif type(toEvaluate) == Board:
			return Board.evaluateChessScore(toEvaluate.lists[0])
		else:
			return None
	def printBoard(self, appendix = True, isPrintDetailed = False) -> None:
		toShow = [
			["╔", "═", "═", "═", "╤", "═", "═", "═", "╤", "═", "═", "═", "╤", "═", "═", "═", "╤", "═", "═", "═", "╤", "═", "═", "═", "╤", "═", "═", "═", "╤", "═", "═", "═", "╗"], 
			["║", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", "╲", " ", "│", " ", "╱", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "║"], 
			["╟", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "╢"], 
			["║", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", "╱", " ", "│", " ", "╲", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "║"], 
			["╟", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "╢"], 
			["║", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "║"], 
			["╟", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "╢"], 
			["║", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "║"], 
			["╟", "─", "─", "─", "┴", "─", "─", "─", "┴", "─", "─", "─", "┴", "─", "─", "─", "┴", "─", "─", "─", "┴", "─", "─", "─", "┴", "─", "─", "─", "┴", "─", "─", "─", "╢"], 
			["║", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "║"], 
			["╟", "─", "─", "─", "┬", "─", "─", "─", "┬", "─", "─", "─", "┬", "─", "─", "─", "┬", "─", "─", "─", "┬", "─", "─", "─", "┬", "─", "─", "─", "┬", "─", "─", "─", "╢"], 
			["║", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "║"], 
			["╟", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "╢"], 
			["║", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "║"], 
			["╟", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "╢"], 
			["║", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", "╲", " ", "│", " ", "╱", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "║"], 
			["╟", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "┼", "─", "─", "─", "╢"], 
			["║", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", "╱", " ", "│", " ", "╲", " ", "│", " ", " ", " ", "│", " ", " ", " ", "│", " ", " ", " ", "║"], 
			["╚", "═", "═", "═", "╧", "═", "═", "═", "╧", "═", "═", "═", "╧", "═", "═", "═", "╧", "═", "═", "═", "╧", "═", "═", "═", "╧", "═", "═", "═", "╧", "═", "═", "═", "╝"]
		]
		idx1, idx2 = 0, 0 # 索引
		for ch in self.FEN:
			if ch == " ": # 全部结束
				break
			elif ch == "/": # 一行结束
				idx1 += 2 # 设置为下一行
				idx2 = 0 # 设置至行首
			elif ch in "123456789":
				idx2 += int(ch) << 2 # 跳过空位
			elif ch in Board.nodes: # 符号代码
				toShow[idx1][idx2] = ch
				if idx2 < len(toShow[0]) - 1:
					toShow[idx1][idx2 + 1] = ""
				if idx2 > 0:
					toShow[idx1][idx2 - 1] = " "
				idx2 += 4
		if appendix:
			print("模式：{0} | 当前回合：{1} | 无吃子步数：{2}\n局面代码：{3}".format(self.mode_statement[self.mode], self.lists[-1], self.lists[-2], self.FEN))
			score = self.evaluate()
			if self.startMsg:
				print("红方局面分：{0}{1} | 开局名称：{2}".format("" if score == 0 else ("+" if score > 0 else "-"), abs(score), self.startMsg))
			else:
				print("红方局面分：{0}{1}".format("" if score == 0 else ("+" if score > 0 else "-"), abs(score)))
			if self.prevStep:
				print("上一步着法：{0}{1}．{2}".format(self.lists[-1], self.prev.lists[1], self.prevStep))
			if isPrintDetailed: # 是否打印具体的子（一般情况不可打印）
				print("红方被吃子{0}：{1}".format("（无序）" if self.mode & 0b0100 else "", " ".join([(Node.dicts[piece[0]] if len(piece) > 1 else "") + Node.dicts[piece[-1]] for piece in self.eaten if "A" <= piece <= "Z"]) if len([piece for piece in self.eaten if "A" <= piece <= "Z"]) else "（无）") + defaultFore)
				print("黑方被吃子{0}：{1}".format("（无序）" if self.mode & 0b0100 else "", " ".join([(Node.dicts[piece[0]] if len(piece) > 1 else "") + Node.dicts[piece[-1]] for piece in self.eaten if "a" <= piece <= "z"]) if len([piece for piece in self.eaten if "a" <= piece <= "z"]) else "（无）") + defaultFore)
			else:
				print("红方被吃子{0}：{1}".format("（无序）" if self.mode & 0b0100 else "", "".join([Node.dicts[piece[-1]] for piece in self.eaten if "A" <= piece <= "Z"]) if len([piece for piece in self.eaten if "A" <= piece <= "Z"]) else "（无）") + defaultFore)
				print("黑方被吃子{0}：{1}".format("（无序）" if self.mode & 0b0100 else "", "".join([Node.dicts[piece[-1]] for piece in self.eaten if "a" <= piece <= "z"]) if len([piece for piece in self.eaten if "a" <= piece <= "z"]) else "（无）") + defaultFore)
			print("\n \t１  ２  ３  ４  ５  ６  ７  ８  ９")
		for idx1, line in enumerate(toShow):
			if appendix:
				print(foreRound + str((18 - idx1) >> 1 if idx1 % 2 == 0 else " ") + defaultFore, end = "\t")
			for ch in line:
				if ch and ch in Board.nodes:
					Node.printChar(ch)
				else:
					print(end = ch)
			print()
		if appendix:
			print(" \t九  八  七  六  五  四  三  二  一", end = "\n\n")
			print(foreRound + " \tAa  Bb  Cc  Dd  Ee  Ff  Gg  Hh  Ii" + defaultFore, end = "\n\n")
	def printLog(self) -> None:
		logs = []
		tmpBoard = self
		while tmpBoard.prev:
			logs.append("{0}{1}．{2}".format(tmpBoard.prev.lists[-1], tmpBoard.prev.lists[1], tmpBoard.prevStep))
			tmpBoard = tmpBoard.prev
		clearScreen()
		if logs:
			for line in logs[::-1]: # 反转顺序
				print(line)
			print("\n\n打印完成，请按任意键继续。")
		else:
			print("暂无走子记录，请按任意键继续。")
		getch()
	def dump(self, isAll = False, filepath = None, encoding = "utf-8") -> bool:
		clearScreen()
		if not filepath:
			filepath = input("请输入导出位置（*.FEN）：")
		if isAll:
			toDump = []
			tmpBoard = self
			while tmpBoard.prev:
				toDump.append(str(tmpBoard))
				tmpBoard = tmpBoard.prev
			toDump = "\n".join(toDump[::-1])
		else:
			toDump = str(self)
		try:
			with open(filepath.replace("\"", "").replace("\'", ""), "w", encoding = encoding) as f:
				f.write(toDump)
			print("保存成功，请按任意键返回。")
			getch()
			return True
		except Exception as e:
			print("保存失败，原因如下：\n{0}".format(e))
			print("\n请按任意键返回。")
			getch()
			return False
	def __eq__(self, board) -> bool: # 比较
		if type(board) == Board:
			return self.FEN == board.FEN
		elif type(board) in (str, list):
			return self.FEN == board
		else:
			return False
	def __str__(self) -> str: # 字符串格式
		return self.FEN

class Node:
	dicts = {				\
		"R":foreRed + "车", 		\
		"N":foreRed + "马", 		\
		"H":foreRed + "马", 		\
		"B":foreRed + "相", 		\
		"E":foreRed + "相", 		\
		"A":foreRed + "仕", 		\
		"K":foreRed + "帅", 		\
		"C":foreRed + "炮", 		\
		"P":foreRed + "兵", 		\
		"X":foreRed + "暗", 		\
		"r":foreBlack + "車", 	\
		"n":foreBlack + "馬", 	\
		"h":foreBlack + "馬", 	\
		"b":foreBlack + "象", 	\
		"e":foreBlack + "象", 	\
		"a":foreBlack + "士", 	\
		"k":foreBlack + "将", 	\
		"c":foreBlack + "砲", 	\
		"p":foreBlack + "卒", 	\
		"x":foreBlack + "暗"		\
	}
	name_dict = {		\
		"R":"车", 		\
		"N":"马", 		\
		"H":"马", 		\
		"B":"相", 		\
		"E":"相", 		\
		"A":"仕", 		\
		"K":"帅", 		\
		"C":"炮", 		\
		"P":"兵", 		\
		"X":"暗", 		\
		"r":"車", 		\
		"n":"馬", 		\
		"h":"馬", 		\
		"b":"象", 		\
		"e":"象", 		\
		"a":"士", 		\
		"k":"将", 		\
		"c":"砲", 		\
		"p":"卒", 		\
		"x":"暗"		\
	}
	code_dict = {		\
		"车":"R", 		\
		"马":"N", 		\
		"相":"B", 		\
		"仕":"A", 		\
		"帅":"K", 		\
		"炮":"C", 		\
		"兵":"P", 		\
		"暗":"X", 		\
		"車":"r", 		\
		"馬":"n", 		\
		"象":"b", 		\
		"士":"a", 		\
		"将":"k", 		\
		"砲":"c", 		\
		"卒":"p", 		\
	}
	score_dict = {		\
		"K":10000, 	\
		"R":200, 		\
		"C":75, 		\
		"N":50, 		\
		"H":50, 		\
		"P":10, 		\
		"B":5, 		\
		"E":5, 		\
		"A":3, 		\
		"a":-3, 		\
		"b":-5, 		\
		"e":-5, 		\
		"p":-10, 		\
		"n":-50, 		\
		"h":-50, 		\
		"c":-75, 		\
		"r":-200, 		\
		"k":-10000	\
	}
	place = ["rnbakabnr", "." * 9, ".c.....c.", "p.p.p.p.p", "." * 9, "." * 9, "P.P.P.P.P", ".C.....C.", "." * 9, "RNBAKABNR"]
	@staticmethod
	def getAvailableCoordinates(mode, node, cur, lists0, useMethod = None) -> list: # 获取可行的坐标（仅获取全部时返回值为内嵌套两个长度为 2 的元组的元组）
		availableCoordinates = []
		if node == True: # 红方全部
			for i in range(len(lists0)):
				for j in range(len(lists0[i])):
					tmpNode = lists0[i][j]
					if "A" <= tmpNode <= "Z":
						availableCoordinates += [((i, j), item) for item in Node.getAvailableCoordinates(mode, tmpNode, (i, j), lists0, useMethod = None)]
			return availableCoordinates
		elif node == False: # 黑方全部
			for i in range(len(lists0)):
				for j in range(len(lists0[i])):
					tmpNode = lists0[i][j]
					if "a" <= tmpNode <= "z":
						availableCoordinates += [((i, j), item) for item in Node.getAvailableCoordinates(mode, tmpNode, (i, j), lists0, useMethod = None)]
			return availableCoordinates
		if node in "RrCc": # 车炮类（炮多一个打少一个吃）
			for j in range(cur[1] - 1, -1, -1):
				if lists0[cur[0]][j] == ".": # 空
					availableCoordinates.append((cur[0], j))
				else: # 有子（车可吃对方炮不可吃但需遍历可打）
					if node == "R" and "a" <= lists0[cur[0]][j] <= "z" or node == "r" and "A" <= lists0[cur[0]][j] <= "Z": # 对方的子
						availableCoordinates.append((cur[0], j))
					elif node in "Cc":
						for k in range(j - 1, -1, -1):
							if lists0[cur[0]][k] != ".": # 有子即停止
								if node == "C" and "a" <= lists0[cur[0]][k] <= "z" or node == "c" and "A" <= lists0[cur[0]][k] <= "Z": # 打对方的子
									availableCoordinates.append((cur[0], k))
								break
					break # 中止
			for j in range(cur[1] + 1, 9, 1):
				if lists0[cur[0]][j] == ".": # 空
					availableCoordinates.append((cur[0], j))
				else: # 有子（车可吃对方炮不可吃但需遍历可打）
					if node == "R" and "a" <= lists0[cur[0]][j] <= "z" or node == "r" and "A" <= lists0[cur[0]][j] <= "Z": # 对方的子
						availableCoordinates.append((cur[0], j))
					elif node in "Cc":
						for k in range(j + 1, 9, 1):
							if lists0[cur[0]][k] != ".": # 有子即停止
								if node == "C" and "a" <= lists0[cur[0]][k] <= "z" or node == "c" and "A" <= lists0[cur[0]][k] <= "Z": # 打对方的子
									availableCoordinates.append((cur[0], k))
								break
					break # 中止
			for i in range(cur[0] - 1, -1, -1):
				if lists0[i][cur[1]] == ".": # 空
					availableCoordinates.append((i, cur[1]))
				else: # 有子（车可吃对方炮不可吃但需遍历可打）
					if node == "R" and "a" <= lists0[i][cur[1]] <= "z" or node == "r" and "A" <= lists0[i][cur[1]] <= "Z": # 对方的子
						availableCoordinates.append((i, cur[1]))
					elif node in "Cc":
						for k in range(i - 1, -1, -1):
							if lists0[k][cur[1]] != ".": # 有子即停止
								if node == "C" and "a" <= lists0[k][cur[1]] <= "z" or node == "c" and "A" <= lists0[k][cur[1]] <= "Z": # 打对方的子
									availableCoordinates.append((k, cur[1]))
								break
					break # 中止
			for i in range(cur[0] + 1, 10, 1):
				if lists0[i][cur[1]] == ".": # 空
					availableCoordinates.append((i, cur[1]))
				else: # 有子（车可吃对方炮不可吃但需遍历可打）
					if node == "R" and "a" <= lists0[i][cur[1]] <= "z" or node == "r" and "A" <= lists0[i][cur[1]] <= "Z": # 对方的子
						availableCoordinates.append((i, cur[1]))
					elif node in "Cc":
						for k in range(i + 1, 10, 1):
							if lists0[k][cur[1]] != ".": # 有子即停止
								if node == "C" and "a" <= lists0[k][cur[1]] <= "z" or node == "c" and "A" <= lists0[k][cur[1]] <= "Z": # 打对方的子
									availableCoordinates.append((k, cur[1]))
								break
					break # 中止
		elif node in "NHnh": # 马类
			availableCoordinates += [(cur[0] + 1, cur[1] + 2), (cur[0] + 2, cur[1] + 1), (cur[0] + 1, cur[1] - 2), (cur[0] + 2, cur[1] - 1), (cur[0] - 1, cur[1] + 2), (cur[0] - 2, cur[1] + 1), (cur[0] - 1, cur[1] - 2), (cur[0] - 2, cur[1] - 1)] # 先固定四个方向（使用 += 保持原引用）
			for item in availableCoordinates[::]: # 剔除越界
				if item[0] < 0 or item[0] > 9 or item[1] < 0 or item[1] > 8:
					availableCoordinates.remove(item)
			for item in availableCoordinates[::]: # 剔除吃己并用中位数排取整除蹩马脚
				footDirection = [item[0] - cur[0], item[1] - cur[1]]
				footDirection = [cur[0] + footDirection[0] + (-1 if footDirection[0] > 0 else 1), cur[1] + footDirection[1] + (-1 if footDirection[1] > 0 else 1)] # 定位马脚位置
				if node in "NH" and "A" <= lists0[item[0]][item[1]] <= "Z" or node in "nh" and "a" <= lists0[item[0]][item[1]] <= "z" or lists0[footDirection[0]][footDirection[1]] != ".":
					availableCoordinates.remove(item)
		elif node in "BEbe": # 象类
			availableCoordinates += [(cur[0] + 2, cur[1] + 2), (cur[0] + 2, cur[1] - 2), (cur[0] - 2, cur[1] + 2), (cur[0] - 2, cur[1] - 2)] # 先固定四个方向（使用 += 保持原引用）
			for item in availableCoordinates[::]: # 剔除越界
				if item[0] < (0 if mode >> 3 or node in "be" else 5) or item[0] > (9 if mode >> 3 or node in "BE" else 4) or item[1] < 0 or item[1] > 8: # 如果象棋且红象则剔除 < 5，如果象棋且黑象则剔除 > 4
					availableCoordinates.remove(item)
			for item in availableCoordinates[::]: # 剔除吃己并用中位数排除蹩象脚
				if node in "BE" and "A" <= lists0[item[0]][item[1]] <= "Z" or node in "be" and "a" <= lists0[item[0]][item[1]] <= "z" or lists0[(cur[0] + item[0]) >> 1][(cur[1] + item[1]) >> 1] != ".":
					availableCoordinates.remove(item)
		elif node in "Aa": # 士类
			availableCoordinates += [(cur[0] + 1, cur[1] + 1), (cur[0] + 1, cur[1] - 1), (cur[0] - 1, cur[1] + 1), (cur[0] - 1, cur[1] - 1)] # 先固定四个方向（使用 += 保持原引用）
			for item in availableCoordinates[::]: # 剔除越界
				if item[0] < (0 if mode >> 3 or node == "a" else 7) or item[0] > (9 if mode >> 3 or node == "A" else 2) or item[1] < (0 if mode >> 3 else 3) or item[1] > (8 if mode >> 3 else 5): # 如果象棋且红士则剔除 < 7，如果象棋且黑士则剔除 > 2
					availableCoordinates.remove(item)
			for item in availableCoordinates[::]: # 剔除吃己
				if node == "A" and "A" <= lists0[item[0]][item[1]] <= "Z" or node == "a" and "a" <= lists0[item[0]][item[1]] <= "z":
					availableCoordinates.remove(item)
		elif node in "Kk": # 将帅类
			availableCoordinates += [(cur[0] + 1, cur[1]), (cur[0] - 1, cur[1]), (cur[0], cur[1] + 1), (cur[0], cur[1] - 1)] # 先固定四个方向（使用 += 保持原引用）
			for item in availableCoordinates[::]: # 剔除越界（象棋揭棋都如此）
				if item[0] < 0 or item[0] > 9 or 2 < item[0] < 7 or item[1] < 3 or item[1] > 5:
					availableCoordinates.remove(item)
			for item in availableCoordinates[::]: # 剔除吃己
				if node == "K" and "A" <= lists0[item[0]][item[1]] <= "Z" or node == "k" and "a" <= lists0[item[0]][item[1]] <= "z":
					availableCoordinates.remove(item)
			for i in range(len(lists0)):
				for j in range(len(lists0[i])):
					if lists0[i][j] == "K":
						K_index = (i, j)
					elif lists0[i][j] == "k":
						k_index = (i, j)
			if K_index[1] == k_index[1]: # 白脸将
				for i in range(k_index[0] + 1, K_index[0]): # 将军之间
					if lists0[i][K_index[1]] not in "Kk.": # 如果有任何子则中断
						break
				else:
					availableCoordinates.append(k_index if node == "K" else K_index)
		elif node in "Pp": # 卒兵类（象棋与揭棋一致）
			if node == "P": # 红兵
				if cur[0] < 5: # 过河兵
					if cur[0] != 0 and not "A" <= lists0[cur[0] - 1][cur[1]] <= "Z": # 底兵
						availableCoordinates.append((cur[0] - 1, cur[1]))
					if cur[1] != 0 and not "A" <= lists0[cur[0]][cur[1] - 1] <= "Z": # 边兵
						availableCoordinates.append((cur[0], cur[1] - 1))
					if cur[1] != 8 and not "A" <= lists0[cur[0]][cur[1] + 1] <= "Z": # 边兵
						availableCoordinates.append((cur[0], cur[1] + 1))
				elif not "A" <= lists0[cur[0] - 1][cur[1]] <= "Z": # 非过河兵只能往前走且不存在越界问题
					availableCoordinates.append((cur[0] - 1, cur[1]))
			else: # 黑卒
				if cur[0] > 4: # 过河卒
					if cur[0] != 9 and not "a" <= lists0[cur[0] + 1][cur[1]] <= "z": # 底卒
						availableCoordinates.append((cur[0] + 1, cur[1]))
					if cur[1] != 0 and not "a" <= lists0[cur[0]][cur[1] - 1] <= "z": # 边卒
						availableCoordinates.append((cur[0], cur[1] - 1))
					if cur[1] != 8 and not "a" <= lists0[cur[0]][cur[1] + 1] <= "z": # 边卒
						availableCoordinates.append((cur[0], cur[1] + 1))
				elif not "a" <= lists0[cur[0] + 1][cur[1]] <= "z": # 非过河卒只能往前走且不存在越界问题
					availableCoordinates.append((cur[0] + 1, cur[1]))
		elif node in "Xx": # 揭棋
			availableCoordinates = Node.getAvailableCoordinates(mode & 0b0111, Node.place[cur[0]][cur[1]], cur, lists0, useMethod = None) # 递归获取（不需要输出）
		if useMethod:
			print("不考虑将军问题时，所选子所有可能的着法如下：")
			print([Node.getStatement(mode, cur, coordinate, lists0, useMethod) for coordinate in availableCoordinates])
		return availableCoordinates
	@staticmethod
	def judgeWhetherCanGo(mode, p, q, cur_list, walker, useMethod = None) -> bool:
		if useMethod:
			print("考虑将军问题时，所选子所有可能的着法如下：")
			print([Node.getStatement(mode, p, coordinate, cur_list, useMethod) for coordinate in Node.getAvailableCoordinatesConsideringGeneral(mode, cur_list[p[0]][p[1]], p, cur_list)])
		next_list = [line[::] for line in cur_list]
		next_list[q[0]][q[1]] = next_list[p[0]][p[1]]
		next_list[p[0]][p[1]] = "."
		for i in (range(7, 10) if walker else range(3)):
			for j in range(3, 6):
				if next_list[i][j] == ("K" if walker else "k"):
					for pairs in Node.getAvailableCoordinates(mode, not walker, None, next_list, useMethod = None):
						if (i, j) == pairs[1]: # 可以吃到将军
							return False
					else: # 吃不到将军
						return True
		return False # （皇宫里）没有将军
	@staticmethod
	def getAvailableCoordinatesConsideringGeneral(mode, node, cur, lists0) -> list: # 获取考虑将军问题的所有可行坐标（提供给 AI 使用）
		availCoor = Node.getAvailableCoordinates(mode, node, cur, lists0, useMethod = None) # 后台加载不显示
		if node in (True, False): # 所有
			for coor in availCoor[::]: # 遍历所有可走的
				if not Node.judgeWhetherCanGo(mode, coor[0], coor[1], lists0, node, useMethod = None): # 走不动
					availCoor.remove(coor)
		else:
			for coor in availCoor[::]: # 遍历某个子所有可走的
				if not Node.judgeWhetherCanGo(mode, cur, coor, lists0, "A" <= node <= "Z", useMethod = None): # 走不动
					availCoor.remove(coor)
		return availCoor
	@staticmethod
	def getStatement(mode, cur, fur, lists0, useMethod): # 表述转换
		if useMethod == 1: # 第一种表述（中文纵线格式）
			node = lists0[cur[0]][cur[1]]
			states = [Node.name_dict[node], None, None, None]
			if "A" <= node <= "Z": # 红棋
				if fur[0] - cur[0] == 0: # 平
					states[2] = "平"
					states[3] = str(9 - fur[1]) # 有平必按纵线
				else: # 进退
					states[2] = "进" if fur[0] - cur[0] < 0 else "退" # 红方进退判断
					states[3] = str(9 - fur[1]) if abs(fur[0] - cur[0]) and abs(fur[1] - cur[1]) else str(abs(fur[0] - cur[0])) # 如果两个都变化了按纵线
			elif "a" <= node <= "z": # 黑棋
				if fur[0] - cur[0] == 0: # 平
					states[2] = "平"
					states[3] = str(fur[1] + 1) # 有平必按纵线
				else: # 进退
					states[2] = "进" if fur[0] - cur[0] > 0 else "退" # 黑方进退判断
					states[3] = str(fur[1] + 1) if abs(fur[0] - cur[0]) and abs(fur[1] - cur[1]) else str(abs(fur[0] - cur[0])) # 如果两个都变化了按纵线
			if states[2] is None: # still None
				return None
			if node in "Kk": # 将帅唯一
				states[1] = str(9 - cur[1]) if node == "K" else str(cur[1] + 1) # 红用 9 减黑加 1
			elif node in "PXpx": # 多样的兵暗
				rpIdx = [] # 当前列重复记录
				for i in (range(0, 10) if "A" <= node <= "Z" else range(9, -1, -1)): # 红方从上到下先判断一次
					if lists0[i][cur[1]] == node:
						rpIdx.append(i)
				if len(rpIdx) == 1: # 此列仅有一个
					states[1] = str(9 - cur[1]) if node in "PX" else str(cur[1] + 1) # 红用 9 减黑加 1
				else:
					multiCnt = 0 # 有多少列重复
					repeatIdx = [] # 存储坐标
					for j in (range(8, -1, -1) if node in "PX" else range(0, 9)): # 红方从右往左
						lineTmp = [] # 单列记录
						for i in (range(0, 10) if node in "PX" else range(9, -1, -1)): # 红方从上到下
							if lists0[i][j] == node: # 相同兵种
								lineTmp.append((i, j))
						if len(lineTmp) > 1: # 有重复
							multiCnt += 1
							repeatIdx += lineTmp # 仅对有重复的列添加
					if multiCnt == 1: # 一列重复
						if (len(rpIdx)) > 3: # 当前列超过三个采用“前二三四”
							states[1] = "前二三四五"[rpIdx.index(cur[0])]
						elif (len(rpIdx)) == 3: # 前中后
							states[1] = "前中后"[rpIdx.index(cur[0])]
						else: # 前后
							states[1] = "前后"[rpIdx.index(cur[0])]
					else:
						states[1] = ["一", "二", "三", "四", "五", "六", "七", "八", "九", "十", "十一", "十二"][repeatIdx.index(cur)] # 最多十四暗
					states[0], states[1] = states[1], states[0] # 交换位置
			elif node in "BEAbea" and mode >> 3 or node in "RNHCrnhc": # 象士揭棋或车马炮
				repeatIdx = [] # 重复记录
				for i in (range(0, 10) if "A" <= node <= "Z" else range(9, -1, -1)): # 红方从上到下
					if lists0[i][cur[1]] == node:
						repeatIdx.append(i)
				if len(repeatIdx) == 2: # 两个重复
					states[1] = "前" if repeatIdx[0] == cur else "后"
					states[0], states[1] = states[1], states[0] # 同时交换位置
				else:
					states[1] = str(9 - cur[1]) if "A" <= node <= "Z" else str(cur[1] + 1) # 红用 9 减黑加 1
			else: # 非揭棋的象士
				states[1] = str(9 - cur[1]) if "A" <= node <= "Z" else str(cur[1] + 1) # 红用 9 减黑加 1
			if "A" <= node <= "Z": # 红子转中文处理
				states[3] = "零一二三四五六七八九"[int(states[3])] # 最后一个字
				if states[1] in "0123456789": # 避免“二兵进一”
					states[1] = "零一二三四五六七八九"[int(states[1])]
			return None if None in states else "".join(states) # 检查局面
		elif useMethod == 2: # 第二种表述（数字纵线格式）
			node = lists0[cur[0]][cur[1]]
			states = [node, None, None, None]
			if "A" <= node <= "Z": # 红棋
				if fur[0] - cur[0] == 0: # 平
					states[2] = "."
					states[3] = str(9 - fur[1]) # 有平必按纵线
				else: # 进退
					states[2] = "+" if fur[0] - cur[0] < 0 else "-" # 红方进退判断
					states[3] = str(9 - fur[1]) if abs(fur[0] - cur[0]) and abs(fur[1] - cur[1]) else str(abs(fur[0] - cur[0])) # 如果两个都变化了按纵线
			elif "a" <= node <= "z": # 黑棋
				if fur[0] - cur[0] == 0: # 平
					states[2] = "."
					states[3] = str(fur[1] + 1) # 有平必按纵线
				else: # 进退
					states[2] = "+" if fur[0] - cur[0] > 0 else "-" # 黑方进退判断
					states[3] = str(fur[1] + 1) if abs(fur[0] - cur[0]) and abs(fur[1] - cur[1]) else str(abs(fur[0] - cur[0])) # 如果两个都变化了按纵线
			if states[2] is None: # still None
				return None
			if node in "Kk": # 将帅唯一
				states[1] = str(9 - cur[1]) if node == "K" else str(cur[1] + 1) # 红用 9 减黑加 1
			elif node in "PXpx": # 多样的兵暗
				repeatIdx = [] # 重复记录
				for i in (range(0, 10) if "A" <= node <= "Z" else range(9, -1, -1)): # 红方从上到下先判断一次
					if lists0[i][cur[1]] == node:
						repeatIdx.append(i)
				if len(repeatIdx) == 1: # 此列仅有一个
					states[1] = str(9 - cur[1]) if node in "PX" else str(cur[1] + 1) # 红用 9 减黑加 1
				else:
					repeatIdx.clear() # 清空（下面开始存储坐标）
					for j in (range(8, -1, -1) if node in "PX" else range(0, 9)): # 红方从右往左
						lineTmp = [] # 单列记录
						for i in (range(0, 10) if node in "PX" else range(9, -1, -1)): # 红方从上到下
							if lists0[i][j] == node: # 相同兵种
								lineTmp.append((i, j))
						if len(lineTmp) > 1: # 有重复
							repeatIdx += lineTmp # 仅对有重复的列添加
					if len(repeatIdx) > 2: # 有多个重复
						states[1] = chr(97 + repeatIdx.index(cur)) # ord("a") = 97
					elif len(repeatIdx) == 2: # 两个重复
						states[1] = "+" if repeatIdx[0] == cur else "-"
			elif node in "BEAbea" and mode >> 3 or node in "RNHCrnhc": # 象士揭棋或车马炮
				repeatIdx = [] # 重复记录
				for i in (range(0, 10) if "A" <= node <= "Z" else range(9, -1, -1)): # 红方从上到下
					if lists0[i][cur[1]] == node:
						repeatIdx.append(i)
				if len(repeatIdx) == 2: # 两个重复
					states[1] = "+" if repeatIdx[0] == cur else "-"
				else:
					states[1] = str(9 - cur[1]) if "A" <= node <= "Z" else str(cur[1] + 1) # 红用 9 减黑加 1
			else: # 非揭棋的象士
				states[1] = str(9 - cur[1]) if "A" <= node <= "Z" else str(cur[1] + 1) # 红用 9 减黑加 1
			return None if None in states else "".join(states) # 检查局面
		elif useMethod == 3: # 第三种表述（坐标格式）
			transf = "ABCDEFGHI" # 转换表述
			return "{0}{1}-{2}{3}".format(transf[cur[1]], 9 - cur[0], transf[fur[1]], 9 - fur[0])
		elif useMethod == 4: # 第四种表述（二维列表格式）
			return "{0}{1}{2}{3}".format(cur[0], cur[1], fur[0], fur[1])
		elif type(useMethod) == list:
			toRet = tuple(Node.getStatement(mode, cur, fur, lists0, uMed) for uMed in useMethod) # 返回元组
			return None if None in toRet else toRet
		else:
			return None
	@staticmethod
	def printChar(ch) -> None:
		print(Node.dicts[ch], end = defaultFore)

class AI:
	statement = {
		0:"傻瓜", 
		1:"小白", 
		2:"菜鸟", 
		3:"新手", 
		4:"入门", 
		5:"业余", 
		6:"专业", 
		7:"大师", 
		8:"特级大师", 
		9:"无敌", 
		10:"天算"
	}
	def __init__(self, level = 1):
		self.level = level if type(level) == int and 0 <= level <= 10 else 1
	def comput(self, board, direction, isEcho = True) -> tuple:
		steps = Node.getAvailableCoordinatesConsideringGeneral(board.mode, board.lists[1] in "WwRr", None, board.lists[0])
		if isEcho:
			print("考虑将军问题时，所有可能的走法如下：")
			print(steps)
		if self.level == 0: # 傻瓜（只会推演一步取最有利于对方的）
			oriBoard = Board(board.mode, board.FEN, eaten = None) # 创建新推演局面
			oriBoard.eaten = board.eaten[::]
			for step in steps:
				oriBoard.generateNextBoard(step, isPrintMethod = False)
			evaluates = [(step[-1], nextBoard.evaluate()) for step, nextBoard in zip(oriBoard.nextStep, oriBoard.next)]
			badValue = {True:min, False:max}[direction](evaluates, key = lambda x:x[1])[1]
			evaluates = [ev for ev in evaluates if ev[1] == badValue]
			return evaluates[randbelow(len(evaluates))][0]
		if self.level == 1: # 小白（随便走）
			return steps[randbelow(len(steps))]
		elif self.level == 2: # 菜鸟（只会推演一步，有吃就吃）
			oriBoard = Board(board.mode, board.FEN, eaten = None) # 创建新推演局面
			oriBoard.eaten = board.eaten[::]
			for step in steps:
				oriBoard.generateNextBoard(step, isPrintMethod = False)
			evaluates = [(step[-1], nextBoard.evaluate()) for step, nextBoard in zip(oriBoard.nextStep, oriBoard.next)]
			goodValue = {True:max, False:min}[direction](evaluates, key = lambda x:x[1])[1]
			evaluates = [ev for ev in evaluates if ev[1] == goodValue]
			return evaluates[randbelow(len(evaluates))][0]
		elif self.level == 3: # 新手（只会推演一回合）
			oriBoard = Board(board.mode, board.FEN, eaten = None) # 创建新推演局面
			oriBoard.eaten = board.eaten[::] if type(board.eaten) == list else None
			evaluates = []
			for step in steps:
				oriBoard.generateNextBoard(step, isPrintMethod = False)
				tmpNextBoard = oriBoard.switchToNextBoard(-1)
				nextSteps = Node.getAvailableCoordinatesConsideringGeneral(tmpNextBoard.mode, tmpNextBoard.lists[1] in "WwRr", None, tmpNextBoard.lists[0])
				if nextSteps:
					for nextStep in nextSteps:
						try:
							tmpNextBoard.generateNextBoard(nextStep, isPrintMethod = False)
						except:
							boardFEN = tmpNextBoard.FEN.split(" ")[0]
							print(boardFEN.count("B"))
							print(boardFEN.count("E"))
							print(boardFEN.count("b"))
							print(boardFEN.count("e"))
							print(boardFEN.count("A"))
							print(boardFEN.count("a"))
							print(len(tmpNextBoard.lists) == 6 and len(tmpNextBoard.lists[0]) == 10 and not any([len(tmpNextBoard.lists[0][i]) != 9 for i in range(10)]))			
							print(boardFEN.count("R") <= 2 and boardFEN.count("r") <= 2)						
							print(boardFEN.count("N") + boardFEN.count("H") <= 2 and boardFEN.count("n") + boardFEN.count("h") <= 2)		
							print(boardFEN.count("B") + boardFEN.count("E") <= 2 and boardFEN.count("b") + boardFEN.count("e") <= 2)		
							print(boardFEN.count("A") <= 2 and boardFEN.count("a") <= 2)						
							print(boardFEN.count("K") == 1 and boardFEN.count("k") == 1)						
							print(boardFEN.count("C") <= 2 and boardFEN.count("c") <= 2)						
							print(boardFEN.count("P") <= 5 and boardFEN.count("p") <= 5)						
							print(len(findall("[A-Z]", boardFEN)) <= 16 and len(findall("[a-z]", boardFEN)) <= 16)				
							print(tmpNextBoard.lists[1] in "WwRrBbGg")									
							print(tmpNextBoard.lists[-2] >> 1 <= tmpNextBoard.lists[-1] - 1)									
							print(tmpNextBoard.mode >> 3 or tmpNextBoard.checkChess())									
							print(not tmpNextBoard.checkIfIsEatingGeneral())									
							print(nextStep)
							input()
					nextEvaluates = [(nextStep[-1], nextBoard.evaluate()) for nextStep, nextBoard in zip(tmpNextBoard.nextStep, tmpNextBoard.next)]
					evaluates.append((step, {True:min, False:max}[direction](nextEvaluates, key = lambda x:x[1])[1]))
				else:
					evaluates.append((step, float("inf") if direction else -float("inf"))) # 做势成杀
			goodValue = {True:max, False:min}[direction](evaluates, key = lambda x:x[1])[1]
			evaluates = [ev for ev in evaluates if ev[1] == goodValue]
			return evaluates[randbelow(len(evaluates))][0]			
		else: # 未开发
			return steps[randbelow(len(steps))]

def clearScreen(fakeClear = 120): # 清屏函数
	if sys.stdin.isatty(): # 在终端
		if PLATFORM == "WINDOWS":
			os.system("cls")
		elif PLATFORM == "LINUX":
			os.system("clear")
		else:
			try:
				print("\n" * int(fakeClear))
			except:
				print("\n" * 120)
	else:
		try:
			print("\n" * int(fakeClear))
		except:
			print("\n" * 120)

def getTxt(filepath, index = 0):#获取 .txt 文本内容
	coding = ("utf-8", "gbk", "utf-16")#目前常见且 Python 支持的编码
	if 0 <= index < len(coding):#索引范围内
		try:
			with open(filepath, "r", encoding = coding[index]) as f:
				content = f.read()
			return content[1:] if content.startswith("\ufeff") else content#如果是带有 BOM 的 utf-8 需要去掉 BOM
		except (UnicodeError, UnicodeDecodeError):
			return getTxt(filepath, index + 1)#递归
		except:
			return None
	else:
		return None#超出索引范围

def endPrompt(board, prompt = None) -> None:
	while True:
		clearScreen()
		board.printBoard()
		if prompt:
			print("\n" + prompt)
		print("M = 返回主菜单 | L = 着法记录 | S = 保存局面过程 | s = 保存当前局面")
		print("请选择一项以继续：", end = "")
		bChoice = getch()
		if bChoice in (b"M", b"m"):
			return
		elif bChoice in (b"L", b"l"):
			board.printLog()
		elif bChoice == b"S":
			board.dump(True)
		elif bChoice == b"s":
			board.dump(False)

def play(mode):
	clearScreen()
	canEndCnt = 3
	stepCnt = 0
	roundCnt = (stepCnt + 1) >> 1
	tSwitch = True
	if mode & 0b0010: # 请指定红方走子等级
		print("所有可能的等级如下：")
		print("\n".join(["\t{0} = {1}".format(key, AI.statement[key]) for key in list(AI.statement.keys())]), end = "\n\n")
		while True:
			try:
				redLevel = int(input("请指定红方走子等级（0 - 10）："))
				if 0 <= redLevel <= 10:
					break
			except:
				pass
		redAI = AI(redLevel)
		clearScreen()
	if mode & 0b0001: # 请指定黑方走子等级
		print("所有可能的等级如下：")
		print("\n".join(["\t{0} = {1}".format(key, AI.statement[key]) for key in list(AI.statement.keys())]), end = "\n\n")
		while True:
			try:
				blackLevel = int(input("请指定黑方走子等级（0 - 10）："))
				if 0 <= blackLevel <= 9:
					break
			except:
				pass
		blackAI = AI(blackLevel)
		clearScreen()
	if mode >> 2 == 0b01:
		while True:
			tmpFEN = input("请输入局面代码或 FEN 文件路径：")
			try:
				board = Board(mode, FEN = tmpFEN) # 从输入初始局面
				break
			except:
				try: # 尝试识别为 FEN 文件路径
					tmpFEN = getTxt(tmpFEN.replace("\"", "").replace("\'", ""))
					if tmpFEN is None:
						print("文件读取失败，请按“M”返回，按任意键重试。")
						if getch() in (b"M", b"m"):
							return
					else:
						if "\n" in tmpFEN: # 多个局面
							tmpFEN = tmpFEN.split("\n")[-1]
						board = Board(mode, FEN = tmpFEN) # 从文件初始局面
						break
				except:
					print("局面代码或文件路径有误，请按任意键重试。")
					getch()
	else:
		board = Board(mode, FEN = Board.Initial_Board_Dark if mode >> 3 else Board.Initial_Board_Light) # 初始局面
	if board.preMsg is None:
		doSpeak(Board.mode_statement[mode] + "，开局！")
	else: # 开局即终局
		board.printBoard()
		doSpeak(board.preMsg)
		print(board.preMsg)
		print("请按任意键返回。")
		getch()
		return
	while True:
		clearScreen()
		board.printBoard()
		if board.lists[1] in "WwRr" and mode & 0b0010 or board.lists[1] in "BbGg" and mode & 0b0001: # 是否电脑走子
			print("电脑思考中，请稍等。")
			if board.lists[1] in "WwRr": # 红方走子
				todo = redAI.comput(board, True)
			elif board.lists[1] in "BbGg": # 黑方走子
				todo = blackAI.comput(board, False)
		else:
			print("\nM = 返回主菜单 | L = 着法记录 | S = 保存局面过程 | s = 保存当前局面")
			print("R = 上一回合 | r = 上一步 | F = 下一回合 | f = 下一步" + (" | T = 提和 | G = 认输" if stepCnt > canEndCnt else ""))
			todo = input("请输入着法（当前{0}方走子）：".format("红" if board.lists[1] in "WwRr" else "黑"))
			if todo in ("M", "m"):
				# 考虑清除内存
				break
			elif todo in ("L", "l"):
				board.printLog()
				continue
			elif todo == b"S":
				board.dump(True)
				continue
			elif todo == b"s":
				board.dump(False)
				continue
			elif todo == "R":
				if board.prev and board.prev.prev:
					board = board.prev.prev
				else:
					print("很抱歉，没有上一回合的记录。\n请按任意键继续。")
					getch()
				continue
			elif todo == "r":
				if board.prev:
					board = board.prev
				else:
					print("很抱歉，没有上一步的记录。\n请按任意键继续。")
					getch()
				continue
			elif todo == "F":
				if len(board.next) <= 0:
					print("很抱歉，当前局面尚未有下一步局面。\n请按任意键继续。")
					getch()
					continue
				else:
					tmp = board.switchToNextBoard()
					if tmp is not None:
						if len(tmp.next) <= 0:
							print("很抱歉，当前局面尚未有下一回合局面。\n请按任意键继续。")
							getch()
						else:
							tmp = tmp.switchToNextBoard()
							if tmp is not None:
								board = tmp # 确认切换
				continue
			elif todo == "f":
				if len(board.next) == 0:
					print("很抱歉，当前局面尚未有下一步局面。\n请按任意键继续。")
					getch()
					continue
				else:
					tmp = board.switchToNextBoard()
					if tmp is not None:
						board = tmp # 确认切换
				continue
			elif stepCnt > canEndCnt and tSwitch and todo in ("T", "t"):
				print("{0}方，你是否愿意接受{1}方的提和？[Y/N]".format("黑" if board.lists[1] in "WwRr" else "红", "红" if board.lists[1] in "WwRr" else "黑"))
				if getch() == b"Y":
					doSpeak("协商结束，双方和棋。")
					endPrompt(board, prompt = "协商结束，双方和棋。")
					break
				else:
					tSwitch = False # 关闭提和开关
					continue
			elif stepCnt > canEndCnt and todo in ("G", "g"):
				doSpeak("{0}方认输，对局结束。".format("红" if board.lists[1] in "WwRr" else "黑"))
				endPrompt(board, prompt = "{0}方认输，对局结束。".format("红" if board.lists[1] in "WwRr" else "黑"))
				break
		results = board.generateNextBoard(todo)
		if results[0]: # 着法正确
			if results[1]: # 此局已终
				clearScreen()
				board = board.switchToNextBoard(-1) # 打印最新局面
				doSpeak(results[2].replace("着", "招"))
				endPrompt(board, prompt = results[2])
				break
			else:
				stepCnt += 1
				roundCnt = stepCnt  >> 1
				board = board.switchToNextBoard(-1)
				tSwitch = True # 允许提和
		else: # 着法不正确
			print(results[2])
			print("请按任意键继续。")
			getch()

def main() -> int:
	while True:
		doSpeak("欢迎使用中国象棋或中国揭棋！")
		clearScreen()
		print("所有支持的模式如下：")
		for key in Board.mode_statement.keys():
			print("\t{0} = {1}".format(hex(key)[2:], Board.mode_statement[key]))
		print("\tq = 退出")
		print("\n请选择一种模式（区分大小写）：", end = "")
		mode = getch()
		if mode == b"q": # 退出
			doSpeak(None)
			clearScreen()
			print("欢迎再次使用！")
			try: # 防止多线程启动失败
				t.join()
			except:
				pass
			clearScreen()
			break
		try:
			mode = int("0x" + chr(int.from_bytes(mode, byteorder = "little")), 16)
		except:
			continue
		if mode in Board.mode_statement.keys():
			play(mode)
	return EXIT_SUCCESS



if __name__ == "__main__":
	sys.exit(main())