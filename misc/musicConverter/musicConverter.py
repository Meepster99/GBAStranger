
import os, sys
import shutil
import subprocess
import re
import hashlib
import struct
import json
import wave
import math


from colorama import init, Fore, Back, Style

init(convert=True)

RED = Fore.RED 
GREEN = Fore.GREEN 
CYAN = Fore.CYAN
WHITE = Fore.WHITE

RESET = Style.RESET_ALL

"""

mission objective: read a file, edit a file, write a () file.
there is like,, no documentation on the IT format tho, and gods the pattern table is one thing, but having 
to adjust the number of samples??
https://github.com/pr2502/ittech maybe its time to rust?
nvm the writer isnt implimented yet
im going to have to do this on my own. fuck 

this is the best thing i could find 

https://fileformats.fandom.com/wiki/Impulse_tracker

BASICALLY ALL CREDIT TO:
https://github.com/ramen/pytrax/blob/master/pytrax/impulsetracker.py 
will also be helpful




"""

#

def runCommand(cmd):

	res = subprocess.run(cmd)
		
	if res.returncode != 0:
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

# gods this took forever 


baseLength = 8 # wtf is this 
defWidth = 17

lowerTab= [0, -1, -3, -7, -15, -31, -56, -120, -248, -504, -1016, -2040, -4088, -8184, -16376, -32760, -32768];
upperTab = [0, 1, 3, 7, 15, 31, 55, 119, 247, 503, 1015, 2039, 4087, 8183, 16375, 32759, 32767];
fetchA = 4;
lowerB = -8;
upperB = 7;
#defWidth = 17;
mask = 0xFFFF;
ITWidthChangeSize= [ 4, 5, 6, 7, 8, 9, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 ];

bwt = []

packedLength = 0
packedTotalLength = 0
packedData = []
bufferSize =  2 + 0xFFFF
remBits = 0
bitPos = 0
byteVal = 0

def deltafy(src):
	
	prevVal = 0
	
	for i in range(0, len(src)):
		newVal = src[i]
		src[i] = newVal - prevVal
		prevVal = newVal

def WriteBits(width, v):
	global remBits
	global bitPos
	global byteVal 
	while(width > remBits):
		byteVal |= (v << bitPos);
		width -= remBits;
		v >>= remBits;
		bitPos = 0;
		remBits = 8;
		WriteByte(byteVal);
		byteVal = 0;
	

	if(width > 0):
		byteVal |= ((v & ((1 << width) - 1)) << bitPos);
		remBits -= width;
		bitPos += width;
	   
def WriteByte(v):
	global packedLength
	global packedData
	if(packedLength < bufferSize):
		packedData[packedLength] = (v & 0xFF);
		packedLength += 1
	else:
		print("loook,,, this should like never happen")
		exit(1)
	
def GetWidthChangeSize(w, is16):
	wcs = ITWidthChangeSize[w - 1];
	if(w <= 6 and is16):
		wcs+=1;
	return wcs;
	
def ConvertWidth(curWidth, newWidth):
	curWidth-=1;
	newWidth-=1;
	#MPT_ASSERT(newWidth != curWidth);
	if(newWidth > curWidth):
		newWidth-=1;
	return newWidth;

def SquishRecurse(sWidth, lWidth, rWidth, width, offset, length, src):
	global bwt
	global defWidth
	global baseLength

	
	if width + 1 < 1:
		#for(SmpLength i = offset; i < offset + length; i++):
		for i in range(offset, offset+length):
			bwt[i] = sWidth;
		return;
	

	#MPT_ASSERT(width >= 0 && static_cast<unsigned int>(width) < std::size(Properties::lowerTab));

	i = offset;
	end = offset + length;

	while(i < end):
		if(src[i] >= lowerTab[width] and src[i] <= upperTab[width]):
			start = i;
			#// Check for how long we can keep this bit width
			while(i < end and src[i] >= lowerTab[width] and src[i] <= upperTab[width]):
				i+=1;

			blockLength = i - start;
			#xlwidth = start == offset ? lWidth : sWidth;
			xlwidth = lWidth if start == offset else sWidth;
			#xrwidth = i == end ? rWidth : sWidth;
			xrwidth = rWidth if i == end else sWidth;

			#is16 = sizeof(typename sample_t) > 1;
			is16 = True
			wcsl = GetWidthChangeSize(xlwidth, is16);
			wcss = GetWidthChangeSize(sWidth, is16);
			wcsw = GetWidthChangeSize(width + 1, is16);

			comparison = False;
			if(i == baseLength):
				keepDown = wcsl + (width + 1) * blockLength;
				levelLeft = wcsl + sWidth * blockLength;

				if(xlwidth == sWidth):
					levelLeft -= wcsl;

				comparison = (keepDown <= levelLeft);
			else:
				keepDown = wcsl + (width + 1) * blockLength + wcsw;
				levelLeft = wcsl + sWidth * blockLength + wcss;

				if(xlwidth == sWidth):
					levelLeft -= wcsl;
				if(xrwidth == sWidth):
					levelLeft -= wcss;

				comparison = (keepDown <= levelLeft);
			#SquishRecurse(comparison ? (width + 1) : sWidth, xlwidth, xrwidth, width - 1, start, blockLength, sampleData);
			
			temp = width + 1 if comparison else sWidth
			SquishRecurse(temp, xlwidth, xrwidth, width - 1, start, blockLength, src);
		else:
			bwt[i] = sWidth;
			i+=1;

def CompressBlock(data, offset, actualLength):

	global bwt
	global defWidth
	global baseLength

	# CopySample<typename Properties::sample_t>(sampleData, data, offset, baseLength, mptSample.GetNumChannels());
	# void ITCompression::CopySample(T *target, const T *source, SmpLength offset, SmpLength length, SmpLength skip)

	blockSize = 0x8000
	baseLength = min(actualLength, blockSize // 2);
	
	
	bwt = [defWidth] * baseLength

	sampleData = [ data[i] for i in range(offset, offset+baseLength)]

	deltafy(sampleData)
	#deltafy(sampleData)

	SquishRecurse(defWidth, defWidth, defWidth, defWidth - 2, 0, baseLength, sampleData);	   

	width = defWidth
	for i in range(0, baseLength):
		if bwt[i] != width:
			if width <= 6:
				#// Mode A: 1 to 6 bits
				#MPT_ASSERT(width);
				WriteBits(width, (1 << (width - 1)));
				WriteBits(fetchA, ConvertWidth(width, bwt[i]));
			elif width < defWidth:
				#// Mode B: 7 to 8 / 16 bits
				xv = (1 << (width - 1)) + lowerB + ConvertWidth(width, bwt[i]);
				WriteBits(width, xv);
			else:
				#// Mode C: 9 / 17 bits
				#MPT_ASSERT((bwt[i] - 1) >= 0);
				WriteBits(width, (1 << (width - 1)) + bwt[i] - 1);
			width = bwt[i];
		
		WriteBits(width, (sampleData[i]) & mask);
		
	WriteByte(byteVal);
	packedData[0] = ((packedLength - 2) & 0xFF);
	packedData[1] = ((packedLength - 2) >> 8);

def Compress(mptSampleData, maxLength):
	
	global packedLength 
	global packedTotalLength
	global packedData
	global byteVal 
	global remBits 
	global bitPos
	
	packedData = [0] * bufferSize
	
	res = bytearray()

	packedTotalLength = 0
	offset = 0;
	remain = maxLength;
	while(remain > 0):
		#// Initialise output buffer and bit writer positions
		packedLength = 2;
		bitPos = 0;
		remBits = 8;
		byteVal = 0;

		CompressBlock(mptSampleData, offset, remain)

		#if(file) mpt::IO::WriteRaw(*file, packedData.data(), packedLength);
		
		# why??? why,, why?? why?????
		for i in range(0, packedLength):
			if packedData[i] > 0xFF or packedData[i] < 0:
				print("wtf")
				exit(1)
			res.append(packedData[i])
		
		
		packedTotalLength += packedLength;

		offset += baseLength;
		remain -= baseLength;
		
	packedData = [0]
	
	return res

#Compress(src_samples, len(src_samples))
		
# 

#GBASAMPLERATE = 9000
#GBASAMPLERATE = 24000

# 31k might,, be a bit to high for the gba to handle? it started having some static "blips" occasionally
GBASAMPLERATE = 31000

# 27k still has,, some,, but less(i think?) 
# but is it worth the quality drop?
#GBASAMPLERATE = 27000

class ITFile:

	def __init__(self, filename):
	
		self.filename = filename
		
		# not efficient
		#self.bytes = []
		
		#self.samples = []
		
		#self.f = None
		
		shutil.rmtree("tempFiles")
		createFolder("tempFiles")
		
		
		# TODO, REMOVE THIS STEP, NO POINT IN PIPING THROUGH GSM WHEN WE MURDER THE QUALITY ON OUR OWN LATER 
		
		# ffmpeg -i msc_voidsong.ogg -ac 1 -af 'aresample=18157' -strict unofficial -c:a gsm test2.gsm
		"""
		runCommand([
			'ffmpeg',
			"-y", '-i', filename,
			#"-ac", "1", "-af", "aresample=18157",
			#"-ac", "1", "-af", "aresample=8000",
			"-ac", "1", "-af", "aresample=18000",
			"-strict", "unofficial", "-c:a", "gsm",
			"tempOutput.gsm"
			])
		"""
		
		# looking back on things,,, does the bs im doing with freq changing even help at all??
		
		runCommand([
			'ffmpeg',
			"-y", '-i', filename,
			#"-ac", "1", "-af", "aresample=18157",
			#"-ac", "1", "-af", "aresample=8000",
			#"-ac", "1", "-af", "aresample=18000",
			#"-ac", "1", "-af", "aresample={:d}".format(GBASAMPLERATE*2),
			#"-ac", "1", "-af", "aresample={:d}".format(GBASAMPLERATE),
			"tempOutput.wav"
			])
		
		# does,,, lowing the shit and playing it at 2x speed actually help my filesize at all???
		# increaseing the duration also like,,, fucks me in terms of sample count 
		
		
		
		
		
		#print(self.duration)
		#print(self.tickValue)
		#print(self.segmentTime)
		#exit(1)
		
		#print(self.duration)
		#exit(1)
			
		# gods 
		# if i downsample to 16000 instead of 18157 then,,,,, '
		# i can pitch up by a factor of 2 i butano,,,, which will also reduce length by 2,,,,
			
		
		# fixes pitch, cost filesize?
		# there doesnt seem to be a good way around this.
		# ffmpeg -i .\tempOutput.gsm -t 00:05:10.58 -filter:a "asetrate=18157" tempOutputFromGsm.wav
		
		#speed_factor = 18157 / 8000
		#speed_factor = 2
		# ffmpeg -i .\tempOutput.gsm -t 00:05:10.58 tempOutputFromGsm.wav
		"""
		runCommand([
			'ffmpeg',
			#"-y", '-i', "tempOutput.gsm",
			"-y", '-i', "tempOutput.wav",
			#"-t", "00:05:10.58",
			#"-t", "155.29",
			#'-filter:a', f'atempo={speed_factor}',
			#'-filter:a', 'asetrate=9000', 
			'-filter:a', 'asetrate={:d}'.format(GBASAMPLERATE), 
			#"-af", "aresample=8000",
			"tempOutputFromGsm.wav"
			])
			
		runCommand([
			'ffmpeg',
			"-y", '-i', "tempOutputFromGsm.wav",
			#'-filter:a', f'atempo={speed_factor}',
			#'-filter_complex', f'[0:a]asetrate=8000*{speed_factor},atempo={speed_factor}[outa]',
			#'-filter_complex', f'[0:a]asetrate=8000*{speed_factor}[outa]',
			#'-filter_complex', f'[0:a]atempo={speed_factor}[outa]',
			#'-map', '[outa]',
			#"-af", "aresample=8000",
			"tempOutput.wav"
			])
		"""
		
		"""
		# ffmpeg -i test.wav -f segment -segment_time 2 -c copy tempFiles/output_%03d.wav
		runCommand([
			'ffmpeg',
			"-y", '-i', "tempOutput.wav",
			"-ac", "1", "-af", "aresample={:d}".format(GBASAMPLERATE),
			#"-f", "segment", "-segment_time", "2", 
			#"-f", "segment", "-segment_time", "4", 
			#"-f", "segment", "-segment_time", "3.5", 
			#"-f", "segment", "-segment_time", "3", 
			#"-f", "segment", "-segment_time", "2.5", # fucking edging getting to 0xFF, im at 248 on voided with this 
			#"-f", "segment", "-segment_time", "2.000",
			"-f", "segment", "-segment_time", "{:f}".format(self.segmentTime),
			#"-c", "copy", 
			#"-af", "aresample=18157",
			"tempFiles/output_%03d.wav"
			])
		"""
		
		runCommand([
			'ffmpeg',
			"-y", '-i', "tempOutput.wav",
			"-ac", "1", "-af", "aresample={:d}".format(GBASAMPLERATE),
			"tempOutput2.wav"
			])
		
		#ffmpeg -i input_audio.wav -filter_complex "[0:a]asplit=n=2000[S1][S2][S3]..." -map "[S1]" output1.wav -map "[S2]" output2.wav -map "[S3]" output3.wav
		# ffmpeg -i input_audio.wav -filter_complex "[0:a]asplit=n=2000[S]" -map "[S]" output_%03d.wav
		
		#filterParam = "[0:a]aresample={:d},asplit=33000[S]".format(GBASAMPLERATE)
		
		""""
		runCommand([
			'ffmpeg',
			"-y", '-i', "tempOutput2.wav",
			#"-ac", "1",# "-af", "aresample={:d}".format(GBASAMPLERATE),
			"-filter_complex", "[0:a]asplit=33000[S]", #"-map", "\"[S]\"",
			"tempFiles/output_%03d.wav"
			])
		"""
		
		waveFile = wave.open("tempOutput2.wav")
		#frameCount = waveFile.getnframes()
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
		
		#print(samplesPerSegment)
		
		#ffmpeg -i input.wav -filter_complex "[0]atrim=start=0:end=9999[a]; [0]atrim=start=10000:end=19999[b]; [0]atrim=start=20000:end=29999[c]; ..." -map "[a]" output_1.wav -map "[b]" output_2.wav -map "[c]" output_3.wav ...
		# do not ask.
		
		cmd = [
			"ffmpeg", "-i", "tempOutput2.wav", "-filter_complex"
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
			mapCmds.append("tempFiles/output_{:03d}.wav".format(index))
			
			index += 1
		
		#print(filterString)
		#filterString += "\""
		cmd.append("; ".join(filterString))
		
		cmd += mapCmds
		
		
		
		runCommand(cmd)
		
		#print(" ".join(cmd))
		#print(frameCount)
		
		#exit(1)
		
		
	
		
		
		removeFile("tempOutput.wav")
		removeFile("tempOutput2.wav")
		removeFile("tempOutput.gsm")
		removeFile("tempOutputFromGsm.wav")
		
			
		
		
		self.sampleFilenames = [os.path.join("./tempFiles", f) for f in os.listdir("./tempFiles") if os.path.isfile(os.path.join("./tempFiles", f))]
		
		self.sampleCount = len(self.sampleFilenames)
		
		if self.sampleCount > 255:
			print("max samplecount of 255 exceded")
			print("idk what to do dumbass, either split the song, increase the segment time, or fix shit")
			exit(1)
		#print(self.sampleFilenames)
		
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
			
		#exit(1)
		
		
		pass
	
	def writeSample(self, bytes, sampleFilename):
	
		offsetRes = len(bytes)
		
		
		# https://github.com/OpenMPT/openmpt/blob/47fc65b7b01455021284e3a5251ef16736bcd077/soundlib/ITCompression.cpp
		
		# i have like,,, no clue what to do here
		
		# this duplicate file open is stupid.
		wavFile = wave.open(sampleFilename)
		sampleCount = wavFile.getnframes() # once again, var name duplication 
		wavBytes = wavFile.readframes(sampleCount)
		wavFile.close()
		
		wavSamples = struct.unpack('<' + 'h' * (len(wavBytes) // 2), wavBytes)
		
		# https://github.com/OpenMPT/openmpt/blob/47fc65b7b01455021284e3a5251ef16736bcd077/soundlib/ITCompression.cpp
		# this is going to suck.
		# https://github.com/marmalade/tune4Airplay/blob/master/trunk/docs/ITTECH.TXT
	
		res = Compress(wavSamples, sampleCount)
		#Compress(wavBytes, len(wavBytes))
		
		
		bytes += res
		#bytes += wavBytes

		return offsetRes
	
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
		bytes += struct.pack("B", 0x0B) 
		
		# default vol 
		bytes += struct.pack("B", 64)
		
		# sample name 
		temp = bytearray(os.path.basename(sampleFilename).rsplit(".", 1)[0][:26], "ascii")
		while len(temp) < 26:
			temp.append(0)
		bytes += temp
	
		# convert flags
		bytes += struct.pack("B", 0x01)
		
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
		bytes += struct.pack("H", 73)
		
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

		
		f = open(self.filename.rsplit(".", 1)[0] + ".it", "wb")
		f.write(bytes)
		f.close()
		
		pass
	
# 
	
def doFile(filename):

	print("converting {:s}".format(filename))
	
	
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

	test = ITFile(filename)
	test.write()
	
	print("done converting {:s}".format(filename))
	
	pass

def getNeededFiles():

	res = []




	return res
	
def copyNeededMusic():




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
	#files = ["msc_voidsong.ogg", "msc_013.ogg"]
	#files = ["msc_013.ogg"]
	#files = ["msc_voidsong.ogg"]
	
	for file in files:
		doFile(file)
		
	shutil.rmtree("tempFiles")
	createFolder("tempFiles")



	pass

if __name__ == "__main__":
	
	convertAllFiles()
	
	pass




