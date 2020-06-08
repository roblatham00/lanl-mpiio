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

/******************************************************************
* PROGRAM:   parse
*
* PURPOSE:   Routines that deal with parsing the command line
*
* AUTHOR:    James Nunez 
* DATE:      January 3, 2005
* LAST MODIFIED: February 14, 2006 [swh]
* VERSION:   1.00.007
*
********************************************************************/

#include "mpi_io_test.h"
#include "boolean.h"

#include <stdio.h>
#include <string.h>

/*******************************************
 *   ROUTINE: parse_size
 *   PURPOSE: Convert character inputs to number bytes. Allow 
 *            for (Linux) dd-like expressions; w 2, b 512, KB 1000, K 1024, 
 *            MB 1,000,000,  M 1,048,576, GB 1,000,000,000, and G 1,073,741,824
 *            A positive number followed by one of the above letter will 
 *            result in the two multiplied, i.e. 2b => 1024.
 *
 *   DATE: February 19, 2004
 *   LAST MODIFIED: September 7, 2005 [jnunez]
 *******************************************/
static int parse_size(int my_rank, char *chbytes, long long int *out_value){

  char last, next_last;
  unsigned long long int insize = 0;
  
  last = chbytes[strlen(chbytes) - 1];
  next_last = chbytes[strlen(chbytes) - 2];
  
  if( !isdigit(last))
    switch(last){
    case 'w':
      if(insize == 0) insize = 2;
    case 'b':
      if(insize == 0) insize = 512;
    case 'K':
      if(insize == 0) insize = 1024;
    case 'M':
      if(insize == 0) insize = 1048576;
    case 'G':
      if( !isdigit(next_last)){
	fprintf(stderr,"[RANK %d] ERROR: Unknown multiplicative suffix (%c%c) for input string %s.\n", 
		my_rank, next_last, last, chbytes);
	return 1;
      }
      chbytes[strlen(chbytes) - 1] = '\0';
      if(insize == 0) insize = 1073741824;
      break;
    case 'B':
      if( isdigit(next_last)){
	fprintf(stderr,"[RANK %d] ERROR: Unknown multiplicative suffix (%c) for input string %s.\n", 
		my_rank, last, chbytes);
	return 1;
      }
      
      chbytes[strlen(chbytes) - 1] = '\0';
      last = chbytes[strlen(chbytes) - 1];
      next_last = chbytes[strlen(chbytes) - 2];
      
      if( !isdigit(next_last)){
	fprintf(stderr,"[RANK %d] ERROR: Unknown multiplicative suffix (%cB) for input string %s.\n", 
		my_rank, last, chbytes);
	return 1;
      }
      
      switch(last){
      case 'K':
	if(insize == 0) insize = 1000;
      case 'M':
	if(insize == 0) insize = 1000000;
      case 'G':
	if(insize == 0) insize = 10000000000ULL;
	chbytes[strlen(chbytes) - 1] = '\0';
	break;
      default:
	fprintf(stderr,"[RANK %d] ERROR: Unknown multiplicative suffix (%cB) for input string %s.\n", 
		my_rank, last, chbytes);
	return 1;
	break;
      }
      
      break;
    default:
      fprintf(stderr,"[RANK %d] ERROR: Unknown multiplicative suffix (%c) for input string %s.\n", 
	      my_rank, last, chbytes);
      return 1;
    }
  else{
    insize = 1;
  }

  /*  XXXX Must find way to handle decimal points instead of atol
  fprintf(stderr,"chbytes = %s atol = %ld\n", chbytes, atol(chbytes));
  */
  *out_value = insize * (unsigned long long)atol(chbytes);

  return 0;
}

/*******************************************
 *   ROUTINE: parse_command_line
 *   PURPOSE: Parse the command line
 *   DATE:   January 3, 2005 
 *   LAST MODIFIED: February 14, 2006 [swh]
 *******************************************/
int parse_command_line(int my_rank, int argc, char *argv[], 
		       int *test_type, size_t *num_objs, 
		       size_t *obj_size, size_t *cbuf_size, 
		       MPI_Offset *prealloc_size, int *prealloc_flag,
		       int *ahead, int *touch, int *strided,
		       int *list_host_flag, int *num_hosts, 
		       char *my_host, int *write_file_flag, 
                       int *barrier_flag, int *sync_flag, int *sleep_seconds, 
		       int *delete_flag, char **ofname, 
		       char **efname, char **tfname,
		       char ***host_io_list, int **io_procs_list,
		       int *init_flag, int *num_procs_io,
		       int *aggregate_flag, MPI_Info *info,
		       int *info_allocated_flag, int *direct_io_flag,
		       int *targets_flag, char **tgtsfname, 
		       int *write_only_flag, int *read_only_flag,
		       int *check_data_ndx, int *collective_flag, 
		       int *io_proc_send_flag, int *verbose_flag )
{
  int i = 1;
  int index = 0;

  long long int temp_num;

/******************************************************************
* First allocate the MPI_Info structure. If no hints are set, the structure 
* will be freed at the end of this routine.
******************************************************************/
  if(MPI_Info_create(info) != MPI_SUCCESS){
    if(my_rank == 0)
      fprintf(stderr, "[RANK %d] ERROR: Unable to create the MPI_info structure.\n", my_rank);
    MPI_Finalize();
    return -1;
  }

/******************************************************************
* Now read and load all command line arguments. 
******************************************************************/
  i = 1;
  while(i < argc){
    
    if( !strcmp(argv[i], "-num_io") ){
      if(!string_to_int_array(argv[i+1], " ", &index, io_procs_list, 
			      num_procs_io)){
	if(my_rank == 0)
	  fprintf(stderr, "ERROR: Problem parsing string of number of I/O processors per host.\n");
	MPI_Finalize();
	return -1;
      }
      *aggregate_flag = TRUE;
      i += 2;
    }
    else if(!strcmp(argv[i], "-type")){
      *test_type = atoi(argv[i+1]);
      i += 2;
    }
    else if(!strcmp(argv[i], "-ahead")){
      *ahead = atoi(argv[i+1]);
      i += 2;
    }
    else if(!strcmp(argv[i], "-nobj")){
      *num_objs = (size_t)strtoul(argv[i+1], NULL, 10);
      i += 2;
    }
    else if(!strcmp(argv[i], "-strided")){
      *strided = atoi(argv[i+1]);
      i +=2;
    }
    else if(!strcmp(argv[i], "-size")){
      parse_size(my_rank, argv[i+1], &temp_num);
      *obj_size = (size_t)temp_num;
      i +=2;
    }
    else if(!strcmp(argv[i], "-csize")){
      parse_size(my_rank, argv[i+1], &temp_num);
      *cbuf_size = (size_t)temp_num;
      i +=2;
    }
    else if(!strcmp(argv[i], "-preallocate")){
      parse_size(my_rank, argv[i+1], &temp_num);
      *prealloc_size = (MPI_Offset)temp_num;
      *prealloc_flag = TRUE;
      i +=2;
    }
    else if(!strcmp(argv[i], "-target")){
      if ((*tfname = expand_path(argv[i+1], MPI_COMM_WORLD)) == (char *)0) {
	fprintf(stderr, "[RANK %d] ERROR: Problem expanding file name %s.\n", 
		my_rank, argv[i+1]);
	MPI_Abort(MPI_COMM_WORLD, -1);
	return -1;
      }
      i +=2;
    }
    else if (!strcmp(argv[i], "-targets")) {
      *targets_flag = TRUE;
      *tgtsfname = argv[i+1];
      i +=2;
    }
    else if (!strcmp(argv[i], "-op")) {
      if (!strcmp(argv[i+1], "write")){
	*write_only_flag = TRUE;
      }
      else if (!strcmp(argv[i+1], "read")){
	*read_only_flag = TRUE;
      }
      else{
	fprintf(stderr, "[RANK %d] ERROR: Operation %s is not supported. Only recognized operation is read or write.\n", my_rank, argv[i+1]);
	MPI_Finalize();
	return -1;
      }
      i +=2;
    }
    else if(!strcmp(argv[i], "-touch")){
      *touch = atoi(argv[i+1]);
      i +=2;
    }
    else if(!strcmp(argv[i], "-sleep")){
      *sleep_seconds = atoi(argv[i+1]);
      i +=2;
    }
    else if(!strcmp(argv[i], "-host")){
      if (!string_to_string_array(argv[i+1], " ", num_hosts, 
				  host_io_list)) {
	if(my_rank == 0)
	  fprintf(stderr, "ERROR: Problem parsing string of hostnames.\n");
	MPI_Finalize();
	return -1;
      }
      
      i +=2;
    }
    else if(!strcmp(argv[i], "-lhosts")){
      *list_host_flag = TRUE;
      i++;
    }
    else if(!strcmp(argv[i], "-dio")){
      *info_allocated_flag = TRUE;
      MPIIO_set_hint(my_rank, info, "direct_read", "true");
      MPIIO_set_hint(my_rank, info, "direct_write", "true");
      i++;
      *direct_io_flag = TRUE;
    }
    else if(!strcmp(argv[i], "-hints")){
      *info_allocated_flag = TRUE;
      i++;
      
      while(i < argc && strncmp(argv[i], "-", 1)){
	MPI_Info_set(*info, argv[i], argv[i+1]);
	i += 2;
      }
    }
    else if(!strcmp(argv[i], "-nofile")){
      *write_file_flag = FALSE;
      i++;
    }
    else if(!strcmp(argv[i], "-norsend")){
      *io_proc_send_flag = FALSE;
      i++;
    }
    else if(!strcmp(argv[i], "-verbose")){
      *verbose_flag = TRUE;
      i++;
    }
    else if(!strcmp(argv[i], "-output")){
      *ofname = (char *)malloc( (strlen(argv[i+1])+1) * sizeof(char));
      strcpy(*ofname, argv[i+1]);
      i += 2;
    }
    else if(!strcmp(argv[i], "-errout")){
      *efname = (char *)malloc( (strlen(argv[i+1])+1) * sizeof(char));
      strcpy(*efname, argv[i+1]);
      i += 2;
    }
    else if(!strcmp(argv[i], "-barrier")){
      *barrier_flag = TRUE;
      i++;
    }
    else if(!strcmp(argv[i], "-sync")){
      *sync_flag = TRUE;
      i++;
    }
    else if(!strcmp(argv[i], "-deletefile")){
      *delete_flag = TRUE;
      i++;
    }
    else if(!strcmp(argv[i], "-chkdata")){
      *check_data_ndx = atoi(argv[i+1]);
      i += 2;
    }
    else if(!strcmp(argv[i], "-collective")){
      *collective_flag = TRUE;
      i++;
    }
    else if(!strcmp(argv[i], "-help")){
      if(my_rank == 0) show_usage(TRUE);
      MPI_Finalize();
      return -1;
    }
    else{
/*    ******************************************************************
*     Skip all unrecognized flags
*     *****************************************************************/
      i++;
    }
  }

/********************************************************
* If no hints were set, free the MPI_Info structure.
**********************************************************/
  if(!(*info_allocated_flag)){
    if(MPI_Info_free(info) != MPI_SUCCESS){
    }
  }
  
  return(1);
}
