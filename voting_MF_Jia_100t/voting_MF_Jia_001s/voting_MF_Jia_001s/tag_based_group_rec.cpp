
#include "DS.h"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <utility>
#include <list>
#include <math.h>
#include "user.h"
#include "item.h"
#include "gtag.h"
#include "utag.h"
#include "group.h"

const float weight_utag_gtag = 0;	///channel weight of method of utag matching gtag
const float weight_utag_gtitle = 0; ///channel weight of method of utag matching gtitle
const float weight_utag_g = 0;	//channel weight of method of utag matching group member's utag
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
void utag_group_weight(map<size_u, Group>& groups,  map<size_u, User>& users,  map<size_u, UTag>& utags, map< string, map<size_u, Item> >& multi_items);
void gtag_group_weight(map<size_u, Group>& groups, map<size_u, GTag>& gtags);

void tag_based_group_rec( map<size_u, User>& users, map<size_u, Group>& groups, map<size_u, UTag>& utags, map<size_u, GTag>& gtags, map< string, map<size_u, Item> >& multi_items){

	utag_user_weight(users,utags);
	cout<<"utag user weight done\n";
	//utag_group_weight(groups,users,utags, multi_items);
	//cout<<"utag group weight done\n";
	//gtag_group_weight(groups,gtags);
	//cout<<"gtag group weight done\n";

	Total_liked total_liked;
	Hit_liked hit_liked;
	string channel = Channels().channel_map[0];
	const int USER_NUM  = id_Index.id_index[channel].size(); //total user number 

	#pragma omp parallel for schedule(dynamic, 100)
	for(int u = 0; u< USER_NUM; u+= 1) {	//for each user

		if(u%10000==0){cout<<"user: "<<u<<endl;}
		size_u user_id = id_Index.id_index[channel].at(u);
		if(users[user_id].sim_list["group"].size() > 0) { //testing on sampled users
		map<size_u, float> group_weight; //key: candidate group id for this user, val: weight for recommend
		for(map<size_u, float>::iterator MIt = users[user_id].utag_weight.begin(); MIt != users[user_id].utag_weight.end(); MIt++) { ///for each user tag
		
			size_u utag_id = MIt->first;
			float w_1 = MIt->second; ///utag's weight in this user
			
		
			////////////////////////////////////////////////////////////////////////////////////////////for utag-gtag matching begin
			
			for(vector<size_u>::iterator VIt = utags[utag_id].gtag_list.begin(); VIt != utags[utag_id].gtag_list.end(); VIt++) { ///for each group tag match this user tag
			
				size_u gtag_id = *VIt;
				for(vector<size_u>::iterator VIt1 = gtags[gtag_id].group_list.begin(); VIt1 != gtags[gtag_id].group_list.end(); VIt1++){///for each group who owns this group tag
				
					size_u group_id = *VIt1;
					float w_2 = groups[group_id].gtag_weight[gtag_id];
					group_weight[group_id] += w_1 * w_2 * weight_utag_gtag* sqrt(multi_items["group"][group_id].user_list.size() * 1.0);

				}///for each group who owns this group tag
			}///for each group tag match this user tag
			/////////////////////////////////////////////////////////////////////////////////////////////for utag-gtag matching end


			/////////////////////////////////////////////////////////////////////////////////////////////for utag-gtitle matching begin
			for(vector<size_u>::iterator VIt = utags[utag_id].group_list.begin(); VIt != utags[utag_id].group_list.end(); VIt++) { //for each group whose title match this utag
			
				group_weight[*VIt] += w_1 * weight_utag_gtitle;
			}
			
			/////////////////////////////////////////////////////////////////////////////////////////////for utag-gtitle matching end


			/////////////////////////////////////////////////////////////////////////////////////////////for utag-utag matching begin
			
			for(vector<size_u>::iterator VIt = utags[utag_id].group_list_by_group_member.begin(); VIt != utags[utag_id].group_list_by_group_member.end(); VIt++) { //for each group whose group members match this user tag
			
				size_u group_id = *VIt;
				float w_2 = groups[group_id].utag_weight[utag_id];
				group_weight[*VIt] += w_1 * w_2 * weight_utag_g;
			}
			
			/////////////////////////////////////////////////////////////////////////////////////////////for utag-utag matching end
			
 
		}//for each user tag

		//////////////////////////////////////////////////////////////social friend based voting begin
		string channel = "social";
		for(vector<size_u>::iterator VIt = users[user_id].ratings[channel].begin(); VIt != users[user_id].ratings[channel].end(); VIt++) { ///for user's each followee
			
			size_u followee_id = *VIt;
			for(vector<size_u>::iterator VIt = users[followee_id].ratings["group"].begin(); VIt != users[followee_id].ratings["group"].end(); VIt++) { //for followee's each joined group
				
				size_u group_id = *VIt;
				float sim = (float) PCC(user_id, followee_id, "social");
				#pragma omp atomic
				group_weight[group_id] += weight_followee_rec * sim;
			}
		}
		//////////////////////////////////////////////////////////////social friend based voting end
		set<size_u> sn_set;
		copy(users[user_id].ratings[channel].begin(), users[user_id].ratings[channel].end(), std::inserter(sn_set, sn_set.begin()));
		

		//////////////////////////////////////////////////////////////CF neigborhood based voting begin
		for(std::list<std::pair<size_u, float> >::iterator LIt = users[user_id].sim_list["group"].begin(); LIt != users[user_id].sim_list["group"].end(); LIt++) {
		
			if(sn_set.count(LIt->first) ==0){ ///not find in social neighborhood
			
				size_u neighbor_id = LIt->first;
				for(vector<size_u>::iterator VIt = users[neighbor_id].ratings["group"].begin(); VIt != users[neighbor_id].ratings["group"].end(); VIt++) {
				
					size_u group_id = *VIt;
					#pragma omp atomic
					group_weight[group_id] += weight_CF_rec * LIt->second;

				}
			}
		}
		//////////////////////////////////////////////////////////////CF neigborhood based voting end
				

		list<pair<size_u,float> > recomm_pool;
		for(map<size_u, float>::iterator MIt = group_weight.begin(); MIt != group_weight.end(); MIt++) {
			
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
			string channel = "group";
			for(It = users[user_id].testset[channel].begin(); It != users[user_id].testset[channel].end(); It++) {

				#pragma omp atomic
				total_liked.total_liked[top_n]++;
						
				//if(recomm_item_set.find(MIt->first) != recomm_item_set.end() ){
				if(Recomm_item_set[top_n].count(*It) > 0){
					
					#pragma omp atomic
					hit_liked.hit_liked[top_n]++;
				}		
			}
		}

	}//if(user[user_id].sim_list.size() > 0)
	}//for each user

	cout<<"total liked #: "<< total_liked.total_liked[5]<<"\t hit #: "<<hit_liked.hit_liked[5]<<"\t hit ratio: "<<float(hit_liked.hit_liked[5])/total_liked.total_liked[5]<<endl;
	
	ofstream o1("/home/xiwangy/code/zhongdian/weibo1/weiboMF/results_tag_based.txt", ios_base::app);
	assert(o1);

	TopN topN;
	Recall recall;
	vector<int>::iterator VIt;
	o1<<"group rec:\n";
	for(VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
	
		int top_n = *VIt;
		recall.recall_total[top_n] = float(hit_liked.hit_liked[top_n])/total_liked.total_liked[top_n];
		o1<<recall.recall_total[top_n]<<"\t";
	}
	o1<<endl;


}
