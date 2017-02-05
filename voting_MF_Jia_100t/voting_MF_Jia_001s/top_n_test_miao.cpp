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
extern map< pair<string, string>, map< size_u, map< size_u, float> > > test_matching_score;
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
  cout << "Test start!"<< endl;
  cout << "total number of user!" << USER_NUM/1000 << endl;
  int userNum=0;
	 #pragma omp parallel for schedule(dynamic, 10)
	 for(int u = 0; u < USER_NUM ; u += 1000) { //for all users


		size_u user_id = id_Index.id_index["social"][u];

		if(users[user_id].testset.size() > 0){
			userNum=userNum+1;

			list<pair<size_u,float> > recomm_pool;
			set<size_u> training_set(users[user_id].ratings["voting"].begin(), users[user_id].ratings["voting"].end());//find training set

            cout << "total number of user!" << USER_NUM/1000 << endl;
			cout << "current user:" << userNum << endl;
			cout << "user:ID" << user_id << endl;

			map<size_u, Item>::iterator MIt1;

			int log1=0;
   // #pragma omp parallel for schedule(dynamic, 10)

			for(MIt1 = multi_items["voting"].begin(); MIt1 != multi_items["voting"].end(); MIt1++) { //for all votings
			    log1=log1+1;
				size_u vote_id = MIt1->first;
				if(training_set.count(vote_id) == 0){
					// predict the rating for all votes not in training setset
					float predict_rating = 0;

					for(int i = 0; i< dim; i++) {
                                              predict_rating += MIt1->second.item_latent_feature[i] * users[user_id].user_latent_feature[i];

					}
					float score=test_matching_score[Pair_Channels().pair_channel_map[0]][user_id][vote_id];

         float ts_part_1 =score;
		    //if(log1% 1000000 == 0) {
					//  cout << "userID: " << user_id << endl;
            //          cout << "voteID: " << vote_id<< endl;
              //        cout << "Score: " << score << endl;
                //    }
					float ts_part_2 = 0;
					for (vector<size_u>::iterator FIt = users[user_id].ratings["social"].begin(); FIt != users[user_id].ratings["social"].end(); FIt++) {
						size_u followee_id = *FIt;


                        vector<size_u>::iterator it = find (multi_items["voting"][vote_id].user_list.begin(), multi_items["voting"][vote_id].user_list.end(), followee_id);

					if(it != multi_items["voting"][vote_id].user_list.end()) {
			               // cout << "here is the answer!" << endl;
                                         //cout << *it << endl;
	        	                 ts_part_2 += users[user_id].trust_val[followee_id];
							}



					}
					ts_part_2 = 2 / (1 + exp(-1 * ts_part_2 / regular_coefficient)) - 1;

					//if(ts_part_2 != 0) {

					//cout << "Before ts_part_2 " << vote_id<< endl;
                                        //cout << ts_part_2 << endl;




                                      //  cout << ts_part_2  << endl;
                                        //}

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

								ts_part_3 += 1;
							}
						}
					}

                                        if(ts_part_3  != 0) {

					ts_part_3 = 2 / (1 + exp(-1 * ts_part_3 / regular_coefficient)) - 1;

                                        }



					// calculate the topic-social infleence TS_{ij}
					float current_ts = ts_coefficient_1 * ts_part_1 + ts_coefficient_2 * ts_part_2 + (1 - ts_coefficient_1 - ts_coefficient_2) * ts_part_3;
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
	//cout<<"total liked #: "<< total_liked.total_liked[5]<<"\t hit #: "<<hit_liked.hit_liked[5]<<"\t hit ratio: "<<float(hit_liked.hit_liked[5])/total_liked.total_liked[5]<<endl;

ofstream o1("/home/centos/data/weibo_MF_code/voting_MF_Jia/semantic_0_0_1.txt", ios_base::app);
assert(o1);
//char str[1000];
//sprintf(str, "/home/centos/data/weibo_MF_code/voting_MF_Jia/results/recall_and_precision_1.txt");
//ofstream o1(str, ios_base::app);
//assert(o1);

	o1 << "wm1: " << w_m_1 << endl;
	o1 << "ts1: " << ts_coefficient_1 << endl;
	o1 << "ts2: " << ts_coefficient_2 << endl;
	o1 << "dim: " << dim << endl;
	o1 << "epsino: " << epsino << endl;
	o1 << "regular coefficient: " << regular_coefficient << endl;


	TopN topN;
	vector<int>::iterator VIt;

	Recall recall;
	o1 << "recall:" << endl;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		  int top_n = *VIt;
	//	recall.recall_total[top_n] = 1.0 * hit_liked.hit_liked[top_n] / total_liked.total_liked[top_n];
      recall.recall_total[top_n] = float(hit_liked.hit_liked[top_n]) / total_liked.total_liked[top_n];
		  o1 << recall.recall_total[top_n] << "\t";
	}
	o1 << endl;

/* added by Jia Wang at 2016/09/25 17:01 */
	// precision
	Precision precision;
	o1 << "precision:" << endl;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		int top_n = *VIt;
		precision.precision_total[top_n] = float(hit_liked.hit_liked[top_n]) /(top_n * userNum);
		o1 << precision.precision_total[top_n] << "\t";
	}
	o1 << endl;

	// f value
	o1 << "fvalue:" << endl;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		int top_n = *VIt;
		int fvalue = 2 * precision.precision_total[top_n] * recall.recall_total[top_n] / (precision.precision_total[top_n] + recall.recall_total[top_n]);
		o1 << fvalue << "\t";
	}
	o1 << endl;

	// roc curve
	o1 << "roc curve points:" << endl;
	o1 << "x\ty" << endl;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		int top_n = *VIt;
		float fpr = float(top_n*userNum - hit_liked.hit_liked[top_n]) / (id_Index.id_index["voting"].size() - top_n - total_liked.total_liked[top_n] + hit_liked.hit_liked[top_n]);
		float tpr = precision.precision_total[top_n];
		o1 << fpr << "\t" << tpr << endl;
	}
	o1 << endl << endl;


	return 0;
}
