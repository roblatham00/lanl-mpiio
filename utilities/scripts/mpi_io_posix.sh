#! /bin/tcsh

# Script for POSIX and MPI-IO N to N, N to 1 strided, and N to 1 non-strided

# Cadillac
set basepath=/home2/jnunez
# set mpicall=/usr/local/project/mpich/mpich-panfs/bin/mpirun 
set mpicall=${basepath}/MPI/MPICH/mpich-1.2.6_panfs2.3.0/bin/mpirun

set machinefile="-machinefile ${basepath}/IO_tests/MACHINES/lanl_machines.txt"
set target=/panfs/REALM1/scratch/jnunez

# Lightning
#set basepath=/net/scratch/jnunez
#set machinefile=" "
#set target=/panfs/ll/home/jnunez

# set mpicall="mpirun"
set mpiiotest=${basepath}/IO_tests/mpi_io_test/src
set output=${basepath}/Panasas/results/client2.3.1/tests_20050919
set posixtest=${basepath}/IO_tests/POSIX
 
	rm -f ${target}/test_*

# 0.125, 0.25, 0.5, 1, 4, 8, 16, 32, 64, 128 MB messages
foreach run (1 2 3)
    foreach procs (4 8 16 32 64 128 160 192 176 144 112 96 80 48 24 2)
        foreach size (131072 262144 524288 1048576 4194304 8388608 16777216 33554432 67108864 134217728)
 
        @ nobj = (1024 * 1024 * 1024) / $size          # 1 GB file
        @ nobj = (${nobj} * 40) / ${procs}  # 40 GB file
 

	if(${nobj} < 2) then
	    @ nobj = 2
	endif

	date
# N to 1 MPI-IO: non-strided
set ofile=n1_p${procs}_s${size}_o${nobj}_r${run}

    if( ! -f ${output}/${ofile}) then
	echo ${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -size $size -nobj $nobj -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 
	${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -size $size -nobj $nobj -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 

	ls -l ${target}
	rm -f ${target}/test_*
    endif

	date

# POSIX N to 1 - nonstrided write and read
set ofile=pn1_p${procs}_s${size}_o${nobj}_r${run}

       if( ! -f ${output}/${ofile}) then
           echo ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -write -blocks $nobj -blockoffset 0 -blocksize $size -superblocks 1 -bopen -copen ${target}/testpx_${procs}_${size}_${nobj}
 
           echo ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -write -blocks $nobj -blockoffset 0 -blocksize $size -superblocks 1 -bopen -copen ${target}/testpx_${procs}_${size}_${nobj} > ${output}/${ofile}

           ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -write -blocks $nobj -blockoffset 0 -blocksize $size -superblocks 1 -bopen -copen ${target}/testpx_${procs}_${size}_${nobj} >> ${output}/${ofile}
 
           date
 
           echo ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -read -blocks $nobj -blockoffset 0 -blocksize $size -superblocks 1 -bopen -copen ${target}/testpx_${procs}_${size}_${nobj}
 
           echo ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -read -blocks $nobj -blockoffset 0 -blocksize $size -superblocks 1 -bopen -copen ${target}/testpx_${procs}_${size}_${nobj} >> ${output}/${ofile}
 
       ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -read -blocks $nobj -blockoffset 0 -blocksize $size -superblocks 1 -bopen -copen ${target}/testpx_${procs}_${size}_${nobj} >> ${output}/${ofile}
 
           rm -f ${target}/testpx_*
           ls -l ${target}
 
       endif

	date

# N to 1 MPI-IO: strided
set ofile=s1_p${procs}_s${size}_o${nobj}_r${run}

    if( ! -f ${output}/${ofile}) then
	echo ${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -strided 1 -size $size -nobj $nobj -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 
	${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -strided 1 -size $size -nobj $nobj -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 

	ls -l ${target}
	rm -f ${target}/test_*
    endif

        date

# POSIX N to 1 - strided write and read
set ofile=ps1_p${procs}_s${size}_o${nobj}_r${run}

       if( ! -f ${output}/${ofile}) then
           echo ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -write -blocks 1 -blockoffset 0 -blocksize $size -superblocks ${nobj} -bopen -copen ${target}/testp_${procs}_${size}_${nobj}
 
           echo ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -write -blocks 1 -blockoffset 0 -blocksize $size -superblocks ${nobj} -bopen -copen ${target}/testp_${procs}_${size}_${nobj} > ${output}/${ofile}

           ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -write -blocks 1 -blockoffset 0 -blocksize $size -superblocks ${nobj} -bopen -copen ${target}/testp_${procs}_${size}_${nobj} >> ${output}/${ofile}
 
           date
 
           echo ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -read -blocks 1 -blockoffset 0 -blocksize $size -superblocks ${nobj} -bopen -copen ${target}/testp_${procs}_${size}_${nobj}
 
           echo ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -read -blocks 1 -blockoffset 0 -blocksize $size -superblocks ${nobj} -bopen -copen ${target}/testp_${procs}_${size}_${nobj} >> ${output}/${ofile}
 
       ${mpicall} -np $procs $machinefile ${posixtest}/ntoone/ntoone -read -blocks 1 -blockoffset 0 -blocksize $size -superblocks ${nobj} -bopen -copen ${target}/testp_${procs}_${size}_${nobj} >> ${output}/${ofile} 
 
           rm -f ${target}/testp_*
           ls -l ${target}
 
       endif

        @ nobj = 536870912 / $size   # 4 GB file
 
	date

# MPI-IO N to N
set ofile=nn_p${procs}_s${size}_o${nobj}_r${run}
 
    if( ! -f ${output}/${ofile}) then
        echo ${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 1 -size $size -nobj $nobj -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj}.%r -deletefile
        ${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 1 -size $size -nobj $nobj -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj}.%r -deletefile
    endif
 
        date
 

# POSIX N to N
set ofile=pnn_p${procs}_s${size}_o${nobj}_r${run}
       if( ! -f ${output}/${ofile}) then
       touch ${target}/testposx_${procs}_${size}_${nobj}
           echo ${mpicall} -np $procs $machinefile ${posixtest}/nton/nton -write -blocks $nobj -blocksize $size -bopen ${target}/testposx_${procs}_${size}_${nobj}
           echo ${mpicall} -np $procs $machinefile ${posixtest}/nton/nton -write -blocks $nobj -blocksize $size -bopen ${target}/testposx_${procs}_${size}_${nobj} > ${output}/${ofile}
           ${mpicall} -np $procs $machinefile ${posixtest}/nton/nton -write -blocks $nobj -blocksize $size -bopen ${target}/testposx_${procs}_${size}_${nobj} >> ${output}/${ofile}

           date
           echo ${mpicall} -np $procs $machinefile ${posixtest}/nton/nton -read -blocks $nobj -blocksize $size -bopen ${target}/testposx_${procs}_${size}_${nobj}
           echo ${mpicall} -np $procs $machinefile ${posixtest}/nton/nton -read -blocks $nobj -blocksize $size -bopen ${target}/testposx_${procs}_${size}_${nobj} >> ${output}/${ofile}
           ${mpicall} -np $procs $machinefile ${posixtest}/nton/nton -read -blocks $nobj -blocksize $size -bopen ${target}/testposx_${procs}_${size}_${nobj} >> ${output}/${ofile}


       rm -rf ${target}/testposx_*
           ls -l ${target}

	   endif
           date
 

    ls -l ${target}
    rm -f ${target}/test_*
 
end
end
end
