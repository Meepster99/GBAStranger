
import json 
import os, sys
import numpy as np
import re
import shutil
import math 
import inspect

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "EasyPoolProcessing"))

from poolQueue import PoolQueue
from multiprocessing import Queue, Pool, cpu_count, Event
import queue
import time 
from threading import Thread


from colorama import init, Fore, Back, Style

init(convert=True)

RED = Fore.RED 
GREEN = Fore.GREEN 
CYAN = Fore.CYAN
WHITE = Fore.WHITE

RESET = Style.RESET_ALL

failures = {}

isHardMode = False
#isHardMode = True


# add: id: 8 spr_voider
# eus: id: 6 spr_lover
# bee: id: 2 spr_smiler
# mon: id: 7 spr_greeder
# tan: id: 3 spr_killer
# gor: id: 5 spr_slower
# lev: id: 1 spr_watcher
# cif: id: 4 spr_atoner

class Pos:
	def __init__(self, x, y):
		self.rawX = x
		self.rawY = y
		self.x = (x - 8) // 16
		self.y = (y - 8) // 16

#creationCodeData = { }

newCreationCodesData = {}

# (curse) horrible code omfg
room = "ugh"


# (curse) it, going to be doing this manually. ( not anymore )
def readCreationCode(p, creationCode):
	
	if creationCode is None:
		return None
		
	if creationCode == "gml_RoomCC_rm_e_000_1_Create":
		# this rooms the push 8 tiles in a row room, i do not want to deal with it rn"
		return None
		
	if creationCode == "dont":
		return None
		
	f = open(os.path.join("../ExportData/Export_Code", creationCode + ".gml"))
	originalLines = [ line.strip() for line in f.readlines() if len(line.strip()) != 0 ]
	f.close()	
	
	lines = list(originalLines)	

	i = 0
	while i < len(lines):
		# to call this bad is an understatement
		if "with " in lines[i] or "with(" in lines[i]:
			lines[i] = "while False:"
			if lines[i+1] != "{":
				lines.insert(i+1, "{")
				lines.insert(i+3, "}")
		i += 1
				
			
	# an alternative. it is HELLA JANK, tho
	execString = ""

	indentLevel = 1
	
	i = 0
	while i < len(lines):
		if ("if" in lines[i] or lines[i] == "else") and lines[i+1] != "{":
			lines.insert(i+1, "{")
			lines.insert(i+3, "}")
		i += 1
		
	i = 0
	while i < len(lines):
		if lines[i] == "{" and lines[i+1] == "}":
			lines.insert(i+1, "pass")
		i += 1
	
	stranger = int(isHardMode)
	obj_music_controller = 0
	spr_n_right = 0
	spr_mural_tile = 0
	# THIS MAY BE HORRID IF X AND Y ARE EVER USED IN COMPARASINS
	x = p.rawX
	y = p.rawY
	destroy = False
	layer = None
	b_form = None
	dl_form = None
	
	def room_goto(roomNameVal):
		global roomName
		roomName = roomNameVal

	def instance_destroy():
		global destroy
		destroy = True
		
	def ds_grid_get(a, b, c):
		return 0
		
	def ds_list_find_value(a, b):
		return 0
	
	def scr_test2_active():
		return 0
		
	def instance_create_layer(x, y, idek, data):
	
	
		
		
		# ive done a lot of coding in my life.
		# and i can safely say that this is the most atrocious thing ive ever done. 
		# why didnt i just make objectfunctions global 
		
		frames = inspect.stack()

		for i, f in enumerate(frames):
			if frames[i].function == "convertObjects":
				break
		else:
			print("(curse)")
			exit(1)
			
		frame = frames[i]
			
		try:
			#frames[4].ObjectFunctions.obj_spawnpoint(x, y, None)
			#print(frames[4].frame.f_locals)
			#exec("ObjectFunctions.obj_spawnpoint(Pos({:d}, {:d}), 'dont')".format(x, y), globals(), frame.frame.f_locals)
			
			#temp = "getattr(ObjectFunctions, '{:s}')(Pos({:d}, {:d}), None)".format(data, x, y)
			#temp = "print(getattr(ObjectFunctions, '{:s}'))".format(data)
			#print(temp)
			#exec(temp, frame.frame.f_globals, frame.frame.f_locals)
			
			temp = "getattr(ObjectFunctions, '{:s}')(Pos({:d}, {:d}), None)".format(data, x, y)
			#print(temp)
			exec(temp, frame.frame.f_globals, frame.frame.f_locals)
			#print("-----")
			
		except Exception as e:
			
			print(e)
			print("omfg")
			exit(1)
		
		
		global destroy
		destroy = True
		return
	
	def instance_create_depth(x, y, idek, data):
		return None
	
	def instance_exists(data):
		return False
		
	removeStrings = [
		"var px = x",
		"var py = y",
		'show_debug_message(((("Player spawn position: " + string(px)) + " x ") + string(py)))',
		"global.player_blink = 1",
		"show_debug_message",
		"randomize()",
		"irandom_range",
	]
	
	# room identification regex, very important, was the thing causing the crash on rm_test2_075
	#pattern = r'rm_\d+(?:_void)?'
	pattern = r'rm_(?:test2_\d+)?\d+(?:_void)?'
	
	for i, l in enumerate(lines):
	
		l = re.sub(pattern, lambda match: f'"{match.group(0)}"', l)
	
		if l == "{":
			indentLevel += 1
			continue
	
		if l == "}":
			indentLevel -= 1
			continue

		temp = l

		if "if" in temp:
			temp += ":"
		
		if temp == "else":
			temp += ":"
		
		
		temp = temp.replace("global.stranger", str(stranger))
		temp = temp.replace("global.loop", str(stranger))
		temp = temp.replace("global.voider", str(stranger))
		
		temp = temp.replace("global.collect_wings", "0")
		
		temp = temp.replace("||", "or")
		temp = temp.replace("&&", "and")
		
		# autism, omfg 
		temp = temp.replace("!=", "JESUS(curse)INGCHRIST")
		
		temp = temp.replace("!", "not ")
		
		temp = temp.replace("JESUS(curse)INGCHRIST", "!=")
		
		temp = temp.replace("else if", "elif")
		temp = temp.replace("global.cc_state", "0")
		temp = temp.replace("obj_inventory.ds_player_info", "None")
		temp = temp.replace("obj_inventory.ds_equipment", "None")
		temp = temp.replace("global.voidrod_get", "1")
		temp = temp.replace("var ", "")
		
		temp = temp.replace("true", "True")
		
		temp = temp.replace("spr_n_up", "0")
		
		
		# ex rooms?
		temp = temp.replace("global.jukebox_song", "jukebox_song")
		temp = temp.replace("obj_inventory.ds_ccr", "False")
		
		
		for s in removeStrings:
			if s in temp:
				temp = "pass"
				break
	
		temp = temp.replace("return;", "break")
	
		if "instance_create_layer" in temp or "instance_create_depth" in temp:
			lastSpaceIndex = temp.rfind(" ")+1
			temp = temp[:lastSpaceIndex] + "\"" + temp[lastSpaceIndex:]
			
			
			temp = temp[:-1] + "\"" + temp[-1:]
			
		if "instance_exists" in temp:
			index = temp.find("instance_exists") + len("instance_exists") + 1
			temp = temp[:index] + "\"" + temp[index:]
			
			while index < len(temp):
				if temp[index] == ")":
					break
				index += 1
			else:
				print("wtf, what the (curse). what the (curse)")
				exit(1)
			
			temp = temp[:index] + "\"" + temp[index:]
			
				

		execString += ("\t" * indentLevel) + temp + "\n"
	
		
	# this lets break break out of the while loop, which is basically the same as like, just returning
	execString = "while True:\n" + execString + "\tbreak\n"
	
	bruh = locals()
	globalBruh = globals()
	globalBruh["layer"] = None
	globalBruh["destroy"] = False
	globalBruh["jukebox_song"] = None

	try:
		exec(execString, globalBruh, bruh)
	except Exception as e:
		
		print(RED + "readCreationCode (curse)ed up" + RESET)
		
	
		print(WHITE + creationCode + RESET)
		print("\n".join(originalLines))
		print("---\n")
		print("\n".join(lines))
		print("---\n")
		
		maxNumLen = len(str(len(execString.split("\n"))))
		
		p = r'line (\d+)'
		errorLines = [ int(n) for n in re.findall(p, str(e)) ]
	
		for i, line in enumerate(execString.split("\n")):
			print(f"{RED if i+1 in errorLines else ""}{i+1:{maxNumLen}}\t{line}{RESET}")
		print("-----")
		
		print(RED + "readCreationCode (curse)ed up" + RESET)
		print(WHITE + creationCode + RESET)
		print(e)
		print("\n")
		exit(1)
		
	
	if "layer" in globalBruh:
		layer = globalBruh["layer"]
	
	if "destroy" in globalBruh:
		destroy = globalBruh["destroy"]
	
	b_form = bruh["b_form"]
	dl_form = bruh["dl_form"]
	
	
	#print(creationCode)
	#print("\n".join(originalLines))
	#print("\n")
	#print("\n".join(lines))
	#print("\n")
	#print(execString)
	#print(layer, destroy, b_form, dl_form)
	#print("-----")
	
	if layer:
		return layer
	
	if destroy: # low prio, since the code destroys the prev thing after creating another
		return "destroy"
	
	if b_form:
		return "b_form = {:d}".format(b_form)

	if dl_form:
		return "dl_form = {:d}".format(dl_form)
		
	if "na_secret" in bruh:
		res = bruh["na_secret"]
		tempRoom = None
		if "roomName" in globalBruh:
			tempRoom = globalBruh["roomName"] 
			
	
		return [res, tempRoom]
	
	if "contents" in bruh:
		return "contents = {:d}".format(bruh["contents"])
	
	return None
	
	
	
	f = open(os.path.join("../ExportData/Export_Code", creationCode + ".gml"))
	lines = [ line.strip() for line in f.readlines() if len(line.strip()) != 0 ]
	f.close()
	idek = "   ".join(lines)
	#creationCodeData[idek] = [None]
	
	if idek not in creationCodeData:
		print(RED + idek + " wasnt found in creationcode data!!! this is rlly (curse)ing bad!!!! add it!!!!" + RESET)
		
		
		newCreationCodesData[idek] = [None]

		#return None
		exit(1)
		
	thisCode = creationCodeData[idek]
	if len(thisCode) == 1:
		return creationCodeData[idek][0]
	
	return creationCodeData[idek][isHardMode]
	
def writeFooter(f, successRoomsList):
	
	data = """
	
constexpr static inline Room {:s}rooms[{:d}] = {{ {:s} }};

// inneffecient, but my gods do i not care at this point
constexpr static inline MessageStrJank {:s}roomNames[] = {{ {:s} }};
"""

	# reorder the room list, drop unneeded rooms
	
	roomSelectFile = open("../ExportData/Export_code/gml_GlobalScript_scr_roomselect.gml")
	lines = [ l.strip() for l in roomSelectFile.readlines() if len(l.strip()) != 0 ]
	roomSelectFile.close()
	#successRoomsList
	
	newRoomsList = []
	
	lineIndex = 0 
	# its 3am, forgive me for this 
	for i, line in enumerate(lines):
		if line == "switch room_number":
			
			i += 1
			
			while lines[i] != "}":
				
				if "case" in lines[i]:
					while lines[i] != "}":
						if "room_goto" in lines[i]:
							newRoomsList.append(lines[i].split("(")[1].split(")")[0])
							break
						i += 1
					
				i+=1
			lineIndex = i
			break
	
	# im just doing this part manually. im tired
	# the room order,, in the actual undertalemodtool seems to be a good ref for this
	
	# ex rooms could(and were for 5 seconds) be done automatically, but due to not all rooms(rm_test2_002) being in the roomsel script, its being added manually
	
	tempRoomsToAdd = """
	
	rm_test2_000
	rm_test2_001
	rm_test2_054
	rm_test2_036
	rm_test2_050
	rm_test2_051
	rm_test2_053
	rm_test2_056
	rm_test2_057
	rm_test2_058
	rm_test2_059
	rm_test2_061
	rm_test2_062
	rm_test2_060
	rm_test2_063
	rm_test2_055
	rm_test2_064
	
	# i have no idea if this is correct
	rm_test2_002
	
	rm_test2_010
	rm_test2_003
	rm_test2_009
	rm_test2_004
	rm_test2_014
	rm_test2_011
	rm_test2_015
	rm_test2_013
	rm_test2_006
	rm_test2_007
	rm_test2_008
	rm_test2_005
	rm_test2_012
	rm_test2_016
	rm_test2_017
	
	# see above note for confusion
	rm_test2_018
	
	rm_test2_031
	rm_test2_022
	rm_test2_019
	rm_test2_023
	rm_test2_020
	rm_test2_024
	rm_test2_029
	rm_test2_032
	rm_test2_026
	rm_test2_030
	rm_test2_021
	rm_test2_028
	rm_test2_025
	rm_test2_027
	rm_test2_033
	
	# see above note for confusion
	rm_test2_034
	
	rm_test2_035
	rm_test2_046
	rm_test2_038
	rm_test2_052
	rm_test2_039
	rm_test2_047
	rm_test2_040
	rm_test2_041
	rm_test2_042
	rm_test2_048
	rm_test2_043
	rm_test2_044
	rm_test2_045
	rm_test2_037
	rm_test2_049
	
	rm_u_0001
	rm_u_0002
	rm_u_0003
	rm_u_0004
	rm_u_0005
	rm_u_0006
	rm_u_0007
	rm_u_0008
	rm_u_0009
	rm_u_0010
	rm_u_0011
	rm_u_0012
	rm_u_0013
	rm_u_0014
	rm_u_0015
	rm_u_0016
	rm_u_0017
	rm_u_0018
	rm_u_0019
	rm_u_0020
	rm_u_0021
	rm_u_0022
	rm_u_0023
	rm_u_0024
	rm_u_0025
	rm_u_0026
	rm_u_0027
	rm_u_0028
	rm_u_end
	
	rm_secret_001
	rm_secret_002
	rm_secret_003
	rm_secret_004
	rm_secret_005
	rm_secret_006
	rm_secret_007
	rm_secret_008
	rm_secret_009
	
	rm_bee_001
	rm_bee_002
	rm_bee_003
	rm_bee_004
	rm_bee_005
	rm_bee_006
	rm_bee_007
	rm_bee_008
	rm_bee_009
	rm_bee_010
	rm_bee_011
	rm_bee_012
	rm_bee_013
	rm_bee_014
	rm_misc_0002
	rm_bee_015
	
	rm_e_intermission
	rm_e_001
	rm_e_023
	rm_e_004
	rm_e_002
	rm_e_006
	rm_e_007
	rm_e_018
	rm_e_009
	rm_e_005
	rm_e_010
	rm_e_026
	rm_e_012
	rm_e_019
	rm_e_011
	rm_e_013
	rm_e_017
	rm_e_014
	rm_e_024
	rm_e_015
	rm_e_022
	rm_e_025
	rm_e_020
	rm_e_016
	rm_e_008
	rm_e_003
	rm_e_021
	rm_e_027
	rm_e_000
	
	rm_mon_001
	rm_mon_002
	rm_mon_003
	rm_mon_004
	rm_mon_005
	rm_mon_006
	rm_mon_007
	rm_mon_008
	rm_mon_009
	rm_mon_010
	rm_mon_011
	rm_mon_012
	rm_mon_013
	rm_mon_014
	rm_mon_015
	rm_test_0006
	rm_mon_016
	
	rm_mon_shortcut_001
	rm_mon_shortcut_002
	rm_mon_shortcut_003
	rm_mon_shortcut_004
	rm_mon_shortcut_005
	
	
	
	
	rm_rm4
	"""
	
	# rm_rm4 MUST be the last one
	
	[ newRoomsList.append(t.strip()) for t in tempRoomsToAdd.split("\n") if len(t.strip()) != 0 and t.strip()[0] != "#" ]
	
	newRoomsList[newRoomsList.index("rm_u_0001")] = "rm_voidend"
			
	successRoomsList = newRoomsList

	successRoomsCount = len(successRoomsList)
	longestRoomStringLen = 1+len(max(successRoomsList, key=len))
	
	if isHardMode:
		longestRoomStringLen += 5
	
	formatArray = lambda prepend, append, arr : ",".join([ "{:s}{:s}{:s}".format(prepend, elem, append) for elem in arr ])

	if not isHardMode:
		f.write("// goofy legacy code\n");
		f.write("#define LOADROOMMACRO(roomName) Room((void*)&roomName::collision, (void*)&roomName::floor, (void*)&roomName::details, (void*)&roomName::entities, roomName::entityCount, (void*)&roomName::effects, roomName::effectsCount, (void*)&roomName::secrets, roomName::secretsCount, (void*)roomName::exitDest, (void*)roomName::collisionTiles, (void*)roomName::detailsTiles)\n")

		f.write("""
// should this be different between normal and hard?? idek
#define MAXROOMS {:d}
		
// i (curse)ing despise that this is the only way i can get this to work
struct MessageStrJank {{
	const char* str;
	const char idek = '\\0'; // sanity
}};
""".format(successRoomsCount))
	
	prepend = "LOADROOMMACRO(" if not isHardMode else "LOADROOMMACRO(hard_"
	roomData = formatArray(prepend, ")", successRoomsList)
	
	prepend = "MessageStrJank(\"" if not isHardMode else "MessageStrJank(\"hard_"
	roomNameData = formatArray(prepend, "\\0\")", successRoomsList)
	
	hardModeString = "hard_" if isHardMode else ""
	data = data.format(hardModeString, successRoomsCount, roomData, hardModeString, roomNameData)
	
	f.write(data + "\n")

	pass

uncompressedBytes = 0
compressedBytes = 0

byteFrequency = {}
frequencyFrequency = {}

def compressData(arr):

	global uncompressedBytes
	global compressedBytes
	global maxByteVal
	
	# basic rle
	
	# further stuff could be compressed by combining everything into one array(tbh why didnt i do that)
	
	#maxByteVal = max(maxByteVal, max(arr))
	for elem in arr:
		if elem not in byteFrequency:
			byteFrequency[elem] = 0	
		byteFrequency[elem] += 1
		
	
	res = []
	
	count = 1
	val = arr[0]
	for i in range(1, len(arr) + 1):
		
		# using this second bit here (curse)s me in terms of the 126 thing, but i still think it will help me in the long run
		# this reduced my bits by like,, 2% but idc, im leaving it in
		if arr[min(i, len(arr)-1)] & 0xC0: # 1100 0000 
			print("a value passed into the compression func had bit 7 or bit 6 set. this is not allowed!")
			exit(1)
		
		if i == len(arr) or arr[i] != val:
			
			if count == 1 or count == 2 or count == 3:
				if count not in frequencyFrequency:
					frequencyFrequency[count] = 0
				frequencyFrequency[count] += 1
				
				res.append(val | (count << 6))
			else:
				res.append(count)
				res.append(val)
				
				if count not in frequencyFrequency:
					frequencyFrequency[count] = 0
				frequencyFrequency[count] += 1
			
			if i == len(arr):
				break
		
			count = 1
			val = arr[i]
		else:
			count += 1
			
			if count == 0x40:
				
				count-=1
			
				# this range is about to overflow into the upper two bits, prevent that
				if count not in frequencyFrequency:
					frequencyFrequency[count] = 0
				frequencyFrequency[count] += 1
				
				res.append(count)
				res.append(val)
				
				count = 1
				val = arr[i]
				
		
	uncompressedBytes += len(arr)
	compressedBytes += len(res)
	
	return res
	
def uncompressData(comp):

	# because 14*9=126 < 128, this means that bit 8 will always be safe, thank the gods

	uncomp = []
	i = 0
	while i < len(comp):
		
		if comp[i] & 0xC0:
			
			count = comp[i] >> 6
			val = comp[i] & ~0xC0
		
			uncomp += [val] * count
			i+=1
		else:
			count = comp[i]
			val = comp[i+1]
			uncomp += [val] * count
			i += 2
			
	return uncomp

def convertCollisionAndDetails(layerData):
	
	details = {}	
	
	if "Tiles_2" in layerData:
		
		details["tileset"] = layerData["Tiles_2"]["layer_data"]["background"]
		details["data"] = [ elem["id"] for line in layerData["Tiles_2"]["layer_data"]["tile_data"] for elem in line]

		for elem in details["data"]:
			if elem >= 64:
				return None

		if details["tileset"] == "tile_house_2":
			print("tile_house_2 aint properly converted!")
			return None
	else:
		details["data"] = [ 0 for i in range(0, 14*9) ]
	
	collision = {}	
	
	if "Tiles_1" in layerData:
		
		collision["tileset"] = layerData["Tiles_1"]["layer_data"]["background"]
		collision["data"] = [ elem["id"] for line in layerData["Tiles_1"]["layer_data"]["tile_data"] for elem in line]

		for elem in collision["data"]:
			if elem >= 64:
				return None

		if collision["tileset"] == "tile_house_2":
			print("tile_house_2 aint properly converted!")
			return None
	else:
		collision["data"] = [ 0 for i in range(0, 14*9) ]
	
	if len(details["data"]) == 0:
		details["data"] = [ 0 for i in range(0, 14*9) ]
	
	if len(collision["data"]) == 0:
		collision["data"] = [ 0 for i in range(0, 14*9) ]
	
	
	# basic compression
		
	start = collision["data"]
	comp = compressData(start)
	uncomp = uncompressData(comp)

	if not np.array_equal(np.array(start), np.array(uncomp)):
		print("array decomp somehow failed???")
		exit()
		
	collision["data"] = comp
	
	start = details["data"]
	comp = compressData(start)
	uncomp = uncompressData(comp)

	if not np.array_equal(np.array(start), np.array(uncomp)):
		print("array decomp somehow failed???")
		exit()

	details["data"] = comp
		
	return collision, details

def convertObjects(layerData):
	
	# due to the extremely large amount of difficulty ive had with floor objects being instances, and vice versa, this is here to fix thtat
	
	
	floorExport = [ ["Pit" for i in range(9)] for j in range(14)]
	
	specialFloorExport = []
	floorExitDest = []
	
	entityExport = []
	
	effectExport = []
	
	# goofy 
	# oh god its so goofy
	# does python seriously not allow lambdas to be multiline/assign (curse)?
	# i despise that this is actually the best/most readable/easiest way to do this
	class ObjectFunctions:
		
		def obj_floor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Floor"
	
		def obj_glassfloor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Glass"
			
		def obj_pit(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Pit"
			
		def obj_exit(p, creationCode):
			if creationCode is not None:
				if creationCode == "b_form = 1":
					print(RED + "a exit had a bform of 1. still no idea why" + RESET)
					floorExport[p.x][p.y] = "Exit"
					return
			
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			
			
			# gml_Object_obj_exit_Alarm_0 
			# in the future, the data here should also like,, decide where they go for the voided end?
			# that may require some reimplimentation, but thats for the future
			
			customExitData = {
				"rm_secret_001": "rm_0025",
				"rm_secret_002": "rm_0056",
				"rm_secret_003": "rm_0072",
				"rm_secret_004": "rm_0114",
				"rm_secret_005": "rm_rest_area_5",
				"rm_secret_006": "rm_0168",
				"rm_secret_007": "rm_0185",
				"rm_secret_008": "rm_rest_area_8", 
				"rm_0000": "rm_1intro",
				"rm_rm4": "rm_2intro",
				"rm_0230": "rm_rest_area_8",
				"rm_secret_009": "rm_0235",
				"rm_rest_area_9": "rm_voidend",
				"rm_rest_area_8": "rm_0231",
				"rm_0259": "rm_rest_area_9",
				"rm_mon_015": "rm_test_0006",
				"rm_mon_016": "rm_rest_area_7",
				
				"rm_mon_shortcut_001": "rm_0023",
				"rm_mon_shortcut_002": "rm_0054",
				"rm_mon_shortcut_003": "rm_0112",
				"rm_mon_shortcut_004": "rm_0172",
				"rm_mon_shortcut_005": "rm_0202",
				
				"rm_mon_001": "rm_0201",
				
				"rm_bee_001": "rm_0149",
				"rm_misc_0001": "rm_rest_area_6",
				"rm_test2_002": "rm_test2_003",
				"rm_test2_018": "rm_test2_019",
				"rm_test2_034": "rm_test2_035",
				
				"rm_bee_015": "rm_0165",
				"rm_return_000": "rm_e_000",
				"rm_ee_000": "rm_2intro",
				"rm_ee_006": "rm_2intro",
				"rm_ee_008": "rm_noway",

				
			}
			
			
			if layerData["roomName"] in customExitData:
				#specialFloorExport.append("{:d},{:d},\"{:s}\"".format(p.x, p.y, customExitData[layerData["roomName"]]))
				print("custom exit from {:s} to {:s}".format(layerData["roomName"], customExitData[layerData["roomName"]]))
				floorExitDest.append(customExitData[layerData["roomName"]])
			
			floorExport[p.x][p.y] = "Exit"
			
		def obj_floorswitch(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Switch"
			
		def obj_bombfloor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Bomb"
			
		def obj_deathfloor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Death"
			
		def obj_copyfloor(p, creationCode):
			if creationCode is not None:
				getattr(ObjectFunctions, creationCode)(p, None)
				return
			floorExport[p.x][p.y] = "Copy"
			
	
	

		def obj_spawnpoint(p, creationCode):	
		
			res = ""
		
			# this code makes me want to cry 
			
			# i was going to pass "dont" in as a creation code, but that (curse)ed to many things up. so now, im going to see if 
			# this is being called from exec via the stack frames. omfg 
			
		
			res = readCreationCode(p, "gml_Object_obj_spawnpoint_Create_0")
			if res == "destroy":
				#print(CYAN + "room: {:s} creating player at alternate pos, i hope lmao".format(room) + RESET)
				return
			elif res is not None:
				print("a player had a creation code, this is not defined!")
				exit(1)
						
			entityExport.insert(0, "EntityType::Player,{:d},{:d}".format(p.x, p.y))
			
		def obj_player(p, creationCode): 
			# this one like,, gods idek whats going on anymore
			entityExport.insert(0, "EntityType::Player,{:d},{:d}".format(p.x, p.y))
			
		def obj_chest_small(p, creationCode):
		
			chestCreationCodes = {
				None: "EntityType::Chest",
				"contents = 0": "EntityType::EmptyChest", # actually should be a chest that gives nothing by default
				"contents = 1": "EntityType::Chest",
				"contents = 5": "EntityType::EmptyChest",
				
				# unknown. i think that 5 is empty, these are special stuff 
				"contents = 4": "EntityType::EmptyChest",
				"contents = 3": "EntityType::EmptyChest",
				"contents = 2": "EntityType::EmptyChest",
				"contents = 6": "EntityType::EmptyChest",
				"contents = 7": "EntityType::EmptyChest",
				"contents = 8": "EntityType::EmptyChest",
				"contents = 9": "EntityType::EmptyChest",
				
				# ex? 
				"contents = 10": "EntityType::EmptyChest",
			}
			
			if creationCode not in chestCreationCodes:
				print("couldnt find what to do with a chest with creationcode: ", creationCode)
				exit(1)
		
			entityExport.append("{:s},{:d},{:d}".format(chestCreationCodes[creationCode], p.x, p.y))

		def obj_boulder(p, creationCode):
		
			if creationCode is None:
				entityExport.append("EntityType::Boulder,{:d},{:d}".format(p.x, p.y))
			else:				
				
				# this being here is an assumption of that only boulders will be statues. i hope that holds.
				
				statueMap = {
					# HEY THIS NEEDS TO BE UPDATED
					"b_form = 9": lambda p : "EntityType::JukeBox,{:d},{:d}".format(p.x, p.y),
					
					"b_form = 8": lambda p : "EntityType::AddStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 7": lambda p : "EntityType::MonStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 6": lambda p : "EntityType::EusStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 5": lambda p : "EntityType::GorStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 4": lambda p : "EntityType::CifStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 3": lambda p : "EntityType::TanStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 2": lambda p : "EntityType::BeeStatue,{:d},{:d}".format(p.x, p.y),
					"b_form = 1": lambda p : "EntityType::LevStatue,{:d},{:d}".format(p.x, p.y),
				}
				
				entityExport.append(statueMap[creationCode](p))
				
		def obj_enemy_cg(p, creationCode):
			# i should be checking creationcode here, but i,,,, dont want to!
			entityExport.append("EntityType::Bull,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cl(p, creationCode):
			entityExport.append("EntityType::Leech,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cc(p, creationCode):
			entityExport.append("EntityType::Maggot,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_ch(p, creationCode):
			entityExport.append("EntityType::Eye,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cs(p, creationCode):
			entityExport.append("EntityType::Chester,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cl(p, creationCode):
			entityExport.append("EntityType::Leech,{:d},{:d}".format(p.x, p.y))
		
		def obj_enemy_cm(p, creationCode):
			 entityExport.append("EntityType::Mimic,{:d},{:d}".format(p.x, p.y))
			 
		def obj_enemy_co(p, creationCode):
			entityExport.append("EntityType::Diamond,{:d},{:d}".format(p.x, p.y))
	
	
	
	
		def obj_collision(p, creationCode):
			# for now, donothing, but i could maybe,,, modify this to put a black tile in collision/details that has collision?
			pass
			
		def obj_dustemiter_unknown(p, creationCode):
			pass
			
		def obj_enter_the_secret(p, creationCode):
			ObjectFunctions.obj_spawnpoint(p, creationCode)
		
		def obj_na_secret_exit(p, creationCode):
		
			# general room switch: gml_GlobalScript_scr_roomselect
		
			# contains the,, mapping of secrets to like yea: gml_Object_obj_na_secret_exit_Alarm_5
			# 
			# 
	
			if creationCode is None:
				specialFloorExport.append("{:d},{:d},NULL".format(p.x, p.y))
				return
			
			#i could get the room to goto dynamically, but im tired ok
			
			creationCode = creationCode[0]
			
			secretData = {
				4: "rm_mon_shortcut_001",
				1: "next",
				5: "rm_mon_shortcut_002",
				6: "rm_mon_shortcut_003",
				7: "rm_mon_shortcut_004",
				8: "rm_mon_shortcut_005",
				9: "rm_return_000",
				10: "rm_bee_001",
				2: "rm_mon_001",
				3: "rm_cif_end",
			
				11: "rm_test2_002",
				12: "rm_test2_018",
				13: "rm_test2_034",
				14: "next",
				15: "rm_trailer_001", # are trailer rooms playable??
			}
			
			
			
			if creationCode not in secretData:
				print("creationCode not in secret data! how! ", str(creationCode))
				
				exit(1)
			
			res = secretData[creationCode]
			
			if res == "next":
				specialFloorExport.append("{:d},{:d},NULL".format(p.x, p.y))
			else:
				specialFloorExport.append("{:d},{:d},\"{:s}\"".format(p.x, p.y, res))
			
			pass
			
		def obj_npc_friend(p, creationCode):
			pass
			
		def obj_mural(p, creationCode):
			pass
				
		def obj_enemy_nm(p, creationCode):
			pass
			
		def obj_secret_exit(p, creationCode):
			pass
			
		def obj_solarsystem(p, creationCode):
			pass
		
		def obj_rest(p, creationCode):
			x, y = p.rawX, p.rawY 
			# ideally, these offsets would be gotten from the images themself, but like,,, im tired 
	
			effectExport.append("&bn::sprite_tiles_items::dw_spr_birch,{:d},{:d},3,4".format(x, y))
			
			pass
		
		def obj_dustemit(p, creationCode):
			pass
			
		def obj_fakewall(p, creationCode):
			pass
			
		def obj_enemy_invisiblemon(p, creationCode):
			pass

		def obj_npc_mon(p, creationCode):
			pass

		def obj_npc_riddle(p, creationCode):
			pass

		def obj_npc_ykko(p, creationCode):
			pass

		def obj_demonlords_statue(p, creationCode):
			x, y = p.rawX, p.rawY 
			
			if creationCode is None:
				effectExport.append("&bn::sprite_tiles_items::dw_spr_statue_baal,{:d},{:d},6,4".format(x, y))
			elif creationCode == "dl_form = 1":
				effectExport.append("&bn::sprite_tiles_items::dw_spr_statue_sa,{:d},{:d},6,4".format(x, y))
			elif creationCode == "dl_form = 2":
				effectExport.append("&bn::sprite_tiles_items::dw_spr_statue_lev,{:d},{:d},3,4".format(x, y))
			elif creationCode == "dl_form = 3":
				effectExport.append("&bn::sprite_tiles_items::dw_spr_statue_cif,{:d},{:d},4,4".format(x - 8, y))
			else:
				print("demonlord statue loaded with unknown creation code: " + creationCode)
				exit(1)

			pass

		def obj_enemy_nm_b(p, creationCode):
			pass

		def obj_gray_d(p, creationCode):
			pass

		def obj_npc_failure_001(p, creationCode):
			pass

		def obj_npc_nun(p, creationCode):
			pass
		
		def _tailCoordinateFunction(x, y, w, h):
		
			x += int(w / 2)
			y += int(h / 2)
		
			w = int(math.ceil(w/16))
			h = int(math.ceil(h/16))
			
			# bigsprite's coordinates like, have a seperate system, and since tails can be both normal and bigsprites, this handles the conversion issues
		
			if w <= 4 and h <= 4:
				# small sprite
				
				x -= 0
				y += 16
				
				
		
		
			return [x, y, w, h]
		
			pass
		def obj_npc_tail(p, creationCode):
			x, y = p.rawX, p.rawY 
			x += 4
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_upperbody,{:d},{:d},3,5,false,2,true".format(x, y))
			
			y += 2 * 16
			y -= 3
			y += 8;
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_boobytrap,{:d},{:d},3,3,false,1".format(x, y))
		
			pass
			
		def obj_npc_tail_tail(p, creationCode):
			x, y = p.rawX, p.rawY 
			x += 4
			
			y += 4
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_lowerbody,{:d},{:d},6,3,false".format(x, y))
			
			"""
			x += 16
			y -= 2 * 16
			y -= 6
			x -= 1
			y -= 1
			"""
			
			x, y = p.rawX, p.rawY 
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, 52, 40)
			x, y = p.rawX, p.rawY 
			
			y -= 28 - 1
			x += 12 - 1
			

			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_tail,{:d},{:d},{:d},{:d},false,1,true".format(x, y, w, h))
			
			pass
		
		def obj_voidtail_004(p, creationCode):
		
			x, y = p.rawX, p.rawY 
			
			w, h = 128, 32
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_004,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
			pass

		def obj_voidtail_005(p, creationCode):
		
			x, y = p.rawX, p.rawY + 4
	
			w, h = 176, 56
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_x56,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
		
			pass

		def obj_voidtail_013(p, creationCode):
		
			x, y = p.rawX - 8, p.rawY - 8
			
			w, h = 56, 48
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_013,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
			pass
		
		def obj_voidtail_001(p, creationCode):
			
			x, y = p.rawX, p.rawY 
			
			w, h = 80, 128
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_001,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
			pass

		def obj_voidtail_014(p, creationCode):
	
			x, y = p.rawX, p.rawY 
	
			w, h = 112, 128
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_014,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
			pass

		def obj_voidtail_008(p, creationCode):
		
			x, y = p.rawX, p.rawY - 12
			
			w, h = 64, 52
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_008,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
			pass

		def obj_voidtail_003(p, creationCode):
		
			x, y = p.rawX, p.rawY 
			
			w, h = 80, 16
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_003,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
	
			pass

		def obj_voidtail_009(p, creationCode):
		
			# tails are going to cause collision issues, and i rlly should do something abt that, but i wont
			x, y = p.rawX, p.rawY 
			
			w, h = 96, 160
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_009,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
			pass

		def obj_voidtail_007(p, creationCode):
		
			x, y = p.rawX, p.rawY + 6
	
			w, h = 64, 148
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_007,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
			pass

		def obj_voidtail_012(p, creationCode):
		
			
			x, y = p.rawX - 8, p.rawY - 16
	
			w, h = 56, 32
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_012,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
			pass

		def obj_voidtail_011(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
	
			w, h = 112, 128
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_011,{:d},{:d},{:d},{:d},false".format(x, y, w, h))

		
			pass

		def obj_voidtail_006(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
	
			w, h = 128, 56
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_006,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
			pass

		def obj_voidtail_010(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
	
			w, h = 112, 160
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_010,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
			pass
		
		def obj_voidtail_002(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
			
			w, h = 64, 48
			
			x, y, w, h = ObjectFunctions._tailCoordinateFunction(x, y, w, h)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_002,{:d},{:d},{:d},{:d},false".format(x, y, w, h))
		
		
			pass
			
		def obj_chest(p, creationCode):
			x, y = p.rawX - 8, p.rawY 
			effectExport.append("&bn::sprite_tiles_items::dw_spr_chest,{:d},{:d},2,1".format(x, y))
			pass
			
		def obj_sealchest(p, creationCode):
			x, y = p.rawX - 8, p.rawY 
			effectExport.append("&bn::sprite_tiles_items::dw_spr_chest,{:d},{:d},2,1".format(x, y))
			pass

			
		def obj_npc_failure_002(p, creationCode):
			pass

		def obj_npc_bride(p, creationCode):
			pass

		def obj_npc_famished(p, creationCode):
			pass

		def obj_npc_failure_004(p, creationCode):
			pass

		def obj_npc_nomad(p, creationCode):
			pass

		def obj_npc_unknown(p, creationCode):
			pass

		def obj_npc_nomad2(p, creationCode):
			pass

		def obj_npc_failure_005(p, creationCode):
			pass

		def obj_npc_spawner(p, creationCode):
			pass

		def obj_npc_gor(p, creationCode):
			x, y = p.rawX, p.rawY 
			effectExport.append("&bn::sprite_tiles_items::dw_spr_gor,{:d},{:d},3,4,false".format(x, y))
			
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_gor_hair,{:d},{:d},7,4,false".format(x, y))
			
			pass

		def obj_npc_failure_006(p, creationCode):
			pass

		def obj_npc_failure_008(p, creationCode):
			pass

		def obj_npc_oldman(p, creationCode):
			pass

		def obj_randomfloorlayout(p, creationCode):
			pass

		def obj_enter_the_danger(p, creationCode):
			ObjectFunctions.obj_spawnpoint(p, creationCode)

		def obj_spawn_intro(p, creationCode):
			ObjectFunctions.obj_spawnpoint(p, creationCode)

		def obj_intermission(p, creationCode):
			pass

		def obj_change_music(p, creationCode):
			pass

		def obj_cc_menu(p, creationCode):
			pass

		def obj_cif_epilogue2(p, creationCode):
			pass

		def obj_gateofdis(p, creationCode):
			pass

		def obj_ex_aftermath(p, creationCode):
			pass

		def obj_riddle_000(p, creationCode):
			pass

		def obj_tot_mural(p, creationCode):
			pass

		def obj_riddle_002(p, creationCode):
			pass

		def obj_riddle_003(p, creationCode):
			pass

		def obj_riddle_004(p, creationCode):
			pass

		def obj_riddle_005(p, creationCode):
			pass

		def obj_riddle_006(p, creationCode):
			pass

		def obj_riddle_001(p, creationCode):
			pass

		def obj_elevator_activate(p, creationCode):
			pass

		def obj_stainedglass_eclipse(p, creationCode):
			pass

		def obj_gray_finalmeeting(p, creationCode):
			pass

		def obj_zero_d(p, creationCode):
			pass

		def obj_lillie_d(p, creationCode):
			pass

		def obj_results(p, creationCode):
			pass

		def obj_confinement(p, creationCode):
			pass

		def obj_enemy_ct(p, creationCode):
			pass

		def obj_soundtest(p, creationCode):
			pass

		def obj_staffroll_b(p, creationCode):
			pass

		def obj_npc_dr_ab___on(p, creationCode):
			pass
		
		def obj_baal_d(p, creationCode):
			pass
			
		def obj_npc_failure_003(p, creationCode):
			pass

		def obj_npc_randobandit(p, creationCode):
			pass

		def obj_baal_m_d(p, creationCode):
			pass

		def obj_npc_failure_007(p, creationCode):
			pass

		def obj_baal_epilogue2(p, creationCode):
			pass

		def obj_gateofdis_roots(p, creationCode):
			pass

		def obj_sera_d(p, creationCode):
			pass

		def obj_intro2_bg(p, creationCode):
			pass

		def obj_lillie_finalmeeting(p, creationCode):
			pass

		def obj_freya_d(p, creationCode):
			pass

		def obj_a_corpse(p, creationCode):
			pass

		def obj_cifer_d(p, creationCode):
			pass

		def obj_tail_kill(p, creationCode):
			pass

		def obj_statue_abaddon(p, creationCode):
			x, y = p.rawX, p.rawY 
			# ideally, these offsets would be gotten from the images themself, but like,,, im tired 

			effectExport.append("&bn::sprite_tiles_items::dw_spr_statue_abaddon,{:d},{:d},6,4".format(x, y))
			
			pass

		def obj_gateofdis_bg(p, creationCode):
			pass

		def obj_cutscene_finalwaltz(p, creationCode):
			pass

		def obj_fm_cemetery(p, creationCode):
			pass

		def obj_npc_tail_floor(p, creationCode):
			pass

		def obj_cutscene_epilogue_part3(p, creationCode):
			pass

		def obj_cutscene_grayend(p, creationCode):
			pass

		def obj_cutscene_finalmeeting(p, creationCode):
			pass

		def obj_cutscene_lillie_end(p, creationCode):
			pass

		def obj_npc_tail_lowerfloor(p, creationCode):
			pass

		def obj_cifandbee_bg(p, creationCode):
			pass

		def obj_stinklines(p, creationCode):
		
			x, y = p.rawX, p.rawY 
		
			effectExport.append("&bn::sprite_tiles_items::dw_spr_stinklines,{:d},{:d},1,1".format(x, y))
			
			pass

		def obj_spawnpoint__dream(p, creationCode):
			pass

		def obj_npc_nolla(p, creationCode):
			pass

		def obj_floor_lev(p, creationCode):
			pass

		def obj_npc_levextra(p, creationCode):
			pass

		def obj_gor_cube(p, creationCode):
			pass

		def obj_horse(p, creationCode):
			pass

		def obj_johann_dreamIX(p, creationCode):
			pass

		def obj_gray_dreamVII(p, creationCode):
			pass

		def obj_gray_dreamVI(p, creationCode):
			pass

		def obj_prin_dreamV(p, creationCode):
			pass

		def obj_seal(p, creationCode):
			pass

		def obj_floor_hpn(p, creationCode):
			pass

		def obj_music_controller_v2(p, creationCode):
			pass

		def obj_gray_dreamIII(p, creationCode):
			pass

		def obj_bigwindow(p, creationCode):
			pass

		def obj_gray_dreamXI(p, creationCode):
			pass

		def obj_gray_dreamIX_b(p, creationCode):
			pass

		def obj_torch(p, creationCode):
			pass

		def obj_prin_c(p, creationCode):
			pass

		def obj_gray_dreamVIII(p, creationCode):
			pass

		def obj_prin_dreamVIII(p, creationCode):
			pass

		def obj_johann_dreamVIII(p, creationCode):
			pass

		def obj_ee_eclipse(p, creationCode):
			pass

		def obj_enemy_floormaster(p, creationCode):
			pass

		def obj_og_jewel(p, creationCode):
			pass

		def obj_dr_un(p, creationCode):
			pass

		def obj_player_a(p, creationCode):
			pass

		def obj_player_child(p, creationCode):
			pass

		def obj_enemy_5(p, creationCode):
			pass

		def obj_tan_intro(p, creationCode):
			pass

		def obj_mas_stuff(p, creationCode):
			pass

		def obj_baal_battleintro(p, creationCode):
			pass

		def obj_mon_battleintro(p, creationCode):
			pass

		def obj_enemy_cifhandl(p, creationCode):
			pass

		def obj_wagon(p, creationCode):
			pass

		def obj_npc_ran001(p, creationCode):
			pass

		def obj_prin_dreamVI(p, creationCode):
			pass

		def obj_gray_dreamV(p, creationCode):
			pass

		def obj_prin_dreamIII(p, creationCode):
			pass

		def obj_gray_dreamIV(p, creationCode):
			pass

		def obj_prison_bucket(p, creationCode):
			pass

		def obj_torch_fallen(p, creationCode):
			pass

		def obj_prin_dreamIX(p, creationCode):
			pass

		def obj_baal_dreamIX(p, creationCode):
			pass

		def obj_curtains(p, creationCode):
			pass

		def obj_gray_c(p, creationCode):
			pass

		def obj_enter_door(p, creationCode):
			pass

		def obj_eclipse_begin(p, creationCode):
			pass

		def obj_jguard_dreamVIII_a(p, creationCode):
			pass

		def obj_prin_dreamVII(p, creationCode):
			pass

		def obj_npc_ran002_dreamV(p, creationCode):
			pass

		def obj_npc_ran003_dreamV(p, creationCode):
			pass

		def obj_npc_ran001_dreamV(p, creationCode):
			pass

		def obj_ee_enemies(p, creationCode):
			pass

		def obj_ee_bg_future(p, creationCode):
			pass

		def obj_ex_levbg_001(p, creationCode):
			pass

		def obj_giant(p, creationCode):
			pass

		def obj_dummyfloor(p, creationCode):
			pass

		def obj_og_onion(p, creationCode):
			pass

		def obj_stairs_001(p, creationCode):
			pass

		def obj_lock(p, creationCode):
			pass

		def obj_tan_intro_body(p, creationCode):
			pass

		def obj_riddle_007(p, creationCode):
			pass

		def obj_cif_monbattleintro(p, creationCode):
			pass

		def obj_enemy_cifhandr(p, creationCode):
			pass
		
		def obj_pr_cupboard(p, creationCode):
			pass

		def obj_cifer_dreamIX(p, creationCode):
			pass

		def obj_exit_dream(p, creationCode):
			pass

		def obj_npc_ran002(p, creationCode):
			pass

		def obj_wagon_inside(p, creationCode):
			pass

		def obj_rando_dreamIII(p, creationCode):
			pass

		def obj_bandit_dreamIII(p, creationCode):
			pass

		def obj_johann_dreamIV(p, creationCode):
			pass

		def obj_jguard_dreamVIII_b(p, creationCode):
			pass

		def obj_johann_dreamV(p, creationCode):
			pass

		def obj_stairs_002(p, creationCode):
			pass

		def obj_enemy_invmon_prelude(p, creationCode):
			pass

		def obj_npc_ran003(p, creationCode):
			pass

		def obj_henchman_ded_dreamIII(p, creationCode):
			pass

		def obj_judgement_crater(p, creationCode):
			pass

		def obj_zero_dreamIX(p, creationCode):
			pass

		def obj_open_door(p, creationCode):
			pass

		def obj_jguard_dreamVIII(p, creationCode):
			pass

		def obj_jguard_dreamV(p, creationCode):
			pass

		def obj_stairs_002b(p, creationCode):
			pass

		def obj_mural_dream(p, creationCode):
			pass

		def obj_stairs_003(p, creationCode):
			pass

		def obj_floor_un(p, creationCode):
			pass
	
		def obj_console(p, creationCode):
			pass

		def obj_un_bgdecour_001(p, creationCode):
			pass

		def obj_memorywindow(p, creationCode):
			pass

		def obj_un_bgdecour_002(p, creationCode):
			pass

		def obj_ee_bonus(p, creationCode):
			pass

		def obj_soul_eyecatch(p, creationCode):
			pass

		def obj_prin_dreamII(p, creationCode):
			pass

		def obj_gray_dreamIX(p, creationCode):
			pass

		def obj_prin_blanket(p, creationCode):
			pass

		def obj_ex_prine_wakeup(p, creationCode):
			pass

		def obj_og_p_turret(p, creationCode):
			pass

		def obj_un_bgdecour_003(p, creationCode):
			pass

		def obj_prin_ending_eyecatch(p, creationCode):
			pass

		def obj_prin_d(p, creationCode):
			pass

		def obj_dustemiter_lev(p, creationCode):
			pass

		def obj_jguard_dreamIX(p, creationCode):
			pass

		def obj_princess_room(p, creationCode):
			pass

		def obj_princess_room_dreamV(p, creationCode):
			pass

		def obj_ex_testspawn(p, creationCode):
			pass

		def obj_music_controller(p, creationCode):
			pass

		def obj_begin(p, creationCode):
			pass

		def obj_sm_statue(p, creationCode):
			pass

		def obj_rando_d(p, creationCode):
			pass

		def obj_dreamIII_battle(p, creationCode):
			pass

		def obj_bandit_d(p, creationCode):
			pass

		def obj_king_d(p, creationCode):
			pass

		def obj_cutscene_dreamIV_snowfall(p, creationCode):
			pass

		def obj_jguard_dreamIXb(p, creationCode):
			pass

		def obj_ranknight_dreamIX(p, creationCode):
			pass

		def obj_archway(p, creationCode):
			pass

		def obj_cutscene_dreamIX_sacrifice(p, creationCode):
			pass

		def obj_battle_dreamXI(p, creationCode):
			pass

		def obj_cutscene_dreamIX_void(p, creationCode):
			pass

		def obj_dream_battle(p, creationCode):
			pass

		def obj_cutscene_dreamVIII_morning(p, creationCode):
			pass

		def obj_cutscene_dreamVII_dress(p, creationCode):
			pass

		def obj_johann_dreamVII(p, creationCode):
			pass

		def obj_cutscene_dreamV_intro(p, creationCode):
			pass

		def obj_ee_gray(p, creationCode):
			pass

		def obj_ee_bg_elements(p, creationCode):
			pass

		def obj_ee_earlyconcept(p, creationCode):
			pass

		def obj_ee_lillie(p, creationCode):
			pass

		def obj_cutscene_intro(p, creationCode):
			pass

		def obj_cutscene_floormaster(p, creationCode):
			pass

		def obj_og_bg(p, creationCode):
			pass

		def obj_soldier_un(p, creationCode):
			pass

		def obj_enemy_tan_dummy(p, creationCode):
			pass

		def obj_stg_menu(p, creationCode):
			pass

		def obj_test_0000(p, creationCode):
			pass

		def obj_teaser(p, creationCode):
			pass

		def obj_cutscene_true_beebattle(p, creationCode):
			pass

		def obj_mon_d(p, creationCode):
			pass

		def obj_cutscene_true_cifbattle1(p, creationCode):
			pass

		def obj_johannc_d(p, creationCode):
			pass

		def obj_jguard_d(p, creationCode):
			pass

		def obj_ballroom(p, creationCode):
			pass

		def obj_ex_controller(p, creationCode):
			pass

		def obj_init(p, creationCode):
			pass

		def obj_camera(p, creationCode):
			pass

		def obj_cutscene_dreamIII_wagon(p, creationCode):
			pass

		def obj_cutscene_dreamIII_bandit(p, creationCode):
			pass

		def obj_johann_d(p, creationCode):
			pass

		def obj_cutscene_dreamII_introp(p, creationCode):
			pass

		def obj_cutscene_dreamII_cemetery(p, creationCode):
			pass

		def obj_cutscene_dreamIV_intro(p, creationCode):
			pass

		def obj_cutscene_dreamIX_prison(p, creationCode):
			pass

		def obj_cutscene_dreamIX_escape1(p, creationCode):
			pass

		def obj_cutscene_dreamIX_escape2(p, creationCode):
			pass

		def obj_jguard_dreamIXd(p, creationCode):
			pass

		def obj_cutscene_dreamIX_menacing(p, creationCode):
			pass

		def obj_princess_room_fore(p, creationCode):
			pass

		def obj_cutscene_prin_reveal(p, creationCode):
			pass

		def obj_cutscene_dreamVIII_balcony(p, creationCode):
			pass

		def obj_cutscene_dreamVI_rytmipeli(p, creationCode):
			pass

		def obj_og_controller(p, creationCode):
			pass

		def obj_mon_m_d(p, creationCode):
			pass

		def obj_ex_background_controller(p, creationCode):
			pass

		def obj_dragwindow(p, creationCode):
			pass

		def obj_collision_persistent(p, creationCode):
			pass

		def obj_henchman_dreamIII(p, creationCode):
			pass

		def obj_cutscene_dreamIII_rescue(p, creationCode):
			pass

		def obj_gray_dreamII(p, creationCode):
			pass

		def obj_blackwall(p, creationCode):
			pass

		def obj_cutscene_dreamIX_knights(p, creationCode):
			pass

		def obj_cutscene_dreamIX_sacrifice_part2(p, creationCode):
			pass

		def obj_cutscene_dreamVIII_betrayal(p, creationCode):
			pass

		def obj_cutscene_dreamVII_johann(p, creationCode):
			pass

		def obj_cutscene_dreamVI_practice(p, creationCode):
			pass

		def obj_cutscene_dreamVI_break(p, creationCode):
			pass

		def obj_cutscene_dreamV_announcement(p, creationCode):
			pass

		def obj_dr_d(p, creationCode):
			pass

		def obj_enemy_mon_dummy(p, creationCode):
			pass

		def obj_snowy_bg(p, creationCode):
			pass

		def obj_ex_wavecontroller(p, creationCode):
			pass

		def obj_game(p, creationCode):
			pass

		def obj_ranknight_d(p, creationCode):
			pass

		def obj_cutscene_dr_end(p, creationCode):
			pass

		def obj_cutscene_monbattle(p, creationCode):
			pass

		def obj_com_d(p, creationCode):
			pass

		def obj_stg_modecontroller(p, creationCode):
			pass

		def obj_voidshatter(p, creationCode):
			pass

		def obj_npc_mimic(p, creationCode):
			# new 1.0.6 content?
			pass
			
		def obj_riddle_008(p, creationCode):
			# new as of ex
			pass

		def obj_floor_hpn2(p, creationCode):
			# DEF NEW AS OF EX, 
			# rm_test2_054
			# going to cry 
			# this will mess with floor code. im going to cry
			# temp fix:
			ObjectFunctions.obj_floor(p, creationCode)
			pass
		
		def obj_floor_hpn3(p, creationCode):
			ObjectFunctions.obj_floor(p, creationCode)

		def obj_floor_hpn4(p, creationCode):
			ObjectFunctions.obj_floor(p, creationCode)

		def obj_floor_hpn(p, creationCode):
			ObjectFunctions.obj_floor(p, creationCode)


	
	allData = []
	
	for label in ["Instances", "Floor_INS", "Floor", "Pit", "Player"]:
		if label in layerData:
			allData += layerData[label]["layer_data"]["instances"]
	
			
	
	for inst in allData:
	
		#x, y = (inst["x"] - 8) // 16, (inst["y"] - 8) // 16
		
		p = Pos(inst["x"], inst["y"])

		objectDef = inst["object_definition"]
		creationCodeFilename = inst["creation_code"]
		
		
		
		#if p.x < 0 or p.x >= 14 or p.y < 0 or p.y >= 9:
		#	continue
		
	
		
	
		if hasattr(ObjectFunctions, objectDef):
		
			creationCode = readCreationCode(p, creationCodeFilename)
			# avoid needing this check every time 
			if creationCode == "destroy":
				continue
		
			getattr(ObjectFunctions, objectDef)(p, creationCode)
		else:
		
			print("we dont have a definition for {:s} in convert entities".format(objectDef))
		
			if objectDef not in failures:
				failures[objectDef] = []
		
			failures[objectDef].append(layerData["roomName"])
			
			return None
	
	# remove any entities at a below 0
	# we allow effects
	for e in list([ e for e in entityExport ]):
		x, y = [ int(x) for x in e.split(",")[1:] ]
		
		if x < 0 or x >= 14 or y < 0 or y >= 9:
			entityExport.remove(e)
			
	# MAKE SURE THE PLAYER HAS A SPAWNPOINT HERE
	if len(entityExport) == 0:
		#print(RED + "there were no entities, not even a player??" + RESET)
		return None
		
		
	# this is dumb, and also (i think) slows compile time considerably.
	# changing this to not waste space
	# nvm, imjust going to hope O3 works
	effectExport.insert(0, "&bn::sprite_tiles_items::dw_spr_statue_abaddon,-1,-1,-1,-1")
	
	# convert the floor array to values
	floorExport = np.transpose(floorExport).flatten()
	
	# MAKE SURE THIS IS THE SAME AS THE ENUM IN SHAREDTYPES
	
	tileTypes = {
		"Pit": 0,
		"Floor": 1,
		"Glass": 2,
		"Bomb": 3,
		"Death": 4,
		"Copy": 5,
		"Exit": 6,
		"Switch": 7,
		"WordTile": 8,
		"RodTile": 9,
		"LocustTile": 10,
	}
	
	floorExport = [ tileTypes[tile] for tile in floorExport ]
	
	start = floorExport
	comp = compressData(start)
	uncomp = uncompressData(comp)

	if not np.array_equal(np.array(start), np.array(uncomp)):
		print("array decomp somehow failed???")
		exit()
		
	floorExport = comp
	
	entityPoses = set()
	for e in entityExport:
		temp = tuple([ int(x) for x in e.split(",")[1:]])
		if temp in entityPoses:
			print(RED + "room " + room + " had a duplicate entity at " + str(temp))
			exit(1)
		entityPoses.add(temp)
	
	specialFloorExport.insert(0, "-1,-1,NULL")
	
	return [floorExport, entityExport, effectExport, specialFloorExport, floorExitDest]
	
def convertRoom(data, isHardModePass):

	global isHardMode
	isHardMode = isHardModePass

	layerData = {}
	
	for l in data["layers"]:
		layerData[l["layer_name"]] = l
	
	layerData["roomName"] = data["name"]
	
	global room
	room = data["name"]
	
	
	formatArray = lambda arr : "".join([ "{:d},".format(elem) for elem in np.array(arr).flatten() ])
	
	formatFullArray = lambda name, arr : "constexpr static inline u8 {:s}[] = {{{:s}}};".format(name, formatArray(arr))
	
	temp = convertCollisionAndDetails(layerData)
	
	if temp is None:
		#print(RED + "collision/details convert failed!" + RESET)
		return None
		
	collision, details = temp
		
	objectsExport = convertObjects(layerData)
	if objectsExport is None:
		return None
		
	floorExport, instanceExport, effectExport, specialFloorExport, floorExitDest = objectsExport
	

	output = []
	
	tempNamespaceName = data["name"] if not isHardMode else "hard_" + data["name"]
	
	output.append("namespace {:s} {{".format(tempNamespaceName))
	
	if len(details["data"]) == 0:
		details["data"] = [ [0 for i in range(9)] for j in range(14)]
	
	output.append(formatFullArray("collision", collision["data"]))
	output.append(formatFullArray("details", details["data"]))

	output.append("constexpr static inline u8 floor[] = {" + "".join([ "{:d},".format(instance) for instance in floorExport ]) + "};")
	
	output.append("constexpr static inline EntityHolder entities[] = {")	
	output.append("".join([ "{{{:s}}},".format(instance) for instance in instanceExport ]))
	output.append("};")
	
	output.append("constexpr static inline int entityCount = {:d};".format(len(instanceExport)))
	
	output.append("constexpr static inline EffectHolder effects[] = {")
	output.append("".join([ "{{{:s}}},".format(effect) for effect in effectExport ]))
	output.append("};")	
	
	output.append("constexpr static inline int effectsCount = {:d};".format(len(effectExport)))
	
	output.append("constexpr static inline SecretHolder secrets[] = {")
	output.append("".join([ "{{{:s}}},".format(secret) for secret in specialFloorExport ]))
	output.append("};")	
	
	output.append("constexpr static inline int secretsCount = {:d};".format(len(specialFloorExport)))
	
	if len(floorExitDest) == 0:
		output.append("constexpr static inline const char* exitDest = NULL;")
	else:
		output.append("constexpr static inline const char* exitDest = \"{:s}\\0\";".format(floorExitDest[0]))
		
	
	if "tileset" not in collision or collision["tileset"] is None:
		collision["tileset"] = "tile_bg_1"
	
	if "tileset" not in details or details["tileset"] is None:
		details["tileset"] = "tile_edges"
	
	
	output.append("constexpr static inline const bn::regular_bg_tiles_item* collisionTiles = &bn::regular_bg_tiles_items::dw_{:s};".format(collision["tileset"]))
	output.append("constexpr static inline const bn::regular_bg_tiles_item* detailsTiles = &bn::regular_bg_tiles_items::dw_{:s};".format(details["tileset"]))
	
	output.append("};")
	
	return output

def convertRoomWorker(jobQueue: Queue, returnQueue: Queue, shouldStop: Event):

	while not shouldStop.is_set():
			
		try:
			data = jobQueue.get(timeout = 0.5)
		except queue.Empty:
			time.sleep(0.001)
			continue

		data, isHardModePass = data
			
		res = convertRoom(data, isHardModePass)
		
		returnQueue.put([data["name"], res])

def convertAllRoomsWorker(f, isHardModePass):

	inputPath = "../ExportData/Room_Export/"
	
	jsonFiles = [file for file in os.listdir(inputPath) if file.lower().endswith('.json')]
	
	# dont need this, but i think it might give me an extra bit for compression'
	# misc is needed for the,,, bee face room?
	# test is needed for rm_test_0006
	# i dont want to cause issues so, ima just manually readd it 
	removeStrings = ["test", "trailer", "dream", "rm_cc_results", "rm_cif_end", "memories"]
	includeStrings = ["test2"]
	
	for removeStr in removeStrings:
		# i could, and should one line this
		jsonFiles = [ file for file in jsonFiles if removeStr not in file or "test2" in file]
	
	jsonFiles.append("rm_test_0006.json")
	jsonFiles.append("rm_rm4.json")
	
	#jsonFiles = ["rm_0005.json"]
	#jsonFiles = ["rm_0027.json"]
	#jsonFiles = ["rm_0008.json"]
	#jsonFiles = ["rm_0018.json"]
	#jsonFiles = ["rm_2intro.json"]
	#jsonFiles = ["rm_0002.json"]
	#jsonFiles = ["rm_0009.json"]
	#jsonFiles = ["rm_0008.json", "rm_0009.json"]
	
	successRooms = 0 
	totalRooms = len(jsonFiles)
	
	successRoomsList = []
	
	allData = []
	
	for file in jsonFiles:
		with open(os.path.join(inputPath, file)) as jsonFilePointer:
			data = json.load(jsonFilePointer)
			allData.append(data)
	
	global isHardMode
	isHardMode = isHardModePass
	
	#pool = PoolQueue(convertRoomWorker, cpuPercent = 0.75)
	#pool = PoolQueue(convertRoomWorker, cpuPercent = 0.05)
	
	#pool.start()
	
	resData = {}
	
	for data in allData:
		#pool.send([data, isHardModePass])
		print("doing room " + data["name"])
		temp = convertRoom(data, isHardModePass)
		resData[data["name"]] = temp
		
	#resData = pool.join()
	
	for roomName, roomData in resData.items():
		
		if roomData is not None:
			successRooms += 1
			successRoomsList.append(roomName)
			[ f.write(line + "\n") for line in roomData ]
		else:
			print(RED + roomName + " failure" + RESET)
		
	
	writeFooter(f, [ elem.rsplit(".json", 1)[0] for elem in successRoomsList ])
	
	print("")
	
	print("we converted {:6.2f}% rooms({:d}/{:d}), i hope thats acceptable.".format(100*successRooms/totalRooms, successRooms, totalRooms))
	
	print("")
	
	print("primary failure sources:")

	for _, v in failures.items():
		if type(v) != list:
			print("wtf")
			exit(1)
	
	tempSorted = sorted([ [k, len(v), v] for k, v in failures.items() ], key = lambda elem : len(elem[2]), reverse=True)
	
	highlightStrings = ["mon", "_e_"]
	
	for t in tempSorted:
		temp = str(t)
		
		col = WHITE
		
		for s in highlightStrings:
			if s in temp:
				col = RED
				break
				
		print(col + temp + RESET)
	print("")
	
	#print("compressed data had a ratio of {:6.2f}%".format(100 * compressedBytes / uncompressedBytes))

	# program hands without this being here?
	print("convertAllRoomsWorker done\n")
	
	pass
	
def convertAllRooms():

	"""
	global creationCodeData
	with open("creationCodeData.json") as f:
		creationCodeData = json.load(f)
	"""

	f = open("AllRooms.h", "w")
	
	f.write("//Did you know every time you sigh, a little bit of happiness escapes?\n")

	
	# this is (curse)ing dumb, i should be recording the CHANGES between the normal and hard mode, this turns all the effort i put into compression to (curse)
	isHardModePass = False
	
	print("starting normal mode conversion")
	convertAllRoomsWorker(f, isHardModePass)
	
	print("starting hard mode conversion")
	isHardModePass = True
	convertAllRoomsWorker(f, isHardModePass)
	
	f.close()
	
	pass

def main():

	os.chdir(os.path.dirname(__file__))

	# run ExportAllRooms.csx(in this dir), name it roomExport, put it in this dir
	# run ExportAllCode, move to this folder
	
	convertAllRooms()
	print("done, copying roomdata")
	
	shutil.copy("AllRooms.h", "../../code/src")
	
	
	print("room conversion success")

	# sometimes (most likely due to multiprocessing) this program hangs??
	
	
	return None
	
if __name__ == "__main__":
	main()

