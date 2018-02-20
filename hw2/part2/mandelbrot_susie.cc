/**
 *  \file mandelbrot_susie.cc
 *
 *  \brief Implement your parallel mandelbrot set in this file.
 */

#include <iostream>
#include <cstdlib>
#include <mpi.h>

#include "render.hh"

using namespace std;

#define WIDTH 1000
#define HEIGHT 1000

int
mandelbrot(double x, double y) {
  int maxit = 511;
  double cx = x;
  double cy = y;
  double newx, newy;

  int it = 0;
  for (it = 0; it < maxit && (x*x + y*y) < 4; ++it) {
    newx = x*x - y*y + cx;
    newy = 2*x*y + cy;
    x = newx;
    y = newy;
  }
  return it;
}

int
main (int argc, char* argv[])
{
  /* Lucky you, you get to write MPI code */
  double minX = -2.1;
  double maxX = 0.7;
  double minY = -1.25;
  double maxY = 1.25;

  int height, width;
  if (argc == 3) {
    height = atoi (argv[1]);
    width = atoi (argv[2]);
    assert (height > 0 && width > 0);
  } else {
    fprintf (stderr, "usage: %s <height> <width>\n", argv[0]);
    fprintf (stderr, "where <height> and <width> are the dimensions of the image.\n");
    return -1;
  }

  double it = (maxY - minY)/height;
  double jt = (maxX - minX)/width;
  double x, y;

  int rank, np;
  double start, end;

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &np);

  if (rank == 0 ){
    start = MPI_Wtime();
  }

  int nrow = height/np + 1;
  int block = nrow * width;
  int send[block];
  int * recv;
  y = minY + rank * it;

  int row = 0;

  for (int i = rank; i < height ; i += np){
    x = minX;
    for (int j = 0; j < width; ++j){
      send[row * width + j] = mandelbrot(x,y);
      x += jt;
    }
    y += (np * it);
    row += 1;
  }
  MPI_Barrier (MPI_COMM_WORLD);
   if(rank == 0){
   recv = (int *)malloc(sizeof(int) * block * np);
   }
  MPI_Gather (send, block, MPI_INT, recv, block, MPI_INT, 0 , MPI_COMM_WORLD);

  if (rank ==0){
    int m = 0;
    int n = 0;
    gil::rgb8_image_t img(height, width);
		auto img_view = gil::view(img);
    for (int i = 0; i < height; ++i){
      for (int j = 0;j < width;++j){
        m = (i % np) * block;
        img_view(j,i) = render(recv[m + (n*width) + j]/512.0);
      }
      n = i /np;
    }
    end = MPI_Wtime();
    cout<< "time: "<<end-start<<endl;
    gil::png_write_view("mandelbrot_susie.png", const_view(img));

  }


  MPI_Finalize();



}

/* eof */
