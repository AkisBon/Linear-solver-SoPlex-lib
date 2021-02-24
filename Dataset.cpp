#include <algorithm>
#include <iostream>
#include <cassert>
#include <cstdbool>

#include "./Dataset.hpp"
#include "./MathUtils.hpp"

using namespace std;

// Just reading the file in order to initialize the dataset
Dataset::Dataset(ifstream& stream)
{
    int length = 4, int_value, magic_number;
    int numberOfImages, rows, cols;
    char* buffer = new char[length];

    int i = 0;
    while (i < 4){
        //first 4 values are 4 bytes each
        stream.read(buffer,4);

        int* buffer2 = (int*) buffer; // read the 4 bytes as int
        int int_value = reverseInt(*buffer2);

        if (i == 0) {magic_number = int_value;}
        else if (i == 1) {numberOfImages = int_value;}
        else if (i == 2) {rows = int_value;}
        else {cols = int_value;}
        i++;
    }

    //buffer = (char*) realloc(buffer,1);
    for(int i=0; !stream.eof() ; ++i)
    {
        vector<double> img_vector;
        assert(img_vector.size() == 0);

        for(int r = 0; r < rows ; ++r)
        {
            for(int c = 0; c < cols; ++c)
            {
                stream.read(buffer,1);
                unsigned char* buffer2 = (unsigned char*) buffer;
                unsigned char pixel = buffer2[0];
                img_vector.push_back((double) pixel);
            }
        }

        assert(img_vector.size() == rows * cols);

        pictures.push_back(img_vector);
    }

    pictures.pop_back(); // Because the last element is a null

    delete[] buffer;
}


std::vector<int> Dataset::readLabels(ifstream& stream){
    typedef unsigned char uchar;

    if(stream.is_open()) {
      int magic_number = 0;
      stream.read((char *)&magic_number, sizeof(magic_number));
      magic_number = reverseInt(magic_number);

      if(magic_number != 2049) throw runtime_error("Invalid MNIST label file!");

      int number_of_labels;
      stream.read((char *)&number_of_labels, sizeof(number_of_labels)), number_of_labels = reverseInt(number_of_labels);

      uchar img_label;
      std::vector<int> labels;
      for(int i = 0; i < number_of_labels; i++) {
          stream.read((char*) &img_label, 1);
          labels.push_back((int) img_label);
      }
      return labels;
    } else {
      throw runtime_error("Unable to open file!");
    }
}

// Power factor is the greatest value of a coordinate of
// the dataset + 1 as theory suggests.
int Dataset::LSH_findPowerFactor() const
{
    int powerFactor;
    bool powerFactorSet = false;

    for(int i = 0; i < pictures.size(); i++){
        for(int j = 0; j < pictures[i].size(); j++){
            if((!powerFactorSet) || (pictures[i][j] > powerFactor)){
                powerFactorSet = true;
                powerFactor    = pictures[i][j];
            }
        }
    }

    return powerFactor + 1;
}

// This is a heuristic to find the window size that determines the
// average of NN distances of each of k points however it is not used
// now
double Dataset::LSH_getWindowSize(double (*dist_fn)(const vector<double>&, const vector<double>&),
                                const int& picts_num) const
{
    int picts_check;

    if(picts_num > this->getRows()){
        picts_check = this->getRows();
    }else{
        picts_check = picts_num;
    }

    double dist_sum = 0;
    double curr_nearest_dist = 0;

    for(int i = 0; i < picts_check; i++){
        for(int j = 0; j < picts_check; j++){
            if(i == j) continue;

            double curr_dist = dist_fn(pictures[i], pictures[j]);

            if((j == 0) || (curr_dist < curr_nearest_dist)){
                curr_nearest_dist = curr_dist;
            }
        }

        dist_sum += curr_nearest_dist;
        curr_nearest_dist = 0;
    }

    return dist_sum / ((double) picts_num);
}

// This is used to find the distances of the true k Nearest
// Neighbors
vector<double> Dataset::getKNearestDistances(double (*dist_fn)(const vector<double>&,
                                                const vector<double>&),
                                    const vector<double>& query_vec, const int& k) const
{
    vector<double> dists;

    // Holds all distances into a vector and afterwards does a sorting
    // and keeps the first k ones
    for(int i = 0; i < pictures.size(); i++){
        dists.push_back(dist_fn(query_vec, pictures[i]));
    }

    sort(dists.begin(), dists.end());

    if(dists.size() > k)
        dists.resize(k);

    return dists;
}
