
// utag of all users belong to one group consist utag keywds of this group, and then calculate the utag weight

#include "DS.h"
#include "utag.h"
#include "gtag.h"
#include "vote.h"
#include "user.h"
#include "item.h"
#include <algorithm>
#include <math.h>

//utag's weight of user, TF-IDF weight or maybe simple or weight equal is more reasonable. 

extern Id_Index id_Index;	///global varaible, initialized in  data loading process

bool comp(map<size_u,float>::value_type& i1, map<size_u,float>::value_type& i2);

void utag_vote_weight(map<size_u, Vote>& votes,  map<size_u, User>& users,  map<size_u, UTag>& utags, map< string, map<size_u, Item> >& multi_items) {

	string channel = "voting";
	cout<<channel<<endl;
	const int VOTE_NUM  = id_Index.id_index[channel].size(); //total user number 
	
	for(int i = 0; i< VOTE_NUM; i++) {	///for each gruop
	
		size_u vote_id = id_Index.id_index[channel].at(i);
		//cout<<"group id "<<group_id<<endl;
		for(vector<size_u>::iterator VIt = multi_items[channel][vote_id].user_list.begin(); VIt != multi_items[channel][vote_id].user_list.end(); VIt++) { //for each user who vote this vote
		
			//cout<<"y1"<<endl;
			size_u user_id = *VIt;
			for(map<size_u, float>::iterator MIt = users[user_id].utag_weight.begin(); MIt != users[user_id].utag_weight.end(); MIt++) { //for each utag
			
				//cout<<"y2"<<endl;
				size_u utag_id = MIt->first; ///utag could be same of different user in same group. Cannot parallize. 
				votes[vote_id].utag_tf[utag_id] += 1; ///this can also be weighted by utag weight. MIt->second. 
				utags[utag_id].vote_list_by_vote_member.push_back(vote_id);
				
				
			}//for each utag

		}//for each user who vote this vote
		
	}//for each group

	//cout<<"y3"<<endl;

	///next calculate tf-idf weight of utag in group
	#pragma omp parallel for schedule(dynamic, 20)
	for(int i = 0; i< VOTE_NUM; i++) {	///for each gruop
	
		if(i%100000 ==0){cout<<"vote index:"<<i<<endl;}
		size_u vote_id = id_Index.id_index[channel].at(i);
		
		if(votes[vote_id].utag_tf.size() > 0) {
		
			map<size_u,float>::iterator It = max_element(votes[vote_id].utag_tf.begin(), votes[vote_id].utag_tf.end(), comp);
			float max_tf = It->second;

			for(map<size_u, float>::iterator MIt = votes[vote_id].utag_tf.begin(); MIt != votes[vote_id].utag_tf.end(); MIt++) {
		
				size_u utag_id = MIt->first;
				float utag_tf = MIt->second/max_tf; ///normalized
				assert(utag_tf <= 1);
				float n_k = utags[utag_id].vote_list_by_vote_member.size();  ///number of gorups related to this utag. 
				assert(n_k > 0);
				votes[vote_id].utag_weight[utag_id] = utag_tf * log(VOTE_NUM/n_k);
			}
		}	
	}

}