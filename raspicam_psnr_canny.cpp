/**
*/
#include <iostream>
#include <unistd.h>
#include <raspicam/raspicam.h>
#include "canny_local_raspicam.c"
#include "calcpsnr.c"
#include <sys/time.h>
using namespace std;

#define WIDTH 176
#define HEIGHT 144
#define NFRAME 100.0

main(int argc, char *argv[])
{
   char* dirfilename;        /* Name of the output gradient direction image */
   char outfilename[128];    /* Name of the output "edge" image */
   char composedfname[128];  /* Name of the output "direction" image */
   unsigned char *image;     /* The input image */
   unsigned char *edge;      /* The output edge image */
   int rows, cols;           /* The dimensions of the image. */
   float sigma,              /* Standard deviation of the Gaussian kernel. */
	 tlow,               /* Fraction of the high threshold in hysteresis. */
	 thigh;              /* High hysteresis threshold control. The actual
			        threshold is the (100 * thigh) percentage point
			        in the histogram of the magnitude of the
			        gradient image that passes non-maximal
			        suppression. */
   int count;            /* Frame count iterator

   /****************************************************************************
   * Get the command line arguments.
   ****************************************************************************/
   if(argc < 4){
   fprintf(stderr,"\n<USAGE> %s sigma tlow thigh [writedirim]\n",argv[0]);
      fprintf(stderr,"      sigma:      Standard deviation of the gaussian");
      fprintf(stderr," blur kernel.\n");
      fprintf(stderr,"      tlow:       Fraction (0.0-1.0) of the high ");
      fprintf(stderr,"edge strength threshold.\n");
      fprintf(stderr,"      thigh:      Fraction (0.0-1.0) of the distribution");
      fprintf(stderr," of non-zero edge\n                  strengths for ");
      fprintf(stderr,"hysteresis. The fraction is used to compute\n");
      fprintf(stderr,"                  the high edge strength threshold.\n");
      fprintf(stderr,"      writedirim: Optional argument to output ");
      fprintf(stderr,"a floating point");
      fprintf(stderr," direction image.\n\n");
      exit(1);
   }

   sigma = atof(argv[1]);
   tlow = atof(argv[2]);
   thigh = atof(argv[3]);
   rows = HEIGHT;
   cols = WIDTH;

   if(argc == 5) dirfilename = NULL;

   raspicam::RaspiCam Camera; //Camera object
   Camera.setFormat(raspicam::RASPICAM_FORMAT_GRAY);
   Camera.setCaptureSize(WIDTH, HEIGHT);

   //Open camera
   cout<<"Opening Camera..."<<endl;
   if ( !Camera.open()) {cerr<<"Error opening camera"<<endl;return -1;}

   //wait a while until camera stabilizes
   cout<<"Sleeping for 3 secs"<<endl;
   usleep(3*1000000);
   //allocate memory
   unsigned long bytes = Camera.getImageBufferSize();
   image=new unsigned char[bytes];

   count = 0;
   struct timeval start, end;
   gettimeofday(&start,NULL);
   while(count<NFRAME) {
     //capture
     Camera.grab();
     //extract the image in gray format
     Camera.retrieve ( image,raspicam::RASPICAM_FORMAT_IGNORE );//get camera image

     /****************************************************************************
     * Perform the edge detection. All of the work takes place here.
     ****************************************************************************/
     if(VERBOSE) printf("Starting Canny edge detection.\n");
     if(dirfilename != NULL){
        sprintf(composedfname, "camera_s_%3.2f_l_%3.2f_h_%3.2f_%d.fim",
        sigma, tlow, thigh,count);
        dirfilename = composedfname;
     }

     /****************************************************************************
     *Apply Canny Edge Detection Algorithm
     ****************************************************************************/
     canny(image, rows, cols, sigma, tlow, thigh, &edge, dirfilename);


     /****************************************************************************
     * Write out the edge image to a file.
     ****************************************************************************/
     sprintf(outfilename, "FRAME_%04d.pgm", count);
     if(VERBOSE) printf("Writing the edge image in the file %s.\n", outfilename);
     if(write_pgm_image(outfilename, edge, rows, cols, "", 255) == 0){
        fprintf(stderr, "Error writing the edge image, %s.\n", outfilename);
        exit(1);
     }

     /****************************************************************************
     * Write out the captured image to a file.
     ****************************************************************************/
     sprintf(outfilename, "REF_%04d.pgm", count);
     if(VERBOSE) printf("Writing the edge image in the file %s.\n", outfilename);
     if(write_pgm_image(outfilename, image, rows, cols, "", 255) == 0){
        fprintf(stderr, "Error writing the edge image, %s.\n", outfilename);
        exit(1);
     }
     count++;
   } //end of while loop

     /****************************************************************************
     * Get FPS information.
     ****************************************************************************/
   gettimeofday(&end,NULL);
   double time_elapsed =  ((end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 +start.tv_usec));
   printf("Total Elapsed Time : %lf sec.\n", time_elapsed/1000000);
   printf("FPS: %4f.\n", NFRAME/(time_elapsed/1000000));

   /****************************************************************************
   * Read the edge detected files and raw_images and calculate PSNR.
   ****************************************************************************/
   count = 0;
   char raw_image_name[128];
   char edge_image_name[128];
   double PSNR = 0.0;

   if(VERBOSE) printf("Calculating PSNR value...\n");
   while(count<NFRAME) {

     /* Get the edge image name */
     sprintf(edge_image_name, "FRAME_%04d.pgm", count);

    /* Get the raw image name */
     sprintf(raw_image_name, "REF_%04d.pgm", count);
     PSNR = PSNR + calcpsnr(raw_image_name, edge_image_name);
     count++;
     }

     PSNR = PSNR / NFRAME;
     printf("Average PSNR value = %3.2f \n", PSNR);

    //free resources
    delete image;
    return 0;
}
