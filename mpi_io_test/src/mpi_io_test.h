/*************************************************************************
 * Copyright (c) 2005, The Regents of the University of California
 * All rights reserved.
 *
 * Copyright 2005. The Regents of the University of California. 
 * This software was produced under U.S. Government contract W-7405-ENG-36 for 
 * Los Alamos National Laboratory (LANL), which is operated by the University 
 * of California for the U.S. Department of Energy. The U.S. Government has 
 * rights to use, reproduce, and distribute this software.  
 * NEITHER THE GOVERNMENT NOR THE UNIVERSITY MAKES ANY WARRANTY, EXPRESS OR 
 * IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.  If 
 * software is modified to produce derivative works, such modified software 
 * should be clearly marked, so as not to confuse it with the version available
 * from LANL.
 * Additionally, redistribution and use in source and binary forms, with or 
 * without modification, are permitted provided that the following conditions 
 * are met:
 *      Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer. 
 *      Redistributions in binary form must reproduce the above copyright 
 *      notice, this list of conditions and the following disclaimer in the 
 *      documentation and/or other materials provided with the distribution. 
 *      Neither the name of the University of California, LANL, the U.S. 
 *      Government, nor the names of its contributors may be used to endorse 
 *      or promote products derived from this software without specific prior 
 *      written permission. 
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
 * DAMAGE.
*************************************************************************/

/****************************************************************
* AUTHOR:    James Nunez 
* DATE:      February 12, 2001
* LAST MODIFIED: September 16, 2005 [jnunez]
* VERSION:   1.00.001
*
******************************************************************/

#ifndef   __CRBENCH_H_INCLUDED
#define   __CRBENCH_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <mpio.h>
#include "mpi.h"

#define TRUE 1
#define FALSE 0
  
  /* MPIIO_hints.c Routines Begin */

  extern int MPIIO_set_hint(int my_rank, MPI_Info *info, char *key, char *value);
  extern int MPIIO_get_hints(int my_rank, MPI_File wfh, int *nkeys, 
			     char ***keys, char ***values, FILE *ofptr, 
			     FILE *efptr);
  
  /* MPIIO_hints.c Routines End */

  int MPIIO_master_write(int my_rank, int my_rank_io, int io_processor, 
			 int write_file_flag, 
			 MPI_Comm comm_file, char *tfname, int amode_write, 
			 MPI_Info info, int *nkeys, char ***key, 
			 char ***value, int blksize, int send_to, 
			 size_t num_objs, int num_receive, int num_of_cbuf, 
			 int num_file_writes, size_t obj_size, int touch, 
			 int pagesize, MPI_Offset prealloc_size,
			 MPI_Offset file_offset, MPI_Offset poffset_val, 
			 MPI_Offset other_offset, 
			 int num_of_cjob, int f_procs, size_t cbuf_size, 
			 double *file_open_wait_time, 
			 double *prealloc_wait_time,
			 double *file_op_wait_time,
			 double *proc_recv_wait_time, 
			 double *proc_send_wait_time,
                         double *total_op_time, 
			 double *total_time, 
                         double *barrier_wait_time, double *file_sync_wait_time, 
			 double *file_close_wait_time,
			 int aggregate_flag, int io_proc_send_flag, 
			 int info_allocated_flag, int init_flag, 
			 int collective_flag, int set_view_all, 
			 int data_pattern, int *cbuf_use,
                         int barrier_flag, int sync_flag,
			 FILE *ofptr, FILE *efptr);
  
  int MPIIO_master_read(int my_rank, int my_rank_io, int io_processor, 
			int write_file_flag, 
			MPI_Comm comm_file, char *tfname, int amode_read, 
			MPI_Info info, int nkeys, char **key, 
			char **value, int blksize, int send_to, 
			size_t num_objs, int num_receive, int num_of_cbuf, 
			int num_file_writes, size_t obj_size, int touch, 
			int pagesize, int check_data_ndx,
			MPI_Offset file_offset, MPI_Offset poffset_val, 
			MPI_Offset other_offset, 
			int num_of_cjob, int f_procs, 
			size_t cbuf_size, 
			double *file_open_wait_time, double *file_op_wait_time,
			double *proc_recv_wait_time, 
			double *proc_send_wait_time, double *total_op_time, 
			double *total_time, 
			double *file_close_wait_time,
			int aggregate_flag, int io_proc_send_flag, 
			int info_allocated_flag, int init_flag, 
			int collective_flag, int set_view_all, 
			int strided_flag, int *cbuf_use, int sync_flag, 
			FILE *ofptr, FILE *efptr);
  
  void show_usage(int fullhelp);
  
  int parse_command_line(int my_rank, int argc, char *argv[], 
			 int *test_type, size_t *num_objs, 
			 size_t *obj_size, size_t *cbuf_size, 
			 MPI_Offset *prealloc_size, int *prealloc_flag,
			 int *ahead, int *touch, int *data_pattern,
			 int *list_host_flag, int *num_hosts, 
			 char *my_host, int *write_file_flag, 
			 int *barrier_flag, int *sync_flag, int *sleep_sec, 
			 int *delete_flag, char **ofname, 
			 char **efname, char **tfname,
			 char ***host_io_list, int **io_procs_list,
			 int *init_flag, int *num_procs_io,
			 int *aggregate_flag, MPI_Info *info,
			 int *info_allocated_flag, int *direct_io_flag,
			 int *targets_flag, char **tgtsfname, 
			 int *write_only_flag, int *read_only_flag, 
			 int *check_data_flag, 
			 int *collective_flag, int *io_proc_send_flag,
			 int *verbose_flag);
  
  int compute_aggregators(int my_rank, int num_procs_world,int num_procs_io,
			  int *io_procs_list, int num_hosts, 
			  char **host_io_list, int test_type, size_t num_objs, 
			  size_t obj_size, size_t cbuf_size, int init_flag, 
			  int io_proc_send_flag, char *my_host, 
			  int *myhost_num, int *f_procs, int *num_of_cjob,
			  int *num_file_writes, int *io_processor, 
			  int *my_rank_io, int *send_to, int *num_receive,
			  int **recv_procs, MPI_Offset *poffset_val,
			  MPI_Comm *comm_io, MPI_Comm *noncomm_io,
			  FILE *ofptr, FILE *efptr);
  /*
  int print_input_environment(int my_rank, int test_type, int num_procs_world, 
			      int num_procs_io, int aggregate_flag, 
			      int *io_procs_list, int num_hosts, 
			      char **host_io_list, int ahead, size_t num_objs, 
			      size_t obj_size, size_t cbuf_size, 
			      MPI_Offset prealloc_size, int check_data_flag,
			      int collective_flag, int data_pattern, 
			      int delete_flag, int write_file_flag, 
			      int io_proc_send_flag, 
			      int barrier_flag, int sync_flag, int targets_flag, 
			      char *tgtsfname, char *tfname, int touch, 
			      int list_host_flag, int init_flag, 
			      int myhost_num, char *my_host, char *ofname, 
			      char *efname, FILE *ofptr, FILE *efptr);

  int collect_and_print_time(int my_rank, int num_procs, int num_io_procs, 
			     size_t obj_size, size_t num_objs, 
			     double total_time_start, double total_time_end,
			     double file_open_wait_time,
			     double prealloc_wait_time, 
			     double file_close_wait_time, 
			     double barrier_wait_time, 
			     double file_sync_wait_time, 
			     double proc_send_wait_time, 
			     double proc_recv_wait_time, 
			     double file_write_wait_time, 
			     int barrier_flag, int sync_flag, int io_proc_send_flag, 
			     int aggregation, int verbose_flag, char *op, 
			     FILE *ofptr, FILE *efptr);
extern
int print_parm(char *msg, int my_rank, int num_procs, MPI_Comm comm, FILE *output, FILE *error);     

  */    
  int find_min_jobs(int my_rank, 
		    int num_procs_io, 
		    int num_of_cjob,
		    int *min_all_proc_cjobs,
		    MPI_Comm comm_io, 
		    FILE *ofptr, FILE *efptr);

extern
void crb_print( const char *msg, MPI_Comm comm, FILE *stream );
  /* 
   * Print a message, prefixed by MPI rank, into a file stream.
   *
   * msg     Message to print.  A null terminated string.
   * comm    MPI communicator used to determine MPI rank.
   * stream  File stream to print into.
   */


  size_t verify_buffer( int my_rank, double *obj, size_t chk_data_end, 
			int touch, int pagesize, MPI_Offset touch_val,
			FILE *ofptr, FILE *efptr);

extern
size_t crb_verify_2( const double obj[], size_t nobj, size_t size,
                   unsigned int key, size_t offset );
  /*
   * Verify correctness of data objects.
   *
   * Returns the number of errors found.
   *
   * obj   Array of data objects to verify.
   * nobj  Number of data objects in array.
   * size  Size of each data object (in doubles).
   * key   Verification key.
   * offset  offset of this node data in the file.
   */


  int fill_buffer( int my_rank, double *obj, size_t obj_size, 
		   int touch, int pagesize, MPI_Offset poffset_val,
		   FILE *ofptr, FILE *efptr);
  
  extern
  void crb_fill_2( double obj[], size_t nobj, size_t size, unsigned int key,
		   size_t offset);
  /*
   * Fill (initialize) data objects according to a verification key.
   *
   * obj   Array of data objects to fill (initialize).
   * nobj  Number of data objects in array.
   * size  Size of each data object (in doubles).
   * key   Verification key.
   * offset  offset of this node data in the file.
   */



double crb_barrier( MPI_Comm comm );
  /*
   * A "shorthand" for the following functionality.
   *
   *   MPI_Barrier(comm);
   *   return MPI_Wtime();
   */


  extern char *expand_path(char *str, MPI_Comm comm);
  extern int get_file_name(char *input_fname, int my_rank, int max_rank,
		    MPI_Comm comm, char **target_fname, FILE *ofptr, FILE *efptr);

  extern int string_to_string_array(char *input_string, char *delimiter,
			     int *num_args, char ***output_string);
  extern int string_to_int_array(char *input_string, char *delimiter,
                           int *num_args, int **output_ints, int *sum);

#ifdef __cplusplus
}
#endif

#endif /* __CRBENCH_H_INCLUDED */
