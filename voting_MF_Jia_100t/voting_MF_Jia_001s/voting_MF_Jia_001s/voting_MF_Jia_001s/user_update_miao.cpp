/*
 * user_update_miao.cpp
 * Created on: 06-20-2016
 * Created by: miao
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
#include <utility>
#include "DS.h"
#include "user.h"
#include "item.h"

using namespace std;

extern map<size_u, User> users;	///key: user_id, val: User*
extern map< string, map<size_u, Item> > multi_items; //key: channel, val{key: item_id, val: item class}
extern Id_Index id_Index;
//extern map<pair<string, string>, map<size_u, pair<size_u, float> > > multi_matching_score;



void user_update_miao() {//item latent feature update for vote

	/////moved out here, save computation
	//start = time(NULL);
	cout << "user update begin\n";
	float r_m = RM().rm["voting"];
	//float r_m = 0;
	//cout<<"rm "<<r_m<<endl;
	//cout<<"gamma "<<GAMMA().gamma[channel]<<endl;
	float w_m = Weights().weights["voting"];
	//cout<<"weight: "<<w_m<<endl;
	const int USER_NUM = id_Index.id_index["social"].size();
	//cout<<"user num: "<<USER_NUM<<endl;


	float zz1[dim][dim] = {0};
	memset(zz1, 0, sizeof(zz1[0][0])* dim * dim);
	//cout<<"y1\n";

	#pragma omp parallel for schedule(dynamic, 1)
	for(int i=0; i<dim; i++) {

		for(int j=0; j< dim; j++) {

			//for(int k =0; k< USER_NUM; k++){
			for(map<size_u, Item>::iterator VIt = multi_items["voting"].begin(); VIt != multi_items["voting"].end(); VIt++) {

				zz1[i][j] += VIt->second.item_latent_feature[i] * VIt->second.item_latent_feature[j];
			}
			//qq1[i][j] *= w_m * GAMMA().gamma[channel];
			zz1[i][j] *= w_m * TS_DEFAULT * TS_DEFAULT;
		}
	}
	//cout<<"qq1 "<<qq1[0][0]<<endl;

	//cout<<"y2\n";
	#pragma omp parallel for schedule(dynamic, 100)

	for(int u = 0; u < USER_NUM; u++) {
		//if(u%10000 ==0){cout<<"user id: "<<m<<endl;}
		size_u user_id = id_Index.id_index[Channels().channel_map[0]][u];
		//cout<<"user id: "<<user_id<<endl;

		for(int n = 0; n < dim; n++){
			users[user_id].user_latent_feature_previous[n] = users[user_id].user_latent_feature[n];
		}
		//cout<<"y2.09\n"<<endl;
		///store item latent feature before update
		//if(m%100 == 0) {	cout<<"item: "<<m<<endl;}
		int user_index = u;
		vector<float> first_part(dim,0);

		vector<size_u>::iterator It;	///piont to users who rated this item m

		//cout<<"y2.1\n";
		int count = 0;
		for(It = users[user_id].ratings["voting"].begin(); It != users[user_id].ratings["voting"].end(); It++){
			//cout<<"user_id "<<*It<<"\t"<<users[*It].ratings.size() <<endl;
			//cout<<"user id: "<<*It<<"\t rating"<<users[*It].ratings[channel][item_id]<<endl;
			//if(users[*It].ratings[channel][item_id] < 1){cout<<users[*It].ratings[channel][item_id]<<"\t"<<channel<<endl;	}
			size_u vote_id = *It;
			float rate_val = 0;
			//if(channel.compare("social") == 0){rate_val =  users[user_id].trust_val[item_id]; }
			//else{rate_val = DEFAULT_RATING;  }
			rate_val = DEFAULT_RATING;
			for(int k = 0; k < dim; k++) {

				//first_part[k] += (rate_val - r_m) * users[*It].user_latent_feature[k] * GAMMA().gamma[channel] * w;
				first_part[k] += (rate_val - r_m) * multi_items["voting"][vote_id].item_latent_feature[k] * w * users[user_id].ts_influence[vote_id];
			}
			count++;
		}
		//if(fabs(first_part[0]) > 100)
		//cout<<"first part sum: "<<first_part[0]<<"\t count "<<count<<endl;


		//cout<<"y3\n";
		//////

		float zz2[dim][dim] = {0};
		float zz3[dim][dim] = {0};

		for(int i=0; i<dim; i++) {

			for(int j=0; j< dim; j++) {

				vector<size_u>::iterator It;

				for(It = users[user_id].ratings["voting"].begin(); It != users[user_id].ratings["voting"].end(); It++){

					zz2[i][j] += multi_items["voting"][*It].item_latent_feature[i] * multi_items["voting"][*It].item_latent_feature[j] * users[user_id].ts_influence[*It] * users[user_id].ts_influence[*It] * w;
					zz3[i][j] += multi_items["voting"][*It].item_latent_feature[i] * multi_items["voting"][*It].item_latent_feature[j];
				}

				//qq2[i][j] *= (w- w_m)* GAMMA().gamma[channel]; /////attetion, set w = 1 for all observed ratings
				//qq2[i][j] *= (w- w_m*TS_DEFAULT)* GAMMA().gamma[channel]; /////attetion, set w = 1 for all observed ratings
				zz2[i][j] = zz2[i][j] - w_m * TS_DEFAULT * TS_DEFAULT * zz3[i][j];
			}
		}
		//cout<<"qq2 "<<qq2[0][0]<<endl;
		//cout<<"time cost of qq2: "<<end - start<<endl;

		float zz_final[dim][dim] = {0};
		for(int i=0; i<dim; i++) {

			for(int j=0; j< dim; j++) {

				zz_final[i][j] = zz1[i][j] + zz2[i][j];
				if(i == j) {zz_final[i][j] += lamda;	}
			}
		}

		//cout<<"qq_final "<<qq_final[0][0]<<endl;
		matrix<float> M1(dim,dim);
		for(int i=0; i< dim; i++) {

			for(int j=0; j< dim; j++) {

				M1.setvalue(i,j,zz_final[i][j]);
			}
		}
		M1.invert();

		//cout<<"time cost of matrix invert: "<<end - start<<endl;

		vector<float>::iterator DIt;
		int col = 0;
		for(DIt = users[user_id].user_latent_feature.begin(); DIt != users[user_id].user_latent_feature.end(); DIt++, col++ ){
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
