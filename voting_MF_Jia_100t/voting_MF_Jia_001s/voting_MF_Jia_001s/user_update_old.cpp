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
#include <string.h>
#include <time.h>
#include "DS.h"

using namespace std;

extern vector<User*> users;
extern vector<Item*> items;


void user_update(){

	float pp1[dim][dim] = {0};
	memset(pp1, 0, sizeof(pp1[0][0])* dim * dim);
	set<int>::iterator SIt;
	for(int i= 0; i< dim; i++) {

		for(int j=0; j< dim; j++) {

			for(int k=0; k < ITEM_NUM; k++){
				
				pp1[i][j] += items[k]->item_latent_feature.at(i) * items[k]->item_latent_feature.at(j);
			}
			pp1[i][j] *= w_m;
		}
	}
	

	#pragma omp parallel for schedule(static, 100)
	for(int u = 0; u < USER_NUM; u++){
	
		for(int n =0; n< dim; n++) {users[u]->user_latent_feature_previous[n] = users[u]->user_latent_feature[n];	}	///store user latent feature before update
				//if(u % 100 ==0)
		//{cout<<"user: "<<u<<" size "<<users[u]->ratings.size()<< endl;	}
		vector<float> first_part(dim,0);

		vector<int>::iterator It;
		map<int, int>::iterator MIt;
		for(It = users[u]->item_list.begin(); It != users[u]->item_list.end(); It++) {

			//cout<<"item:"<<*It<<"\t"<<users[u]->ratings[*It] - r_m<<endl;
			for(int k=0; k< dim; k++){

				first_part[k] += (users[u]->ratings[*It] - r_m) * items[*It -1]->item_latent_feature[k];
			}
		}
		//for(int i=0; i< dim; i++) {cout<<"first part: "<<first_part[i]<<"\t";} cout<<endl;
		//for(int m =0; m< dim; m++) {cout<<first_part[m]<<endl;}
		

		float pp2[dim][dim] = {0};

		for(int i=0; i< dim; i++) {

			for(int j=0; j< dim; j++) {

				vector<int>::iterator It;
				for(It = users[u]->item_list.begin(); It != users[u]->item_list.end(); It++) {

					pp2[i][j] += items[*It -1]->item_latent_feature[i] * items[*It -1]->item_latent_feature[j];
				}
				pp2[i][j] *= (w - w_m);
			}
		}

		//	cout<<"pp2 time cost: "<<(end1 - start1)/1000.0<<endl;
		float trace_W = w_m * ITEM_NUM + users[u]->item_list.size() * (w - w_m);
		if(trace_W == 0){trace_W =1;	}
		float pp_final[dim][dim] = {0};

		for(int i=0; i< dim; i++) {

			for(int j=0; j< dim; j++) {

				pp_final[i][j] = pp1[i][j] + pp2[i][j];
				if(i == j) {

					pp_final[i][j] += lamda * trace_W;
				}
			}
		}

			///go to calculate inverse of matrix

		matrix<float> M1(dim,dim);
		for(int i=0; i< dim; i++) {

			for(int j=0; j< dim; j++) {

				M1.setvalue(i,j,pp_final[i][j]);
			}
		}
		M1.invert();


		vector<float>::iterator DIt;
		int col = 0;
		for(DIt = users[u]->user_latent_feature.begin(); DIt !=	users[u]->user_latent_feature.end(); DIt++, col++) {

			*DIt = 0;
			for(int m = 0; m< dim; m++) {

				bool success;
				float value;
				M1.getvalue(m,col,value,success);
				*DIt += first_part[m] * value;
			}
			//if(u == 0)cout<<*DIt<<"\t";
		}


	}


}
