
import json 
import os 
import numpy as np
import re
import shutil
import math 
import inspect


from colorama import init, Fore, Back, Style

init(convert=True)

RED = Fore.RED 
GREEN = Fore.GREEN 
CYAN = Fore.CYAN
WHITE = Fore.WHITE

RESET = Style.RESET_ALL

failures = {}

#isHardMode = False
isHardMode = True


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

creationCodeData = { }

newCreationCodesData = {}

# shit horrible code omfg
room = "ugh"


# fuck it, going to be doing this manually.
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

	def instance_destroy():
		global destroy
		destroy = True
		
	def ds_grid_get(a, b, c):
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
			print("fuck")
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
		
	pattern = r'rm_\d+(?:_void)?'
		
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
		temp = temp.replace("!=", "JESUSFUCKINGCHRIST")
		
		temp = temp.replace("!", "not ")
		
		temp = temp.replace("JESUSFUCKINGCHRIST", "!=")
		
		temp = temp.replace("else if", "elif")
		temp = temp.replace("global.cc_state", "None")
		temp = temp.replace("obj_inventory.ds_player_info", "None")
		temp = temp.replace("obj_inventory.ds_equipment", "None")
		temp = temp.replace("global.voidrod_get", "1")
		temp = temp.replace("var ", "")
		
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
				print("wtf, what the fuck. what the fuck")
				exit(1)
			
			temp = temp[:index] + "\"" + temp[index:]
			
				

		execString += ("\t" * indentLevel) + temp + "\n"
	
		
	# this lets break break out of the while loop, which is basically the same as like, just returning
	execString = "while True:\n" + execString + "\tbreak\n"
	
	bruh = locals()
	globalBruh = globals()
	globalBruh["layer"] = None
	globalBruh["destroy"] = False

	try:
		exec(execString, globalBruh, bruh)
	except Exception as e:
		
		print(RED + "readCreationCode fucked up" + RESET)
		
	
		print(creationCode)
		print("\n".join(originalLines))
		print("---\n")
		print("\n".join(lines))
		print("---\n")
		print(execString)
		print("-----")
		
		print(RED + "readCreationCode fucked up" + RESET)
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
	
	return None
	
	
	
	f = open(os.path.join("../ExportData/Export_Code", creationCode + ".gml"))
	lines = [ line.strip() for line in f.readlines() if len(line.strip()) != 0 ]
	f.close()
	idek = "   ".join(lines)
	#creationCodeData[idek] = [None]
	
	if idek not in creationCodeData:
		print(RED + idek + " wasnt found in creationcode data!!! this is rlly fucking bad!!!! add it!!!!" + RESET)
		
		
		newCreationCodesData[idek] = [None]

		#return None
		exit(1)
		
	thisCode = creationCodeData[idek]
	if len(thisCode) == 1:
		return creationCodeData[idek][0]
	
	return creationCodeData[idek][isHardMode]
	
def writeFooter(f, successRoomsList):
	
	data = """

#define LOADROOMMACRO(roomName) Room((void*)&roomName::collision, (void*)&roomName::floor, (void*)&roomName::details, (void*)&roomName::entities, roomName::entityCount, (void*)&roomName::effects, roomName::effectsCount)	

constexpr static inline Room rooms[{:d}] = {{ {:s} }};

// inneffecient, but my gods do i not care at this point
constexpr static inline char roomNames[{:d}][{:d}] = {{ {:s} }};
"""

	# reorder the room list, drop unneeded rooms
	
	roomSelectFile = open("../ExportData/Export_code/gml_GlobalScript_scr_roomselect.gml")
	lines = [ l.strip() for l in roomSelectFile.readlines() if len(l.strip()) != 0 ]
	roomSelectFile.close()
	#successRoomsList
	
	newRoomsList = []
	
	
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
				
			
			
			break

	successRoomsList = newRoomsList

	successRoomsCount = len(successRoomsList)
	longestRoomStringLen = 1+len(max(successRoomsList, key=len))
	
	
	formatArray = lambda prepend, append, arr : ",".join([ "{:s}{:s}{:s}".format(prepend, elem, append) for elem in arr ])

	
	
	roomData = formatArray("LOADROOMMACRO(", ")", successRoomsList)
	roomNameData = formatArray("\"", "\"", successRoomsList)
	
	
	data = data.format(successRoomsCount, roomData, successRoomsCount, longestRoomStringLen, roomNameData)
	
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
		
		# using this second bit here fucks me in terms of the 126 thing, but i still think it will help me in the long run
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
	
	entityExport = []
	
	effectExport = []
	
	# goofy 
	# oh god its so goofy
	# does python seriously not allow lambdas to be multiline/assign shit?
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
					print("a exit had a bform of 1. still no idea why")
					floorExport[p.x][p.y] = "Exit"
					return
			
				getattr(ObjectFunctions, creationCode)(p, None)
				return
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
			
			# i was going to pass "dont" in as a creation code, but that fucked to many things up. so now, im going to see if 
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
			entityExport.append("EntityType::Chest,{:d},{:d}".format(p.x, p.y))

		def obj_boulder(p, creationCode):
		
			if creationCode is None:
				entityExport.append("EntityType::Boulder,{:d},{:d}".format(p.x, p.y))
			else:				
				
				# this being here is an assumption of that only boulders will be statues. i hope that holds.
				
				statueMap = {
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
				effectExport.append("&bn::sprite_tiles_items::dw_spr_statue_cif,{:d},{:d},4,4".format(x, y))
			else:
				print("demonlord statue loaded with unknown creation code: " + creationCode)
				exit(1)

			pass

		def obj_enemy_nm_b(p, creationCode):
			pass

		def obj_gray_d(p, creationCode):
			pass

		def obj_sealchest(p, creationCode):
			pass

		def obj_npc_failure_001(p, creationCode):
			pass

		def obj_npc_nun(p, creationCode):
			pass

		def obj_npc_tail(p, creationCode):
			x, y = p.rawX, p.rawY 
			x += 4
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_upperbody,{:d},{:d},3,5".format(x, y))
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

		def obj_npc_tail_tail(p, creationCode):
			x, y = p.rawX, p.rawY 
			x += 4
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_lowerbody,{:d},{:d},6,3".format(x, y))
			pass

		def obj_npc_failure_003(p, creationCode):
			pass

		def obj_npc_randobandit(p, creationCode):
			pass

		def obj_baal_m_d(p, creationCode):
			pass

		def obj_npc_failure_007(p, creationCode):
			pass

		def obj_chest(p, creationCode):
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

		def obj_voidtail_004(p, creationCode):
		
			x, y = p.rawX, p.rawY 
			
			w, h = 128, 32
			
			x += int(w / 2)
			y += int(h / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_004,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
		
			pass

		def obj_voidtail_005(p, creationCode):
		
			x, y = p.rawX, p.rawY 
	
			w, h = 176, 56
			
			x += int(w / 2)
			y += int(h / 2) + 4
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_x56,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
		
		
			pass

		def obj_voidtail_013(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
	
			w, h = 56, 48
			
			x += int(w / 2)
			y += int(h / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_013,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
		
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

		def obj_voidtail_001(p, creationCode):
			
			x, y = p.rawX, p.rawY 
			
			x += int(16 * 5 / 2)
			y += int(16 * 8 / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_001,{:d},{:d},5,8,false".format(x, y))
		
			pass

		def obj_voidtail_014(p, creationCode):
	
			x, y = p.rawX, p.rawY 
	
			w, h = 112, 128
			
			x += int(w / 2)
			y += int(h / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_014,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
		
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

		def obj_voidtail_008(p, creationCode):
		
			x, y = p.rawX, p.rawY 
			
			x += int(4 / 2) * 16 
			y += int(4 / 2) * 16 
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_008,{:d},{:d},4,4,false".format(x, y))
			pass

		def obj_voidtail_003(p, creationCode):
		
			x, y = p.rawX, p.rawY 
			
			w, h = 80, 16
			
			x += int(w / 2)
			y += int(h / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_003,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
	
			pass

		def obj_voidtail_009(p, creationCode):
		
			# tails are going to cause collision issues, and i rlly should do something abt that, but i wont
			x, y = p.rawX, p.rawY 
			
			x += int(6 / 2) * 16
			y += int(10 / 2) * 16
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_009,{:d},{:d},6,10,false".format(x, y))
		
			pass

		def obj_voidtail_007(p, creationCode):
		
			x, y = p.rawX, p.rawY 
	
			w, h = 64, 148
			
			x += int(w / 2) - 2
			y += int(h / 2) + 4 + 2
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_007,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
		
			pass

		def obj_voidtail_012(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
	
			w, h = 56, 32
			
			x += int(w / 2)
			y += int(h / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_012,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
		
			pass

		def obj_voidtail_011(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
	
			w, h = 112, 128
			
			x += int(w / 2)
			y += int(h / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_011,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))

		
			pass

		def obj_voidtail_006(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
	
			w, h = 128, 56
			
			x += int(w / 2)
			y += int(h / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_006,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
		
			pass

		def obj_voidtail_010(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
	
			w, h = 112, 160
			
			x += int(w / 2)
			y += int(h / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_010,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
		
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

		def obj_voidtail_002(p, creationCode):
		
			
			x, y = p.rawX, p.rawY 
			
			w, h = 64, 48
			
			x += int(16 * (w / 16) / 2)
			y += int(16 * (h / 16) / 2)
			
			effectExport.append("&bn::sprite_tiles_items::dw_spr_tail_void_002,{:d},{:d},{:d},{:d},false".format(x, y, math.ceil(w/16), math.ceil(h/16)))
		
		
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
		print(RED + "there were no entities, not even a player??" + RESET)
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
	
	return [floorExport, entityExport, effectExport]
	
def convertRoom(data, outputFile):

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
		print(RED + "collision/details convert failed!" + RESET)
		return None
		
	collision, details = temp
		
	objectsExport = convertObjects(layerData)
	if objectsExport is None:
		return None
		
	floorExport, instanceExport, effectExport = objectsExport
	
	
	
	output = []
	
	output.append("namespace {:s} {{".format(data["name"]))
	
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
	
	output.append("};")
	
	[ outputFile.write(line + "\n") for line in output ]

	# TODO, WE DONT EXPORT THE TILESET HERE(of what the room tiles should be)
	
	return True

def convertAllRooms(inputPath):

	global creationCodeData
	with open("creationCodeData.json") as f:
		creationCodeData = json.load(f)
	

	f = open("AllRooms.h", "w")
	
	f.write("//Did you know every time you sigh, a little bit of happiness escapes?\n")
	
	#f.write("EffectHolder defaultEffectPlaceholder = {&bn::sprite_tiles_items::dw_spr_statue_abaddon,-1,-1,-1,-1};\n")
	
	jsonFiles = [f for f in os.listdir(inputPath) if f.lower().endswith('.json')]
	
	
	#removeRooms = ["rm_misc_0002"]
	
	#jsonFiles = [ f for f in jsonFiles if f.rsplit(".", 1)[0] not in removeRooms ]

	#removeStrings = ["secret", "test", "misc", "trailer"]
	#removeStrings = ["stg", "house", "secret", "test", "misc", "trailer", "dream", "memories", "bee", "lev", "_ee_"]
	# dont need this, but i think it might give me an extra bit for compression
	removeStrings = ["test", "trailer", "dream", "rm_cc_results", "rm_cif_end", "memories"]
	
	for removeStr in removeStrings:
		# i could, and should one line this
		jsonFiles = [ f for f in jsonFiles if removeStr not in f ]
	
	
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
	
	for file in jsonFiles:
		with open(os.path.join(inputPath, file)) as jsonFilePointer:
			data = json.load(jsonFilePointer)
			
			print("doing room {:s}".format(data["name"]))
			
			res = convertRoom(data, f)
			if res is not None:
				successRooms += 1
				successRoomsList.append(file)
			else:
				print(RED + "failure" + RESET)
			
	
	#print(successRoomsList)
	
	writeFooter(f, [ elem.rsplit(".json", 1)[0] for elem in successRoomsList ])
	
	f.close()
	
	print("")
	
	print("we converted {:6.2f}% rooms({:d}/{:d}), i hope thats acceptable.".format(100*successRooms/totalRooms, successRooms, totalRooms))
	
	print("")
	
	print("primary failure sources:")
	
	#print(json.dumps(failures, indent=4))
	
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
	
	tempFile = open("temp.txt", "w")
	for t in tempSorted:
		tempFile.write("def {:s}(p, creationCode):\n".format(t[0]))
		tempFile.write("\tpass\n")
		tempFile.write("\n")
	tempFile.close()
	
	with open('tempCreationCodeData.json', 'w') as f:
		json.dump(newCreationCodesData, f, indent=4)
	
	print("compressed data had a ratio of {:6.2f}%".format(100 * compressedBytes / uncompressedBytes))
	
	"""
	temp = sorted([ [k, v] for k, v in byteFrequency.items() ], key = lambda x : x[1], reverse=True)
	for byte, freq in temp:
		print("{:5d} {:5d}".format(byte, freq))
	print("max byte value was " + str(max(byteFrequency.keys())))
	print("-----")
	
	temp = sorted([ [k, v] for k, v in frequencyFrequency.items() ], key = lambda x : x[1], reverse=True)
	for byte, freq in temp:
		print("{:5d} {:5d}".format(byte, freq))
	"""
	
	pass

def main():

	os.chdir(os.path.dirname(__file__))

	# run ExportAllRooms.csx(in this dir), name it roomExport, put it in this dir
	# run ExportAllCode, move to this folder
	
	convertAllRooms("../ExportData/Room_Export/")
	
	shutil.copy("AllRooms.h", "../../code/src")
	
	pass
	
if __name__ == "__main__":
	main()

