#include "DS.h"
#include "user.h"
#include "item.h"
#include <fstream>
#include <string>
#include <set>
#include <utility>
#include <list>
using namespace std;

extern map<size_u, User> users;	///key: user_id, val: User* 
extern map< string, map<size_u, Item> > multi_items; //key: channel, val{key: item_id, val: item class}
extern Id_Index id_Index;


bool comp2(pair<size_u,float> first, pair<size_u,float> second);



void pop_among_friend(string channel){  ///testing on recommendation of channel

	Total_liked total_liked;
	Hit_liked hit_liked;
	const int USER_NUM = id_Index.id_index[Channels().channel_map[0]].size();	
	#pragma omp parallel for schedule(dynamic, 10)
	for(int u=0; u<USER_NUM; u+= 1) {

		if(u%10000==0){cout<<"user: "<<u<<endl;}	
		size_u user_id = id_Index.id_index["social"][u]; 
		if(users[user_id].testset.size() > 0){	///this in for one-class rating. 
		
			///generate top-k list for user_id
			list<pair<size_u,float> > recomm_pool; ///store all predicted item-rating pair
			map<size_u, int> item_pop;	//key: item_id, val: popularity
			//go through user_id's followees to count 
			for(vector<size_u>::iterator VIt = users[user_id].ratings["social"].begin(); VIt != users[user_id].ratings["social"].end(); VIt++) {
			
				size_u followee_id = *VIt;
				///go through followee's voting/group list
				for(vector<size_u>::iterator VIt1 = users[followee_id].ratings[channel].begin(); VIt1 != users[followee_id].ratings[channel].end(); VIt1++){
				
					item_pop[*VIt1]++;
				}
			}

			for(map<size_u, int>::iterator MIt = item_pop.begin(); MIt != item_pop.end(); MIt++) {
			
				pair<size_u, float> mypair(MIt->first, MIt->second * 1.0);
				recomm_pool.push_back(mypair);
			}
			recomm_pool.sort(comp2);
			
			map<int, set<size_u> > Recomm_item_set;	///key: top_n, val, item_id
			TopN topN;
			
			for(vector<int>::iterator VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
			
				int top_n = *VIt;
				int count = 0;
				for(list<pair<size_u,float> >::iterator LIt = recomm_pool.begin(); LIt!= recomm_pool.end() && count< top_n; LIt++, count++){	///recomm Top N items
					Recomm_item_set[top_n].insert((*LIt).first);
				}
			}
			recomm_pool.clear(); 
			///the go through current user's test set to test hit ratio
			for(vector<int>::iterator VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
				int top_n = *VIt;
				
				vector<size_u>::iterator It;
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
		}//if(users[user_id].testset.size() > 0)
	}//foreach user u done

	cout<<"total liked #: "<< total_liked.total_liked[5]<<"\t hit #: "<<hit_liked.hit_liked[5]<<"\t hit ratio: "<<float(hit_liked.hit_liked[5])/total_liked.total_liked[5]<<endl;
	//Jia 2016-09-26
	/*ofstream o1("/home/xiwangy/code/weibo/results_weiboMF.txt", ios_base::app);
	assert(o1);

	TopN topN;
	Recall recall;
	vector<int>::iterator VIt;
	for(VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
	
		int top_n = *VIt;
		recall.recall_total[top_n] = float(hit_liked.hit_liked[top_n])/total_liked.total_liked[top_n];
		o1<<recall.recall_total[top_n]<<"\t";
	}
	o1<<endl;*/

}