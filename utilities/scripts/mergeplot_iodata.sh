#! /bin/tcsh

# This script parses the output of POSIX and MPI-IO N to N, N to 1 strided,
# and N to 1 non-strided I/O tests.  After Parsing, the data is fed to
# x-y-parser.  x-y-parser formats the data for input into cts_diff,
# for plotting purposes.

# At some point, this script should be converted to a perl script
# to allow for hashing of test types and better structure.

# Parse MPI-IO test results
set test = $1
set Version = $2


if (${test} == "") then
  set test = all
else
  if (${test} == "-h") then
      echo "usage:  mergeplot_iodata.sh TestType VersionLabel" 
      echo "        TesType = [all | n1 | s1 | nn | pn1 | ps1 | pnn]" 
      exit 
  endif
endif
   
if (${test} == "all") then 
  set teststring = "n1 s1 nn pn1 ps1 pnn" 
else
  if (${test} == "n1" || \
      ${test} == "s1" || \
      ${test} == "nn" || \
      ${test} == "pn1" || \
      ${test} == "ps1" || \
      ${test} == "pnn") then 
      set teststring = ${test}
  else
      echo "First argument should be [all | n1 | s1 | nn | pn1 | ps1 | pnn]"
      echo "usage:  mergeplot_iodata.sh TestType VersionLabel" 
      echo "        TestType = [all | n1 | s1 | nn | pn1 | ps1 | pnn]" 
      exit 
  endif    
endif
if (${Version} == "" ) then
  echo "Please Enter Version title"
  echo ""
  echo "usage:  mergeplot_iodata.sh TestType VersionLabel" 
  echo "        TestType = [all | n1 | s1 | nn | pn1 | ps1 | pnn]" 
  exit 
endif 
  

@ new_line = 1
foreach test_case (${teststring})
#foreach test_case (n1 s1 nn pn1 ps1 pnn)
    if (${test_case} == "nn" ) set test_title = "MPI-IO_N_to_N_____________Test---${Version}"
    if (${test_case} == "pnn") set test_title = "POSIX__N_to_N_____________Test---${Version}"
    if (${test_case} == "n1" ) set test_title = "MPI-IO_N_to_1_NON-STRIDED_Test---${Version}"
    if (${test_case} == "pn1") set test_title = "POSIX__N_to_1_NON-STRIDED_Test---${Version}"
    if (${test_case} == "s1" ) set test_title = "MPI-IO_N_to_1_____STRIDED_Test---${Version}"
    if (${test_case} == "ps1") set test_title = "POSIX__N_to_1_____STRIDED_Test---${Version}"

    rm -f ${test_title}*
    rm -f *_Procs

    foreach procs (192 176 160 144 128 112 96 80 64 48 32 24 16 8 4 2)
        foreach size (131072 262144 524288 1048576 4194304 8388608 16777216 33554432 67108864 134217728)
            foreach run (1 2 3)
 
        # number of objects for n to n is unique
        if(${test_case} == "nn" || ${test_case} == "pnn") then
           @ nobj = 536870912 / $size   # 4 GB file
        else
           @ nobj = (1024 * 1024 * 1024) / $size          # 1 GB file
           @ nobj = (${nobj} * 40) / ${procs}  # 40 GB file
        endif 
 

	if(${nobj} < 2) then
	    @ nobj = 2
	endif
 
        if ($new_line == 1) set str_1 = "/home2/jnunez/IO_tests/utilities/scripts/parse_time.pl -label B -noheader -output results_${procs}_${test_case}.txt "
 
@ new_line = 0
set str_2 = ${test_case}_p${procs}_s${size}_o${nobj}_r${run}
    if( -f $str_2 )    set str_1 = "$str_1 $str_2"
 
        end    # foreach run
#	echo $str_1
    end # foreach size
#    end
 
@ new_line = 1
$str_1
echo "Finished $test_case"
cat results_${procs}_${test_case}.txt >> ${test_title}
#cat results_${procs}_${test_case}.txt
end #for each procs

set outputa = ${test_title}
echo ${outputa} 

./x-y-parser.pl -x2 -y4  -q 0 -tio_test -f${outputa} -d"(a) ${outputa}: Write Bandwidth-NET  " -m"Message Size (MB)  " -n"Minimum_Bandwidth (MB/sec)" -a
./x-y-parser.pl -x2 -y14  -q 0 -tio_test -f${outputa} -d"(b) ${outputa}: Write Bandwidth-RAW  " -m"Message Size (MB)  " -n"Minimum_Bandwidth (MB/sec)" -a
./x-y-parser.pl -x2 -y27  -q 0 -tio_test -f${outputa} -d"(c: ${outputa}: Write File Open  Time" -m"Message Size (MB)  " -n"Maximum_Time_(sec)"         -a
./x-y-parser.pl -x1 -y27  -q 0 -tio_test -f${outputa} -d"(d) ${outputa}: Write File Open  Time" -m"Number of Processes" -n"Maximum_Time_(sec)"         -a
./x-y-parser.pl -x2 -y32  -q 0 -tio_test -f${outputa} -d"(e) ${outputa}: Write File Close Time" -m"Message Size (MB)  " -n"Maximum_Time_(sec)"         -a
./x-y-parser.pl -x1 -y32  -q 0 -tio_test -f${outputa} -d"(f) ${outputa}: Write File Close Time" -m"Number of Processes" -n"Maximum_Time_(sec)"         -a

./x-y-parser.pl -x2 -y39 -q 0 -tio_test -f${outputa} -d"(g) ${outputa}: Read  Bandwidth-NET  " -m"Message Size (MB)  " -n"Minimum_Bandwidth (MB/sec)" -a
./x-y-parser.pl -x2 -y49 -q 0 -tio_test -f${outputa} -d"(h) ${outputa}: Read  Bandwidth-RAW  " -m"Message Size (MB)  " -n"Minimum_Bandwidth (MB/sec)" -a
./x-y-parser.pl -x2 -y62 -q 0 -tio_test -f${outputa} -d"(i) ${outputa}: Read  File Open  Time" -m"Message Size (MB)  " -n"Maximum_Time_(sec)"         -a
./x-y-parser.pl -x1 -y62 -q 0 -tio_test -f${outputa} -d"(j) ${outputa}: Read  File Open  Time" -m"Number of Processes" -n"Maximum_Time_(sec)"         -a
./x-y-parser.pl -x2 -y67 -q 0 -tio_test -f${outputa} -d"(k) ${outputa}: Read  File Close Time" -m"Message Size (MB)  " -n"Maximum_Time_(sec)"         -a
./x-y-parser.pl -x1 -y67 -q 0 -tio_test -f${outputa} -d"(l) ${outputa}: Read  File Close Time" -m"Number of Processes" -n"Maximum_Time_(sec)"         -a

./cts_diff.pl -presult -ft cts -no_intp -plot_orig 4_Procs 8_Procs 16_Procs 32_Procs 48_Procs 64_Procs 96_Procs 128_Procs 144_Procs 160_Procs 176_Procs 192_Procs 210_Procs

mv cts_diff.pdf ${test_title}.pdf

end #for each test_case 
