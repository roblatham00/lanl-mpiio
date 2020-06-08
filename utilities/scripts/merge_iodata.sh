#! /bin/tcsh

# Script to parse the POSIX and MPI-IO N to N, N to 1 strided, and N to 1 
# non-strided I/O tests run on Cadillac in August of 2005. In addition, 
# a test to see if flushing the data before file close reduced the time of 
# file close. The file close test result file names begin with an "f".

# Parse MPI-IO test results

#rm -f total*results.txt

@ new_line = 1
foreach i (n1 s1 nn pn1 ps1 pnn)
    foreach procs (192 176 160 144 128 112 96 80 64 48 32 24 16 8 4 2)
        foreach size (131072 262144 524288 1048576 4194304 8388608 16777216 33554432 67108864 134217728)
            foreach run (1 2 3)
 
        # number of objects for n to n is unique
        if (${i} == "nn" || ${i} == "pnn") then
           @ nobj = 536870912 / $size   # 4 GB file
        else
           @ nobj = (1024 * 1024 * 1024) / $size          # 1 GB file
           @ nobj = (${nobj} * 40) / ${procs}  # 40 GB file
        endif 
 

	if(${nobj} < 2) then
	    @ nobj = 2
	endif
 
        if ($new_line == 1) set str_1 = "/home2/atorrez/iotests/mpi_io_test/src/parse_time.pl -label B -output results_${procs}_${i}.txt "
 
@ new_line = 0
set str_2 = ${i}_p${procs}_s${size}_o${nobj}_r${run}
    if( -f $str_2 )    set str_1 = "$str_1 $str_2"
 
        end
#	echo $str_1
    end
#    end
 
@ new_line = 1
$str_1
echo "Finished $i"
cat results_${procs}_${i}.txt >> total_${i}_results_test.txt
end
end
