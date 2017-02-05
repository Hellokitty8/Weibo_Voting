#ifndef DATABASE_H
#define DATABASE_H


#include <string>
#include <list>
#include <map>
#include <set>
#include <cstring>
#include <vector>
#include <stdio.h>
#include "user.h"
#include "item.h"
#include "gtag.h"
#include "utag.h"
#include "group.h"
#include "vote.h"
#include "DS.h"
#include <mysql/mysql.h>
#include <utility>
using namespace std;

extern map<size_u, User> users;	///key: user_id, val: User*
extern map< string, map<size_u, Item> > multi_items;
extern map<size_u, UTag> utags;
extern map<size_u, GTag> gtags;
extern map<size_u, Group> groups;

class database
{
    private:
        database(string server, string user, string pass, string dbname);
        MYSQL  mysql;
    public:
        friend class dbfactory;
        void close();
        MYSQL_RES* querySQLCmd(string sql);
        bool exeSQLCmd(string sql);

		int getTotalRows(string table);

		void getAllId(string table, vector<size_u>& Ids);	///get all user/item ids

		void getUserItemInteract(std::string table, string channel, std::map<size_u,User>& users, std::map<string,map<size_u,Item> >& multi_items);
		void getUserItemInteract(std::string table, string channel, std::map<size_u,User>& users, std::map<string,map<size_u,Item> >& multi_items, bool isTest);

		void getUserItemInteract(string table, int start_i, int end_i, map<size_u, User>& users, map< string, map<size_u, Item> >& multi_items );

		void getUtag(string table, map<size_u, User>& users, map<size_u, UTag>& utags);
		void getGtag(string table, map<size_u, Group>& groups, map<size_u, GTag>& gtags);
		void getUtagGtagInteract(string table, map<size_u, UTag>& utags, map<size_u, GTag>& gtags);
		void getUtagGroupInteract(string table, map<size_u, UTag>& utags, map<size_u, Group>& groups);
		void getUtagVoteInteract(string table, map<size_u, UTag>& utags, map<size_u, Vote>& votes);

		// added by miao-06-15-2016: add the definition of new member function to load matching score data
		void getMatchScore(string table, pair<string, string> pair_channel, map< pair<string, string>, map< size_u, map<size_u, float> > >& multi_matching_score);
		// added by Jia-06-15-2016: add the definition of new member function to load matching score data
		void getTestScore(string table, pair<string, string> pair_channel, map< pair<string, string>, map<size_u, map<size_u, float> > > &test_matching_score);

		// added by miao-06-28-2016: add the definition of new member function to load topic distribution of one item, such as vote, user or group
		void getTopicDist(string table, string channel, size_u item_id, pair< size_u, vector<float> >& topic_distribution);

};

#endif /*DATABASE_H */
