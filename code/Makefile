#---------------------------------------------------------------------------------------------------------------------
# TARGET is the name of the output.
# BUILD is the directory where object files & intermediate files will be placed.
# LIBBUTANO is the main directory of butano library (https://github.com/GValiente/butano).
# PYTHON is the path to the python interpreter.
# SOURCES is a list of directories containing source code.
# INCLUDES is a list of directories containing extra header files.
# DATA is a list of directories containing binary data.
# GRAPHICS is a list of directories containing files to be processed by grit.
# AUDIO is a list of directories containing files to be processed by mmutil.
# DMGAUDIO is a list of directories containing files to be processed by mod2gbt and s3m2gbt.
# ROMTITLE is a uppercase ASCII, max 12 characters text string containing the output ROM title.
# ROMCODE is a uppercase ASCII, max 4 characters text string containing the output ROM code.
# USERFLAGS is a list of additional compiler flags:
#     Pass -flto to enable link-time optimization.
#     Pass -O0 or -Og to try to make debugging work.
# USERASFLAGS is a list of additional assembler flags.
# USERLDFLAGS is a list of additional linker flags:
#     Pass -flto=<number_of_cpu_cores> to enable parallel link-time optimization.
# USERLIBDIRS is a list of additional directories containing libraries.
#     Each libraries directory must contains include and lib subdirectories.
# USERLIBS is a list of additional libraries to link with the project.
# DEFAULTLIBS links standard system libraries when it is not empty.
# USERBUILD is a list of additional directories to remove when cleaning the project.
# EXTTOOL is an optional command executed before processing audio, graphics and code files.
#
# All directories are specified relative to the project directory where the makefile is found.
#---------------------------------------------------------------------------------------------------------------------
#TARGET      :=  $(notdir $(CURDIR))
TARGET      :=  GBAStranger
BUILD       :=  build
LIBBUTANO   :=  ../butano/butano/
PYTHON      :=  python
SOURCES     :=  src ../butano/common/src
INCLUDES    :=  include ../butano/common/include ./build ../butano/butano/src/ ../butano/butano/hw/src/ ../butano/butano/hw/include/ ../butano/butano/hw/3rd_party/libugba/include/ ../butano/butano/hw/3rd_party/libugba/src/
DATA        :=
GRAPHICS    :=  graphics ../butano/common/graphics
AUDIO       :=  audio ../butano/common/audio
DMGAUDIO    :=  dmg_audio ../butano/common/dmg_audio
ROMTITLE    :=  GBAStranger
ROMCODE     :=  GBST

#USERFLAGS   := -save-temps=cwd -fverbose-asm -mcpu=arm7tdmi -O3 -mthumb-interwork -mthumb -Wall -Wextra -DBN_CFG_AUDIO_MAX_MUSIC_CHANNELS=32 -DBN_CFG_AUDIO_MIXING_RATE=BN_AUDIO_MIXING_RATE_31_KHZ
USERFLAGS   := -mcpu=arm7tdmi -O3 -mthumb-interwork -mthumb -Wall -Wextra -DBN_CFG_AUDIO_MAX_MUSIC_CHANNELS=32 -DBN_CFG_AUDIO_MIXING_RATE=BN_AUDIO_MIXING_RATE_31_KHZ
#USERFLAGS   := -fdump-tree-cfg=cfg.cfg -mcpu=arm7tdmi -O3 -mthumb-interwork -mthumb -Wall -Wextra -DBN_CFG_AUDIO_MAX_MUSIC_CHANNELS=32 -DBN_CFG_AUDIO_MIXING_RATE=BN_AUDIO_MIXING_RATE_31_KHZ

# valgrind --tool=callgrind ./mmutil TESTS/audio/cifdream.xm TESTS/audio/dancing_lesson.it TESTS/audio/egg.wav TESTS/audio/metal_pipe_falling_sound_effect.wav TESTS/audio/msc_001.it TESTS/audio/msc_007.it TESTS/audio/msc_013.it TESTS/audio/msc_beecircle.it TESTS/audio/msc_beesong.it TESTS/audio/msc_cifcircle.it TESTS/audio/msc_dungeon_wings.it TESTS/audio/msc_dungeongroove.it TESTS/audio/msc_endless.it TESTS/audio/msc_gorcircle_lo.it TESTS/audio/msc_levcircle.it TESTS/audio/msc_monstrail.it TESTS/audio/msc_themeofcif.it TESTS/audio/msc_voidsong.it TESTS/audio/snd_activate.wav TESTS/audio/snd_beginningbell.wav TESTS/audio/snd_bounce.wav TESTS/audio/snd_bounceimpact.wav TESTS/audio/snd_cifer.wav TESTS/audio/snd_enemy_explosion.wav TESTS/audio/snd_ex_heartbeat.wav TESTS/audio/snd_ex_heartbeat_b.wav TESTS/audio/snd_fall.wav TESTS/audio/snd_gestaltcreate.wav TESTS/audio/snd_golden.wav TESTS/audio/snd_gor.wav TESTS/audio/snd_judgment.wav TESTS/audio/snd_lillie.wav TESTS/audio/snd_lockdamage.wav TESTS/audio/snd_open.wav TESTS/audio/snd_player_damage.wav TESTS/audio/snd_player_fall.wav TESTS/audio/snd_push.wav TESTS/audio/snd_push_small.wav TESTS/audio/snd_resurrect.wav TESTS/audio/snd_reveal.wav TESTS/audio/snd_stairs.wav TESTS/audio/snd_stepglassfloor.wav TESTS/audio/snd_vanish.wav TESTS/audio/snd_voice2.wav TESTS/audio/snd_voidrod_place.wav TESTS/audio/snd_voidrod_store.wav TESTS/audio/snd_wingspawn.wav TESTS/audio/void_stranger_ost_56.wav -oTESTS/build/_bn_audio_soundbank.bin -hTESTS/build/_bn_audio_soundbank.h

# .\mmutil  TESTS/audio/msc_voidsong.it TESTS/audio/snd_activate.wav -o./TESTS/build/_bn_audio_soundbank.bin -h./TESTS/build/_bn_audio_soundbank.h

# ..\mmutil  audio/msc_voidsong.it audio/snd_activate.wav -obuild/_bn_audio_soundbank.bin -hbuild/_bn_audio_soundbank.h
# ..\mmutil  audio/msc_voidsong.it audio/snd_activate.wav -o"build/_bn_audio_soundbank.bin" -h"build/_bn_audio_soundbank.h"

USERASFLAGS :=
USERLDFLAGS :=  -flto=auto
USERLIBDIRS :=
USERLIBS    :=
# this is here to link to std::functional, which i need for only one very stupid use case
# as for if this thing actually links to the standard lib(as in runs this comp flag) or if it
# just includes defaultlib, bc its not empty, i do not know
# maybe this will fix my previous issues with the WTF strlen func
DEFAULTLIBS :=  -static-libstdc++
USERBUILD   :=
EXTTOOL     :=  python  ../misc/massFormatter/massFormatter.py regenIncludes; python  ../misc/soundConverter/soundConverter.py regenIncludes; python  ../misc/musicConverter/musicConverter.py regenIncludes


ifdef DISABLEASSERTS
	USERFLAGS += -DDISABLEASSERTS=true
endif

USERFLAGS += -DGITCOMMIT=\"$(shell git describe --always --dirty | sed 's/dirty/d/')\"

# vine boom soound effect
USERFLAGS += -DENABLELOGGING=true

ifdef LOG
	USERFLAGS += -DENABLELOGGING=true
endif

ifdef PROFILE
    USERFLAGS += -DENABLELOGGING=true -DENABLEPROFILER=true
endif

#---------------------------------------------------------------------------------------------------------------------
# Export absolute butano path:
#---------------------------------------------------------------------------------------------------------------------
ifndef LIBBUTANOABS
	export LIBBUTANOABS	:=	$(realpath $(LIBBUTANO))
endif



#---------------------------------------------------------------------------------------------------------------------
# Include main makefile:
#---------------------------------------------------------------------------------------------------------------------
include $(LIBBUTANOABS)/butano.mak

# gives a warning, idc tho
fullClean:
	rm -fr build GBAStranger.elf GBAStranger.elf.* GBAStranger.gba
	rm -fr audio graphics

	rm -fr ./src/AllRooms.h
	rm -fr ./src/fontData.h
	rm -fr ./src/dataWinIncludes.h

	rm -fr ../misc/massFormatter/formattedOutput
	rm -fr ../misc/musicConverter/formattedOutput
	rm -fr ../misc/soundConverter/formattedOutput
	rm -fr ../misc/roomConverter/AllRooms.h

	rm -fr ../misc/ExportData
	rm -fr ../misc/audiogroup1.dat
	rm -fr ../misc/audiogroup2.dat
	rm -fr ../misc/data.win

	rm -fr ../misc/CLI-windows-latest-Release-isBundled-true

	python  ../misc/convertAllAssets.py
