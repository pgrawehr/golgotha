/*
     /usr/bin/time a.out 0 (for POSIX threading)

     or

     /usr/bin/time a.out 1 (for Solaris threading)
*/
     /* cc thisfile.c -lthread -lpthread */
     #define _REENTRANT    /* basic 3-lines for threads */
     #include <pthread.h>
     #include <thread.h>

     #define NUM_THREADS 5
     #define SLEEP_TIME 10

     void *sleeping(void *);   /* thread routine */
     int i;
     thread_t tid[NUM_THREADS];      /* array of thread IDs */

     int
     main(int argc, char *argv[])
     {
     	     int retval;
             if (argc == 1)  {
                     printf("use 0 as arg1 to use pthread_create()\n");
                     printf("or use 1 as arg1 to use thr_create()\n");
                     return (1);
             };
	     

             switch (*argv[1])  {
             case '0':  /* POSIX */
                     for ( i = 0; i < NUM_THREADS; i++)
		     {
                             retval=pthread_create(&tid[i], NULL, sleeping,
                                 (void *)SLEEP_TIME);
			     if (retval!=0) 
				printf("Thread creation problem, Error %i\n",retval);
		     }
                     for ( i = 0; i < NUM_THREADS; i++)
                             pthread_join(tid[i], NULL);
                     break;

             case '1':  /* Solaris */
                     for ( i = 0; i < NUM_THREADS; i++)
                             thr_create(NULL, 0, sleeping, (void *)SLEEP_TIME, 0,
                                 &tid[i]);
                     while (thr_join(NULL, NULL, NULL) == 0)
                             ;
                     break;
             }  /* switch */




             printf("main() reporting that all %d threads have terminated\n", i);
             return (0);
     }  /* main */

     void *
     sleeping(void *arg)
     {
             int sleep_time = (int)arg;
             printf("thread %d sleeping %d seconds ...\n", thr_self(), sleep_time);
             sleep(sleep_time);
             printf("\nthread %d awakening\n", thr_self());
             return (NULL);
     }
