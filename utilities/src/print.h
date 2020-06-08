/****************************************************************
* AUTHOR:    Brett Kettering
* DATE:      July 26, 2005
* LAST MODIFIED: September 7, 2005 [jnunez]
*
*      LOS ALAMOS NATIONAL LABORATORY
*      An Affirmative Action/Equal Opportunity Employer
*
* Copyright (c) 2005
* the Regents of the University of California.
*
* Unless otherwise indicated, this information has been authored by an
* employee or employees of the University of California, operator of the Los
* Alamos National Laboratory under Contract No. W-7405-ENG-36 with the U. S.
* Department of Energy. The U. S. Government has rights to use, reproduce, and
* distribute this information. The public may copy and use this information
* without charge, provided that this Notice and any statement of authorship
* are reproduced on all copies. Neither the Government nor the University
* makes any warranty, express or implied, or assumes any liability or
* responsibility for the use of this information.
******************************************************************/

#ifndef   __PRINT_H_INCLUDED
#define   __PRINT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>

#include "mpi.h"
#include "mpio.h"

  
  void show_usage(int fullhelp);
  
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
  
  int print_parm(char *msg, int my_rank, int num_procs, MPI_Comm comm, FILE *output, FILE *error);     

  static int get_min_sum_max(int my_rank, double base_num, double *min, int *min_ndx,
                    double *sum, double *max, int *max_ndx, char *op,
                    FILE *ofptr, FILE *efptr);

#ifdef __cplusplus
}
#endif

#endif /* __PRINT_H_INCLUDED */
