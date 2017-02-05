
#include "DS.h"
#include "user.h"
#include "utag.h"
#include "group.h"
#include <math.h>

//utag's weight of user, TF-IDF weight or maybe simple or weight equal is more reasonable. 


extern Id_Index id_Index;	///global varaible, initialized in  data loading process

void utag_user_weight(map<size_u, User>& users, map<size_u, UTag>& utags) {

	string channel = Channels().channel_map[0];
	const int USER_NUM  = id_Index.id_index[channel].size(); //total user number 
	
	#pragma omp parallel for schedule(dynamic, 100)
	for(int u = 0; u< USER_NUM; u++) {
	
		size_u user_id = id_Index.id_index[channel].at(u);
		for(map<size_u, float>::iterator MIt = users[user_id].utag_weight.begin(); MIt != users[user_id].utag_weight.end(); MIt++) {
		
			size_u tag_id = MIt->first;
			float n_k = utags[tag_id].user_list.size(); 
			assert(n_k > 0);	////tag must owned by someone. 
			MIt->second = log(USER_NUM/n_k);
			//MIt->second = 1; //better a little

		}
	}

}