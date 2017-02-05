
#ifndef PARAS_H
#define PARAS_H

typedef unsigned int size_u;

const double GAMMA_0 = 0; ///this is weight for social channel info
const double GAMMA_1 = 1; ///this is weight for voting channel info
const double GAMMA_2 = 0; ///this is weight for group channel info
const double GAMMA_3 = 0; ///this is weight for tag channel info
//user_user number: 84,648,065
//user_vote number: 4,438,845
//user_group number: 5,954,093
//user_tag number:   1,494,413


// added by miao_06-28-2016, coefficients for calculate topic-social influence
const double ts_coefficient_1 = 1;
const double ts_coefficient_2 = 0;
const int TOPIC_K = 50;

///define all parameters here.
const double w = 1;		///weight when rating is observed in given channel
const double w_m_0 = 0;	///weight when rating is missing for social channel info
const double w_m_1 = 0.01;	///weight when rating is missing for voting channel info, density: 3.44e-06
const double w_m_2 = 0; ///weight when rating is missing for  group channel info, 1e-05
const double w_m_3 = 0;	///weight when rating is missing for  tag channel info, 2.42e-06

const double lamda = 0.5;
const int dim = 10;	///latent feature dimention
const double r_m_0 = 0;	///imputed value for social channel info
const double r_m_1 = 0; ///imputed value for voting channel info
const double r_m_2 = 0; ///imputed value for group channel info
const double r_m_3 = 0;  ///imputed value for tag channel info
const double epsino = 0.1;


// added by miao-06-20-2016
const double TS_DEFAULT = 0.01; // default value of ts_influence index for unobserved user-item interaction

const int TOP_N = 10;
const double MAX_R = 1.0;
const float DEFAULT_RATING = 1.0;
const int GRANULARITY = 30000000;   ///1M record one time

const int regular_coefficient = 5;

#endif/*PARAS_H*/
