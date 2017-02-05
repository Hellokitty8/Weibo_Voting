//------------------------------------------------------------
// top_n_test_miao.cpp
// created on: 06-28-2016
// created by: miao
// ------------------------------------------------------------

#include "DS.h"
#include "user.h"
#include "item.h"
#include "database.h"
#include "dbfactory.h"
#include "paras.h"
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <set>
#include <utility>
#include <list>
using namespace std;

extern map<size_u, User> users;	///key: user_id, val: User*
extern map< string, map<size_u, Item> > multi_items; //key: channel, val{key: item_id, val: item class}
extern Id_Index id_Index;
extern database* db;


bool comp2(pair<size_u,float> first, pair<size_u,float> second){	////large one is in front

	if(first.second > second.second) return true;
		else {return false;}
}

Total_liked total_liked;
Hit_liked hit_liked;

int top_n_test_miao() {	///top-k recomm
	const int USER_NUM = id_Index.id_index[Channels().channel_map[0]].size();

	//#pragma omp parallel for schedule(dynamic, 10)
	for(int u = 0; u < USER_NUM; u += 100) {

		//if(u % 10000 == 0) {
            cout << "user: " << u << endl;
        //}
		//string channel = "group"; ///testing group recommendation
		//assert(multi_items[channel].begin()->second.item_latent_feature.size() == dim); //make sure this channel has item latent feature.

		size_u user_id = id_Index.id_index["social"][u];

		/*
		int max_rate = 0;
		map<string, float>::iterator MIt;
		for(MIt = users[user_id].testset[channel].begin(); MIt != users[user_id].testset[channel].end(); MIt++) {

			if(MIt->second > max_rate) {max_rate = MIt->second;	}
		}*/

		//if(max_rate == MAX_R){	///need testing
		if(users[user_id].testset.size() > 0){	///  this in for one-class rating.
			list<pair<size_u,float> > recomm_pool; ///store all predicted item-rating pair
			set<size_u> training_set(users[user_id].ratings["voting"].begin(), users[user_id].ratings["voting"].end());

			pair< size_u, vector<float> > user_topic_dist_rec;
            db->getTopicDist("user_dis", "social", user_id, user_topic_dist_rec);

			map<size_u, Item>::iterator MIt1;
			for(MIt1 = multi_items["voting"].begin(); MIt1 != multi_items["voting"].end(); MIt1++) { //for all votings
				size_u vote_id = MIt1->first;
				if(training_set.count(vote_id) == 0){
					// predict the rating for all votes not in training set
					float predict_rating = 0;
					float ts_influence = 0;
					for(int i = 0; i< dim; i++) {
                        predict_rating += MIt1->second.item_latent_feature[i] * users[user_id].user_latent_feature[i];
                    }

                    //cout << u << " " << 1 << endl;
					// load the topic distributions of current user and current vote

					pair< size_u, vector<float> > vote_topic_dist_rec;
					db->getTopicDist("vote_dis", "voting", vote_id, vote_topic_dist_rec);

					// calculate Jensen-Shannon Divergence between the topic distributions of current user and current vote
					vector<float> mix_topic_dist_rec;
					for (int k = 0; k < TOPIC_K; k++ ) {
						mix_topic_dist_rec.push_back(0.5 * (user_topic_dist_rec.second[k] + vote_topic_dist_rec.second[k]));
					}
					float user_KL = 0;
					float vote_KL = 0;

					// calculate symmetric KL divergence
					for (int k = 0; k < TOPIC_K; k++) {
						if (fabs(user_topic_dist_rec.second[k]) >= 1e-5) {
							user_KL += user_topic_dist_rec.second[k] * (log(user_topic_dist_rec.second[k]) - log(mix_topic_dist_rec[k]));
						}
						if (fabs(vote_topic_dist_rec.second[k]) >= 1e-5) {
							vote_KL += vote_topic_dist_rec.second[k] * (log(vote_topic_dist_rec.second[k]) - log(mix_topic_dist_rec[k]));
						}
					}

					float JSD = 0.5 * user_KL + 0.5 * vote_KL;
					float ts_part_1 = 1 - JSD;

					mix_topic_dist_rec.clear();


					// calculate the popularity of the vote among the friends and group members of current user
					//users[user_id].popularity["social"].push_back([vote_id, 0]);
					float ts_part_2 = 0;
					for (vector<size_u>::iterator FIt = users[user_id].ratings["social"].begin(); FIt != users[user_id].ratings["social"].end(); FIt++) {
						size_u followee_id = *FIt;
						vector<size_u>::iterator iLoc = find(multi_items["voting"][vote_id].user_list.begin(), multi_items["voting"][vote_id].user_list.end(), followee_id);
						if(iLoc != multi_items["voting"][vote_id].user_list.end()) {
							//users[user_id].popularity["social"][vote_id] += users[user_id].trust_val[followee_id];
							ts_part_2 += users[user_id].trust_val[followee_id];
						}
					}
					ts_part_2 = 2 / (1 + exp(-1 * ts_part_2 / regular_coefficient)) - 1;

					//users[user_id].popularity["group"].push_back([vote_id, 0]);
					float ts_part_3 = 0;
					for (vector<size_u>::iterator GIt = users[user_id].ratings["group"].begin(); GIt != users[user_id].ratings["group"].end(); GIt++) {
						size_u group_id = *GIt;
						for (vector<size_u>::iterator GUIt = multi_items["group"][group_id].user_list.begin(); GUIt != multi_items["group"][group_id].user_list.end(); GUIt++) {
							size_u group_member_id = *GUIt;
							if (group_member_id == user_id) {
                                continue;
							}
							vector<size_u>::iterator iLoc = find(multi_items["voting"][vote_id].user_list.begin(), multi_items["voting"][vote_id].user_list.end(), group_member_id);
							if(iLoc != multi_items["voting"][vote_id].user_list.end()) {
								//users[user_id].popularity["group"][vote_id] += 1;
								ts_part_3 += 1;
							}
						}
					}
					ts_part_3 = 2 / (1 + exp(-1 * ts_part_3 / regular_coefficient)) - 1;

					// calculate the topic-social infleence TS_{ij}
					float current_ts = ts_coefficient_1 * ts_part_1 + ts_coefficient_2 * ts_part_2 + (1 - ts_coefficient_1 - ts_coefficient_2) * users[user_id].popularity["group"][vote_id];
					//users[user_id].ts_influence[vote_id] = current_ts; // let's decide whether need to store this value at a later time

					// update predict _rating
					predict_rating = RM().rm["voting"] + current_ts * predict_rating;

					pair<size_u, float> mypair(vote_id, predict_rating);
					recomm_pool.push_back(mypair);
				}
			}

			recomm_pool.sort(comp2);

			map<int, set<size_u> > Recomm_item_set;	///key: top_n, val, item_id
			TopN topN;
			vector<int>::iterator VIt;
			for(VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {

				int top_n = *VIt;
				int count = 0;
				for(list<pair<size_u,float> >::iterator LIt = recomm_pool.begin(); LIt!= recomm_pool.end() && count< top_n; LIt++, count++){	///recomm Top N items
					Recomm_item_set[top_n].insert((*LIt).first);
				}
			}
			recomm_pool.clear();

			///the go through current user's test set to test hit ratio
			for(VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
				int top_n = *VIt;

				vector<size_u>::iterator It;
				for(It = users[user_id].testset["voting"].begin(); It != users[user_id].testset["voting"].end(); It++) {

					#pragma omp atomic
					total_liked.total_liked[top_n]++;

					//if(recomm_item_set.find(MIt->first) != recomm_item_set.end() ){
					if(Recomm_item_set[top_n].count(*It) > 0){

						#pragma omp atomic
						hit_liked.hit_liked[top_n]++;
					}
				}
			}
		}//if(max_rate == MAX_R)

	}///for(int u=0; u<USER_NUM; u+= 1)

	cout<<"total liked #: "<< total_liked.total_liked[5]<<"\t hit #: "<<hit_liked.hit_liked[5]<<"\t hit ratio: "<<float(hit_liked.hit_liked[5])/total_liked.total_liked[5]<<endl;

	ofstream o1("/home/centos/data/weibo_MF_code/voting_MF_miao/voting_MF_miao_model_1/voting_MF/results_weiboMF_miao_model_1.txt", ios_base::app);
	assert(o1);

	o1 << "wm1: " << w_m_1 << endl;
	o1 << "ts1: " << ts_coefficient_1 << endl;
	o1 << "ts2: " << ts_coefficient_2 << endl;
	o1 << "dim: " << dim << endl;
	o1 << "epsino: " << epsino << endl;
	o1 << "regular coefficient: " << regular_coefficient << endl;
	TopN topN;
	Recall recall;
	vector<int>::iterator VIt;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		int top_n = *VIt;
		recall.recall_total[top_n] = 1.0 * hit_liked.hit_liked[top_n] / total_liked.total_liked[top_n];
		o1 << recall.recall_total[top_n] << "\t";
	}
	o1 << endl << endl;

	return 0;
}



