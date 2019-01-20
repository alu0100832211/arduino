#include <opencv2/opencv.hpp>
#include <iostream>



int main (void){
  Mat imag = imread("sans.jpg",0);

  for(int i = 0; i < 128; i++){ //Filas
    for(int j = 0; j < 128; j++){ //Columnas
      Scalar intensity = img.at<uchar>(i,j)
      if(intensity.val[0] > 100){
        printf("x");
      }
      else{
        printf(" ");
      }
    }
    printf("\n");
  }
}
