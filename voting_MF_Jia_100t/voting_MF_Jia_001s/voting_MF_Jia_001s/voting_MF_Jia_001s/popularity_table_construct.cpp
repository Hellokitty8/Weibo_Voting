//------------------------------------------------------------------------------------------
// Created at: 06-13-2016
// Created by: miao
// Created for: calculate item's friend-level and group-level popularity regarding each user
//------------------------------------------------------------------------------------------
//
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "database.h"
#include "dbfactory.h"

using namespace std;

extern map<size_u, User> users;
extern map<string, map<size_u, Item> > multi_items;
extern Id_Index id_Index;

void popularity_table_construct() {


	const int USER_NUM = id_Index.id_index[Channels().channel_map[0]].size();
	cout << "constructing popularity table ..." << endl;

	// for each user
	#pragma omp parallel for schedule(dynamic, 1)
	for(int i = 0; i < USER_NUM; i++) {
		size_u user_id = id_Index.id_index[Channels().channel_map[0]][i];

		// for each voting that user i ever participated
		for(vector<size_u>::iterator VIt = users[user_id].ratings["voting"].begin(); VIt != users[user_id].ratings["voting"].end(); VIt++) {
			size_u vote_id = *VIt;

			// calculate item's friend-level popularity
			users[user_id].popularity["social"][vote_id] = 0;

			for(vector<size_u>::iterator FIt = users[user_id].ratings["social"].begin(); FIt != users[user_id].ratings["social"].end(); FIt++ ) {
				size_u followee_id = *FIt;
				vector<size_u>::iterator iLoc = find(multi_items["voting"][vote_id].user_list.begin(), multi_items["voting"][vote_id].user_list.end(), followee_id);
				if(iLoc != multi_items["voting"][vote_id].user_list.end()) {
					users[user_id].popularity["social"][vote_id] += users[user_id].trust_val[followee_id];
				}
			}

			// calculate items's group-level popularity
			users[user_id].popularity["group"][vote_id] = 0;

			for(vector<size_u>::iterator GIt = users[user_id].ratings["group"].begin(); GIt != users[user_id].ratings["group"].end(); GIt++) {
				size_u group_id = *GIt;
				// find how many users in user_i's relevant groups that ever participated in the vote with vote_id
				for(vector<size_u>::iterator GUIt = multi_items["group"][group_id].user_list.begin(); GUIt != multi_items["group"][group_id].user_list.end(); GUIt ++) {
					size_u group_member_id = *GUIt;
					if (group_member_id == user_id) {
                        continue;
					}
					vector<size_u>::iterator iLoc = find(multi_items["voting"][vote_id].user_list.begin(), multi_items["voting"][vote_id].user_list.end(), group_member_id);
					if(iLoc != multi_items["voting"][vote_id].user_list.end()) {
						users[user_id].popularity["group"][vote_id] += 1;
					}
				}
			}

		}

	}



}
