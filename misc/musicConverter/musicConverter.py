
import os, sys
import shutil
import subprocess
import re
import hashlib
import struct
import json
import wave
import math
import time

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "EasyPoolProcessing"))

from poolQueue import PoolQueue
from multiprocessing import Queue, Pool, cpu_count, Event
import queue
import time
from threading import Thread

from pydub import AudioSegment

import io

from colorama import init, Fore, Back, Style

init(convert=True)

RED = Fore.RED
GREEN = Fore.GREEN
YELLOW = Fore.YELLOW
CYAN = Fore.CYAN
WHITE = Fore.WHITE + Style.BRIGHT

RESET = Style.RESET_ALL

#

def runCommand(cmd):

	res = subprocess.run(cmd, stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)

	if res.returncode != 0:
		res = subprocess.run(cmd)
		print(RED + " ".join(cmd) + " failed!" + RESET)
		exit(1)

	pass

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

def createFolder(folderPath):
	if not os.path.exists(folderPath):
		os.mkdir(folderPath)

def removeFile(filePath):
	if os.path.isfile(filePath):
		os.remove(filePath)

#

# folder containing audio files.
audioFolder = "../ExportData/Exported_Sounds/audiogroup_music/"

# path to code(butano dir), i could,, maybe have the makefile pass this in?
codeFolder = "../../code/"

srcFolder = os.path.join(codeFolder, "src/")
outputFolder = os.path.join(codeFolder, "audio/")

# true for 8 bit export, false for 16 bit
is8Bit = True

# when converting the 16 samples bit to 8 bit, using an external
# tool either didnt work(ffmpeg) or was slow(pydub)
# this chooses how you want those values to be parsed
# i like floor tbh, but round also could,, work
# you can also replace this with a lambda if desired

sampleConvertFunc = math.floor
#sampleConvertFunc = round
#sampleConvertFunc = math.ceil

# sample rate of output
GBASAMPLERATE = 8000
#GBASAMPLERATE = 8400
#GBASAMPLERATE = 9000
#GBASAMPLERATE = 14000
#GBASAMPLERATE = 24000
#GBASAMPLERATE = 21000
#GBASAMPLERATE = 27000
#GBASAMPLERATE = 31000

# if you want a song to have a different sample rate, put it here
customSampleRate = {
	#"msc_voidsong": 12000,

	# bumping to 12k really fucked this over??
	# also,,, the overtones in it are,, not ideal.
	# maybe change the convert func?
	#"msc_voidsong": 14000,
	#"msc_voidsong": 18000,
	"msc_voidsong": 23000,
	#"msc_endless": 9000, # overtones were pretty with this,,, but gods tails music needs more bits
	"msc_endless": 7500, # this REALLY NEEDS MORE BITS FOR MORE ECHO
	#"msc_001": 8300,
	#"msc_001": 8000,
	"msc_001": 6000,
	#"msc_gorcircle_lo": 8000,
	"msc_gorcircle_lo": 6000,
	#"msc_gorcircle_lo": 8000,
	#"msc_013": 8000,
	"msc_013": 7000,
	#"msc_007": 8000,
	"msc_007": 9000, # this song has issues with the silent breaks. its not a mmutil problem, its a butano issue/me fucking with butano issue
	"msc_levcircle": 5000, # sorry lev (actually it sounds rlly cool)
	#"msc_dungeon_wings": 8300, # to much high freq to, do this
	#"msc_beesong": 7000,
	"msc_beesong": 4000,
	#"msc_cifcircle": 8000,
	"msc_cifcircle": 6000,

	"msc_beecircle": 9000,
}

#for song in customSampleRate.keys():
#	customSampleRate[song] = 41000 if song == "msc_voidsong" else 1000


customConvertFunc = {

	#"msc_voidsong": round,

}

#

class ITFile:

	def __init__(self, filename):

		# tbh this doesnt need to be a class, and i barely used any members, but,, yea

		self.filename = os.path.basename(filename)

		tempBuffer = io.BytesIO()

		# ffmpeg -y -i ../ExportData/Exported_Sounds/audiogroup_music/msc_voidsong.ogg -map 0 -f s16le pipe:
		cmd = ["ffmpeg", "-y", "-i", filename + ".ogg",
		"-ac", "1",
		"-filter_complex",
		"[0]highpass=f=120:p=2[3];[3]lowpass=f={:d}:p=2[2];[2]aresample={:d}[1]".format(int( (GBASAMPLERATE // 2) * 0.707 ) , GBASAMPLERATE),
		"-map", "[1]", "-f", "s16le", "pipe:"]

		res = subprocess.run(cmd, stdout = subprocess.PIPE, stderr = subprocess.DEVNULL)


		if res.returncode != 0:
			res = subprocess.run(cmd)
			print(RED + " ".join(cmd) + " failed!" + RESET)
			exit(1)

		self.sampleBytes = struct.unpack('<' + 'h' * (len(res.stdout) // 2), res.stdout)

		# div by 2 bc 16 bit
		frameCount = len(self.sampleBytes)
		self.duration = frameCount / GBASAMPLERATE

		self.frameCount = frameCount

		self.tickValue = math.ceil( (1440/60) * ( (self.duration/254)/1))
		# WAIT TICKVALUE CAN GO LIKE,, TO 1????
		# WAS IT BEFORE I WAS ON THE IT FORMAT THAT I LIKE,,, THOUGHT IT COULDNT??
		self.tickValue = min(max(self.tickValue, 1), 255)

		while self.tickValue < 64:

			self.segmentTime = (self.tickValue / 24) * 1

			samplesPerSegment = self.segmentTime * GBASAMPLERATE

			if not samplesPerSegment.is_integer():
				self.tickValue += 1
				continue

			break
		else:
			print(RED + "couldnt find a valid tickrate??? this should never happen!" + RESET)
			exit(1)

		samplesPerSegment = int(samplesPerSegment)

		if samplesPerSegment / GBASAMPLERATE > 2:
			print(RED + "sample length is {:f} which may be to long, and might cause issues with playback!".format(samplesPerSegment / GBASAMPLERATE) + RESET)

		self.samplesPerSegment = samplesPerSegment
		self.fileSampleCount = samplesPerSegment
		self.lastSampleCount = frameCount % samplesPerSegment # IS THIS CORRECT

		self.sampleCount = int(math.ceil(frameCount / samplesPerSegment))

		if self.sampleCount > 255:
		#if frameCount / samplesPerSegment > 255:
			print("max samplecount of 255 exceded")
			print("idk what to do dumbass, either split the song, increase the segment time, or fix shit")
			exit(1)
		#print(self.sampleFilenames)

		# FURTHER OPTIMIZATION COULD(and should) be done by identifying
		# duplicated parts of songs.
		# however, due to the process of slicing being used, those duplicates will,,, basically never happen
		# ALSO, this should occur,, in the 8bit area if 8bit slicing is occuring
		# weirdly enough, monstrail had a duplicate(even when 16 bit)
		# funny
		# i could,,,, use tick commands to alter the playback speed live in order to,,
		# have different length samples.
		# it would be so nice if there was a "play sample after previous one ends" thing
		# also, being able to compare samples between,,, 2 different songs??

		self.sampleFilenames = [ "{:s}{:03d}".format(self.filename, i) for i in range(0, self.sampleCount) ]

		# convert the samples to 8bit if needed

		if is8Bit:

			tempData = [ short for short in self.sampleBytes ]

			#tempData = [ short // 256 for short in tempData ]

			#basicConvertFunc = lambda v : v // 256
			#basicConvertFunc = lambda v : v // 256 if v > 0 else -((-v) // 256)

			roundDown = lambda v : math.floor(v) if v > 0 else -math.floor(-v)
			roundUp = lambda v : math.ceil(v) if v > 0 else -math.ceil(-v)

			def basicConvertFunc(index):
				# only pass indicies 1- len-1 here

				curr = self.sampleBytes[index]

				if index == 0 or index == -1 or index == len(self.sampleBytes) - 1:
					#return curr // 256 if curr > 0 else -((-curr) // 256)
					return round(curr / 256)

				prev = self.sampleBytes[index-1]
				next = self.sampleBytes[index+1]

				isNegative = curr < 0

				# unsigned short range: -32768 <= number <= 32767

				temp = curr / 256

				# 0 clue if this is actually helpful
				# might caus ea lot more background static???
				# MORE RESEARCH NEEDS TO BE DONE HERE
				# now that im converting the whole thing in one go, i could go back to the ffmpeg unsigned 8bit -128 solution??
				# and do it in ram
				# ESP WITH TAILS THEME
				# ughhhhh

				useFunc = round

				return useFunc(temp)

			for i in range(0, len(tempData)):

				temp = basicConvertFunc(i)

				tempData[i] = int(max(min(temp, 127), -128))

			self.sampleBytes = tempData


		pass

	def writeSample(self, bytes, sampleFilename):

		offsetRes = len(bytes)

		# https://github.com/OpenMPT/openmpt/blob/47fc65b7b01455021284e3a5251ef16736bcd077/soundlib/ITCompression.cpp

		index = self.sampleFilenames.index(sampleFilename)

		start = index * self.samplesPerSegment
		end = (index + 1) * self.samplesPerSegment
		#end += 1 # IS THIS OK?
		end = min(end, self.frameCount)

		tempData = self.sampleBytes[start:end]

		if is8Bit:
			bytes += struct.pack("{:d}b".format(len(tempData)), *tempData)
		else:
			bytes += struct.pack("{:d}h".format(len(tempData)), *tempData)

		return offsetRes

	def writeSampleHeader(self, bytes, sampleFilename):

		# ik bytes should be self.bytes, but if theres one thing that annoys me in python, its writing self a ton
		# also,,, should this even be a class tbh?
		# bytes is passed by ref too, so this is ok

		# also, if the sample is converted in any form during,,,, the import to openmpt
		# having to replicate that here might be a deadend

		#wavFile = wave.open(sampleFilename)
		## shit var name, i should have this be a lot different from samplecount/samplefilenames, but im tired
		#fileSampleCount = wavFile.getnframes()
		#wavFile.close()

		#fileSampleCount = os.path.getsize(sampleFilename)

		# return the offset where we started writing
		offsetRes = len(bytes)

		# HEADER HEADER
		bytes += bytearray("IMPS", "ascii")

		# DOS NAME
		temp = bytearray(os.path.basename(sampleFilename).rsplit(".", 1)[0][:12], "ascii")
		while len(temp) < 12:
			temp.append(0)
		bytes += temp

		# reserved
		bytes += struct.pack("B", 0)

		# global vol
		bytes += struct.pack("B", 64)

		# flags vol
		# turning on pingpong sustain,,, to maybe stop the blips between notes?
		#bytes += struct.pack("B", 0x0B | 0x80 | 0x20)
		# ACTUALLY NO, BC THAT WILL FUCK UP THE END OF THE SONG,,, BUT DOES IT HELP?
		# ya know what? im just going to ,,,, hope this doesnt happen.
		#bytes += struct.pack("B", 0x0B)
		#bytes += struct.pack("B", 0x0B)
		#bytes += struct.pack("B", 0b1011)
		#bytes += struct.pack("B", 0b1001)
		#bytes += struct.pack("B", 0b0001)
		#bytes += struct.pack("B", 0b0011)

		# val = ((not is8Bit) << 1) | 1

		if is8Bit:
			val = 0b01
		else:
			val = 0b11

		bytes += struct.pack("B", val)

		# default vol
		bytes += struct.pack("B", 64)

		# sample name
		temp = bytearray(os.path.basename(sampleFilename).rsplit(".", 1)[0][:26], "ascii")
		while len(temp) < 26:
			temp.append(0)
		bytes += temp

		# convert flags
		# 8bit uses unsigned 8 bit, 16 bit uses signed 16 bit
		#bytes += struct.pack("B", not is8Bit)
		bytes += struct.pack("B", 0b00000001)
		#bytes += struct.pack("B", 0x01 | 0x04)
		#bytes += struct.pack("B", 0x00 | 0x04)

		# default pan
		bytes += struct.pack("B", 32)

		# sample length
		#bytes += struct.pack("I", fileSampleCount)
		# THIS DOESNT OUTPUT THE RIGHT LENGTH FOR THE LAST SAMPLE!!

		if sampleFilename == self.sampleFilenames[-1]:
			bytes += struct.pack("I", self.lastSampleCount)
		else:
			# DOES THIS HELP AT ALL?
			bytes += struct.pack("I", self.fileSampleCount+16)

		# Loop beginning
		bytes += struct.pack("I", 0)

		# Loop end
		bytes += struct.pack("I", 0)

		# C5 (middle C) speed
		bytes += struct.pack("I", GBASAMPLERATE)

		# Sustain loop beginning
		bytes += struct.pack("I", 0)

		# Sustain loop end
		bytes += struct.pack("I", 0)
		#bytes += struct.pack("I", fileSampleCount)
		#bytes += struct.pack("I", self.fileSampleCount)

		sampleDataOffset = len(bytes)
		# Sample pointer
		bytes += struct.pack("I", 0)

		# Vibrato speed
		bytes += struct.pack("B", 0)

		# Vibrato depth
		bytes += struct.pack("B", 0)

		# Vibrato waveform
		bytes += struct.pack("B", 0)

		# Vibrato Sweep
		bytes += struct.pack("B", 0)

		return [offsetRes, sampleDataOffset]

	def writePattern(self, bytes):

		offsetRes = len(bytes)

		# we need a row per sample

		if self.sampleCount < 3:
			print("look, i understand how the sample format works, but im scared to work with it for this low of a samplecount")
			exit(1)

		patternBytes = bytearray()

		for i in range(0, self.sampleCount):

			if i == 0:
				patternBytes += struct.pack("B", 0x81)
				patternBytes += struct.pack("B", 0x03)
				patternBytes += struct.pack("B", 0x3C)
				patternBytes += struct.pack("B", 0x01)
			elif i == 1:
				patternBytes += struct.pack("B", 0x81)
				patternBytes += struct.pack("B", 0x12)
				patternBytes += struct.pack("B", 0x02)
			else:
				patternBytes += struct.pack("B", 0x01)
				patternBytes += struct.pack("B", i+1)

			# nullterm
			patternBytes += struct.pack("B", 0)
			# twice, bc we are going in 4sec incs now
			#patternBytes += struct.pack("B", 0)

			#patternBytes += struct.pack("B", 0)

		# NOT sure if needed, prevent shit from getting cut off?
		patternBytes += struct.pack("B", 0)

		# pattern length
		bytes += struct.pack("H", len(patternBytes))

		if self.sampleCount+1 > 0xFF:
			print("SHITS FUCKED")
			exit(1)

		bytes += struct.pack("H", self.sampleCount+1)

		# reserved
		bytes += bytearray("SDWM", "ascii")

		# actual data
		bytes += patternBytes

		return offsetRes

	def writeHeader(self, bytes):

		#self.f = open(self.filename.rsplit(".", 1)[0] + ".it", "wb")

		bytes += bytearray("IMPM", "ascii")

		temp = bytearray(self.filename[:26], "ascii")
		while len(temp) < 26:
			temp.append(0)

		bytes += temp

		# rows per beat,
		bytes += struct.pack("B", 4)
		#rows per measure
		bytes += struct.pack("B", 16)

		# order number(i rlly think this should be 1? but its two for some reason?
		bytes += struct.pack("H", 2)

		# instrument num, 0
		bytes += struct.pack("H", 0)

		# samplenum, DYNAMIC
		bytes += struct.pack("H", self.sampleCount)

		# pattern number(i rlly think this should be 1? but its two for some reason?
		bytes += struct.pack("H", 1)

		# software version num and created num
		#bytes += struct.pack("H", 0x5131)
		bytes += struct.pack("H", 0x5131)
		#bytes += struct.pack("H", 0x0214)
		bytes += struct.pack("H", 0x0214)

		# flags.
		bytes += struct.pack("H", 0b00001000)

		# special
		bytes += struct.pack("H", 6)

		# "globvol": 128,
		bytes += struct.pack("B", 128)
		# "mixvol": 48,
		#bytes += struct.pack("B", 48)
		bytes += struct.pack("B", 128) # actually becomes 255 for some reason

		# does this control synth or sampkle vol??
		#bytes += struct.pack("B", 255)


		# "initspeed": 24,
		#bytes += struct.pack("B", 24)
		bytes += struct.pack("B", self.tickValue)
		#bytes += struct.pack("B", 21)
		#bytes += struct.pack("B", 18)
		#bytes += struct.pack("B", 15)
		# "inittempo": 60,
		bytes += struct.pack("B", 60)
		# "pansep": 128,
		bytes += struct.pack("B", 128)
		# "Pitch wheel depth": ,
		bytes += struct.pack("B", 0)
		# disagreement here


		# msg length
		bytes += struct.pack("H", 0)

		# msg offset
		bytes += struct.pack("I", 0)

		# reserved
		#f.write(struct.pack("I", 0))
		#bytes += bytearray("OMPT", "ascii")
		bytes += bytearray("SDWM", "ascii")


		# this is now either the pantable/voltable, or vice versa
		# my "documentation" and the other python script disagree
		# https://fileformats.fandom.com/wiki/Impulse_tracker

		bytes += struct.pack("B", 32)

		for _ in range(0, 63):
			bytes += struct.pack("B", 160)

		for _ in range(0, 64):
			bytes += struct.pack("B", 64)

		# no clue, something abt order numbers
		bytes += struct.pack("B", 0)
		bytes += struct.pack("B", 255)

		pass

	def write(self):


		bytes = bytearray()

		self.writeHeader(bytes)

		# write Sample Header offsets

		sampleOffsetStart = len(bytes)

		for _ in range(0, self.sampleCount):
			bytes += struct.pack("I", 0xFFFFFFFF)

		# write Pattern offsets
		patternOffsetStart = len(bytes)
		bytes += struct.pack("I", 0xFFFFFFFF)

		# AFTER WRITING HEADERS, WE NEED 2 BYTES TO 0 THE EDIT HIST
		bytes += struct.pack("H", 0)

		sampleOffsets = []
		sampleDataPointerOffsets = []

		for sampleFilename in self.sampleFilenames:
			offsetRes, offsetDataRes = self.writeSampleHeader(bytes, sampleFilename)
			sampleOffsets.append(offsetRes)
			sampleDataPointerOffsets.append(offsetDataRes)

		patternOffset = self.writePattern(bytes)

		sampleDataOffsets = []
		for sampleFilename in self.sampleFilenames:
			sampleDataOffsetRes = self.writeSample(bytes, sampleFilename)
			sampleDataOffsets.append(sampleDataOffsetRes)

		# i have no clue, i wanted to set sample vol, which is down here for some reason
		footerBytes = [
		0x53, 0x54, 0x50, 0x4D, 0x2E, 0x2E, 0x2E, 0x43, 0x02, 0x00, 0x01, 0x00,
		0x2E, 0x2E, 0x4D, 0x54, 0x01, 0x00, 0x00, 0x2E, 0x4D, 0x4D, 0x50, 0x04,
		0x00, 0x04, 0x00, 0x00, 0x00, 0x56, 0x57, 0x53, 0x4C, 0x04, 0x00, 0x00,
		0x04, 0x31, 0x01, 0x2E, 0x41, 0x50, 0x53, 0x04, 0x00, 0xFF, 0x00, 0x00,
		0x00, 0x56, 0x54, 0x53, 0x56, 0x04, 0x00, 0x30, 0x00, 0x00, 0x00, 0x2E,
		0x46, 0x53, 0x4D, 0x10, 0x00, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x05,
		0x00, 0x00, 0x00, 0x80, 0x01, 0xD0, 0x01, 0x88, 0x01, 0x43, 0x43, 0x4F,
		0x4C, 0x04, 0x00, 0xFF, 0xA8, 0xA8, 0x00, 0x49, 0x4E, 0x41, 0x4E, 0x41
		]

		for b in footerBytes:
			bytes += struct.pack("B", b)

		memoryView = memoryview(bytes)

		for i, offset in enumerate(sampleOffsets):
			index = sampleOffsetStart + (i * 4)
			memoryView[index:index+4] = struct.pack("I", offset)

		memoryView[patternOffsetStart:patternOffsetStart+4] = struct.pack("I", patternOffset)

		for i in range(0, len(sampleDataPointerOffsets)):
			index = sampleDataPointerOffsets[i]
			val = sampleDataOffsets[i]

			memoryView[index:index+4] = struct.pack("I", val)

		output = os.path.join("./formattedOutput/", self.filename.rsplit(".", 1)[0] + ".it")
		f = open(output, "wb")
		f.write(bytes)
		f.close()

		pass

#

def doFile(filename):

	global GBASAMPLERATE
	global sampleConvertFunc

	sampleBackup = GBASAMPLERATE
	convertBackup = sampleConvertFunc

	if filename in customSampleRate:
		GBASAMPLERATE = customSampleRate[filename]
		#print(CYAN + "running {:s} with {:d} as the samplerate".format(filename, GBASAMPLERATE) + RESET)

	if filename in customConvertFunc:
		sampleConvertFunc = customConvertFunc[filename]
		#print(CYAN + "running {:s} with a custom convert func".format(filename) + RESET)


	#print(WHITE + "cooking {:s}".format(filename) + RESET)

	inputPath = os.path.join(audioFolder, filename)

	test = ITFile(inputPath)
	test.write()

	#print(WHITE + "done cooking {:s}".format(filename) + RESET)

	GBASAMPLERATE = sampleBackup
	sampleConvertFunc = convertBackup

	pass

def getNeededFiles():


	codeFiles = [os.path.join(srcFolder, f) for f in os.listdir(srcFolder) if f.lower().endswith('.h') or f.lower().endswith('.cpp')]

	if "../../code/src/dataWinIncludes.h" in codeFiles:
		codeFiles.remove("../../code/src/dataWinIncludes.h")

	refs = set()

	pattern1 = r'bn::music_items::\w+'

	for file in codeFiles:
		with open(file, 'r', encoding='utf-8', errors='ignore') as f:
			content = f.read()
			matches = [ s.strip()[len("bn::music_items::"):] for s in re.findall(pattern1, content) ]
			refs.update(matches)

	refs.remove("cifdream")
	refs.remove("dancing_lesson")

	#refs = ["msc_voidsong"]

	return refs

def copyNeededMusic():

	copyIfChanged("cifdream.xm", outputFolder)
	copyIfChanged("dancing_lesson.it", outputFolder)

	files = getNeededFiles()

	for file in files:
		if not os.path.isfile(os.path.join("./formattedOutput/", file + ".it")):
			doFile(file)
		res = copyIfChanged(os.path.join("./formattedOutput/", file + ".it"), outputFolder)
		if res:
			print(CYAN + "copied over {:s}".format(file) + RESET)


	pass

def convertSongWorker(jobQueue: Queue, returnQueue: Queue, shouldStop: Event):

	while not shouldStop.is_set():

		try:
			data = jobQueue.get(timeout = 0.5)
		except queue.Empty:
			time.sleep(0.001)
			continue

		data = data[0]

		res = doFile(data)

		returnQueue.put([data, True])

def convertAllFiles():

	os.chdir(os.path.dirname(__file__))

	createFolder("./formattedOutput")
	createFolder(outputFolder)
	if len(sys.argv) != 1:
		copyNeededMusic()
		exit(0)

	if os.path.exists("./formattedOutput"):
		shutil.rmtree("./formattedOutput")

	os.mkdir("./formattedOutput/")

	files = getNeededFiles()

	pool = PoolQueue(convertSongWorker, cpuPercent = 0.75)
	pool.start()

	for file in files:
		#doFile(file)
		pool.send([file])
	#print(CYAN + "WAITING ON POOL JOIN" + RESET)
	resData = pool.join()

	copyNeededMusic()

	pass

def main():
	convertAllFiles()

if __name__ == "__main__":

	main()

	pass
