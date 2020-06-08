#! /bin/tcsh

# Script for MPI-IO N to 1 non-strided, strided, without and with barrier before each write

set basepath=/net/scratch1/swh/mpi_io_test
set machinefile=" "
set target=/net/scratch2/swh/panasas_testfiles
set mpicall="mpirun"
set mpiiotest=${basepath}/src
set output=${basepath}/results
 
	rm -f ${target}/test_*

# 0.125, 0.25, 0.5, 1, 4, 8, 16, 32, 64, 128 MB messages
foreach run (1 )
    foreach procs ( $1 )
        foreach size (131072 131080 262144 262152)
 
        @ nobj = (1024 * 1024 * 1024) / $size   # 1 GB file
        @ nobj = (${nobj} * 40) / ${procs}  # 40 GB file
 

	if(${nobj} < 2) then
	    @ nobj = 2
	endif

	    @ nobj = 1000

	date
# N to 1 MPI-IO: non-strided, no barrier before each write
set ofile=n1_p${procs}_s${size}_o${nobj}_r${run}

    if( ! -f ${output}/${ofile}) then
	echo ${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -size $size -nobj $nobj -sync -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 
	${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -size $size -nobj $nobj -sync -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 

	ls -l ${target}
	rm -f ${target}/test_*
    endif

	date
# N to 1 MPI-IO: non-strided, barrier before each write
set ofile=n1_p${procs}_s${size}_o${nobj}_barrier_r${run}

    if( ! -f ${output}/${ofile}) then
	echo ${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -size $size -nobj $nobj -barrier -sync -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 
	${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -size $size -nobj $nobj -barrier -sync -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 

	ls -l ${target}
	rm -f ${target}/test_*
    endif

	date

# N to 1 MPI-IO: strided, no barrier before each write
set ofile=s1_p${procs}_s${size}_o${nobj}_r${run}

    if( ! -f ${output}/${ofile}) then
	echo ${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -strided 1 -size $size -nobj $nobj -sync -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 
	${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -strided 1 -size $size -nobj $nobj -sync -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 

	ls -l ${target}
	rm -f ${target}/test_*
    endif

# N to 1 MPI-IO: strided,    barrier before each write
set ofile=s1_p${procs}_s${size}_o${nobj}_barrier_r${run}

    if( ! -f ${output}/${ofile}) then
	echo ${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -strided 1 -size $size -nobj $nobj -barrier -sync -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 
	${mpicall} -np $procs $machinefile ${mpiiotest}/mpi_io_test.x -type 2 -strided 1 -size $size -nobj $nobj -barrier -sync -touch 3 -chkdata 3 -output ${output}/${ofile} -target ${target}/test_${size}_${nobj} -deletefile -hints panfs_concurrent_write 1 

	ls -l ${target}
	rm -f ${target}/test_*
    endif

        date

    ls -l ${target}
    rm -f ${target}/test_*
 
end
end
end
