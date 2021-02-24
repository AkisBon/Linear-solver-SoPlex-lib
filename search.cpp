/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the class library                   */
/*       SoPlex --- the Sequential object-oriented simPlex.                  */
/*                                                                           */
/*    Copyright (C) 1996-2019 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SoPlex is distributed under the terms of the ZIB Academic Licence.       */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SoPlex; see the file COPYING. If not email to soplex@zib.de.  */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   example.cpp
 * @brief  simple example of how to build up and solve an lp using the SoPlex callable library
 *
 * @author Ambros Gleixner
 */

#include <iostream>
#include <fstream>
#include "soplex.h"

#include <algorithm>
#include <cassert>
#include <cstdbool>
#include <chrono>

#include <bits/stdc++.h>
#include "soplex/DistanceMetrics.hpp"

//#include "soplex/Dataset.hpp"
//#include "soplex/MathUtils.hpp"

using namespace std;
using namespace soplex;


int main(int argc, char* argv[])
{

    char*  input_file_c_str      = NULL;
    char*  query_file_c_str      = NULL;
    char*  input_file_c_labels   = NULL;
    char*  query_file_c_labels   = NULL;
    char*  output_file_c_str     = NULL;

    //////////////////////// Checking the arguments ///////////////////////////

    if(argc != 12){
       cout << "Usage: ./search  -d <input file original space> -q <query file original space> " << endl;
       cout << "-l1 <labels of input dataset> -l2  <labels of query dataset> "<< endl;
       cout << "-ο <output file> -EMD" << endl;

       return 1;
    }

    // For convenience I will handle argv
    // elements as char* instead of STL string
    for(int i = 1; i < argc; i=i+2){
       if(!strcmp(argv[i], "-d")){
           if(input_file_c_str != NULL){
               cout << "Cannot give same argument twice" << endl;
               cout << "Usage: ./search  -d <input file original space> -q <query file original space> " << endl;
               cout << "-l1 <labels of input dataset> -l2  <labels of query dataset> "<< endl;
               cout << "-ο <output file> -EMD" << endl;

               return 1;
           }

           input_file_c_str = argv[i+1];
       } else if(!strcmp(argv[i], "-q")){
           if(query_file_c_str != NULL){
               cout << "Cannot give same argument twice" << endl;
               cout << "Usage: ./search  -d <input file original space> -q <query file original space> " << endl;
               cout << "-l1 <labels of input dataset> -l2  <labels of query dataset> "<< endl;
               cout << "-ο <output file> -EMD" << endl;

               return 1;
           }

           query_file_c_str = argv[i+1];
       } else if(!strcmp(argv[i], "-l1")){
            if(input_file_c_labels != NULL){
               cout << "Cannot give same argument twice" << endl;
               cout << "Usage: ./search  -d <input file original space> -q <query file original space> " << endl;
               cout << "-l1 <labels of input dataset> -l2  <labels of query dataset> "<< endl;
               cout << "-ο <output file> -EMD" << endl;

               return 1;
           }

           input_file_c_labels = argv[i+1];
       } else if(!strcmp(argv[i], "-l2")){
            if(query_file_c_labels != NULL){
               cout << "Cannot give same argument twice" << endl;
               cout << "Usage: ./search  -d <input file original space> -q <query file original space> " << endl;
               cout << "-l1 <labels of input dataset> -l2  <labels of query dataset> "<< endl;
               cout << "-ο <output file> -EMD" << endl;

               return 1;
            }

          query_file_c_labels = argv[i+1];
       } else if(!strcmp(argv[i], "-o")){
            if(output_file_c_str != NULL){
                cout << "Cannot give same argument twice" << endl;
                cout << "Usage: ./search  -d <input file original space> -q <query file original space> " << endl;
                cout << "-l1 <labels of input dataset> -l2  <labels of query dataset> "<< endl;
                cout << "-ο <output file> -EMD" << endl;

                return 1;
            }

          output_file_c_str = argv[i+1];
       } else{
            if (strcmp(argv[i], "-EMD") != 0){
                cout << "Usage: ./search  -d <input file original space> -q <query file original space> " << endl;
                cout << "-l1 <labels of input dataset> -l2  <labels of query dataset> "<< endl;
                cout << "-ο <output file> -EMD" << endl;

                return 1;
            }
       }

    }

    string input_file_path;
    string input_labels_path;
    string query_file_path;
    string query_labels_path;
    string output_file_path;

    input_file_path = (const char*) input_file_c_str;
    input_labels_path = (const char*) input_file_c_labels;
    query_file_path = (const char*) query_file_c_str;
    query_labels_path = (const char*) query_file_c_labels;
    output_file_path = (const char*) output_file_c_str;


    ifstream input_file_stream(input_file_path, ios::binary);

    ofstream output_file_stream(output_file_path, ofstream::binary);


    if(!input_file_stream.is_open()){
       cout << "Cannot open input file " << input_file_path << endl;
       return 1;
    }

    Dataset pictures(input_file_stream);
    input_file_stream.close();

    ifstream input_labels_stream(input_labels_path, std::ios::binary);

    if(!input_labels_stream.is_open()){
       cout << "Cannot open input labels file " << input_file_path << endl;
       return 1;
    }

    vector<int> labels_pic = pictures.readLabels(input_labels_stream);
    input_labels_stream.close();

    ifstream query_file_stream(query_file_path, std::ios::binary);

    if(!query_file_stream.is_open()){
       cout << "Cannot open query file " << input_file_path << endl;
       return 1;
    }

    Dataset query_pictures(query_file_stream);
    query_file_stream.close();

    ifstream query_labels_stream(query_labels_path, std::ios::binary);

    if(!query_labels_stream.is_open()){
       cout << "Cannot open query labels file " << input_file_path << endl;
       return 1;
    }

    vector<int> labels_query = pictures.readLabels(query_labels_stream);
    query_labels_stream.close();

    if(!output_file_stream.is_open()){
       cout << "Cannot open input file " << output_file_path << endl;
       return 1;
    }

    //using namespace std::chrono;

    Emd emd(pictures, query_pictures);

    //set queries and pictures to check, write info file
    string str;
    int* queries_vals;
    int* pic_vals;

    ofstream info_file_stream("info.txt", ofstream::app | ofstream::binary);

    do{
      cout << "queries to test from 1 to " << query_pictures.getRows() << " : [lower,upper]\n";
      cin >> str;
      queries_vals = extractIntegerWords(str);
    }while(queries_vals[0] < 1 && queries_vals[1] > query_pictures.getRows());
    info_file_stream << "queries checked: " << queries_vals[0] << "-" << queries_vals[1] << "\n";

    do{
      cout << "pictures to test on queries from 1 to " << pictures.getRows() << " : [lower,upper]\n";
      str.clear();
      cin >> str;
      pic_vals = extractIntegerWords(str);
    }while(pic_vals[0] < 1 && pic_vals[1] > pictures.getRows());
    info_file_stream << "pictures checked: " << pic_vals[0] << "-" << pic_vals[1] << "\n\n";


    double correct;

    vector <  vector< pair<double, int> >  >  nearests_all;
    //auto start = high_resolution_clock::now();

    nearests_all = emd.findNearests(queries_vals, pic_vals, labels_pic);

    /*auto stop = high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::seconds>(stop - start);
    cout << "Time taken by function: " << duration.count() << " seconds" << endl;*/
    int index = queries_vals[0]-1;
    int query_label = labels_query[index];

    for (int i = 0; i < nearests_all.size(); i++){
        //nearests_all[i][0].second stores query's label to check correct results
        output_file_stream << "query's label: " << query_label << "\n" <<"nearests neighbors:\n";
        output_file_stream << "<dist,label>\n";
        for (int j = 0; j < nearests_all[i].size(); j++){
           output_file_stream << nearests_all[i][j].first << "," << nearests_all[i][j].second << "  ";
           if (query_label == nearests_all[i][j].second) correct++;
        }
        output_file_stream << "\n\n";
        index++;
        query_label = labels_query[index];
    }

    output_file_stream << "\n\n";

    double emd_avg = correct/(nearests_all.size()*10);
    output_file_stream << emd_avg << "\n";

    //manhattan
    double d;
    correct = 0;
    vector <  vector< pair<double, int> >  >  nearests_manh;
    vector< pair<double, int> > nearest;

    for (int i = queries_vals[0]-1; i < queries_vals[1]; i++){
      query_label = labels_query[i];

      for (int j = pic_vals[0]-1; j < pic_vals[1]; j++)
      {
          d = 0.0;
          d = ManhattanDistance(query_pictures[i],pictures[j]);
          nearest.push_back(make_pair(d, labels_pic[j]));
      }
      sort(nearest.begin(), nearest.end());
      nearest.resize(10);
      for (int j = 0; j < nearest.size(); j++){
          if (query_label == nearest[j].second) correct++;
      }
      nearests_manh.push_back(nearest);
      nearest.clear();
    }

    double manh_avg = correct/((nearests_manh.size()*10));
    output_file_stream << manh_avg << "\n";

    output_file_stream.close();
    return 0;
}
