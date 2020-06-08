#!/usr/bin/perl
# #!/usr/local/gnu/bin/perl

# Script to parse and print the output of the MPI IO test program.
# parse_time.pl -label % -separate -sync -output % filename1 ... filenameN
#
#    -label %  = Convert and print the first number in each file name between a
#                pair of "_" at the beginning of each line in the 
#                output file.
#                All numbers are assumed to be doubles (8 bytes). Convert to:
#                  B - Convert to Bytes; multiply by 8.
#                  K - Convert to Kilobytes; divide by 128
#                  M - Convert to Megabytes; divide by 128*1024 
#             
#    -separate = Write the read and write results to separate files. The
#                output name will be appended with .rd or .wr
#
#    -sync     = Look for File sync data for the write operation
#
#    -pess     = Process/print  ALL minimum(pessimistic)           performance data only
#    -besteff  = Find   /print BEST minimum(pessimistic) effective performance data only
#
#    -agg      = Look for aggregation data for the read and write operations 
#                in the input files.
#
#    -output % = Output file name. Must be the last argument specified 
#                followed by input files. 
#                (Default: STDOUT)
#
#    -verbose  = Verbose printing to stdout
#
#  Written:       February 12, 2002
#  Last Modified: December 8, 2003

# At least one file must be specified, else exit
if($#ARGV < 0){ 
    printf "Usage parse_time.pl -label [B | K | M] -separate -sync -pess -besteff -output ofname filename1 filename2 ... filenameN\n";
    die "You must enter at least one file name to parse\n";
}

# Set some default values
$Index = 0;
$nobj = -1;
$procs = -1;
$Delim = 0;
$Label = 0;
$convert = 1;
$agg_flag = 0;
$sep_flag = 0;
$sync_flag = 0;
$pess_flag = 0;
$besteff_run_only_flag = 0;
$verbose_flag = 0;
$procs_max = $msize_max = $nobj_max = 0;

# Parse input string
while ($Index <= $#ARGV){
    
    if ($verbose_flag == 1) { printf "input arg $Index = $ARGV[$Index]\n"; }
    
# Check if user wants to parse the file name
    if($ARGV[$Index] eq "-delim"){
	$Delim = 1;
	$Index++;
    }
# Unit of file size as found in file name
    elsif($ARGV[$Index] eq "-label"){
	$Label = 1;
	$Index++;
	
	$units = $ARGV[$Index];
	if($ARGV[$Index] eq "B"){
	    $convert = 8;
	}
	elsif($ARGV[$Index] eq "K"){
	    $convert = 1/128;
	}
	elsif($ARGV[$Index] eq "M"){
	    $convert = 1/(128*1024);
	}
	else{
	    die "An M, K, or B must follow the -label command line flag.\n";
	}
	$Index++;
    }
# Check if user specified what parts of the file to print
    elsif($ARGV[$Index] eq "-separate"){
	$sep_flag = 1;
	$Index++;
    }
# Check if user wants file sync data for the file close on write
    elsif($ARGV[$Index] eq "-sync"){
	$sync_flag = 1;
	$Index++;
    }
# Check if user wants only miniumum(pessimistic) performance data
    elsif($ARGV[$Index] eq "-pess"){
	$pess_flag = 1;
	$Index++;
    }
# Check if user wants only miniumum(pessimistic) performance data, and besteff case only
    elsif($ARGV[$Index] eq "-besteff"){
	$besteff_run_only_flag = 1;
	$Index++;
    }
# Check if user wants aggregation data included
    elsif($ARGV[$Index] eq "-agg"){
	$agg_flag = 1;
	$Index++;
    }
# Check if user wants verbose stuff printed to stdout
    elsif($ARGV[$Index] eq "-verbose"){
	$verbose_flag = 1;
	$Index++;
    }
# Check if user specified output file name. 
    elsif($ARGV[$Index] eq "-output"){
	if($sep_flag == 1){
	  open(ROFNAME, ">$ARGV[$Index + 1].rd") or die "Unable to open file $ARGV[$Index + 1]\n"; 
	  open(WOFNAME, ">$ARGV[$Index + 1].wr") or die "Unable to open file $ARGV[$Index + 1]\n"; 
	    
	    # Print name of files to be read
	    print ROFNAME "# Filenames:  "; 
	    print WOFNAME "# Filenames:  "; 
	    for($i = $Index; $i <= $#ARGV; $i++){
		print ROFNAME "$ARGV[$i] "; 
		print WOFNAME "$ARGV[$i] "; 
	    }
	    print ROFNAME "\n"; 
	    print WOFNAME "\n"; 
	    print ROFNAME "# \n"; 
	    print WOFNAME "# \n"; 
	    
	    # Print Header
	    print ROFNAME "# Num \t"; 
	    print WOFNAME "# Num \t"; 
	    if($Label){
		print ROFNAME "Size \t"; 
		print WOFNAME "Size \t"; 
	    }
	    
	    print ROFNAME "Num \tEffective Bandwidth \t\t\t\t\t\tTotal \t\t\t\t\t\t\tRead Bandwidth \t\t\t\t\t\t\tRead  \t\t\t\t\t\t\t\tFile Open\t\t\t\t\t\t\tFile Close\t\t\t\t\t\t\tRead Wait "; 
	    print WOFNAME "Num \tEffective Bandwidth \t\t\t\t\t\tTotal \t\t\t\t\t\t\tWrite Bandwidth \t\t\t\t\t\tWrite  \t\t\t\t\t\t\t\tFile Open\t\t\t\t\t\t\tFile Close\t\t\t\t\t\t\tWrite Wait "; 
	    
	    if($sync_flag){
		print WOFNAME "\t\t\t\t\tFile Sync"; 
	    }
	    if($agg_flag){
		print WOFNAME "\t\t\t\t\t\t\tSend Wait \t\t\t\tRecv Wait";
		print ROFNAME "\t\t\t\t\t\t\tSend Wait \t\t\t\tRecv Wait";
	    }
	    
	    print WOFNAME "\n"; 
	    print ROFNAME "\n"; 
	    
	    print ROFNAME "# Procs "; 
	    print WOFNAME "# Procs "; 
	    if($Label){
		print ROFNAME "Obj($units)\t"; 
		print WOFNAME "Obj($units)\t"; 
	    }
	    
	    print ROFNAME "Obj \t(MB/sec) \t\t\t\t\t\t\tTime (sec.)\t \t\t\t\t\t(MB/sec) \t\t\t\t\t\t\tTime (sec.) \t\t\t\t\t\t\tTime (sec) \t\t\t\t\t\t\tTime (sec) \t\t\t\t\t\t\tTime (sec.) ";   
	    print WOFNAME "Obj \t(MB/sec) \t\t\t\t\t\t\tTime (sec.)\t \t\t\t\t\t(MB/sec) \t\t\t\t\t\t\tTime (sec.) \t\t\t\t\t\t\tTime (sec) \t\t\t\t\t\t\tTime (sec) \t\t\t\t\t\t\tTime (sec.) "; 
	    
	    if($sync_flag){
		print WOFNAME "\t\t\t\t\tTime (sec)";   
	    }
	    if($agg_flag){
		print WOFNAME "\t\t\t\t\t\t\tTime (sec) \t\t\t\tTime (sec)";
		print ROFNAME "\t\t\t\t\t\t\tTime (sec) \t\t\t\tTime (sec)";
	    }
	    
	    print WOFNAME "\n";   
	    print ROFNAME "\n";   
	    
	    print ROFNAME "# \t"; 
	    print WOFNAME "# \t"; 
	    if($Label){
		print ROFNAME "\t"; 
		print WOFNAME "\t"; 
	    }
	    
	    print ROFNAME "\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax)";
	    print WOFNAME "\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax)";
	    
	    if($sync_flag){
		print WOFNAME "\t\t\t\t(min \tavg \tmax)";
	    }
	    if($agg_flag){
		print WOFNAME "\t\t\t\t\t\t(min \tavg \tmax)\t\t\t(min \tavg \tmax)";
		print ROFNAME "\t\t\t\t\t\t(min \tavg \tmax)\t\t\t(min \tavg \tmax)";
	    }
	    
	    print WOFNAME "\n";
	    print ROFNAME "\n";
	    
	}
	else
        {
	 open(OFNAME, ">$ARGV[$Index + 1]") or die "Unable to open file $ARGV[$Index + 1]\n"; 
	    
	    # Print name of files to be read
#	    print OFNAME "# Filenames:  "; 
#	    for($i = $Index; $i <= $#ARGV; $i++){
#		print OFNAME "$ARGV[$i] "; 
#	    }
#	    print OFNAME "\n"; 
#	    print OFNAME "# \n"; 
	    
	    # Print Header
            if ($pess_flag == 0)
            {
	      print OFNAME "# Num \t"; 
  	      if($Label){ print OFNAME "Size\t"; }
	    
	      print OFNAME "Num \tEffective Bandwidth \t\t\t\t\t\tTotal \t\t\t\t\t\t\tWrite Bandwidth \t\t\t\t\t\tWrite  \t\t\t\t\t\t\t\tFile Open\t\t\t\t\t\t\tFile Close\t\t\t\t\t\t\tWrite Wait ";
	      if($sync_flag){ print OFNAME "\t\t\t\t\tFile Sync "; }
	      if($agg_flag ){ print OFNAME "\t\t\t\t\t\t\tSend Wait \t\t\t\tRecv Wait "; }
	    
	      print OFNAME "\t\t\t\t\t\t\tEffective Bandwidth \t\t\t\t\t\tTotal \t\t\t\t\t\t\t\tRead Bandwidth \t\t\t\t\t\t\tRead  \t\t\t\t\t\t\t\tFile Open\t\t\t\t\t\t\tFile Close\t\t\t\t\t\t\tRead Wait ";
	    
	      if($agg_flag){ print OFNAME "\t\t\t\t\t\t\tSend Wait \t\t\t\tRecv Wait"; }
	    
	      print OFNAME "\n# Procs "; 
	      if($Label){ print OFNAME "Obj($units)\t"; }
	    
	      print OFNAME "Obj \t(MB/sec) \t\t\t\t\t\t\tTime (sec.)\t \t\t\t\t\t(MB/sec) \t\t\t\t\t\t\tTime (sec.) \t\t\t\t\t\t\tTime (sec) \t\t\t\t\t\t\tTime (sec) \t\t\t\t\t\t\tTime (sec.) ";
	      if($sync_flag){ print OFNAME "\t\t\t\t\tTime (sec) "; }
	      if($agg_flag ){ print OFNAME "\t\t\t\t\t\t\tTime (sec) \t\t\t\tTime (sec) "; }
	    
	      print OFNAME "\t\t\t\t\t\t\t(MB/sec) \t\t\t\t\t\t\tTime (sec.)\t \t\t\t\t\t\t(MB/sec) \t\t\t\t\t\t\tTime (sec.) \t\t\t\t\t\t\tTime (sec) \t\t\t\t\t\t\tTime (sec) \t\t\t\t\t\t\tTime (sec.) ";   
	    
	      if($agg_flag){ print OFNAME "\t\t\t\t\t\t\tTime (sec) \t\t\t\tTime (sec)"; }
	    
	      print OFNAME "\n# \t";
	      if($Label){ print OFNAME "\t"; }
	    
	      print OFNAME "\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax)";
	    
	      if($sync_flag){ print OFNAME "\t\t\t\t(min \tavg \tmax)"; }
	      if($agg_flag ){ print OFNAME "\t\t\t\t\t\t(min \tavg \tmax)\t\t\t(min \tavg \tmax)"; }
	    
	      print OFNAME "\t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax) \t\t\t\t\t\t(min \tavg \tmax)";
	      if($agg_flag) { print OFNAME "\t\t\t\t\t\t(min \tavg \tmax)\t\t\t(min \tavg \tmax)"; }
	      print OFNAME "\n";
            }
            if ($pess_flag == 1)
            {
	      print  "\n#Procs "; 
	      print  "Size($units)\t";
	    
	      print  "#Writes W-Eff(MB/s) W-Eff(s) W-Raw(MB/s) W-Raw(s) W-Open(s) W-Close(s) W-Wait(s)";
	      if($sync_flag){ print  " W-Sync(s)"; }
	      if($agg_flag ){ print  " W-Send_Wait(s) W-Recv_Wait(s)"; }
	      print  " r#";
	      print  " R-Eff(MB/s) R-Eff(s) R-Raw(MB/s) R-Raw(s) R-Open(s) R-Close(s) R-Wait(s)";
	      print  " r#";

	      print  " Test Results File\n";
            }
          }
	
	$Index += 2;
# Get the index in the input where the file names are.
	$infile_ndx = $Index;
    }
# Else skip over any unknown input
    else{
	$Index++;
    }
}
    if ($sep_flag  == 1 && $pess_flag             == 1) { die "-pess option not implemented for -separate option.\n"; }
    if ($sep_flag  == 1 && $besteff_run_only_flag == 1) { die "-besteff option not implemented for -separate option.\n"; }
    if ($pess_flag == 0 && $besteff_run_only_flag == 1) { die "Must specify -pess option with -besteff.\n"; }

for($i = $infile_ndx; $i <= $#ARGV; $i++){
    
# Open input file $i 
    open(DATA, $ARGV[$i]) or die "Unable to open file $ARGV[$i]\n"; 
    
# Parse file name to get information on number of processors, etc.
    
    $procs = $nobj = $msize = -1;
    if($ARGV[$i] =~ /(\w+)_p(\d+)_/){
	    $procs = $2;
	}
    if($ARGV[$i] =~ /(\w+)_o(\d+)_/){
	    $nobj = $2;
	}
    if($ARGV[$i] =~ /(\w+)_s(\d+)_/){
	    $msize = $convert * $2;
	}
    
    if ($verbose_flag == 1) {print "procs $procs objects $nobj size converted $msize\n"; }

# For each new file, reset output variables
    $total_write_rate = $total_write_rate_2 = $total_write_rate_3 = $total_write_rate_p = -1.0;
    $total_read_rate = $total_read_rate_2 = $total_read_rate_3 = $total_read_rate_p = -1.0;
    $write_rate = $write_rate_2 = $write_rate_3 = -1.0;
    $read_rate = $read_rate_2 = $read_rate_3 = -1.0;
    $write_time = $write_time_2 = $write_time_3 = -1.0;
    $read_time = $read_time_2 = $read_time_3 = -1.0;
    $write_file_open = $write_file_open_2 = $write_file_open_3 = -1.0;
    $read_file_open = $read_file_open_2 = $read_file_open_3 = -1.0;
    $write_file_sync = $write_file_sync_2 = $write_file_sync_3 = -1.0;
    $read_time_fc = $read_time_fc_2 = $read_time_fc_3 = -1;
    $write_time_fc = $write_time_fc_2 = $write_time_fc_3 = -1;
    $write_wait = $write_wait_2 = $write_wait_3 = -1.0;
    $read_wait = $read_wait_2 = $read_wait_3 = -1.0;
    $write_send = $write_send_2 = $write_send_3 = -1.0;
    $read_send = $read_send_2 = $read_send_3 = -1.0;
    $write_recv = $write_recv_2 = $write_recv_3 = -1.0;
    $read_recv = $read_recv_2 = $read_recv_3 = -1.0;
    
    do{    
	$lines = <DATA>;
	

	# Get the number of processors from the input file
	if($lines =~ m/Total number of processors (.*?): (\d*)/){
	    $procs = $2;
	}
	# Get the number of objects from the input file
	if($lines =~ m/Number of objects (.*?): (\d*)/){
	    $nobj = $2;
	}
	# Get the size of each object from the input file
	if($lines =~ m/Number of doubles .*? (.*?): (\d*)/){
	    $msize = $convert*$2;
	}
	
# For MPI-IO output files
	if($lines =~ m/=== MPI write/){
	    $op = 2;
	}
# For POSIX output files
	elsif($lines =~ /(\w+) -write /){
	    $op = 2;
	}
	elsif($lines =~ /(\w+) -read /){
	    $op = 1;
	}
	elsif($lines =~ m/=== MPI read/){
	    $op = 1;
	}
 	elsif($lines =~ /=== Effective Bandwidth (.*?): (.*? .*?) (.*?) (.*?) M/ ){
	    if($op == 1){
		$total_read_rate = $2;
		$total_read_rate_2 = $3;
		$total_read_rate_3 = $4;
	    }
	    elsif($op == 2){
		$total_write_rate = $2;
		$total_write_rate_2 = $3;
		$total_write_rate_3 = $4;
	    }
	}
	elsif($lines =~ /=== Total Time (.*?): (.*? .*?) (.*?) (.*?) s/ ){
	    if($op == 1){
		$total_read_time = $2;
		$total_read_time_2 = $3;
		$total_read_time_3 = $4;
	    }
	    elsif($op == 2){
		$total_write_time = $2;
		$total_write_time_2 = $3;
		$total_write_time_3 = $4;
	    }
	}
	elsif($lines =~ /=== (Read|Write) Bandwidth (.*?): (.*? .*?) (.*?) (.*?) M/ ){
	    if($op == 1){
		$read_rate = $3;
		$read_rate_2 = $4;
		$read_rate_3 = $5;
	    }
	    elsif($op == 2){
		$write_rate = $3;
		$write_rate_2 = $4;
		$write_rate_3 = $5;
	    }
	}
	elsif($lines =~ /=== (Read|Write) Time (.*?): (.*? .*?) (.*?) (.*?) s/ ){
	    if($op == 1){
		$read_time = $3;
		$read_time_2 = $4;
		$read_time_3 = $5;
	    }
	    elsif($op == 2){
		$write_time = $3;
		$write_time_2 = $4;
		$write_time_3 = $5;
	    }
	}
	elsif($lines =~ /=== MPI File Open (.*?): (.*? .*?) (.*?) (.*?) s/ ){
	    if($op == 1){
		$read_file_open = $2;
		$read_file_open_2 = $3;
		$read_file_open_3 = $4;
	    }
	    elsif($op == 2){
		$write_file_open = $2;
		$write_file_open_2 = $3;
		$write_file_open_3 = $4;
	    }
	}
	elsif($lines =~ /=== MPI File Sync Time (.*?): (.*? .*?) (.*?) (.*?) s/ ){
	    # Remember, there is no file sync when reading the file
	    if($op == 2){
		$write_file_sync = $2;
		$write_file_sync_2 = $3;
		$write_file_sync_3 = $4;
	    }
	}
	elsif($lines =~ /=== MPI File Close (.*?): (.*? .*?) (.*?) (.*?) s/ ){
	    if($op == 1){
		$read_time_fc = $2;
		$read_time_fc_2 = $3;
		$read_time_fc_3 = $4;
	    }
	    elsif($op == 2){
		$write_time_fc = $2;
		$write_time_fc_2 = $3;
		$write_time_fc_3 = $4;
	    }
	}
	elsif($lines =~ /=== MPI File (.*?): (.*? .*?) (.*?) (.*?) s/ ){
	    if($op == 1){
		$read_wait = $2;
		$read_wait_2 = $3;
		$read_wait_3 = $4;
	    }
	    elsif($op == 2){
		$write_wait = $2;
		$write_wait_2 = $3;
		$write_wait_3 = $4;
	    }
	}
	elsif($lines =~ /=== MPI Processor send (.*?): (.*? .*?) (.*?) (.*?) s/ ){
	    if($op == 1){
		$read_send = $2;
		$read_send_2 = $3;
		$read_send_3 = $4;
	    }
	    elsif($op == 2){
		$write_send = $2;
		$write_send_2 = $3;
		$write_send_3 = $4;
	    }
	}
	elsif($lines =~ /=== MPI Processor receive (.*?): (.*? .*?) (.*?) (.*?) s/ ){
	    if($op == 1){
		$read_recv = $2;
		$read_recv_2 = $3;
		$read_recv_3 = $4;
	    }
	    elsif($op == 2){
		$write_recv = $2;
		$write_recv_2 = $3;
		$write_recv_3 = $4;
	    }
	}
	
    } while ($lines);
    
    # Print data to file
    
    if($sep_flag == 1)
    {
	
      if ($pess_flag == 0)
      {
	print ROFNAME "$procs\t $msize\t $nobj\t $total_read_rate \t$total_read_rate_2 \t$total_read_rate_3 \t$total_read_time \t$total_read_time_2 \t$total_read_time_3 $read_rate \t$read_rate_2 \t$read_rate_3 \t$read_time \t$read_time_2 \t$read_time_3 \t$read_file_open \t$read_file_open_2 \t$read_file_open_3 \t$read_time_fc \t$read_time_fc_2 \t$read_time_fc_3 \t$read_wait \t$read_wait_2 \t$read_wait_3 ";
	print WOFNAME "$procs\t $msize\t $nobj\t $total_write_rate \t$total_write_rate_2 \t$total_write_rate_3 \t$total_write_time \t$total_write_time_2 \t$total_write_time_3 $write_rate \t$write_rate_2 \t$write_rate_3 \t$write_time \t$write_time_2 \t$write_time_3 \t$write_file_open \t$write_file_open_2 \t$write_file_open_3 \t$write_time_fc \t$write_time_fc_2 \t$write_time_fc_3 \t$write_wait \t$write_wait_2 \t$write_wait_3 ";
	
	if($sync_flag){
	    print WOFNAME "\t$write_file_sync \t$write_file_sync_2 \t$write_file_sync_3";
	}
	if($agg_flag){
	    print WOFNAME "\t$write_send \t$write_send_2 \t$write_send_3 \t$write_recv \t$write_recv_2 \t$write_recv_3";
	    print ROFNAME "\t$read_send \t$read_send_2 \t$read_send_3 \t$read_recv \t$read_recv_2 \t$read_recv_3";
	}
      }
      else
      {
	
	print ROFNAME "$procs\t $msize\t $nobj \t$total_read_rate \t$total_read_time_3  \t$read_rate  \t$read_time_3  \t$read_file_open_3  \t$read_time_fc_3  \t$read_wait_3 ";
	print WOFNAME "$procs\t $msize\t $nobj \t$total_write_rate \t$total_write_time_3 \t$write_rate \t$write_time_3  \t$write_file_open_3 \t$write_time_fc_3 \t$write_wait_3 ";
	if($sync_flag){
	    print WOFNAME "\t$write_file_sync_3";
	}
	if($agg_flag){
	    print WOFNAME "\t$write_send_3 \t$write_recv_3";
	    print ROFNAME "\t$read_send_3  \t$read_recv_3";
	}
      }
	
	print ROFNAME "\n";
	print WOFNAME "\n";
	
    }
    else
    {
      if ($pess_flag == 0)
      {
	print OFNAME "$procs \t$msize \t$nobj \t$total_write_rate \t$total_write_rate_2 \t$total_write_rate_3 \t$total_write_time \t$total_write_time_2 \t$total_write_time_3 $write_rate \t$write_rate_2 \t$write_rate_3 \t$write_time \t$write_time_2 \t$write_time_3 \t$write_file_open \t$write_file_open_2 \t$write_file_open_3 \t$write_time_fc \t$write_time_fc_2 \t$write_time_fc_3 \t$write_wait \t$write_wait_2 \t$write_wait_3 ";
	
	if($sync_flag){
	    print OFNAME "\t$write_file_sync \t$write_file_sync_2 \t$write_file_sync_3 ";
	}
	if($agg_flag){
	    print OFNAME "\t$write_send \t$write_send_2 \t$write_send_3 \t$write_recv \t$write_recv_2 \t$write_recv_3 ";
	}
	
	print OFNAME "\t $total_read_rate \t$total_read_rate_2 \t$total_read_rate_3 \t$total_read_time \t$total_read_time_2 \t$total_read_time_3 \t$read_rate \t$read_rate_2 \t$read_rate_3 \t$read_time \t$read_time_2 \t$read_time_3 \t$read_file_open \t$read_file_open_2 \t$read_file_open_3 \t$read_time_fc \t$read_time_fc_2 \t$read_time_fc_3 \t$read_wait \t$read_wait_2 \t$read_wait_3 ";
	
	if($agg_flag){
	    print OFNAME "\t$read_send \t$read_send_2 \t$read_send_3 \t$read_recv \t$read_recv_2 \t$read_recv_3";
	}
	print OFNAME "\n";
      }
      else
      {
        if ($procs_max != $procs || $msize_max != $msize || $nobj_max != $nobj || $besteff_run_only_flag == 0)
        {
	  if ($verbose_flag == 1) { printf "%4d %4d %9.3f %9.3f %7d %7d ####\n",$procs_max,$procs,$msize_max,$msize,$nobj_max,$nobj; }
          if ($procs_max != 0)
          {
            if ($total_write_rate_max != -1 && $total_read_rate_max != -1)
            {
	      printf OFNAME "%d %9.3f %7d %5d %5d %5d %5d %6.2f %6.2f %6.2f ",$procs_max,$msize_max,$nobj_max,$total_write_rate_max,$total_write_time_max,$write_rate_max,$write_time_max,$write_file_open_max,$write_time_fc_max,$write_wait_max; 
              if($sync_flag){ printf OFNAME " %6.2f ",$write_file_sync_max; }
              if($agg_flag) { printf OFNAME " %6.2f %6.2f ",$write_send_max,$write_recv_max; }
              printf OFNAME " r%s",$write_run_max;
              printf OFNAME " %5d %5d %5d %5d %6.2f %6.2f %6.2f ",$total_read_rate_max,$total_read_time_max,$read_rate_max,$read_time_max,$read_file_open_max,$read_time_fc_max,$read_wait_max;
              if($agg_flag){ printf OFNAME " %5d %5d ",$read_send_max,$read_recv_max; }
              printf OFNAME " r%s",$read_run_max;
	      printf OFNAME " %s#\n",$result_file;
            }
          }
          $procs_max            = $procs;
          $msize_max            = $msize;
          $nobj_max             = $nobj;
          $total_write_rate_max = $total_write_rate;
          $total_read_rate_max  = $total_read_rate;
          @_ = split(/\//,$ARGV[$i]);
          chop($_[2]);
          $result_file          = $_[2];
        }
        if ( $total_write_rate_max <= $total_write_rate || $besteff_run_only_flag == 0)
        {
          $total_write_rate_max = $total_write_rate;
          $total_write_time_max = $total_write_time_3;
                $write_rate_max = $write_rate;
                $write_time_max = $write_time_3;
           $write_file_open_max = $write_file_open_3;
           $write_time_fc_max   = $write_time_fc_3;
           $write_wait_max      = $write_wait_3;
           $write_file_sync_max = $write_file_sync_3;
           $write_send_max      = $write_send_3;
           $write_recv_max      = $write_recv_3;
                                  @_ = split(/\//,$ARGV[$i]);
           $write_run_max       = chop($_[2]);
        }
        if ( $total_read_rate_max <= $total_read_rate || $besteff_run_only_flag == 0)
        {
          $total_read_rate_max  = $total_read_rate;
          $total_read_time_max  = $total_read_time_3;
                $read_rate_max  = $read_rate;
                $read_time_max  = $read_time_3;
           $read_file_open_max  = $read_file_open_3;
           $read_time_fc_max    = $read_time_fc_3;
           $read_wait_max       = $read_wait_3;
           $read_send_max       = $read_send_3;
           $read_recv_max       = $read_recv_3;
                                  @_ = split(/\//,$ARGV[$i]);
           $read_run_max        = chop($_[2]);
        }
      }
    }
    
    close DATA;

}
    if ($pess_flag == 1)
    {
      if ($procs_max != 0)
      {
        if ($total_write_rate_max != -1 && $total_read_rate_max != -1)
        {
           printf OFNAME "%d %9.3f %7d %5d %5d %5d %5d %6.2f %6.2f %6.2f ",$procs_max,$msize_max,$nobj_max,$total_write_rate_max,$total_write_time_max,$write_rate_max,$write_time_max,$write_file_open_max,$write_time_fc_max,$write_wait_max; 
           if($sync_flag){ printf OFNAME " %6.2f ",$write_file_sync_max; }
           if($agg_flag) { printf OFNAME " %6.2f %6.2f ",$write_send_max,$write_recv_max; }
  	   printf OFNAME " r%s",$write_run_max;
           printf OFNAME " %5d %5d %5d %5d %6.2f %6.2f %6.2f ",$total_read_rate_max,$total_read_time_max,$read_rate_max,$read_time_max,$read_file_open_max,$read_time_fc_max,$read_wait_max;
           if($agg_flag){ printf OFNAME " %6.2f %6.2f ",$read_send_max,$read_recv_max; }
  	   printf OFNAME " r%s",$read_run_max;
	   printf OFNAME " %s#\n",$result_file;
        }
      }
    }
   printf "%d input files parsed\n", $i-$infile_ndx;
