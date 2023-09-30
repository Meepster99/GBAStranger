
from pydub import AudioSegment
import os


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




	pass

if __name__ == "__main__":


	# makesure that audiogroup1.dat and audiogroup2.dat are next to data.win
	# run exportallsounds 
	# move that folder here

	convertAllMusic()
	convertAllSounds()
	
	pass