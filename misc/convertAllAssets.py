
import importlib.util
import sys, os
import subprocess


def install(package):

	if type(package) != str:
		installName = package[1]
		package = package[0]
	else:
		installName = package

	if (spec := importlib.util.find_spec(package)) is not None:
		# If you choose to perform the actual import ...
		module = importlib.util.module_from_spec(spec)
		sys.modules[package] = module
		spec.loader.exec_module(module)
	else:
		print("installing", package)
		subprocess.check_call([sys.executable, "-m", "pip", "install", installName])


if __name__ == "__main__":
	[ install(p) for p in ["numpy", ("PIL", "Pillow"), "colorama", "pydub", "requests", "zipfile", ("win32api", "pywin32")] ]

	try:
		res = subprocess.check_output(['which', 'ffmpeg'], stderr=subprocess.STDOUT)
	except:
		print("please install ffmpeg")
		exit(0)


import winreg
import win32api
import requests
import zipfile
import shutil

def read_reg(ep, p = r"", k = ''):
	try:
		key = winreg.OpenKeyEx(ep, p)
		value = winreg.QueryValueEx(key,k)
		if key:
			winreg.CloseKey(key)
		return value[0]
	except Exception as e:
		return None
	return None

if __name__ == "__main__":

	# https://github.com/krzys-h/UndertaleModTool/releases/download/bleeding-edge/CLI-windows-latest-Release-isBundled-true.zip
	
	"""

	going to need to grab:
	undertalemodtool cli 
	data.win, and the audio dats, which means i need the steam dir.
	
	
	"""
	
	if not os.path.exists("CLI-windows-latest-Release-isBundled-true"):
		
		print("couldnt find the undertalemodtool's cli, grabbing it.")
	
		res = requests.get("https://github.com/krzys-h/UndertaleModTool/releases/download/bleeding-edge/CLI-windows-latest-Release-isBundled-true.zip")
		if res.status_code == 200:
			with open("CLI-windows-latest-Release-isBundled-true.zip", 'wb') as f:
				f.write(res.content)
				print("got it")
		else:
			print("download failed, we got a", res.status_code)
			exit(1)
				
		with zipfile.ZipFile("CLI-windows-latest-Release-isBundled-true.zip", 'r') as zip_ref:
			zip_ref.extractall("CLI-windows-latest-Release-isBundled-true/")
		
		os.remove("CLI-windows-latest-Release-isBundled-true.zip")
	
	
	if not (os.path.isfile("data.win") and os.path.isfile("audiogroup1.dat") and os.path.isfile("audiogroup2.dat")):
		print("game datafiles not found, grabbing a copy.")
		
		print("trying my bests to find ur steam dir.")
		
		steamInstallPath = str(read_reg(ep = winreg.HKEY_LOCAL_MACHINE, p = r"SOFTWARE\Wow6432Node\Valve\Steam", k = 'InstallPath'))
		
		if steamInstallPath is None:
			print("despite my best(mediocore) efforts, i couldnt find your steam path. hand it over")
			print("make sure to only have forward slashes (/) not backslashes (\\) in the path")
		
			while True:
				steamInstallPath = input(">>>")
				
				if os.path.exists(os.path.join(steamInstallPath, "steamapps")):
					break
					
				print("sorry, this should be the path that has steam.exe in it")
		
		f = open(os.path.join(steamInstallPath, "steamapps/libraryfolders.vdf"), "r")
		lines = [ line.strip() for line in f.readlines() ]
		f.close()
		
		searchPaths = []
		for line in lines:
			if "path" in line:
			
				temp = line.split("\t")[2].replace("\\\\", "/")[1:-1] + "/"
				searchPaths.append(temp)
		
		
		for path in searchPaths:
	
			testPath = os.path.join(path, "steamapps/common/Void Stranger/")
			
			if os.path.exists(testPath):
				print("finally, found the void stranger path")
				
				shutil.copy(os.path.join(testPath, "data.win"), "./")
				shutil.copy(os.path.join(testPath, "audiogroup1.dat"), "./")
				shutil.copy(os.path.join(testPath, "audiogroup2.dat"), "./")
				break
		else:
			print("couldnt find the void stranger data.win, and audiogroup files.")
			print("please just copy them in manually, and then rerun")
			exit(1)
		
		
		if os.path.exists("./ExportData"):
			shutil.rmtree("./ExportData")
		
		print("extracting assets from data.win")
		
		# ./CLI-windows-latest-Release-isBundled-true/UndertaleModCli.exe load data.win -s Scripts/ExportAllCode.csx Scripts/ExportAllRooms.csx Scripts/ExportAllSounds.csx Scripts/ExportAllSpritesWithPadding.csx Scripts/ExportAllTexturesGrouped.csx Scripts/ExportFontData.csx
		
		command = "./CLI-windows-latest-Release-isBundled-true/UndertaleModCli.exe load data.win -s Scripts/ExportAllCode.csx Scripts/ExportAllRooms.csx Scripts/ExportAllSounds.csx Scripts/ExportAllSpritesWithPadding.csx Scripts/ExportAllTexturesGrouped.csx Scripts/ExportFontData.csx"
		
		res = subprocess.check_call(command.split(" "))
		
		if res != 0:
			print("the undertalemodtool command crashed. this shouldnt happen(i hope)")
			exit(1)
			

	


	import roomConverter.roomConverter 
	import massFormatter.massFormatter
	import soundConverter.soundConverter 

	
	print("running massFormatter")
	massFormatter.massFormatter.main()
	print("running roomConverter")
	roomConverter.roomConverter.main()
	print("running soundConverter")
	soundConverter.soundConverter.main()
	
	
	print("")
	print("if you are seeing this, then everything(suprisingly) ran correctly.")
	
	# i rlly should include some form of cleanu after all the functions copy to the code area
	
	pass