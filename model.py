# -*- coding: utf-8 -*-
__author__= 'WANG jia<bjtuwangjia@gmail.com>'
__date__ = '12/17/2016'

#author:V
from decimal import *
import re
import math

class Model:

    """
    This is model to analysis the sismilarity of each user based on voting they have participate.

    After cutting the votes into bag of words and aggregate them into an user-level document
    .A user may have a very long list of words. The most important words will be selected
    by the creatword function. The outputs reduced the number of words in the list, which still
    can pefect represrent the semantic meaning of the user level perferences.

    model digram:

         ---------           |           ----------           ------------           ------------       -----------------
    ----|  word2vt |-------- |----------|  creatword | ------| list generate | ------|list ID   |-------|Cosine distance|
         ---------           |           ----------           ------------            ------------       ----------------
                             |



    e.g. This model used to caulate the similarity of the users who have participated the voting
    based on the semantic analysis in their user-level aggregated voting document.

    Dependency: /mnt/lab/data1.txt

    """

    def __init__(self, origin_file1, origin_file2, result_file1, result_file2, result_file3, result_file4, num):
    """

    @brief The function to select the subset of the important words from the words list
    @param Origin_file: The user_level words aggragated document eg: /mnt/lab/user_comments_hot.csv
    Result_file: The selected key words will be saved in this file. eg: /mnt/lab/word_b_1.txt
    Num: the number of lines in the document eg: 4836

    """
        #load the config
        #the input data
        self.origin_file1 = origin_file1
        self.origin_file2 = origin_file2
        self.result_file1 = result_file1
        self.result_file2 = result_file2
        self.result_file3 = result_file3
        self.result_file4 = result_file4
        self.num = num

    def tol (self，gui):
    """
    @brief The counter to caculate the number of 'gui' word in the given document
    @param gui: The target word thta to be found in the document
    """
        patt = re.compile(gui)

        f = open('/mnt/lab/data1.txt','r') # the all votng words document.

        try:
            return len(patt.findall(f.read())) #findall接受str类型，
        finally:                                #不管结果如何，都会执行finally模块的语句
            f.close()


    def creatword (self):

    """
    @brief The function to select the subset of the important words from the words list
    @param Origin_file: The user_level words aggragated document eg: /mnt/lab/user_comments_hot.csv
    Result_file: The selected key words will be saved in this file. eg: /mnt/lab/word_b_1.txt
    Num: the number of lines in the document eg: 4836

    """
        count1=0
        with open(self.origin_file,'wb') as f:
            for line in open(self.result_file1):
                count1=count1+1
                if count1<=self.num:
                    temp = line.split(',')[1]
                    if len(temp)>2:
                        wordlist=temp.split('"')[1].split(' ')
                        count=[]
                        for n in wordlist:
                            if n <> '':
                                count=count+[(tol(n)/wordlist.count(n))]
                        count=sorted(count)
                        thr=count[0]
                        if 1<len(count)<10:
                            thr=count[int(len(count)*0.5)]
                        if 10<len(count)<20:
                            thr=count[int(len(count)*0.75)]
                        if 30<len(count):
                            thr=count[int(len(count)*0.8)]
                        for i in wordlist:
                            if (tol(i)/wordlist.count(i))>=thr and i <> ' ' and  i <> '':
                                f.write( i+"|")
                        f.write("\n")
        print "finish creatword"
        return 0


    def creatlist (self):

    """
    @brief The function to select the subset of the important words from the words list
    @param Origin_file: The user_level words aggragated document eg: /mnt/lab/user_comments_hot.csv
    Result_file: The selected key words will be saved in this file. eg: /mnt/lab/word_b_1.txt
    Num: the number of lines in the document eg: 4836

    """

        with open(self.result_file2,'wb') as f:
            for line in open(self.result_file1):
                count=0
                if count<self.num:
                    count=count+1
                    lis = line.split('|')
                    lis_len=len(lis)
                    del (lis[lis_len-1])

                    word_list=[]
                    for lis_word in lis:
                        b=a.loc[a['word'] == lis_word]
                        if len(b)==0:
                            word_list_1=[0.1]*9

                        else:

                            for name in range(10):
                                name=name+1
                                word_list+=[b.iloc[0][name]]


                            myarray = np.asarray(word_list)
                            temp_list=myarray[0:10]
                            print temp_list

                            for oo in range((len(word_list)/10)):
                                array=myarray[10*oo:10*(oo+1)]
                                temp_list=array+temp_list
                                print array
                            word_list_1= temp_list.tolist()
                    f.write(str(word_list_1)+"\n")

        print "finish list"
        return 0


    def list_id (self):

    """
    @brief The function is to append the id to the words list

    """

        a=pd.read_csv(self.origin_file12,sep='"')
        b=pd.read_csv(self.result_file2,sep='[')
        #b=a[list(range(a.shape[1]-1))]

        print "list_id"
        d=len(a)
        print "list_id:"+d


        with open(self.result_file3,'wb') as f:
            for i in range(181448):
                id_1=a.iloc[i][0]
                dis=b.iloc[i][0]
                dis=dis.split(']')[0]
                if i<3:
                    print id_1
                    print dis
                f.write(str(id_1) + ";" + str(dis) + "\n")

        print "finish list_id"
        return 0




    def distance (self):

    """
    @brief The final function to caculate the diatance
    """
        a=pd.read_csv(self.result_file3,sep=';')
        #b=pd.read_csv('C:/Users/hkpuadmin/Desktop/origin.csv',sep=',')

        with open(self.result_file4,'wb') as f:
            for n in range(len(a)):
                dataSetI = a.iloc[n][1].split(',')
                SetI=[float(i) for i in dataSetI]
                ID_1=a.iloc[n][0]

                for j in range(n+1,len(a)):
                    dataSetII =a.iloc[j][1].split(',')
                    SetII=[float(i) for i in dataSetII]
                    plain_list=[0.1]*200
                    if (SetI<>plain_list) and (SetII<>plain_list):
                        result = (2 - spatial.distance.cosine(SetI, SetII))/2

                    if (result==1):

                        result=0.98
                    ID_2=a.iloc[j][0]

                    if result>0.91:
                        #c1=b.loc[b['id'] ==ID_1].iloc[0][1]

                        #c2=b.loc[b['id'] ==ID_2].iloc[0][1]

                        f.write(str(ID_1)+";"+str(ID_2)+";"+str(result)+"\n")

        print "finish distance"
        return 0
