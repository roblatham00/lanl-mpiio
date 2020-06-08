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
* PROGRAM:   MPIIO_hints
*
* PURPOSE:   Routines that set or query MPI-IO hints
*
* AUTHOR:    James Nunez 
* DATE:      August 15, 2003
* LAST MODIFIED: September 16, 2005 [jnunez]
* VERSION:   1.00.002
*
******************************************************************/
#include "mpi_io_test.h"

/*******************************************
 *   ROUTINE: MPIIO_set_hint
 *   PURPOSE: Set the MPI hint in the MPI_Info structure.
 *   LAST MODIFIED: 
 *******************************************/
int MPIIO_set_hint(int my_rank, MPI_Info *info, char *key, char *value)
{

  if((MPI_Info_set(*info, key, value)) !=  MPI_SUCCESS){
    if(my_rank == 0)
      fprintf(stderr, "[RANK %d] WARNING: Unable to set the hint %s to value %s in the MPI_info structure.\n", my_rank, key, value);
  }
  
  return 1;
}

/*******************************************
 *   ROUTINE: MPIIO_print_hints
 *   PURPOSE: Print the MPI hints in the MPI_Info structure.
 *   LAST MODIFIED: September 3, 2004 [jnunez]
 *******************************************/
int MPIIO_print_hints(int my_rank, int nkeys,
		      char **keys, char **values, FILE *ofptr, FILE *efptr)
{
  int i;
  
  fprintf(ofptr, "\nMPI_info Hints Used (RANK 0):\n");
  
  for(i=0; i < nkeys; i++){
    fprintf(ofptr, "Key: %s ", keys[i]);
    fprintf(ofptr, " = %s\n", values[i]);
  }
  fflush(ofptr);
  fprintf(ofptr, "\n");

  return 1;
}


/*******************************************
 *   ROUTINE: MPIIO_get_hints
 *   PURPOSE: Get the MPI hint in the MPI_Info structure.
 *   LAST MODIFIED: July 16, 2004 [jnunez]
 *******************************************/
int MPIIO_get_hints(int my_rank, MPI_File wfh, int *nkeys,
		   char ***keys, char ***values, FILE *ofptr, FILE *efptr)
{
  
  MPI_Info info = MPI_INFO_NULL;
  int i=0, dummy_int;
  
  MPI_File_get_info(wfh, &info);      
  MPI_Info_get_nkeys(info, nkeys);
  
  if( ( (*keys) = (char **)malloc( (*nkeys) * sizeof(char *) )) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for array of hints.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
  
  if( (*values = (char **)malloc((*nkeys) * sizeof(char *)) ) == NULL){
    fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for array of hint values.\n", my_rank);
    MPI_Finalize();
    return 0;
  }
  
  for(i=0; i < (*nkeys); i++){
    if( ((*keys)[i] = (char *)malloc(200 * sizeof(char)) ) == NULL){
      fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for array of hints.\n", my_rank);
      MPI_Finalize();
      return 0;
    }
    if( ((*values)[i] = (char *)malloc(200 * sizeof(char)) ) == NULL){
      fprintf(efptr,"[RANK %d] ERROR: Unable to allocate memory for array of hint values.\n", my_rank);
      MPI_Finalize();
      return 0;
    }
    
    MPI_Info_get_nthkey(info, i, (*keys)[i]);
    MPI_Info_get(info, (*keys)[i], 200, (*values)[i], &dummy_int);
    /*
    fprintf(ofptr, "Key %d: %s = %s flag %d\n",i, 
	    (*keys)[i],(*values)[i], dummy_int);
    */
  }
  
  MPI_Info_free(&info);
  
  return 1;
}
