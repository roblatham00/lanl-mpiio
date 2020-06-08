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
* PROGRAM:   MPIIO_aggregation
*
* PURPOSE:   Contains routines that perform data aggregation
*
* AUTHOR:    James Nunez 
* DATE:      February 12, 2001
* LAST MODIFIED: September 16, 2005 [jnunez]
* VERSION:   1.00.002
*
******************************************************************/
#include "mpi_io_test.h"

/*******************************************
 *   ROUTINE: 
 *   PURPOSE: 
 *******************************************/
int compute_aggregators(int my_rank, int num_procs_world, int num_procs_io,
			int *io_procs_list, int num_hosts, char **host_io_list,
			int test_type, size_t num_objs, size_t obj_size, 
			size_t cbuf_size, int init_flag, int io_proc_send_flag,
			char *my_host, int *myhost_num, int *f_procs,
			int *num_of_cjob, int *num_file_writes, 
			int *io_processor, int *my_rank_io, int *send_to,
			int *num_receive, int **recv_procs,
			MPI_Offset *poffset_val, MPI_Comm *comm_io, 
			MPI_Comm *noncomm_io, FILE *ofptr, FILE *efptr)
{
  MPI_Group group_world = MPI_GROUP_NULL;/* Group of m processors          */
  MPI_Group group_io = MPI_GROUP_NULL; /* Group of n processors            */
  
  MPI_Request temp_request;
  MPI_Status temp_status;
  
  int *send_counts = NULL; /* Length of characters to receive  */

  int *hostnum_list = NULL;
  int *iohostnum_list = NULL;
  int *io_ranks = NULL;    /* Array of ranks for I/O processors in world */
  int np_comm = 0;         /* Size of I/O communicator               */
  
  int i, j, k, index, dummy_int;

/********************************************************************
* If the user has specified host names to perform I/O, compute who each
* processor should receive and send data.
********************************************************************/
  if(num_hosts > 0){

/*  *******************************************************************
*   Make sure there is no duplication in user provided host list.
*   *******************************************************************/
    for(i = 0; i < num_hosts; i++)
      for(j = i+1; j < num_hosts; j++)
	if(!strcmp(host_io_list[i], host_io_list[j])){
	  if(my_rank == 0)
	    fprintf(efptr, "[RANK %d] ERROR: Host name %s is duplicated in input host list. Duplication of host names is not allowed.\n",
		    my_rank, host_io_list[i]);
	  return -1;
	}
    
/*  *******************************************************************
*   Get my host name and find the index into the host_io_list. Zero 
*   represents host not in host_io_list, i.e. not a host that does I/O.
*   *******************************************************************/
    if(!init_flag)
      if( MPI_Get_processor_name(my_host, myhost_num) != MPI_SUCCESS){
	if(my_rank == 0)
	  fprintf(efptr, "[RANK %d] ERROR: Unable to get processor name from MPI_Get_processor_name.\n", my_rank);
	MPI_Finalize();
	return -1;	      
      }
    
    *myhost_num = 0;
    for(i = 0; i < num_hosts; i++)
      if(!strncmp(my_host, host_io_list[i], strlen(my_host)))
	*myhost_num = i + 1;
    
    /* XXX
    fprintf(stderr, "AGG RANK %d: Host name %s I/O host number %d.\n",
	    my_rank, my_host, *myhost_num);
    fflush(stderr);
    XXX */
/*  *******************************************************************
*   Allocate memory for and send I/O host number to processor zero
*   *******************************************************************/
    if((hostnum_list = (int *)calloc(num_procs_world, sizeof(int))) == NULL){
      if(my_rank == 0)
	fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for array of host numbers.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    
    if( MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS) {
      if(my_rank == 0)
	fprintf(efptr, "[RANK %d] ERROR: MPI_Barrier before gather of host index.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    
    if(MPI_Gather(myhost_num, 1, MPI_INT, hostnum_list, 1,
		  MPI_INT, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
      if(my_rank == 0)
	fprintf(efptr, "[RANK %d] ERROR: Unable to gather number of processors to accept data from.\n", my_rank);
      MPI_Finalize();
      return -1;
    }

/*  *******************************************************************
*   Allocate memory for array for I/O processor ranks
*   *******************************************************************/
    if((io_ranks = (int *) malloc(num_procs_io * num_hosts * sizeof(int))) == 
       NULL){
      if(my_rank == 0)
	fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for I/O processor rank array.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    
/*  *******************************************************************
*   Let processor zero figure out who sends data to who. 
*   *******************************************************************/
    if(my_rank == 0){
      if( (send_counts = (int *) calloc(num_procs_world, sizeof(int))) == 
	  NULL){
	fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for receive data array.\n", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
      if((iohostnum_list = (int *) calloc((num_hosts + 1), 
					  sizeof(int))) == NULL){
	if(my_rank == 0)
	  fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for array of I/O host numbers.\n", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }

/*    *******************************************************************
*     Count the number of processors on each host and make sure the 
*     requested number of I/O processors exist. iohostnum_list[0] holds the 
*     number of processors not on an I/O host.
*     *******************************************************************/
      for(i=0; i < num_procs_world; i++)
	iohostnum_list[hostnum_list[i]]++;
      
      for(i=1; i <= num_hosts; i++)
	if(iohostnum_list[i] < io_procs_list[i-1]){
	  fprintf(efptr, "[RANK %d] ERROR: Requested number of I/O processors (%d) not available on host number %d; only %d available.\n", 
		  my_rank, io_procs_list[i-1], i, iohostnum_list[i]);
	  fflush(efptr);
	  MPI_Abort(MPI_COMM_WORLD, -1);
	}

          /* XXX
      fprintf(stderr, "AGG [RANK %d] Number of procs not on I/O host %d.\n",
	      my_rank, iohostnum_list[0]);
      for(i=1; i <= num_hosts; i++)
	fprintf(stderr, "AGG [RANK %d] Number of procs on I/O host %d %d.\n",
		my_rank, i, iohostnum_list[i]);
      fflush(stderr);
    XXX */

/*    *******************************************************************
*     First assign the destination processor for processors on hosts that 
*     do I/O. They will send to a processor on the same host.
*     *******************************************************************/
      index = 0;
      for(i=0; i < num_procs_world; i++){
	if(hostnum_list[i] > 0){
	  
/*        ******************************************************************
*         The first user defined number of processors on an I/O host will 
*         perform I/O. 
*         ******************************************************************/
	  if(io_procs_list[hostnum_list[i] - 1] > 0){
	    io_procs_list[hostnum_list[i] - 1]--;
	    io_ranks[index++]  = i;
	    *io_processor = TRUE;
	    send_counts[i]++;
	    if( MPI_Isend(io_processor, 1, MPI_INT, i, i, MPI_COMM_WORLD, 
			  &temp_request) != MPI_SUCCESS) {
	      fprintf(efptr, "[RANK %d] ERROR: Unable to send I/O processor flag to processor %d.\n", my_rank, i);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
	    
	  }
	  else{
	    *io_processor = FALSE;
	    if( MPI_Isend(io_processor, 1, MPI_INT, i, i, MPI_COMM_WORLD, 
			  &temp_request) != MPI_SUCCESS) {
	      fprintf(efptr, "[RANK %d] ERROR: Unable to send I/O processor flag to processor %d.\n", my_rank, i);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }

/*          ****************************************************************
*           Since all the I/O processors on your host are assigned, find one
*           of them with the least number of processors sending data to it.
*           ****************************************************************/
	    k = dummy_int = num_procs_world + 1;
	    for(j = 0; j < i; j++)
	      if( (hostnum_list[i] == hostnum_list[j]) && 
		  (send_counts[j] > 0) && (send_counts[j] < dummy_int)){
		dummy_int = send_counts[j];
		k = j;
	      }
	    
	    if( dummy_int < (num_procs_world + 1) && 
		k < (num_procs_world + 1)){
	      send_counts[k]++;
	      if( MPI_Isend(&k, 1, MPI_INT, i, i, MPI_COMM_WORLD, 
			    &temp_request) != MPI_SUCCESS) {
		fprintf(efptr, "[RANK %d] ERROR: Unable to send destination rank to processor %d.\n", my_rank, i);
		fflush(efptr);
		MPI_Abort(MPI_COMM_WORLD, -1);
	      }
	      
	    }
	    else{
	      fprintf(efptr, "[RANK %d] ERROR: Unable to find a destination processor for processor %d.\n", my_rank, i);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
	    
	  }
	}
      }
	
/*    *******************************************************************
*     Check that we assigned the correct total number of I/O processors
*     and number on each I/O host.
*     *******************************************************************/
      if(index != num_procs_io){
	fprintf(efptr, "[RANK %d] ERROR: Requested number of I/O processors (%d) were not found. Only %d were designated as I/O processors.\n", 
		my_rank, num_procs_io, index);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
      for(i=0; i < num_hosts; i++)
	if(io_procs_list[i] != 0){
	  fprintf(efptr, "[RANK %d] ERROR: Requested number of I/O processors were not found on host %s. %d processors were not designated as I/O processors.\n", 
		  my_rank, host_io_list[i], io_procs_list[i]);
	  fflush(efptr);
	  MPI_Abort(MPI_COMM_WORLD, -1);
	}

/*    *******************************************************************
*     Now assign the destination processor for all processors not on hosts 
*     that do I/O. They will send to a processor on any host with the least 
*     number of processors sending data to it.
*     *******************************************************************/
      for(i=0; i < num_procs_world; i++){
	/* XXX
	fprintf(stderr, "AGG RANK %d: Initial Proc %d send counts %d.\n",
		my_rank, i, send_counts[i]);
	fflush(stderr);
	XXX */
	
	if(hostnum_list[i] == 0){
	  /* XXX
	  fprintf(stderr, "AGG RANK %d: Proc %d not on I/O host, need destination\n",
		  my_rank, i);
	  fflush(stderr);
	  XXX */

	  *io_processor = FALSE;
	  if( MPI_Isend(io_processor, 1, MPI_INT, i, i, MPI_COMM_WORLD, 
			&temp_request) != MPI_SUCCESS) {
	    fprintf(efptr, "[RANK %d] ERROR: Unable to send I/O processor flag to processor %d.\n", my_rank, i);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
	  
	  k = dummy_int = num_procs_world + 1;
	  for(j = 0; j < num_procs_world; j++)
	    if( (hostnum_list[j] > 0) && (send_counts[j] < dummy_int) 
		&& (send_counts[j] > 0 )){
	      dummy_int = send_counts[j];
	      k = j;
	    }
	  
	  if(dummy_int < (num_procs_world + 1) && k < (num_procs_world + 1)){
	    send_counts[k]++;
	    if( MPI_Isend(&k, 1, MPI_INT, i, i, MPI_COMM_WORLD, 
			  &temp_request) != MPI_SUCCESS) {
	      fprintf(efptr, "[RANK %d] ERROR: Unable to send destination rank to processor %d.\n", my_rank, i);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
	  }
	  else{
	    fprintf(efptr, "[RANK %d] ERROR: Unable to find a destination processor for processor %d.\n", my_rank, i);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
	  
	}
      }
      
      /* XXX
      for(i=0; i < num_procs_world; i++){
	fprintf(stderr, "AGG RANK %d: IN THE END  Proc %d send counts %d.\n",
		my_rank, i, send_counts[i]);
	fflush(stderr);
      }
      XXX */

/*    *******************************************************************
*     Now send the number of processors to receive data from to all I/O
*     processors.
*     *******************************************************************/
      index = 0;
      for(i=0; i < num_procs_world; i++)
	if(send_counts [i] > 0){
	  hostnum_list[index++] = send_counts[i];
	  
	  if( MPI_Isend(&send_counts[i], 1, MPI_INT, i, i, MPI_COMM_WORLD, 
			&temp_request) != MPI_SUCCESS) {
	    fprintf(efptr, "[RANK %d] ERROR: Unable to send number of senders to processor %d.\n", my_rank, i);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
	}
      
    }
    
/*  *******************************************************************
*   Receive the I/O processor flag and, based on that flag, set constants 
*   and receive the rank for the processor to send data to.
*   *******************************************************************/
    if( MPI_Recv(io_processor, 1, MPI_INT, 0, my_rank, MPI_COMM_WORLD, 
		 &temp_status) != MPI_SUCCESS) {
      fprintf(efptr, "[RANK %d] ERROR: Unable to receive I/O processor flag.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    
    if(*io_processor){
      *send_to = my_rank;
      if( MPI_Recv(num_receive, 1, MPI_INT, 0, my_rank, MPI_COMM_WORLD, 
		   &temp_status) != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to receive number of processors sending data.\n", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
      /* XXX
      fprintf(stderr, "AGG RANK %d: I/O processor, number receive %d.\n",
	      my_rank, *num_receive);
      fflush(stderr);
      XXX */
    
      if(*num_receive < 1){
	fprintf(efptr, "[RANK %d] ERROR: Processor identified as an I/O processor, but no one sending data to it.\n", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
      if( (*recv_procs = (int *) malloc(*num_receive * sizeof(int))) == NULL){
	fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for processor ranks receive array.\n", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
      (*recv_procs)[0] = my_rank;
      *poffset_val = (MPI_Offset)0.0;
      for(i=1; i< *num_receive; i++){
	if( MPI_Recv(&(*recv_procs)[i], 1, MPI_INT, MPI_ANY_SOURCE, 
		     MPI_ANY_TAG, MPI_COMM_WORLD, &temp_status) != 
	    MPI_SUCCESS) {
	  fprintf(efptr, "[RANK %d] ERROR: Unable to receive processor rank number %d.\n", my_rank, i);
	  fflush(efptr);
	  MPI_Abort(MPI_COMM_WORLD, -1);
	}
	if( MPI_Isend(&i, 1, MPI_INT, (*recv_procs)[i], (*recv_procs)[i], 
		      MPI_COMM_WORLD, &temp_request) != MPI_SUCCESS) {
	  fprintf(efptr, "[RANK %d] ERROR: Unable to send offset to source processor number %d.\n", my_rank, i);
	  fflush(efptr);
	  MPI_Abort(MPI_COMM_WORLD, -1);
	}
	if( MPI_Isend(num_receive, 1, MPI_INT, (*recv_procs)[i], (*recv_procs)[i],
		      MPI_COMM_WORLD, &temp_request) != MPI_SUCCESS) {
	  fprintf(efptr, "[RANK %d] ERROR: Unable to send number receive to source processor number %d.\n", my_rank, i);
	  fflush(efptr);
	  MPI_Abort(MPI_COMM_WORLD, -1);
	}
      }	
    }
    else{
      /* XXX
      fprintf(stderr, "AGG RANK %d: NOT I/O processor send to %d.\n",
	      my_rank, *send_to);
      fflush(stderr);
      XXX */
      
      if( MPI_Recv(send_to, 1, MPI_INT, 0, my_rank, MPI_COMM_WORLD, 
		   &temp_status) != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to receive destination processor rank.\n", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
      if( MPI_Isend(&my_rank, 1, MPI_INT, *send_to, my_rank, 
		    MPI_COMM_WORLD, &temp_request) != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to send processor rank to destination processor rank %d.\n", my_rank, *send_to);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
      if( MPI_Recv(&dummy_int, 1, MPI_INT, *send_to, my_rank, MPI_COMM_WORLD, 
		   &temp_status) != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to receive offset from processor %d.\n", my_rank, *send_to);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
      *poffset_val = (MPI_Offset)dummy_int;
      if( MPI_Recv(num_receive, 1, MPI_INT, *send_to, my_rank, 
		   MPI_COMM_WORLD, &temp_status) != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to receive number of processors sending data.\n", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
    }
    
    /* XXX
    fprintf(stderr, "AGG RANK %d: I/O processor %d number receive %d send to %d offset %lf.\n",
	    my_rank, *io_processor, *num_receive, *send_to, *poffset_val);
    fflush(stderr);
    if(*io_processor)
      for(j=0; j < *num_receive; j++){
	fprintf(stderr, "AGG RANK %d: receive from %d =  %d.\n",
		my_rank, j+1, (*recv_procs)[j]);
	fflush(stderr);
      }
    
    if(my_rank == 0){
      for(j=0; j < num_procs_io; j++)
	fprintf(stderr, "AGG RANK %d: io proc rank %d =  %d.\n", j, 
		my_rank, io_ranks[j]);
      fflush(stderr);
    }
    XXX */
    
/*  *******************************************************************
*   Broadcast the list of processor ranks that will perform I/O.
*   *******************************************************************/
    if( MPI_Bcast(io_ranks, num_procs_io, MPI_INT, 0, 
		  MPI_COMM_WORLD) !=  MPI_SUCCESS) {
      fprintf(efptr, "[RANK %d] ERROR: Unable to broadcast array of I/O processor ranks to all processors.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    
    if (MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(efptr, "[RANK %d] ERROR: MPI_Barrier for write.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    
    if(test_type == 4){
      if(MPI_Bcast(hostnum_list, num_procs_io, MPI_INT, 0, 
		   MPI_COMM_WORLD) !=  MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to broadcast array of number receives to all processors.\n", my_rank);
	MPI_Finalize();
	return -1;
      }
      
      for(i=0; i < num_procs_io; i++)
	if(io_ranks[i] < *send_to)
	  *poffset_val += (MPI_Offset)hostnum_list[i];
      
      *poffset_val *= (MPI_Offset)(obj_size * num_objs);
      *poffset_val += (MPI_Offset)1.0;
    }
    else{
      *poffset_val *= (MPI_Offset)(obj_size * num_objs);
      *poffset_val += (MPI_Offset)1.0;
    }
    
    /* XXX
    fprintf(stderr, "AGG RANK %d: Final  offset %lf.\n",
	    my_rank, *poffset_val);
    fflush(stderr);
    XXX */
  }
  else{
     
/*  ************************************************************
*   Since the user has not specified the hosts to perform I/O, evenly space
*   the I/O processors by rank.
*   *************************************************************/
    if( (io_ranks = (int *) malloc( num_procs_io * sizeof(int))) == NULL){
      fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for I/O processor array.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    
    *num_receive = num_procs_world / num_procs_io;
    for ( i = 0 ; i < num_procs_io ; i++ ) {
      io_ranks[i] = *num_receive * i;
    }
    
    dummy_int = num_procs_world%num_procs_io;
    if(dummy_int != 0){
      if(my_rank < dummy_int*(*num_receive+1))
	(*num_receive)++;
      
      for ( i = 1; i < dummy_int; i++ ){
	io_ranks[i] += i;
      }
      
      for ( i = dummy_int; i < num_procs_io; i++ )
	io_ranks[i] += dummy_int;
    }
    
    for(i=0; i< num_procs_io; i++)
      if(my_rank >= io_ranks[i])
	*send_to = io_ranks[i] ;
    
    if( (*recv_procs = (int *) malloc(*num_receive * sizeof(int))) == NULL){
      fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for receive data from processor ranks array.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    
    for(i=0; i< *num_receive; i++)
      (*recv_procs)[i] = *send_to + i;
    /* XXX
    fprintf(stderr,"AGG RANK %d: Send data to rank %d receive %d.\n",
	    my_rank, *send_to, *num_receive);
    
    for(i=0; i< *num_receive; i++)
      fprintf(stderr, "AGG RANK %d: Receive[%d] = %d.\n", my_rank, i, 
	      (*recv_procs)[i]);
    fflush(stderr);
    */ 
/*  ************************************************************
*   Set the processor offset value to be written at the beginning of each
*   object. This value will be used to check that objects were written in 
*   the correct order.
*   *************************************************************/
    if(test_type == 3) 
      *poffset_val = (MPI_Offset)((my_rank - *send_to) * obj_size * num_objs) +1.0;
    else
      *poffset_val = (MPI_Offset)(my_rank * obj_size * num_objs) + 1.0;
  } /* else{ */
  
/**********************************************************
* Compose the new group and communicators
**********************************************************/
  if( MPI_Comm_group(MPI_COMM_WORLD, &group_world) != MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: (MPI_Comm_Group, m)", my_rank);
    MPI_Finalize();
    return -1;
  }
  
  if( MPI_Group_incl( group_world, num_procs_io, io_ranks, &group_io) 
      != MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: (MPI_Group_incl)", my_rank);
    MPI_Finalize();
    return -1;
  }
  
  if ( MPI_Comm_create( MPI_COMM_WORLD, group_io, comm_io) != 
       MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: (MPI_Comm_create)", my_rank);
    MPI_Finalize();
    return -1;
  }
  
  if ( MPI_Comm_group(MPI_COMM_WORLD, &group_world) != MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: (MPI_Comm_Group, m)", my_rank);
    MPI_Finalize();
    return -1;
  }
  
  if (MPI_Group_excl( group_world, num_procs_io,
		      io_ranks, &group_io) != MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: (MPI_Group_incl)", my_rank);
    MPI_Finalize();
    return -1;
  }
  
  if( MPI_Comm_create( MPI_COMM_WORLD, group_io, noncomm_io) !=
      MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: (MPI_Comm_create)", my_rank);
    MPI_Finalize();
    return -1;
  }
  
  MPI_Group_free( &group_io);
  MPI_Group_free( &group_world);
  if (io_ranks != NULL) free(io_ranks);
  
  *my_rank_io = -1000 ;
  *io_processor = FALSE;
  
  if(*comm_io != MPI_COMM_NULL){
    *io_processor = TRUE;
    if (MPI_Comm_rank(*comm_io, my_rank_io) != MPI_SUCCESS) {
      fprintf(efptr, "[RANK %d] ERROR: (MPI_Comm_rank)", my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
    
    if ( MPI_Comm_size(*comm_io, &np_comm) != MPI_SUCCESS) {
      fprintf(efptr, "[RANK %d] ERROR: (MPI_Comm_size)", my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
    
    if(num_procs_io != np_comm){
      fprintf(efptr,"ERROR: Error computing size of I/O communicator; input %d commumicator size %d.\n", num_procs_io, np_comm);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
    
    /* XXX
    fprintf(stderr,"AGG Rank %d: Part of comm_io with rank %d\n", 
	    my_rank, *my_rank_io);
    fflush(stderr);
    XXX */
  }
  
  /* XXX
  fprintf(stderr, "AGG RANK %d: IO proc %d Send data to rank %d receive %d.\n",
	  my_rank, *io_processor, *send_to, *num_receive);
  
  if(*io_processor)
    for(i=0; i< *num_receive; i++)
      fprintf(stderr, "AGG RANK %d: Receive[%d] = %d.\n", my_rank, i, 
	      (*recv_procs)[i]);
  fflush(stderr);
  XXX */
  
/**********************************************************
* If the IO procs are not sending data to themselves, decrement the number 
* of processors to receive from and modify the processor receive list.
**********************************************************/
  if(!io_proc_send_flag && *io_processor){
    
    for(i=0; i< *num_receive; i++)
      if((*recv_procs)[i] == my_rank){
	for(j=i+1; j< *num_receive; j++)
	  (*recv_procs)[j-1] = (*recv_procs)[j];
	(*num_receive)--;
      }
    
  /* XXX
    fprintf(stderr, "AGG RANK %d: Number to receive changed to %d; requested receiver no send.\n", my_rank, *num_receive);
    for(i=0; i< *num_receive; i++)
      fprintf(stderr, "AGG RANK %d: Receive[%d] = %d.\n", my_rank, i, 
	      (*recv_procs)[i]);
    fflush(stderr);
  XXX */
  }

/***********************************************************************
* Compute number of processors to fill a buffer
************************************************************************/
  *f_procs = (int)(cbuf_size / (num_objs*obj_size)) ;
  
  if( *f_procs == 0 ){
    if(my_rank == 0)
      fprintf(efptr, "ERROR: Circular buffer size (%d) must be equal to or larger than the data buffer (%d)", (int)cbuf_size, (int)obj_size);
    MPI_Finalize();
    return -1;
  }
  
  *num_of_cjob = *num_receive / *f_procs ; 
  if ( *num_receive % *f_procs != 0 ) (*num_of_cjob)++ ;
  
  /* XXX
  fprintf(stderr,"AGG RANK %d: Number of cjobs = %d (*num_receive/f_procs %d/%d).\n",
	  my_rank, *num_of_cjob, *num_receive, *f_procs);
  fflush(stderr);
  XXX */

  *num_file_writes = 1;
  
  return 1;
}

/*******************************************
 *   ROUTINE: 
 *   PURPOSE: Find the minimum number of processors sending data to any 
 *            of the I/O processors.
 *******************************************/
int find_min_jobs(int my_rank, int num_procs_io, int num_of_cjob,
		  int *min_all_proc_cjobs, MPI_Comm comm_io, 
		  FILE *ofptr, FILE *efptr)
{
  int *send_counts = NULL;
  int j;
  
  if( (send_counts = (int *) malloc(num_procs_io * sizeof(int))) == NULL){
    fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for receive buffer counts array.\n", my_rank);
    fflush(efptr);
    return -1;
  }
  
  if(MPI_Allgather(&num_of_cjob , 1, MPI_INT, send_counts, 1, MPI_INT, 
		   comm_io) != MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: Unable to gather number of processors to send data to all I/O processors.\n", my_rank);
    fflush(efptr);
    return -1;
  }
  
  *min_all_proc_cjobs = num_of_cjob;
  for(j=0; j < num_procs_io; j++)
    if(send_counts[j] < *min_all_proc_cjobs) 
      *min_all_proc_cjobs = send_counts[j];
  
  free(send_counts);
  return 1;
}
