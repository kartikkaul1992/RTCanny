/* Colleen Murphy
 * CS410 Multimedia Networking
 * Project 2
 * November 8, 2013
 * https://github.com/cmurphy/calcpsnr
 */

#include <stdio.h>
#include <math.h>

double psnr(double mse);
double mse(FILE * orig, FILE * eval, int total);
double calcpsnr(char* original_file, char* eval_file);

double calcpsnr(char* original_file, char* eval_file)
{
  FILE * orig_ifp = fopen(original_file, "r");
  FILE * eval_ifp = fopen(eval_file, "r");
  unsigned char tmp;
  unsigned char filetype_orig, filetype_eval;
  int xsize_orig, ysize_orig;
  int xsize_eval, ysize_eval;
  fscanf(orig_ifp, "%c%c%c", &tmp, &filetype_orig, &tmp);
  fscanf(eval_ifp, "%c%c%c", &tmp, &filetype_eval, &tmp);
  if(filetype_orig != filetype_eval) {
    fprintf(stderr, "Those aren't the same type of image.\n");
    exit(1);
  }
  fscanf(orig_ifp, "%d %d%c", &xsize_orig, &ysize_orig, &tmp);
  fscanf(eval_ifp, "%d %d%c", &xsize_eval, &ysize_eval, &tmp);
  if(xsize_orig != xsize_eval || ysize_orig != ysize_eval) {
    fprintf(stderr, "Those images aren't the same size.");
    exit(1);
  }
  fscanf(orig_ifp, "%c%c%c%c", &tmp, &tmp, &tmp, &tmp);
  fscanf(eval_ifp, "%c%c%c%c", &tmp, &tmp, &tmp, &tmp);
  int size = xsize_orig * ysize_orig;
  if(filetype_orig == '6') size *= 3;
  double mean_square_error = mse(orig_ifp, eval_ifp, size);
  double peak_signal_noise_ratio = psnr(mean_square_error);
  // printf("PSNR value is = %.2f\n", peak_signal_noise_ratio);
  return peak_signal_noise_ratio;
}

double mse(FILE * orig_ifp, FILE * eval_ifp, int size)
{
  int i;
  int sum = 0;
  for(i = 0; i < size; ++i) {
    unsigned char orig;
    unsigned char eval;
    fscanf(orig_ifp, "%c", &orig);
    fscanf(eval_ifp, "%c", &eval);
    unsigned char diff;
    if(orig > eval) {
      diff = orig - eval;
    } else {
      diff = eval - orig;
    }
    sum += diff * diff;
  }
  return sum / (double)size;
}

double psnr(double mse)
{
  return 10 * log10(255 * 255/mse);
}
