
#include "DS.h"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <utility>
#include <list>
#include "user.h"
#include "item.h"
#include "gtag.h"
#include "utag.h"
#include "vote.h"


const float weight_utag_vtitle = 0; ///channel weight of method of utag matching gtitle
const float weight_utag_v = 1;	//channel weight of method of utag matching vote's related user's utag
const float weight_followee_rec = 1; ///channel weight of recommendation from followee's recommendation
const float weight_CF_rec = 0;
//recommendation based on tag weight. 
///for each utag matching gtag, denote w_1 weight of utag in user, and w_2 weight of gtag in group. It contribute w_1*w_2. 
//1. user's utag matching group's gtag, the more macthing is simply better. \sum(w_1*w_2) 
//2. user's utag and group's utag, the more matching the better, \sum(w_1 * w_2); 
//3. user's utag and group's title, the more matching the better. w_2 = 1, \sum(w_1 * w_2); 
//4. combine 1,2 and 3's result. linearly? 
extern Id_Index id_Index;	///global varaible, initialized in  data loading process

bool comp2(pair<size_u,float> first, pair<size_u,float> second);
double PCC(size_u i1, size_u i2, string channel);
void utag_user_weight(map<size_u, User>& users, map<size_u, UTag>& utags);
void utag_vote_weight(map<size_u, Vote>& votes,  map<size_u, User>& users,  map<size_u, UTag>& utags, map< string, map<size_u, Item> >& multi_items);

void tag_based_vote_rec( map<size_u, User>& users, map<size_u, Vote>& votes, map<size_u, UTag>& utags, map<string, map<size_u, Item> >& multi_items){

	utag_user_weight(users,utags);
	cout<<"utag user weight done\n";
	//utag_vote_weight(votes,users, utags, multi_items);
	//cout<<"utag vote weight done\n";

	Total_liked total_liked;
	Hit_liked hit_liked;
	const int USER_NUM  = id_Index.id_index["social"].size(); //total user number 

	#pragma omp parallel for schedule(dynamic, 100)
	for(int u = 0; u< USER_NUM; u+= 1) {	//for each user

		if(u%100000==0){cout<<"user: "<<u<<endl;}
		size_u user_id = id_Index.id_index["social"].at(u);
		if(users[user_id].sim_list["voting"].size() >= 0) { //testing on sampled users, been modified
		map<size_u, float> vote_weight; //key: candidate group id for this user, val: weight for recommend
		for(map<size_u, float>::iterator MIt = users[user_id].utag_weight.begin(); MIt != users[user_id].utag_weight.end(); MIt++) { ///for each user tag
		
			size_u utag_id = MIt->first;
			float w_1 = MIt->second; ///utag's weight in this user
			
		
			/////////////////////////////////////////////////////////////////////////////////////////////for utag-vtitle matching begin
		
			for(vector<size_u>::iterator VIt = utags[utag_id].vote_list.begin(); VIt != utags[utag_id].vote_list.end(); VIt++) { //for each vote whose title match this utag
			
				vote_weight[*VIt] += w_1 * weight_utag_vtitle;
			}
			
			/////////////////////////////////////////////////////////////////////////////////////////////for utag-vtitle matching end

			

			/////////////////////////////////////////////////////////////////////////////////////////////for utag-utag matching begin
			for(vector<size_u>::iterator VIt = utags[utag_id].vote_list_by_vote_member.begin(); VIt != utags[utag_id].vote_list_by_vote_member.end(); VIt++) { //for each vote related user's utag matching this utag.
			
				size_u vote_id = *VIt;
				float w_2 = votes[vote_id].utag_weight[utag_id];
				vote_weight[*VIt] += w_1 * w_2 * weight_utag_v;
			}
			
			/////////////////////////////////////////////////////////////////////////////////////////////for utag-utag matching end
			
 
		}//for each user tag

		//////////////////////////////////////////////////////////////social friend based voting begin
		for(vector<size_u>::iterator VIt = users[user_id].ratings["social"].begin(); VIt != users[user_id].ratings["social"].end(); VIt++) { ///for user's each followee
			
			size_u followee_id = *VIt;
			for(vector<size_u>::iterator VIt = users[followee_id].ratings["voting"].begin(); VIt != users[followee_id].ratings["voting"].end(); VIt++) { //for followee's each joined vote
				
				size_u vote_id = *VIt;
				//float sim = (float) PCC(user_id,followee_id, "social");
				float sim = 1;
				#pragma omp atomic
				vote_weight[vote_id] += weight_followee_rec * sim;
			}
		}
		//////////////////////////////////////////////////////////////social friend based voting end
		set<size_u> sn_set;
		copy(users[user_id].ratings["voting"].begin(), users[user_id].ratings["voting"].end(), std::inserter(sn_set, sn_set.begin()));
		

		//////////////////////////////////////////////////////////////CF neigborhood based voting begin
		for(std::list<std::pair<size_u, float> >::iterator LIt = users[user_id].sim_list["voting"].begin(); LIt != users[user_id].sim_list["voting"].end(); LIt++) {
		
			if(sn_set.count(LIt->first) ==0){ ///not find in social neighborhood
			
				size_u neighbor_id = LIt->first;
				for(vector<size_u>::iterator VIt = users[neighbor_id].ratings["voting"].begin(); VIt != users[neighbor_id].ratings["voting"].end(); VIt++) {
				
					size_u vote_id = *VIt;
					#pragma omp atomic
					vote_weight[vote_id] += weight_CF_rec * LIt->second;

				}
			}
		}
		//////////////////////////////////////////////////////////////CF neigborhood based voting end
		

		list<pair<size_u,float> > recomm_pool;
		for(map<size_u, float>::iterator MIt = vote_weight.begin(); MIt != vote_weight.end(); MIt++) {
			
			pair<size_u, float> mypair(MIt->first, MIt->second);
			recomm_pool.push_back(mypair);
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

		}//if(users[user_id].sim_list["voting"].size() > 0)
	}//for each user

	cout<<"total liked #: "<< total_liked.total_liked[5]<<"\t hit #: "<<hit_liked.hit_liked[5]<<"\t hit ratio: "<<float(hit_liked.hit_liked[5])/total_liked.total_liked[5]<<endl;
	
	ofstream o1("/home/xiwangy/code/zhongdian/weibo1/weiboMF/results_tag_based.txt", ios_base::app);
	assert(o1);

	TopN topN;
	Recall recall;
	vector<int>::iterator VIt;
	o1<<"vote rec:\n";
	for(VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
	
		int top_n = *VIt;
		recall.recall_total[top_n] = float(hit_liked.hit_liked[top_n])/total_liked.total_liked[top_n];
		o1<<recall.recall_total[top_n]<<"\t";
	}
	o1<<endl;

/*	ofstream o2("/home/xiwangy/code/zhongdian/weibo1/weiboMF/pool_size.txt");
	assert(o2);
	for(vector<int>::iterator VIt = pool_size.begin(); VIt != pool_size.end(); VIt++) {
	
		o2 << *VIt<<endl;
	}*/

}
