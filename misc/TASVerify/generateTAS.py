

# tassing manually is annoying, because i do not like the software.
# this is the solution, and will let things be automated

import os
import re
import json
import psutil
import ctypes
from ctypes import wintypes
import time
#import keyboard
#kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)


from colorama import init, Fore, Back, Style

init(convert=True)

RED = Fore.RED 
GREEN = Fore.GREEN 
CYAN = Fore.CYAN
WHITE = Fore.WHITE

RESET = Style.RESET_ALL

def error(s):
	print(RED + s + RESET)
	exit(1)

"""
0x140000000 # offset
0x08B0798  # room id 



move count?
0862F7A0
0862F7B8

# actual move
0862F944

"""

recentPresses = []

class Process:

	def __init__(self, pid):
		self.pid = pid
		#self.process = ctypes.windll.kernel32.OpenProcess(0x1F0FFF, False, self.pid)
		#self.process = ctypes.windll.kernel32.OpenProcess(0x10, False, self.pid)
		#if not self.process:
		#	raise ctypes.WinError(ctypes.get_last_error())
		
		self.keyState = [0,0,0,0,0]
		self.prevMoveCount = -1
		pass
		
	def read(self, addr, size, raw=False):
	
		if not raw:
			addr += 0x140000000	
	
		#res = ctypes.create_string_buffer(size)
		#bytesRead = ctypes.c_size_t(size)
		#what = ctypes.byref(bytesRead)
	
		self.process = ctypes.windll.kernel32.OpenProcess(0x10, False, self.pid)
		
		data = ctypes.create_string_buffer(size)
		bytesRead = ctypes.c_size_t(size)
		
		#data = ctypes.c_ulonglong()
		#bytesRead = ctypes.c_ulonglong()
		# https://stackoverflow.com/questions/33855690/trouble-with-readprocessmemory-in-python-to-read-64bit-process-memory
		result = ctypes.windll.kernel32.ReadProcessMemory(self.process, ctypes.c_void_p(addr), ctypes.byref(data), ctypes.sizeof(data), ctypes.byref(bytesRead))
		e = ctypes.get_last_error()
		
		ctypes.windll.kernel32.CloseHandle(self.process)

		#print(data, data.value, len(data))
		
		return bytes(data.value)
		
	def readRoom(self):
		return int.from_bytes(self.read(0x08B0798, 8))

	def getKeys(self):

		# 0865B6AB possibly for,,, detecting if someting actually moved
		# WHY??? does this only work with raw?
		
		
		# 0E10FF90 unsure, bump count? 
		
		temp = self.read(0x0865B4F0, 1, raw=True)
		temp = int.from_bytes(temp)
		
		print(temp)
		
		"""
		temp = self.read(0x0865B6AB, 1, raw=True)
		temp = int.from_bytes(temp)
			
		if temp != 0xE0:
			return ""
		"""
		
		# 14068AA15 left
		# 14068AA16 up
		# 14068AA17 right
		# 14068AA18 down 
		
		# 14068AA4A use?

		# READ FUNC IS ONLY ABLE TO READ ULONG??? FIX THIS

		tempState = [
			1 if self.read(0x068AA4A, 1) else 0, # use
		
			1 if self.read(0x068AA16, 1) else 0, # up
			1 if self.read(0x068AA18, 1) else 0, # down
			1 if self.read(0x068AA15, 1) else 0, # left
			1 if self.read(0x068AA17, 1) else 0, # right
		]

		risingState = [ (t ^ s) and (s == 0) for t, s in zip(tempState, self.keyState) ]
			
		for i in range(0, 5):
			self.keyState[i] = tempState[i]
		
		# only returning one key here. this will hopefully not MESS things up
		if 1 not in risingState:
			return ""
	
		print("".join([str(v) for v in self.keyState]))
		
		return ["Z", "U", "D", "L", "R"][risingState.index(1)]
		
def updateJSON(room):
	if room is None:
		return
		
	# why i cant get a job:
	#parsedPresses = "".join([ f"{len(m.group())}{m.group()[0]}" for m in re.finditer(r"(.)\1*", "".join([ matches[k] for k in recentPresses ])) ])
	parsedPresses = "".join([ f"{len(m.group())}{m.group()[0]}" for m in re.finditer(r"(.)\1*", recentPresses) ])
	
	print(room, parsedPresses)
		
	data = {}
	with open("TASdata.json", "r") as f:
		data[room] = recentPresses
		
	
	pass
	
def logKeys():
	global recentPresses

	pid = -1
	for proc in psutil.process_iter():
		if proc.name() == "VoidStranger.exe":
			pid = proc.pid 
	
	if pid == -1:
		error("unable to find void stranger, is it running?")
		
	print(f"{GREEN}found vs pid at {pid}{RESET}")
	
	vs = Process(pid)
	
	# VoidStranger.exe+8B0798
	# 1408B0798
	
	
	
	room = None
	while True:
		tempRoom = vs.readRoom()
		recentPresses += vs.getKeys()
		
		#print(bytearray(vs.read(0x0865B4F0, 8)))	
		
		if room != tempRoom:
			updateJSON(room)
			recentPresses = ""
			room = tempRoom
		#time.sleep(1/60)
		time.sleep(1/120)

if __name__ == "__main__":
	os.chdir(os.path.dirname(__file__))

	logKeys()
	exit(0)
	

	with open("TASdata.json", "r") as f:
		data = json.load(f)
	
	for room in list(data.keys()):
		if len(data[room]) == 0:
			del data[room]
	
	# TAS format will be using UDLR for directions, Z for rod, and if multiple presses
	# are desired, a number before them. 
	
	print(data)
	
	pass