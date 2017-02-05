
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
#include "group.h"
#include "vote.h"
////this is for printing interesting intermideate results
extern map<size_u, User> users;	///key: user_id, val: User* 
extern map< string, map<size_u, Item> > multi_items;
extern map<size_u, UTag> utags;
extern map<size_u, GTag> gtags;
extern map<size_u, Group> groups;
extern map<size_u, Vote> votes; 

extern Id_Index id_Index;	///global varaible, initialized in  data loading process

void utag_vote_weight(map<size_u, Vote>& votes,  map<size_u, User>& users,  map<size_u, UTag>& utags, map< string, map<size_u, Item> >& multi_items);
void utag_group_weight(map<size_u, Group>& groups,  map<size_u, User>& users,  map<size_u, UTag>& utags, map< string, map<size_u, Item> >& multi_items);

void print_result() {

	utag_group_weight(groups,users,utags, multi_items);
	cout<<"utag group weight done\n";
	utag_vote_weight(votes,users, utags, multi_items);
	cout<<"utag group weight done\n";
	///print some interested parameters. 
	int test_size_group_rec = 0;
	int test_size_vote_rec = 0;	
	vector<size_u> with_utag_num_group_rec;
	vector<size_u> related_group_num;
	vector<size_u> with_utag_num_vote_rec;
	vector<size_u> related_vote_num;

	const int USER_NUM  = id_Index.id_index["social"].size(); //total user number 
	for(int u = 0; u< USER_NUM; u+= 1) {
	
		map<size_u, float> group_weight; //key: candidate group id for this user, val: weight for recommend
		map<size_u, float> vote_weight;
		size_u user_id = id_Index.id_index["social"].at(u);	
		if(users[user_id].testset["group"].size() > 0) {
			
			test_size_group_rec++;	
			with_utag_num_group_rec.push_back(users[user_id].utag_weight.size());

			for(map<size_u, float>::iterator MIt = users[user_id].utag_weight.begin(); MIt != users[user_id].utag_weight.end(); MIt++) { ///for each user tag
			
				size_u utag_id = MIt->first;
				for(vector<size_u>::iterator VIt = utags[utag_id].group_list_by_group_member.begin(); VIt != utags[utag_id].group_list_by_group_member.end(); VIt++) { //for each group whose group members match this user tag
			
					size_u group_id = *VIt;
					float w_2 = groups[group_id].utag_weight[utag_id];
					group_weight[*VIt] += 1;
				}
			}
			related_group_num.push_back(group_weight.size());
		}
		if(users[user_id].testset["voting"].size() > 0) {
		
			test_size_vote_rec ++;
			with_utag_num_vote_rec.push_back(users[user_id].utag_weight.size());
			for(map<size_u, float>::iterator MIt = users[user_id].utag_weight.begin(); MIt != users[user_id].utag_weight.end(); MIt++) { ///for each user tag
			
				size_u utag_id = MIt->first;
				for(vector<size_u>::iterator VIt = utags[utag_id].vote_list_by_vote_member.begin(); VIt != utags[utag_id].vote_list_by_vote_member.end(); VIt++) {
				
					size_u vote_id = *VIt;
					vote_weight[vote_id] += 1;
				}
			
			}
			related_vote_num.push_back(vote_weight.size());		
		}
		
		//if(users[user_id].utag_weight.size() > 0) {with_utag_size++; }
		//if(users[user_id].testset["group"].size() * users[user_id].utag_weight.size() > 0) {intersect_size++; }
	}
	
	assert(with_utag_num_group_rec.size() == related_group_num.size());
	ofstream o2("/home/xiwangy/code/zhongdian/weibo1/weiboMF/utag_num_group_rec.txt");
	assert(o2);
	for(int i=0; i< with_utag_num_group_rec.size(); i++) {
	
		o2<< with_utag_num_group_rec.at(i)<<"\t"<<related_group_num.at(i)<<endl;
	}

	assert(with_utag_num_vote_rec.size() == related_vote_num.size());
	ofstream o3("/home/xiwangy/code/zhongdian/weibo1/weiboMF/utag_num_vote_rec.txt");
	assert(o3);
	for(int i=0; i< with_utag_num_vote_rec.size(); i++) {
	
		o3<< with_utag_num_vote_rec.at(i)<<"\t"<<related_vote_num.at(i)<<endl;
	}

}