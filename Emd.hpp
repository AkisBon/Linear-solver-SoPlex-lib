#ifndef __EMD__
#define __EMD__

#include <iostream>
#include <vector>
#include "Dataset.hpp"

using namespace std;

class Emd{
private:
    int clustersInImg;
    int pixelsInCluster;
    int originalImgDims;

    vector< vector< vector<double> > >    clusters_vec;
    vector< vector< vector<double> > >    queries_vec;
    vector< vector<double> >     pictures_weights;
    vector< vector<double> >     queries_weights;
    vector< vector<double> >     ground_dist;

    vector<double> c_LinearSolver; // store gd[i][j] in one vector

public:
    Emd(Dataset pictures,Dataset query_pictures); // read parameters
    ~Emd();
    vector< vector< vector<double> > > imgclustersRead(vector<vector<double>>);
    void groundDistance();
    void setWeights(Dataset, Dataset);
    vector < vector< pair<double, int> > > findNearests(int*, int*, vector<int>);
    void convertGDtoVec(Dataset);
};

int* extractIntegerWords(string);

#endif
