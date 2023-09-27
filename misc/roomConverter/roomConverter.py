
import json 
import os 
import numpy as np
import re

from colorama import init, Fore, Back, Style

init(convert=True)

RED = Fore.RED 
GREEN = Fore.GREEN 
CYAN = Fore.CYAN
WHITE = Fore.WHITE

RESET = Style.RESET_ALL


# there might be a way to,,,, do this via a datamine?
# but for now like,,, things are still much to difficult 
# how does the game decide what direction a enemy should start facing?
# fuck it, ill deal with that later.
# how does the game decide what enemies to spawn during,, hard mode??
# some sprites are even spawned on top of each other?
# i need a room order as well??


# gml_GlobalScript_scr_roomselect
# seems important 
# gods, i guess i need to export all code now.

"""

gml_Object_obj_na_secret_exit_Alarm_5
gml_GlobalScript_scr_roomselect




"""

"""

# each sprite has a creation code, which contains either:

gml_RoomCC_rm_0095_3_Create 
if (global.loop == 1)
    instance_destroy()
	
if (global.loop == 0)
    instance_destroy()

it seems that global.loop represents hardmode state. 
fuck.
we are going to have to import that too??????????
also some sprites are layered upon each other??

but there are also some varieties between allthis like,, 
gods this is going to be EXTREMELY painful.
im going to have to change up my 

gods.

"""

"""
# christ 

MyClass* (*factories[])() = {
		[]() -> MyClass* { return new MyClass(42); },
		[]() -> MyClass* { return new MyClass(32, 5); },

	};

"""

def createSpawnData(d, x, y):
	#return "new Player({:d}, {:d})".format(x, y)
	return "EntityType::Player,{:d},{:d}".format(x, y)
	
def createLeech(d, x, y):
	#return "new Leech({:d}, {:d}, {:s})".format(x, y, "Direction::Right")
	return "EntityType::Leech,{:d},{:d}".format(x, y)

def createMaggot(d, x, y):
	# no clue abt starting dir
	#return "new Maggot({:d}, {:d}, {:s})".format(x, y, "Direction::Down")
	return "EntityType::Maggot,{:d},{:d}".format(x, y)

def createEye(d, x, y):
	#return "new Eye({:d}, {:d})".format(x, y)
	return "EntityType::Eye,{:d},{:d}".format(x, y)

def createBull(d, x, y):
	#return "new Bull({:d}, {:d})".format(x, y)
	return "EntityType::Bull,{:d},{:d}".format(x, y)

def createChester(d, x, y):
	#return "new Chester({:d}, {:d}, {:s})".format(x, y, "Direction::Right")
	return "EntityType::Chester,{:d},{:d}".format(x, y)

	
# abysmal use of globals.
playerX, playerY = -1, -1 
	
def createMimic(d, x, y):
	# gml_Object_obj_enemy_cm_Create_0
	
	"""
	if x > playerX:
		x_rotation = 0
	if x <= playerX:
		x_rotation = 1
	if y > playerY:
		y_rotation = 1
	if y <= playerY:
		y_rotation = 0
		
	# black people are the default, i think?
	res = "white"
	
	
	if x_rotation == 0 and y_rotation == 1: # greater x greater y
		# black 
		res = "black"
	elif x_rotation == 1 and y_rotation == 0: # lessereq x lessereq y
		res = "gray"
	elif x_rotation == 1 and y_rotation == 1: # lessereq x greater y
		res = "white"
	else:
		print(RED + "TRAUMATIC" + RESET)
	"""
	
	res = "black"
	
	if x == playerX:
		res = "gray"
		
	if y == playerY:
		res = "white"
		
		
	temp = {
		"black": "EntityType::BlackMimic,{:d},{:d}".format(x, y),
		"gray": "EntityType::GrayMimic,{:d},{:d}".format(x, y),
		"white": "EntityType::WhiteMimic,{:d},{:d}".format(x, y),
	}
	
	return temp[res]

instanceMap = {
	"obj_spawnpoint": createSpawnData,
	"obj_enter_the_secret": createSpawnData,
	
	#"obj_npc_nun": createDefault,

	"obj_enemy_cl": createLeech,
	"obj_enemy_cc": createMaggot,
	"obj_enemy_ch": createEye,
	"obj_enemy_cg": createBull,
	"obj_enemy_cs": createChester,
	
	"obj_boulder": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	
	"obj_npc_riddle": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	
	"obj_riddle_000": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	"obj_riddle_001": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	"obj_riddle_002": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	"obj_riddle_003": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	"obj_riddle_004": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	"obj_riddle_005": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	"obj_riddle_006": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	"obj_riddle_007": lambda d, x, y : "EntityType::Boulder,{:d},{:d}".format(x, y),
	
	
	
	"obj_chest_small": lambda d, x, y : "EntityType::Chest,{:d},{:d}".format(x, y),
	
	"b_form = 8": lambda d, x, y : "EntityType::AddStatue,{:d},{:d}".format(x, y),
	"b_form = 7": lambda d, x, y : "EntityType::MonStatue,{:d},{:d}".format(x, y),
	"b_form = 6": lambda d, x, y : "EntityType::EusStatue,{:d},{:d}".format(x, y),
	"b_form = 5": lambda d, x, y : "EntityType::GorStatue,{:d},{:d}".format(x, y),
	"b_form = 4": lambda d, x, y : "EntityType::CifStatue,{:d},{:d}".format(x, y),
	"b_form = 3": lambda d, x, y : "EntityType::TanStatue,{:d},{:d}".format(x, y),
	"b_form = 2": lambda d, x, y : "EntityType::BeeStatue,{:d},{:d}".format(x, y),
	"b_form = 1": lambda d, x, y : "EntityType::LevStatue,{:d},{:d}".format(x, y),
	
	"obj_enemy_cm": createMimic,
	
	"obj_enemy_co": lambda d, x, y : "EntityType::Diamond,{:d},{:d}".format(x, y),
	
	
	
}


floorMap = {

	"obj_floor": "Floor",
	"obj_glassfloor": "Glass",
	"obj_pit": "Pit",
	"obj_exit": "Exit",
	
	# this is just temp
	#"obj_chest_small": 0,
	"obj_chest_small": "FUCk",
	"obj_boulder": "FUCK",
	
	"obj_floorswitch": "Switch",
	"obj_bombfloor": "Bomb",
	"obj_deathfloor": "Death",
	"obj_copyfloor": "Copy",
}

failures = {}

isHardMode = False


# add: id: 8 spr_voider
# eus: id: 6 spr_lover
# bee: id: 2 spr_smiler
# mon: id: 7 spr_greeder
# tan: id: 3 spr_killer
# gor: id: 5 spr_slower
# lev: id: 1 spr_watcher
# cif: id: 4 spr_atoner


creationCodeData = { }

newCreationCodesData = {}

# fuck it, going to be doing this manually.
def readCreationCode(inst, creationCode):

	x, y = (inst["x"] - 8) // 16, (inst["y"] - 8) // 16

	
	f = open(os.path.join("Export_Code", creationCode + ".gml"))
	lines = [ line.strip() for line in f.readlines() if len(line.strip()) != 0 ]
	f.close()
	idek = "   ".join(lines)
	#creationCodeData[idek] = [None]
	
	if idek not in creationCodeData:
		print(idek + " wasnt found in creationcode data!!! this is rlly fucking bad!!!! add it!!!!")
		
		
		#newCreationCodesData[idek] = [None]

		#return [None]
		exit(1)
	
	return creationCodeData[idek]
	
def writeFooter(f, successRoomsList):
	
	data = """

#define LOADROOMMACRO(roomName) Room((void*)&roomName::collision, (void*)&roomName::floor, (void*)&roomName::details, (void*)&roomName::entities, roomName::entityCount)	

constexpr static inline Room rooms[{:d}] = {{ {:s} }};

// inneffecient, but my gods do i not care at this point
constexpr static inline char roomNames[{:d}][{:d}] = {{ {:s} }};
"""

	idjk="""

class RoomManager {{
public:
	// what i am about to do here, is in a word, horrible 
	// but my gods im tired.
	// literally just put a array of strings and array of structs
	//,, will the structs be like
	// GODS 
	// i fucking hate how paranoid i am with memory here. 
	// im trying my best to make sure that we only load the room needed from rom but 
	// maybe im being to paranoid
	// also i havent ate.

	// getready for a bad time.
	
	// should be constexpr but im tired
	
	

	int roomIndex = 100;
	
	Room loadRoom() {{
	
		if(roomIndex < 0 || roomIndex >= (int)(sizeof(roomNames)/sizeof(roomNames[0]))) {{
			BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
		}}

		return rooms[roomIndex];
	}}
	
	const char* currentRoomName() {{ 
	if(roomIndex < 0 || roomIndex >= (int)(sizeof(roomNames)/sizeof(roomNames[0]))) {{
			BN_ERROR("Roomindex ", roomIndex, " out of bounds, max ", sizeof(roomNames)/sizeof(roomNames[0]));
	}}
	
	return roomNames[roomIndex]; 
	}}

}};

"""

	successRoomsCount = len(successRoomsList)
	longestRoomStringLen = 1+len(max(successRoomsList, key=len))
	
	
	formatArray = lambda prepend, append, arr : ",".join([ "{:s}{:s}{:s}".format(prepend, elem, append) for elem in arr ])

	
	
	roomData = formatArray("LOADROOMMACRO(", ")", successRoomsList)
	roomNameData = formatArray("\"", "\"", successRoomsList)
	
	
	data = data.format(successRoomsCount, roomData, successRoomsCount, longestRoomStringLen, roomNameData)
	
	f.write(data + "\n")

	pass

def convertCollisionAndDetails(layerData):
	
	details = {}	
	details["tileset"] = layerData["Tiles_2"]["layer_data"]["background"]
	details["data"] = [ elem["id"] for line in layerData["Tiles_2"]["layer_data"]["tile_data"] for elem in line]

	if details["tileset"] == "tile_house_2":
		print("tile_house_2 aint properly converted!")
		return None
	
	collision = {}	
	collision["tileset"] = layerData["Tiles_1"]["layer_data"]["background"]
	collision["data"] = [ elem["id"] for line in layerData["Tiles_1"]["layer_data"]["tile_data"] for elem in line]

	if collision["tileset"] == "tile_house_2":
		print("tile_house_2 aint properly converted!")
		return None
		
	return collision, details

	
# obj_fakewall are the fake boulders btw
# I REALLY SHOULD OF JUST SKIPPED EVERYTHING WITH NPC IN ITS NAME OMFG
skipObjects = set(["obj_fakewall", "obj_na_secret_exit", "obj_solarsystem", "obj_secret_exit", "obj_mural", "obj_collision", "obj_npc_failure_006", "obj_na_secret_exit", "obj_rest", "obj_dustemiter_unknown", "obj_dustemit", "obj_npc_friend", "obj_mural", "obj_secret_exit", "obj_enemy_nm", "obj_npc_mon", "obj_npc_ykko", "obj_npc_failure_001", "obj_npc_nun", "obj_tail_kill", "obj_npc_failure_002", "obj_npc_bride", "obj_demonlords_statue", "obj_npc_failure_003", "obj_npc_famished", "obj_npc_unknown", "obj_npc_spawner", "obj_npc_gor", "obj_baal_m_d", "obj_npc_failure_008", "obj_npc_oldman", "obj_enemy_invisiblemon", "obj_npc_nomad", "obj_npc_nomad2", "obj_npc_failure_005", "obj_npc_failure_007", "obj_enemy_nm_b", "obj_npc_tail_floor", "obj_npc_failure_004", "obj_npc_randobandit", "obj_tot_mural", "obj_npc_tail_lowerfloor", "obj_npc_tail", "obj_npc_tail_tail"])
	
def convertFloor(layerData):
	
	floorExport = [ ["Pit" for i in range(9)] for j in range(14)]
	
	floorData = layerData["Floor"]["layer_data"]["instances"]
	pitData = layerData["Pit"]["layer_data"]["instances"]
	
	exitData = layerData["Floor_INS"]["layer_data"]["instances"]

	floorPassObjects = ["obj_chest_small", "obj_boulder", "obj_enter_the_secret"]
	
	for floorInstance in floorData + pitData + exitData:
		
		if floorInstance["object_definition"] in skipObjects:
			continue
			
		if "riddle" in floorInstance["object_definition"] or floorInstance["object_definition"] in floorPassObjects:
			layerData["Instances"]["layer_data"]["instances"].append(floorInstance)
			continue
		
		x, y = (floorInstance["x"] - 8) // 16, (floorInstance["y"] - 8) // 16
	
		if floorInstance["object_definition"] not in floorMap:
			print("we dont have a definition for {:s}".format(floorInstance["object_definition"]))
			
			
			if floorInstance["object_definition"] not in failures:
				failures[floorInstance["object_definition"]] = []
			failures[floorInstance["object_definition"]].append(layerData["roomName"])
			
			return None
		
		if floorInstance["creation_code"] == None:
		
		
			if floorInstance["object_definition"] == "obj_chest_small":
				# if the instance is a chest and is in here for some reason, we 
				# want to move that over to being a sprite
				
				layerData["Instances"]["layer_data"]["instances"].append(floorInstance)
				continue
					
			floorExport[x][y] = floorMap[floorInstance["object_definition"]]
		
		else:
		
			res = readCreationCode(floorInstance, floorInstance["creation_code"])
			
			res = res[isHardMode]
			
			if res == "destroy":
				continue
		
			# now that we know this obj aint getting destroyed, do this test
			if floorInstance["object_definition"] == "obj_chest_small" or floorInstance["object_definition"] == "obj_boulder":
				# if the instance is a chest and is in here for some reason, we 
				# want to move that over to being a sprite
				
				layerData["Instances"]["layer_data"]["instances"].append(floorInstance)
				continue
				
			if res is not None and "b_form" in res:
				print(RED + "there is somehow a form setter in a floor tile. this is undefined." + RESET)
				res = None
			
			if res is None:
				floorExport[x][y] = floorMap[floorInstance["object_definition"]]
			else:
				#print(x,y)
				floorExport[x][y] = floorMap[res]
	
	return floorExport
	
def convertEntities(layerData):

	tempInstanceData = layerData["Instances"]["layer_data"]["instances"]
	
	instanceExport = []
	
	global playerX
	global playerY
	
	for inst in tempInstanceData:
		if inst["object_definition"] == "obj_spawnpoint" or inst["object_definition"] == "obj_enter_the_secret":
		
				if inst["creation_code"] != None:
					print(RED + "a player had a creation code? oh gods" + RESET)
					#exit(1)
				
				x, y = (inst["x"] - 8) // 16, (inst["y"] - 8) // 16
				instanceExport.insert(0, instanceMap[inst["object_definition"]](inst, x, y))
				playerX, playerY = x, y
				break
	else:
		print("we couldnt find a player spawnpoint")
		return None
			
	# from what i can tell, this just overlays already existing collision?
	
	for inst in tempInstanceData:
	
		if inst["object_definition"] == "obj_spawnpoint" or inst["object_definition"] == "obj_enter_the_secret":
			continue
	
		if inst["object_definition"] in skipObjects:
			continue
	
		if inst["object_definition"] not in instanceMap:
			print("we dont have a definition for {:s}".format(inst["object_definition"]))
			
			if inst["object_definition"] not in failures:
				failures[inst["object_definition"]] = []
			
			failures[inst["object_definition"]].append(layerData["roomName"])
			
			return None
			
		x, y = (inst["x"] - 8) // 16, (inst["y"] - 8) // 16
		
		if x < 0 or y < 0 or x >= 14 or y >= 9:
			continue
		
		if inst["creation_code"] == None:
			instanceExport.append(instanceMap[inst["object_definition"]](inst, x, y))
		else:
		
			res = readCreationCode(inst, inst["creation_code"])
		
			res = res[isHardMode]
			
			if res == "destroy":
				continue
				
			if res is not None and "b_form" in res and inst["object_definition"] != "obj_boulder":
				print(RED + "why the fuck is a non boulder object have a bform???" + RESET)
	
			
			if res is None:
				instanceExport.append(instanceMap[inst["object_definition"]](inst, x, y))
			else:				
				instanceExport.append(instanceMap[res](inst, x, y))
		
			
			
	
	#instanceExport.append("NULL")

	return instanceExport
	
def convertRoom(data, outputFile):

	layerData = {}
	
	for l in data["layers"]:
		layerData[l["layer_name"]] = l
	
	if "Tiles_2" not in layerData:
		return None
	
	if "Tiles_1" not in layerData:
		return None
	
	layerData["roomName"] = data["name"]
	
	formatArray = lambda arr : "".join([ "{:d},".format(elem) for elem in np.array(arr).flatten() ])
	
	formatFullArray = lambda name, arr : "constexpr static inline u8 {:s}[] = {{{:s}}};".format(name, formatArray(arr))
	
	temp = convertCollisionAndDetails(layerData)
	
	if temp is None:
		return None
		
	collision, details = temp
		
	floorExport = convertFloor(layerData)
	
	if floorExport is None:
		return None
	
	instanceExport = convertEntities(layerData)
	
	if instanceExport is None:
		return None
	
	
	output = []
	
	output.append("namespace {:s} {{".format(data["name"]))
	
	if len(details["data"]) == 0:
		details["data"] = [ [0 for i in range(9)] for j in range(14)]
	
	output.append(formatFullArray("collision", collision["data"]))
	output.append(formatFullArray("details", details["data"]))

	output.append("constexpr static inline TileType floor[] = {")
	output.append("".join([ "TileType::{:s},".format(instance) for instance in np.transpose(floorExport).flatten() ]))
	output.append("};")
	
	output.append("constexpr static inline EntityHolder entities[] = {")	
	output.append("".join([ "{{{:s}}},".format(instance) for instance in instanceExport ]))
	output.append("};")
	
	
	output.append("constexpr static inline int entityCount = {:d};".format(len(instanceExport)))
	
	output.append("};")
	
	[ outputFile.write(line + "\n") for line in output ]

	# TODO, WE DONT EXPORT THE TILESET HERE.
	
	return True

def convertAllRooms(inputPath):

	global creationCodeData
	with open("creationCodeData.json") as f:
		creationCodeData = json.load(f)
	

	f = open("AllRooms.h", "w")
	
	f.write("//Did you know every time you sigh, a little bit of happiness escapes?\n")
	
	
	jsonFiles = [f for f in os.listdir(inputPath) if f.lower().endswith('.json')]
	
	
	#removeRooms = ["rm_misc_0002"]
	
	#jsonFiles = [ f for f in jsonFiles if f.rsplit(".", 1)[0] not in removeRooms ]

	#removeStrings = ["secret", "test", "misc", "trailer"]
	removeStrings = ["stg", "house", "secret", "test", "misc", "trailer", "dream", "memories", "bee", "lev", "_ee_"]
	
	for removeStr in removeStrings:
		# i could, and should one line this
		jsonFiles = [ f for f in jsonFiles if removeStr not in f ]
	
	#jsonFiles = ["rm_0005.json"]
	#jsonFiles = ["rm_0027.json"]
	#jsonFiles = ["rm_0008.json"]
	#jsonFiles = ["rm_0018.json"]
	
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
				print("failure")
			
	
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
	
	temp = sorted([ [k, len(v), v] for k, v in failures.items() ], key = lambda elem : len(elem[2]), reverse=True)
	
	highlightStrings = ["mon", "_e_"]
	
	for t in temp:
		temp = str(t)
		
		col = WHITE
		
		for s in highlightStrings:
			if s in temp:
				col = RED
				break
				
		print(col + temp + RESET)
		
		
		
	
	"""
	with open('tempCreationCodeData.json', 'w') as f:
		json.dump(creationCodeData, f, indent=4)
	"""
	
	
	with open('tempCreationCodeData.json', 'w') as f:
		json.dump(newCreationCodesData, f, indent=4)
	
	
	pass

if __name__ == "__main__":

	# run ExportAllRooms.csx(in this dir), name it roomExport, put it in this dir
	# run ExportAllCode, move to this folder
	
	convertAllRooms("./roomExport/")
	
	pass
	

