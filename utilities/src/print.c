 /******************************************************************
* Copyright (c) 2003
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

/******************************************************************
* PROGRAM:   print
*
* PURPOSE:   Routines that deal with program output, 
*
* AUTHOR:    James Nunez 
* DATE:      February 12, 2001
* LAST MODIFIED: September 7, 2005 [jnunez]
* VERSION:   1.00.006
*
*      LOS ALAMOS NATIONAL LABORATORY
*      An Affirmative Action/Equal Opportunity Employer
*
******************************************************************/

#include "print.h"

#include <string.h>

/*******************************************
 *   ROUTINE: show_usage
 *   PURPOSE: Print input parameters for routine
 *   DATE:    February 19,2004
 *   LAST MODIFIED: September 7, 2005
 *******************************************/
void show_usage(int fullhelp){
  printf("mpi_io_test:\n");
  printf("Gather timing information for reading from and writing to\n"
	 "file using N processors to N files, one file, M processors to M\n"
	 "files, or M processors to one file.\n\n");
  if(fullhelp){
    printf("-type #             Type of data movement:\n"
	   "                     (1) N processors to N files\n"
	   "                     (2) N processors to 1 file\n"
	   "                     (3) N processors to M processors to M files\n"
	   "                     (4) N processors to M processors to 1 file\n"
	   "                     (5) N processors to M files\n");
    printf("-collective         Use collective read and write calls \n"
	   "                    (MPI_File_read/write_at_all) instead of \n"
	   "                    independent calls (MPI_File_iread/iwrite). \n"
	   "                    (Default: Use independent read/write calls)\n");
    printf("-num_io \"# # ... #\" Total number of I/O processors. If using the\n"
	   "                    -host flag, this is the number of I/O\n"
	   "                    processors on each host. For type 5, this is\n"
	   "                    the number of files to write to.\n"
	   "                    (Default: all N processors will do I/O)\n");
    printf("-nobj  #            Number of objects to write to file or to send\n"
	   "                    to each of the M I/O processors\n"
	   "                    (Default: 1)\n");
    printf("-strided #         All processors write in a strided, all \n"
	   "                   processors write to one region of the file and\n"
	   "                   then all seek to the next portion of the file,\n"
	   "                   or non-strided, all processors have a region \n"
	   "                   of the file that they exclusively write into,\n"
	   "                   pattern.\n"
	   "                        0. Non-strided (Default)\n"
	   "                        1. Strided - not implemented for use\n"
	   "                           with aggregation (-type 3 and 4)\n");
    printf(" -size  #           Number of bytes that each object will send\n"
	   "                       to the I/O processors or write to file.\n"
	   "                       Must be a multiple of sizeof(double).\n"
	   "                       Allows for (Linux) dd-like expressions; w 2, \n"
	   "                       b 512, KB 1000, K 1024, MB 1,000,000,  \n"
	   "                       M 1,048,576, GB 1,000,000,000, and \n"
	   "                       G 1,073,741,824.  A positive number followed by\n"
	   "                       one of the above letter will result in the two \n"
	   "                       multiplied, i.e. 2b => 1024.\n");
    printf("-csize #            Number of bytes in each I/O processors\n"
	   "                    circular buffers\n");
    printf("-target %%          Full path and file name to write and read test\n"
	   "                    data. The following in the file name or path\n"
	   "                    will resolve to:\n"
	   "                      %%r processor rank\n"
	   "                      %%h host name\n"
	   "                      %%p processor ID\n"
	   "                    (Default: ./test_file.out)\n");
    printf("-targets %%         File name of file containing one file name\n"
	   "                   for each processor. File should have one file\n"
	   "                   name on each line and an IO processor with IO\n"
	   "                   rank N will read the file name from line N+1.\n"
	   "                   The special characters for -target flag are \n"
	   "                   valid.\n"
	   "                   Test type 5 must use this flag and the first M\n"
	   "                   (-num_io) file names will be read from the \n"
	   "                   targets file.\n");
    printf("-deletefile        Causes the target file to be deleted at the \n"
	   "                    end of the program. \n"
	   "                    (Default: FALSE)\n");
    printf("-touch #           All data objects a processors sends/writes\n"
	   "                    will contain the (double) values processor \n"
	   "                    rank to size+processor rank. Some of these\n"
	   "                    values can be overwritten by the following: \n"
	   "                    the file offset will overwrite the rank \n"
	   "                    (first element):\n"
	   "                     (1) Never (default)\n"
	   "                     (2) Once per object\n"
	   "                     (3) Once per page per object\n");
    printf("-barrier            Call MPI_Barrier before each write\n"
	   "                    (Default: FALSE - do not call MPI_Barrier before each write)\n");
    printf("-sync              Sync the data before file close on writes\n"
	   "                    (Default: FALSE - do not sync data)\n");
    printf("-sleep #           Each processor will \"sleep\" for this number of\n"
	   "                    seconds between when the file is closed from \n"
	   "                    writing and opened to read.\n"
	   "                    (Default: 0 seconds)\n");
    printf("-chkdata #         When reading the data file, check that every \n"
	   "                    element equals the expected value, which \n"
	   "                    depends on the \"touch\" option. \n"
	   "                    Check the following data elements:\n"
	   "                    (0) None\n"
	   "                    (1) First data element of each object (Default)\n"
	   "                    (2) First two data elements of each object\n"
	   "                    (3) All data elements\n"
	   "                    Checking data will decrease the read bandwidth.\n");
    printf("-preallocate #      Preallocate this number of bytes before write\n");
    printf("-ahead #            Number of circular buffers for asynchronous I/O\n"
	   "                    (Default: 0)\n");
    printf("-host  \"%% %% ... %%\" List of host names to do I/O\n"
	   "                    (Default: all hosts)\n");
    printf("-lhosts             Print out all hosts running this program.\n");
    printf("-dio                (optional) Use direct read and write for all\n"
	   "                    read and write operations. This is a ROMIO\n"
	   "                    specific hint(\"direct_read\" and \"direct_write\")\n");
    printf("-hints %%s %%s ... %%s %%s Multiple MPI-IO key and value hint pairs. Any \n"
	   "                       MPI, ROMIO or file system specific hint can be \n"
	   "                       specified here.  \n");
    printf("-op %%              (Optional) Only read or write the target file.\n"
	   "                    1. \"read\" - only read the target files\n"
	   "                    2.\"write\" - only write out the target files\n");
    printf("-nofile             Only allocate memory and send message, do\n"
	   "                    not write the messages to file.\n");
    printf("-norsend            In normal operation, all processors send data\n"
	   "                    to an IO processor. Even IO processors send \n"
	   "                    data to themselves. This flag indicates that \n"
	   "                    IO processors do not send data to themselves.\n");
    printf("-output %%           File to write timing results, input parameters,\n"
	   "                    and MPI environment variables.\n"
	   "                    (Default: stdout)\n");
    printf("-errout %%           File to write all errors encountered while\n"
	   "                    running the program.\n"
	   "                    (Default: stderr)\n");
    printf("-verbose           Print all times for each processes\n"
	   "                           (Default: off)\n");
    printf("-help               (optional) Display the input options\n\n");
  }
}

/*******************************************
 *   ROUTINE: 
 *   PURPOSE: 
 *******************************************/
int print_parm(char *msg, int my_rank, int num_procs, MPI_Comm comm, 
	       FILE *output, FILE *error)
{
  int          i; 
  char        *new_msg;
  MPI_Status   recv_stat; 

  if(msg == NULL)
    return -1;

  if((new_msg = (char *) malloc((int)(strlen(msg) + 1)*sizeof(char))) == NULL){
    return -1;
  }

/***************************************************************************
* All processors send message to processor 0. Processor 0 will always print 
* its message and then print all other messages, with rank, that does not 
* match its own.
*****************************************************************************/
  if (my_rank != 0) {
    if(MPI_Send(msg, (int)(strlen(msg) + 1), MPI_CHAR, 0, my_rank, comm) != 
       MPI_SUCCESS) {
      fprintf(error, "[RANK: %d] ERROR: Unable to send message to processor 0.\n", my_rank);
      return -1;
    } 

  } 
  else {
    fprintf(output, "%s\n", msg);
    for (i = 1; i < num_procs; i++) {
      
      if(MPI_Recv(new_msg,  (int)(strlen(msg) + 1), MPI_CHAR, i, i, 
		  comm, &recv_stat) != MPI_SUCCESS) {
	fprintf(error, "%d: main (MPI_Recv) failed.", my_rank);
	return -1;
      }
      
      if (strcmp(msg, new_msg))
	fprintf(output, "RANK %d: %s.\n", i, new_msg);
    }
  }
  
  free(new_msg);
  return 1;
}

/*******************************************
 *   ROUTINE: print_input_environment
 *   PURPOSE: 
 *   LAST MODIFIED: August 20, 2005 [jnunez]
 *******************************************/
int print_input_environment(int my_rank, int test_type, int num_procs_world, 
			    int num_procs_io, int aggregate_flag, 
			    int *io_procs_list, 
			    int num_hosts, char **host_io_list, int ahead, 
			    size_t num_objs, size_t obj_size, size_t cbuf_size,
			    MPI_Offset prealloc_size, int check_data_ndx, 
			    int collective_flag, int data_pattern, 
			    int delete_flag, int write_file_flag, 
			    int io_proc_send_flag, 
                            int barrier_flag, int sync_flag, int targets_flag, 
			    char *tgtsfname, char *tfname, int touch, 
			    int list_host_flag, 
			    int init_flag, int myhost_num, char *my_host,
			    char *ofname, char *efname, FILE *ofptr, FILE *efptr){
  int i = 0;

  extern char **environ;

/******************************************************************
* Print input parameters
******************************************************************/
  if(my_rank == 0){
    fprintf(ofptr, "Input Parameters:\n");
    if(test_type == 1){
      fprintf(ofptr, "I/O Testing type N -> N: %d\n", test_type);
    }
    else if(test_type == 2){
      fprintf(ofptr, "I/O Testing type N -> 1: %d\n", test_type);
    }
    else if(test_type == 3){
      fprintf(ofptr, "I/O Testing type N -> M -> M: %d\n", test_type);
    }
    else if(test_type == 4){
      fprintf(ofptr, "I/O Testing type N -> M -> 1: %d\n", test_type);
    }
    else if(test_type == 5){
      fprintf(ofptr, "I/O Testing type N -> M: %d\n", test_type);
    }
    fprintf(ofptr, "Total number of processors (N): %d\n", num_procs_world);
    fprintf(ofptr, "Total number of I/O processors (M): %d\n", num_procs_io);
    if(aggregate_flag){
      fprintf(ofptr, "Number of I/O processors per host: %d", io_procs_list[0]);
      for(i=1; i < num_hosts; i++)  /* XXX Not right upper limit */
	fprintf(ofptr, " %d", io_procs_list[i]);
      fprintf(ofptr, "\n");
    }
    
    if(num_hosts > 0){
      fprintf(ofptr, "Host Name List: %s", host_io_list[0]);
      for(i=1; i < num_hosts; i++)
	fprintf(ofptr, " %s", host_io_list[i]);
      fprintf(ofptr, "\n");
    }
    else{
      fprintf(ofptr, "Host Name List: None\n");
    }
    
    fprintf(ofptr, "Number of circular buffers (-ahead): %d\n", ahead);
    fprintf(ofptr, "Number of objects (-nobj): %d\n", (int)num_objs);
    fprintf(ofptr, "Number of bytes in each object (-size): %lld\n", 
	    (long long)obj_size);
    fprintf(ofptr, "Circular buffer size (-csize): %lld\n", (long long)cbuf_size);
    fprintf(ofptr, "Preallocate file size (-preallocate): %lld\n", 
	    (long long)prealloc_size);
    fprintf(ofptr, "Check data read (-chkdata): %d\n", check_data_ndx);
    fprintf(ofptr, "Use collective read/write calls (-collective): %d\n", 
	    collective_flag);
    fprintf(ofptr, "Call MPI_Barrier before each write (-barrier): %d\n", barrier_flag);
    fprintf(ofptr, "Flush data before file close (-sync): %d\n", sync_flag);
    fprintf(ofptr, "Strided data layout (-strided): %d\n", data_pattern);
    fprintf(ofptr, "Delete data file when done reading (-deletefile): %d\n", 
	    delete_flag);
    fprintf(ofptr, "Write output data to file (-nofile): %d\n",
	    write_file_flag);
    fprintf(ofptr, "Aggregators send data to themselves (-norsend): %d\n",
	    io_proc_send_flag);
    fflush(ofptr);
    if(targets_flag)
      fprintf(ofptr, "File containing target files (-targets): %s\n",tgtsfname);
    else
      fprintf(ofptr, "Test data written to (-target): %s\n", tfname);
    fprintf(ofptr, "Touch object option (-touch): %d\n", touch);
    if(ofname == NULL)
      fprintf(ofptr, "Timing results written to (-output): stdout\n");
    else
      fprintf(ofptr, "Timing results written to (-output): %s\n", ofname);
    if(efname == NULL)
      fprintf(ofptr, "Errors and warnings written to (-errout): stderr\n");
    else
      fprintf(ofptr, "Errors and warnings written to (-errout): %s\n", efname);
    fflush(ofptr);
  }
 
/******************************************************************
* Print out MPI environment variables
******************************************************************/
  if (my_rank == 0) {
    fflush(ofptr);
    fprintf(ofptr, "\nEnvironment Variables (MPI):\n");
    for (i = 0; environ[i] != NULL; i++){
      if (strstr(environ[i], "MPI") != NULL) {
	fprintf(ofptr, "%s\n", environ[i]);
      }
    }
    fprintf(ofptr, "\n");
    fflush(ofptr);
  }
  
/******************************************************************
* If necessary, print all hosts
******************************************************************/
  if(list_host_flag){
    if(!init_flag)
      if( MPI_Get_processor_name(my_host, &myhost_num) != MPI_SUCCESS){
	if(my_rank == 0)
	  fprintf(stderr, "[RANK %d] ERROR: Unable to get processor name from MPI_Get_processor_name.\n", my_rank);
	MPI_Finalize();
	return -1;	      
      }
    
    if (my_rank == 0){
      fprintf(ofptr, "\nHosts List:\n");
      fprintf(ofptr, "RANK %d: %s\n", my_rank, my_host);
      fflush(ofptr);
      MPI_Barrier(MPI_COMM_WORLD);
    }      
    else{
      MPI_Barrier(MPI_COMM_WORLD);
      fprintf(ofptr, "RANK %d: %s\n", my_rank, my_host);
      fflush(ofptr);
    }
  }


  MPI_Barrier(MPI_COMM_WORLD);

  return 1;
}

/*******************************************
 *   ROUTINE: get_min_sum_max
 *   PURPOSE: Collect minimum, sum, and maximum of input value across all 
 *            processors. Only processor with rank 0 will have the value and 
 *            rank of the processor with the maximum, minimum, and sum
 *******************************************/
static int get_min_sum_max(int my_rank, double base_num, double *min, int *min_ndx,
		    double *sum, double *max, int *max_ndx, char *op, 
		    FILE *ofptr, FILE *efptr)
{
  struct{
    double dval;
    int rank;
  } in, out;
  
  in.dval = base_num;
  in.rank = my_rank;
  
  if(MPI_Reduce(&in, &out, 1, MPI_DOUBLE_INT,
		MPI_MAXLOC, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: Unable to find (reduce) maximum %s.\n", 
	    my_rank, op);
    MPI_Finalize();
    return -1;
  }
  
  *max = out.dval;
  *max_ndx = out.rank;
  MPI_Barrier(MPI_COMM_WORLD);

  if(MPI_Reduce(&base_num, sum, 1, MPI_DOUBLE,
		MPI_SUM, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: Unable to find (reduce) sum of the %s.\n",
	    my_rank, op);
    MPI_Finalize();
    return -1;
  }
  
  MPI_Barrier(MPI_COMM_WORLD);

  if(MPI_Reduce(&in, &out, 1, MPI_DOUBLE_INT,
		MPI_MINLOC, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
    fprintf(efptr, "[RANK %d] ERROR: Unable to find (reduce) minimum %s.\n", 
	    my_rank, op);
    MPI_Finalize();
    return -1;
  }
  
  *min = out.dval;
  *min_ndx = out.rank;
  
  return MPI_SUCCESS;
}

/*******************************************
 *   ROUTINE: collect_and_print_time
 *   PURPOSE: Collect and print minimum, average, and maximum of input 
 *            processor wait and file open and close times across all procs
 *   LAST MODIFIED: July 24, 2005 [jnunez]
 *******************************************/
int collect_and_print_time(int my_rank, int num_procs, int num_io_procs, 
			   size_t obj_size, size_t num_objs, 
			   double elapsed_time, double op_time,
			   double file_open_wait_time, 
			   double prealloc_wait_time, 
			   double file_close_wait_time, 
			   double barrier_wait_time, 
			   double file_sync_wait_time, 
			   double proc_send_wait_time, 
			   double proc_recv_wait_time, 
			   double file_write_wait_time, 
                           int barrier_flag, int sync_flag, 
			   int io_proc_send_flag, 
			   int aggregation, int verbose_flag, char *op, FILE *ofptr, FILE *efptr)
{
  double speed = 0.0; 
  double sum_time = 0.0, min_time = 0.0, max_time = 0.0;
  int min_ndx = 0, max_ndx = 0;
  
  if(!io_proc_send_flag) num_procs -= num_io_procs;
  speed = ((double)(num_procs * num_objs) * (obj_size))/(1024.0*1024.0);
  
/*******************************************************************
* Find and print minimum and maximum total elapsed time; file open, 
* write/read data, and file close time.
*******************************************************************/
  if( get_min_sum_max(my_rank, elapsed_time, &min_time, &min_ndx,
		      &sum_time, &max_time, &max_ndx, 
		      "total elapsed time", ofptr, efptr) != MPI_SUCCESS){
    fprintf(efptr, "[RANK %d] ERROR: Problem computing min, max, and sum of total elapsed %s time.\n", 
	    my_rank, op);
    MPI_Finalize();
    return -1;
  }
  
  if ( my_rank == 0 ) {
    fprintf(ofptr, "=== Effective Bandwidth (min [rank] avg max [rank]): %e [%d] %e %e [%d] Mbytes/s.\n", 
	    speed/max_time, max_ndx,(num_procs*speed)/sum_time,
	    speed/min_time, min_ndx);
    fprintf(ofptr, "=== Total Time (min [rank] avg max [rank]): %e [%d] %e %e [%d] sec.\n", 
	    min_time, min_ndx, sum_time/num_procs, max_time, max_ndx);
  }
  
  
  if(verbose_flag){
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
    
    fprintf(ofptr, "[%d] Effective Bandwidth = %e\n", my_rank, speed/elapsed_time);
    
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
  }  
  
/*******************************************************************
* Find and print minimum and maximum time to write the data to file; this 
* does not include file open and close times.
*******************************************************************/
  if( get_min_sum_max(my_rank, op_time, &min_time, &min_ndx,
		      &sum_time, &max_time, &max_ndx, 
		      "operation time", ofptr, efptr) != MPI_SUCCESS){
    fprintf(efptr, "[RANK %d] ERROR: Problem computing min, max, and sum of the %s time.\n", 
	    my_rank, op);
    MPI_Finalize();
    return -1;
  }
  
  if ( my_rank == 0 ) {
    fprintf(ofptr, "=== %s Bandwidth (min [rank] avg max [rank]): %e [%d] %e %e [%d] Mbytes/s.\n", 
	    op, speed/max_time, max_ndx,(num_procs*speed)/sum_time,
	    speed/min_time, min_ndx);
    fprintf(ofptr, "=== %s Time (min [rank] avg max [rank]): %e [%d] %e %e [%d] sec.\n", 
	    op, min_time, min_ndx, sum_time/num_procs, max_time, max_ndx);
  }
  
  if(verbose_flag){
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
    
    fprintf(ofptr, "[%d] %s Bandwidth = %e\n", my_rank, op, speed/op_time);
    
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
  }

/*******************************************************************
* Find and print minimum and maximum time any processor took to open the 
* output file
*******************************************************************/
  if( get_min_sum_max(my_rank, file_open_wait_time, &min_time, &min_ndx,
		      &sum_time, &max_time, &max_ndx,
		      "file open time", ofptr, efptr) != MPI_SUCCESS){
    fprintf(efptr, "[RANK %d] ERROR: Problem getting min, max, and sum of file open time.\n", 
	    my_rank);
    MPI_Finalize();
    return -1;
  }
  
  if ( my_rank == 0 ) {
    fprintf(ofptr, "=== MPI File Open Time (min [rank] avg max [rank]): %e [%d] %e %e [%d] sec.\n", 
	    min_time, min_ndx, sum_time/num_procs, max_time, max_ndx);
  }
  
  if(verbose_flag){
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
    
    fprintf(ofptr, "[%d] File Open Time = %e\n", my_rank, file_open_wait_time);
    
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
  }

/*******************************************************************
* Find minimum and maximum time any processor waited to read/write to file
*******************************************************************/
  if( get_min_sum_max(my_rank, file_write_wait_time, &min_time, 
		      &min_ndx, &sum_time, &max_time, &max_ndx, 
		      "processor operation wait time", ofptr, efptr) !=
      MPI_SUCCESS){
    fprintf(efptr, "[RANK %d] ERROR: Problem getting min, max, and sum of processor wait to write time.\n", 
	    my_rank);
    MPI_Finalize();
    return -1;
  }

  if ( my_rank == 0 ) {
    fprintf(ofptr, "=== MPI File %s Wait Time (min [rank] avg max [rank]): %e [%d] %e %e [%d]  sec.\n",
	    op, min_time, min_ndx, sum_time/num_procs, max_time, max_ndx) ;
  }
  
  if(verbose_flag){
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
    
    fprintf(ofptr, "[%d] File %s Wait Time = %e\n", my_rank, op, 
	    file_write_wait_time);
    
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
  }

/*******************************************************************
* If necessary, find and print minimum and maximum time any 
* processor took to preallocate storage space for the output file. 
*******************************************************************/
  if( get_min_sum_max(my_rank, prealloc_wait_time, &min_time, 
		      &min_ndx, &sum_time, &max_time, &max_ndx,
		      "preallocate wait time", ofptr, efptr) != MPI_SUCCESS){
      fprintf(efptr, "[RANK %d] ERROR: Problem getting min, max, and sum of preallocate wait time.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    if ( my_rank == 0 ) {
      fprintf(ofptr, "=== MPI File Preallocate Time (min [rank] avg max [rank]): %e [%d] %e %e [%d] sec.\n", 
	      min_time, min_ndx, sum_time/num_procs, max_time, max_ndx) ;
    }

    if(verbose_flag){
      fflush(ofptr);
      MPI_Barrier(MPI_COMM_WORLD);
      
      fprintf(ofptr, "[%d] File Preallocate Time = %e\n", my_rank, file_sync_wait_time);
      
      fflush(ofptr);
      MPI_Barrier(MPI_COMM_WORLD);
    }

/*******************************************************************
* If specified, find and print minimum and maximum total time time any 
* processor took at the barrier before each write.
*******************************************************************/
  if( barrier_flag )
  {
    if( get_min_sum_max(my_rank, barrier_wait_time, &min_time, 
			&min_ndx, &sum_time, &max_time, &max_ndx,
			"barrier wait time", ofptr, efptr) != MPI_SUCCESS){
      fprintf(efptr, "[RANK %d] ERROR: Problem getting min, max, and sum of barrier wait time.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    if ( my_rank == 0 ) {
      fprintf(ofptr, "=== MPI Barrier Wait Time (min [rank] avg max [rank]): %e [%d] %e %e [%d] sec.\n", 
	      min_time, min_ndx, sum_time/num_procs, max_time, max_ndx) ;
    }

    if(verbose_flag){
      fflush(ofptr);
      MPI_Barrier(MPI_COMM_WORLD);
      
      fprintf(ofptr, "[%d] MPI_Barrier Time before each write = %e\n", my_rank, barrier_wait_time);
      
      fflush(ofptr);
      MPI_Barrier(MPI_COMM_WORLD);
    }
  }


/*******************************************************************
* If necessary, find and print minimum and maximum time any 
* processor took to sync data to the output file. We don't sync the file for 
* reads.
*******************************************************************/
  if( sync_flag ){
    if( get_min_sum_max(my_rank, file_sync_wait_time, &min_time, 
			&min_ndx, &sum_time, &max_time, &max_ndx,
			"file sync wait time", ofptr, efptr) != MPI_SUCCESS){
      fprintf(efptr, "[RANK %d] ERROR: Problem getting min, max, and sum of file sync wait time.\n", my_rank);
      MPI_Finalize();
      return -1;
    }
    if ( my_rank == 0 ) {
      fprintf(ofptr, "=== MPI File Sync Time (min [rank] avg max [rank]): %e [%d] %e %e [%d] sec.\n", 
	      min_time, min_ndx, sum_time/num_procs, max_time, max_ndx) ;
    }

    if(verbose_flag){
      fflush(ofptr);
      MPI_Barrier(MPI_COMM_WORLD);
      
      fprintf(ofptr, "[%d] File Sync Time = %e\n", my_rank, file_sync_wait_time);
      
      fflush(ofptr);
      MPI_Barrier(MPI_COMM_WORLD);
    }

  }

/*******************************************************************
* Find and print minimum and maximum time any processor took to close the 
* output file
*******************************************************************/
  if( get_min_sum_max(my_rank, file_close_wait_time, &min_time,
		      &min_ndx, &sum_time, &max_time,
		      &max_ndx, "file close wait time", ofptr, efptr) != MPI_SUCCESS){
    fprintf(efptr, "[RANK %d] ERROR: Problem getting min, max, and sum of file close wait time.\n", 
	    my_rank);
    MPI_Finalize();
    return -1;
  }
  if ( my_rank == 0 ) {
    fprintf(ofptr, "=== MPI File Close Wait Time (min [rank]  avg max [rank]): %e [%d] %e %e [%d]  sec.\n", 
	    min_time, min_ndx, sum_time/num_procs, max_time, max_ndx) ;
  }
  
  if(verbose_flag){
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
    
    fprintf(ofptr, "[%d] File Close Time = %e\n", my_rank, file_close_wait_time);
    
    fflush(ofptr);
    MPI_Barrier(MPI_COMM_WORLD);
  }

/*******************************************************************
* If using aggregation, find and print the minimum, sum, and maximum time any 
* processor waited to send and receive data to another processor and wait to 
* write to file time.
*******************************************************************/
  if(aggregation){
    if( get_min_sum_max(my_rank, proc_send_wait_time, &min_time, 
			&min_ndx, &sum_time, &max_time, 
			&max_ndx, "processor send wait time", ofptr, efptr) != 
	MPI_SUCCESS){
      fprintf(efptr, "[RANK %d] ERROR: Problem getting min, max, and sum of processor send wait time.\n", 
	      my_rank);
      MPI_Finalize();
      return -1;
    }
    if ( my_rank == 0 ) {
      fprintf(ofptr, "=== MPI Processor send wait time (min [rank] avg max [rank]): %e [%d]  %e %e [%d]  sec.\n",
	      min_time, min_ndx, sum_time/num_procs, max_time, max_ndx) ;
    }
    
    if( get_min_sum_max(my_rank, proc_recv_wait_time, &min_time, 
			&min_ndx, &sum_time, &max_time,
			&max_ndx, "processor receive wait time", ofptr, efptr) !=
	MPI_SUCCESS){
      fprintf(efptr, "[RANK %d] ERROR: Problem getting min, max, and sum of processor receive wait time.\n", 
	      my_rank);
      MPI_Finalize();
      return -1;
    }
    if ( my_rank == 0 ) {
      fprintf(ofptr, "=== MPI Processor Receive Wait Time (min avg max): %e[%d]  %e %e[%d]  sec.\n",
	      min_time, min_ndx, sum_time/num_procs, max_time, max_ndx);
    }
    
    if(verbose_flag){
      fflush(ofptr);
      MPI_Barrier(MPI_COMM_WORLD);
      
      fprintf(ofptr, "[%d] Processor send wait time: %e sec.\n",
	      my_rank, proc_send_wait_time) ;
      
      fflush(ofptr);
      MPI_Barrier(MPI_COMM_WORLD);
    }
  }

/*******************************************************************
* Flush the print statements.
*******************************************************************/
  if ( my_rank == 0 ) {
    fprintf(ofptr, "=== Completed MPI-IO %s.\n", op);
    fflush(ofptr) ;
  }
  
  return MPI_SUCCESS;
}

