

from PIL import Image, ImageOps
import os 
import numpy as np
import json 
import string 
import shutil

palette = {
	(0, 255, 255)	: 0,
	(0, 0, 0)		: 1,
	(255, 255, 255)	: 2,
	(192, 192, 192)	: 3,
	(128, 128, 128)	: 4,
	
	# just in here as debug
	(255, 0, 255)	: 0,
}

validChars = '_.%s%s' % (string.ascii_lowercase, string.digits)

def createFolder(folderPath):
	if not os.path.exists(folderPath):
		os.mkdir(folderPath)

def writeBitmap(inputImageDontTouchLol, dest):

	inputImage = np.array(inputImageDontTouchLol)
	
	max_width, max_height = inputImageDontTouchLol.size
	
	# each row in the pixel array is padded to a multiple of 4
	padding = 4 - ((max_width / 2) % 4)
	pixelBytesPerRow = max_width/2
	
	if padding == 4:
		padding = 0
	
	#print(max_width, padding, pixelBytesPerRow)

	if (pixelBytesPerRow + padding) % 1 != 0:
		print("something is horribly wrong")
		exit(1)
		
	bytesPerRow = int(pixelBytesPerRow + padding)
	
	fileLength = 118 + (bytesPerRow * max_height)
	
	image = np.array([0] * fileLength, dtype=np.uint8)
	
	# BM header
	image[:2] = [0x42, 0x4D]
	
	def write4Num(arr, num, index):
		
		# gods there is def a better way todo this, but im fucking tired 
		# this,, was the better way, i changed item
		
		byte_0 = (num >> 24) & 0xFF
		byte_1 = (num >> 16) & 0xFF
		byte_2 = (num >> 8) & 0xFF
		byte_3 = num & 0xFF
		
		#arr[index: index+4] = [byte_0, byte_1, byte_2, byte_3]
		arr[index: index+4] = [byte_3, byte_2, byte_1, byte_0]

	def write2Num(arr, num, index):
	
		byte_2 = (num >> 8) & 0xFF
		byte_3 = num & 0xFF
		
		arr[index: index+2] = [byte_3, byte_2]

	# write fileLength
	
	write4Num(image, 150, 2)

	
	# starting addr, hopefully constant?
	image[0x0A] = 0x76
	
	# size of header 
	image[0x0E] = 40
	
	w, h = inputImageDontTouchLol.size 
	
	# width 
	write4Num(image, w, 0x12)
	
	# height 
	write4Num(image, h, 0x16)
	
	# color planes?
	image[0x1A] = 1
	
	# BITS PER FUCKING PIXEL
	image[0x1C] = 4
	
	write2Num(image, 16, 0x2E)
	
	# write palette
	write4Num(image, 0x00FFFF, 54)
	write4Num(image, 0x000000, 58)
	write4Num(image, 0xFFFFFF, 62)
	write4Num(image, 0xC0C0C0, 66)
	write4Num(image, 0x808080, 70)
	
	# THIS SHOULD PROBS BE VECTORIZED, FOOL.
	currentIndex = 118
	for row in reversed(range(0, max_height)):
		
		for xPos in range(0, max_width):
			
			col = tuple(inputImage[row][xPos])
			
			paletteIndex = 0
			
			if col not in palette:
				#print("something is very very wrong")
				#print(str(inputImage[row][xPos]) + " wasnt found in the palette")
				#print("currently working on " + folder)
				#exit(1)
				
				# approximate
				
				bestCol = (-1, -1, -1)
				bestColDistance = 1000
				bestColIndex = -1
				
				# i could do this check with some less thans, but ill just cache the result and it willbe fine
				# i am however assuming that the only colors coming through here will be grey.
				for testCol, index in palette.items():
					newDist = abs(int(col[0]) - int(testCol[0]))
					if newDist < bestColDistance:
						bestColDistance = newDist
						bestCol = testCol
						bestColIndex = index
						
				if bestColIndex == -1:
					print("this shouldnt happen. pray")
				
				
				palette[col] = bestColIndex
										
				paletteIndex = bestColIndex
				
				print("indexed {:s} as {:d} for {:s}".format(str(col), bestColIndex, os.path.basename(dest)))
					
				
			else:
				paletteIndex = palette[col]
			
			if currentIndex % 1 == 0:
				paletteIndex = paletteIndex << 4
			
			image[int(currentIndex)] |= paletteIndex
			currentIndex += 0.5
	
		currentIndex += padding
	
	"""
	for a in [image[i:i+16] for i in range(0, len(image), 16)]:
		
		temp = str([ "{:02X}".format(i) for i in a ])
		temp = temp[1:-1].replace("'", "")
	
		print(temp)
	"""


	# file_info.py in butano has some whack requirements.
	# they only check if the first char is lowercase?? 
	# and i couldjust lower everything, but thats a risk.
	# ill just append something that says datawin on it, i hope that 
	# that doesnt get annoying
	
	# nvm, i read their code wrong, im a fool.
	# im just going to pray that my bullshittery here doesnt like
	# if any files get overwritten bc of case differences, ima be so mad.

	newDest = list(os.path.split(dest))
	#newDest[1] = newDest[1].lower()
	newDest[1] = "dw_" + newDest[1].lower()
	newDest = os.path.join(*newDest)
	
	
	
			
	with open(newDest, "wb") as f:
		f.write(image.tobytes())

def preformatSprites():
	
	print("preformatting sprites to fit the goofy ahh format from when i first got them")

	currentFolder = "./Export_Textures_Padded/"
	outputFolder = "./Sprites_Padded/"
	
	createFolder("./Sprites_Padded/")
	
	
	png_files = [f for f in os.listdir(currentFolder) if f.lower().endswith('.png')]
	
	data = {}
	
	for filename in png_files:
	
		foldername = filename.rsplit("_", 1)[0]
		
		if foldername not in data:
			data[foldername] = []
			
		data[foldername].append(filename)
	
	shutil.rmtree(outputFolder)
	
	for folder, filenames in data.items():
		
		
		destFolder = os.path.join(outputFolder, folder)
		os.makedirs(destFolder)
		
		for file in filenames:
			shutil.copyfile(os.path.join(currentFolder, file), os.path.join(destFolder, file))
		
		
		
	

	print("done preformatting")

spriteSuccess = 0
spriteTotal = 0
	
def convertSprites(spritePath, outputPath):
	
	global spriteSuccess
	global spriteTotal
	
	"""
	Invalid sprite width: 224 (valid sprite sizes: 8x8, 16x16, 32x32, 64x64, 16x8, 32x8, 32x16, 8x16, 8x32, 16x32, 32x64)
	dw_spr_white_screen.bmp error: Invalid sprite width: 224 (valid sprite sizes: 8x8, 16x16, 32x32, 64x64, 16x8, 32x8, 32x16, 8x16, 8x32, 16x32, 32x64)
	dw_spr_windowframe.bmp error: Invalid sprite width: 224 (valid sprite sizes: 8x8, 16x16, 32x32, 64x64, 16x8, 32x8, 32x16, 8x16, 8x32, 16x32, 32x64)
	dw_spr_window_001.bmp error: Invalid width: 58
	dw_spr_window_002.bmp error: Invalid width: 50
	dw_spr_window_003.bmp error: Invalid width: 66
	dw_spr_window_004.bmp error: Invalid width: 82
	dw_spr_window_005.bmp error: Invalid width: 42
	dw_spr_window_error.bmp error: Invalid width: 66
	dw_spr_window_errorbg.bmp error: Invalid width: 60
	dw_spr_wings_001.bmp error: Invalid width: 50
	dw_spr_wings_0012134.bmp error: Invalid width: 50
	dw_spr_wings_001b.bmp error: Invalid width: 50

	why the inconsistencies?

	dw_spr_zerostate_glow.bmp error: File height is not divisible by item height: 120 - 16
	
	ugh screw it, ill just skip sprites that would error
	i dont need most of them anyway, at least in this phase
	
	
	"""
	
	validSizes = set([
	(8, 8), 
	(16, 16), 
	(32, 32), 
	(64, 64), 
	(16, 8), 
	(32, 8), 
	(32, 16), 
	(8, 16), 
	(8, 32), 
	(16, 32), 
	(32, 64),
	])
	
	
	#spritePath = "./Sprites/."
	
	
	contents = os.listdir(spritePath)
	folders = [item for item in contents if os.path.isdir(os.path.join(spritePath, item))]
	
	totalFolders = len(folders)
	
	spriteTotal = totalFolders

	for i, folder in enumerate(folders):
	
		print("working on {:50s} {:5d}/{:5d}   {:6.2f}% done".format(folder, i, totalFolders, 100*i/totalFolders))
	
		currentFolder = os.path.join(spritePath, folder)
	
		testIfFile = os.path.join(outputPath, "dw_" + folder.lower() + ".json")
		
	
		if os.path.isfile(testIfFile):
			print("skipping {:s}, already done padded".format(testIfFile))
			continue
	
		png_files = [f for f in os.listdir(currentFolder) if f.lower().endswith('.png')]
		
		# THIS MAY BE A BIG MISTAKE
		png_files = sorted(png_files)
		
		max_width = 0
		total_height = 0
		max_height = 0 
		
		# Determine the maximum width and calculate the total height
		for png_file in png_files:
			image = Image.open(os.path.join(currentFolder, png_file))
			width, height = image.size
			max_width = max(max_width, width)
			total_height += height
			max_height = max(max_height, height)
		
		if (max_width, max_height) not in validSizes:
			print("skipping, lets just hope that it wasnt needed <3")
			continue
		
		total_height = max_height * len(png_files)
		
		# Create a new blank image to stack PNGs vertically
		stacked_image = Image.new('RGBA', (max_width, total_height))
		
		# Paste each PNG file onto the stacked image
		y_offset = 0
		for png_file in png_files:
			image = Image.open(os.path.join(currentFolder, png_file))
			width, height = image.size
			image = image.convert("RGBA")
			stacked_image.paste(image, (0, y_offset))
			y_offset += max_height

		cyan_background = Image.new("RGBA", (max_width, total_height), (0, 255, 255, 255))
		cyan_background.paste(stacked_image, (0, 0), stacked_image)
		
		cyan_background = cyan_background.convert("RGB")

		tempImage = cyan_background
		
		writeBitmap(tempImage, os.path.join(outputPath, folder + ".bmp"))
		
		outputJson = {
			"type": "sprite_tiles",
			"height": max_height
		}
	
		spriteSuccess += 1
	
		with open(os.path.join(outputPath, "dw_" + folder.lower() + ".json"), "w") as f:
			json.dump(outputJson, f)
		
		pass
		
	pass

def convertAllSprite(outputPath):
	
	preformatSprites()
	
	# remove all sprites in the folder bc later on we are going to be passing on dupes.
	shutil.rmtree(outputPath)

	os.makedirs(outputPath)
	
	convertSprites("./Sprites_Padded/.", outputPath)
	
	convertSprites("./Sprites/.", outputPath)

		
	global spriteSuccess
	global spriteTotal
	
	print("we converted {:6.2f}% sprites({:d}/{:d}), i hope thats acceptable.".format(100*spriteSuccess/spriteTotal, spriteSuccess, spriteTotal))
	
	pass
	
def convertTiles(outputPath):


	inputPath = "./Backgrounds/."
	
	
	contents = os.listdir(inputPath)
	folders = [item for item in contents if os.path.isdir(os.path.join(inputPath, item))]
	
	totalFolders = len(folders)
	
	#folders = [folders[1]]
	#folders = [folders[0]]
	
	for i, folder in enumerate(folders):
		
		print("working on {:30s} {:5d}/{:5d}   {:6.2f}% done".format(folder, i, totalFolders, 100*i/totalFolders))
	
		if folder == "tile_house_2":
			# thins thing,,, is just going to need manual processing 
			# dw_tile_house_2.bmp error: Regular BG tiles items with more than 1024 tiles not supported: 1089
			# ugh 
			continue
	
		currentFolder = os.path.join(inputPath, folder)
	
		png_files = [f for f in os.listdir(currentFolder) if f.lower().endswith('.png')]

		if len(png_files) != 1:
			print("why in tarnation are there {:d} png files in {:s}???".format(len(png_files), folder))
			exit(1)
			
		pngFile = png_files[0]
		
		image = Image.open(os.path.join(currentFolder, pngFile))
	
		width, height = image.size
		
		cyan_background = Image.new("RGBA", (width, height), (0, 255, 255, 255))
		cyan_background.paste(image, (0, 0), image)
	
		cyan_background = cyan_background.convert("RGB")

		tileSize = 20
		
		if folder == "tile_house_2":
			# for reasons unknown to the gods, this thing has a 8 bit tile
			# why?????????
			# MAKE SURE THIS IS IN THE JSON 
			# ah, that only matters in sprites, good!
			tileSize = 8
			# no resizing will occur here.
		else:	
			# resize the image
			width, height = (16 * width // 20, 16 * height // 20)
			# is nearest the best option here?
			# are there any tiles which this corrupts?
			width, height = image.size
			# it seems like a 2x2 border is cropped out of each image? kinda feel 
			# dumb for not noticing that before
			# THAT EXPLAINS THE 2PX BORDER AROUND TILE 0!
			
			#cyan_background = cyan_background.resize((width, height), Image.Resampling.NEAREST)
			pass
		
		
		#cyan_background.save("test.png")
		
		input_array = np.array(cyan_background)
		
		
		sections = []
		
		for j in range(0, height, tileSize):
			for i in range(0, width, tileSize):
				section = input_array[j:j+tileSize, i:i+tileSize]
				
				# now we actually crop the sections to 16x16
				
				section = section[2:18, 2:18]
				#print(section.shape)
				sections.append(section)
		
		tileSize = 16
		
		# make first icon transparent
		sections[0][:] = (0, 255, 255)
		
		#print([[section.size] for section in sections])	
		#print(image.size)
		output_array = np.vstack(sections)

		#print(output_array)
		
		# Create a new PIL image from the NumPy array
		output_image = Image.fromarray(output_array)

		# Save the resulting 16x64 image (replace 'output.png' with your desired output file)
		#output_image.save("output.png")
		
		writeBitmap(output_image, os.path.join(outputPath, folder + ".bmp"))
	
		outputJson = {
		    "type": "regular_bg_tiles",
			"bpp_mode": "bpp_4"
		}
		
		
	
		with open(os.path.join(outputPath, "dw_" + folder.lower() + ".json"), "w") as f:
			json.dump(outputJson, f)
	
		# gods what is the difference between a sprite tiles and a sprite??
		

	pass

def convertFonts(outputPath):
	pass

def generateCustomFloorBackground(outputPath):

	print("generating custom floor tiles")

	# currently, i know of:
	# FloorTile, Glass, Bomb, Death, Shadow, Exit, Switch
	
	
	floorTileSprites = [
	"dw_spr_floor_editor",
	"dw_spr_glassfloor",
	"dw_spr_explofloor",
	"dw_spr_deathfloor",
	"dw_spr_copyfloor",
	"dw_spr_stairs",
	"dw_spr_floorswitch",
	]
	
	tiles = []
	
	tiles.append(np.zeros((16, 16), dtype=np.uint8))

	
	for i, spriteName in enumerate(floorTileSprites):
		
		testPath = os.path.join("./formattedOutput/sprites/", spriteName + ".bmp")
		
		if not os.path.isfile(testPath):
			print("unable to find {:s}, bruh".format(spriteName))
			exit(1)
		
		temp = np.array(Image.open(testPath))
		
		if i != 0:	
			tiles.append(temp)
			continue 
			
		# if here, i=0, we need to fix this tile.
		
		floor = temp[0:16, 0:16]
		detail = temp[16:, 0:16]
		
		# why do i always end up thinking that numpy arrays are immutable??????
		
		for i in range(1, 16):
			for j in range(1, 16):
				floor[i][j] = 2
				
		for i in range(2, 15):
			if floor[i][0] == 4:
				floor[i][0] = 2
				
			if floor[0][i] == 4:
				floor[0][i] = 2
		
		tiles.append(floor)
		
		# for good measure ima make the floor detailer black transparent
		
		for i in range(0, 16):
			for j in range(0, 16):
				if detail[i][j] == 1:
					detail[i][j] = 0
			
		tiles.append(detail)
		
		
	for t in tiles:
		print(t.shape)
		
	
	stackedTiles = np.vstack(tiles).tolist()
	
	reversePalette = {
	0 : (0, 255, 255)	,
	1 : (0, 0, 0)		,
	2 : (255, 255, 255)	,
	3 : (192, 192, 192)	,
	4 : (128, 128, 128)	,
	}
	
	for i in range(0, len(stackedTiles)):
		for j in range(0, len(stackedTiles[0])):
			stackedTiles[i][j] = reversePalette[stackedTiles[i][j]]
	

	temp = Image.fromarray(np.uint8(stackedTiles))
	
	writeBitmap(temp, os.path.join(outputPath, "customfloortiles.bmp"))
	
	# this right here is horrid, basically, when loading the img i think it loads it in 
	# still as a 4bit bmp
	# i dont want to have to like,,, go through the pain of editing the bitmap writer, 
	# so ima just convert it to a normal image array, and then ball.
		
		
	outputJson = {
		"type": "regular_bg_tiles",
		"bpp_mode": "bpp_4"
	}
		
		

	with open(os.path.join(outputPath, "dw_" + "customfloortiles" + ".json"), "w") as f:
		json.dump(outputJson, f)
	
	print("done generating custom floor tiles")
	
	pass
	
def generateIncludes(folders):

	print("generating include file")
	
	output = []
	
	for folder in folders:
		
		jsonFiles = [f for f in os.listdir(folder) if f.lower().endswith('.json')]
		
		for file in jsonFiles:
			#print(jsonFiles)
			# i dont rlly need to open each json here, but im being safe.
			
			with open(os.path.join(folder, file)) as f:
				idk = json.load(f)
			
				line = "#include \"bn_{:s}_items_{:s}.h\"".format(idk["type"], os.path.basename(file).split(".")[0])
			
				output.append(line)
	

	f = open("dataWinIncludes.h", "w")
	f.write("#pragma once\n")
	f.write("""
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_memory.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_regular_bg_map_cell_info.h"

#include "bn_sprite_ptr.h"
#include "bn_sprite_tiles_item.h"
#include "bn_sprite_tiles_ptr.h"
#include "bn_sprite_shape_size.h"
#include "bn_sprite_palette_item.h"
#include "bn_bg_palette_item.h"
#include "bn_sprite_palette_ptr.h"

#include "common_fixed_8x8_sprite_font.h"
#include "bn_timer.h"


#include "bn_optional.h"

#include <cstring> 
#include "bn_string_view.h"
#include "bn_sstream.h"
#include "bn_istring_base.h"
#include "bn_string.h"
#include "bn_vblank_callback_type.h"
#include "bn_utility.h"
#include "bn_unordered_set.h"

#include "bn_color.h" 
#include "bn_bg_palette_item.h"
#include "bn_sprite_palette_item.h"


""")
	for line in output:
		f.write(line + "\n")
	f.close()
			
	print("done generating include file")
	
	pass
	
if __name__ == "__main__":

	# with undertalemodtool:
	
	# run ExportAllSprites(script in this folder) , rename to Export_Textures_Padded, move to folder
	# run ExportAllTexturesGrouped, copy all 3 folders inside to this folder.
	
	createFolder("./formattedOutput/")
	createFolder("./formattedOutput/sprites/")
	createFolder("./formattedOutput/fonts/")
	createFolder("./formattedOutput/tiles/")
	createFolder("./formattedOutput/customFloor/")
	
	#convertSprites("./formattedOutput/sprites/")
	
	convertAllSprite("./formattedOutput/sprites/")
	
	convertTiles("./formattedOutput/tiles/")
	
	#convertFonts("./formattedOutput/fonts/")
	
	generateCustomFloorBackground("./formattedOutput/customFloor/")
	
	generateIncludes(["./formattedOutput/sprites/", "./formattedOutput/tiles/", "./formattedOutput/fonts/", "./formattedOutput/customFloor/"])
	
	
	
	pass
	
