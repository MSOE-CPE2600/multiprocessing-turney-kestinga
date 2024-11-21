/* Lab 11 - Multiprocessing
 * 11/20/2024
 * Abe Kesting
 * CPE 2600 111
 */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include "mandel.h"

int main(int argc, char *argv[]) {
    char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	char* xcenter = "-0.102";
	char* ycenter = "0.88";
	double xscale = .1;
	char*    image_width = "1000";
	char*    image_height = "1000";
	char*    max = "1000";
    int children = 1;
	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:n:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = optarg;
				break;
			case 'y':
				ycenter = optarg;
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = optarg;
				break;
			case 'H':
				image_height = optarg;
				break;
			case 'm':
				max = optarg;
				break;
            case 'n':
                children = atoi(optarg);
                break;
			case 'h':
				exit(1);
				break;
		}

	}

    sem_t *sem = mmap(NULL, sizeof(void*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sem == MAP_FAILED) {
        perror("mmap fail");
        exit(1);
    }
    //create semaphore in anonymous shared memory with a starting count of the amount of children
    int error = sem_init(sem, 1, children);
    //open the nameless semaphore
    if (error == -1) {
        perror("semaphore fail");
        exit(1);
    }

    //set up the argument things to put into the given code
    int newargc = 15;
    char *newargv[newargc];
    newargv[0] = "mandel";
    newargv[1] = "-W";
    newargv[2] = image_width;
    newargv[3] = "-H";
    newargv[4] = image_height;
    newargv[5] = "-m";
    newargv[6] = max;
    newargv[7] = "-x";
    newargv[8] = xcenter;
    newargv[9] = "-y";
    newargv[10] = ycenter;
    
    int numImages = 0;
    //keep all the pids so we can wait for all of them. there will be 50 for 50 images
    pid_t pid[50];
    
    while (numImages < 50) {
        error = sem_wait(sem); //the parent waits for a an available slot before forking again
        if (error == -1) {
            perror("sem_wait");
            exit(1);
        }
       
       
        pid[numImages] = fork();
        if (pid[numImages] == 0) { //in the child process only

            newargv[11] = "-s";
            char buffer1[50];
            //change only the scale for each iteration
            sprintf(buffer1, "%f", xscale/((numImages+1)*(numImages+1)/10.0));
            newargv[12] = buffer1;
            

            newargv[13] = "-o";
            char buffer2[50];
            //also change the name each time to be numbered properly
            sprintf(buffer2, "images/mandel%d.jpg", numImages+1);
            newargv[14] = buffer2;
            
            //run the given program
            printf("generating image #%d\n", numImages+1);
            mandel(newargc, newargv);

            sem_post(sem); //make a slot available to the parent
            return 0;
        }
        numImages++;
    }

    for (int i = 0; i < 50; i++) {
        waitpid(pid[i], NULL, 0); //wait for all the children
    }
    printf("images done\n");

    sem_destroy(sem); //clean up
    
    //below is code for automatically making the .mpg and playing it

    // pid_t pid2 = fork();
    // if (pid2==0) {
    //     execlp("ffmpeg", "ffmpeg", "-i", "images/mandel%d.jpg", "mandel.mpg", NULL);
    // }
    // wait(NULL);
    // printf("mandel.mpg created\n");
    // pid2 = fork();
    // if (pid2==0) {
    //     execlp("vlc", "vlc", "-L", "mandel.mpg", NULL);
    // }
    // wait(NULL);
    // printf("displayed video\n");
    return 0;
    
}