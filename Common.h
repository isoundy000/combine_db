#ifndef _COMMON_H_
#define _COMMON_H_

#include <iostream>
#include <stdint.h>
#include <sys/time.h>
#include <vector>
#include <map>
#include <list>
#include "Debug_Log.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

class Calculate_Time {
public:
	Calculate_Time();
	~Calculate_Time();
private:
	timeval start, end;
};

#endif

