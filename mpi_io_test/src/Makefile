#
# Makefile for Linux machines 
#

MPI_PATH	= 	${MPI_ROOT}

CC		=	gcc
CFLAGS          =       -g -I. -I${MPI_PATH}/include -DROMIO -I../../utilities/src
CLIBS           =       -L${MPI_PATH}/lib -lmpi


AR		=	ar
ARFLAGS		=	-r -c
MPIIO           =       MPIIO_hints MPIIO_aggregation MPIIO_ops parse verify
MPIIOOBJS       =       MPIIO_hints.o MPIIO_aggregation.o MPIIO_ops.o parse.o verify.o
UTILS           =       print
UTILOBJS        =       print.o

all:    mpi_io_test

mpi_io_test:    $(OBJS) $(UTILS) $(MPIIO)
	$(CC) $(CFLAGS) \
                -DCRBENCH_COMM=MPI_COMM_WORLD\
                -o $@.x mpi_io_test.c $(MPIIOOBJS) $(UTILOBJS) $(CLIBS)

MPIIO_hints:    MPIIO_hints.c
	$(CC) $(CFLAGS) \
                -c MPIIO_hints.c

MPIIO_aggregation:      MPIIO_aggregation.c
	$(CC) $(CFLAGS) \
                -c MPIIO_aggregation.c

MPIIO_ops:      MPIIO_ops.c
	$(CC) $(CFLAGS) \
                -c MPIIO_ops.c

parse:          parse.c
	$(CC) $(CFLAGS) \
                -c parse.c

print:          ../../utilities/src/print.c
	$(CC) $(CFLAGS) \
                -c ../../utilities/src/print.c

verify:         verify.c
	$(CC) $(CFLAGS) \
                -c verify.c


clean:
	- /bin/rm -f *~
	- /bin/rm -f *.o
	- /bin/rm -f *.x
