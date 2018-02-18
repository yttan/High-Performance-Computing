/**
 *  \file parallel-mergesort.cc
 *
 *  \brief Implement your parallel mergesort in this file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <omp.h>
#include "sort.hh"



static int compare (const void* a, const void* b)
{
  keytype ka = *(const keytype *)a;
  keytype kb = *(const keytype *)b;
  if (ka < kb)
    {return -1;}
  else if (ka == kb)
    {return 0;}
  else
    {return 1;}
}
int binarysearch (keytype* key, keytype* A, int p, int r)
{
    int low = p;
    int high;
    if (p>=(r+1)) {high = p;}
    else {high = (r+1);}
    int mid;
    while(low<high)
    {
      mid = (high+low)/2;
      if (compare(key,A+mid) == 0){ high = mid; }
      else if (compare(key, A+mid) == -1) {high = mid;}
      else  { low = mid+1;}
    }
    return high;
}


void pmerge(keytype* T, int p1, int r1, int p2, int r2, int p3, keytype* A)
{
  int n1 = r1 - p1 + 1;
  int n2 = r2 - p2 + 1;

  if (n1<n2)
  {
    int tmp = p1;
    p1 = p2;
    p2 = tmp;
    tmp = r1;
    r1 = r2;
    r2 = tmp;
    tmp = n1;
    n1 = n2;
    n2 = tmp;
  }

  if (n1==0) {return;}
  else
  {
    int q1 = (p1+r1)/2;
    int q2 = binarysearch((T+q1),T,p2,r2);
    int q3 = p3 +(q1-p1)+(q2-p2);
    A[q3] = T[q1];
    #pragma omp task
    pmerge(T,p1,q1-1,p2,q2-1,p3,A);
    pmerge(T,q1+1,r1,q2,r2,q3+1,A);
    #pragma omp taskwait
  }
}

void pmergesort (keytype* A, int p, int r, keytype* B)
{
  int n = r-p+1;
  if (n>1)
  {
    int q = (p+r)/2;
    #pragma omp task
    pmergesort(A,p,q,B);
    pmergesort(A,q+1,r,B);
    #pragma omp taskwait
    pmerge(A,p,q,q+1,r,p,B);
    int i;
    //#pragma omp parallel for shared (A,r,p,B) private (i)
    for (i = p; i <= r; i++)
    {
      A[i] = B[i];
    }
  }
}

void parallelSort (int N, keytype* A)
{

    keytype* B = newKeys (N);
    #pragma omp parallel
    #pragma omp single nowait
	  pmergesort(A, 0, N-1, B);
 /* Lucky you, you get to start from scratch */
}

/* eof */
