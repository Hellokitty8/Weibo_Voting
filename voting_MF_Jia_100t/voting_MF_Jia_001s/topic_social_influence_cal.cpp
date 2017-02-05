/*
 * topic_social_influence_cal.cpp
 * Created on: 06-18-2016
 * Created by: miao
 */

#include "DS.h"
#include <sys/time.h>
#include <algorithm>
#include <string>
#include <iterator>
#include <math.h>
#include "user.h"
#include "item.h"

using namespace std;

extern map<size_u, User> users;
extern map< string, map<size_u, Item> > multi_items;
extern map< pair<string, string>, map< size_u, map< size_u, float> > > multi_matching_score;
extern Id_Index id_Index;


void topic_social_influence_cal() {
	cout << "\nCalculation on topic_social_influence index for the training set begins..." << endl;

	float ts_part1, ts_part2, ts_part3;
	const int USER_NUM = id_Index.id_index[Channels().channel_map[0]].size();

	#pragma omp parallel for schedule(dynamic, 1)
	for (int i = 0; i < USER_NUM; i++) {
		size_u user_id = id_Index.id_index["social"][i];
		for (vector<size_u>::iterator VIt = users[user_id].ratings["voting"].begin(); VIt != users[user_id].ratings["voting"].end(); VIt++) {
			size_u vote_id = *VIt;
			ts_part1 = multi_matching_score[Pair_Channels().pair_channel_map[0]][user_id][vote_id];
			ts_part2 = 2 / (1 + exp(-1 * users[user_id].popularity["social"][vote_id] / regular_coefficient)) - 1;
			ts_part3 = 2 / (1 + exp(-1 * users[user_id].popularity["group"][vote_id] / regular_coefficient)) - 1;
			users[user_id].ts_influence[vote_id] = ts_coefficient_1 * ts_part1 + ts_coefficient_2 * ts_part2 + (1 - ts_coefficient_1 - ts_coefficient_2) * ts_part3;
		}
	}
}
