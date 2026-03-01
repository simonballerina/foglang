#include <stdio.h>
#include <time.h>
     
    
int main() {
clock_t start, end;
double cpu_time_used;
start = clock();
for (int a = 0; a < 10; a++){
  int i;

  int n = 100;

  // initialize first and second terms
  int t1 = 0, t2 = 1;

  // initialize the next term (3rd term)
  int nextTerm = t1 + t2;

  // get no. of terms from user
  

  // print the first two terms t1 and t2
  printf("Fibonacci Series: %d, %d, ", t1, t2);

  // print 3rd to nth terms
  for (i = 3; i <= n; ++i) {
    printf("%d, ", nextTerm);
    t1 = t2;
    t2 = nextTerm;
    nextTerm = t1 + t2;
  }

  
  
  
  
  
  }
  
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("\n\n\nCPU time elapsed (C): %f\n", cpu_time_used);
  return 0;
}