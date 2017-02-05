
#ifndef VOTE_H
#define VOTE_H

#include <vector>
#include <string>
#include "paras.h"
using namespace std;

class Vote{

public:
	
	map<size_u, float> utag_tf; ///utag's term frequency in this group. utag is from all users belong to this gruop
	map<size_u, float> utag_weight; //key: utag id, val: weight in this group(TF-IDF weight). utag is from all users belong to this gruop
	std::vector<size_u> utag_list; //list of user tag which match this vote title.
	Vote() {	}
};

#endif
