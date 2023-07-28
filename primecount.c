#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>

//We are 3 students at hadar group, and we learned and made that task together,
//so it might be simillarity between our codes. We have talked about that with hadar 
//and he said that it is ok by him. Our code is not the same but with some similarity,
//everyone of us wrote his own code on his own computer but when we have been together.  

typedef struct {
  int lval;
  int uval;
  int* count;
  int* num;
  char* flagarr;
} ThreadParameters;

void parseargs(char *argv[], int argc, int *lval, int *uval, int *nval, int *tval);
int isprime(int n);
void* setFlagarr(void* parameters);

pthread_mutex_t lock;

int main(int argc, char **argv)
{
  int lval = 1;
  int uval = 100;
  int nval = 10;
  int tval = 4;
  char *flagarr = NULL;
  int num;
  int count = 0;
  pthread_t* threads;

  // Parse arguments
  parseargs(argv, argc, &lval, &uval, &nval, &tval);
  if (uval < lval)
  {
    fprintf(stderr, "Upper bound should not be smaller than lower bound\n");
    exit(1);
  }
  if (lval < 2)
  {
    lval = 2;
    uval = (uval > 1) ? uval : 1;
  }
  
  num = lval;

  threads = (pthread_t*)malloc(sizeof(pthread_t) * tval);
  flagarr = (char*)malloc(sizeof(char) * (uval - lval + 1));
  if (threads == NULL || flagarr == NULL)
    exit(1);
  
  ThreadParameters parm;
  parm.lval = lval;
  parm.uval = uval;
  parm.num = &num;
  parm.count = &count;
  parm.flagarr = flagarr;

  // Initialize mutex lock
  if (pthread_mutex_init(&lock, NULL) != 0)
  {
    fprintf(stderr, "Mutex initialization failed.\n");
    exit(1);
  }

  // Create threads
  for (int i = 0; i < tval; i++)
  {
    if (pthread_create(&threads[i], NULL, setFlagarr, (void*)&parm) != 0)
    {
      fprintf(stderr, "Thread creation failed.\n");
      exit(1);
    }
  }

  // Wait for threads to complete
  for (int i = 0; i < tval; i++)
  {
    if (pthread_join(threads[i], NULL) != 0)
    {
      fprintf(stderr, "Thread join failed.\n");
      exit(1);
    }
  }

  // Destroy mutex lock
  pthread_mutex_destroy(&lock);

  // Print results
  printf("Found %d primes%c\n", count, count ? ':' : '.');
  for (num = lval; num <= uval && nval != 0; num++)
  {
    if (flagarr[num - lval])
    {
      count--;
      nval--;
      printf("%d%c", num, (count && nval) ? ',' : '\n');
    }
  }

  // Clean up
  free(threads);
  free(flagarr);

  return 0;
}

void* setFlagarr(void* parameters)
{
 	ThreadParameters* parm = (ThreadParameters*)parameters;

	int temp = 2; 
	
  while (*(parm->num) <= parm->uval)
  {
	pthread_mutex_lock(&lock); 
  	temp = *(parm->num);
  	(*parm->num)++;
  	pthread_mutex_unlock(&lock);
  	
    if (isprime(temp) && temp <= parm->uval)
    {
      parm->flagarr[temp - parm->lval] = 1;
      pthread_mutex_lock(&lock); // Lock the critical section
      (*parm->count)++;
      pthread_mutex_unlock(&lock); // Unlock the critical section
    }
    else
    {
      parm->flagarr[temp - parm->lval] = 0;
    }
  }

  pthread_exit(NULL);
}

// NOTE : use 'man 3 getopt' to learn about getopt(), opterr, optarg and optopt 
void parseargs(char *argv[], int argc, int *lval, int *uval
		,int *nval, int *tval)
{
  int ch;

  opterr = 0;
  while ((ch = getopt (argc, argv, "l:u:n:t:")) != -1)
    switch (ch)
    {
      case 'l':  // Lower bound flag
        *lval = atoi(optarg);
        break;
      case 'u':  // Upper bound flag 
        *uval = atoi(optarg);
        break;
      case 'n':
      	*nval = atoi(optarg);
      	break;
      case 't':
      	*tval = atoi(optarg);
      	break;
      case '?':
        if ((optopt == 'l') || (optopt == 'u') || (optopt == 'n')
        || (optopt == 't'))
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        exit(1);
      default:
        exit(1);
    }    
}

int isprime(int n)
{
  static int *primes = NULL; 	// NOTE: static !
  static int size = 0;		// NOTE: static !
  static int maxprime;		// NOTE: static !
  int root, i , doesPrime;
	
	 // Check 'special' cases
  if (n <= 0)
    return -1;
  if (n == 1)
    return 0;
    
  // Init primes array (executed on first call)
  pthread_mutex_lock(&lock); 
  if (primes == NULL)
  {
    primes = (int *)malloc(2*sizeof(int));
    if (primes == NULL)
      exit(1);
    size = 2;
    primes[0] = 2;
    primes[1] = 3;
    maxprime = 3;
  }
	pthread_mutex_unlock(&lock); 
	
  root = (int)(sqrt(n));

		while (root > maxprime) {
  		for (i = maxprime + 2; ; i += 2) {
 		 	  doesPrime = 1;

  	  // Check if i is prime
  	  for (int j = 2; j * j <= i; j++) {
  	    if (i % j == 0) {
  	      doesPrime = 0;
  	      break;
  	    }
  	  }

  	  if (doesPrime) {
  	    pthread_mutex_lock(&lock); 
  	    primes = (int *)realloc(primes, (size+1) * sizeof(int));
  	    size += 1;
  	    if (primes == NULL)
  	      exit(1);
  	    primes[size - 1] = i;
  	    maxprime = i;
  	    pthread_mutex_unlock(&lock); 
  	    break;
  	  }
  	}
	}

  // Check prime
  for (i = 0 ; ((i < size) && (root >= primes[i])) ; i++)
    if ((n % primes[i]) == 0)
      return 0;
  return 1;
}
