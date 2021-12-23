// Imagine++ project
// Project:  Panorama
// Author:   Pascal Monasse
// Date:     2013/10/08

#include <Imagine/Graphics.h>
#include <Imagine/Images.h>
#include <Imagine/LinAlg.h>
#include <vector>
#include <sstream>


using namespace Imagine;
using namespace std;


#define DRAW_R 0 //draw circle radius
#define DRAW_S 3 //draw circle stroke(line width)


// Record clicks in two images, until right button click
void getClicks(Window w1, Window w2,
               vector<IntPoint2>& pts1, vector<IntPoint2>& pts2) {
    // ------------- TODO/A completer ----------
    int button;
    int sw;
    IntPoint2 p;
    Window w_act;

    do {
        //if user click right button, jump out!
        button = anyGetMouse(p, w_act, sw);
        if(button == 3)
            break;
        //judge if user clicks in the window 1 or 2 
        if(w_act == w1){
            pts1.push_back(p);
        }
        if(w_act == w2){
            pts2.push_back(p);
        }
        //show the point where user clicks
        setActiveWindow(w_act);
        Color color; 
        color[0] = 255 * ((w_act == w1 ? pts1 : pts2).size() % 3);
        color[1] = 255 * (((w_act == w1 ? pts1 : pts2).size()+1) % 3);
        color[2] = 255 * (((w_act == w1 ? pts1 : pts2).size()+2) % 3);
        drawCircle(p, DRAW_R, color, DRAW_S);
    } while(button == 1);
}


// Return homography compatible with point matches
Matrix<float> getHomography(const vector<IntPoint2>& pts1,
                            const vector<IntPoint2>& pts2) {
    size_t n = min(pts1.size(), pts2.size());
    if(n<4) {
        cout << "Not enough correspondences: " << n << endl;
        return Matrix<float>::Identity(3);
    }
    Matrix<double> A(2*n,8);
    Vector<double> B(2*n);
    // ------------- TODO/A completer ----------
    for(unsigned int i = 0; i < n; ++i){
        unsigned int n = 2 * i;
        unsigned int m = n + 1;

        float x = pts1[i][0];
        float y = pts1[i][1];
        float x_ = pts2[i][0];
        float y_ = pts2[i][1];

        A(n,0) = x;
        A(n,1) = y;
        A(n,2) = 1;
        A(n,3) = 0;
        A(n,4) = 0;
        A(n,5) = 0;
        A(n,6) = -x_ * x;
        A(n,7) = -x_ * y;

        A(m,0) = 0;
        A(m,1) = 0;
        A(m,2) = 0;
        A(m,3) = x;
        A(m,4) = y;
        A(m,5) = 1;
        A(m,6) = -y_ * x;
        A(m,7) = -y_ * y;

        B[n] = x_;
        B[m] = y_;
    }

    B = linSolve(A, B);
    Matrix<float> H(3, 3);
    H(0,0)=B[0]; H(0,1)=B[1]; H(0,2)=B[2];
    H(1,0)=B[3]; H(1,1)=B[4]; H(1,2)=B[5];
    H(2,0)=B[6]; H(2,1)=B[7]; H(2,2)=1;

    // Sanity check
    for(size_t i=0; i<n; i++) {
        float v1[]={(float)pts1[i].x(), (float)pts1[i].y(), 1.0f};
        float v2[]={(float)pts2[i].x(), (float)pts2[i].y(), 1.0f};
        Vector<float> x1(v1,3);
        Vector<float> x2(v2,3);
        x1 = H*x1;
        cout << "sanity check" << endl;
        cout << x1[1]*x2[2]-x1[2]*x2[1] << ' '
             << x1[2]*x2[0]-x1[0]*x2[2] << ' '
             << x1[0]*x2[1]-x1[1]*x2[0] << endl;
    }
    return H;
}

// Grow rectangle of corners (x0,y0) and (x1,y1) to include (x,y)
void growTo(float& x0, float& y0, float& x1, float& y1, float x, float y) {
    if(x<x0) x0=x;
    if(x>x1) x1=x;
    if(y<y0) y0=y;
    if(y>y1) y1=y;    
}

//return the average color of c1 and c2
Color averageColor(Color& c1, Color& c2){
    Color c_avg;
    c_avg[0] = (c1[0] + c2[0]) / 2;
    c_avg[1] = (c1[1] + c2[1]) / 2;
    c_avg[2] = (c1[2] + c2[2]) / 2;
    return c_avg;
}

// Panorama construction
void panorama(const Image<Color,2>& I1, const Image<Color,2>& I2,
              Matrix<float> H) {
    Vector<float> v(3);
    float x0=0, y0=0, x1=I2.width(), y1=I2.height();
    //calculate the left-top point of I1 in plane of I2
    v[0]=0; v[1]=0; v[2]=1;
    v=H*v; v/=v[2];
    growTo(x0, y0, x1, y1, v[0], v[1]);
    //calculate the right-top point of I1 in plane of I2 
    v[0]=I1.width(); v[1]=0; v[2]=1;
    v=H*v; v/=v[2];
    growTo(x0, y0, x1, y1, v[0], v[1]);
    //calculate the right-bottom point of I1 in plane of I2 
    v[0]=I1.width(); v[1]=I1.height(); v[2]=1;
    v=H*v; v/=v[2];
    growTo(x0, y0, x1, y1, v[0], v[1]);
    //calculate the left-bottom point of I1 in plane of I2
    v[0]=0; v[1]=I1.height(); v[2]=1;
    v=H*v; v/=v[2];
    growTo(x0, y0, x1, y1, v[0], v[1]);

    cout << "x0 x1 y0 y1=" << x0 << ' ' << x1 << ' ' << y0 << ' ' << y1 <<endl;

    Image<Color> I(int(x1-x0), int(y1-y0));
    setActiveWindow( openWindow(I.width(), I.height()) );
    I.fill(WHITE);
    // ------------- TODO/A completer ----------

    //Applying homography to image (pull pixels from I1 by interpolation)
    Matrix<float> H_inv = inverse(H);
    for(unsigned int i = 0; i < I.height(); ++i){
        for(unsigned int j = 0; j < I.width(); ++j){
            Vector<float> p2(3); p2[0] = j; p2[1] = i; p2[2] = 1;//pixel in new plane
            /*---calculate the same pixel in new plane(I2) which in original I2---*/
            Vector<float> offest(3); offest[0] = x0; offest[1] = y0; offest[2] = 0;
            p2 += offest;
            p2 /= p2[2];
            /*---calculate the same pixel in new plane(I2) which in original I1---*/
            Vector<float> p1(3); //pixel in original I1
            p1 = H_inv * p2;
            p1 /= p1[2];

            /*---Skip where there is no pixel corresponding---*/
            bool inI1 = p1[0] >= 0 && p1[1] >= 0 && p1[0] < I1.width() && p1[1] < I1.height();
            bool inI2 = p2[0] >= 0 && p2[1] >= 0 && p2[0] < I2.width() && p2[1] < I2.height();
            if(inI1 && !inI2){ //original pixel in I1 but not in I2
                I(j,i) = I1.interpolate(p1[0], p1[1]);
            }
            if(inI2 && !inI1){ //original pixel in I2 but not in I1
                I(j,i) = I2.interpolate(p2[0], p2[1]);
            }
            if(inI1 && inI2){ //overlapping area (in I1 and in I2)
                Color c1 = I1.interpolate(p1[0], p1[1]);
                Color c2 = I2.interpolate(p2[0], p2[1]);
                I(j,i) = averageColor(c1, c2);
            }
        }
    }
    display(I,0,0);
}

// Main function
int main(int argc, char* argv[]) {
    const char* s1 = argc>1? argv[1]: srcPath("image0006.jpg");
    const char* s2 = argc>2? argv[2]: srcPath("image0007.jpg");

    // Load and display images
    Image<Color> I1, I2;
    if( ! load(I1, s1) ||
        ! load(I2, s2) ) {
        cerr<< "Unable to load the images" << endl;
        return 1;
    }
    Window w1 = openWindow(I1.width(), I1.height(), s1);
    display(I1,0,0);
    Window w2 = openWindow(I2.width(), I2.height(), s2);
    setActiveWindow(w2);
    display(I2,0,0);

    cout << "Click corresponding points in order and minimum 4 points in each image." << endl;
    cout << "If you finish selecting corresponding points, click right button to see panprama!" << endl;

    // Get user's clicks in images
    vector<IntPoint2> pts1, pts2;
    getClicks(w1, w2, pts1, pts2);

    vector<IntPoint2>::const_iterator it;
    cout << "pts1="<<endl;
    for(it=pts1.begin(); it != pts1.end(); it++)
        cout << *it << endl;
    cout << "pts2="<<endl;
    for(it=pts2.begin(); it != pts2.end(); it++)
        cout << *it << endl;

    // Compute homography
    Matrix<float> H = getHomography(pts1, pts2);
    cout << "H=" << H/H(2,2);

    // Apply homography
    panorama(I1, I2, H);

    endGraphics();
    return 0;
}