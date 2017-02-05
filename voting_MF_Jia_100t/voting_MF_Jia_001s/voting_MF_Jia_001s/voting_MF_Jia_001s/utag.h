
#ifndef UTAG_H
#define UTAG_H

#include <vector>
#include <string>
#include "paras.h"
using namespace std;

class UTag{

	//user tag

public:
	std::vector<size_u> user_list;	///store users who own this user tag
	std::vector<size_u> gtag_list;		///the list of group tag which match this user tag
	std::vector<size_u> group_list;	///list of group title which match this user tag
	std::vector<size_u> vote_list;   ///list of vote title which match this user tag.

	std::vector<size_u> group_list_by_group_member;  ///list of group related by users in the group. utag->user->group
	std::vector<size_u> vote_list_by_vote_member;   ///list of vote related by users vote(vt) the vote(noun). utag->user->group

	UTag() {}
};

#endif
