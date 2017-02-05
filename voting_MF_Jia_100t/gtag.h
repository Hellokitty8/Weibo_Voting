
#ifndef GTAG_H
#define GTAG_H

#include <vector>
#include <string>
#include "paras.h"
using namespace std;

class GTag{
///group tag

public:
	vector<size_u> group_list;	///store groups who own this group tag
	std::vector<size_u> utag_list; ///list of utag which match this group tag
	GTag() {}
};

#endif
