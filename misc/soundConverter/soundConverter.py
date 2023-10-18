
from pydub import AudioSegment
import os
import shutil

outputPath = "./formattedOutput/"

def convertAllMusic():

	# msc_013.ogg	
	
	inputPath = "./Exported_Sounds/audiogroup_music"
	
	file = "msc_013.ogg"
	
	inputFilePath = os.path.join(inputPath, file)
	
	song = AudioSegment.from_ogg(inputFilePath)
	
	song = song.set_channels(1)
	
	#song = song[:30*1000]
	
	#song = song.compress(2)


	outputFilePath = os.path.join(outputPath, file.rsplit(".", 1)[0] + ".wav")
	song.export(outputFilePath, format="wav", parameters=["-ar","4000"])
	
	pass

def convertAllSounds():

	inputPath = "../ExportData/Exported_Sounds/audiogroup_soundeffects/"

	wavFiles = [f for f in os.listdir(inputPath) if f.lower().endswith('.wav')]
	
	successCount = 0
	failCount = 0
	for i, wav in enumerate(wavFiles):
	
		print("converting soundfile {:50s}, {:5d} out of {:5d}".format(wav, i, len(wavFiles)))
	
		song = AudioSegment.from_wav(os.path.join(inputPath, wav))
		
		if song.duration_seconds > 5:
			print("{:50s} is to long, skipping".format(wav))
			
			failCount += 1
			continue
		
		song = song.set_channels(1)
	
		outputFilePath = os.path.join(outputPath, wav.rsplit(".", 1)[0] + ".wav")
		#song.export(outputFilePath, format="wav", parameters=["-ar","22050"])
		song.export(outputFilePath, format="wav", parameters=["-ar","44100"])

		
		successCount += 1

	totalCount = successCount + failCount
	print("converted {:5.2f}% ({:d}/{:d}) of sound effects, hope thats good enough".format(100 * successCount / totalCount, successCount, totalCount))
		
	pass

def main():

	os.chdir(os.path.dirname(__file__))

	shutil.rmtree("./formattedOutput")
	
	if not os.path.exists("./formattedOutput"):
		os.mkdir("./formattedOutput/")

	# makesure that audiogroup1.dat and audiogroup2.dat are next to data.win
	# run exportallsounds 
	# move that folder here

	#convertAllMusic()
	convertAllSounds()
	
	[ os.remove(os.path.join("../../code/audio/", f)) for f in os.listdir("../../code/audio/") if f.endswith(".wav") ]
	copyFunc = lambda copyFrom : [ shutil.copy(os.path.join(copyFrom, f), os.path.join("../../code/audio/", f)) for f in os.listdir(copyFrom) if f.endswith(".wav") ]
	copyFunc("./formattedOutput/")
	
	pass
	
if __name__ == "__main__":
	main()
	
