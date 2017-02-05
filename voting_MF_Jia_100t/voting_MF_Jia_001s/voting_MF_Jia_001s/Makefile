#Makefile begin
CC = g++
CFLAGS = -g -c -O3 -fopenmp
INCLUDE = 
LIBS = -lmysqlclient -I/usr/include/mysql/ -L/usr/lib/mysql
OBJ = ./obj
OBJECTS = $(OBJ)/database.o $(OBJ)/dbfactory.o $(OBJ)/converge.o $(OBJ)/item_update_miao.o $(OBJ)/latent_feature_initialize.o $(OBJ)/load_data.o $(OBJ)/popularity_table_construct.o $(OBJ)/topic_social_influence_cal.o $(OBJ)/main.o $(OBJ)/user_update_miao.o $(OBJ)/top_n_test_miao.o $(OBJ)/gtag_group_weight.o $(OBJ)/utag_user_weight.o  $(OBJ)/utag_group_weight.o $(OBJ)/utag_vote_weight.o $(OBJ)/tag_based_group_rec.o $(OBJ)/tag_based_vote_rec.o $(OBJ)/print_result.o $(OBJ)/PCC.o $(OBJ)/sim_table_construct_user.o $(OBJ)/pop_among_friend.o

TARGETS = ./weiboMF

all:${TARGETS}

./weiboMF:$(OBJECTS)
	$(CC) -o $@ -fopenmp $(OBJECTS) $(INCLUDE) $(LIBS)

$(OBJECTS):$(OBJ)/%.o:%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	-rm $(OBJ)/*.o
