#include "Common.h"

Calculate_Time::Calculate_Time():
	start(),
	end()
{
	gettimeofday(&start, NULL);
}

Calculate_Time::~Calculate_Time() {
	gettimeofday(&end, NULL);
	int64_t delta = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
	DEBUG_LOG("Cost time %ld.%ld ms", delta / 1000, delta % 1000);
}

