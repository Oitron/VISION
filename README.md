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
![image](https://github.com/Oitron/VISION/blob/main/Panorama/result_img/result.png)

****
# RANSAC algorithm for Fundamental matrix computation  
### Background:
Display **epipolar line** in an image which correspond with a point in another image.  
Using normalized **8 points** algorithm to compute the Fundamental matrix **F** , and use **RANSAC** algorithm to find the best matrix **F** with all correspond points among all interest points(find with **SIFT**).
### Usage:  
Once application start, it will display the result of **SIFT**, show the gradients of interest points. As in example, it find 1350 interest points and suppose 675 matches.  
  
![image](https://github.com/Oitron/VISION/blob/main/Fundamental_matrix/result_img/sift_result.png)  
  
After SIFT is completed, user can click left button to process the next step, which it will compute Fundamental matrix **F** and display all true correspond points(285) in 675 matches. As in example, we get 285 correspond points and matrix **F**.
  
![image](https://github.com/Oitron/VISION/blob/main/Fundamental_matrix/result_img/correspond_points.png)  
  
After get Fundamental matrix, user can click left button in left or right button, if user clicks in left image, application will display the point where user clicks(in yellow) and display the correspond epipolar line(in red) in right image, and the same way if user clicks in right image.  

![image](https://github.com/Oitron/VISION/blob/main/Fundamental_matrix/result_img/epipolar_lines_01.png)    
![image](https://github.com/Oitron/VISION/blob/main/Fundamental_matrix/result_img/epipolar_lines_02.png)  
  
****
# Disparity map computation by propagation of seeds
### Background:
Get the disparity map of two images which have already completed epipolar rectification.  
Use local method to compute disparity evaluate it by NCC to find the best disparity, then use seeds expansion to fill up the disparity map.  
### Usage:   
Once application start, it will compute first disparity map by no limited on NCC score, which is easily influenced by noise, lack of gradient variation, intensity discontinuities.  
  
![image](https://github.com/Oitron/VISION/blob/main/Disparity_map_01/result_img/first_disparity_map.png)  
  
Then it will compute disparity map by limited on NCC score (higher then 0.95), of course some places will not get a best value of disparity, these places will be filled up by next step: propagation of seeds.  
  
![image](https://github.com/Oitron/VISION/blob/main/Disparity_map_01/result_img/correct_disparity.png)  
  
After propagation of seeds we can finally get a not bad disparity map, and also it generate a 3D image to show the effect on disparity map.  
  
![image](https://github.com/Oitron/VISION/blob/main/Disparity_map_01/result_img/propagation_of_seeds.png)
![image](https://github.com/Oitron/VISION/blob/main/Disparity_map_01/result_img/result.png)  
  
****
# Disparity map estimation using Graph Cuts
### Background:
Get the disparity map of two images which have already completed epipolar rectification.  
Using global method to compute disparity by using max flow method to find minimum cut in graph to obtain the disparity map.  
### Usage:  
Increase **Lambda** from 0.1 to 0.5 [0.1, 0.25, 0.5]. The bigger value of **Lambda**, the more smoother the disparity will be, can reduce the gradient of disparity map, the mesh become more flat.  
  
![image](https://github.com/Oitron/VISION/blob/main/Disparity_map_02/result_img/res_01.png)  
  
Increase **n (patch radius)** from 3 to 7 [ 3, 5, 7]. The bigger value of **patch radius**, the less noise of disparity map will be, the more smoother surface will be.
  
![image](https://github.com/Oitron/VISION/blob/main/Disparity_map_02/result_img/res_02.png)  
  
Compare with the local method, the global method is more faster and have a more smoother result.




