#include "database.h"
#include "paras.h"
#include "DS.h"
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <cassert>
#include <utility>
using namespace std;

extern Id_Index id_Index;

database::database(string server, string user, string pass, string dbname)
{
    mysql_init(&mysql);
    mysql_real_connect(&mysql, server.c_str(), user.c_str(), pass.c_str(), dbname.c_str(), 3306, NULL, 0);
    mysql_set_character_set(&mysql, "gbk");
}

void database::close()
{
    mysql_close(&mysql);
    delete this;
}

MYSQL_RES* database::querySQLCmd(string sql)
{
    if (mysql_query(&mysql, sql.c_str()))
        return NULL;

    MYSQL_RES* result = mysql_store_result(&mysql);

    return result;
}

bool database::exeSQLCmd(string sql)
{
    if (mysql_query(&mysql, sql.c_str()))
        return false;
    return true;
}


int database::getTotalRows(string table)
{
    string sql = "select count(*) from " + table;
    MYSQL_RES* result = querySQLCmd(sql);
    if (result == NULL)
        return -1;

    int count = -1;
    MYSQL_ROW row;
    while(row = mysql_fetch_row(result))
    {
        sscanf(row[0], "%d", &count);
    }

    mysql_free_result(result);
	result = NULL;

    return count;
}



/////////////////////////////////////////////////////////////////////////////
///////////this is used in loading data module
void database::getAllId(string table, vector<size_u>& Ids) {

	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
    result = mysql_store_result(&mysql);

    while(row = mysql_fetch_row(result)) {

		string id = row[0];
		std::stringstream id_s(id);
		size_u id_i;
		id_s >> id_i;
		Ids.push_back(id_i);
	}
	mysql_free_result(result);
	//result = NULL;
}



void database::getUserItemInteract(string table, int start_i, int end_i, map<size_u, User>& users, map< string, map<size_u, Item> >& multi_items ){

	///write data directly into users and multi_items class, for large table split into fragments for reading, only for channel[0]--user relation
	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	std::stringstream start_s,end_s,step_s;
	start_s<<start_i;
	end_s<<end_i;
	const int step = end_i - start_i;
	step_s << step;

	sql = "select * from " + table + " limit " + start_s.str() + "," + step_s.str();
	mysql_query(&mysql, sql.c_str());
        result = mysql_store_result(&mysql);

	int count = 0;

	string channel = Channels().channel_map[0];

	while(row = mysql_fetch_row(result)) {
		string id1 = row[0];
		string id2 = row[1];

		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		size_u id1_i, id2_i;
		id1_s >> id1_i;
		id2_s >> id2_i;

		users[id1_i].ratings[channel].push_back(id2_i);
		multi_items[channel][id2_i].user_list.push_back(id1_i);
		count++;
	}
	std::cout<<"records #: "<<count<<endl;
	mysql_free_result(result);
	result = NULL;
}




void database::getUserItemInteract(std::string table, string channel, std::map<size_u,User>& users, std::map<string,map<size_u,Item> >& multi_items){

	///write data directly into users and multi_items class, read in one shot
	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
    result = mysql_store_result(&mysql);

	int count = 0;
    while(row = mysql_fetch_row(result))
    {
        string id1 = row[0];	///user id
		string id2 = row[1];	///item id
		//if(id2 != ""){

			std::stringstream id1_s(id1);
			std::stringstream id2_s(id2);
			size_u id1_i, id2_i;
			id1_s >> id1_i;
			id2_s >> id2_i;
			users[id1_i].ratings[channel].push_back(id2_i);

			//#pragma omp critical
			multi_items[channel][id2_i].user_list.push_back(id1_i);
			count++;
		//}

	}
	std::cout << "records #: "<<count<<endl;
	mysql_free_result(result);
	//result = NULL;
}

void database::getUserItemInteract(std::string table, string channel, std::map<size_u,User>& users, std::map<string,map<size_u,Item> >& multi_items, bool isTest){

	//loading test data
	///write data directly into users and multi_items class, read in one shot
	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
    result = mysql_store_result(&mysql);

	int count = 0;
    while(row = mysql_fetch_row(result))
    {
        string id1 = row[0];	///user id
		string id2 = row[1];	///item id
		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		size_u id1_i, id2_i;
		id1_s >> id1_i;
		id2_s >> id2_i;
		users[id1_i].testset[channel].push_back(id2_i);
		//#pragma omp critical
		//multi_items[channel][id2_i].user_list.push_back(id1_i); ///overfitting here
		count++;
	}
	std::cout<<"records #: "<<count<<endl;
	mysql_free_result(result);
	//result = NULL;
}

// added by miao-06-18-2016
void database::getMatchScore(string table, pair<string, string> pair_channel, map< pair<string, string>, map<size_u, map<size_u, float> > > &multi_matching_score) {

	// added by miao-06-18-2016
	// load match score data
	// write data direclty into multi_matching_score
	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
	result = mysql_store_result(&mysql);

	int count = 0;

	while(row = mysql_fetch_row(result)) {
		string id1 = row[0];
		string id2 = row[1];
		string score = row[2];

		stringstream id1_s(id1);
		stringstream id2_s(id2);
		stringstream score_s(score);
		size_u id1_i, id2_i;
		id1_s >> id1_i;
		id2_s >> id2_i;
		float score_f;
		score_s >> score_f;

		multi_matching_score[pair_channel][id1_i][id2_i] = score_f;
		count++;
	}
	std::cout << "Record #: " << count << endl;
	mysql_free_result(result);

}


//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// added by miao-06-28-2016
void database::getTopicDist(string table, string channel, size_u item_id, pair <size_u, vector<float> > &topic_distribution) {

    //pthread_mutex_lock(&mutex);

	// load particular topic distribution of either a user, a group or a vote
	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;
	string item_id_type;

	stringstream ss;
    string item_id_s;
    ss << item_id;
    ss >> item_id_s;

	if (channel == "social") {
		item_id_type = "user_id";
	} else if (channel == "voting") {
        item_id_type = "vote_id";
    } else {
        item_id_type = "group_id";
	}
	sql = "select * from " + table + " where " + item_id_type + " = " + item_id_s;
	mysql_query(&mysql, sql.c_str());
	result = mysql_store_result(&mysql);
	vector<float> prob_list;

	//cout << sql << endl;

	if (mysql_num_rows(result) != 0) {
        row = mysql_fetch_row(result);
        string distribution = row[1];
        stringstream distribution_s(distribution);
        for (float dist_prob; distribution_s >> dist_prob;) {
            prob_list.push_back(dist_prob);
        }
	} else {
        for (int i = 0; i < 50; i++) {
            prob_list.push_back(1.0 / TOPIC_K);
        }
	}
	topic_distribution = make_pair(item_id, prob_list);

	//cout << sql << "haha" << endl;

	mysql_free_result(result);

	//pthread_mutex_unlock(&mutex);
}




// added by Jia-09-06-2016
void database::getTestScore(string table, pair<string, string> pair_channel, map< pair<string, string>, map<size_u, map<size_u, float> > > &test_matching_score) {

	// added by Jia-09-06-2016
	// load test score data
	// write data direclty into test_matching_score
	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
	result = mysql_store_result(&mysql);

	int count = 0;
        std::cout << "here!!! " << count << endl;
	while(row = mysql_fetch_row(result)) {
		string id1 = row[0];
		string id2 = row[1];
		string score = row[2];

		stringstream id1_s(id1);
		stringstream id2_s(id2);
		stringstream score_s(score);
		size_u id1_i, id2_i;
		id1_s >> id1_i;
		id2_s >> id2_i;
		float score_f;
		score_s >> score_f;

		test_matching_score[pair_channel][id1_i][id2_i] = score_f;
                		count++;
	}
        std::cout << "here: " << endl;
	std::cout << "Record #: " << count << endl;


	mysql_free_result(result);

}







void database::getUtag(std::string table, std::map<size_u,User> &users, std::map<size_u,UTag> &utags){

	///load utag info

	MYSQL_RES* result;
	string sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
	cout<<"y1"<<endl;
    result = mysql_store_result(&mysql);
	int count = 0;
	cout<<"y2"<<endl;
    while(MYSQL_ROW row = mysql_fetch_row(result))
    {
        string id1 = row[0];	///user id
		string id2 = row[1];	///tag id
		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		size_u id1_i, id2_i;
		id1_s >> id1_i;
		id2_s >> id2_i;
		users[id1_i].utag_weight[id2_i] = 1.0;  ///assign initial weight to be 1.
		utags[id2_i].user_list.push_back(id1_i);
		count++;
	}
	std::cout<<"records #: "<<count<<endl;
	mysql_free_result(result);
}

void database::getGtag(std::string table, std::map<size_u,Group> &groups, std::map<size_u,GTag> &gtags){

	///load gtag info
	MYSQL_RES* result;
	string sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
	cout<<"y1"<<endl;
    result = mysql_store_result(&mysql);
	int count = 0;
	cout<<"y2"<<endl;
    while(MYSQL_ROW row = mysql_fetch_row(result))
    {
        string id1 = row[0];	///user id
		string id2 = row[1];	///tag id
		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		size_u id1_i, id2_i;
		id1_s >> id1_i;
		id2_s >> id2_i;
		groups[id1_i].gtag_weight[id2_i] = 1.0;  ///assign initial weight to be 1.
		gtags[id2_i].group_list.push_back(id1_i);
		count++;
	}
	std::cout<<"records #: "<<count<<endl;
	mysql_free_result(result);

}

void database::getUtagGtagInteract(std::string table, std::map<size_u,UTag> &utags, std::map<size_u,GTag> &gtags){

	///load utag and gtag matching info
	MYSQL_RES* result;
	string sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
	cout<<"y1"<<endl;
    result = mysql_store_result(&mysql);
	int count = 0;
	cout<<"y2"<<endl;
    while(MYSQL_ROW row = mysql_fetch_row(result))
    {
        string id1 = row[0];	///user tag id
		string id2 = row[1];	///group tag id
		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		size_u id1_i, id2_i;

		id1_s >> id1_i;
		id2_s >> id2_i;

		utags[id1_i].gtag_list.push_back(id2_i);
		gtags[id2_i].utag_list.push_back(id1_i);
		count++;
	}
	std::cout<<"records #: "<<count<<endl;
	mysql_free_result(result);
}

void database::getUtagGroupInteract(std::string table, std::map<size_u,UTag> &utags, std::map<size_u,Group> &groups){

	///load utag and group matching info
	MYSQL_RES* result;
	string sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
	cout<<"y1"<<endl;
    result = mysql_store_result(&mysql);
	int count = 0;
	cout<<"y2"<<endl;
    while(MYSQL_ROW row = mysql_fetch_row(result))
    {
        string id1 = row[0];	///user tag id
		string id2 = row[1];	///group id
		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		size_u id1_i, id2_i;

		id1_s >> id1_i;
		id2_s >> id2_i;

		utags[id1_i].group_list.push_back(id2_i);
		groups[id2_i].utag_list.push_back(id1_i);
		count++;
	}
	std::cout<<"records #: "<<count<<endl;
	mysql_free_result(result);

}

void database::getUtagVoteInteract(std::string table, std::map<size_u,UTag> &utags, std::map<size_u,Vote> &votes){

	///load utag and vote matching info
	MYSQL_RES* result;
	string sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
	cout<<"y1"<<endl;
    result = mysql_store_result(&mysql);
	int count = 0;
	cout<<"y2"<<endl;
    while(MYSQL_ROW row = mysql_fetch_row(result))
    {
        string id1 = row[0];	///user tag id
		string id2 = row[1];	///vote id
		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		size_u id1_i, id2_i;

		id1_s >> id1_i;
		id2_s >> id2_i;
		utags[id1_i].vote_list.push_back(id2_i);
		votes[id2_i].utag_list.push_back(id1_i);
		count++;
	}
	std::cout<<"records #: "<<count<<endl;
	mysql_free_result(result);
}
