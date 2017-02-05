/*
 * user_update.cpp
 *
 *  Created on: Mar 30, 2011
 *      Author: eric
 */


#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include <cassert>
#include "matrix_h"
#include <cstring>
#include <string>
#include <utility>
#include <math.h>
#include <time.h>
#include "DS.h"
#include "user.h"
#include "item.h"

using namespace std;

extern map<size_u, User> users;	///key: user_id, val: User* 
extern map< string, map<size_u, Item> > multi_items; //key: channel, val{key: item_id, val: item class}
extern Id_Index id_Index;
	
class AA {
public:
	float aa[dim][dim];
	AA(){memset(aa, 0, sizeof(float)* dim * dim);}
};

void user_update(){

	cout<<"user update begin..\n";
	map<string, AA> multi_zz;	///this is the multi-channel Z^{T}Z (item latent feature matrix production) result
	///initialize all channel item latent feature production result
	//Channels channels;
	
	{
		//map<int, string>::iterator MIt;
		map< string, map<size_u, Item> >::iterator MIt;
		//for(MIt = channels.channel_map.begin(); MIt != channels.channel_map.end(); MIt++) {
		for(MIt =multi_items.begin(); MIt != multi_items.end(); MIt++){

			multi_zz[MIt->first] = AA();
		}
	}
	
	//cout<<"y1"<<endl;
	/*AA zz0; multi_zz[channels.channel_map[0]] = zz0;
	AA zz1;	multi_zz[channels.channel_map[1]] = zz1;
	AA zz2; multi_zz[channels.channel_map[2]] = zz2;
	AA zz3; multi_zz[channels.channel_map[3]] = zz3;*/
	///////////////////////initializing is done////////////////////////////////////////////
	

	float zz_sum[dim][dim] = {0};	///this is for summation of all Z^{T}Z result
		
	map<string,AA>::iterator MItzz;
	for(MItzz = multi_zz.begin(); MItzz != multi_zz.end(); MItzz++) {

		string channel = MItzz->first;
		cout<<"channle "<<channel<<endl;
		#pragma omp parallel for schedule(dynamic, 1)
		for(int i = 0; i< dim; i++) {

			#pragma omp parallel for schedule(static, 1)
			for(int j=0; j< dim; j++) {

				for(map<size_u, Item>::iterator MIt = multi_items[channel].begin(); MIt != multi_items[channel].end(); MIt++){
				
					MItzz->second.aa[i][j] += MIt->second.item_latent_feature[i] * MIt->second.item_latent_feature[j]; 
				}
				MItzz->second.aa[i][j] *= Weights().weights[channel] * GAMMA().gamma[channel];
			}
		}
		//cout<<MItzz->second.aa[0][0]<<endl;
		for(int i = 0; i< dim; i++) {
		
			for(int j=0; j< dim; j++) {zz_sum[i][j] += MItzz->second.aa[i][j];	}
		}

	}
	
	//cout<<"y2"<<endl;

	string channel = Channels().channel_map[0];
	const int	ITEM_NUM = id_Index.id_index[channel].size();
	#pragma omp parallel for schedule(dynamic, 100)
	for(int u = 0; u < ITEM_NUM; u++){
	
		if(u%200000 ==0){cout<<"user id :"<<u<<endl;}
		size_u user_id = id_Index.id_index["social"][u]; 
		//for(int n =0; n< dim; n++) {users[u]->user_latent_feature_previous[n] = users[u]->user_latent_feature[n];	}	///store user latent feature before update
		for(int n =0; n< dim; n++) {users[user_id].user_latent_feature_previous[n] = users[user_id].user_latent_feature[n];	}	///store user latent feature before update
		
		vector<float> first_part_sum(dim,0);
		map<string, map<size_u, Item> >::iterator MIt;
		for(MIt = multi_items.begin(); MIt != multi_items.end(); MIt ++) {
			vector<float> first_part(dim,0);
			string channel = MIt->first;
			
			vector<size_u>::iterator RIt;
			for(RIt = users[user_id].ratings[channel].begin(); RIt != users[user_id].ratings[channel].end(); RIt++) {
			
				size_u item_id = *RIt;
				
				float rate_val = 0;
				if(channel.compare("social") == 0){rate_val =  users[user_id].trust_val[item_id]; }
				else{rate_val = DEFAULT_RATING;  }

				for(int k = 0; k< dim; k++) {
				
					first_part[k] += (rate_val - RM().rm[channel]) * multi_items[channel][item_id].item_latent_feature[k] * GAMMA().gamma[channel] * w;
					//if(users[user_id].ratings[channel][item_id] < 1){cout<<channel<<endl;	}
				}
			}

			for(int k = 0; k< dim; k++) {first_part_sum[k] += first_part[k];	}
			
		}
		
		//if(fabs(first_part_sum[0]) > 100){			cout<<"first part: "<<first_part_sum[0]<<endl;}
		
		
		

		///this is the multi-channel sub Z^{T}Z (item latent feature matrix production) result
		map<string, AA> multi_zz2;	///key:channel, val:float[dim][dim]
		//Channels channels;
		///initialize all channel item latent feature production result
		{
			//map<int, string>::iterator MIt;
			for(map< string, map<size_u, Item> >::iterator MIt = multi_items.begin(); MIt != multi_items.end(); MIt++){	
			//for(MIt = channels.channel_map.begin(); MIt != channels.channel_map.end(); MIt++) {

				multi_zz2[MIt->first] = AA();
			}
		}
		/*AA zz0;	multi_zz2[channels.channel_map[0]] = zz0;
		AA zz1;	multi_zz2[channels.channel_map[1]] = zz1;
		AA zz2;	multi_zz2[channels.channel_map[2]] = zz2;
		AA zz3;	multi_zz2[channels.channel_map[3]] = zz3;
		*/
		///////////////////////////////////////////initialization is done


		float zz2_sum[dim][dim] = {0};	///this is for summation of all Z^{T}Z result
		map<string, AA>::iterator MItzz;
		for(MItzz = multi_zz2.begin(); MItzz != multi_zz2.end(); MItzz ++) {
		
			string channel = MItzz->first;
			for(int i=0; i< dim; i++) {

				for(int j=0; j< dim; j++) {

					vector<size_u>::iterator RIt;  ///RIt for rating
					for(RIt = users[user_id].ratings[channel].begin(); RIt != users[user_id].ratings[channel].end(); RIt++) {

						size_u item_id = *RIt;
						multi_zz2[channel].aa[i][j] += multi_items[channel][item_id].item_latent_feature[i] * multi_items[channel][item_id].item_latent_feature[j];
					}
					multi_zz2[channel].aa[i][j] *= (w - Weights().weights[channel]) * GAMMA().gamma[channel];
					zz2_sum[i][j] += multi_zz2[channel].aa[i][j]; 
				}
			}
		}
		
		//	cout<<"pp2 time cost: "<<(end1 - start1)/1000.0<<endl;

		float zz_final[dim][dim] = {0};
		
		for(int i=0; i< dim; i++) {

			for(int j=0; j< dim; j++) {

				zz_final[i][j] = zz_sum[i][j] + zz2_sum[i][j];
				if(i == j) {

					zz_final[i][j] += lamda;
				}
			}
		}
		
		///go to calculate inverse of matrix

		matrix<float> M1(dim,dim);
		for(int i=0; i< dim; i++) {

			for(int j=0; j< dim; j++) {

				M1.setvalue(i,j,zz_final[i][j]);
			}
		}
		M1.invert();

		vector<float>::iterator DIt;
		int col = 0;
		for(DIt = users[user_id].user_latent_feature.begin(); DIt !=	users[user_id].user_latent_feature.end(); DIt++, col++) {

			*DIt = 0;
			for(int m = 0; m< dim; m++) {

				bool success;
				float value;
				M1.getvalue(m,col,value,success);
				*DIt += first_part_sum[m] * value;
			}
			//if(u == 0)cout<<*DIt<<"\t";
		}
	}


}
