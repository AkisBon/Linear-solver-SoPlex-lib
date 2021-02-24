#include <string>
#include <math.h>
#include <sstream>
#include "Dataset.hpp"
#include "Emd.hpp"
#include "../soplex.h"

using namespace soplex;

Emd::Emd(Dataset pictures,Dataset query_pictures){
    originalImgDims = 28;

    do{
          cout << "give the number of clusters to split image (nxn)\n";

          string str;
          getline(cin, str);

          clustersInImg = stoi(str);

          if (originalImgDims % clustersInImg != 0) cout << "28 mod [value] must be 0\n";

    } while(originalImgDims % clustersInImg != 0);

    pixelsInCluster = originalImgDims / clustersInImg;

    vector<vector<double>> pic_vec;
    vector<vector<double>> query_vec;
    for (int i = 0; i < pictures.getRows(); i++){
         pic_vec.push_back(pictures[i]);
         if (i < query_pictures.getRows())
                query_vec.push_back(query_pictures[i]);
    }

    clusters_vec = imgclustersRead(pic_vec);
    queries_vec = imgclustersRead(query_vec);
    setWeights(pictures, query_pictures);
    groundDistance();
    convertGDtoVec(pictures);

    ofstream info_file_stream("info.txt", ofstream::app | ofstream::binary);
    info_file_stream << "cluster dimensions: " << clustersInImg << "x" << clustersInImg << "\n";
    info_file_stream << "pixels in cluster: " << pixelsInCluster << "x" << pixelsInCluster << "\n\n";
}

Emd::~Emd(){
  //cout<<"\n\nDestroy emd\n";
}

// creates clusters <==> fill img with blocks of n*n expanding row and column
vector< vector< vector<double> > > Emd::imgclustersRead(vector<vector<double>> pictures){
    vector< vector< vector<double> > > vec;
    vector<vector<double>> v(clustersInImg*clustersInImg,
                             vector<double>(pixelsInCluster*pixelsInCluster));
    vec.resize(pictures.size(), v);

    int loopRows = clustersInImg*clustersInImg;
    int loopCols = pixelsInCluster*pixelsInCluster;

    for (int n = 0; n < pictures.size(); n++){
        for (int i = 0; i < loopRows; i++){
            int row = i ;
            int start_row = i / clustersInImg;
            int pic_row = (start_row * pixelsInCluster)* originalImgDims
                                  + (row % clustersInImg) * pixelsInCluster; // img row for the cluster

            //cout << i + 1 << "th cluster:\n";
            for (int j = 0; j < loopCols; j++){
                int pic_col = pic_row + (j/pixelsInCluster) * originalImgDims;

                // find row and column for every cluster corresponding in pictures vector
                vec[n][i][j] = pictures[n][pic_col + (j%pixelsInCluster)];


            }
        }
    }
    return vec;
}


// groundDistance between 2 signatures
void Emd::groundDistance(){
    int loopRows = clustersInImg*clustersInImg;
    int loopCols = pixelsInCluster*pixelsInCluster;

    vector<vector<double>> cluster_dists;
    vector<double> distance;
    for (int n = 0; n < loopRows;/*pictures.size();*/ n++){
        for (int i = 0; i < loopRows; i++){
            int cluster1_x, cluster2_x;
            int cluster1_y, cluster2_y;
            cluster1_x = (n / clustersInImg) * pixelsInCluster + (pixelsInCluster/2);
            cluster1_y = (n % clustersInImg) * pixelsInCluster + (pixelsInCluster/2);

            cluster2_x = (i / clustersInImg) * pixelsInCluster + (pixelsInCluster/2);
            cluster2_y = (i % clustersInImg) * pixelsInCluster + (pixelsInCluster/2);

            double euclidean = sqrt(pow(cluster1_x - cluster2_x, 2.0) + pow(cluster1_y - cluster2_y, 2.0));
            distance.push_back(euclidean);
        }
        ground_dist.push_back(distance);
        distance.clear();
    }
}


void Emd::setWeights(Dataset pictures, Dataset queries){
    for (int n = 0; n < pictures.getRows(); n++){
        double pic1_weight = 0.0;
        double pic2_weight = 0.0;

        for (int i = 0; i < pictures[n].size(); i++){
              pic1_weight += pictures[n][i];
              if (n < queries.getRows()) pic2_weight += queries[n][i];
        }

        int loopRows = clustersInImg*clustersInImg;
        int loopCols = pixelsInCluster*pixelsInCluster;

        vector<double> cluster_weights1;
        vector<double> query_weights2;

        double sum1 = 0.0;
        double sum2 = 0.0;

        for (int i = 0; i < loopRows;/*pictures.size();*/ i++){
            for (int j = 0; j < loopCols; j++){
                sum1 += clusters_vec[n][i][j];
                if (n < queries.getRows()) sum2 += queries_vec[n][i][j];
            }
            cluster_weights1.push_back(sum1/pic1_weight);
            if (n < queries.getRows()) query_weights2.push_back(sum2/pic2_weight);
            sum1 = 0.0;
            sum2 = 0.0;
        }

        pictures_weights.push_back(cluster_weights1);
        if (n < queries.getRows())  queries_weights.push_back(query_weights2);
        cluster_weights1.clear();
        query_weights2.clear();
    }
}

void Emd::convertGDtoVec(Dataset pictures){
      // store gd[i][j] in one vector
      for (int i = 0; i < ground_dist.size(); i++){
         for (int j = 0; j < ground_dist[i].size(); j++){
             c_LinearSolver.push_back(ground_dist[i][j]);
         }
      }
}

vector < vector< pair<double, int> > > Emd::findNearests(int* queries_vals,int* pic_vals,
                                                         vector<int> pic_labels){
    vector< pair<double, int> > nearests_vec;
    vector< vector < pair<double, int> > > nearests_all;

    SoPlex mysoplex;
    /* set the objective sense */
    /* vars x1,x2 >= 0 (default)--constraints*/
    mysoplex.setIntParam(SoPlex::OBJSENSE,SoPlex::OBJSENSE_MINIMIZE);
    mysoplex.setIntParam(SoPlex::VERBOSITY,SoPlex::VERBOSITY_ERROR);
    //mysoplex.setIntParam(SoPlex::ALGORITHM,SoPlex::ALGORITHM_DUAL);

    /* we first add variables */
    DSVector dummycol(0);
    for (int i = 0; i < c_LinearSolver.size(); i++){
         mysoplex.addColReal(LPCol(c_LinearSolver[i], dummycol, infinity, 0.0));
    }

    for (int x = queries_vals[0]-1; x < queries_vals[1]; x++){
        mysoplex.removeRowRangeReal(0, mysoplex.numRowsReal());

        cout << "remaining queries: " << queries_vals[1] - x - 1 << "\n";
        for (int k = pic_vals[0]-1; k < pic_vals[1]; k++){

            /* then constraints one by one */
            double nearest;
            int groundDistSize = ground_dist.size();

            for (int n = 0; n < groundDistSize && k == (pic_vals[0]-1); n++){
                 //cout << "gd: " << ground_dist.size() <<"\n";
                 DSVector row1(groundDistSize); // parameter = number of columns
                 DSVector row2(groundDistSize);

                 for (int i = 0; i < groundDistSize; i++){
                      row1.add(n*groundDistSize + i, 1.0);
                      row2.add(n + groundDistSize*i, 1.0);
                 }

                 mysoplex.addRowReal(LPRow(row1, LPRowBase<Real>::EQUAL, pictures_weights[k][n]));
                 mysoplex.addRowReal(LPRow(row2, LPRowBase<Real>::EQUAL, queries_weights[x][n]));
                 //mysoplex.addRowReal(LPRow(weights[1][n], row2, weights[1][n]));

            }

            if (k > (pic_vals[0] - 1)){
               for (int i = 0; i < groundDistSize; i++){
                   mysoplex.changeLhsReal(i*2, pictures_weights[k][i]);
                   mysoplex.changeRhsReal(i*2, pictures_weights[k][i]);
               }
            }

           /* NOTE: alternatively, we could have added the matrix nonzeros in dummycol already; nonexisting rows are then
            * automatically created. */

           /* write LP in .lp format */
           mysoplex.writeFileReal("dump.lp", NULL, NULL, NULL);

           /* solve LP */
           SPxSolver::Status stat;
           DVector prim(c_LinearSolver.size());

           stat = mysoplex.optimize();
           /* get solution */
           if(stat == SPxSolver::OPTIMAL){
              nearest = mysoplex.objValueReal();
              nearests_vec.push_back(make_pair(nearest, pic_labels[k]));
           }
        }

        sort(nearests_vec.begin(), nearests_vec.end());
        nearests_vec.resize(10);
        cout << nearests_vec[0].first << "\n";
        nearests_all.push_back(nearests_vec);
        nearests_vec.clear();
    }

    return nearests_all;
}


int* extractIntegerWords(string str)
{
    int* values = (int*) malloc(2*sizeof(int));
    string::size_type sz;

    int i = 0;
    while(str[i] < '0' || str[i] > '9') i++;

    values[0] = stoi(&str[i], &sz);
    str = str.substr(sz);

    i = 0;
    while(str[i] < '0' || str[i] > '9') i++;
    values[1] = stoi(&str[i],&sz);

    return values;
}
