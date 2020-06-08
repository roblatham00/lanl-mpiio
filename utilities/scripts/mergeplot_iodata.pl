#!/usr/bin/perl -w 

###############################################################################
# Name:  mergeplot_iodata.pl 
# Description: This script creates results files based on number of procs
#              and test type for all the io test data output files.  The 
#              data is then parsed in a format that cts_diff understands 
#              (for plotting).
#
#
# Author:      Alfred Torrez
# Date:        12/16/2005
# History:
#              Changed by       Date        Description
###############################################################################

use Getopt::Long;
use strict;

$| = 1;

##################################################################
# default directory for data and utilities is ./ - use command
# arguments --dataloc and --utiliitesloc to specify directory or
# specify paths here
# 
my $datapath = ".";
my $utilitiespath = ".";
#
##################################################################
my @tests;

my %opt = (
	    testtype     => "",
	    testlabel    => "",
	    dataloc      => "",
	    utilitiesloc => "",
	  );

GetOptions (\%opt, 
	    "testtype=s",
	    "testlabel=s",
	    "dataloc=s",
            "utilitiesloc=s",
	   ) || &print_usage;


if ($opt{testtype} eq "" || $opt{testlabel} eq "") {
  print "Must specify testtype and filename label\n";
  &print_usage;
  exit;
}
else {
        push(@tests, $opt{testtype});
}

if ($opt{testtype} eq "all") {
	@tests = qw/n1 s1 nn pn1 ps1 pnn/;
}
if ($opt{dataloc} ne "") {
	$datapath = $opt{dataloc};
}
if ($opt{utilitiesloc} ne "") {
	$utilitiespath = $opt{utilitiesloc};
}

my $versionlabel = $opt{testlabel};

my $ctsstring = "";

my $newline = 1;
my $str_1 = "";
my $str_2 = "";
my $testtitle = "";
my $testcase;
my $nobj;
my $procs;
my $size;
my $run;


foreach $testcase (@tests) {

	if ($testcase eq "nn" ) {$testtitle = "MPI-IO_N_to_N_____________Test---$versionlabel"}
	if ($testcase eq "pnn") {$testtitle = "POSIX__N_to_N_____________Test---$versionlabel"}
	if ($testcase eq "n1" ) {$testtitle = "MPI-IO_N_to_1_NON-STRIDED_Test---$versionlabel"}
	if ($testcase eq "pn1") {$testtitle = "POSIX__N_to_1_NON-STRIDED_Test---$versionlabel"}
	if ($testcase eq "s1" ) {$testtitle = "MPI-IO_N_to_1_____STRIDED_Test---$versionlabel"}
	if ($testcase eq "ps1") {$testtitle = "POSIX__N_to_1_____STRIDED_Test---$versionlabel"}

        `rm -f $datapath/$testtitle*`;
	`rm -f $datapath/*_Procs`;
        `rm -f $datapath/results*`;

#	foreach $procs (qw/192 176 160 144 128 112 96 80 64 48 32 24 16 8 4 2/) {
 	foreach $procs (qw/2 4 8 16 24 32 48 64 80 96 112 128 144 160 176 192 210/) {
		foreach $size (qw/32768 65536 103424 131072 262144 524288 1048576 4194304 8388608 16777216 33554432 67108864 134217728/) {
			foreach $run (qw/1 2 3/) {
				if ($testcase eq "nn" || $testcase eq "pnn") {
					$nobj = int 536870912 / $size;   # 4 GB file
				}
				else {
					$nobj = int ((1024 * 1024 * 1024) / $size);   # 1 GB file
					$nobj = int (($nobj * 40) / $procs);          # 40 GB file
				}
				if ($nobj < 2) {$nobj = 2}
				if ($newline == 1) {
					$str_1 = $utilitiespath."/parse_time.pl -label B -noheader -path $datapath -output results_".$procs."_".$testcase.".txt";

				}
				$newline = 0;
				$str_2 = $testcase."_p".$procs."_s".$size."_o".$nobj."_r".$run;
 				if (-e $datapath."/".$str_2) {$str_1 = $str_1." ".$str_2}
			} # end foreach run
		} # end foreach size
		$newline = 1;
 		`$str_1`;
		print "Finished $testcase\n";
 		`cat $datapath/results_$procs"_"$testcase".txt" >> $datapath/$testtitle`;
                 if (!-z $datapath."/results_".$procs."_".$testcase.".txt") {
                        my $pathfile = $datapath."/".$procs."_Procs ";
 			$ctsstring =  $ctsstring.$pathfile;
 		}

	} # end foreach procs	

my $outputa = $testtitle;
print "Running x-y-parser on $outputa\n";
print "\n";

`$utilitiespath/x-y-parser.pl -x2 -y4  -tio_test -f$outputa -p$datapath -d"(a) $outputa: Write Bandwidth-NET  " -m"Message Size (MB)  " -n"Minimum_Bandwidth (MB/sec)" -a`;
`$utilitiespath/x-y-parser.pl -x2 -y14  -tio_test -f$outputa -p$datapath -d"(b) $outputa: Write Bandwidth-RAW  " -m"Message Size (MB)  " -n"Minimum_Bandwidth (MB/sec)" -a`;
`$utilitiespath/x-y-parser.pl -x2 -y27  -tio_test -f$outputa -p$datapath -d"(c: $outputa: Write File Open  Time" -m"Message Size (MB)  " -n"Maximum_Time_(sec)" -a`;
`$utilitiespath/x-y-parser.pl -x1 -y27  -tio_test -f$outputa -p$datapath -d"(d) $outputa: Write File Open  Time" -m"Number of Processes" -n"Maximum_Time_(sec)" -a`;
`$utilitiespath/x-y-parser.pl -x2 -y32  -tio_test -f$outputa -p$datapath -d"(e) $outputa: Write File Close Time" -m"Message Size (MB)  " -n"Maximum_Time_(sec)" -a`;
`$utilitiespath/x-y-parser.pl -x1 -y32  -tio_test -f$outputa -p$datapath -d"(f) $outputa: Write File Close Time" -m"Number of Processes" -n"Maximum_Time_(sec)" -a`;

`$utilitiespath/x-y-parser.pl -x2 -y39 -tio_test -f$outputa -p$datapath -d"(g) $outputa: Read  Bandwidth-NET  " -m"Message Size (MB)  " -n"Minimum_Bandwidth (MB/sec)" -a`;
`$utilitiespath/x-y-parser.pl -x2 -y49 -tio_test -f$outputa -p$datapath -d"(h) $outputa: Read  Bandwidth-RAW  " -m"Message Size (MB)  " -n"Minimum_Bandwidth (MB/sec)" -a`;
`$utilitiespath/x-y-parser.pl -x2 -y62 -tio_test -f$outputa -p$datapath -d"(i) $outputa: Read  File Open  Time" -m"Message Size (MB)  " -n"Maximum_Time_(sec)" -a`;
`$utilitiespath/x-y-parser.pl -x1 -y62 -tio_test -f$outputa -p$datapath -d"(j) $outputa: Read  File Open  Time" -m"Number of Processes" -n"Maximum_Time_(sec)" -a`;
`$utilitiespath/x-y-parser.pl -x2 -y67 -tio_test -f$outputa -p$datapath -d"(k) $outputa: Read  File Close Time" -m"Message Size (MB)  " -n"Maximum_Time_(sec)" -a`;
`$utilitiespath/x-y-parser.pl -x1 -y67 -tio_test -f$outputa -p$datapath -d"(l) $outputa: Read  File Close Time" -m"Number of Processes" -n"Maximum_Time_(sec)" -a`;
#

if ($ctsstring ne "") {
	print "Running cts_diff on  $outputa\n";
	print "\n";
	`$utilitiespath/cts_diff.pl -presult -ft cts -no_intp -plot_orig $ctsstring`;

	$ctsstring = "";

 	`mv ./cts_diff.pdf $datapath/$testtitle.pdf`;
 	`mv ./cts_diff.ps $datapath/$testtitle.ps`;
 	`mv ./cts_diff.data $datapath/$testtitle.data`;
}

} # end foreach test case



sub print_usage
{
	print "Usage:  mergeplot.pl --testtype=[all|s1|n1|nn|pn1|ps1|pnn|] --testlabel=[versionlabel]\n";
	print "                     --dataloc=[datapath]  --utilitiesloc=[utilitiespath]\n";
	print "\n";
	print "  Note:  --testtype and --testlabel are required arguments\n";
	print "         --dataloc and --utilitiesloc are optional - default directory will be ./\n";
}








