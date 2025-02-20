
import importlib.util
import sys, os
import platform
import subprocess
import traceback
from colorama import init, Fore, Back, Style
RED = Fore.RED 
GREEN = Fore.GREEN 
CYAN = Fore.CYAN
WHITE = Fore.WHITE
RESET = Style.RESET_ALL
import requests
import zipfile
import shutil

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


def installpackages_win():
	[ install(p) for p in ["numpy", ("PIL", "Pillow"), "colorama", "pydub", "requests", ("win32api", "pywin32"), "pytube"] ]

	try:
		res = subprocess.check_output(['ffmpeg', '-version'], stderr=subprocess.STDOUT)
	except:
		print("please install ffmpeg")
		exit(0)
		
	import winreg
	import win32api

	init(convert=True)

def read_reg(ep, p = r"", k = ''):
	import winreg
	import win32api
	try:
		key = winreg.OpenKeyEx(ep, p)
		value = winreg.QueryValueEx(key,k)
		if key:
			winreg.CloseKey(key)
		return value[0]
	except Exception as e:
		return None
	return None

def install_win():
	installpackages_win()
	# https://github.com/krzys-h/UndertaleModTool/releases/download/bleeding-edge/CLI-windows-latest-Release-isBundled-true.zip
	
	"""

	going to need to grab:
	undertalemodtool cli 
	data.win, and the audio dats, which means i need the steam dir.
	
	
	"""
	
	os.chdir(os.path.dirname(__file__))
	if not os.path.exists("CLI-windows-latest-Release-isBundled-true"): 
		print("couldnt find the undertalemodtool's cli, grabbing it.")
	
		# what kind of fucking idiot runs things literally off the bleeding edge??
		#res = requests.get("https://github.com/krzys-h/UndertaleModTool/releases/download/bleeding-edge/CLI-windows-latest-Release-isBundled-true.zip")
		# also, butanos on HEAD detached from edb6de37,,,, but the github is on commit 97213e3???
		
		
		#UMTURL = "https://github.com/krzys-h/UndertaleModTool/releases/download/0.5.0.0/v0.5.0.0_UndertaleModToolCLI_Windows.zip"
		#UMTURL = "https://github.com/UnderminersTeam/UndertaleModTool/releases/download/0.5.1.0/UndertaleModTool_v0.5.1.0.zip"
		#UMTURL = "https://github.com/UnderminersTeam/UndertaleModTool/releases/download/bleeding-edge/CLI-windows-latest-Release-isBundled-true.zip"
		#UMTURL = "https://github.com/UnderminersTeam/UndertaleModTool/releases/download/bleeding-edge/CLI-windows-latest-Debug-isBundled-true.zip"
		
		UMTURL = "https://github.com/UnderminersTeam/UndertaleModTool/releases/download/bleeding-edge/CLI-windows-latest-Debug-isBundled-true.zip"
		
		# wowee maddy its almost as if pulling from bleeding edge constantly would cause bugs what?? who would have thought??
		# there was some fucked shit in the past where,,, god the debug/release or single/multifile would cause critical differences and fuck everything right?
		# clone the repo, checkout 438fc2143b13123b846d45cb03e5f84e5e9f884d
		# dotnet publish UndertaleModCli --configuration=Debug 
		# do that in case my site goes down for some reason, you know what to do. i really should host this,, somwewhere else? im not sure
		#UMTURL = "https://meepster.xyz/CLI-windows-latest-Release-isBundled-true.zip"
		
		res = requests.get(UMTURL)
		
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
		import winreg
		import win32api
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
		
		
	if not os.path.exists("./ExportData"):
		print("extracting assets from data.win")
		
		command = "./CLI-windows-latest-Release-isBundled-true/UndertaleModCli.exe load data.win -v -s Scripts/ExportAllCode.csx Scripts/ExportAllRooms.csx Scripts/ExportAllSounds.csx Scripts/ExportAllSpritesWithPadding.csx Scripts/ExportAllTexturesGrouped.csx Scripts/ExportFontData.csx"
		
		# making the command verbose prevents the crash. WHY

		try:
			res = subprocess.check_call(command.split(" "))
		except Exception as e:
			print(e)
			
			print(traceback.format_exc())
			
			print("HEY, SOMETHINGS MESSED UP")
			print(f"command: \"{command}\"")
			exit(1)
		
		
		if res != 0:
			print("the undertalemodtool command crashed. this shouldnt happen(i hope)")
			exit(1)

	print("initial setup completed")

	import roomConverter.roomConverter 
	import massFormatter.massFormatter
	import soundConverter.soundConverter 
	import musicConverter.musicConverter
	
	print("running massFormatter")
	massFormatter.massFormatter.main()
	print("running roomConverter")
	roomConverter.roomConverter.main()
	print("running soundConverter")
	soundConverter.soundConverter.main()
	print("running musicConverter")
	musicConverter.musicConverter.main()
	
	print("")
	print(CYAN + "if you are seeing this, then everything(suprisingly) ran correctly." + RESET)

def install_mac():
	print(RED + f"Sorry, your platform ({platform.platform()}) is not currently supported." + RESET);
	sys.exit()

def install_linux():
	print(RED + f"Sorry, your platform ({platform.platform()}) is not currently supported." + RESET);
	sys.exit()

if __name__ == "__main__":
	if platform.system() == "Windows":
		install_win()
	elif platform.system() == "Darwin":
		install_mac()
	elif platform.system() != "":
		install_linux()