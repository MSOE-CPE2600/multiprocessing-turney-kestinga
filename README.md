# System Programming Lab 11 Multiprocessing
<p>
My impementation leaves the given files mostly untouched, just converts it to a function that can be called with the regular arguments. There is a semaphore that starts at a value of the number of child processes that this instance of the program uses, decided by the command line argument "-n". The parent process uses sem_wait before forking, each child process generates one image and then does sem_post and exits. This makes it so the parent only forks when less than the total child processes decided by the user are running. The result is the directory named "images" gets filled with 50 image files.
</p>
<img src="Graph.png" alt="Graph showing how runtime improves with more processors">
<p>
These results show that increasing the processes used will improve the runtime of your program, with diminishing returns. This is mostly because the computer only has a limited number of processors (16 for the laptop used), and you can see how it clearly levels out after 16.
</p>
