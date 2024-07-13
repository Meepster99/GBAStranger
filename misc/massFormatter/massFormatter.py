

from PIL import Image, ImageOps, ImageDraw, ImageFilter
import os, sys
import numpy as np
import json
import string
import shutil
import re
import time
import hashlib
#from ..EasyPoolProcessing/poolQueue import PoolQueue

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
YELLOW = Fore.YELLOW
CYAN = Fore.CYAN
WHITE = Fore.WHITE
MAGENTA = Fore.MAGENTA

RESET = Style.RESET_ALL

palette = {
	(0, 255, 255)	: 0,
	(0, 0, 0)		: 1,
	(255, 255, 255)	: 2,
	(192, 192, 192)	: 3,
	(128, 128, 128)	: 4,

	# just in here as debug
	(255, 0, 255)	: 0,

	# manual sanity fix
	(20, 255, 255)	: 2,

	# why was the cyan in,, the thing not accurate?
	(0, 246, 255)	: 0,
}

validChars = '_.%s%s' % (string.ascii_lowercase, string.digits)

# making this global may be a bad idea, but it will allow me to cache writes to
# datawinwhatever
# at the cost of threading
# nvm
# output = []

def getNeededSprites(thisFile = False):

	codeFolder = "../../code/src/"

	codeFiles = [os.path.join(codeFolder, f) for f in os.listdir(codeFolder) if f.lower().endswith('.h') or f.lower().endswith('.cpp')]

	if "../../code/src/dataWinIncludes.h" in codeFiles:
		codeFiles.remove("../../code/src/dataWinIncludes.h")

	# this will also include our own file, for special things needed for customfloortiles
	#codeFiles.append(__file__)

	if thisFile:
		codeFiles = [__file__]

	sprite_matching_references = set()
	bg_matching_references = set()
	font_matching_references = set()

	if thisFile:
		pattern = r'dw_\w+'

		for file in codeFiles:
			with open(file, 'r', encoding='utf-8', errors='ignore') as f:
				content = f.read()
				matches = re.findall(pattern, content)
				sprite_matching_references.update(matches)

	else:
		# gods this is trash
		pattern1 = r'regular_bg_tiles_items::dw_\w+'
		pattern2 = r'sprite_tiles_items::dw_\w+'
		pattern3 = r'sprite_items::dw_\w+'
		pattern4 = r'regular_bg_items::dw_\w+'

		for file in codeFiles:
			with open(file, 'r', encoding='utf-8', errors='ignore') as f:
				content = f.read()
				matches = [ s[len("regular_bg_tiles_items::"):] for s in re.findall(pattern1, content) ]
				bg_matching_references.update(matches)

				matches = [ s[len("regular_bg_items::"):] for s in re.findall(pattern4, content) ]
				bg_matching_references.update(matches)

				matches = [ s[len("sprite_tiles_items::"):] for s in re.findall(pattern2, content) ]
				sprite_matching_references.update(matches)

				matches = [ s[len("sprite_items::"):] for s in re.findall(pattern3, content) ]
				font_matching_references.update(matches)





	if thisFile:
		# remove font stuff, and tile stuff(i think)
		# i could do this via a filter, but this is safer
		sprite_matching_references.remove("dw_fnt_text_12")

		return sprite_matching_references


	# this string matches things with index\d in them
	pattern = r'dw_\w+[^_]_index\d'
	cutsceneBGReferences = set()
	for s in bg_matching_references:
		matches = re.findall(pattern, s)
		cutsceneBGReferences.update(matches)

	#matching_references_noheaders = set([ s[3:] for s in list(matching_references) ])

	# gods this should of just been a dict wtf is wrong with me
	return [sprite_matching_references, bg_matching_references, font_matching_references, cutsceneBGReferences]

# multiprocessing might run this every time, but tbh, im tired, how about that!
# not actually doing this, since we will have all sprites preconverted, but only copy them over to the code area if needed
#neededSprites, neededSpritesNoHeaders = getNeededSprites()

def createFolder(folderPath):
	if not os.path.exists(folderPath):
		os.mkdir(folderPath)

def writeBitmap(inputImageDontTouchLol, dest):

	inputImage = np.array(inputImageDontTouchLol.convert("RGB"))

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

		# gods there is def a better way todo this, but im (curse)ing tired
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

	# BITS PER (curse)ING PIXEL
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
	# im just going to pray that my bull(curse)tery here doesnt like
	# if any files get overwritten bc of case differences, ima be so mad.

	newDest = list(os.path.split(dest))
	#newDest[1] = newDest[1].lower()
	newDest[1] = "dw_" + newDest[1].lower()
	newDest = os.path.join(*newDest)




	with open(newDest, "wb") as f:
		f.write(image.tobytes())


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
(64, 32),
])
"""

# some statues are 64 by 64, and we want those as bigsprites, not as sprite sprites
# but some things (like spr_playerswipe) are 32 by 16, and would be quite nice to have in just one sprite, so this
# will no longer be just 16 by 16 sprites
validSizes = set([
#(8, 8),
#(8, 16),
#(16, 8),
(16, 16),
(32, 16),
(16, 32),
(32, 32),
(64, 32),
(32, 64),
(64, 64)
])

def copyIfChanged(inputFile, outputPath):

	if not os.path.isfile(os.path.join(outputPath, os.path.basename(inputFile))):
		shutil.copy(inputFile, outputPath)
		return True
	else:
		thisFileHash = hashlib.md5(open(inputFile, 'rb').read()).hexdigest()
		otherFileHash = hashlib.md5(open(os.path.join(outputPath, os.path.basename(inputFile)), 'rb').read()).hexdigest()

		if thisFileHash != otherFileHash:
			shutil.copy(inputFile, outputPath)
			return True

	return False

def convertTiles(outputPath):

	print("converting tiles")

	inputPath = "../ExportData/Export_Textures/Backgrounds/."


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

		cyan_background = Image.new("RGBA", (width, height), (0, 0, 0, 255))
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

		count = 0

		for j in range(0, height, tileSize):
			for i in range(0, width, tileSize):
				section = input_array[j:j+tileSize, i:i+tileSize]

				# now we actually crop the sections to 16x16

				section = section[2:18, 2:18]
				#print(section.shape)
				sections.append(section)

				count += 1

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

	# this thing is bc sometimes like,,, i just have random vram being used by other (curse)(that was inited as a default) and im never sure if its properly freed
	# this is just a small bg tile, for uses of initing things

	temp = Image.new("RGBA", (16, 16), (0, 255, 255, 255))
	writeBitmap(temp, os.path.join(outputPath, "default_bg_tiles" + ".bmp"))
	outputJson = {
		"type": "regular_bg_tiles",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_bg_tiles" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGBA", (256, 256), (0, 255, 255, 255))
	writeBitmap(temp, os.path.join(outputPath, "default_bg" + ".bmp"))
	outputJson = {
		"type": "regular_bg",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_bg" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGB", (256, 256), (0, 0, 0))
	writeBitmap(temp, os.path.join(outputPath, "default_black_bg" + ".bmp"))
	outputJson = {
		"type": "regular_bg",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_black_bg" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGB", (256, 256), (255, 255, 255))
	writeBitmap(temp, os.path.join(outputPath, "default_white_bg" + ".bmp"))
	outputJson = {
		"type": "regular_bg",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_white_bg" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGB", (16, 16), (0, 0, 0))
	writeBitmap(temp, os.path.join(outputPath, "default_sprite_tiles" + ".bmp"))
	outputJson = {
		"type": "sprite_tiles",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_sprite_tiles" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGB", (64, 64), (0, 0, 0))
	writeBitmap(temp, os.path.join(outputPath, "default_sprite_64" + ".bmp"))
	outputJson = {
		"type": "sprite",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_sprite_64" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGB", (64, 32), (0, 0, 0))
	writeBitmap(temp, os.path.join(outputPath, "default_sprite_64_32" + ".bmp"))
	outputJson = {
		"type": "sprite",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_sprite_64_32" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGB", (32, 64), (0, 0, 0))
	writeBitmap(temp, os.path.join(outputPath, "default_sprite_32_64" + ".bmp"))
	outputJson = {
		"type": "sprite",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_sprite_32_64" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGB", (32, 32), (0, 0, 0))
	writeBitmap(temp, os.path.join(outputPath, "default_sprite_32_32" + ".bmp"))
	outputJson = {
		"type": "sprite",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_sprite_32_32" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGB", (16, 16), (0, 255, 255))
	writeBitmap(temp, os.path.join(outputPath, "default_sprite_tiles_transparent" + ".bmp"))
	outputJson = {
		"type": "sprite_tiles",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_sprite_tiles_transparent" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGB", (16, 16), (0, 255, 255))
	writeBitmap(temp, os.path.join(outputPath, "default_background_tiles_transparent" + ".bmp"))
	outputJson = {
		"type": "regular_bg_tiles",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + "default_background_tiles_transparent" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.open("../ExportData/Export_Textures_Padded/spr_glitchedsprites_0.png")
	writeBitmap(temp, os.path.join("./formattedOutput/allSprites/", "spr_glitchedsprites" + ".bmp"))
	outputJson = {
		"type": "regular_bg_tiles",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join("./formattedOutput/allSprites/", "dw_" + "spr_glitchedsprites" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.open("idrk.png")
	writeBitmap(temp, os.path.join("./formattedOutput/allSprites/", "idrk" + ".bmp"))
	outputJson = {
		"type": "sprite",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join("./formattedOutput/allSprites/", "dw_" + "idrk" + ".json"), "w") as f:
		json.dump(outputJson, f)

	#

	temp = Image.new("RGBA", (256, 256), (0, 255, 255, 255))
	actualTemp = Image.open("credits.png")
	temp.paste(actualTemp)
	writeBitmap(temp, os.path.join("./formattedOutput/allSprites/", "credits" + ".bmp"))
	outputJson = {
		"type": "regular_bg",
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join("./formattedOutput/allSprites/", "dw_" + "credits" + ".json"), "w") as f:
		json.dump(outputJson, f)

	print("done converting tiles")
	pass

def getFontCharData(font):

	inputPath = "../ExportData/Export_Fonts/."

	fontImageFileName = font
	fontCSVFileName = "glyphs_" + font.rsplit(".", 1)[0] + ".csv"

	fontImage = Image.open(os.path.join(inputPath, fontImageFileName))
	f = open(os.path.join(inputPath, fontCSVFileName))
	fontCSV = [ line.strip() for line in f.readlines() if len(line.strip()) != 0 ][1:]
	f.close()

	cyan_background = Image.new("RGBA", fontImage.size, (0, 255, 255, 255))
	cyan_background.paste(fontImage, (0, 0), fontImage)
	fontImage = cyan_background.convert("RGB")

	charData = {}

	img = ImageDraw.Draw(fontImage)

	for line in fontCSV:
		char, x, y, width, height, shift, offset = [ int(l) for l in line.split(";") ]

		if char < 32 or char >= 127:
			continue

		if width >= 16:
			return None

		# shift and offset are probs needed
		charData[char] = {
			"x": x,
			"y": y,
			"width": width,
			"height": height,
			"shift": shift,
			"offset": offset,
		}

	return fontImage, charData

def createFontTiles(fontImage, charData, font):
	# no clue why, but chars written are 1 off rn.
	# i have 0 clue wtf butano is on rn
	tiles = np.full(((128-32)*16, 16, 3), (0, 255, 255), dtype=np.uint8)

	for char, data in charData.items():
		if char == 32:
			continue

		x, y, width, height, shift, offset = data["x"], data["y"], data["width"], data["height"], data["shift"], data["offset"]

		tempY = (char-32-1)*16
		tempHeight = min(16, height)

		tempImage = np.array(fontImage)[y:y+tempHeight, x:x+width]

		tempImage[np.all(tempImage == [0, 0, 0], axis=-1)] = [0, 0, 255]

		# the colon for fnt_etext_12 needs to be moved to the RIGHT 3 pixels.
		if char == ord(':') and font == "fnt_etext_12.png":
			tiles[tempY:tempY + tempHeight, 3:4] = tempImage
			charData[char]["width"] = 3
			continue

		tiles[tempY:tempY + tempHeight, 0:width] = tempImage

	return Image.fromarray(tiles)

def writeFont(fontTiles, charData, outputPath, font, fontDataFile):

	writeBitmap(fontTiles, os.path.join(outputPath, font.rsplit(".", 1)[0] + ".bmp"))

	outputJson = {
		"type": "sprite",
		"bpp_mode": "bpp_4",
		"height": 16,
	}

	with open(os.path.join(outputPath, "dw_" + font.rsplit(".", 1)[0] + ".json"), "w") as jsonFileHandle:
		json.dump(outputJson, jsonFileHandle)

	writeBitmap(fontTiles, os.path.join(outputPath, font.rsplit(".", 1)[0] + "_bg.bmp"))
	outputJson = {
		"type": "regular_bg_tiles",
		"bpp_mode": "bpp_4",
	}
	with open(os.path.join(outputPath, "dw_" + font.rsplit(".", 1)[0] + "_bg.json"), "w") as jsonFileHandle:
		json.dump(outputJson, jsonFileHandle)

	name = font.rsplit(".", 1)[0]

	fontDataFile.write("constexpr int8_t dw_{:s}_sprite_font_character_widths[] = {{".format(name))
	fontDataFile.write(",".join([ str(charData[char]["width"]) for char in range(32, 126+1) ] + ["1"]))
	fontDataFile.write("};")

	fontDataFile.write("""\n
	constexpr bn::utf8_character dw_{:s}_sprite_font_utf8_characters[] = {{"Á"}}; // dont ask

	constexpr bn::span<const bn::utf8_character> dw_{:s}_sprite_font_utf8_characters_span(dw_{:s}_sprite_font_utf8_characters);

	constexpr auto dw_{:s}_sprite_font_utf8_characters_map =
	bn::utf8_characters_map<dw_{:s}_sprite_font_utf8_characters_span>();

	constexpr bn::sprite_font dw_{:s}_sprite_font(
	bn::sprite_items::dw_{:s},
		dw_{:s}_sprite_font_utf8_characters_map.reference(),
	dw_{:s}_sprite_font_character_widths, 1);

	""".format(name, name, name, name, name, name, name, name, name)) # im tired for this.

def convertFonts(outputPath):

	print("converting fonts")

	# https://github.com/krzys-h/UndertaleModTool/blob/7b876ad457eca5cdd69957dc02ef57a569412e5e/UndertaleModTool/Scripts/Resource%20Unpackers/ExportFontData.csx#L4

	inputPath = "../ExportData/Export_Fonts/."

	fontDataFile = open("fontData.h", "w", encoding='utf-8')
	for font in [item for item in os.listdir(inputPath) if item.lower().endswith('.png') ]:

		res = getFontCharData(font)

		if res is None:
			print(f"{YELLOW}im not dealing with {font} rn{RESET}")
			continue

		fontImage, charData = res

		fontTiles = createFontTiles(fontImage, charData, font)

		writeFont(fontTiles, charData, outputPath, font, fontDataFile)

		if font == "fnt_text_12.png":

			print(f"{CYAN}creating outlined fnt_text_12{RESET}")
			# handle the special case where we need to write dw_fnt_text_12 with an outline
			# using 5 sprites per letter works, but is 5 sprites per letter

			fontData = np.array(fontTiles)
			eachChar = [ fontData[y:y+16, 0:16] for y in range(0, fontData.shape[0], 16) ]

			for i in range(0, len(eachChar)):

				newChar = np.full((16, 16, 3), (0, 255, 255), np.uint8)
				newChar[0:15, 1:16] = eachChar[i][1:16, 0:15]

				# not ideal, im tired tho
				for x in range(1, 15):
					for y in range(1, 15):
						if (newChar[y][x] == (255, 255, 255)).all():
							for p in [(x-1, y), (x+1, y), (x, y-1), (x, y+1)]:
								if not (newChar[p[1], p[0]] == (255, 255, 255)).all():
									newChar[p[1], p[0]] = (0, 0, 0)
				eachChar[i] = newChar

			tempImage = Image.fromarray(np.vstack(eachChar))

			for char in charData.keys():
				charData[char]["width"] += 2

			writeFont(tempImage, charData, outputPath, "fnt_text_12_outline.png", fontDataFile)

	fontDataFile.close()

	copyIfChanged("fontData.h", "../../code/src/")

	print("done converting fonts")

	pass

def getSpriteDimensions(spriteName, padded):

	if getSpriteDimensions.paddedSpriteList is None:
		getSpriteDimensions.paddedSpriteList = [f for f in os.listdir("../ExportData/Export_Textures_Padded/") if f.lower().endswith('.png')]


	if "_index" in spriteName:
		spriteName = spriteName.rsplit("_index", 1)[0]


	paddedInputPath = "../ExportData/Export_Textures_Padded/"
	inputPath = "../ExportData/Export_Textures/Sprites/" + spriteName

	spritePath = paddedInputPath if padded else inputPath

	png_files = []

	search_prefix = spriteName

	pattern = re.compile(r'^' + re.escape(search_prefix) + r'_\d+\.png')

	png_files = [filename for filename in getSpriteDimensions.paddedSpriteList if pattern.search(filename)]

	if len(png_files) == 0:
		print("unable to find " + spriteName)
		exit(1)

	pngFileImages = []

	max_width = -1
	max_height = -1

	#print(png_files)

	for png_file in png_files:
		image = Image.open(os.path.join(spritePath, png_file))
		width, height = image.size
		max_width = max(max_width, width)
		max_height = max(max_height, height)
		pngFileImages.append(image)

	return [(max_width, max_height), pngFileImages]

getSpriteDimensions.paddedSpriteList = None

def convertSprite(spriteName, spriteImages, dimensions, isBackground, isNormalBackground):

	#print(dimensions)

	max_width, max_height = dimensions

	total_height = max_height * len(spriteImages)

	# if isNormalBackground, we need to compress thiese tiles
	# i could also(in the future) do flipping, but for now, just having unique ones will be enough
	# we also need to write the data to properly reconstruct it to datawin.h
	# and hopefully using a set wont make the output different each run

	outputPath = "./formattedOutput/allSprites/"

	if not isNormalBackground:
		# hmm
		# i could maybe,,, leave this to the butano thingy to take care of?
		# im assuming/hoping that if i use regular_bg
		# instead of regular bg tiles that maybe like,,,,,, stuff will happen?
		# but will i have to name every,, sub-background as its own?
		# i could also maybe like,, do it with scaling,
		# or actually, ill have it be the spritename_number
		# but then that will (curse) up my code which,, grabs which items are needed
		# meaning i need ANOTHER one off fix
		# it seems that butano has built in compression though.
		# i just hope this doesnt (curse) (curse) up with my prexisting classes
		# actually,,, it totally will

		spriteIndex = spriteName.rsplit("index", 1)

		#print(spriteName, len(spriteImages), spriteIndex)

		spriteIndex = int(spriteIndex[1])

		image = spriteImages[spriteIndex]

		image = image.convert("RGBA")

		# the width/height must be a multiple of 256

		w, h = image.size

		if w % 256 != 0:
			w += 256 - (w % 256)
		if h % 256 != 0:
			h += 256 - (h % 256)



		cyan_background = Image.new("RGBA", (w, h), (0, 255, 255, 255))

		blackBackgroundNames = set(["spr_vd_bg", "spr_ex_vm2"])

		if "_index" in spriteName:
			tempName = spriteName.rsplit("_index", 1)[0]
			if tempName in blackBackgroundNames:
				cyan_background = Image.new("RGBA", (w, h), (0, 0, 0, 255))

		whiteBackgroundNames = set(["spr_dr_ab___on"])

		if "_index" in spriteName:
			tempName = spriteName.rsplit("_index", 1)[0]
			if tempName in whiteBackgroundNames:
				cyan_background = Image.new("RGBA", (w, h), (255, 255, 255, 255))


		# these are a special case, need to be flipped
		if "spr_lordborders" in spriteName:

			mirrored_image = Image.new("RGBA", cyan_background.size)
			mirrored_image.paste(image, (0, 0))
			mirrored_image.paste(image.transpose(Image.FLIP_LEFT_RIGHT), (224-max_width, 0))

			image = mirrored_image

		if "spr_puumerkki_bigframe" in spriteName:

			#tempArray = np.array(image.resize((128+8, 128+8)))
			#tempImage = Image.fromarray(tempArray, Image.NEAREST)

			#image = image.resize((128+8, 128+8), Image.NEAREST)

			pass


		cyan_background.paste(image, (0, 0), image)

		if "spr_dr_ab___on" in spriteName and "spr_dr_ab___on_b" not in spriteName and spriteIndex == 0:
			# spr_dr_ab___on_b and spr_dr_ab___on index 0 should be combined into one, bc like,,,, it makes shit easier
			# they are on the same layer,
			# spr_dr_ab___on index 1 is the one below.

			tempIm = Image.open("../ExportData/Export_Textures_Padded/spr_dr_ab___on_b_0.png").convert("RGBA")

			cyan_background = Image.new("RGBA", (w, h), (0, 255, 255, 255))
			cyan_background.paste(image, (32, 32), image)
			cyan_background.paste(tempIm, (128, 16), tempIm)

			#cyan_background.show()

			pass


		cyan_background = cyan_background.convert("RGB")
		#tempImage = np.array(cyan_background)



		#print("youre gonna have a bad time")

		writeBitmap(cyan_background, os.path.join(outputPath, spriteName.lower() + ".bmp"))
		outputJson = {
			"type": "regular_bg",
			"bpp_mode": "bpp_4"
		}
		with open(os.path.join(outputPath, "dw_" + spriteName.lower() + ".json"), "w") as f:
			json.dump(outputJson, f)

		return None

	outputPath = "./formattedOutput/allSprites/"

	# Create a new blank image to stack PNGs vertically
	#stacked_image = Image.new('RGBA', (max_width, total_height))

	tiles = []

	# Paste each PNG file onto the stacked image
	y_offset = 0
	#for png_file in png_files:
	for image in spriteImages:
		#image = Image.open(os.path.join(spritePath, png_file))
		width, height = image.size
		image = image.convert("RGBA")
		#stacked_image.paste(image, (0, y_offset))
		y_offset += max_height

		cyan_background = Image.new("RGBA", (max_width, total_height), (0, 255, 255, 255))
		cyan_background.paste(image, (0, 0), image)

		cyan_background = cyan_background.convert("RGB")

		#cyan_background.save("test.png")

		tempImage = np.array(cyan_background)

		# gods so many of these funcs could, and should of been standaridized

		#VAL = 8
		VAL = 16

		for y in range(0, max_height, VAL):
			for x in range(0, max_width, VAL):

				xMax = min(x + VAL, max_width)
				yMax = min(y + VAL, max_height)

				idrk = tempImage[y:yMax, x:xMax]

				correctSize = np.full((VAL, VAL, 3), (0, 255, 255), dtype=np.uint8)

				correctSize[0:yMax-y, 0:xMax-x] = idrk

				# why does this func repeat values, so dumb
				#idrk = np.resize(idrk, (16, 16, 3))

				tiles.append(correctSize)

	outputHeight = 16
	if dimensions in validSizes:
		outputHeight = dimensions[1]

		if dimensions[0] != 16:
			hstackCount = dimensions[0] // 16

			if len(tiles) % hstackCount != 0:
				print("jesus, this should never happen. pray.")
				exit(1)

			newTiles = []

			for i in range(0, len(tiles), hstackCount):
				newTiles.append(np.hstack(tiles[i:i+hstackCount]))

			tiles = newTiles


	#print(len(tiles))


	# this is jank, stupid, and dumb.
	if spriteName == "spr_dustparticle" or spriteName == "spr_dustparticle2":

		newTiles = []

		outputHeight = 8

		for tile in tiles:
			newTiles.append(tile[0:8, 0:8])

		tiles = newTiles


	allTiles = np.vstack(tiles)

	tempImage = Image.fromarray(allTiles)

	# im going to want to extend the namespace of spriteitemstiles with my own (curse): mainly
	# the width of the bigsprite
	#,, possibly the loading position of the bigsprite? do i have access to that easily?





	dataType = "regular_bg_tiles" if isBackground else "sprite_tiles"


	writeBitmap(tempImage, os.path.join(outputPath, spriteName.lower() + ".bmp"))
	outputJson = {
		"type": dataType,
		"height": outputHeight,
		"bpp_mode": "bpp_4"
	}
	with open(os.path.join(outputPath, "dw_" + spriteName.lower() + ".json"), "w") as f:
		json.dump(outputJson, f)

	pass

def genSprite(spriteName, isBackground = False):

	# generates any (curse)ing sprite i want, on demand



	# if this sprite is a bg thing,,, dont

	paddedInputPath = "../ExportData/Export_Textures_Padded/"
	inputPath = "../ExportData/Export_Textures/Sprites/"

	spriteName = spriteName
	prefixSpriteName = spriteName

	if spriteName.startswith("dw_"):
		spriteName = spriteName[3:]
	else:
		prefixSpriteName = "dw_" + spriteName

	if os.path.isfile(os.path.join("./formattedOutput/allSprites/", prefixSpriteName + ".bmp")):
		return

	isNormalBackground = True
	if isBackground:
		# normal background tiles are just like,,, the tiles tiles,
		# non normal ones are,, things for cutscenes, like spr_vd_gray
		# those are going to have to have dupe tiles undone, and then the actual tile index map for how to rebuild them
		# included in data.win(most likely)
		# i hope/think i can overload their namespace?
		# we detect if a tileset is normal or not based on if its in the Backgrounds folder.

		tempPath = "../ExportData/Export_Textures/Backgrounds/."
		folders = [item for item in os.listdir(tempPath) if os.path.isdir(os.path.join(tempPath, item))]

		if spriteName not in folders:
			isNormalBackground = False
		else:
			return # this is a background thing, that we already generated in generateTilse
	else:
		if not os.path.isfile(os.path.join(paddedInputPath, spriteName + "_0.png")):
			return

	print("generating " + spriteName)






	# ok so,,, first try it padded, then unpadded right?
	# actually first is detecting if its a bigsprite.


	# this could be sped up by passing the image files to the actual funcs.
	# but im tired
	paddedSpriteSize, paddedSpriteImages = getSpriteDimensions(spriteName, True)
	spriteSize, spriteImages = getSpriteDimensions(spriteName, False)

	# hacky trash workaround
	# telling non-bigsprite sprites apart from sprite sprites,,
	# ugh tbh i should of only had bigsprites be sprites ABOVE 64x64, programming things this way like, ugh
	# i really dont like this solution, i should just check if somethings getting loaded from allRooms
	# i really should like, yea
	# the whole of the bigsprite class needs to be yoted

	nameMatch = False
	"""
	for name in ["tail", "gor"]:
		if name in spriteName:
			nameMatch = True
			break
	"""

	if (paddedSpriteSize in validSizes) and not nameMatch:
		# paddedsprite convert
		convertSprite(spriteName, paddedSpriteImages, paddedSpriteSize, isBackground, isNormalBackground)
		print("done generating " + spriteName)
		return
	elif (spriteSize in validSizes) and not nameMatch:
		# normal sprite convert
		convertSprite(spriteName, spriteImages, spriteSize, isBackground, isNormalBackground)
		print("done generating " + spriteName)
		return

	originalPaddedSpriteSize = paddedSpriteSize

	if not nameMatch and paddedSpriteSize[0] != 16:
		if paddedSpriteSize[0] < 64:
			v = max(5, paddedSpriteSize[0])
			v-=1;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v+=1;
			paddedSpriteSize = (v, paddedSpriteSize[1])

		if paddedSpriteSize[1] < 64:
			v = max(5, paddedSpriteSize[1])
			v-=1;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v+=1;
			paddedSpriteSize = (paddedSpriteSize[0], v)

	# (curse) programming
	if paddedSpriteSize[0] == 64 and paddedSpriteSize[1] == 16:
		paddedSpriteSize = (64, 32)

	if paddedSpriteSize[0] == 16 and paddedSpriteSize[1] == 32:
		paddedSpriteSize = (32, 64)

	if (paddedSpriteSize in validSizes) and not nameMatch:
		# paddedsprite convert
		convertSprite(spriteName, paddedSpriteImages, paddedSpriteSize, isBackground, isNormalBackground)
	elif (spriteSize in validSizes) and not nameMatch:
		# normal sprite convert
		convertSprite(spriteName, spriteImages, spriteSize, isBackground, isNormalBackground)
	else:
		# bigsprite convert

		paddedWidthErr = paddedSpriteSize[0] % 16
		paddedHeightErr = paddedSpriteSize[1] % 16

		widthErr = spriteSize[0] % 16
		heightErr = spriteSize[1] % 16

		paddedErr = paddedWidthErr * paddedHeightErr
		err = widthErr * heightErr

		if paddedErr <= err:
			#convertSprite(spriteName, paddedSpriteImages, paddedSpriteSize)
			pass
		else:
			#convertSprite(spriteName, spriteImages, spriteSize)
			pass

		#convertSprite(spriteName, paddedSpriteImages, paddedSpriteSize, isBackground, isNormalBackground)
		convertSprite(spriteName, paddedSpriteImages, originalPaddedSpriteSize, isBackground, isNormalBackground)


	print("done generating " + spriteName)

	pass

def generateCustomFloorBackground(outputPath):

	print("generating custom floor tiles")

	# in the future, a lot of vram could( and should)  be saved here by not having each rod be its own seperate thingy

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

		testPath = os.path.join("./formattedOutput/allSprites/", spriteName + ".bmp")

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

	# replace the transparency in dropoff with black.
	# this is for the DIS text, i rlly hope it doesnt cause any issues
	temp = tiles[2]
	temp[temp == 0] = 1
	tiles[2] = temp

	#tiles.append(temp[temp.shape[0]-16:temp.shape[0], 0:16])

	#print(temp.shape, stackedTiles.shape)
	#stackedTiles = np.vstack((stackedTiles, temp))

	# tiles for locust
	temp = np.array(Image.open("./formattedOutput/allSprites/dw_spr_locust_idol_menu.bmp"))
	temp[temp == 0] = 2
	tiles.append(temp)

	# tiles for rod
	temp = np.array(Image.open("./formattedOutput/allSprites/dw_spr_voidrod_icon.bmp"))
	temp[temp == 0] = 2
	tiles.append(temp)

	# tiles for super rod
	temp = np.array(Image.open("./formattedOutput/allSprites/dw_spr_voidrod_icon2.bmp"))
	temp[temp == 0] = 2
	tiles.append(temp)

	# tiles for items
	temp = np.array(Image.open("./formattedOutput/allSprites/dw_spr_items.bmp"))
	temp[temp == 0] = 2
	tiles.append(temp[16:32, 0:16])
	tiles.append(temp[32:48, 0:16])
	tiles.append(temp[48:64, 0:16])

	temp = np.array(Image.open("./formattedOutput/allSprites/dw_spr_items_cif.bmp"))
	temp[temp == 0] = 2
	tiles.append(temp[16:32, 0:16])
	tiles.append(temp[32:48, 0:16])
	tiles.append(temp[48:64, 0:16])

	# font stuff

	# blank white tile for like, yea
	tiles.append(np.full((16, 16), 2, dtype=np.uint8))
	# open and write the text data to here aswell.
	temp = np.array(Image.open("./formattedOutput/fonts/dw_fnt_text_12.bmp"))

	temp[temp == 2] = 1
	temp[temp == 0] = 2

	for i in range(0, 10, 2):
		nextTile = np.full((16, 16), 2, dtype=np.uint8)
		tempIndex = 16 * (ord(str(i)) - ord('!'))
		nextTile[0:16, 0:8] = temp[tempIndex:tempIndex+16, 0:8]
		tempIndex = 16 * (ord(str(i+1)) - ord('!'))
		nextTile[0:16, 8:16] = temp[tempIndex:tempIndex+16, 0:8]
		tiles.append(nextTile)

	#tiles.append(temp[0:temp.shape[0]-32, 0:16])

	# change the second to last blank slot in temp to be a mesh of the I and D
	# actually, i think the I and D fit in one tile, so only the second to last will be modded
	# change the last slot to be the remnants of the d

	nextTile = np.full((16, 16), 2, dtype=np.uint8)
	tempIndex = 16 * (ord('V') - ord('!'))
	nextTile[0:16, 0:8] = temp[tempIndex:tempIndex+16, 0:8]
	tempIndex = 16 * (ord('O') - ord('!'))
	nextTile[0:16, 8:16] = temp[tempIndex:tempIndex+16, 0:8]
	tiles.append(nextTile)

	iIndex = 16 * (ord('I') - ord('!'))
	dIndex = 16 * (ord('D') - ord('!'))
	iTile = temp[iIndex:iIndex+16, 0:16]
	dTile = temp[dIndex:dIndex+16, 0:16]

	iTile[0:16, 4:12] = dTile[0:16, 0:8]
	tiles.append(iTile)

	tempNewTile = np.zeros((16, 16), dtype=np.uint8)
	tempNewTile[0:16, 0:4] = dTile[0:16, 4:8]
	tempNewTile[tempNewTile == 0] = 2
	#tiles.append(tempNewTile)

	nextTile = np.full((16, 16), 2, dtype=np.uint8)

	tempIndex = 16 * (ord('B') - ord('!'))
	nextTile[0:16, 0:8] = temp[tempIndex:tempIndex+16, 0:8]
	tempIndex = 16 * (ord('?') - ord('!'))
	nextTile[0:16, 8:16] = temp[tempIndex:tempIndex+16, 0:8]
	tiles.append(nextTile)

	nextTile = np.full((16, 16), 2, dtype=np.uint8)
	tempIndex = 16 * (ord('H') - ord('!'))
	nextTile[0:16, 0:8] = temp[tempIndex:tempIndex+16, 0:8]
	tempIndex = 16 * (ord('P') - ord('!'))
	nextTile[0:16, 8:16] = temp[tempIndex:tempIndex+16, 0:8]
	tiles.append(nextTile)


	# add E for EX rooms
	nextTile = np.full((16, 16), 2, dtype=np.uint8)
	tempIndex = 16 * (ord('E') - ord('!'))
	nextTile[0:16, 0:8] = temp[tempIndex:tempIndex+16, 0:8]
	#tempIndex = 16 * (ord('P') - ord('!'))
	#nextTile[0:16, 8:16] = temp[tempIndex:tempIndex+16, 0:8]
	tiles.append(nextTile)

	# this should be above, but im scared ok
	temp = np.array(Image.open("./formattedOutput/allSprites/dw_spr_sword_flash.bmp"))
	temp[temp == 0] = 2
	tiles.append(temp[0:16, 0:16])
	tiles.append(temp[16:32, 0:16])
	tiles.append(temp[32:48, 0:16])
	tiles.append(temp[48:64, 0:16])

	temp = np.array(Image.open("./formattedOutput/allSprites/dw_spr_sword_flash_cif.bmp"))
	temp[temp == 0] = 2
	tiles.append(temp[0:16, 0:16])
	tiles.append(temp[16:32, 0:16])
	tiles.append(temp[32:48, 0:16])
	tiles.append(temp[48:64, 0:16])

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
	stackedTiles = np.uint8(stackedTiles)

	temp = Image.fromarray(stackedTiles)

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

def generateEffectsTiles(outputPath):

	print("generating custom effects tiles")

	# for saftey, im always having the first tile be a transparent one
	# additionally, these tiles no longer need to be 16x16, and tbh i should make them,,,
	# (curse) it ill 8x8
	# reconsidered on that, in the case that in the future i want more complex background effects, going 16x16 will be better

	tiles = np.full((256+16, 16, 3), (0, 255, 255), dtype=np.uint8)

	for i in range(0, 256+16, 16):

		temp = tiles[i:i+16, 0:16]

		for j in range(0, (256-i)//16):
			temp[j, 0:16] = (0, 0, 0)



	effectsTiles = np.vstack((np.full((16, 16, 3), (0, 255, 255), dtype=np.uint8), tiles))

	# generate the tiles for the textbox
	image = Image.open("../ExportData/Export_Textures/Sprites/spr_textbox_extra/spr_textbox_extra_4.png")

	#cyan_background = Image.new("RGBA", image.size, (0, 255, 255, 255))
	#cyan_background = Image.new("RGBA", image.size, (128, 255, 255, 255))
	cyan_background = Image.new("RGBA", image.size, (0, 0, 0, 255))
	cyan_background.paste(image, (0, 0), image)
	cyan_background = cyan_background.convert("RGB")
	data = np.array(cyan_background)

	sections = []

	# black
	sections.append(data[16:32, 16:32])

	# top left
	sections.append(data[0:16, 0:16])

	# top right
	sections.append(data[0:16, 224-16:224])

	# bottom left
	sections.append(data[32:48, 0:16])

	# bottom right
	sections.append(data[32:48, 224-16:224])

	# top
	sections.append(data[0:16, 16:32])

	# bottom
	sections.append(data[32:48, 16:32])

	# left
	sections.append(data[16:32, 0:16])

	# right
	sections.append(data[16:32, 224-16:224])

	tiles = np.vstack(sections)

	#tiles[np.all(tiles == (0, 255, 255), axis=-1)] = (0, 0, 0)

	tiles = np.vstack((effectsTiles, tiles))

	writeBitmap(Image.fromarray(tiles), os.path.join(outputPath, "customeffecttiles.bmp"))

	outputJson = {
		"type": "regular_bg_tiles",
		"bpp_mode": "bpp_4"
	}

	with open(os.path.join(outputPath, "dw_" + "customeffecttiles" + ".json"), "w") as f:
		json.dump(outputJson, f)
	print("done generating custom effects tiles")

	pass

def generateIncludes(folders):

	print("generating include file")

	# i could drastically speed up compilation time by only including used sprites in here.
	# i could also,,, maybe,,,, have it,,
	# i planned on having the makefile call generateAllIncludes
	# issue is, if a file is in the graphics folder in code, it will be compiled even if its not included.
	# meaning,,,, ill need dynamic copying???
	# this will speed up first time compilation, and build times after a reconvert, but like
	# is it worth the work?
	# looking through

	codeFolder = "../../code/src/"
	graphicsFolder = "../../code/graphics/"

	graphicsFiles = set([f for f in os.listdir(graphicsFolder) if f.lower().endswith('.bmp') ])

	sprite_matching_references, bg_matching_references, font_matching_references, cutsceneBGReferences = getNeededSprites()

	allMatches = set()
	allMatches.update(sprite_matching_references)
	allMatches.update(bg_matching_references)
	allMatches.update(font_matching_references)
	allMatches.update(cutsceneBGReferences)

	for spriteName in sprite_matching_references:
		genSprite(spriteName, False)

	#for spriteName in bg_matching_references:
	#	genSprite(spriteName, True)

	for spriteName in cutsceneBGReferences:
		genSprite(spriteName, True)

	output = []

	# this option is slower than just using if statements, but is still like,
	# ugh but is it only fast since im on an ssd?
	# ugh
	# i rlly shouldnt of switched, but eh, ill live my life
	# actually, once i implimented skipping it was hella fast(still on ssd tho)
	for folder in folders:

		jsonFiles = [f for f in os.listdir(folder) if f.lower().endswith('.json')]

		for file in jsonFiles:

			if os.path.basename(file).split(".")[0] not in allMatches:
				continue


			with open(os.path.join(folder, file)) as f:
				idk = json.load(f)

				line = "#include \"bn_{:s}_items_{:s}.h\"".format(idk["type"], os.path.basename(file).split(".")[0])

				output.append(line)

				# i could(and maybe should?) do something where i read the previous datawin file, but i just like, ima do this and pray
				# this might not like, properly move all changes, but i can always just do a clean run if needed
				# this should rlly be checking the hashes of the images tbh,,
				# i wonder/worry that is to expensive tho



				graphicName = os.path.basename(file).split(".")[0]

				imageFileName = os.path.basename(file).split(".")[0] + ".bmp"
				jsonFileName = os.path.basename(file).split(".")[0] + ".json"


				res = copyIfChanged(os.path.join(folder, imageFileName), graphicsFolder)
				if res:
					print(CYAN + "copying over {:s}".format(graphicName) + RESET)
				res = copyIfChanged(os.path.join(folder, jsonFileName), graphicsFolder)



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

	res = copyIfChanged("dataWinIncludes.h", "../../code/src/")
	if res:
		print(CYAN + "includes were changed, copying" + RESET)

	print("done generating include file")

	pass

def generateAllIncludes():
	os.chdir(os.path.dirname(__file__))
	#generateIncludes(["./formattedOutput/sprites/", "./formattedOutput/tiles/", "./formattedOutput/fonts/", "./formattedOutput/customFloor/", "./formattedOutput/customEffects/", "./formattedOutput/bigSprites/"])
	generateIncludes(["./formattedOutput/tiles/", "./formattedOutput/fonts/", "./formattedOutput/customFloor/", "./formattedOutput/customEffects/", "./formattedOutput/allSprites/"])

def main():

	os.chdir(os.path.dirname(__file__))

	if not os.path.isdir("../../code/graphics/"):
		createFolder("../../code/graphics/")

	#convertFonts("./formattedOutput/fonts/")

	if len(sys.argv) != 1:
		#generateCustomFloorBackground("./formattedOutput/customFloor/")

		# ok look.
		# i didnt want to fork butano
		# ok i get i already have a fork for the tail test branch, but i just, look i just didnt want to (curse)ing do it ok
		# out of all the code, the stack manipulation, the casting integers to pointers, this is what i am the most ashamed of.
		# check out the start of main.cpp for a further explanation

		copyIfChanged("irq_handler.s", "../../butano/butano/hw/3rd_party/libugba/src/")


		generateAllIncludes()
		exit(0)


	#convertBigSprite('spr_tail_boobytrap', './Sprites_Padded/spr_tail_boobytrap', '.')
	#return

	#convertTiles("./formattedOutput/tiles/")
	#return

	#convertFonts("./formattedOutput/fonts/");
	#generateCustomFloorBackground("./formattedOutput/customFloor/")
	#return

	# with undertalemodtool:

	# run ExportAllSpr	ites(script in this folder) , rename to Export_Textures_Padded, move to folder
	# run ExportAllTexturesGrouped, copy all 3 folders inside to this folder.

	# run ExportAllFontData, copy that folder to this folder.

	# these removes are only called on a cleanrun, the makefile just runs genallincludes
	if os.path.isdir("./formattedOutput/"):
		shutil.rmtree("./formattedOutput/")

	# keeping this remove here as a sanitary measue
	[ os.remove(os.path.join("../../code/graphics/", f)) for f in os.listdir("../../code/graphics/") if f.endswith(".bmp") or f.endswith(".json") ]


	createFolder("./formattedOutput/")
	createFolder("./formattedOutput/fonts/")
	createFolder("./formattedOutput/tiles/")
	createFolder("./formattedOutput/customFloor/")
	createFolder("./formattedOutput/customEffects/")
	createFolder("./formattedOutput/allSprites/")

	matching_references = getNeededSprites(True)

	for spriteName in matching_references:
		genSprite(spriteName)

	convertTiles("./formattedOutput/tiles/")
	convertFonts("./formattedOutput/fonts/")
	generateCustomFloorBackground("./formattedOutput/customFloor/")
	generateEffectsTiles("./formattedOutput/customEffects/")

	# bigsprites and sprites shouldnt have any overlap,,, but tbh like,,,, it maybe should

	print("copying over files. this may take a little bit")

	generateAllIncludes()

	pass

if __name__ == "__main__":
	main()
