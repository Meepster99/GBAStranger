	

#include "Profiler.h"


//#include "bn_cstring.h" // may or may not have not known this was a thing until now
//#include "bn_cstdlib.h"
// nvm it doesnt have string funcs, still



//bn::unordered_map<const char*, ProfilerData, MAXPROFILERFUNCS> Profiler::profilerMap = bn::unordered_map<const char*, ProfilerData, MAXPROFILERFUNCS>();
//bn::deque<charHashPair, MAXPROFILERFUNCS> Profiler::profilerStack = bn::deque<charHashPair, MAXPROFILERFUNCS>();

bn::timer BetterTimer::timer = bn::timer();

namespace WTFPROFILER {
	
	// all funcs are defined as inline for the literal only reason of im to lazy to put them in the .h

	inline void *memset(void *ptr, int value, size_t num) {
		unsigned char *byte_ptr = (unsigned char *)ptr;
		unsigned char byte_value = (unsigned char)value;

		for (size_t i = 0; i < num; i++) {
			byte_ptr[i] = byte_value;
		}

		return ptr;
	}

	inline void *memcpy(void *dest, const void *src, size_t num) {
		unsigned char *dest_ptr = (unsigned char *)dest;
		const unsigned char *src_ptr = (const unsigned char *)src;

		for (size_t i = 0; i < num; i++) {
			dest_ptr[i] = src_ptr[i];
		}

		return dest;
	}

	inline int strcmp(const char *str1, const char *str2) {
		while (*str1 != '\0' && *str2 != '\0') {
			if (*str1 != *str2) {
				return (*str1 - *str2);
			}
			str1++;
			str2++;
		}

		return 0;
	}

	inline char *strcpy(char *dest, const char *src) {
		char *original_dest = dest;

		while (*src != '\0') {
			*dest = *src;
			dest++;
			src++;
		}

		*dest = '\0';

		return original_dest;
	}
	
	inline char* writeNumber(char* start, uint64_t n) {
		
		int numLength = 0;
		int temp = n;
		while (temp != 0) {
			temp /= 10;
			numLength++;
		}

		temp = n;
		for (int i = numLength - 1; i >= 0; i--) {
			*(start + i) = '0' + (temp % 10);
			temp = temp / 10;
		}
		
		return start + numLength;
	}

	inline char* padNumber(char* start, uint64_t n) {
		
		// lets just pray we dont overrun the buffer here lmao
		
		*start = '\t';
		
		int numLength = 0;
		int temp = n;
		
		while (temp != 0) {
			temp /= 10;
			numLength++;
		}
		
		int leadingSpaces = MAX(0, 10 - numLength);
		
		// buffer is already full of space, so we dont need to fill it with spaces
		return writeNumber(start + leadingSpaces, n);
	}

	
	// this will fuck up heavily on recursive funcs. 
	
	// I HAVE 0 CLUE
	// i, my map doesnt work, theirs does, theirs is in ewram, and yea??
	
	// should these also be declared as static?
	BN_DATA_EWRAM bn::unordered_map<const char*, ProfilerData, MAXPROFILERFUNCS> profilerMap;
	
	// this queue honestly,, isnt needed tbh. but it makes things cleaner so we ball
	BN_DATA_EWRAM bn::deque<charHashPair, MAXPROFILERFUNCS> profilerStack;

	BN_DATA_EWRAM char buffer[maxFuncLength + 64];
	
	void reset() {
		
		BN_ASSERT(profilerStack.size() == 0, "when reseting the profiler make sure nothing is currently being profiled");
		
		profilerStack.clear();
		profilerMap.clear();
		
		// i have 0 clue whats going on here. i updated my butano version, and clear started giving me insanely weird bugs. this fixes them
		// https://github.com/GValiente/butano/commit/010ce11208ee5bb42b6fbba0baf06b9f02cc0426
		//profilerMap = bn::unordered_map<const char*, ProfilerData, MAXPROFILERFUNCS>();
		
		BN_LOG("Profiler reset");
	}
	
	void show() {
		
		
		// prepare for goofy sorting
		
		bn::vector<bn::pair<const char*, ProfilerData>, MAXPROFILERFUNCS> sortedProfilerData;
		for(const auto& idk : profilerMap) {
			sortedProfilerData.push_back(bn::pair<const char*, ProfilerData>(idk.first, idk.second));
		}
		
		
		for (int i = 0; i < sortedProfilerData.size() - 1; ++i) {
			int minIndex = i;
			for (int j = i + 1; j < sortedProfilerData.size(); ++j) {
				
				auto jObj = sortedProfilerData[j].second;
				auto minObj = sortedProfilerData[minIndex].second;
				
				int jTemp = jObj.total / jObj.timesCalled;
				int minTemp = minObj.total / minObj.timesCalled;
				
				if(jTemp > minTemp) {
					minIndex = j;
				}
			}
			if (minIndex != i) {
				auto temp = sortedProfilerData[i];
				sortedProfilerData[i] = sortedProfilerData[minIndex];
				sortedProfilerData[minIndex] = temp;
			}
		}
		
		
		BN_LOG("-----");
		BN_LOG("Displaying profiler results");
		BN_LOG(profilerMap.size(), " funcs tracked");
		//BN_LOG("Bruh::realllylongfunctionname    	      474 	      254 	        2 	      237                   
		BN_LOG("func name                                       	     total 	      max 	     calls 	      avg");                   
		for(const auto& ticks_per_entry_pair : sortedProfilerData) {
		
			const ProfilerData& ticks = ticks_per_entry_pair.second;
		
			memset(&buffer, ' ', sizeof(buffer));
			buffer[sizeof(buffer) - 1] = '\0';
			
		
			// got stuck on strlen again until i of course remembered, the WTF function
			memcpy(&buffer, ticks_per_entry_pair.first, WTF(ticks_per_entry_pair.first));
		
		
			char* temp = padNumber(buffer + maxFuncLength + 1, ticks.total);
			
			temp = padNumber(temp + 1, ticks.max);
			
			temp = padNumber(temp + 1, ticks.timesCalled);
		
		
			uint64_t averageTickCount = ticks.total / ticks.timesCalled;
		
			temp = padNumber(temp + 1, averageTickCount); 

			BN_ASSERT(buffer[sizeof(buffer) - 1] == '\0', "you overran the profiler buffer idiot lmao. be a better programmer");
			BN_LOG(buffer);
			//BN_LOG(ticks_per_entry_pair.first, "\t", ticks.total, "\t", ticks., "\t", ticks.timesCalled );
		}
		
		BN_LOG("-----");
	}
	
	void start(const char* ID, const unsigned hash) {

		if(profilerStack.size() != 0) {
			charHashPair& temp = profilerStack.back();
			ProfilerData& tempData = profilerMap(temp.hash, temp.ID);
			
			// pause
			tempData.timer.stop(); // does this being slightly delayed from the initial call cause issues?
		}	

		
		profilerStack.push_back({ID, hash});

		// i despise, and dont understand this syntax. 
		// does this create a blank struct if one doesnt exist?
		ProfilerData& data = profilerMap(hash, ID);
		
		// start
		
		data.timer.reset();
		data.timer.start();
		data.timesCalled++;
	
	}
	
	void stop(const char* ID, const unsigned hash) {
		
		BN_ASSERT(profilerStack.size() != 0, "stop was called on the profiler when nothing was even being profiled??");
		
		charHashPair& temp = profilerStack.back();
		profilerStack.pop_back();
		
		ProfilerData& data = profilerMap(hash, ID);
	
	
		// stop 
		
		data.timer.stop();
		unsigned elapsed = data.timer.getElapsed();
		data.total += elapsed;
		data.max = MAX(elapsed, data.max);
		
		if(profilerStack.size() != 0) {
			
			temp = profilerStack.back();
			
			//fuckng supidest bug every. i wanted to reassign the ref, not update it.
			// gods this was so much time loss. 
			// i havent ate enough or slept enough
			// data = profilerMap(temp.hash, temp.ID);
			ProfilerData& tempRef = profilerMap(temp.hash, temp.ID);
		
			// resume
			tempRef.timer.start();
			
		}
	
	}
	

	
}

