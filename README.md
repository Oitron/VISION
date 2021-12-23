# VISION
Practical work on computer vision
### Testing environment:  
Linux (ubuntu 16.04 LTS)
### Dependent packages:
Imagine++ - version 4.3.2  
http://imagine.enpc.fr/~monasse/Imagine++/  

****
# Panorama construction  
### Background:
Get panorama from two images which takes from the same camera (Not moving the camera center but from different value of rotation).  
Using 4 or plus points correspondences in two images to estimate the homography matrix **H**.  
Use one image plane as new panorama plane. Pull pixels (Apply inverse matrix of **H** to pull from original plane in new plane) from the other original image by interpolation in new panorama image. The overlapping area will be treated as taking pixel values from two original images and calculate the average as new pixel value in panorama.
### Usage:  
User need to click minimum 4 points in each images, once finish selecting corresponding points, click right button. It will generate the panorama.
### Example:
![image](https://github.com/Oitron/VISION/blob/main/Panorama/result.png)

****
# RANSAC algorithm for Fundamental matrix computation  
### Background:
Display **epipolar line** in an image which correspond with a point in another image.  
Using normalized **8 points** algorithm to compute the Fundamental matrix **F** , and use **RANSAC** algorithm to find the best matrix **F** with all correspond points among all interest points(find with **SIFT**).
### Usage:  
Once application start, it will display the result of SIFT, show the gradients of interest points. As in example, it find 1350 interest points and suppose 675 matches.  

