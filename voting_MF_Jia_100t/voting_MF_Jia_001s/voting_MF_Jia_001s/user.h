
#ifndef USER_H
#define USER_H

#include <vector>
#include <map>
#include <utility>
#include <list>
#include <string>
#include "paras.h"
using namespace std;

class User{

public:


	map<string,vector<size_u> > ratings;	///multi-channel rated items, including user's voting, user's group
	map<size_u, float> trust_val; ///for social channel, the trust to followees
	map<string,vector<size_u> > testset;	///multi-channel test data

	map<size_u, float> utag_weight;	//key: tag_id, val: tag weight(TF-IDF weight)

	map<string, std::list<std::pair<size_u, float> > > sim_list; ///map key: channel, map val: list of similar users, pair.first: item_id, pair_second: similarity value

	vector<float> user_latent_feature;
	vector<float> user_latent_feature_previous;

    //added by miao-06-13-2016
    map<string, map<size_u, float> > popularity; //map key: channel("social" or "group"), map value: map of the popularity of the items, friend-level or group-level popularity
	  map<size_u, float> ts_influence; // the ts_influence index of user i regarding its relavent votes

	User(){

		user_latent_feature.resize(dim);
		user_latent_feature_previous.resize(dim);
	}

};

#endif
