
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


bool comp2(pair<size_u,float> first, pair<size_u,float> second){	////large one is in front
	
	if(first.second > second.second) return true;
		else {return false;}
}

Total_liked total_liked;
Hit_liked hit_liked;

int top_n_test(string channel) {	///top-k recomm for given channel

	const int USER_NUM = id_Index.id_index[Channels().channel_map[0]].size();	
	#pragma omp parallel for schedule(dynamic, 10)
	for(int u=0; u<USER_NUM; u+= 100) {

		if(u%10000==0){cout<<"user: "<<u<<endl;}
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
		if(users[user_id].testset.size() > 0){	///this in for one-class rating. 

			list<pair<size_u,float> > recomm_pool; ///store all predicted item-rating pair
			set<size_u> training_set(users[user_id].ratings["voting"].begin(), users[user_id].ratings["voting"].end());
			map<size_u, Item>::iterator MIt1;
			for(MIt1 = multi_items[channel].begin(); MIt1 != multi_items[channel].end(); MIt1++) { ///key:item_id, val: Item class
			
				size_u item_id = MIt1->first;
				if(training_set.count(item_id) == 0){
				
					float predict_rating = 0;
					for(int i = 0; i< dim; i++) {predict_rating += 	MIt1->second.item_latent_feature[i] * users[user_id].user_latent_feature[i];  }			
					pair<size_u, float> mypair(item_id,predict_rating);
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
			
			
		}//if(max_rate == MAX_R)
	
	}///for(int u=0; u<USER_NUM; u+= 1)

	cout<<"total liked #: "<< total_liked.total_liked[5]<<"\t hit #: "<<hit_liked.hit_liked[5]<<"\t hit ratio: "<<float(hit_liked.hit_liked[5])/total_liked.total_liked[5]<<endl;
	
	ofstream o1("/home/xiwangy/code/weibo/results_weiboMF.txt", ios_base::app);
	assert(o1);

	o1<<"wm1:"<<w_m_1<<endl;
	TopN topN;
	Recall recall;
	vector<int>::iterator VIt;
	for(VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
	
		int top_n = *VIt;
		recall.recall_total[top_n] = float(hit_liked.hit_liked[top_n])/total_liked.total_liked[top_n];
		o1<<recall.recall_total[top_n]<<"\t";
	}
	o1<<endl;
	
	return 0; 
}



