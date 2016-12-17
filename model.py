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
    def tol (gui):
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


    def creatword (self, origin_file, result_file, num):

    """
    @brief The function to select the subset of the important words from the words list
    @param Origin_file: The user_level words aggragated document eg: /mnt/lab/user_comments_hot.csv
    Result_file: The selected key words will be saved in this file. eg: /mnt/lab/word_b_1.txt
    Num: the number of lines in the document eg: 4836

    """

       self.count1=0
        with open(Result_file,'wb') as self.f:
            for line in open(Origin_file):
                self.count1=self.count1+1
                if self.count1<=num:
                    self.temp = line.split(',')[1]
                    if len(self.temp)>2:
                        self.wordlist=self.temp.split('"')[1].split(' ')
                        self.count=[]
                        for n in self.wordlist:
                            if n <> '':
                                self.count=self.count+[(tol(n)/self.wordlist.count(n))]
                        self.count=sorted(self.count)
                        self.thr=self.count[0]
                        if 1<len(self.count)<10:
                            self.thr=self.count[int(len(self.count)*0.5)]
                        if 10<len(self.count)<20:
                            self.thr=count[int(len(self.count)*0.75)]
                        if 30<len(self.count):
                            self.thr=self.count[int(len(self.count)*0.8)]
                        for i in self.wordlist:
                            if (tol(i)/self.wordlist.count(i))>=self.thr and i <> ' ' and  i <> '':
                                self.f.write( i+"|")
                        self.f.write("\n")
        print "finish"
        return 0
