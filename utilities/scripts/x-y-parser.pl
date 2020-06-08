#!/usr/bin/perl 
###############################################################################
# Name:  x_y_parser
# Description: This script formats data into x y coordinates for plotting 
#              purposes.  The user specifies a column number for x and y 
#              values using the -x and -y arguments.  In the case of netperf,
#              the user may specify either column numbers or string descriptors
#              which are listed via the help option (-h). Based on user input for 
#              the test_type, x coordinate, and y coordinate, the script parses 
#              the data to an output file with two columns and specific headers
#              for use by the cts_diff.pl script
# 
# Author:      Alfred Torrez
# Date:        
# History:     
#              Changed by       Date        Description
#              Alfred Torrez   7/28/05      added rm of tmpfile to read_io_test
#                                           added -m and -n explanations to print_usage
#                                           added additional netperf x/y coordinates
#                                           to print usage
#                "      "      7/29/05      added read_general to handle a general
#                                           test type 
#          
#                "      "      8/5/05       modified format_x_y_data to sort 
#                                           x column
#                                           using associated array (x-key y-value)
#
#                "      "      8/8/05       fixed bug related to spaces or tabs
#                                           before first column (proc count)
#                                           removed redundent "cts" dataset
#                                           label
#
#                "      "      8/10/05      fixed start line/end line bug and
#                                           added capability to select column
#                                           that will serve as legend lines
#                                           in an x/y plot (use -c & -l options)
#
#                "      "     11/1/05       added x and y, min and max limitation
#                                           arguments to analyze data in a 
#                                           smaller range
#
#                "      "     12/15/05      added path argument variable 
###############################################################################

use Getopt::Std;
use strict;


# define the format of test output file to be examined
my %test_params = (

     		       netperf => { "Recv Socket"  => "rx_socket_size",
				    "Send Socket"  => "tx_socket_size",
                                    "Send Message" => "tx_message_size",
				    "Elapsed Time" => "elapsed_time",
			            #"Throughput MBytes/sec"  => "throughput",
			            #"Throughput 10^6bits/s"  => "throughput",
			            "Throughput"  => "throughput",
                                    "Utilization Send" => "tx_utilization",
     				    "Recv remote" => "rx_utilization",
                                    "Service Send" => "service_tx",
				    "Demand Recv" => "demand_rx",
                                  }
                   ); 
my $path = ".";
my $y_lim_low = -1;
my $y_lim_high = -1;
my $x_lim_low = -1;
my $x_lim_high = -1;
###############################################################################
# main 
# main is responsible for reading the command line arguments.  It determines
# the type of test data to format and the columns associated with the desired
# x and y coordinates (x and y are hash keys associated with x and y identifiers
# entered as command line arguments).
# 
###############################################################################
{

 my $parse_type;
 my $x_value;
 my $y_value;
 my $result_file;
 my $line_start = -1;
 my $line_end = -1;
 my $data_offset = 0;
 my $col_def = "names";
 my $data_set_name;
 my $x_axis_label = "x";
 my $y_axis_label = "y";
 my $append = 0;
 my $dataset_name = "";
 my $sort_col = 1;
 my $legend_filename = "_Procs";
  

getopts('t:x:y:f:s:e:k:d:m:n:ac:l:q:r:u:v:p:h') || &print_usage; 

if ($Getopt::Std::opt_h ) {
  &print_usage;
}

if ($Getopt::Std::opt_t ne "") { 
   $parse_type = $Getopt::Std::opt_t;
}
else {
   print "Must specify type of file to parse\n";
   &print_usage;
}
   
if ($Getopt::Std::opt_x ne "") {
   $x_value = $Getopt::Std::opt_x;
}
else {
   print "Must specify x coordinate\n";
   &print_usage;
}

if ($Getopt::Std::opt_y ne "") {
   $y_value = $Getopt::Std::opt_y;
}
else {
   print "Must specify y coordinate\n";
   &print_usage;
}


if ($Getopt::Std::opt_s ne "") {
   $line_start = $Getopt::Std::opt_s;
}

if ($Getopt::Std::opt_e ne "") {
   $line_end = $Getopt::Std::opt_e;
}


if ($Getopt::Std::opt_f ne "") {
  $result_file = $Getopt::Std::opt_f;
}
else {
   print "Must specify result file to parse\n";
   &print_usage;
}

if ($Getopt::Std::opt_k ne "") {
  $data_offset = $Getopt::Std::opt_k;
}

if ($Getopt::Std::opt_d ne "") {
  $dataset_name = $Getopt::Std::opt_d;
}
else {
   print "Must specify dataset name\n";
   &print_usage;
}

if ($Getopt::Std::opt_m ne "") {
  $x_axis_label = $Getopt::Std::opt_m;
}
if ($Getopt::Std::opt_n ne "") {
  $y_axis_label = $Getopt::Std::opt_n;
}

if ($Getopt::Std::opt_a) {
  $append = 1;
}
if ($Getopt::Std::opt_c ne "") {
  $sort_col = $Getopt::Std::opt_c;
}
if ($Getopt::Std::opt_l ne "") {
  $legend_filename = $Getopt::Std::opt_l;
}
if ($Getopt::Std::opt_q ne "") {
  $y_lim_low= $Getopt::Std::opt_q;
}
if ($Getopt::Std::opt_r ne "") {
  $y_lim_high= $Getopt::Std::opt_r;
}
if ($Getopt::Std::opt_u ne "") {
  $x_lim_low= $Getopt::Std::opt_u;
}
if ($Getopt::Std::opt_v ne "") {
  $x_lim_high= $Getopt::Std::opt_v;
}
if ($Getopt::Std::opt_p ne "") {
  $path = $Getopt::Std::opt_p;
}

$path = "$path/";


my $column_x = undef;
my $column_y = undef;
my $column;


# If test type = netperf, use test_params hash to get key for
# associated user input.  The key matches actual netperf
# result output columns

if ($parse_type eq "netperf") {
  if ($x_value =~ (/\d+/) && $y_value =~ (/\d+/)) {
    $col_def = "digits";
    $column_x = $x_value;
    $column_y = $y_value;
  }
  else {



    foreach $column(keys %{$test_params{$parse_type}}) {
  #  print "$test_params{$parse_type}}\n";
      if ($x_value eq $test_params{$parse_type}{$column}) {
#      $column =~ /col_([\d+])/;
        $column_x = $column; 
        last; 
      }
    }
    foreach $column(keys %{$test_params{$parse_type}}) {
      if ($y_value eq $test_params{$parse_type}{$column}) {
#      $column =~ /col_([\d+])/;
        $column_y = $column;
        last; 
      }
    }
  }

if ($column_x eq "undef" || $column_y eq "undef") {
  print "Incorrect specification of x and/or y columns\n";
  exit;
}
}  


# If user has not entered line numbers, default search to lines 1 -
# 100000000 (end of file)
if ($parse_type eq "io_test" || $parse_type eq "general") {

  if ($line_end == -1) {
    $line_end = 100000000;
  }
  if ($line_start == -1) {
    $line_start = 1;
  }
  if ($parse_type eq "io_test") {
# parse io_test data 
    &read_io_test($x_value-1, $y_value-1, $line_start, $line_end, $data_offset, $result_file, $dataset_name, $x_axis_label, $y_axis_label, $append, $sort_col-1, $legend_filename);
  }
  else {
    &read_general($x_value-1, $y_value-1, $line_start, $line_end, $result_file, $dataset_name, $x_axis_label, $y_axis_label, $append);
  }
}

# parse netperf data
elsif ($parse_type eq "netperf") {
  &read_netperf($result_file, $column_x, $column_y, $col_def, $dataset_name, $x_axis_label, $y_axis_label, $append);
}
}

###############################################################################
# sub read_netperf 
# This uses the desired x and y labels (hask keys) passed into it to determine
# the actual column number.  Because netperf results are not always consistent
# in heading descriptors and number of columns, this sub needs to filter and
# determine the number of columns associated with the output
#
###############################################################################
sub read_netperf
{
  my ($netperf_result_file, $column_x, $column_y, $col_type, $data_name, $x_label, $y_label, $append_flag) = @_;

  my ( $line,
       @netperf_numbers,
       @net_array,
       $current_line_columns,
       $needed_columns,
       $i,
       $column_cnt,
       $net_element,
       @netperf_columns,
  );
       my $x_column = -1;
       my $y_column = -1;
 
       my $inputfile = $path.$netperf_result_file;
       my $writetmpfile = $path."wtmpfile";
       my $tmpfile = $path."tmpfile";



  open READ_FILE, "<$inputfile" 
      or die "Cannot open Input File:  $!";
  open WRITE_FILE, ">$writetmpfile"
      or die "Cannot open Write File:  $!";

  my $first_line = 0;

# read netperf file and write data and netperf
# column titles to wtmpfile
  while ($line = <READ_FILE>) {
    # skip blank lines
    if ($line !~ /\S/) {
      next;
    }
    if ($line =~ /^--/ || $line =~ /^TCP/ || $line =~ /\s*TCP/ || $line =~ /\s*\d+\s+segments/) {
      next;
    }
    if ($line =~ /^\d+/) {
      @netperf_numbers = split /\s+/, $line;
      print WRITE_FILE "$line";
      $first_line = 1;
    }
    elsif ($first_line == 0) {
      print WRITE_FILE "$line";
    }
  } 
  close (WRITE_FILE);
  close (READ_FILE);

  if ($col_type eq "digits") { # column numbers specified by user
    `cp wtmpfile tmpfile`;
    &format_x_y_data($column_x-1, $column_y-1, "netperf.out", $data_name, $x_label, $y_label, $append_flag);  
  }
  else { # column names specified by user
    # Because the hash key consists of space seperated words,
    # get each word
    my ($column_x_1, $column_x_2) = (split /\s+/, $column_x)[0,1];
    my ($column_y_1, $column_y_2) = (split /\s+/, $column_y) [0,1];
    # count number of columns containing data
    $column_cnt = scalar(@netperf_numbers);


    open READ_TMP_FILE, "<$writetmpfile" 
      or die "Cannot open Input File:  $!";

    open WRITE_TMP_FILE, ">$tmpfile"
      or die "Cannot open Write File:  $!";


    # netperf headers do not always fill all columns
    # (number of columns containg data)
    # so fill columns with no header with "--"
    while ($line = <READ_TMP_FILE>) {
      if ($line =~ /^\d+/) { print WRITE_TMP_FILE "$line"; }

      @netperf_columns = split /\s+/, $line;
      $current_line_columns = scalar(@netperf_columns);
#      print "$current_line_columns\n";
      $needed_columns = $column_cnt - $current_line_columns;
#      print "$needed_columns\n";
      if ($needed_columns > 0) {
        for ($i=0; $i < $needed_columns; $i++) { push(@netperf_columns, "-"); }
        push @net_array, @netperf_columns; 
      }
      else {
        push @net_array, @netperf_columns; 
      }
    }
    close (READ_TMP_FILE);
    close (WRITE_FILE);
    close (WRITE_TMP_FILE);

#    print "@net_array\n";
#    print "$column_x_1  $column_x_2\n";
#    print "$column_y_1  $column_y_2\n";
#    print "$column_cnt\n";


    $i=0;
    # match key elements to output headers to find column number
    foreach $net_element(@net_array) {
      if ($column_x_1 eq "Throughput" && $column_x_1 eq $net_element) {
        $x_column = $i;
        last;
      }
      else {
        if ($column_x_1 eq $net_element  && $column_x_2 eq $net_array[$i+$column_cnt]) {
          $x_column = $i;
          last;
        }
      }
      $i++
    }



    $i=0;
    foreach $net_element(@net_array) {
      if ($column_y_1 eq "Throughput" && $column_y_1 eq $net_element) {
        $y_column = $i;
        last;
      }
      else {
        if ($column_y_1 eq $net_element  && $column_y_2 eq $net_array[$i+$column_cnt]) {
          $y_column = $i;
          last;
        }
      }
      $i++
    }   

    # net_array is 1 dimensional so compute next lines for columns
    $x_column %= $column_cnt;
    $y_column %= $column_cnt;
#    print "$x_column   $y_column\n";
    &format_x_y_data($x_column, $y_column, "netperf.out", $data_name, $x_label, $y_label, $append_flag);  
     `rm wtmpfile`;
     `rm tmpfile`;
  } # end else col_type
}

###############################################################################
# sub read_general
# 
# This sub reads results from a general testoutput composed of just columns
# of numbers; no headears.  The output is written to the input file name appended
# with the .out extension.
#
###############################################################################
sub read_general
{
   my ($column_x, $column_y, $start_line, $end_line,  $read_file, $data_name, $x_label, $y_label, $append_flag) = @_;

   my $linecnt = 1;
   my $inputfile = $path.$read_file;
   my $tmpfile = $path."tmpfile";
   my $line;

   open READ_RESULT_FILE, "<$inputfile" 
    or die "Cannot open Input File:  $!";
   open WRITE_FILE, ">$tmpfile"
    or die "Cannot open Output File:  $!";

   while ($line = <READ_RESULT_FILE>) {
      if ($linecnt >= $start_line && $linecnt <= $end_line) {
        print WRITE_FILE $line;  
      }
      elsif ($linecnt > $end_line) {  # no need to keep reading lines out of file
        last;
      }
      $linecnt++;
   }
   close (WRITE_FILE);
   close (READ_RESULT_FILE);
   &format_x_y_data($column_x, $column_y, $inputfile.".out", $data_name, $x_label, $y_label, $append_flag);  
}
###############################################################################
# sub read_io_test
# This sub reads the results of I/O tests and generates a temporary file
# containing the data to be parsed.  This file is then parsed and data is 
# written to individual files that contain the x/y data points for that
# particular set of parameters.  The output  file names and column to 
# use (# of procs or message size) are specified
# with the -l and -c argumenets respectively.  -x an -y arguments are used
# to specify the columns for plotting while the -c argument is used to 
# specify the lines that make up the legend for the x/y plot.  
# The I/O result fed to this program must be free of headers.
#
# format_x_y_data is called to retrieve the desired columns and write the 
# data to the individual file described above.
#
###############################################################################
sub read_io_test
{

   my ($column_x, $column_y, $start_line, $end_line, $data_offset, $read_file, $data_name, $x_label, $y_label, $append_flag, $col_to_sort, $part_file) = @_;

  my $line;
  my $linecnt = 1;
  my @line_array;
  my $col_value;
  my $last_col_value = 2;
  my $first_line = 1;
  my $int_line_cnt = 0;
  my $file_written = 0;
  my $last_line;
  my @data_lines;
  my @sorted_lines;
  my @pri;
  my @sec;

  my $inputfile = $path.$read_file;

  my $tmpfile = $path."tmpfile";
  my $sorted_tmpfile = $path."sorted_tmpfile";

  open READ_RESULT_FILE, "<$inputfile" 
    or die "Cannot open Input File:  $!";
  open WRITE_FILE, ">$tmpfile"
      or die "Cannot open Output File:  $!";
  open SORTED_TMP_FILE, ">$sorted_tmpfile"
      or die "Cannot open Output File:  $!";



# Based on user input, determine what column needs to be 
# examined for change so that individual files may be constructed
# i.e.  2_Procs, 8_Procs..... 
# This allows cts_diff to plot with each file representing
# a line on a plot (each line in legend is a seperate file)
#
# Place each line of input file into data_lines array
  while (<READ_RESULT_FILE>) {
    push (@data_lines, $_);
  }

# sort desired column.  Data is not guaranteed to be sorted in columns 
# sorted column retains line information
  @sorted_lines = sort {@pri=(split'\s+', $a); @sec=(split'\s+', $b); $pri[$col_to_sort] <=> $sec[$col_to_sort];} @data_lines;

# write softed data to sorted temp file
  print SORTED_TMP_FILE @sorted_lines;
  close (SORTED_TMP_FILE);
  close (READ_RESULT_FILE);
  
#open sorted_tmpfile for reading - the next stage will determine how to 
#write output files
  open RD_SORTED_FILE, "<$sorted_tmpfile"  
    or die "Cannot open Sorted File:  $!";


  #Read the result file and examine column value  to determine
  #when the next file should be written for format_x_y_data
  #also keep track of line count if the user has entered 
  #starting and ending lines
   while ($line = <RD_SORTED_FILE>) {
    if ($line =~ /^\s+|^\t+/) { #determine if first column has preceding spaces or tabs
       $line =~ s/^[\s\t]*//;
    }

    @line_array = split /\s+/, $line;
    $col_value = $line_array[$col_to_sort];    

    if ($data_offset == 0) {

      if ($first_line == 1) {
        $first_line = 0;
        $last_line = $line;
        $last_col_value = $col_value;
        next;
      }
      if ($linecnt >= $start_line && $linecnt <= $end_line) {
        print WRITE_FILE $last_line;  
      }
      $linecnt++;
      if ($linecnt > $end_line-1) {
        last;
      }

      if ($col_value != $last_col_value) {
        # close write file so that format_x_y_data can read it
        close (WRITE_FILE);
        &format_x_y_data($column_x, $column_y, $last_col_value.$part_file, $data_name, $x_label, $y_label, $append_flag);  

        # open write file again for next chunk of data
        open WRITE_FILE, ">$tmpfile"
          or die "Cannot open Output File:  $!";
      }
      $last_line = $line;
      $last_col_value = $col_value;

    } # end if data_offset == 0

    # similar to above but now only care about specific number of lines
    # per dataset
    else { #data_offset != 0
      if ($first_line == 1) {
        $first_line = 0;
        $last_col_value = $col_value;
      }
      if ($last_col_value == $col_value && $file_written == 0) {
        if ($int_line_cnt < $data_offset) {
          print WRITE_FILE $line;  
          $int_line_cnt++;
          $last_col_value = $col_value;
        }
        if ($int_line_cnt == $data_offset) { 
          close (WRITE_FILE);
          &format_x_y_data($column_x, $column_y, $last_col_value.$part_file, $data_name, $x_label, $y_label, $append_flag);  
          # open write file again for next chunk of data
          open WRITE_FILE, ">$tmpfile"
            or die "Cannot open Output File:  $!";
          $file_written = 1;
        }
      }
      else {
         if ($last_col_value != $col_value) {
            print WRITE_FILE $line;  
            $int_line_cnt = 1;
            $last_col_value = $col_value;
            $file_written = 0;
         }
      }
    }
  }  # end while
# at end of file so write the last line and call format_x_y_data
  if ($data_offset == 0 ) {
     print WRITE_FILE $last_line;
     close (WRITE_FILE);
    &format_x_y_data($column_x, $column_y, $last_col_value.$part_file, $data_name, $x_label, $y_label, $append_flag);  
  }
  else {
    close (WRITE_FILE);
    close (RD_SORTED_FILE);
  }

  `rm $tmpfile`;
  `rm $sorted_tmpfile`;
}    

###############################################################################
# sub format_x_y_data
# This subroutine retrieves the x and y columns from tmpfile and writes the
# result to the output file passed in as an argument 
#
###############################################################################
sub format_x_y_data
{
  my ($x_column, $y_column, $output_file, $data_set_name, $x_label, $y_label, $append) = @_;
  my $tmpfile = $path."tmpfile";
  my $outfile = $path.$output_file;
  my $x_var;
  my $key;

  open READ_FILE, "<$tmpfile" 
    or die "Cannot open Input File:  $!";

  my @col_x = map({my @line = split/\s+/; $line[$x_column];} <READ_FILE>); 

  seek READ_FILE, 0, 0;

  my @col_y = map({my @line = split/\s+/; $line[$y_column];} <READ_FILE>); 

  close (READ_FILE);

  if ($col_x[0] eq "" || $col_y[0] eq "") {
     print "Error:  Column Selection(s) invalid\n";
  }


  if (-e $outfile && $append == 1) { 
    open OUT_FILE, ">>$outfile";
  }
  else {
    open OUT_FILE, ">$outfile";
    print OUT_FILE "# cts\n";
  }

  if (scalar(@col_x) != scalar(@col_y)) { 
    print "Column lengths not equal -- missing data?\n";
  }

  my @filtx;
  my @filty;
  my $x_array_ref;
  my $y_array_ref;
 
  my $orig_index = 0;
  my $filt_index = 0;


  if (($y_lim_low == -1 && $y_lim_high == -1) && (
       $x_lim_low == -1 && $x_lim_high == -1)) {

     $x_array_ref = \@col_x;
     $y_array_ref = \@col_y;

  }
  # If the user has select limits for x and y ranges, this 
  # code is executed.
  else {


     print "$x_lim_high   $x_lim_low\n";

     if ($x_lim_high != -1 || $x_lim_low != -1) {
        foreach my $xtst (@col_x) {
           if ($xtst > $x_lim_high || $xtst < $x_lim_low ) {
               $col_x[$orig_index] = "";
               $col_y[$orig_index] = "";
           }
           $orig_index++;
        }
        $filt_index = 0;
        $orig_index = 0;
        foreach my $xval ( @col_x ) {
           
            if ($xval != "") {
              $filtx[$filt_index] = $xval;
              $filty[$filt_index] = $col_y[$orig_index];      
              $filt_index++;
            }
            $orig_index++;
        }
      
         @col_x = @filtx;
         @col_y = @filty;
      }

      if ($y_lim_high != -1 || $y_lim_low != -1) {
         $filt_index = 0;
         $orig_index = 0;

         print "@col_y\n";
         @filtx = ();
	 @filty = (); 

         foreach my $ytst (@col_y) {
            if ($ytst > $y_lim_high || $ytst < $y_lim_low ) {
               $col_y[$orig_index] = "";
               $col_x[$orig_index] = ""; 
            }
            $orig_index++;
         }
         $filt_index = 0;
         $orig_index = 0;
         print "xyy @col_y\n";
         foreach my $yval ( @col_y ) {
           
            if ($yval != "") {
              $filty[$filt_index] = $yval;
              $filtx[$filt_index] = $col_x[$orig_index];      
              $filt_index++;
            }
            $orig_index++;
        }
      }  

#      @col_x = @filtx;
#      @col_y = @filty;

      $x_array_ref = \@filtx;
      $y_array_ref = \@filty;
  }




  # make hash with x column serving as keys and y values
  my $i = 0;
  my %xy_hash;
  foreach $x_var (@{$x_array_ref}) {
     $x_var = $x_var . "a" . $i;
     $xy_hash{$x_var} = shift(@{$y_array_ref});
     $i++;
  }

  print OUT_FILE "# Dataset Name: $data_set_name\n";
  print OUT_FILE "# Coord Name X: $x_label\n";
  print OUT_FILE "# Coord Name Y: $y_label\n";
  foreach $key (sort {$a <=> $b} keys %xy_hash) {
    $key =~ /(\d*.[0-9]*)a\d+/;
    print OUT_FILE "$1 $xy_hash{$key}\n";
  }

  close (OUT_FILE);
}
sub print_usage
{
  print "Usage:  x_y_parser.pl -t[netperf|io_test|general] -x[x_coordinate_name|column_number]\n";
  print "                      -y[y_coordinate_name|column_number] -f[input_file]\n";
  print "                      -s[start_line] -e[end_line] -d[dataset_name]\n";
  print "                      -k[offset] -m[x_axis_label] -n[y_axis_label]\n";
  print "                      -a   -c[column number]  -l[legend file name]\n";
  print "                      -p[path]\n";
  print "\n";
  print "             Options s, e, k, m, and n are optional\n";
  print "             Options c and l are used together for io test data\n";
  print "\n";
  print "Where\n";
  print "  -h display this info\n";
  print "  -t type of test result data\n";
  print "  -x x column data to parse from file. See below for choices\n";
  print "  -y y column data to parse from file. See below for choices\n";
  print "  -s starting line for retrieving data from input file\n";
  print "  -e ending line for retrieving data from input file\n";  
  print "  -f input file containg data to parse\n";
  print "  -k data offset - pick first n lines from each dataset\n";
  print "  -d dataset name - used to label plot\n";
  print "  -m x axis name - used to label plot.\n";
  print "  -n y axis name - used to label plot.\n";
  print "       NOTE:  for x and y labels, use quotes (\" \") around label\n";
  print "              title to preserve spaces and or special characters\n";
  print "              example  -m\"Time (secs)\" -n\"BW (MB/sec)\"\n";
  print "  -a append data to output_file (useful for iotests)\n";
  print "     this creates multiple datasets per file\n";
  print "  -c specify column to use as lines for legend (i.e proc count, message size)\n";
  print "  -l output file identifier i.e. -l_Procs or -l_MBs The program will \n";
  print "     concatanate the specific column number to the identifier to \n";
  print "     name the file i.e. 2_Procs, 8_Procs......\n";
  print "  -p specify a path to data\n";
  print "  -q y_lim_lowi - specify low limit for y data - use with -r option \n";
  print "  -r y_lim_high - specify high limit for y data -use with -q option\n";
  print "  -u x_lim_low - specify low limit for x data - use with -v option\n";
  print "  -v x_lim_high - specify high limit for x data - use with -u option\n";
  print "\n";
  print "X and Y choices for io_test:\n";
  print "\n";
  print " x and y are single integers specifying a column number\n";
  print "\n";
  print "X and Y choices for netperf:\n";
  print " rx_socket_size\n";
  print " tx_socket_size\n";
  print " tx_message_size\n";
  print " elapsed_time\n";
  print " throughput\n";
  print " tx_utilization\n";
  print " rx_utilization\n";
  print " service_tx\n";
  print " demand_rx\n";
  exit 0;
}
           
