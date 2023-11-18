
import os, sys
import shutil
import subprocess
import re
import hashlib
import struct
import json
import wave
import math


from pydub import AudioSegment


from colorama import init, Fore, Back, Style

init(convert=True)

RED = Fore.RED 
GREEN = Fore.GREEN 
CYAN = Fore.CYAN
WHITE = Fore.WHITE + Style.BRIGHT

RESET = Style.RESET_ALL

"""

mission objective: read a file, edit a file, write a () file.
there is like,, no documentation on the IT format tho, and gods the pattern table is one thing, but having 
to adjust the number of samples??

this is the best thing i could find 

https://fileformats.fandom.com/wiki/Impulse_tracker


"""

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

#Compress(src_samples, len(src_samples))
		
# 

# THERE IS NO REASON FOR IT TO GO SO LOW 
# IS,,, IS DEVKIT CONVERTING MY 8BIT SAMPLES TO 16???
# WHY 
# it seems to,,, yea be doing exactly that.
# it,, is has something which says "adding instrument templates"
# is it,,,,,,,, commiting some sort of fuckery with that?
# the # of instruments is the same as samples
# https://github.com/devkitPro/mmutil/blob/8b0792e45852b58fda4def4d263e246d3a73662d/source/it.c#L510
# going to,,,, gods 
# probs going to need to make modifications to mmutil
# https://github.com/devkitPro/mmutil/blob/8b0792e45852b58fda4def4d263e246d3a73662d/source/msl.c#L55
# msl, not main, seems to have what i need 
# TMP_SAMP is,,, the file whichsize i need to fix.
# ok. 
# the only feasable way this bs would ever occur is:
# REMOVE ALL CALLS TO BN::CORE::UPDATE, EVERYTHING NEEDS TO GO THROUGH DOBUTANOUPDATE
# inside dobutanoupdate, BEFORE we call a frame, we would decompress the pcm data, and 
# then copy that over into maxmod somehow 
# the issue?
# 31000/60 ~= 516.66 bytes per frame.
# i could write my own comp/decom algo, and have it be fast 
# the issue then becomes,,, is that even how fucking sound works?
# is maxmod called via interrupt behind the scenes???
# will all this effort be worth it at all
# ideally, instead of hijacking on maxmod, ill literally just rewrite maxmod's funcs.
# but also,,, does maxmod run during vblank? bc ideally i would give my extra cpu cycles to doing decomp 
# which also means,,, gods 
# which means that my general performance levels now matter.

# does mmutil convert to 8bit samples on its own? lets find out

#GBASAMPLERATE = 9000


# set to false for 16 bit.
# i am using signed pcm for both. i would be better to use unsigned for 8 bit bc ffmpeg easily supports it, 
# but i have had a lot of issues with it

is8Bit = True 
#is8Bit = False 

GBASAMPLERATE = 9000
#GBASAMPLERATE = 14000
#GBASAMPLERATE = 24000
#GBASAMPLERATE = 21000
#GBASAMPLERATE = 27000
#GBASAMPLERATE = 31000


class ITFile:

	def __init__(self, filename):
	
		self.filename = os.path.basename(filename)
		
		# not efficient
		#self.bytes = []
		
		#self.samples = []
		
		#self.f = None
		
		if os.path.isdir(self.filename):
			shutil.rmtree(self.filename)
		createFolder(self.filename)
		
		
		# TODO, REMOVE THIS STEP, NO POINT IN PIPING THROUGH GSM WHEN WE MURDER THE QUALITY ON OUR OWN LATER 
		
	
		
		# looking back on things,,, does the bs im doing with freq changing even help at all??
		
		runCommand([
			'ffmpeg',
			"-y", '-i', filename + ".ogg",
			#"-ac", "1", "-af", "aresample=18157",
			#"-ac", "1", "-af", "aresample=8000",
			#"-ac", "1", "-af", "aresample=18000",
			#"-ac", "1", "-af", "aresample={:d}".format(GBASAMPLERATE*2),
			#"-ac", "1", "-af", "aresample={:d}".format(GBASAMPLERATE),
			self.filename + ".wav"
			])
		
		# does,,, lowing the shit and playing it at 2x speed actually help my filesize at all???
		# increaseing the duration also like,,, fucks me in terms of sample count 

		
		codecCommand = ["-acodec", "pcm_u8"] if is8Bit else ["-acodec", "pcm_s16le"]
		
		runCommand([
			'ffmpeg',
			"-y", '-i', self.filename + ".wav",
			"-ac", "1", "-af", "aresample={:d}".format(GBASAMPLERATE),
			#+ codecCommand + [
			self.filename + "2.wav"
			])
		
		
		waveFile = wave.open(self.filename + "2.wav")
		frameCount = waveFile.getnframes()
		self.duration = waveFile.getnframes() / waveFile.getframerate()
		waveFile.close()
		
		
		self.tickValue = math.ceil( (1440/60) * ( (self.duration/254)/1))
		# WAIT TICKVALUE CAN GO LIKE,, TO 1????
		# WAS IT BEFORE I WAS ON THE IT FORMAT THAT I LIKE,,, THOUGHT IT COULDNT??
		self.tickValue = max(self.tickValue, 1)
		
		while self.tickValue < 64:
		
			self.segmentTime = (self.tickValue / 24) * 1
			
			samplesPerSegment = self.segmentTime * GBASAMPLERATE
			
			if not samplesPerSegment.is_integer():
				print(RED + "samplesPerSegment wasnt a integer! panic!" + RESET)
				print("samplesperseg: ", samplesPerSegment)
				print("segmenttime: ", self.segmentTime)
				print("tickval: ", self.tickValue)
				self.tickValue += 1
				continue
				#exit(1)
				#print("or dont")
				
			break
			
		samplesPerSegment = int(samplesPerSegment)
		
		#ffmpeg -i input.wav -filter_complex "[0]atrim=start=0:end=9999[a]; [0]atrim=start=10000:end=19999[b]; [0]atrim=start=20000:end=29999[c]; ..." -map "[a]" output_1.wav -map "[b]" output_2.wav -map "[c]" output_3.wav ...
		# do not ask.
		
		cmd = [
			"ffmpeg", "-y", "-i", self.filename + "2.wav", "-filter_complex"
		]
		
		filterString = []
		mapCmds = []
		
		#frameCount = samplesPerSegment*4
		index = 1
		for i in range(0, frameCount, samplesPerSegment):
			start = i
			#end = min(i+samplesPerSegment-1, frameCount)
			end = min(i+samplesPerSegment, frameCount)
		
			filterString.append("[0]atrim=start_pts={:d}:end_pts={:d}[{:d}]".format(start, end, index))
			
			mapCmds.append("-map")
			mapCmds.append("[{:d}]".format(index))
			#mapCmds += codecCommand
			mapCmds.append(self.filename + "/output_{:03d}.wav".format(index))
			
			index += 1
			
		cmd.append("; ".join(filterString))
		
		cmd += mapCmds

		runCommand(cmd)
		
		self.sampleFilenames = [os.path.join("./"+self.filename, f) for f in os.listdir("./"+self.filename) if os.path.isfile(os.path.join("./"+self.filename, f))]
		
		self.sampleCount = len(self.sampleFilenames)
		
		if self.sampleCount > 255:
			print("max samplecount of 255 exceded")
			print("idk what to do dumbass, either split the song, increase the segment time, or fix shit")
			exit(1)
		#print(self.sampleFilenames)
		
		"""
		vals = []
		
		for filename in self.sampleFilenames[:-1]:
			waveFile = wave.open(filename)
			#print( waveFile.getnframes())
			temp = waveFile.getnframes()# / waveFile.getframerate()
			waveFile.close()
			vals.append(temp)
		
		minVal = min(vals)
		maxVal = max(vals)
		difVal = maxVal - minVal
		secondsCount = difVal / GBASAMPLERATE
		
		print(CYAN + "min: {:f}  max: {:f}  dif: {:f} which is {:f} seconds".format(minVal, maxVal, difVal, secondsCount) + RESET)
		
		if secondsCount > 0.01:
			print("somethings fucked")
			exit(1)
			
		
		print(CYAN + "converting {:s} to pcm".format(self.filename))
		for filename in self.sampleFilenames:
			output = filename.rsplit(".", 1)[0] + ".pcm"
			
			temp = AudioSegment.from_wav(filename)
			temp.export(output, format='s8')
		
		for filename in self.sampleFilenames:
			removeFile(filename)
			
		self.sampleFilenames = [os.path.join("./"+self.filename, f) for f in os.listdir("./"+self.filename) if os.path.isfile(os.path.join("./"+self.filename, f))]
		"""
	
		
		pass
	
	def writeSample(self, bytes, sampleFilename):
	
		offsetRes = len(bytes)
		
		
		# https://github.com/OpenMPT/openmpt/blob/47fc65b7b01455021284e3a5251ef16736bcd077/soundlib/ITCompression.cpp
		
		# i have like,,, no clue what to do here
		
		# this duplicate file open is stupid.
		
		#wavFile = wave.open(sampleFilename)
		#sampleCount = wavFile.getnframes() # once again, var name duplication 
		#wavBytes = wavFile.readframes(sampleCount)
		#wavFile.close()
		
		#wavFile = wave.open(sampleFilename)
		#sampleCount = wavFile.getnframes() # once again, var name duplication 
		#wavBytes = wavFile.readframes(sampleCount)
		#wavFile.close()
		
		wavFile = wave.open(sampleFilename)
		sampleCount = wavFile.getnframes()
		wavBytes = wavFile.readframes(sampleCount)
		wavFile.close()
		bytes += struct.pack("H", len(wavBytes))
		
		
		if is8Bit:
				
			temp = struct.unpack('<' + 'h' * (len(wavBytes) // 2), wavBytes)		
			
			# is this,,,, ok????
			# im just doing this instead of some other form of quantization
			for i in range(0, len(temp)):
				bytes += struct.pack("b", temp[i] // 256)
		
		else:
			bytes += wavBytes
		
		# stupid, why didnt i need to do this previously???
		return offsetRes+2
	
	def writeSampleHeader(self, bytes, sampleFilename):
	
		# ik bytes should be self.bytes, but if theres one thing that annoys me in python, its writing self a ton 
		# also,,, should this even be a class tbh?
		# bytes is passed by ref too, so this is ok 
	
		# also, if the sample is converted in any form during,,,, the import to openmpt 
		# having to replicate that here might be a deadend
	
		wavFile = wave.open(sampleFilename)
		# shit var name, i should have this be a lot different from samplecount/samplefilenames, but im tired
		fileSampleCount = wavFile.getnframes()
		wavFile.close()
		
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
		bytes += struct.pack("I", fileSampleCount)
		
		# Loop beginning
		bytes += struct.pack("I", 0)
		
		# Loop end
		bytes += struct.pack("I", 0)
	
		# C5 (middle C) speed
		bytes += struct.pack("I", GBASAMPLERATE)
		
		# Sustain loop beginning
		bytes += struct.pack("I", 0)
		
		# Sustain loop end
		#bytes += struct.pack("I", 0)
		bytes += struct.pack("I", fileSampleCount)
		
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
		
		
		"""
		idk = bytearray([
		0x81, 0x03, 0x3C, 0x01, 0x00, 0x81, 0x12, 0x02, 0x00, 0x01, 0x03, 0x00,
		0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
		])
		
		index = [0]
		channelMask = 0
		maskVariable = 0
		
		# 3C is middle C
		
		def getByte():
			
			res = idk[index[0]]
			index[0] += 1
			return res
		
		row = 0
		while index[0] < len(idk):
			
			channelMask = getByte()
			print("\nstarting another while loop, row={:d}".format(row))
			row += 1
			
			while channelMask > 0:
				channel = (channelMask - 1) & 63
				if channelMask & 0x80:
					maskVariable = getByte()
				
					
				if maskVariable & 0x01:
					print("get note {:02X}".format(getByte()))
				elif maskVariable & 0x10:
					print("last note {:02X}".format(maskVariable))

				if maskVariable & 0x02:
					print("get instrument {:02X}".format(getByte()))
				elif maskVariable & 0x20:
					print("last instrument {:02X}".format(maskVariable))
				
				if maskVariable & 0x04:
					print("get volume {:02X}".format(getByte()))
				elif maskVariable & 0x40:
					print("last volume {:02X}".format(maskVariable))
					
				if maskVariable & 0x08:
					#get effect byte
					#get effect value byte
					print("fuck")
					pass
				elif maskVariable & 0x80:
					#use last effect byte
					#use last effect value byte
					print("fuck")
					pass
			
				
				channelMask = getByte()
		"""
	

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
		
		# row count 
		#bytes += struct.pack("H", self.sampleCount)
		#bytes += struct.pack("H", 0xFF + 1)
		#bytes += struct.pack("H", self.sapleCount*2)
		
		# it seems like,,,,, mmutil has some, fuckery when over something over,,, not 0x0100, but 0x010F fails?? 
		# i need to put samples closer together, without,,, fucking shit 
		# or,, i could have a second pattern,, maybe?
		# that will be annoying to program tho 
		# also, i need to lower the samples to 3sec, ithink
		# 0x0107 fails 
		# 0x0103 fails
		# 0x0102 fails
		# 0x0101 passes. what the fuck?
		# actually, while 0x0101 passes, it shits itself on the gba???
		# im setting a hard limit of 0xFF 
		# im not even going to try 0x0100
		# im going to need multiple patterns
		
		# or maybe,,, set pitch to 2, tempo to 0.5,,,
		# YES
		# this doesnt solve my issue of,,, the cutting in and out. is a 4sec sample to much?
		# the cutting is only occasional. 
		# can i goto 3sec without severely fucking my math up?
		# its sucha bummer, i think 3.5 seconds would work tbh 
		# the cuts are so short 
		
		# 4/2)/2 = 1 
		# 3.5/2)/2 = 0.875
		# 60 * 24 = 1440, so currently 1440 ticks per row. 
		# 1440 * .875 = 1260, 1260/60 - 21,,, so 60 bpm, 21 ticks,,,,, yea?
		
		# 3.5 is still cutting out,,, but but only during the start weirdly enough? 
		# i can go down to 3 without hitting 255, so i mean,,, yea
		
		#bytes += struct.pack("H", 0xFF)
		
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
		
		# flags. unsure tbh 
		# most conserning one is for if this is mono or not. bc it should be! but 73 says it is 
		# switching to 72 doesnt,,, seem to cause problems tho?
		#bytes += struct.pack("H", 73)
		#bytes += struct.pack("H", 72) # KILL YOU'RE SELF
		bytes += struct.pack("H", 0b00001000) # KILL YOU'RE SELF
		#bytes += struct.pack("H", 0b00001100) # KILL YOU'RE SELF
		
		# 72 = 0100 1000
		
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
		
		#insoffs = struct.unpack('<%dL' % data[2], f.read(data[2] * 4))
		#smpoffs = struct.unpack('<%dL' % data[3], f.read(data[3] * 4))
		#patoffs = struct.unpack('<%dL' % data[4], f.read(data[4] * 4))
		
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
		
	
		#memoryView[0:0+4] = bytearray("fuck", "ascii")
		
		# the actual file seems to put my name in it at the end, even though i dont believe that is in the spec
		# wait no! it is
		
		# sample vol is here, for some reason?
		#bytes += struct.pack("B", 0xFF)
		
		
		
		# i have no clue, i wanted to set sample vol, which is down here for some reason
		footerBytes = [
		0x53, 0x54, 0x50, 0x4D, 0x2E, 0x2E, 0x2E, 0x43, 0x02, 0x00, 0x01, 0x00,
		0x2E, 0x2E, 0x4D, 0x54, 0x01, 0x00, 0x00, 0x2E, 0x4D, 0x4D, 0x50, 0x04,
		0x00, 0x04, 0x00, 0x00, 0x00, 0x56, 0x57, 0x53, 0x4C, 0x04, 0x00, 0x00,
		0x04, 0x31, 0x01, 0x2E, 0x41, 0x50, 0x53, 0x04, 0x00, 0xFF, 0x00, 0x00,
		0x00, 0x56, 0x54, 0x53, 0x56, 0x04, 0x00, 0x30, 0x00, 0x00, 0x00, 0x2E,
		0x46, 0x53, 0x4D, 0x10, 0x00, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x05,
		0x00, 0x00, 0x00, 0x80, 0x01, 0xD0, 0x01, 0x88, 0x01, 0x43, 0x43, 0x4F,
		0x4C, 0x04, 0x00, 0xFF, 0xA8, 0xA8, 0x00
		]
		
		for b in footerBytes:
			bytes += struct.pack("B", b)
		
		bytes += bytearray("INANA", "ascii")
		
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
		
		
		if os.path.isdir(self.filename):
			shutil.rmtree(self.filename)
		removeFile(self.filename + ".wav")
		removeFile(self.filename + "2.wav")
		removeFile(self.filename + "3.wav")
		
		
		pass
	
# 

# folder containing audio files.
audioFolder = "../ExportData/Exported_Sounds/audiogroup_music/"

# path to code.
codeFolder = "../../code/src/"
	
def doFile(filename):

	

	print(WHITE + "converting {:s}".format(filename) + RESET)
	
	
	"""
	# SOX might generate smaller files????
	# figure it out
	
	# ffmpeg -i msc_voidsong.ogg -ac 1 -af 'aresample=18157' -strict unofficial -c:a gsm test2.gsm
	# gsm is smaller
	# can the gsm format be split? can it be used as a sample?
	
	# also, multiple segs like, might be bad for filesize
	
	# ffmpeg -i .\tempOutput.gsm -t 00:05:10.58 tempOutputFromGsm.wav
	# seems to,,, work,,,??? smaller filesize, pitchshift down tho
	"""
	
	
	
	# after all this, we set the pitch up by a factor of 2 ingame.
	# having a lower max pitch lowers the filesize, and,,, ugh 
	# it seems to have some sacrifice of quality though
	# but ill try to get filewriting actually working first though
	
	#exit(0)
	# SET SAMPLE RATE INSIDE OPENMPT TO 18157
	
	#removeFile("tempOutput.wav")

	inputPath = os.path.join(audioFolder, filename)
	
	test = ITFile(inputPath)
	test.write()
	
	print(WHITE + "done converting {:s}".format(filename) + RESET)
	
	pass

def getNeededFiles():

	
	
	codeFiles = [os.path.join(codeFolder, f) for f in os.listdir(codeFolder) if f.lower().endswith('.h') or f.lower().endswith('.cpp')]
	
	codeFiles.remove("../../code/src/dataWinIncludes.h")
	
	refs = set()
	
	# NOT ALL SOUNDS START WITH SND
	#pattern1 = r'bn::music_items::\w+.play\(\)\;'		
	pattern1 = r'bn::music_items::\w+'		
	
	for file in codeFiles:
		with open(file, 'r', encoding='utf-8', errors='ignore') as f:
			content = f.read()
			#matches = [ s.strip()[len("bn::music_items::"):-8] for s in re.findall(pattern1, content) ]
			matches = [ s.strip()[len("bn::music_items::"):] for s in re.findall(pattern1, content) ]
			refs.update(matches)
		
	#refs.add("metal_pipe_falling_sound_effect")
	#refs.add("egg")
	#refs.add("void_stranger_ost_56")

	#if os.path.basename(filename) == "cifdream":
	#	return
	
	refs.remove("cifdream")

	#refs = ["msc_001"]
	
	return refs
	
def copyNeededMusic():

	destPath = "../../code/audio/"

	copyIfChanged("cifdream.xm", destPath)
	
	files = getNeededFiles()
	
	for file in files:
		if not os.path.isfile(os.path.join("./formattedOutput/", file + ".it")):
			doFile(file)
		res = copyIfChanged(os.path.join("./formattedOutput/", file + ".it"), destPath)
		if res:
			print(CYAN + "copied over {:s}".format(file) + RESET)


	pass
	
def convertAllFiles():

	os.chdir(os.path.dirname(__file__))
	
	if len(sys.argv) != 1:
		copyNeededMusic()
		exit(0)
	
	if os.path.exists("./formattedOutput"):
		shutil.rmtree("./formattedOutput")
		
	os.mkdir("./formattedOutput/")
	
	files = getNeededFiles()
	#print(files)
	#exit(1)
	#files = ["msc_voidsong.ogg", "msc_013.ogg"]
	#files = ["msc_013.ogg"]
	#files = ["msc_voidsong.ogg"]
	
	
	for file in files:
		doFile(file)
		
	#if os.path.isdir("tempFiles"):
	#	shutil.rmtree("tempFiles")
	#createFolder("tempFiles")

	copyNeededMusic()

	pass

if __name__ == "__main__":
	
	convertAllFiles()
	
	pass




