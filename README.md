# VISION
Practical work on computer vision
### Testing environment:  
Linux (ubuntu 16.04 LTS)
### Dependent packages:
Imagine++ - version 4.3.2  
http://imagine.enpc.fr/~monasse/Imagine++/  

****
# Panorama  
### Background:
Get panorama from two images which takes from the same camera (Not moving the camera center but from different value of rotation).  
Using 4 or plus points correspondences in two images to estimate the homography matrix H.  
Use one image plane as new panorama plane. Pull pixels (Apply inverse matrix of H to pull from original plane in new plane) from the other original image by interpolation in new panorama image. The overlapping area will be treated as taking pixel values from two original images and calculate the average as new pixel value in panorama.
### Usage:  
User need to click minimum 4 points in each images, once finish selecting corresponding points, click right button. It will generate the panorama.
### Example:
