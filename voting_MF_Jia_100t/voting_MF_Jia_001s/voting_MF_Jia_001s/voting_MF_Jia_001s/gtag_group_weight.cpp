
///gtag's weight in group, it can be simply TF-IDF weight of gtag among all groups, or maybe simple all equal. 


#include "DS.h"
#include "gtag.h"
#include "group.h"
#include <math.h>

//utag's weight of user, TF-IDF weight or maybe simple or weight equal is more reasonable. 


extern Id_Index id_Index;	///global varaible, initialized in  data loading process

void gtag_group_weight(map<size_u, Group>& groups, map<size_u, GTag>& gtags) {

	string channel = "group";
	const int GROUP_NUM  = id_Index.id_index[channel].size(); //total user number 
	cout<<"group num: "<<GROUP_NUM<<endl;
	
	#pragma omp parallel for schedule(dynamic, 100)
	for(int i = 0; i< GROUP_NUM; i++) {
	
		size_u group_id = id_Index.id_index[channel].at(i);
		//cout<<"group id "<<group_id<<endl;

		for(map<size_u, float>::iterator MIt = groups[group_id].gtag_weight.begin();  MIt != groups[group_id].gtag_weight.end(); MIt++) {
		
			size_u tag_id = MIt->first;
			float n_k = gtags[tag_id].group_list.size();
			assert(n_k > 0);	////tag must owned by someone. 
			MIt->second = log(GROUP_NUM/n_k);
		}
	}

}
