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
* PROGRAM:   MPIIO_ops
*
* PURPOSE:   Routines that perform read or write operations
*
* AUTHOR:    James Nunez 
* DATE:      February 12, 2001
* LAST MODIFIED: December 15, 2006 [jnunez]
* VERSION:   1.00.007
*
******************************************************************/
#include "mpi_io_test.h"

/*******************************************
 *   ROUTINE: MPIIO_master_write
 *   PURPOSE: The write data to file
 *   DATE:    
 *   LAST MODIFIED: December 15, 2006 [jnunez]
 **********************************************/
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
		       double *file_open_wait_time, double *prealloc_wait_time,
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
		       int strided_flag, int *cbuf_use,
                       int barrier_flag, int sync_flag,
		       FILE *ofptr, FILE *efptr){

  MPI_File    wfh;         /* MPI file handle for writes               */
  MPI_Offset  offset = 0;
  MPI_Info info_used = MPI_INFO_NULL;/* Info struct after file opened  */
  MPI_Request *send_req ;  /* MPI_send request                      */
  MPI_Request *recv_req ;  /* MPI_recv request                      */
#ifndef ROMIO
  MPI_Request *write_req ;/* MPI_File_write request                */
  MPI_Request *read_req ; /* MPI_File_read  request                */
#else
  MPIO_Request *write_req ;/* MPI_File_write request                */
  MPIO_Request *read_req ; /* MPI_File_read  request                */
#endif
  MPI_Status *send_stat;   /* MPI_send status                       */
  MPI_Status *recv_stat;   /* MPI_recv status                       */
  MPI_Status *write_stat;  /* MPI_File_write status                 */
  double *m_obj = NULL;
  double ta = 0.0,  tb = 0.0; 
  double wait_start = 0.0;          
  double barrier_start = 0.0;          
  double *tmp_p = NULL;    /* Array of data objects in comm_io        */
  double **c_buf = NULL;   /* Array of circular buffers for async IO  */

  size_t blklen = 0; 
  size_t objsize_div = obj_size/sizeof(double);

  int r_procs;            /* remain procs */
  int start_p = 0, end_p = 0;
  int icbuf ;              /* circular buffer index                   */
  int index = 0, dummy_int = 0; 
  int i, j, k;

/***********************************************************************
* Before starting the write process, allocate memory for all write buffers,
* MPI request and status structures.
****************************************************************/
  if (io_processor) {
    if( (c_buf = (double **) valloc(num_of_cbuf * sizeof(double *))) == NULL){
      fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for the circular buffer.\n", my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
    
    for ( i = 0 ; i < num_of_cbuf ; i++ )
      if( (c_buf[i] = (double *)valloc(cbuf_size)) == NULL) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for the circular buffer %d (valloc failed).\n", my_rank, i+1);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
  }

  if( (send_req = (MPI_Request *) malloc(num_objs *
                                         sizeof(MPI_Request))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for MPI_request array.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
  
  if( (send_stat = (MPI_Status *) malloc(num_objs *
                                         sizeof( MPI_Status))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for.\n", my_rank);
    MPI_Finalize();
    return 0;
  }

  if( (recv_req = (MPI_Request *) malloc(num_objs * num_receive *
                                         sizeof( MPI_Request))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
  
  if( (recv_stat = (MPI_Status *) malloc(num_objs * num_receive *
                                         sizeof( MPI_Status))) == NULL){
    fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
  
#ifndef ROMIO
  if( (write_req = (MPI_Request *) malloc(num_of_cbuf * num_file_writes *
					  sizeof( MPI_Request))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for write request.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
  
#else
  if( (write_req = (MPIO_Request *) malloc(num_of_cbuf * num_file_writes *
					   sizeof( MPIO_Request))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for write request.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
#endif
  
  
  if( (write_stat = (MPI_Status *) malloc(num_of_cbuf * num_file_writes *
					  sizeof( MPI_Status))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for write status structure.\n", my_rank);
    MPI_Finalize();
    return 0;
  }

/***********************************************************************
* If using aggregation, all io processors must allocate memory for a message 
* and fill in the values of the message. Note that valloc allocates page 
* alligned memory.
***********************************************************************/
  if (aggregate_flag) {
    
    if( (m_obj = (double *)valloc( obj_size * sizeof(char))) == NULL) {
      fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for the data buffer m_obj.\n", my_rank);
      MPI_Finalize();
      return 0;
    }
    
    if(!fill_buffer(my_rank, m_obj, blksize, touch, pagesize, poffset_val, 
		    ofptr, efptr )){
      fprintf(efptr, "[RANK %d] ERROR: Unable to initialize the data buffer m_obj.\n", my_rank);
      MPI_Finalize();
      return 0;
    }
  }      
  
/***********************************************************************
* Start write master timer, which includes file open, writing the data, file
* sync (if requested) and file close. Then call the write routine.
***********************************************************************/
  ta = MPI_Wtime();

/***********************************************************************
* All I/O processors, open the checkpoint file(s) and time the open call..
***********************************************************************/
  if (io_processor) {
    
    if(write_file_flag){
      
      if( MPI_File_open(comm_file, tfname, amode_write, info, &wfh)
	  != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to open file %s for write.\n",
		my_rank, tfname);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }

      *file_open_wait_time = MPI_Wtime() - ta;
      
      wait_start = MPI_Wtime();
      if(prealloc_size > -1)
	if( MPI_File_preallocate(wfh, prealloc_size) != MPI_SUCCESS) {
	  fprintf(efptr, "[RANK %d] ERROR: Unable to preallocate before write.\n",
		  my_rank);
	  fflush(efptr);
	  MPI_Abort(MPI_COMM_WORLD, -1);
	}
      *prealloc_wait_time =  MPI_Wtime() - wait_start;

      
/*      *******************************************************************
*       Now that the file is open, all processors will get the MPI file hints 
*       and values if hints were set. All processors get the file hints so 
*       that if there are any differences in hints, we can report them.
*       *******************************************************************/
      if(info_allocated_flag){
	MPI_File_get_info(wfh, &info_used);      
	MPI_Info_get_nkeys(info_used, nkeys);
	
	if( ((*key) = (char **)malloc((*nkeys) * sizeof(char *)) ) == NULL){
	  fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for array of hints.\n", my_rank);
	  MPI_Finalize();
	  return 0;
	}
	
	if( (*value = (char **)malloc((*nkeys) * sizeof(char *)) ) == NULL){
	  fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for array of hint values.\n", my_rank);
	  MPI_Finalize();
	  return 0;
	}
	
	for(i=0; i < (*nkeys); i++){
	  if( ((*key)[i] = (char *)malloc(200 * sizeof(char)) ) == NULL){
	    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for array of hints.\n", my_rank);
	    MPI_Finalize();
	    return 0;
	  }
	  if( ((*value)[i] = (char *)malloc(200 * sizeof(char)) ) == NULL){
	    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for array of hint values.\n", my_rank);
	    MPI_Finalize();
	    return 0;
	  }
	  
	  MPI_Info_get_nthkey(info_used, i, (*key)[i]);
	  MPI_Info_get(info_used, (*key)[i], 200, (*value)[i], &dummy_int);
	  
	}
	/*
	  if(my_rank_io == 0)
	  MPIIO_get_hints(my_rank, wfh, nkeys, key, value, ofptr, efptr);
	*/
      }
      
    }
  }
 
/***********************************************************************
* Start timing the write process and, if using aggregators, send the *
 first data object. 
* **********************************************************************/
  tb = MPI_Wtime();
  if (io_processor){
    
    if(aggregate_flag && io_proc_send_flag){
      index = 0;
      
      if( MPI_Isend(m_obj, blksize, MPI_DOUBLE, send_to, index,
		    MPI_COMM_WORLD, &send_req[index]) != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Problem sending the first data object.\n", 
		my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
#ifdef DEBUG_IO
      fprintf(stderr,"RANK %d: IO Proc sent first message to %d.\n",
	      my_rank, send_to);
      fflush(stderr);
#endif
      
    }
    
/*  **********************************************************************
*   Enter sending and writing to file loop.
*   **********************************************************************/
    for( i = 0; i < num_of_cjob; i++ ){
      icbuf = i % num_of_cbuf ;
      
      if( i < num_of_cbuf )  init_flag = 1;
      else                   init_flag = 0;
      
      start_p = i * f_procs ;
      r_procs = num_receive - start_p;
      if ( r_procs > f_procs) {
	end_p = start_p + f_procs;
	blklen = cbuf_size/sizeof(double);
      }
      else{
	end_p = start_p + r_procs;
	if(aggregate_flag){
	  blklen = num_objs * (objsize_div) * r_procs;
	  
	  if (blklen > INT_MAX) {  
	    fprintf(efptr, "[RANK %d] ERROR: (blklen > INT_MAX)", my_rank);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
	}
	else
	  blklen = objsize_div;
      }

/*    *******************************************************************
*     If using aggregators, receive data into one of the circular
*     buffer and collect time it takes to receive all messages
*     *******************************************************************/
      if(aggregate_flag){

/*      *****************************************************************
*       If not the first run, wait for the data in the current (one we want to 
*       write to) circular buffer to be written to file.
*       ******************************************************************/
	if( (init_flag == 0)  && write_file_flag) { 
	  wait_start = MPI_Wtime() ;
	  
	  
	  for(j=0; j < num_file_writes; j++){
#ifndef ROMIO
	    if( MPI_Wait( &write_req[icbuf * num_file_writes + j],
			  &write_stat[icbuf * num_file_writes + j]) !=
		MPI_SUCCESS) {
	      fprintf(efptr, "[RANK %d] ERROR: Problem with initial MPI_Wait.\n", 
		      my_rank);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }

#else
	    if( MPIO_Wait( &write_req[icbuf * num_file_writes + j],
			   &write_stat[icbuf * num_file_writes + j]) !=
		MPI_SUCCESS) {
	      fprintf(efptr, "[RANK %d] ERROR: Problem with initial MPIO_Wait.\n", 
		      my_rank);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
#endif
	  }
	  *file_op_wait_time += MPI_Wtime() - wait_start ;
	}
	
/*      *****************************************************************
*       Receive data objects from any source with any tag. Once we have 
*       received one of our own data objects, it's okay to send another one.
*       ******************************************************************/
	wait_start = MPI_Wtime();
	for ( j = start_p ; j < end_p ; j++ ) {
	  for(k=0; k < num_objs; k++){
	    
	    tmp_p = c_buf[icbuf] + (k + (j - start_p)*num_objs) * objsize_div;
	    
	    if( MPI_Irecv(tmp_p, (int)blklen, MPI_DOUBLE, MPI_ANY_SOURCE, 
			  MPI_ANY_TAG, MPI_COMM_WORLD,
			  &recv_req[(j - start_p)*num_objs + k]) !=
		MPI_SUCCESS) {
	      fprintf(efptr, "[RANK %d] ERROR: Problem with receiving from processor %d object %d.\n", my_rank, j, k+1);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
	  }
	}
	
	for(j=0; j < num_objs*(end_p - start_p); j++){
	  if( MPI_Wait( &recv_req[j], &recv_stat[j])!= MPI_SUCCESS) {
	    fprintf(efptr, "[RANK %d] ERROR: MPI_Wait for receive request %d.\n", my_rank, j);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
#ifdef DEBUG_IO
	  fprintf(stderr,"RANK %d: Received data object %d of %d from %d\n", 
		  my_rank, j+1, (int)(num_objs*(end_p - start_p)),
		  (int)c_buf[icbuf][j*obj_size/(sizeof(double)) + 1]);
	  fflush(stderr);
#endif
	  
	  
	  if(io_proc_send_flag && (index < (num_objs - 1)) ){
	    
	    MPI_Test(&send_req[index], &dummy_int, &send_stat[index]);
	    if(dummy_int){
	      index++;
	      
	      if(!fill_buffer(my_rank, m_obj, blksize, touch, pagesize, 
			      poffset_val, ofptr, efptr) ){
		fprintf(efptr, "[RANK %d] ERROR: Unable to fill the data buffer m_obj.\n", my_rank);
		MPI_Finalize();
		return 0;
	      }
	      
#ifdef DEBUG_IO
	      fprintf(stderr,"RANK %d: BEFORE send message to %d.\n",
		      my_rank, send_to);
	      fflush(stderr);
#endif
	      if( MPI_Isend(m_obj, blksize, MPI_DOUBLE, send_to, index,
			    MPI_COMM_WORLD, &send_req[index]) != MPI_SUCCESS) {
		fprintf(efptr, "[RANK %d] ERROR: Problem sending the data object %d.\n", 
			my_rank, index+1);
		fflush(efptr);
		MPI_Abort(MPI_COMM_WORLD, -1);
	      }
#ifdef DEBUG_IO
	      fprintf(stderr,"RANK %d: Sent message to %d.\n",
		      my_rank, send_to);
	      fflush(stderr);
#endif
	    }
	  }
	} /* j */
	
	
	*proc_recv_wait_time += MPI_Wtime() - wait_start ;
      } /* end if(aggregation_  */
      
/*    *********************************************************
*     If not using collective write calls, seek into and write the 
*     current buffer to file.
*     *********************************************************/
      if(!collective_flag){
	for(j=0; (j < num_file_writes) && write_file_flag; j++)
        {
/*        ************************************************************
*         If specified, call barrier before each write.
*         *********************************************************/
          if (barrier_flag)
          {
	    barrier_start = MPI_Wtime();
	    if( MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS)
            {
	      fprintf(efptr,"[RANK %d] ERROR: Unable to call MPI_Barrier before write.\n", my_rank);
	        fflush(efptr);
	        MPI_Abort(MPI_COMM_WORLD, -1);
	    }
            *barrier_wait_time += MPI_Wtime() - barrier_start;
          }

	  if(set_view_all){

	    if(strided_flag == 0) set_view_all = FALSE;
	    
	    offset = ((MPI_Offset)(i + j) * other_offset) + file_offset;
	    
#ifdef DEBUG_IO
	    fprintf(stderr,"RANK %d: MPIIO_OPS offset = %u i = %d j = %d\n",
		    my_rank, (unsigned long)offset, i, j);
	    fflush(stderr);
#endif
	    
	    if( MPI_File_seek(wfh, offset, MPI_SEEK_SET) != MPI_SUCCESS){
	      fprintf(efptr,"[RANK %d] ERROR: Unable to seek for write.\n",
		      my_rank);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
	  }
	  
/*      *********************************************************
*       If not using aggregators, then we are writing directly from buffer to 
*       file, i.e. no circular buffers. Thus, make sure last file write is 
*       done before touching next data object.
*       *********************************************************/
	  if(!aggregate_flag){
	    if(j > 0){
	      wait_start = MPI_Wtime() ;
#ifndef ROMIO
	      if( MPI_Wait( &write_req[icbuf * num_file_writes + (j-1)],
			    &write_stat[icbuf * num_file_writes + (j-1)]) !=
		  MPI_SUCCESS) {
		fprintf(efptr,"[RANK %d] ERROR: Problem with write MPI_Wait.\n",
			my_rank);
		fflush(efptr);
		MPI_Abort(MPI_COMM_WORLD, -1);
	      }
#else	      
	      if( MPIO_Wait( &write_req[icbuf * num_file_writes + (j-1)],
			     &write_stat[icbuf * num_file_writes + (j-1)]) !=
		  MPI_SUCCESS) {
		fprintf(efptr,"[RANK %d] ERROR: Problem with write MPIO_Wait.\n",
			my_rank);
		fflush(efptr);
		MPI_Abort(MPI_COMM_WORLD, -1);
	      }
#endif      
	      *file_op_wait_time += MPI_Wtime() - wait_start ;
	    }
	    
	    
	    if(!fill_buffer(my_rank, c_buf[icbuf], (int)blklen, touch, pagesize, 
			    poffset_val + (double)(j*obj_size), ofptr, efptr)){
	      fprintf(efptr, "[RANK %d] ERROR: Unable to initialize the data buffer c_buf.\n", my_rank);
	      MPI_Finalize();
	      return 0;
	    }
	  }
	  
	  if (MPI_File_iwrite(wfh, c_buf[icbuf], (int)blklen, MPI_DOUBLE,
			       &write_req[icbuf * num_file_writes + j] ) != 
	      MPI_SUCCESS) {
	    fprintf(efptr, "[RANK %d] ERROR: Problem writing to file; MPI_File_iwrite.\n", 
		    my_rank);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }

	}
      }
      else{
/*      ******************************************************
 *      For collective writes, since we use a blocking collective write, 
 *      collect time for the routine to return.
 *      *******************************************************/
	for(j=0; (j < num_file_writes) && write_file_flag; j++)
        {
/*        ************************************************************
*         If specified, call barrier before each write.
*         *********************************************************/
          if (barrier_flag)
          {
	    barrier_start = MPI_Wtime();
	    if( MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS)
            {
	      fprintf(efptr,"[RANK %d] ERROR: Unable to call MPI_Barrier before write.\n", my_rank);
	        fflush(efptr);
	        MPI_Abort(MPI_COMM_WORLD, -1);
	    }
            *barrier_wait_time += MPI_Wtime() - barrier_start;
          }

	  offset = ((MPI_Offset)(i + j) * other_offset) + file_offset;
	  
	  if(!fill_buffer(my_rank, c_buf[icbuf], (int)blklen, touch, pagesize, 
			  poffset_val + (double)(j * obj_size), ofptr, efptr )){
	    fprintf(efptr, "[RANK %d] ERROR: Unable to initialize the data buffer c_buf.\n", my_rank);
	    MPI_Finalize();
	    return 0;
	  }
	  
	  wait_start = MPI_Wtime() ;
	  
	  if( MPI_File_write_at_all(wfh, offset, c_buf[icbuf], (int)blklen, 
				    MPI_DOUBLE, 
				    &write_stat[icbuf * num_file_writes + j]) 
	      != MPI_SUCCESS){
	    fprintf(efptr,"[RANK %d] ERROR: Problem writing to file; MPI_File_write_at_all.\n", 
		    my_rank);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
	  *file_op_wait_time += MPI_Wtime() - wait_start ;
	}
      }
      
      cbuf_use[icbuf] = 1 ; /* this c_buf[] is in use. */
      
#ifdef DEBUG_IO
      fprintf(stderr,"RANK %d: Finished loop %d of %d\n", 
	      my_rank, i+1, num_of_cjob);
      fflush(stderr);
#endif
    } /* for ( i = 0 ; */
      
/*  ****************************************************************
*   Wait until all previous writings to file are done.
*   *****************************************************************/
    wait_start = MPI_Wtime() ;
    
    for ( icbuf = 0 ; icbuf < num_of_cbuf && !collective_flag; icbuf++ ) {
      if (cbuf_use[icbuf] == 1 && write_file_flag) {
	for(j=0; j < num_file_writes; j++){
#ifndef ROMIO
	  if( MPI_Wait( &write_req[num_file_writes * icbuf + j],
			&write_stat[num_file_writes * icbuf + j])
	      != MPI_SUCCESS){
	    fprintf(efptr, "[RANK %d] ERROR: Problem with the final write MPI_Wait.\n",
		    my_rank);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
#else
	  if( MPIO_Wait( &write_req[num_file_writes * icbuf + j],
			 &write_stat[num_file_writes * icbuf + j])
	      != MPI_SUCCESS){
	    fprintf(efptr, "[RANK %d] ERROR: Problem with the final write MPIO_Wait.\n",
		    my_rank);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
#endif
	}
      }
    }
    
    *file_op_wait_time += MPI_Wtime() - wait_start ;
  } /* if (io_processor) */
  else{
    
/*  ****************************************************************
*   For all non I/O processors, send all data objects.
*   *****************************************************************/
    wait_start = MPI_Wtime();
    for(j=0; j < num_objs; j++){
      if(!fill_buffer(my_rank, m_obj, blksize, touch, pagesize, 
		      poffset_val + (MPI_Offset)(j * (objsize_div)), ofptr, efptr )){
	fprintf(efptr, "[RANK %d] ERROR: Unable to initialize the data buffer m_obj.\n", my_rank);
	MPI_Finalize();
	return 0;
      }
      
#ifdef DEBUG_IO
      fprintf(stderr,"RANK %d: Sending message %d of %d\n", 
	      my_rank, j+1, num_objs);
      fflush(stderr);
#endif
      
      if( MPI_Send(m_obj, blksize, MPI_DOUBLE, send_to, j,
		   MPI_COMM_WORLD) != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Problem sending data object %d.\n", 
		my_rank, j+1);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
    }
    
    *proc_send_wait_time = MPI_Wtime() - wait_start ;
  }
    
/*****************************************************************
* Stop the write timer, sync the data, and close the file
******************************************************************/
  *total_op_time = MPI_Wtime() - tb;  
  
  if(io_processor && write_file_flag){
    
    if(sync_flag){
      wait_start = MPI_Wtime();
      
      if ( MPI_File_sync(wfh) != MPI_SUCCESS){
	fprintf(efptr, "[RANK %d] ERROR: MPI_File_sync returned unsuccessfully for write.\n", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
      
      *file_sync_wait_time =  MPI_Wtime() - wait_start;
    }
    
    wait_start = MPI_Wtime();
    if (MPI_File_close(&wfh) != MPI_SUCCESS){
      fprintf(efptr, "[RANK %d] ERROR: MPI_File_close returned unsuccessfully for write.\n", 
	      my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

    *file_close_wait_time =  MPI_Wtime() - wait_start;
  }
  
/*******************************************************************
* Stop the master timer, collect and report timing results
*******************************************************************/
  *total_time = MPI_Wtime() - ta; 
  
/***********************************************************************
* Before returning, free all allocated memeory.
***********************************************************************/
  free(send_req);
  free(recv_req);
  free(write_req);
  free(send_stat);
  free(recv_stat);
  free(write_stat);
  if (aggregate_flag)
    free(m_obj);
  
  return 1;   
}


/*******************************************
 *   ROUTINE: MPIIO_master_read
 *   PURPOSE: The routine that reads data from a file
 *   DATE:    
 *   LAST MODIFIED: December 15, 2006 [jnunez]
 *   RETURN: 1 on success, 0 on error, 2 on data verification failed.
 **********************************************/
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
		      FILE *ofptr, FILE *efptr){

  MPI_File    rfh;         /* MPI file handle for writes               */
  MPI_Offset  offset = 0;
  MPI_Info info_used = MPI_INFO_NULL;/* Info struct after file opened  */
  MPI_Request *send_req ;  /* MPI_send request                      */
  MPI_Request *recv_req ;  /* MPI_recv request                      */
#ifndef ROMIO
  MPI_Request *read_req ; /* MPI_File_read  request                */
#else
  MPIO_Request *read_req ; /* MPI_File_read  request                */
#endif
  MPI_Status *send_stat;   /* MPI_send status                       */
  MPI_Status *recv_stat;   /* MPI_recv status                       */
  MPI_Status *read_stat;  /* MPI_File_write status                 */
  double *m_obj = NULL;
  double ta = 0.0,  tb = 0.0; 
  double wait_start = 0.0;          
  double check_buf_time = 0.0;
  double *tmp_p = NULL;    /* Array of data objects in comm_io        */
  double **c_buf = NULL;   /* Array of circular buffers for async IO  */

  size_t num_err = 0;
  size_t blklen = 0;
  size_t objsize_div = obj_size/sizeof(double);

  int r_procs;            /* remain procs */
  int start_p = 0, end_p = 0;
  int icbuf ;              /* circular buffer index                   */
  int index = 0, dummy_int = 0; 
  int i, j, k, rc=1;

/***********************************************************************
* Before starting the read process, allocate memory for all write buffers,
* MPI request and status structures.
****************************************************************/
  if (io_processor) {
    if( (c_buf = (double **) valloc(num_of_cbuf * sizeof(double *))) == NULL){
      fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for the circular buffer.\n", my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
    
    for ( i = 0 ; i < num_of_cbuf ; i++ )
      if( (c_buf[i] = (double *)valloc(cbuf_size)) == NULL) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for the circular buffer %d.\n", my_rank, i+1);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
  }
  
  if( (send_req = (MPI_Request *) malloc(num_objs *
					 sizeof(MPI_Request))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for MPI_request array.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
  
  if( (recv_req = (MPI_Request *) malloc(num_objs * num_receive *
					 sizeof( MPI_Request))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
#ifndef ROMIO
  if( (read_req = (MPI_Request *) malloc(num_of_cbuf * num_file_writes *
					 sizeof( MPI_Request))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for read request.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
#else
  if( (read_req = (MPIO_Request *) malloc(num_of_cbuf * num_file_writes *
					  sizeof( MPIO_Request))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for read request.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
#endif
  
  if( (send_stat = (MPI_Status *) malloc(num_objs *
                                         sizeof( MPI_Status))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
  
  if( (recv_stat = (MPI_Status *) malloc(num_objs * num_receive *
                                         sizeof( MPI_Status))) == NULL){
    fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
  
  if( (read_stat = (MPI_Status *) malloc(num_of_cbuf * num_file_writes *
                                         sizeof( MPI_Status))) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for.\n", my_rank);
    MPI_Finalize();
    return 0;
  }

/***********************************************************************
* If using aggregation, all io processors must allocate memory for a message 
* and fill in the values of the message. Note that valloc allocates page 
* alligned memory.
***********************************************************************/
  if (aggregate_flag) {
    
    if( (m_obj = (double *)valloc( obj_size * sizeof(char))) == NULL) {
      fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for the data buffer m_obj.\n", my_rank);
      MPI_Finalize();
      return 0;
    }

    for ( i = 0 ; i < objsize_div ; i++ ) {
      m_obj[i] = -1000.0 ;
    }
  }      
  
/*  *************************************************************************
*   Start the master timer and open the file for read
*   *****************************************************************/
    ta = MPI_Wtime();
    
    if (io_processor){
      if ( MPI_File_open(comm_file, tfname, amode_read, info, &rfh)
	   != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to open file %s to read.\n", 
		my_rank, tfname);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
    }
    
    *file_open_wait_time =  MPI_Wtime() - ta;
    
    /*
    if(my_rank_io == 0){
      MPIIO_get_hints(my_rank, rfh, &nkeys, &key, &value, ofptr, efptr);
      MPIIO_print_hints(my_rank, nkeys, key, value, ofptr, efptr);
    }
    */

/*  ******************************************************************
*   If using aggregators, each process will get num_objs from only one
*   processor, so post receive requests. All data gets put into same buffer.
*   Non I/O processors will use blocking receives and check the rank in the 
*   object and, if necessary, check the offset values.
*   ******************************************************************/
    tb = MPI_Wtime();
    if(!io_processor){
      
      if(aggregate_flag){
	wait_start = MPI_Wtime() ;
	
	for(j=0; j < num_objs; j++){
	  if( MPI_Recv(m_obj, blksize, MPI_DOUBLE, send_to, MPI_ANY_TAG, 
		       MPI_COMM_WORLD, &recv_stat[j]) != MPI_SUCCESS) {
	    fprintf(efptr, 
		    "[RANK %d] ERROR: Problem receiving data object %d.\n",
		    my_rank, j+1);
	      fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
	  
/*        *****************************************************
*         If necessary, check the data just read in.
*         **************************************************************/
	  if(check_data_ndx > 0)
	    num_err = verify_buffer(my_rank, m_obj, check_data_ndx, touch, 
				    pagesize, poffset_val + (MPI_Offset)(j * obj_size), 
			      ofptr, efptr); 
	  if( num_err > 0){
	    fprintf(efptr,"[RANK %d] ERROR: %lu data elements in object %d does not match expected values.\n", my_rank, num_err, j+1);
	      rc = 2;
	      MPI_Abort(MPI_COMM_WORLD, 1);
	    }
	}
	/* Necessary ?????
	if( MPI_Barrier(noncomm_io) != MPI_SUCCESS) {
	  fprintf(efptr,"[RANK %d] ERROR: Non I/O processors barrier unsuccessful.\n", my_rank);
	  fflush(efptr);
	  MPI_Abort(MPI_COMM_WORLD, -1);
	}
	*/
	*proc_recv_wait_time = MPI_Wtime() - wait_start ;
      }
    }
    else{
      
/*  ****************************************************************
*   For all I/O processors, post receives and loop over reading from file 
*   and sending data objects to processors.
*   *******************************************************************/
      if(aggregate_flag){

	for(j=0; j < num_objs && io_proc_send_flag; j++)
	  if (MPI_Irecv( m_obj, blksize, MPI_DOUBLE, send_to, MPI_ANY_TAG,
			 MPI_COMM_WORLD, &recv_req[j]) != MPI_SUCCESS) {
	    fprintf(efptr, "[RANK %d] ERROR: Problem receiving data object %d.\n",
		    my_rank, j+1);
	      fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }
      }
      
      for( i = 0 ; i < num_of_cjob ; i++ ){
	icbuf = i % num_of_cbuf ;
	
	init_flag = 0 ;
	if( i < num_of_cbuf )
	  init_flag = 1;
	
	start_p = i * f_procs ;
	r_procs = num_receive - start_p;
	
	if ( r_procs > f_procs) {
	  end_p = start_p + f_procs;
	  blklen = cbuf_size/sizeof(double);
	}
	else {
	  end_p = start_p + r_procs;
	  if(aggregate_flag){
	    blklen = num_objs * objsize_div * r_procs;
	    
	    if (blklen > INT_MAX) {
	      fprintf(efptr, "[RANK %d] ERROR: (blklen > INT_MAX)", my_rank);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
	  }
	  else
	    blklen = objsize_div;
	}
	
/*      *************************************************************
*       For non collective read calls, set the file view. Then read file and 
*       load one of the circular buffers. 
*       *************************************************************/
	for(j=0; j < num_file_writes; j++){

	  if(!collective_flag){
	    if(set_view_all){
	      if(strided_flag == 0) set_view_all = FALSE;
	      
	      offset = ((MPI_Offset)(i+j) * other_offset) + file_offset;
	      
	      if( MPI_File_seek(rfh, offset, MPI_SEEK_SET) != MPI_SUCCESS){
		fprintf(efptr,"[RANK %d] ERROR: Unable to seek for read.\n",
			my_rank);
		fflush(efptr);
		MPI_Abort(MPI_COMM_WORLD, -1);
	      }
	    }
	    
	    if(MPI_File_iread(rfh, c_buf[icbuf], (int)blklen, MPI_DOUBLE, 
			      &read_req[icbuf * num_file_writes + j])
	       != MPI_SUCCESS) {
	      fprintf(efptr, "[RANK %d] ERROR: Problem reading from file; MPI_File_iread.", 
		      my_rank);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
	
/*      *******************************************************
*       MPIO Wait for reading
*       *******************************************************/
	    wait_start = MPI_Wtime() ;
#ifndef ROMIO
	    if(MPI_Wait( &read_req[icbuf * num_file_writes + j],
			 &read_stat[icbuf * num_file_writes + j]) != 
	       MPI_SUCCESS){
	      fprintf(efptr, "[RANK %d] ERROR: Problem with MPI_Wait.\n", 
		      my_rank);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
#else
	    if(MPIO_Wait( &read_req[icbuf * num_file_writes + j],
			  &read_stat[icbuf * num_file_writes + j]) != 
	       MPI_SUCCESS){
	      fprintf(efptr, "[RANK %d] ERROR: Problem with MPIO_Wait.\n", 
		      my_rank);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
#endif
	    *file_op_wait_time +=  MPI_Wtime() - wait_start ;

	    /*
	    fprintf(stderr,"RANK %d: Read obj %d of %d for i=%d, %d %d\n",
		    my_rank, j+1, num_file_writes, i+1, (int)c_buf[icbuf][0], 
		    (int)c_buf[icbuf][1]);
	    fflush(stderr);
	    */
	  }
	  else{
/*          *******************************************************
*           Perform (blocking) collective read
*           *******************************************************/
	    offset = ((MPI_Offset)(i + j) * other_offset) + file_offset;
	    wait_start = MPI_Wtime() ;
	    
	    if( MPI_File_read_at_all(rfh, offset, c_buf[icbuf], (int)blklen, 
				     MPI_DOUBLE, 
				     &read_stat[icbuf * num_file_writes + j]) 
		!= MPI_SUCCESS){
	      fprintf(efptr,"[RANK %d] ERROR: Problem reading from file; MPI_File_read_at_all.\n", 
		      my_rank);
	      fflush(efptr);
	      MPI_Abort(MPI_COMM_WORLD, -1);
	    }
	    *file_op_wait_time +=  MPI_Wtime() - wait_start ;
	  }
	  
/*        *******************************************************
*         If necessary, check the data that was just read. Record the amount 
*         of time it takes to check the data and, later, subtract this from 
*         the op and total timers.
*         *******************************************************/
	  if(check_data_ndx > 0){
	    wait_start = MPI_Wtime();
	    num_err = verify_buffer(my_rank, c_buf[icbuf], check_data_ndx, 
				    touch, pagesize, 
				    poffset_val + (MPI_Offset)(j * obj_size), 
				    ofptr, efptr);
	    if( num_err  > 0){
	      fprintf(efptr,"[RANK %d] ERROR: %lu Data elements in object %d does not match expected values.\n", my_rank, num_err, j+1);
	      rc = 2;
	      MPI_Abort(MPI_COMM_WORLD, 1);
	    }
	    check_buf_time += MPI_Wtime() - wait_start;
	  }
	} /* for(j=0; j < num_file_writes ... */
	
/*      ******************************************************************
*       If using aggregators, send data from one of the circular buffer
*       *******************************************************************/
	if(aggregate_flag){
	  wait_start = MPI_Wtime();
	  
	  for ( j = start_p ; j < end_p ; j++ ) {
	    index = (int)((j - start_p) * num_objs);
	    
	    for(k=0; k < num_objs; k++){
	      tmp_p = c_buf[icbuf] + (k + index) * objsize_div;
	      send_to = (int)(c_buf[icbuf][1 + (k + index) * objsize_div]);

	      fprintf(stderr,"RANK %d: send_to %d tmp_p = %lf %lf\n", 
		      my_rank, send_to, tmp_p[0], (tmp_p+1)[0]);
	      fflush(stderr);

	      if( MPI_Isend(tmp_p, (int)blklen, MPI_DOUBLE, send_to, k, 
			    MPI_COMM_WORLD, &send_req[k + index]) !=MPI_SUCCESS){
		fprintf(efptr, "[RANK %d] ERROR: Unable to send data object %d to processor %d.\n", my_rank, k+1, send_to);
		fflush(efptr);
		MPI_Abort(MPI_COMM_WORLD, -1);
	      }
	    }
	  }
	  
	  if( MPI_Waitall((end_p - start_p)*(int)num_objs, send_req,
			  send_stat) != MPI_SUCCESS) {
	    fprintf(efptr, "[RANK %d] ERROR: (MPI_Wait for Isend)", my_rank);
	    fflush(efptr);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	  }

	  *proc_send_wait_time += MPI_Wtime() - wait_start ;
	}
	
	cbuf_use[icbuf] = 1 ; /* this c_buf[] is in use. */
      }/* for ( i = 0 ; */
    } /* else */
  
    *total_op_time = MPI_Wtime() - tb - check_buf_time; 

/*  ****************************************************
*   Close the file
*   ****************************************************/
    tb = MPI_Wtime();
    
    if(io_processor ){
      if (MPI_File_close(&rfh) != MPI_SUCCESS){
	fprintf(efptr, "[RANK %d] ERROR: Unable to close file.", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
    }
    
    *file_close_wait_time =  MPI_Wtime() - tb;

/*  ****************************************************
*   Stop the master timer.
*   ****************************************************/
    *total_time = MPI_Wtime() - ta - check_buf_time;  

/***********************************************************************
* Before returning, free all allocated memeory.
***********************************************************************/
  free(send_req);
  free(recv_req);
  free(read_req);
  free(send_stat);
  free(recv_stat);
  free(read_stat);
  if (aggregate_flag)
    free(m_obj);
  if (io_processor) {
    for( i = 0; i < num_of_cbuf; i++)
      free(c_buf[i]);
    free(c_buf);
  }

  return rc;   
}
