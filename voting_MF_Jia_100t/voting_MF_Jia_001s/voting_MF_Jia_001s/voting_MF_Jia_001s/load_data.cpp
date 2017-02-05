//---------------------------------------------------
// modified by: miao
// last modified on: 06-19-2016
//---------------------------------------------------


/*
 * read_data.cpp
 *
 * Created on: Mar 30, 2011
 * Author: eric
 */

#include "DS.h"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include "gtag.h"
#include "utag.h"
#include "group.h"
#include "database.h"
#include "dbfactory.h"
#include <math.h>


extern map<size_u, User> users;
extern map< string, map<size_u, Item> > multi_items;
extern map<size_u, UTag> utags;
extern map<size_u, GTag> gtags;
extern map<size_u, Group> groups;
extern map<size_u, Vote> votes;

Id_Index id_Index;
database* db = dbfactory::createDataBase("158.132.20.83", "root", "123456", "weibo_bkp");

// added by miao-06-19-2016
extern map< pair<string, string>, map< size_u, map<size_u, float> > > multi_matching_score;
extern map< pair<string, string>, map< size_u, map<size_u, float> > > test_matching_score;

void load_data() {
	map< string, vector<size_u> > id_index;
	Channels channels;
	cout << "loading all ids..." << endl;

	db->getAllId("user_id_all", id_Index.id_index[channels.channel_map[0]]);
	cout << "user id loading done\t" << id_Index.id_index[channels.channel_map[0]].size() << endl;

	db->getAllId("vote_id_all_1", id_Index.id_index[channels.channel_map[1]]);
	cout << "vote id loading done\t" << id_Index.id_index[channels.channel_map[1]].size() << endl;

	db->getAllId("group_id_all_1", id_Index.id_index[channels.channel_map[2]]);
	cout << "group id loading done\t" << id_Index.id_index[channels.channel_map[2]].size() << endl;


    // load user-user relationship
  db->getUserItemInteract("user_relation", Channels().channel_map[0], users, multi_items);
	cout << "user_user loading done\n";
	// assign trust value
  cout << "assign trust value" << endl;
	const int USER_NUM = id_Index.id_index[Channels().channel_map[0]].size();
	#pragma omp parallel for schedule(dynamic, 10)
	for(int u = 0; u < USER_NUM; u++) {
		size_u user_id = id_Index.id_index["social"][u];
		int self_out_degree = users[user_id].ratings["social"].size() + 1;
		for(vector<size_u>::iterator VIt = users[user_id].ratings["social"].begin(); VIt != users[user_id].ratings["social"].end(); VIt++) {
			size_u followee_id = *VIt;
			int friend_in_degree = multi_items["social"][followee_id].user_list.size() + 1;
			float trust_val = sqrt(friend_in_degree * 1.0 / (friend_in_degree + self_out_degree));
			users[user_id].trust_val[followee_id] = trust_val;
		}
		//users[user_id].trust_val[user_id] = 1.0;  ////user trust it self fully
		//multi_items["social"][user_id].user_list.push_back(user_id);  //user is trusted by itself
	}
	cout << "assing trust value done\n";


    //load user-group relationship
    db->getUserItemInteract("user_group", Channels().channel_map[2], users, multi_items);
	cout << "user_group loading done\n";

	// added by miao-06-19-2016
	db->getMatchScore("user_vote_score", Pair_Channels().pair_channel_map[0], multi_matching_score);
	cout << "loading user_vote matching score done" << endl;

	// added by Jia-06-09-2016

	db->getTestScore("user_vote_test_1000_new", Pair_Channels().pair_channel_map[0], test_matching_score);
	cout << "test_score matching score done" << endl;

	// loading training set
	db->getUserItemInteract("user_vote_80", Channels().channel_map[1], users, multi_items);
	cout << "user_vote loading done\n";

	// loading testing set
	db->getUserItemInteract("user_vote_20", Channels().channel_map[1],users, multi_items, 1);
	cout << "user_vote testing loading done\n";

	cout<<"load data done"<<endl;
}
