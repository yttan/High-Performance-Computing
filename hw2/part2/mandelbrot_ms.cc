/**
 *  \file mandelbrot_ms.cc
 *
 *  \brief Implement your parallel mandelbrot set in this file.
 */

#include <iostream>
#include <cstdlib>
#include <mpi.h>
#include "render.hh"
using namespace std;

#define WIDTH 1000;
#define HEIGHT 1000;

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
  double start, end;
	int rank, np;
	MPI_Status status;
	MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &np);
	if(rank == 0){
		start = MPI_Wtime();
		int result[width*height];
		int recv[width + 1];
		int row = 0;
		int p = 1;
		int finish = -1;
		while (row < height) {
			if (p < np) {
				MPI_Send(&row, 1, MPI_INT, p, 0, MPI_COMM_WORLD);
				p++;
			}
			else {
				MPI_Recv(recv, width+1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
				MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
				memcpy(result + recv[width]*width, recv, width*sizeof(int));
			}
			row++;
		}
		for (int i = 1; i < np; i++) {
			MPI_Recv(recv, width+1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			MPI_Send(&finish, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			memcpy(result + recv[width]*width, recv, width*sizeof(int));
		}
		gil::rgb8_image_t img(height, width);
		auto img_view = gil::view(img);
		for (int k = 0; k < height; ++k) {
			for (int p = 0; p < width; ++p) {
				img_view(p, k) = render(recv[ (k*width) + p] / 512.0);
			}
		}
		end = MPI_Wtime ();
		cout<<"time:"<<end- start <<endl;
		gil::png_write_view("mandelbrot_ms.png", const_view(img));
	}
	else {
		int send[width + 1];
		int row;
		while(true) {
			MPI_Recv(&row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
			if (row == -1) break;
			y = minY + row*it;
			x = minX;
			for (int i = 0; i < width; ++i) {
				send[i] = mandelbrot(x,y);
				x += jt;
			}
			send[width] = row;
			MPI_Send(send, width+1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}
	MPI_Finalize();

}

/* eof */
