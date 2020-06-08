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
* PROGRAM:  mpi_io_test
*
* PURPOSE:   Gather timing information for reading from and writing to
*            a(n) file(s) using N processors to N files, one file, M 
*            processors to M files, or M processors to one file.
*
*
* PARAMETERS:
*            -type  #           Type of data movement:
*                                 (1) N processors to N files
*                                 (2) N processors to 1 file
*                                 (3) N processors to M processors to M files
*                                 (4) N processors to M processors to 1 file
*                                 (5) N processors to M files
*            -collective        Use collective read and write calls 
*                               (MPI_File_read/write_at_all) instead of 
*                               independent calls (MPI_File_iread/iwrite). 
*                               (Default: Use independent read/write calls)
*            -num_io "# # ... #" Total number of I/O processors. If using the
*                               -host flag, this is the number of I/O
*                               processors on each host. For type 5, this is 
*                               the number of files to write to.
*                               (Default: all N processors will do I/O)
*            -nobj  #           Number of objects to write to file or to send
*                               to each of the M I/O processors
*                               (Default: 1)
*            -strided #         All processors write in a strided, all 
*                               processors write to one region of the file and 
*                               then all seek to the next portion of the file, 
*                               or non-strided, all processors have a region 
*                               of the file that they exclusively write into, 
*                               pattern.
*                                   0. Non-strided (Default)
*                                   1. Strided - not implemented for use 
*                                      with aggregation (-type 3 and 4)
*            -size  #           Number of bytes that each object will send
*                               to the I/O processors or write to file.
*                               Must be a multiple of sizeof(double).
*                               Allows for (Linux) dd-like expressions; w 2, 
*                               b 512, KB 1000, K 1024, MB 1,000,000,  
*                               M 1,048,576, GB 1,000,000,000, and 
*                               G 1,073,741,824.  A positive number followed by
*                               one of the above letter will result in the two 
*                               multiplied, i.e. 2b => 1024.
*            -csize #           Number of bytes in each I/O processors
*                               circular buffers
*                               Must be a multiple of sizeof(double).
*            -target %          Full path and file name to write and read test
*                               data. The following in the file name or path
*                               will resolve to:
*                                  %r processor rank
*                                  %h host name
*                                  %p processor ID
*                               (Default: ./test_file.out)
*            -targets %         File name of file containing one file name for
*                               each processor. File should have one file name
*                               on each line and an IO processor with IO rank 
*                               N will read the file name from line N+1. The 
*                               special characters for -target flag are valid.
*                               Test type 5 must use this flag and the first M
*                               (-num_io) file names will be read from the 
*                               targets file.
*            -deletefile        Delete the target files at the end of the 
*                               program. 
*                               (Default: FALSE)
*            -barrier           Call MPI_Barrier before every write 
*                               call. (Default: FALSE)
*            -touch #           All data objects sent/written by a processor
*                               will contain the sequence of (double) values 
*                               processor rank to (size input variable +processor 
*                               rank). The first value can be overwritten 
*                               with the following flags: 
*                                (1) Never (default)
*                                (2) Once per object
*                                (3) Once per page per object
*            -sync              Sync the data before file close on writes
*                               (Default: FALSE - do not sync data)
*            -sleep #           Each processor will "sleep" for this number of
*                               seconds between when the file is closed from 
*                               writing and opened to read.
*                               (Default: 0 seconds)
*            -chkdata #         When reading the data file, check that every 
*                               element equals the expected value, which 
*                               depends on the "touch" option. 
*                               Check the following data elements:
*                               (0) None
*                               (1) First data element of each object (Default)
*                               (2) First two data elements of each object
*                               (3) All data elements
*                               Checking data will decrease the read bandwidth.
*            -preallocate #     Preallocate this number of bytes before write.
*            -ahead #           Number of circular buffers for asynchronous I/O
*                               (Default: 0)
*            -host  "% % ... %" List of host names to do I/O
*                               (Default: all hosts)
*            -lhosts            Print out all hosts running this program.
*            -dio               Use direct read and direct write for all
*                               read and write operations. This flag will 
*                               evoke the ROMIO specific hints "direct_read"
*                                and "direct_write"
*            -hints %s %s ... %s %s Multiple MPI-IO key and value hint pairs. Any 
*                               MPI, ROMIO or file system specific hint can be 
*                               specified here.  
*            -op %              Read or write the target file. If this flag is
*                               not used, both write then read will be 
*                               performed.
*                                "read" - only read the target files
*                                "write" - only write out the target files
*            -nofile            Only allocate memory and send message, do 
*                               not write the messages to file.
*            -norsend           In normal aggregation operation, all 
*                               processors send data to an IO processor. Even 
*                               IO processors send data to themselves. This 
*                               flag indicates that IO processors do not send 
*                               data to themselves.
*            -output %          File to write timing results, input parameters,
*                               and MPI environment variables.
*                               (Default: stdout)
*            -errout %          File to write all errors encountered while
*                               running the program.
*                               (Default: stderr)
*            -verbose           Print all times for each processes
*                               (Default: off)
*            -help              Display the input options
*
* RETURN:    0 on success, -1 otherwise
* AUTHOR:    James Nunez 
* DATE:      February 12, 2001
* LAST MODIFIED: December 15, 2006 [jnunez]
* VERSION:   1.00.009
*
******************************************************************/

#include "mpi_io_test.h"

int main( int argc, char *argv[] )
{
  MPI_Comm comm_io = MPI_COMM_NULL; /* Communicator of I/O processors       */
  MPI_Comm noncomm_io = MPI_COMM_NULL; /* Communicator of non-I/O processors*/
  MPI_Comm comm_file = MPI_COMM_NULL;/* Communicator to open files with    */
  
  MPI_Offset  offset;      /* offset for set view                      */
  MPI_Offset file_offset;  /* offset in comm_io for set view           */
  MPI_Offset poffset_val = 0.0;/* constant obj offset for each processor   */
  MPI_Offset other_offset =0;

  MPI_Info info = MPI_INFO_NULL; /* Info struct to load inputs         */
  int info_allocated_flag = FALSE;/* Flag denoting MPI_info struct allocated */

  FILE *ofptr = NULL;      /* Set-up and output file pointer         */
  FILE *efptr = NULL;      /* Error log file pointer                 */
  
  int my_rank = -1;        /* my rank in MPI_COMM_WORLD              */
  int my_rank_io = -1;     /* my rank in comm_io                     */
  
  int *recv_procs = NULL;  /* Array of proc ranks to receive data    */
  int num_procs_world = 0; /* number of compute or total processors  */
  int num_procs_io = 0;    /* total number of I/O processors         */
  int *io_procs_list = NULL; /* Number of I/O processors per host    */
  int f_procs = 0;         /* Number of processors to fill one buffer*/
  
  int num_of_cbuf;         /* number of circular buffers              */
  int num_of_cjob;         /* number of circular buffers  jobs        */
  int min_all_proc_cjobs = 0;

  int icbuf ;              /* circular buffer index                   */
  int *cbuf_use = NULL;    /* flag for c_buf[] is in use or not       */
  size_t cbuf_size;        /* Size of each data object n in bytes     */
  
  int  blksize;            /* blksize = number of elements in buffer  */
  int  pagesize;           /* page size for direct IO                 */
  int  amode_read;         /* MPI file access mode bits to read file  */
  int  amode_write;        /* MPI file access mode bits to write file */
  
  size_t obj_size = 0;     /* Number of bytes in each of the data objects*/
  size_t num_objs = 1;     /* Number of data objects per processor     */
  int direct_io = FALSE;   /* on/off for the direct IO                 */
  int ahead = 0;           /* Number of the circular buffers for async. IO */
  int test_type = 0;       /* Type of I/O test to be performed         */
  
  char *tfname = NULL;     /* File containing test data                */
  char *tgtsfname = NULL;  /* File containing target file names         */
  char *ofname = NULL;     /* File containing timing results           */
  char *efname = NULL;     /* File containing errors                   */
  int num_files = 0;       /* Number of files to write to              */
  int num_file_writes = 0; /* Number of times we write to a file       */

  int check_data_ndx = 1;  /* Number of data elements to check         */

  int init_flag = FALSE;   /* initial flag to skip wait for writing    */
  int write_only_flag = FALSE;/* Write target file only                */
  int read_only_flag = FALSE; /* Read target file only                 */
  int list_host_flag = FALSE; /* Flag denoting print all hosts         */
  int aggregate_flag = FALSE;/* Flag denoting use of aggregate procs   */
  int barrier_flag = FALSE; /* MPI_Barrier call before each write      */
  int sync_flag = FALSE;   /* Sync the data to file before close       */

  int sleep_seconds = 0;   /* Amount of time to slepp between write & read */
  int delete_flag = FALSE; /* Flag denoting delete the target file     */
  int collective_flag = FALSE; /* Use collective read/write calls */
  int targets_flag = FALSE; /* Flag denoting a targets file exists     */
  int write_file_flag = TRUE;/* Flag denoting messages written to file */
  int io_proc_send_flag = TRUE;/* Flag denoting IO procs send data      */
  int verbose_flag = FALSE; /* Print times for all processes           */

  int io_processor = FALSE;/* Flag denoting processor performs I/O     */
  int num_hosts = 0;       /* Number of hosts to do I/O                */
  char **host_io_list = NULL; /* Array of user supplied host names     */
  char my_host[MPI_MAX_PROCESSOR_NAME];
  int myhost_num = 0;      /* Index of my host in hostname_list        */
  
  int *send_counts = NULL; /* Length of characters to receive  */
  int num_receive = -2;    /* Number of processor to recv data from */
  int send_to = 0;

  int set_view_all = FALSE;/* Flag denoting I/O procs need to set file view */
  int strided_flag = 0;         /* By default, write non-strided            */
  int touch = 1;           /* Touch the object before sending          */
  int index = 1;
  int nkeys;               /* Number of MPI file hints */
  char **key=NULL, **value=NULL; /* Array of strings to hold hints */
  
  double total_time = 0.0;          
  double total_op_time = 0.0;       /* Time for write/read from open to close*/
  double proc_send_wait_time = 0.0; /* Time for send commands to complete   */
  double proc_recv_wait_time = 0.0; /* Time waiting to receive messages     */
  double file_open_wait_time = 0.0; /* Time to open file                    */
  double file_close_wait_time = 0.0;/* Time it takes to close the data file */
  double barrier_wait_time = 0.0;        /* Time spend at MPI_Barrier before each write*/
  double file_sync_wait_time = 0.0; /* Time it takes to sync the data file  */
  double file_op_wait_time = 0.0;   /* Time to write/read all data to file  */

  MPI_Offset prealloc_size = -1;    /* Number bytes to preallocate for file */
  double prealloc_wait_time = 0.0;  /* Time it takes to preallocate file    */
  int prealloc_flag = FALSE; /* Flag set when the preallocate option is set */

  int i, j, k;          /* For loop indices                           */
  int rc = 0;
  
  char temp_value[256];    /* Varilables to get current time         */
  struct tm *ptr;
  time_t lt;

/******************************************************************
* Initialize MPI and get processor rank and number of processors.
******************************************************************/
  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to initialize MPI (MPI_Init).\n");
    return -1;
  }
  
  if (MPI_Comm_rank(MPI_COMM_WORLD, &my_rank) != MPI_SUCCESS) {
    fprintf(stderr, "ERROR: Problem getting processor rank (MPI_Comm_rank).\n");
    return -1;
  }
  
  if ( MPI_Comm_size(MPI_COMM_WORLD, &num_procs_world) != MPI_SUCCESS) {
    fprintf(stderr, "[RANK %d] ERROR: Problem getting number of processors in MPI_COMM_WORLD.\n", my_rank);
    return -1;
  }

/******************************************************************
* Print input parameters
******************************************************************/
  if(my_rank == 0){
    lt = time(NULL);
    ptr = localtime(&lt);
    fprintf(stdout,"MPI-IO TEST v1.00.009: %s\n", asctime(ptr));
  }

/******************************************************************
* Get input parameters
******************************************************************/
  if (argc > 2){
    if( !parse_command_line(my_rank, argc, argv, &test_type, &num_objs, 
			    &obj_size, &cbuf_size, &prealloc_size, &prealloc_flag, 
			    &ahead, &touch, 
			    &strided_flag, &list_host_flag, &num_hosts,
			    my_host, &write_file_flag, &barrier_flag, &sync_flag, 
			    &sleep_seconds, &delete_flag, &ofname,
			    &efname, &tfname, &host_io_list, &io_procs_list, 
			    &init_flag, &num_procs_io, &aggregate_flag, &info, 
			    &info_allocated_flag, &direct_io, &targets_flag, 
			    &tgtsfname, &write_only_flag, &read_only_flag, 
			    &check_data_ndx, &collective_flag, 
			    &io_proc_send_flag, &verbose_flag)){
      fprintf(stderr, "ERROR [RANK %d]: Problem parsing the command line.\n",
	      my_rank);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
  }
  else{
    if(my_rank == 0) {
      fprintf(stderr, "[RANK %d] ERROR: Too few command-line args.\n", my_rank);
      show_usage(TRUE);
    }
    MPI_Finalize();
    return -1;
  }
  
/******************************************************************
* Check input parameters
******************************************************************/
  if (num_objs <= 0) {
    if(my_rank == 0){
      fprintf(stderr, "[RANK %d] ERROR: Illegal number of objects(%d) to write to/read from file.\n",
	      my_rank, (int)num_objs);
      show_usage(FALSE);
    }
    MPI_Finalize();
    return -1;
  }

  if ( obj_size%sizeof(double)!= 0) {
    if(my_rank == 0){
      fprintf(stderr, "[RANK %d] ERROR: Object size (%ld) must be a multiple of %d (sizeof(double)).\n",
	      my_rank, obj_size, sizeof(double));
      show_usage(FALSE);
    }
    MPI_Finalize();
    return -1;
  }
  
  if( (strided_flag > 1) || (strided_flag < 0) ){
    if(my_rank == 0){
      fprintf(stderr, "[RANK %d] ERROR: Illegal strided option (%d); must be 0 (non-strided) or 1 (Strided).\n", my_rank, strided_flag);
      show_usage(FALSE);
    }
    MPI_Finalize();
    return -1;
  }
  /*  XXXXXXXXXXX
  fprintf(stderr,"obj_size = %d INT_MAX = %d sizeof(size_t) = %d\n",
	  obj_size, INT_MAX, sizeof(size_t));
  */

  if( (obj_size <= 1) || (obj_size > UINT_MAX) ){
    if(my_rank == 0){
      fprintf(stderr, "[RANK %d] ERROR: Illegal object size (%ld); must be greater than one and less than UINT_MAX (%lu).\n", my_rank, (int)obj_size, UINT_MAX);
      show_usage(FALSE);
    }
    MPI_Finalize();
    return -1;
  }
  
  if((test_type == 1) || (test_type == 2)){
    cbuf_size = obj_size;
    

    if(aggregate_flag && (num_procs_io != num_procs_world)){
      if(my_rank == 0){
	fprintf(stderr, "[RANK %d] ERROR: Test N->(N or 1), but number of I/O processors specified as %d.\n", my_rank, num_procs_io);
	show_usage(FALSE);
      }
      MPI_Finalize();
      return -1;
    }
  
    if(!write_file_flag || !io_proc_send_flag){
      if(my_rank == 0){
	fprintf(stderr, "[RANK %d] ERROR: Test N->(N or 1), the -nofile and -norsend options are not allowed.\n", my_rank);
	show_usage(FALSE);
      }
      MPI_Finalize();
      return -1;
    }

    aggregate_flag = FALSE;
    num_procs_io = num_procs_world;
   
  }
  else if((test_type == 3) || (test_type == 4)){
    if(!aggregate_flag){
      if(my_rank == 0){
	fprintf(stderr, "[RANK %d] ERROR: Test N->M->(M or 1), but number of I/O processors (M) not specified.\n", my_rank);
	show_usage(FALSE);
      }
      MPI_Finalize();
      return -1;
    }

    if( (cbuf_size <= 0) || (cbuf_size > INT_MAX) ){
      if(my_rank == 0){
	fprintf(stderr, "[RANK %d] ERROR: Circular buffer size must be greater than zero and less than INT_MAX (%d).\n", my_rank, INT_MAX);
	show_usage(FALSE);
      }
      MPI_Finalize();
      return -1;
    }
    
    if ( cbuf_size%(obj_size*num_objs) != 0 ) {
      if(my_rank == 0)
	fprintf(stderr, "[RANK %d] ERROR: A multiple of objects (-size * -nobj = %d) must fit into one circular buffer of size %d.\n", my_rank, (int)(obj_size*num_objs), 
		(int)cbuf_size);
      MPI_Finalize();
      return -1;
    }
    
    if(strided_flag == 1){
      if(my_rank == 0){
	fprintf(stderr, "[RANK %d] ERROR: strided writes are not implemented for aggregation yet.\n", my_rank);
	show_usage(FALSE);
      }
      MPI_Finalize();
      return -1;
    }

  }
  else if(test_type == 5){
    aggregate_flag = FALSE;

    if( (num_procs_io <= 0) || (num_procs_io> num_procs_world)){
      if(my_rank == 0){
	fprintf(stderr, "[RANK %d] ERROR: Invalid number of files to write to (%d) (-num_io option) for the N processor to M files (type 5) job.\n", my_rank, num_procs_io);
	show_usage(FALSE);
      }
      MPI_Finalize();
      return -1;
    }
  }
  else{
    if(my_rank == 0){
      fprintf(stderr, "[RANK %d] ERROR: Unknown I/O test number %d. Valid tests values are 1 - 5.\n", my_rank, test_type);
      show_usage(FALSE);
    }
    MPI_Finalize();
    return -1;
  }
  
  if((num_procs_io <= 0) || (num_procs_io > num_procs_world)){
    if(my_rank == 0){
      fprintf(stderr, "[RANK %d] ERROR: Illegal number of I/O processors specified (%d); must be between 0 and %d.\n", 
	      my_rank, num_procs_io, num_procs_world);
      show_usage(FALSE);
    }
    MPI_Finalize();
    return -1;
  }
  
  if( direct_io ){
    pagesize = getpagesize();
    
    if(aggregate_flag){
      if( ( cbuf_size % pagesize) != 0 ){
	if(my_rank == 0)
	  fprintf(stderr, "[RANK %d] ERROR: For direct I/O the circular buffer size (-csize) must be a multiple of the system page size %d.\n",  
		  my_rank, pagesize);
	MPI_Finalize();
	return -1;
      }
    }
    else{
      if( ( (num_objs * obj_size) % pagesize) != 0 ){
	if(my_rank == 0)
	  fprintf(stderr, "[RANK %d] ERROR: For direct I/O the number of objects * buffer size ( %d * %d) must be a multiple of the system page size %d.\n",  
		  my_rank, (int)num_objs, (int)obj_size, pagesize);
	MPI_Finalize();
	return -1;
      }
    }
  }

  if( (touch < 1) || (touch > 3)){
    if(my_rank == 0)
      fprintf(stderr, "[RANK %d] ERROR: Invalid touch option (%d).\n",  
	      my_rank, touch);
    MPI_Finalize();
    return -1;
  }
 
/********************************************************************
* Check if the user has supplied file names for the test data,
* output timing data, and the error file. If no target file is given, use 
* the default file name.
* The variable targets_flag will now hold the line number in the targets 
* file to read.
********************************************************************/
  if(targets_flag){
    if(tfname != NULL){
      if(my_rank ==0)
	fprintf(stderr, "[RANK %d] ERROR: Both -target and -targets flags were supplied.Please specify only one target file name source.\n", my_rank);
      MPI_Finalize();
      return -1;
    }

    if(test_type == 5) targets_flag = my_rank%num_procs_io + 1;
    else               targets_flag = my_rank + 1;

    if( (test_type == 2) || (test_type == 4)){
      if(my_rank ==0)
	fprintf(stderr, "[RANK %d] ERROR: Specifying a targets file can only be used with test types 1, 3, and 5; not %d.\n", my_rank, test_type);
      MPI_Finalize();
      return -1;
    }
  }
  else{
    if(test_type == 5){
      if(my_rank ==0)
	fprintf(stderr, "[RANK %d] ERROR: Specifying a targets file must be used with test type 5. Please specify a targets file.\n", my_rank);
      MPI_Finalize();
      return -1;
    }

    if(tfname == NULL){
      if ((tfname = (char *)malloc((size_t)16)) == NULL) {
	if(my_rank ==0)
	  fprintf(stderr, "[RANK %d] ERROR: Unable to allocate memory for the target file name.\n", my_rank);
	MPI_Finalize();
	return -1;
      }
      if(write_file_flag)
	sprintf(tfname, "%s", "./test_file.out") ;
      else
	sprintf(tfname, "%s", " ") ;
    }
  }

  if(ofname == NULL){
    ofptr = stdout;
  }
  else{
    /* Currently only processor with rank 0 must write to the output file */
    if(my_rank == 0)
      if ((ofptr = fopen(ofname, "w")) == NULL){
	fprintf(stderr, "[RANK %d] ERROR: Unable to open output information file %s.\n", my_rank , ofname);
	MPI_Abort(MPI_COMM_WORLD, -1);
	return -1;
      }
  }

  if(efname == NULL){
    efptr = stderr;
  }
  else{
    /* Everyone needs to write to the error file, ....
	if( MPI_File_open(MPI_COMM_WORLD, ofname, amode_write??, NULL_INFO, &wfh)
	    != MPI_SUCCESS) {
    */
    if ((efptr = fopen(efname, "w")) == NULL){
      if(my_rank == 0)
	fprintf(stderr, "[RANK %d] ERROR: Unable to open output error log file %s.\n", my_rank, efname);
      MPI_Finalize();
      return -1;
    }
  }
 
/******************************************************************
* Print input parameters and MPI environment variables
******************************************************************/
  if(!print_input_environment(my_rank, test_type, num_procs_world, 
			      num_procs_io, aggregate_flag, io_procs_list, 
			      num_hosts, host_io_list, ahead, num_objs,  
			      obj_size, cbuf_size, prealloc_size, check_data_ndx, 
			      collective_flag, strided_flag, delete_flag, 
			      write_file_flag, io_proc_send_flag,  
			      barrier_flag, sync_flag,
			      targets_flag, tgtsfname, tfname, touch, 
			      list_host_flag, init_flag, myhost_num, my_host,
			      ofname, efname, ofptr, efptr)){
    if(my_rank == 0)
      fprintf(efptr, "[RANK %d] ERROR: Problem detected printing input and envirnoment variables.\n",
	      my_rank);
    MPI_Finalize();
    return -1;
  }

  MPI_Barrier(MPI_COMM_WORLD);

/*******************************************************************
* If using processors to aggregate data, compute destination and source
* processors for sends and receives and create a new communicator
*******************************************************************/
  if(aggregate_flag){
    if( !compute_aggregators(my_rank, num_procs_world, num_procs_io,
			     io_procs_list, num_hosts, host_io_list,
			     test_type, num_objs, obj_size, 
			     cbuf_size, init_flag, io_proc_send_flag,
			     my_host, &myhost_num, &f_procs,
			     &num_of_cjob, &num_file_writes,
			     &io_processor, &my_rank_io, &send_to,
			     &num_receive, &recv_procs, &poffset_val,
			     &comm_io, &noncomm_io, ofptr, efptr)){
      if(my_rank == 0)
	fprintf(efptr, "[RANK %d] ERROR: Problem Detected in computing the I/O processors.\n", my_rank);
      MPI_Finalize();
      return -1;	      
    }
    
#ifdef DEBUG_IO
    
    if(io_processor){
      fprintf(stderr,"RANK %d: Aggregator. Receive data from %d procs (send to proc %d).\n",
	      my_rank, num_receive, send_to);
      
      for(i=0; i< num_receive; i++)
	fprintf(stderr, "RANK %d: Receive[%d] = %d.\n", my_rank, i, 
		recv_procs[i]);
      fflush(stderr);
    }
    else{
      fprintf(stderr,"RANK %d: NonAggregator. Send data to rank %d.\n",
	      my_rank, send_to);
      fflush(stderr);
    }
#endif

  }
  else{

/*  *********************************************************************
*   Else, all processors perform I/O 
*   *********************************************************************/
    num_file_writes = (int)num_objs;
    num_receive = 1;
    cbuf_size = obj_size;
    io_processor = TRUE;
    comm_io = MPI_COMM_WORLD;
    my_rank_io = my_rank;
    num_of_cjob = 1;
    
    if(test_type == 5){
      /* Round robin processors to write to the M files */
      nkeys = my_rank % num_procs_io;
      MPI_Comm_split(MPI_COMM_WORLD, nkeys, my_rank, &comm_io);
    }
    
    /************
     * poffset_val is divided by sizeof(double) because values written to 
     * the output file are doubles
     ****************************/
    if(test_type == 1) poffset_val = (MPI_Offset)1.0;
    else               poffset_val = ((MPI_Offset)(my_rank_io) * 
				      (MPI_Offset)(obj_size/sizeof(double)) * 
				      (MPI_Offset)(num_objs)) + (MPI_Offset)(1.0);
  }
  
  /* 
  if (MPI_Comm_rank(comm_io, &my_rank_io) != MPI_SUCCESS) {
    fprintf(stderr, "ERROR: Problem getting processor rank (MPI_Comm_rank) for the comm_io Communicator.\n");
    return -1;
  }

  if ( MPI_Comm_size(MPI_COMM_WORLD, &nkeys) != MPI_SUCCESS) {
    fprintf(stderr, "[RANK %d] ERROR: Problem getting number of processors in MPI_COMM_WORLD.\n", my_rank);
    return -1;
  }
  */

/********************************************************************
* If necessary get the pagesize and compute the number of times the object 
* needs to be touched. The "touch" variable will be modified to the number of 
* times we need to write the offset in the object.
********************************************************************/
  if(touch < 3){
    touch--;
  }
  else if(touch == 3){
    pagesize = getpagesize();
    touch = (int)( (obj_size/sizeof(double))/pagesize );
    if( (obj_size/sizeof(double))%pagesize != 0) touch += 1;
  }

#ifdef DEBUG_IO
  fprintf(stderr,"Rank %d: MAIN Pagesize = %d number of times to touch object %d starting offset %lf\n", 
	  my_rank, pagesize, touch, (float)poffset_val);
#endif


/****************************************************************
* Compute the number of circular buffers to use and allocate memory for
* a circular buffer "in use" array
****************************************************************/
  num_of_cbuf = ahead + 1 ;
  
  if( (cbuf_use = (int *)malloc(num_of_cbuf * sizeof(int)) ) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for circular buffer status array.", my_rank);
    MPI_Finalize();
    return -1;
  }
  for ( i = 0 ; i < num_of_cbuf ; i++ ) {
    cbuf_use[i] = 0 ;
  }
  
  if ( num_of_cjob < num_of_cbuf ) {
    fprintf(efptr,"WARNING: Number of circular buffers (-ahead + 1) %d is larger than necessary.\n", num_of_cbuf);
  }
  
/**********************************************************************
* Set constants based on the test type. Tests where several processors write 
* to one file need the additional knowledge of where to start writing in the 
* file.
**********************************************************************/
  if((test_type == 1) || (test_type == 3)){
    comm_file = MPI_COMM_SELF;
    amode_write = MPI_MODE_WRONLY | MPI_MODE_CREATE | MPI_MODE_UNIQUE_OPEN;
    amode_read = MPI_MODE_RDONLY | MPI_MODE_UNIQUE_OPEN;
    num_files = num_procs_io;

    file_offset = (MPI_Offset)0;
    set_view_all = FALSE;
    other_offset = (MPI_Offset)(cbuf_size);
  }
  else{
    comm_file = comm_io;
    amode_write = MPI_MODE_WRONLY | MPI_MODE_CREATE;
    amode_read = MPI_MODE_RDONLY;
    if(test_type == 5)
      num_files = num_procs_io;
    else
      num_files = 1;
    
    if(io_processor){
      if(send_counts == NULL)
	if( (send_counts = (int *) malloc( num_procs_world * sizeof(int))) ==
	    NULL){
	  fprintf(efptr, "[RANK %d] ERROR: Unable to allocate memory for receive buffer counts array.\n", my_rank);
	  fflush(efptr);
	  MPI_Abort(MPI_COMM_WORLD, -1);
	}
      
      if(MPI_Allgather(&num_receive, 1, MPI_INT, send_counts, 1,
		       MPI_INT, comm_io) != MPI_SUCCESS) {
	fprintf(efptr, "[RANK %d] ERROR: Unable to gather number of processors to send data to all I/O processors.\n", my_rank);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }

      file_offset = (MPI_Offset)0;
      for(j=0; j < my_rank_io; j++)
	file_offset += (MPI_Offset)send_counts[j];
      
      set_view_all = TRUE;
      if(strided_flag == 0){
	file_offset *= (MPI_Offset)(num_objs) * (MPI_Offset)(obj_size);
	other_offset = (MPI_Offset)(cbuf_size);
      }
      else{
	file_offset *= (MPI_Offset)(obj_size);
	other_offset = (MPI_Offset)(num_procs_world) * (MPI_Offset)(cbuf_size);
      }
      
#ifdef DEBUG_IO
      fprintf(stderr,"RANK %d: MAIN set_view_all %d file offset %lu other_offset %d\n",
	      my_rank, set_view_all, (unsigned long)file_offset, (int)other_offset);
	fflush(stderr);
#endif
    }
    
  }

/**********************************************************************
* If necessary, all IO processors, open the targets file and load the target 
* file name.
**********************************************************************/
  if (io_processor) {
    if(targets_flag)
      if(!get_file_name(tgtsfname, my_rank, targets_flag, MPI_COMM_WORLD,
			&tfname, ofptr, efptr)){
	fprintf(efptr, "[RANK %d] ERROR: Problem reading targets file %s.\n", 
		my_rank, tgtsfname);
	fflush(efptr);
	MPI_Abort(MPI_COMM_WORLD, -1);
      }
  }
  
/****************************************************************
* If using aggregators, each I/O processor could have a different number of 
* processors sending data to it. Find the least number of processors sending
* data to an I/O processor.
****************************************************************/
  if(aggregate_flag && io_processor){
    if( !find_min_jobs(my_rank, num_procs_io, num_of_cjob, 
		       &min_all_proc_cjobs, comm_io, ofptr, efptr)){
      fprintf(efptr,"[RANK %d] ERROR: Unable to find maximum number of messages to send.\n", my_rank);
      MPI_Finalize();
      return -1;
    }

#ifdef DEBUG_IO
    fprintf(stderr,"RANK %d: Minimum number of num_of_cjob (%d) %d.\n",
	    my_rank, num_of_cjob, min_all_proc_cjobs);
	    fflush(stderr);
#endif

  }


/*************************************************************
* First phase: Write all data objects to the checkpoint file(s).
* If we are to write out a file, print information to output
*******************************************************************/
  blksize = (int)(obj_size/sizeof(double));
  
  if(!read_only_flag){
    if (my_rank == 0) {
      fprintf(ofptr, "=== MPI write [%d->%d->%d %s] Starting ...\n",
	      num_procs_world, num_procs_io, num_files, tfname );
      fflush(ofptr);
    }
    
    if (MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(efptr,"[RANK %d] ERROR: Problem with initial write MPI_Barrier.\n",
	      my_rank);
      MPI_Finalize();
      return -1;
    }
  
/***********************************************************************
* Call the master write routine, which will time the file open, writing of the
* data, filesync (if requested) and file close.
***********************************************************************/
    if(!MPIIO_master_write(my_rank, my_rank_io, io_processor, write_file_flag, comm_file, 
			   tfname, amode_write, info, &nkeys, &key, 
			   &value, blksize, send_to, num_objs, num_receive,
			   num_of_cbuf, num_file_writes, obj_size, touch, 
			   pagesize, prealloc_size, file_offset, poffset_val, 
			   other_offset, num_of_cjob, 
			   f_procs, cbuf_size, 
			   &file_open_wait_time, &prealloc_wait_time,
			   &file_op_wait_time,
			   &proc_recv_wait_time, 
			   &proc_send_wait_time, &total_op_time, 
			   &total_time, &barrier_wait_time, &file_sync_wait_time, 
			   &file_close_wait_time,
			   aggregate_flag, io_proc_send_flag, 
			   info_allocated_flag, init_flag, 
			   collective_flag, set_view_all, 
			   strided_flag, cbuf_use, barrier_flag, sync_flag, 
			   ofptr, efptr)){
      fprintf(efptr, "[RANK %d] ERROR: Problem in MPIIO_master_write routine.\n", 
	      my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

    if (MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(efptr, "[RANK %d] ERROR: Problem in MPI_Barrier after writing data.\n", 
	      my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
    
/*******************************************************************
* Collect and report timing results
*******************************************************************/
    if(collect_and_print_time(my_rank, num_procs_world, num_procs_io, obj_size,
			      num_objs, total_time, total_op_time, 
			      file_open_wait_time, prealloc_wait_time,
			      file_close_wait_time, barrier_wait_time, file_sync_wait_time, 
			      proc_send_wait_time, 
			      proc_recv_wait_time, file_op_wait_time, 
			      barrier_flag, sync_flag, io_proc_send_flag, 
                              aggregate_flag, verbose_flag, 
			      "Write", ofptr, efptr) != MPI_SUCCESS) {
      fprintf(efptr, "[RANK %d] ERROR: Problem in collect_and_print_time routine.\n", my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
    
/********************************************************************
* Now that the master timer is closed, print out the MPI file hints.
* It's possible that the Rank 0 I/O processor is not the global rank zero 
* processor and, thus, has not opened the output file. IF this is true, 
* have I/O rank 0 processor open the output file.
********************************************************************/
    if(info_allocated_flag){
      
      if(my_rank_io == 0){
	/*
	  fprintf(stderr,"**********I am rank %d and IO rank %d\n", my_rank, my_rank_io);
	  if(my_rank !=0){
	  
	  if(ofname != NULL)
	  if ((ofptr = fopen(ofname, "w")) == NULL){
	  fprintf(stderr, "[RANK %d] ERROR: Unable to open output information file %s.\n", my_rank , ofname);
	  MPI_Abort(MPI_COMM_WORLD, -1);
	  return -1;
	  }
	  }
	*/
	
	fprintf(ofptr, "\nMPI_info Hints Used (RANK 0):\n");
	
	for(i=0; i < nkeys; i++){
	  if(my_rank_io == 0){
	    fprintf(ofptr, "Key: %s ", key[i]);
	    fprintf(ofptr, " = %s\n", value[i]);
	  }
	  fflush(ofptr);
	}
	
	fprintf(ofptr, "\n");
	/*
	  if(ofname != NULL)
	  fclose(ofptr);
	*/
      }
    }
    
  }
  
/*******************************************************************
* Sleep before starting the read process.
*******************************************************************/
  sleep(sleep_seconds);

/******************************************************************
* Read phase:  Processes read all their respective data objects back
*              from the checkpoint file(s).
*              Do not read from file if the flag was set to not write data to 
*              file (-nofile) or if the write only flag was set (-op write).
*******************************************************************/
  if(!write_only_flag && write_file_flag){
    if (my_rank == 0) {
      fprintf(ofptr, "=== MPI read [%d->%d->%d %s] Starting ...\n",
	      num_files, num_procs_io, num_procs_world, tfname);
      fflush(ofptr);
    }
    
    if(check_data_ndx ==3) check_data_ndx = blksize;

/*  ******************************************************************
*   Reinitialize variables and data buffer arrays.
*   For all I/O processors, reset the circular buffers.
*   ******************************************************************/
    if( !collective_flag && (strided_flag == 0) && 
	((test_type == 2) || (test_type == 4) || (test_type == 5))) 
      set_view_all = TRUE;

    proc_send_wait_time = 0.0;
    file_op_wait_time = 0.0;
    
    for ( i = 0 ; i < num_of_cbuf ; i++ ) {
      cbuf_use[i] = 0 ;
    }
    
/***********************************************************************
* Call the master read routine, which will time the file open, reading of the
* data, and file close.
***********************************************************************/
    rc = MPIIO_master_read(my_rank, my_rank_io, io_processor, write_file_flag, comm_file, 
			   tfname, amode_read, info, nkeys, key, 
			   value, blksize, send_to, num_objs, num_receive,
			   num_of_cbuf, num_file_writes, obj_size, touch, 
			   pagesize, check_data_ndx, file_offset, poffset_val, 
			   other_offset, num_of_cjob, 
			   f_procs, cbuf_size, 
			   &file_open_wait_time, &file_op_wait_time,
			   &proc_recv_wait_time, 
			   &proc_send_wait_time, &total_op_time, 
			   &total_time, 
			   &file_close_wait_time,
			   aggregate_flag, io_proc_send_flag, 
			   info_allocated_flag, init_flag, 
			   collective_flag, set_view_all, 
			   strided_flag, cbuf_use, sync_flag, 
			   ofptr, efptr);

    if(!rc || rc == 2){
      fprintf(efptr, "[RANK %d] ERROR: Problem in MPIIO_master_read routine.\n", 
	      my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, 2);
    }
    
    if (MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS) {
      fprintf(efptr, "[RANK %d] ERROR: Problem in MPI_Barrier after writing data.\n", 
	      my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
    
/*  ****************************************************
*   Print timing results
*   ****************************************************/
    if(collect_and_print_time(my_rank, num_procs_world, num_procs_io, obj_size,
			      num_objs, total_time, total_op_time, 
			      file_open_wait_time, 0.0,
			      file_close_wait_time, 0.0, 0.0, proc_send_wait_time, 
			      proc_recv_wait_time, file_op_wait_time, 
			      FALSE, FALSE, io_proc_send_flag, aggregate_flag, verbose_flag, 
			      "Read", ofptr, efptr)  != MPI_SUCCESS) {
      fprintf(efptr, "[RANK %d] ERROR: Problem in collect_and_print_time routine.\n", my_rank);
      fflush(efptr);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
  } /* if(!write_only_flag && write_file_flag) */
  
/*****************************************************
* If necessary, delete the target file(s).
*****************************************************/
  if(io_processor && write_file_flag){
    if(delete_flag)
      if( (test_type == 1) ||(test_type == 3)){
	if( MPI_File_delete(tfname, info) != MPI_SUCCESS){
          fprintf(efptr,"[RANK %d] WARNING: Unable to remove file %s.\n",
                  my_rank, tfname);
        }
      }
      else{
        if(my_rank_io == 0)
	  if( MPI_File_delete(tfname, info) != MPI_SUCCESS){
            fprintf(efptr,"[RANK %d] WARNING: Unable to remove file %s.\n",
                    my_rank, tfname);
          }
      }
  }
  
  
/*******************************************************************
* Close files, free allocated memory, finish MPI and return
*******************************************************************/
  if(info_allocated_flag){
    for(i=0; i < nkeys; i++){
      free(key[i]);
      free(value[i]);
    }
    free(key);
    free(value);
  }

  if (ofname != NULL && !my_rank)   fclose(ofptr);
  if (efname != NULL)   fclose(efptr);
  if (cbuf_use != NULL) free(cbuf_use);
  if(num_hosts > 0){
    for ( i = 0 ; i < num_hosts; i++ )
      free(host_io_list[i]);
    free(host_io_list);
    if (io_procs_list != NULL) free(io_procs_list);
  }

  if(info != MPI_INFO_NULL){
    MPI_Info_free(&info);
  }
  
  if (MPI_Finalize() != MPI_SUCCESS) {
    fprintf(stderr,"Rank %d ERROR: MPI_Finalize failed.\n", my_rank);
    return -1;
  }
  
  return 0;
}
