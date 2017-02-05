#include "DS.h"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <utility>
#include <list>
#include <math.h>
#include "user.h"
#include "item.h"
#include "gtag.h"
#include "utag.h"
#include "group.h"
#include "vote.h"
/*
calculate Perrson Correlation between two items in latent features spaces;
*/

extern map<size_u, User> users;	///key: user_id, val: User* 
extern Id_Index id_Index;

////calculate the average of a vector
double avg(vector<float> V){
	
	assert(V.size() > 0);
	double sum = 0;
	vector<float>::iterator VIt;
	for(VIt = V.begin(); VIt != V.end(); VIt++) {sum += *VIt;	}
	return (sum/V.size());
}

double PCC(size_u i1, size_u i2,string channel) {	///calculate item similarity in channel channel, currently channel is useless


	double ave1 = avg(users[i1].user_latent_feature);
	double ave2 = avg(users[i2].user_latent_feature);
	double den1 =0;	///fen mu
	double den2 = 0;
	double num = 0; ///fen zi

	for(int k =0; k< dim; k++) {
	
		num += (users[i1].user_latent_feature[k] - ave1) * (users[i2].user_latent_feature[k] - ave2);
		den1 += (users[i1].user_latent_feature[k] - ave1) * (users[i1].user_latent_feature[k] - ave1);
		den2 += (users[i2].user_latent_feature[k] - ave2) * (users[i2].user_latent_feature[k] - ave2);
	}
	double den = sqrt(den1 * den2);
	if(den > 0){

		double corr = num/den;
		return (corr);
	}
	else{return -1;	}	////den = 0, similarity value 0/0 is unknown	
	
}
