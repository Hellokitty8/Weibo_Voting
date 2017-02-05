
#include "DS.h"
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "gtag.h"
#include <time.h>
#include "utag.h"
#include "group.h"
#include <sys/time.h>
#include "database.h"
#include "dbfactory.h"
using namespace std;

const float SIM_THRESHOLD = 0.5;
const int SIM_ITEM_NUM = 15;

extern map<size_u, User> users;	///key: user_id, val: User* 
extern map< string, map<size_u, Item> > multi_items; //key: channel, val{key: item_id, val: item class}
extern Id_Index id_Index;

bool comp2(pair<size_u,float> first, pair<size_u,float> second);

double PCC(size_u i1, size_u i2, string channel);
	
///need calculate and store in memory then update into db. 
///to do...
void sim_table_construct_user(string channel_test, string table){	///item similiar list construct for items in channel channel

	string channel = "social";
	cout<<"sim table contruction of channel "<<channel<<"..."<< endl;
	const int ITEM_NUM = id_Index.id_index[channel].size();
	cout<<"item num "<<ITEM_NUM<<endl;

	////////////////////////////////////////////////////////////////////////////////////////////////////

	cout<<"table "<<table<<endl;

	vector<size_u> user_in_test; 
	for(int i = 0; i <ITEM_NUM; i++){

		size_u item_id = id_Index.id_index["social"][i]; 
		if(users[item_id].testset[channel_test].size() > 0){user_in_test.push_back(item_id);	}
	}
	cout<<"size "<<user_in_test.size()<<endl;
	
	const int TEST_SIZE = user_in_test.size();
	srand(0);
	const int granularity = 90;
	const int thread_num = TEST_SIZE/granularity + 1;
	
	for(int thread_index =1; thread_index <= thread_num; thread_index++) {
		
		if(thread_index% 1000 == 0){	cout<<"thread index: "<<thread_index<<endl; }
		int start_index = granularity * (thread_index -1);
		int end_index = 0;
		if(thread_index < thread_num) {	end_index =  granularity * thread_index; }
		else{end_index = TEST_SIZE; }
		vector<string> commands; ///mysql commands
		if(rand()%50 == 1){
			
			#pragma omp parallel for schedule(dynamic, 2)
			for(int i = start_index; i <end_index ; i++){
	
				size_u item_id = user_in_test.at(i);
				for(int j= 0; j< ITEM_NUM; j++) {
		
					size_u item_id2 = id_Index.id_index["social"][j]; 
					float sim = (float)PCC(item_id, item_id2, channel);
					if(sim > SIM_THRESHOLD) {	///only considering items with postive correlation

						pair<size_u,float> p1;
						p1 = make_pair(item_id2,sim);
						users[item_id].sim_list[channel_test].push_back(p1);
							
					}//if(sim)
				}///for(int j)
				
		
				std::stringstream item_id_s;
				item_id_s<< item_id;

				users[item_id].sim_list[channel_test].sort(comp2);

				list<pair<size_u, float> >::iterator LIt;
				int count = 0;
		
				string sql = "insert into " + table;
				string attr = "(UserId";
				string value = " values(" + item_id_s.str();
			
				list<pair<size_u, float> > tmp;
	
				for(LIt = users[item_id].sim_list[channel_test].begin(); LIt != users[item_id].sim_list[channel_test].end() && count < SIM_ITEM_NUM; LIt++, count++) {
		
					pair<size_u,float> mypair(LIt->first, LIt->second); ///copy the top similar users
					tmp.push_back(mypair);
					std::stringstream ss;
					ss<< count+1;
					attr += " ,SUserId" + ss.str();
					std::stringstream id_s;
					id_s << (*LIt).first;
					value += "," + id_s.str();
				}


				count = 0;
				for(LIt = users[item_id].sim_list[channel_test].begin(); LIt != users[item_id].sim_list[channel_test].end() && count < SIM_ITEM_NUM; LIt++, count++) {
					ostringstream sim_s;
					sim_s << (*LIt).second;
					std::stringstream ss;
					ss << count + 1;
					attr += " ,SimVal" + ss.str();
					value += "," + sim_s.str();
				}
		
				attr += ")";
				value += ")";
				sql += attr + value;
		
				//cout<<"sql "<<sql<<endl;
				
				users[item_id].sim_list[channel_test].clear();
				users[item_id].sim_list[channel_test] = tmp;
				
				//#pragma omp critical
				//commands.push_back(sql);
			}//for(int i)
			//cout<<"u"<<endl;

			/*
			database* db = dbfactory::createDataBase("localhost", "gbtian", "gbtian", "weibo"); 
			///batch sql exe
			for(vector<string>::iterator It = commands.begin(); It != commands.end(); It++) {
			
				///step3: update insert into table
				string sql = *It;
				if(!db->exeSQLCmd(sql)) {//dupicate key 

					if(!db->exeSQLCmd(sql)) {cout<<"error\t" <<sql<<endl;}

				}//if(!db->exe)
			}
		
			db->close();
			*/
		}
	}//for(thread_index )
	//gettimeofday(&end, NULL);
	//cout<<"time cost: "<<end.tv_sec - start.tv_sec<<endl;
	
}
