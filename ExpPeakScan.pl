#!/usr/bin/perl
#Author:Karl Smith
#Date: Dec 1, 2015
#This script scans text files containing table of fit result from the 19F(a,n) 
#experiment at ND using VANDLE.
#
#Usage: ./ExpPeakScan.pl --file1 file1.txt --file2 file2.peaks
#
#The first file specified by --file1 is scanned to form the initial database, 
#the second file specified by --file2 is then scanned replacing anything form 
#file1. The files are scanned by first finding a line containing "Bar \d+", where 
#\d+ is an integer value. Then any line following this with a number are scanned
#and values in the columns matching that in @columnsToCapture are stored. This
#is continued until a line containing anything other than a literal contained 
#in an exponential number or a comma found, the others are considered valid 
#data to be parsed. The output is then dumped 
#to the screen containing the bar number followed by the series of values 
#matching the specified columns and repeating for each line found following "Bar
#\d+".

use strict;
use Getopt::Long;

#The list of column numbers to store starting from 0.
my @columnsToCapture = (1, 3, 4);

#Hash of output data 
my %outData;

#Set default values
my $file1 = '';
my $file2 = '';

#Get long options from the user. Valid options:
# --file1 file.dat //The first fiel to be incorporated
# --file2 file.dat //The second file to be incorporated overwriting data from the first.
# --peak file.peaks //Takes the short peak file type format
# --txt file.txt //Takes the longer txt file type
GetOptions ('file1=s' => \$file1,
		'file2:s' => \$file2);

#Check that the options are valid
my $error = 0;
if (!-f $file1) {
	print STDERR ("ERROR: File 1 specified '$file1' is not a regular file!\n");
	$error = 1;
}
if ($file2 ne "" && !-f $file2) {
	print STDERR ("ERROR: File 2 specified '$file2' is not a regular file!\n");
	$error = 1;
}
#Exit if the options were invalid
if ($error) {exit;}

#Append/Replace the first file to the data storage
UpdateOutputData($file1);

#Append/Replace the second file to the data storage
if ($file2 ne '') {
	UpdateOutputData($file2);
}

#Output the resulting data.
print "Bar x0i Ai dAi\n";
for my $bar (sort {$a <=> $b} keys %outData) {
	my $href = $outData{$bar};
	print "$bar ";
	foreach (sort {$a <=> $b} keys %{$href}) {
		print "@{ $href->{$_} } ";
	}
	print "\n";
}

sub ScanTable {	
	#Make a hash to store the peak information.
	my %peaks;
	my $tableFound = 0;

	#Loop over the array again searching for the peak data
	while (my $line = shift(@_)) {
		#If we find the text BarID then the table has ended.
		if ($tableFound && $line =~ m/[^\d,\.E\+-\s]/) {
			unshift @_, $line;
			last;
		}
		#Otherwise we look for lines with numbers.
		elsif ($line =~ m/(\d+\.\d+\s+)+/) {
			$tableFound = 1;
			#Remove leading and trailing space
			$line =~ s/^\s+|\s+$//g;
			#Split the rest of the line on white space
			my @values = split /\s+/, $line;

			#Create an array of the numbers we want and store them into the hash
			my @data;
			for (@columnsToCapture) {
				push @data, $values[$_];
			}
			#Push array into the hash
			$peaks{$values[0]} = [ @data ];
		}

		#for (keys %peaks) {
		#	print "$_ => @{ $peaks{$_} }\n";
		#}
		#print "$line\n";
	}

	return \%peaks;
}

sub UpdateOutputData {
	#open the TXT file for reading
	open FILE, $_[0] or die $!;

	#Stuff each line of the file into an element of an array.
	my @inData = <FILE>;

	#Loop over the array looking for "Bar \d+"
	while (my $line = shift(@inData)) {
		#Check if line matches specified regular expression
		if ($line=~m/Bar (\d+)/i) {
			#Store the bar number form the regex match
			my $bar = $1;
			
			#Scan the data table
			$outData{$bar} = ScanTable(@inData);
		}
	}

	close FILE;
}

