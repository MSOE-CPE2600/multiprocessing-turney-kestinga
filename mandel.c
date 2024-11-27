/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "jpegrw.h"

// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax, int max, int threads );
static void show_help();
static void *compute_part(void *arg);


int mandel(int argc, char *argv[])
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	const char *outfile = "mandel.jpg";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
	int threads = 1;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:t:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 't':
				threads = atoi(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale = xscale / image_width * image_height;

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s threads=%d\n",xcenter,ycenter,xscale,yscale,max,outfile, threads);

	// Create a raw image of the appropriate size.
	imgRawImage* img = initRawImage(image_width,image_height);

	// Fill it with a black
	setImageCOLOR(img,0);

	// Compute the Mandelbrot image
	compute_image(img,xcenter-xscale/2,xcenter+xscale/2,ycenter-yscale/2,ycenter+yscale/2,max,threads);

	// Save the image in the stated file.
	storeJpegImageFile(img,outfile);

	// free the mallocs
	freeRawImage(img);

	return 0;
}




/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

struct compute_s { //pass into function for threads as a void*
	imgRawImage* img;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
	int threads;
	int thread;
};


/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int threads) //WORK HERE
{
	pthread_t thread[threads]; // use an array of threads
	struct compute_s params[threads];
	

	for (int i = 0; i < threads; i++) {//set up the parameters for the threads
		params[i].img = img;
		params[i].xmin = xmin;
		params[i].xmax = xmax;
		params[i].ymin = ymin;
		params[i].ymax = ymax;
		params[i].max = max;
		params[i].threads = threads;
		params[i].thread = i;
		int error = pthread_create(&(thread[i]), NULL, &compute_part, (void*)(&(params[i])));//create the threads
		if (error != 0) {
			perror("error creating threads");
			exit(1);
		}
	}
	for (int i = 0; i < threads; i++) { //wait for all of them to join back
		int error = pthread_join(thread[i], NULL);
		if (error != 0) {
			perror("error joining");
			exit(1);
		}
	}
}

void *compute_part(void *arg) {
	struct compute_s *params = ((struct compute_s*)(arg)); // cast back to struct pointer
	

	int width = params->img->width;
	int height = params->img->height;
	imgRawImage* img=params->img;
	double xmin=params->xmin;//get all the parameters into normal variables
	double xmax=params->xmax;
	double ymin=params->ymin;
	double ymax=params->ymax;
	int max=params->max;
	int threads=params->threads;
	int thread=params->thread;

	// printf("mandel: width=%d height=%d xmin=%lf xmax=%1f ymin=%lf ymax=%lf max=%d threads=%d thread=%d\n",width,height,xmin,xmax,ymin,ymax, max, threads, thread);

	// For every pixel in the image...


	for(int j=(int)((double)thread*height/threads); j<(int)((double)thread+1)*height/threads; j++) { // compute this part of the image

		for(int i=0; i<width; i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(img,i,j,iteration_to_color(iters,max));
		}
	}
	return 0;
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}
