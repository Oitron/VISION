// Imagine++ project
// Project:  Fundamental
// Author:   Pascal Monasse

#include "./Imagine/Features.h"
#include <Imagine/Graphics.h>
#include <Imagine/LinAlg.h>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace Imagine;
using namespace std;

#define DRAW_R 0 //draw circle radius
#define DRAW_S 3 //draw circle stroke(line width)

static const float BETA = 0.01f; // Probability of failure

struct Match {
    float x1, y1, x2, y2;
};

// Display SIFT points and fill vector of point correspondences
void algoSIFT(Image<Color,2> I1, Image<Color,2> I2,
              vector<Match>& matches) {
    // Find interest points
    SIFTDetector D;
    D.setFirstOctave(-1);
    Array<SIFTDetector::Feature> feats1 = D.run(I1);
    drawFeatures(feats1, Coords<2>(0,0));
    cout << "Im1: " << feats1.size() << flush;
    Array<SIFTDetector::Feature> feats2 = D.run(I2);
    drawFeatures(feats2, Coords<2>(I1.width(),0));
    cout << " Im2: " << feats2.size() << flush;

    const double MAX_DISTANCE = 100.0*100.0;
    for(size_t i=0; i < feats1.size(); i++) {
        SIFTDetector::Feature f1=feats1[i];
        for(size_t j=0; j < feats2.size(); j++) {
            double d = squaredDist(f1.desc, feats2[j].desc);
            if(d < MAX_DISTANCE) {
                Match m;
                m.x1 = f1.pos.x();
                m.y1 = f1.pos.y();
                m.x2 = feats2[j].pos.x();
                m.y2 = feats2[j].pos.y();
                matches.push_back(m);
            }
        }
    }
}

//8 points algorithm
FMatrix<float, 3, 3> eight_points_algo(vector<Match>& coords){
    Matrix<float> A;
    if(coords.size() > 8){
        A = Matrix<float>(coords.size(),9);
    }else{
        A = Matrix<float>(9,9);
    }
    //compute matrix A
    for(unsigned int i=0; i<coords.size(); ++i){
        float x1  = coords[i].x1;
        float y1  = coords[i].y1;
        float x1_ = coords[i].x2;
        float y1_ = coords[i].y2;
        A(i,0) = x1_ * x1;
        A(i,1) = x1_ * y1;
        A(i,2) = x1_;
        A(i,3) = y1_ * x1;
        A(i,4) = y1_ * y1;
        A(i,5) = y1_;
        A(i,6) = x1;
        A(i,7) = y1;
        A(i,8) = 1;
    }
    //fill row 9 with 0
    if(coords.size() == 8){
        for(unsigned int i=0; i<9; ++i){
            A(8,i) = 0;
        }
    }
    //implement SVD and get last col of V_trans
    Matrix<float> U;
    Matrix<float> V_trans;
    Vector<float> D;
    svd(A, U, D, V_trans);
    FMatrix<float,3,3> F;
    int row = V_trans.nrow() - 1;
    F(0,0) = V_trans(row, 0); F(0,1) = V_trans(row, 1); F(0,2) = V_trans(row, 2);
    F(1,0) = V_trans(row, 3); F(1,1) = V_trans(row, 4); F(1,2) = V_trans(row, 5);
    F(2,0) = V_trans(row, 6); F(2,1) = V_trans(row, 7); F(2,2) = V_trans(row, 8);

    return F;
}

//Normalized 8 points algorithm
FMatrix<float, 3, 3> Normalized_8_points_algo(vector<Match>& coords){
    /*------normalized------*/
    //compute mean    
    float x1_mean = 0.0, x2_mean = 0.0, y1_mean = 0.0, y2_mean = 0.0;
    for(auto& coord : coords){
        x1_mean += coord.x1;
        y1_mean += coord.y1;
        x2_mean += coord.x2;
        y2_mean += coord.y2;
    }
    x1_mean /= coords.size();
    y1_mean /= coords.size();
    x2_mean /= coords.size();
    y2_mean /= coords.size();
    //compute scale 
    float scale1 = 0.0, scale2 = 0.0;
    for(auto& coord : coords){
        scale1 += (coord.x1 - x1_mean) * (coord.x1 - x1_mean) + (coord.y1 - y1_mean) * (coord.y1 - y1_mean);
        scale2 += (coord.x2 - x2_mean) * (coord.x2 - x2_mean) + (coord.y2 - y2_mean) * (coord.y2 - y2_mean);
    }
    scale1 = sqrt(2*coords.size()/scale1);
    scale2 = sqrt(2*coords.size()/scale2);
    //compute transform matrix
    FMatrix<float, 3, 3> trans1; trans1.fill(0);
    FMatrix<float, 3, 3> trans2; trans2.fill(0);
    trans1(0,0) = scale1;
    trans1(1,1) = scale1;
    trans1(0,2) = -x1_mean * scale1;
    trans1(1,2) = -y1_mean * scale1;
    trans2(0,0) = scale2;
    trans2(1,1) = scale2;
    trans2(0,2) = -x2_mean * scale2;
    trans2(1,2) = -y2_mean * scale2;
    //start normalized
    vector<Match> coords_normalized;
    for(auto coord : coords){
        FVector<float, 3> p (coord.x1, coord.y1, 1);
        FVector<float, 3> p_(coord.x2, coord.y2, 1);
        p = trans1 * p;
        p_= trans2 * p_;
        coord.x1 = p[0]; coord.y1 = p[1]; coord.x2 = p_[0]; coord.y2 = p_[1];
        coords_normalized.push_back(coord);
    }
    FMatrix<float, 3, 3> F = eight_points_algo(coords_normalized);
    //denormalized
    F = transpose(trans2) * F * trans1;
    return F;
}


// RANSAC algorithm to compute F from point matches (8-point algorithm)
// Parameter matches is filtered to keep only inliers as output.
FMatrix<float,3,3> computeF(vector<Match>& matches) {
    const float distMax = 1.5f; // Pixel error for inlier/outlier discrimination
    int Niter=100000; // Adjusted dynamically
    FMatrix<float,3,3> bestF;
    vector<int> bestInliers;
    // --------------- TODO ------------
    // DO NOT FORGET NORMALIZATION OF POINTS
    for(unsigned int i=0; i<Niter; ++i){
        vector<Match> coords;
        FMatrix<float, 3, 3> F;
        //choose 8 points randomly
        srand((unsigned)time(NULL)); 
        for(unsigned int i=0; i<8; ++i){
            coords.push_back(matches[rand()%matches.size()]);
        }
        //compute matrix F
        F = Normalized_8_points_algo(coords);
        //get inliers
        vector<int> inliers;
        for(unsigned int i=0; i<matches.size(); ++i){
            FVector<float, 3> I (matches[i].x1, matches[i].y1, 1); 
            I = F * I; //get epipolar's parameters
            FVector<float, 3> p_(matches[i].x2, matches[i].y2, 1); 
            //calculate distance between point e' and I(epipolar line) of e
            float A = I[0];
            float B = I[1];
            float C = I[2];
            float distance = abs(A*p_[0]+B*p_[1]+C)/sqrt(A*A+B*B);
            //if distance is small than distMax, compte as inlier
            if(distance < distMax){
                inliers.push_back(i);
            }
        }
        //update Niter and assign the bestInliers with Inliers at this etape 
        //float proba_b = 0.01;
        if(inliers.size() > bestInliers.size()){
            float m = inliers.size();
            float n = matches.size();
            Niter = (int)ceil(log(BETA)/log(1-pow(m/n,8)));
            bestInliers = inliers;
        }
    }
    //get best F based on inliers
    vector<Match> coords;
    for(unsigned int i=0; i<bestInliers.size(); ++i){
        coords.push_back(matches[bestInliers[i]]);
    }
    bestF = Normalized_8_points_algo(coords);
    // Updating matches with inliers only
    vector<Match> all=matches;
    matches.clear();
    for(size_t i=0; i<bestInliers.size(); i++)
        matches.push_back(all[bestInliers[i]]);
    return bestF;
}

// Expects clicks in one image and show corresponding line in other image.
// Stop at right-click.
void displayEpipolar(Image<Color> I1, Image<Color> I2,
                     const FMatrix<float,3,3>& F) {
    while(true) {
        int x,y;
        if(getMouse(x,y) == 3)
            break;
        // --------------- TODO ------------
        //display the point where user click
        drawCircle(x, y, DRAW_R, YELLOW, DRAW_S);
        //justifier if user clicks on left img or right img
        int x1, y1, x2, y2 = 0;
        if(x < I1.width()){ //click on left
            cout << "click in left" << endl; 
            FVector<float, 3> p(x, y, 1);
            FVector<float, 3> I;
            I = F*p;
            //determine coords of two points on epipolar line
            x1 = I1.width();
            y1 = (-I[0]*0 - I[2])/I[1];
            x2 = I2.width()+I1.width();
            y2 = (-I[0]*I2.width() - I[2])/I[1];
        }else{ //click on right
            cout << "click in right" << endl; 
            FVector<float, 3> p_(x-I1.width(), y, 1);
            FVector<float, 3> I;
            I = transpose(F)*p_;
            //determine coords of two points on epipolar line
            x1 = 0;
            y1 = (-I[0]*0 - I[2])/I[1];
            x2 = I1.width();
            y2 = (-I[0]*I1.width() - I[2])/I[1];
        }
        drawLine(x1,y1,x2,y2,RED,2,false);
    }
}

int main(int argc, char* argv[])
{
    srand((unsigned int)time(0));

    const char* s1 = argc>1? argv[1]: srcPath("im1.jpg");
    const char* s2 = argc>2? argv[2]: srcPath("im2.jpg");

    // Load and display images
    Image<Color,2> I1, I2;
    if( ! load(I1, s1) ||
        ! load(I2, s2) ) {
        cerr<< "Unable to load images" << endl;
        return 1;
    }
    int w = I1.width();
    openWindow(2*w, I1.height());
    display(I1,0,0);
    display(I2,w,0);

    vector<Match> matches;
    algoSIFT(I1, I2, matches);
    const int n = (int)matches.size();
    cout << " matches: " << n << endl;
    drawString(100,20,std::to_string(n)+ " matches",RED);
    click();
    
    FMatrix<float,3,3> F = computeF(matches);
    cout << "F="<< endl << F;

    // Redisplay with matches
    display(I1,0,0);
    display(I2,w,0);
    for(size_t i=0; i<matches.size(); i++) {
        Color c(rand()%256,rand()%256,rand()%256);
        fillCircle(matches[i].x1+0, matches[i].y1, 2, c);
        fillCircle(matches[i].x2+w, matches[i].y2, 2, c);        
    }
    drawString(100, 20, to_string(matches.size())+"/"+to_string(n)+" inliers", RED);
    click();

    // Redisplay without SIFT points
    display(I1,0,0);
    display(I2,w,0);
    displayEpipolar(I1, I2, F);

    endGraphics();
    return 0;
}
