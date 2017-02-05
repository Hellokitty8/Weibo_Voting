


#include "DS.h"
#include "gtag.h"
#include "utag.h"
#include "group.h"
#include "vote.h"
#include "user.h"
#include "item.h"
#include <fstream>
#include <string>
#include <sstream>
// added by miao-06-15-2016
#include <utility>
using namespace std;

void load_data();
void latent_feature_initialize();
void popularity_table_construct();
void topic_social_influence_cal();
void item_update_miao();
void user_update_miao();
bool converge();
int top_n_test_miao();

map<size_u, User> users;
map<string, map<size_u, Item> > multi_items;


// no use
void tag_based_group_rec( map<size_u, User>& users, map<size_u, Group>& groups, map<size_u, UTag>& utags, map<size_u, GTag>& gtags, map< string, map<size_u, Item> >& multi_items);
void tag_based_vote_rec( map<size_u, User>& users, map<size_u, Vote>& votes, map<size_u, UTag>& utags, map<string, map<size_u, Item> >& multi_items);
void print_result();
void sim_table_construct_user(string channel_test, string table);
void pop_among_friend(string channel);
map<size_u, UTag> utags; // key: utag_id, val: UTag class.
map<size_u, GTag> gtags; // key: gtag_id, val: GTag class.
map<size_u, Group> groups; // key: group_id, val: group class
map<size_u, Vote> votes;   // key: vote_id, val: Vote class


// added by miao-06-15-2013
map< pair<string, string>, map< size_u, map<size_u,float> > > multi_matching_score;
// added by Jia-06-09-2013
map< pair<string, string>, map< size_u, map<size_u,float> > > test_matching_score;


int main() {
	load_data();

    latent_feature_initialize();
	popularity_table_construct();
    topic_social_influence_cal();
	
   cout << "iteration is done" << endl;


    int iteration = 1;

  
   do {
        cout << "\niteration: " << iteration++ << endl;
        item_update_miao();
        user_update_miao();
    } while(!converge() && iteration < 30);
  

    top_n_test_miao();
}
 

