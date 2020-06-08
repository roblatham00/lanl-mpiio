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
* PROGRAM:  verify
*
* PURPOSE:   A Collection of routines that will fill and check the data 
*            buffers that are aggregated and/or written to file.
*
* AUTHOR:    James Nunez 
* DATE:      February 12, 2001
* LAST MODIFIED: September 16, 2005 [jnunez]
* VERSION:   1.00.002
*
******************************************************************/

#include "mpi_io_test.h"
#include "boolean.h"

/*******************************************
 *   ROUTINE: verify_buffer
 *   PURPOSE: Check the data in the input buffer against the expected data 
 *            pattern.
 *  LAST MODIFIED: August 13, 2004 [jnunez]
 *******************************************/
size_t verify_buffer( int my_rank, double *obj, size_t chk_data_end, 
		      int touch, int pagesize, MPI_Offset touch_val,
		      FILE *ofptr, FILE *efptr){
  size_t errcnt = 0;
  size_t i;
  size_t expct_val = 0;
  
/****************
* Check that memory was allocated for the data object
***************/
  if(obj == NULL){
    fprintf(efptr, "[RANK %d] ERROR: Memory for buffer to fill is not allocated.\n", my_rank);
    return 0;
  }

/* **********
* Compute the expected value and check the input data buffer
************** */
  for(i=0; i < touch; i++){
    expct_val = i*pagesize;
    if( obj[expct_val] != (double)(touch_val + (MPI_Offset)expct_val)){
      
#ifdef DEBUG_IO
      fprintf(efptr,"[RANK %d] WARNING: Offset %ld (%ld) does not match expected values (offset = %ld).\n", my_rank, (long)i + 1,
	      (long)obj[expct_val], touch_val + (long)expct_val);
#endif
      errcnt++;
    }

/*  *******************************
*   Reset the offset values in the obj buffer so the next loop that checks 
*   each data element doesn't have to check for or skip the "touched" values.
*   ************************************/
    if(i == 0) obj[expct_val] = my_rank;
    else obj[expct_val] = (double)(expct_val + (MPI_Offset)my_rank);
  }
  
  /*********************
   * Check for unexpected values in the buffer.
   *************************/
  expct_val = my_rank;
  for(i=0; i < chk_data_end; i++){
    if((MPI_Offset)obj[i] != expct_val ){
      
#ifdef DEBUG_IO
      
      fprintf(efptr,"[RANK %d] WARNING: Data element %ld (value %lf) does not match expected value (%ld).\n", 
	      my_rank, (long)i+1, obj[i], (long)expct_val);
#endif
      
      errcnt++;
    }
    expct_val++;
  }
  
  /***************
   * Return the number of errors found
   ************************/
  return errcnt;
}

/*******************************************
*   ROUTINE: fill_buffer
*   PURPOSE: Fill the input buffer of doubles with a known data pattern.
*  LAST MODIFIED: June 12, 2004 [jnunez]
 *******************************************/
int fill_buffer( int my_rank, double *obj, size_t obj_size, int touch, 
		 int pagesize, MPI_Offset touch_val, FILE *ofptr, FILE *efptr){
  size_t i;
  
  /* ***************
   * Check that memory was allocated for the data object
   * **************/
  if(obj == NULL){
    fprintf(efptr, "[RANK %d] ERROR: Memory for buffer to fill is not allocated.\n", my_rank);
    return 0;
  }
  
  /* ***************
   * If the input value of touch is zero, this means that the offset is 
   * not written into the buffer
   ***************/
  for(i=0; i < obj_size; i++)
    obj[i] = (double)(i + (size_t)my_rank);

  if(touch > 0){
    for(i=0; i < touch; i++){
      obj[i*pagesize] = (double)(touch_val + 
				 (MPI_Offset)(i * (size_t)pagesize));
#ifdef DEBUG_IO
      fprintf(stderr,"RANK %d: VERIFY obj[%d] = %lf\n",
	      my_rank, i*pagesize, obj[i*pagesize]);
      fflush(stderr);
#endif
    }
  }
  
  return 1;
}

extern char *
expand_path(char *str, MPI_Comm comm)
{
  char tmp1[2048];
  char tmp2[1024];
  
  char *p;
  char *q;
  
  int rank;
  int string_modified = FALSE;
  
  if (str == (char *)0) {
    return ((char *)0);
  }
  
  for (p = str, q = tmp1; *p != '\0';) {
    if (*p == '%') {
      string_modified = TRUE;
      p++;
      if (*p == '\0') {
	return ((char *)0);
      }

      switch (*p) {
      case 'h':
	if (gethostname(tmp2, sizeof(tmp2) - 1) != 0)
	  return ((char *)0);
	tmp2[sizeof(tmp2) - 1] = '\0';
	strncpy(q, tmp2, (sizeof(tmp1) - (q - tmp1) - 1));
	tmp1[sizeof(tmp1) - 1] = '\0';
	q += strlen(tmp2);
	break;

      case 'p':
	snprintf(tmp2, (sizeof(tmp2) - 1), "%d", getpid());
	tmp2[sizeof(tmp2) - 1] = '\0';
	strncpy(q, tmp2, (sizeof(tmp1) - (q - tmp1) - 1));
	tmp1[sizeof(tmp1) - 1] = '\0';
	q += strlen(tmp2);
	break;

      case 'r':
	if (MPI_Comm_rank(comm, &rank) != MPI_SUCCESS) {
	  return ((char *)0);
	}
	snprintf(tmp2, (sizeof(tmp2) - 1), "%d", rank);
	tmp2[sizeof(tmp2) - 1] = '\0';
	strncpy(q, tmp2, (sizeof(tmp1) - (q - tmp1) - 1));
	tmp1[sizeof(tmp1) - 1] = '\0';
	q += strlen(tmp2);
	break;

      default:
	return ((char *)0);
      }

      p++;
    } 
    else {
      *q++ = *p++;
    }
  }

  if(!string_modified)
    strcpy(tmp1,str);

  return (strdup(tmp1));
}

/***************************************************************************
*
*  FUNCTION:  get_file_name
*  PURPOSE:   Open the file and load a file name. The processor with rank 
*             "rank" will load the file name on line "rank" + 1.
*
*  PARAMETERS:
*             char *input_fname     - Input file name containing target files
*             int my_rank           - Processor rank. 
*             int max_rank          - Maximum rank (Total Number) of processors
*                                     calling this routine
*             MPI_Comm comm         - Communicator all processors calling this routine
*             char **target_fname   - Output target file name
*             FILE *ofptr
*             FILE *efptr
*
*  RETURN:    TRUE(1) on successful exit; FALSE(0) otherwise
*  AUTHOR:    James Nunez
*  DATE:      August 30, 2001 
*  LAST MODIFIED:  August 13, 2003 [jnunez]
*
****************************************************************************/
int get_file_name(char *input_fname, int my_rank, int max_rank, MPI_Comm comm, 
		  char **target_fname, FILE *ofptr, FILE *efptr){
  
  FILE *fp = NULL;
  char buff[512];
  int fname_size, i = 0;
  MPI_Status status, status_2;  /* MPI_send status                       */
  MPI_Request request, request_2;
  
  /* All processors calling fopen should be changed to something like the 
     commented code below. We've seen mass fopen calls cause problems on 
     some file systems.*/

  if( (fp = fopen(input_fname, "r")) == NULL){
    fprintf(stderr, "[RANK %d] ERROR: Unable to open file %s\n",my_rank, input_fname);
    return(FALSE);
  }

  for(i=0; i< max_rank; i++){
    fscanf(fp, "%s", buff);
  }
  fclose(fp);         

  /* First crack at getting around the everyone call fopen problem */
    /*
  if(my_rank == 0){
    
    if( (fp = fopen(input_fname, "r")) == NULL){
      fprintf(stderr, "[RANK %d] ERROR: Unable to open file %s\n",my_rank, input_fname);
      return(FALSE);
    }
    
    fscanf(fp, "%s", buff);
    for(i=1; i< max_rank; i++){
      fscanf(fp, "%s", buff);
      fname_size = (int)strlen(buff) + 1;
      
      if( MPI_Isend(&fname_size, 1, MPI_INT, i, 5150, comm, 
		    &request) != MPI_SUCCESS) {
	fprintf(stderr, "[RANK %d] ERROR: Problem sending length of file name to processor %d.\n", 
		my_rank, i);
	return(FALSE);
      }
      
      if( MPI_Isend(buff, fname_size, MPI_CHAR, i, 5752, comm, 
		    &request_2) != MPI_SUCCESS) {
	fprintf(stderr, "[RANK %d] ERROR: Problem sending file name to processor %d.\n", 
		my_rank, i);
	return(FALSE);
      }
    }
    
    fseek(fp, 0, SEEK_SET);
    fscanf(fp, "%s", buff);
    fclose(fp);         
  }
  else{
    if(MPI_Recv(&fname_size, 1, MPI_INT, 0, 5150, comm, &status) != MPI_SUCCESS){
      fprintf(stderr, 
	      "[RANK %d] ERROR: Problem receiving length of file name from processor 0.\n",
	      my_rank);
      return(FALSE);
    }
    
    if( MPI_Recv(buff, fname_size, MPI_CHAR, 0, 5752, comm, 
		 &status_2) != MPI_SUCCESS) {
      fprintf(stderr, 
	      "[RANK %d] ERROR: Problem receiving file name from processor 0.\n",
	      my_rank);
      return(FALSE);
    }
  }
    */
  *target_fname = expand_path(buff, comm);

  return(TRUE);
}


/***************************************************************************
*
*  FUNCTION:  string_to_string_array
*  PURPOSE:   Parse an input string, with user defined delimined substrings,
*             into an array of strings. The original input string is not
*             modified by this routine.
*
*  PARAMETERS:
*             char *input_string    - Input string to parse containing
*                                     deliminted substrings
*             char *delimiter       - String used to parse the input string.
*                                     May be multiple characters, ex " *:"
*             int *num_args         - Number of substrings contained in the
*                                     input string
*             char ***output_string - Array of substrings contained in the
*                                     input string
*
*  RETURN:    TRUE(1) on successful exit; FALSE(0) otherwise
*  HISTORY:   03/05/01 [jnunez] Original version.
*
****************************************************************************/
int string_to_string_array(char *input_string, char *delimiter,
                           int *num_args, char ***output_string){

  char *temp_string = NULL;
  char *str_ptr = NULL;
  int max_len, counter;
  int i;                          /* General for loop index */


/*******************************************************************
* Check for missing input string.
******************************************************************/
  if(input_string == NULL){
    printf("Input string to parse does not exist (NULL).\n");
    return(FALSE);
  }

/*******************************************************************
* Allocate memory for a copy of the input string. This is used to count
* the number of and maximum length of the substrings and not destroy the
* input string.
******************************************************************/
  if( (temp_string = (char *)malloc(strlen(input_string) +1)) == NULL){
    printf("Unable to allocate memory for copy of the input string.\n");
    return(FALSE);
  }
  strcpy(temp_string, input_string);

/*******************************************************************
* Parse the input string to find out how many substrings it contains
* and the maximum length of substrings.
******************************************************************/
  str_ptr = strtok(temp_string, delimiter);
  counter = 1;
  max_len = (int)strlen(str_ptr);

  while(str_ptr){
    str_ptr = strtok(NULL, delimiter);
    if(str_ptr){
      counter++;
      if(strlen(str_ptr) > max_len) max_len = (int)strlen(str_ptr);
    }
  }

/*******************************************************************
* Allocate memory for the array of strings
******************************************************************/
  if( (*output_string = (char **)calloc(counter, sizeof(char *))) == NULL){
    printf("Unable to allocate memory for the output string.\n");
    return(FALSE);
  }
  for(i=0; i < counter; i++)
    if( ((*output_string)[i] = (char *)malloc(max_len + 1)) == NULL){
      printf("Unable to allocate memory for the output string.\n");
      return(FALSE);
    }

/*******************************************************************
* Allocate memory for a copy of the input string to parse and copy into
* the output array of strings.
******************************************************************/
  if( (temp_string = (char *)malloc(strlen(input_string) + 1)) == NULL){
    printf("Unable to allocate memory for a copy of the input string.\n");
    return(FALSE);
  }

  strcpy(temp_string, input_string);

/*******************************************************************
* Finally, copy the input substrings into the output array
******************************************************************/
  str_ptr = strtok(temp_string, delimiter);
  counter = 0;
  while(str_ptr){
    if(str_ptr){
      strcpy((*output_string)[counter], str_ptr);
      counter++;
    }
    str_ptr = strtok(NULL, delimiter);
  }
  /*
  for(i=0; i < counter; i++)
    printf("output string %d = %s.\n", i, (*output_string)[i]);
  */
/*******************************************************************
* Copy the number of substrings into the output variable and return
******************************************************************/
  *num_args = counter;

  return(TRUE);
}

/***************************************************************************
*
*  FUNCTION:  string_to_int_array
*  PURPOSE:   Parse an input string, with user defined delimined substrings,
*             into an array of integers. The original input string is not
*             modified by this routine.
*
*  PARAMETERS:
*             char *input_string    - Input string to parse containing
*                                     deliminted substrings
*             char *delimiter       - String used to parse the input string.
*                                     May be multiple characters, ex " *:"
*             int *num_args         - Number of substrings contained in the
*                                     input string
*             int **output_ints     - Array of integers contained in the
*                                     input string
*             int *sum              - Sum of the integers in the string
*
*  RETURN:    TRUE(1) on successful exit; FALSE(0) otherwise
*  HISTORY:   03/05/01 [jnunez] Original version.
*
****************************************************************************/
int string_to_int_array(char *input_string, char *delimiter,
                           int *num_args, int **output_ints, int *sum){

  char *temp_string = NULL;
  char *str_ptr = NULL;
  int counter;


/*******************************************************************
* Check for missing input string.
******************************************************************/
  if(input_string == NULL){
    printf("Input string to parse does not exist (NULL).\n");
    return(FALSE);
  }

/*******************************************************************
* Allocate memory for a copy of the input string. This is used to count
* the number of and maximum length of the substrings and not destroy the
* input string.
******************************************************************/
  if( (temp_string = (char *)malloc(strlen(input_string) +1)) == NULL){
    printf("Unable to allocate memory for copy of the input string.\n");
    return(FALSE);
  }
  strcpy(temp_string, input_string);

/*******************************************************************
* Parse the input string to find out how many substrings it contains
* and the maximum length of substrings.
******************************************************************/
  str_ptr = strtok(temp_string, delimiter);
  counter = 1;

  while(str_ptr){
    str_ptr = strtok(NULL, delimiter);
    if(str_ptr)
      counter++;
  }

/*******************************************************************
* Allocate memory for the array of strings
******************************************************************/
  if( (*output_ints = (int *)calloc(counter, sizeof(int))) == NULL){
    printf("Unable to allocate memory for the output string.\n");
    return(FALSE);
  }

/*******************************************************************
* Allocate memory for a copy of the input string to parse and copy into
* the output array of strings.
******************************************************************/
  if( (temp_string = (char *)malloc(strlen(input_string) + 1)) == NULL){
    printf("Unable to allocate memory for a copy of the input string.\n");
    return(FALSE);
  }
  
  strcpy(temp_string, input_string);

/*******************************************************************
* Finally, copy the input substrings into the output array
******************************************************************/
  str_ptr = strtok(temp_string, delimiter);
  *sum = 0;
  counter = 0;
  while(str_ptr){
    if(str_ptr){
      (*output_ints)[counter] =  atoi(str_ptr);
      *sum += (*output_ints)[counter];
      counter++;
    }
    str_ptr = strtok(NULL, delimiter);
  }
  /*
  for(i=0; i < counter; i++)
    printf("output string %d = %d.\n", i, (*output_ints)[i]);
  printf("output string sum %d.\n", *sum);
  */
/*******************************************************************
* Copy the number of substrings into the output variable and return
******************************************************************/
  *num_args = counter;

  return(TRUE);
}
