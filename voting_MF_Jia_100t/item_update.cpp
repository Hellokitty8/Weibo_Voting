/*
 * item_update.cpp
 *
 *  Created on: Mar 30, 2011
 *      Author: eric
 */

/*
 * item_update.cpp
 *
 *  Created on: Mar 15, 2011
 *      Author: eric
 */

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include <cassert>
#include "matrix_h"
#include <string>
#include <cstring>
#include <time.h>
#include <math.h>
#include "DS.h"
#include "user.h"
#include "item.h"

using namespace std;

extern map<size_u, User> users;	///key: user_id, val: User* 
extern map< string, map<size_u, Item> > multi_items; //key: channel, val{key: item_id, val: item class}
extern Id_Index id_Index;


void item_update(string channel){	///item latent feature update for item in given channel

	/////moved out here, save computation
	//start = time(NULL);
	cout<<channel<<" update begin\n";
	float r_m = RM().rm[channel];
	//cout<<"rm "<<r_m<<endl;
	//cout<<"gamma "<<GAMMA().gamma[channel]<<endl;
	float w_m = Weights().weights[channel];
	//cout<<"weight: "<<w_m<<endl;
	const int	ITEM_NUM = id_Index.id_index[channel].size();
	//const int ITEM_NUM = ITEM_NUMS().item_num[channel];
	//cout<<"item num: "<<ITEM_NUM<<endl;


	float qq1[dim][dim] = {0};
	memset(qq1, 0, sizeof(qq1[0][0])* dim * dim);
	//cout<<"y1\n";
	
	#pragma omp parallel for schedule(dynamic, 1)
	for(int i=0; i<dim; i++) {

		for(int j=0; j< dim; j++) {

			//for(int k =0; k< USER_NUM; k++){
			for(map<size_u, User>::iterator MIt = users.begin(); MIt != users.end(); MIt++) {
				
				qq1[i][j] += MIt->second.user_latent_feature[i] * MIt->second.user_latent_feature[j]; 
			}
			qq1[i][j] *= w_m * GAMMA().gamma[channel];
		}
	}
	cout<<"qq1 "<<qq1[0][0]<<endl;
	
	cout<<"y2\n";
	#pragma omp parallel for schedule(dynamic, 100)

	for(int m = 0; m <ITEM_NUM; m++){
		//if(m%10000 ==0){cout<<"item id: "<<m<<endl;}
		size_u item_id = id_Index.id_index[channel][m]; 
		//cout<<"item id: "<<item_id<<endl;
		
		for(int n=0; n< dim; n++){multi_items[channel][item_id].item_latent_feature_previous[n] = multi_items[channel][item_id].item_latent_feature[n];}
		//cout<<"y2.09\n"<<endl; 
		///store item latent feature before update
		//if(m%100 == 0) {	cout<<"item: "<<m<<endl;}
		int item_index = m;
		vector<float> first_part(dim,0);

		vector<size_u>::iterator It;	///piont to users who rated this item m

		//cout<<"y2.1\n";
		int count = 0;
		for(It = multi_items[channel][item_id].user_list.begin(); It != multi_items[channel][item_id].user_list.end(); It++){
			//cout<<"user_id "<<*It<<"\t"<<users[*It].ratings.size() <<endl;
			//cout<<"user id: "<<*It<<"\t rating"<<users[*It].ratings[channel][item_id]<<endl;
			//if(users[*It].ratings[channel][item_id] < 1){cout<<users[*It].ratings[channel][item_id]<<"\t"<<channel<<endl;	}
			size_u user_id = *It;
			float rate_val = 0;
			if(channel.compare("social") == 0){rate_val =  users[user_id].trust_val[item_id]; }
			else{rate_val = DEFAULT_RATING;  }
			for(int k=0; k< dim; k++){

				first_part[k] += (rate_val - r_m) * users[*It].user_latent_feature[k] * GAMMA().gamma[channel] * w; 
			}
			count++;
		}
		//if(fabs(first_part[0]) > 100)
		//cout<<"first part sum: "<<first_part[0]<<"\t count "<<count<<endl;

	
		//cout<<"y3\n";
		//////
		
		float qq2[dim][dim] = {0};

		for(int i=0; i<dim; i++) {

			for(int j=0; j< dim; j++) {

				vector<size_u>::iterator It;
				
				for(It = multi_items[channel][item_id].user_list.begin(); It != multi_items[channel][item_id].user_list.end(); It++){

					qq2[i][j] += users[*It].user_latent_feature[i] * users[*It].user_latent_feature[j];
				}

				qq2[i][j] *= (w- w_m)* GAMMA().gamma[channel]; /////attetion, set w = 1 for all observed ratings
			}
		}
		//cout<<"qq2 "<<qq2[0][0]<<endl;
		//cout<<"time cost of qq2: "<<end - start<<endl;

		float qq_final[dim][dim] = {0};
		for(int i=0; i<dim; i++) {

			for(int j=0; j< dim; j++) {

				qq_final[i][j] = qq1[i][j] + qq2[i][j];
				if(i == j) {qq_final[i][j] += lamda;	}
			}
		}

		//cout<<"qq_final "<<qq_final[0][0]<<endl;
		matrix<float> M1(dim,dim);
		for(int i=0; i< dim; i++) {

			for(int j=0; j< dim; j++) {

				M1.setvalue(i,j,qq_final[i][j]);
			}
		}
		M1.invert();
		
		//cout<<"time cost of matrix invert: "<<end - start<<endl;

		vector<float>::iterator DIt;
		int col = 0;
		for(DIt = multi_items[channel][item_id].item_latent_feature.begin(); DIt != multi_items[channel][item_id].item_latent_feature.end(); DIt++, col++ ){
			*DIt = 0;
			for(int m = 0; m< dim; m++) {

				bool success;
				float value;
				//if(fabs(first_part[m] * value) > 1000){cout<<channel<<"\t"<<item_id<<"\t first part: "<<first_part[m]<<"\t val "<<value<<endl;}
				M1.getvalue(m,col,value,success);
				//cout<<"val "<<value<<endl;
				*DIt += first_part[m] * value;
			}

		}

	}


}
