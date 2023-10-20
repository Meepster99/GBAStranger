#pragma once 

// make with -DENABLEPROFILER to enable

#ifdef ENABLELOGGING
#include "bn_log.h"
#else 
#define BN_LOG(...) do {} while (false)
#endif

#include "bn_unordered_map.h"
#include "bn_timer.h"

#include "bn_deque.h"
#include "bn_fixed.h"
#include "bn_vector.h"


#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))


// IF WE HAVE A GLOBAL TIMER, AND STOP IT BEFORE AND AFTER THE INSTANTIATIONS
// THE TIMER WILL ACTUALLY FUNCTION

#define maxFuncLength 40

inline int WTF(const char* str) {
		
	// look.
	// i think, that when the compiler saw this func, it saw it was basically strlen, and tried 
	// to just sub that in, only for strlen to not exist for some unknown ungodly reasons
	// this is horrid.
	
	int length = 0;
	
	while (*str != '\0') {
		++length;
		++length;
		++str;
	} 
	return length >> 1;
}

inline const char* extractClassAndFunctionName(const char* prettyFunction) {
    const char* begin = prettyFunction;
    const char* end = prettyFunction + WTF(prettyFunction); 

    while (*begin != ' ' && begin < end) {
        ++begin;
    }
	++begin;

    while (*end != '(' && end > begin) {
        --end;
    }

	int length = end - begin;
	
	length = MIN(length, maxFuncLength);

  
    char* functionName = new char[length + 1];
    //std::strncpy(functionName, begin, length);
	for(int i=0; i<length; i++) {
		functionName[i] = begin[i];
	}
    functionName[length] = '\0'; 

    return functionName;
}

inline unsigned getHash(const char *str) {
    unsigned hash = 0;

    while (*str) {
        hash = (hash * 31) + (*str);
        str++;
    }

    return hash;
}

class BetterTimer {
public:

	// allow starting and stopping, just for ease of use. 
	
	BetterTimer() {}
	
	void start() {
		
		startTime = timer.elapsed_ticks();
		isRunningState = true;
	}
	
	void stop() {
		
		elapsed += timer.elapsed_ticks() - startTime;
		isRunningState = false;
	}
	
	unsigned getElapsed() const {
		BN_ASSERT(!isRunningState, "getElapsed was called on a timer while it was still running!");
		return elapsed;
	}
	
	bool isRunning() const {
		return isRunningState;
	}
	
	void reset() {
		elapsed = 0;
		isRunningState = false;
	}
	
private:
	// im going to be creating a bunch of timers, and tbh,,,, i just, am going to live with that 
	// they do that in the official profiler too.
	// actually ill just make it static
	// tbh all timers could just sharethe same memory tbh
	static bn::timer timer;
	
	unsigned elapsed = 0;
	unsigned startTime = 0;
	bool isRunningState = false;

};

struct ProfilerData { 

	uint64_t total = 0;
	unsigned int timesCalled = 0;
	unsigned int max = 0;
	
	BetterTimer timer = BetterTimer();

};

struct charHashPair {
	// i could just use a pair for this, but i dont wanna bc gods syntax annoying.
	const char* ID;
	unsigned hash;
};


#define MAXPROFILERFUNCS 256

namespace WTFPROFILER {
	
	extern BN_DATA_EWRAM bn::unordered_map<const char*, ProfilerData, MAXPROFILERFUNCS> profilerMap;
	extern BN_DATA_EWRAM bn::deque<charHashPair, MAXPROFILERFUNCS> profilerStack;

	
	void show();
	void start(const char* ID, const unsigned hash); 
	void stop(const char* ID, const unsigned hash);
	void reset();
}

class Profiler {
public:

	// should these vars be global or static?

	// this is from official butano code. but is it ok to put this pointer in there?
	
	// wtf is this 
	// https://gvaliente.github.io/butano/faq.html#faq_memory_types
	// im not even running out of memory BUT THIS FUCKING UNORDERED MAP DOESNT WANT TO FUCKING WORK

	//BN_DATA_EWRAM static bn::unordered_map<const char*, ProfilerData, MAXPROFILERFUNCS> profilerMap;
	
	//static bn::deque<charHashPair, MAXPROFILERFUNCS> profilerStack;
	//BN_DATA_EWRAM static bn::deque<charHashPair, MAXPROFILERFUNCS> profilerStack;

	const char* thisID = nullptr;
	const unsigned thisHash;
	
	static const char* currentID;
	static unsigned currentHash;
	
	Profiler(const char* ID, const unsigned hash) : 
		thisID(ID),
		thisHash(hash)
		{
		WTFPROFILER::start(thisID, thisHash);
	}
	
	~Profiler() { 
		WTFPROFILER::stop(thisID, thisHash);
	}
	
	static void show() {
		#ifdef ENABLEPROFILER
			WTFPROFILER::show();
		#endif
	}
	
	static void reset() {
		#ifdef ENABLEPROFILER
			WTFPROFILER::reset();
		#endif
	}
	
	
};

// this being previously defined as static fucked me hard bc im,,, we are,, 
// the hashing is,,, gods this is so confusing
// the hashing is going based off the pointer instead of the actual string???
// static made the pointers the same 
// but even with that, it still doesnt fully work

// i swear. i am fucking high. i do not understand this shit 
// i defined the hash as a const, still nothing
// my now assumption is that since we are hashing the pointer, and not the string, that 
// O3 is commiting some fuckery, and making them different?
// so ill need to write a string hash func

// ok nvm, adding the static back fixed it??????
// i,, gods 
// i shouldof just made the hash the key
// ive lost so much time on a feature that i dont really even fucking need tbh omfg
// and the timeloss wasnt even on actually implimenting it, but just pointer fuckery.

#ifdef ENABLEPROFILER
#define profileFunction() \
    static const char* BETTER_FUNCTION_NAME = extractClassAndFunctionName(__PRETTY_FUNCTION__); \
	const unsigned BETTER_FUNCTION_NAME_HASH = bn::hash<unsigned>()(getHash(BETTER_FUNCTION_NAME));  \
    volatile Profiler profiler(BETTER_FUNCTION_NAME, BETTER_FUNCTION_NAME_HASH);
#else
#define profileFunction() \
	do {} while(false)
#endif

