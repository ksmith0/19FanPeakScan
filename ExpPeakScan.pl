#!/usr/bin/perl
#Author:Karl Smith
#Date: Dec 1, 2015
#This script scans text files contatining table of fit result from the 19F(a,n) experiment at ND using VANDLE.
#

use Getopt::Long;

#The list of column numbers to store starting from 0.
my @columnsToCapture = (1, 3, 4);

#Hash of output data 
my %outData;

sub ScanTable {	
	#Make a hash to store the peak information.
	my %peaks;

	#Loop over the array again searching for the peak data
	while (my $line = shift(@_)) {
		#If we find the text BarID then the table has ended.
		if ($line =~ m/Bar/) {
			unshift @_, $line;
			last;
		}
		#Otherwise we look for lines with numbers.
		elsif ($line =~ m/(\d+\.\d+\s+)+/) {
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

	#Stuff each line of the txtfile into an element of an array.
	my @inData = <FILE>;

	#Loop over the array looking for "Bar \d+"
	while (my $line = shift(@inData)) {
		#Check if line matches specified regular expression
		if ($line=~m/Bar (\d+)/) {
			#Store the bar number form the regex match
			my $bar = $1;
			
			#Scan the data table
			$outData{$bar} = ScanTable(@inData);
		}
	}

	close FILE;
}

############# MAIN PROGRAM ############################

#Set default values
my $peakFile = '';
my $txtFile = '';
my $outputFile = '';

#Get long options from the user. Valid options:
# --peak file.peaks //Takes the short peak file type format
# --txt file.txt //Takes the longer txt file type
# --out output.dat //Specifies the output file name
GetOptions ('peak=s' => \$peakFile,
		'txt=s' => \$txtFile,
		'out=s' => \$outputFile);

#Check that the options are valid
if ($outputFile eq '') {
	print STDERR ("ERROR: No output file specified!\n");
	$error = true;
}
if (!-f $peakFile) {
	print STDERR ("ERROR: Peak file specified '$peakFile' is not a regular file!\n");
	$error = true;
}
if (!-f $txtFile) {
	print STDERR ("ERROR: TXT file specified '$txtFile' is not a regular file!\n");
	$error = true;
}
#Exit if the options were invalid
if ($error) {exit;}

#Append/Replace the txt file to the data storage
UpdateOutputData($txtFile);

#Append/Replace the peak file to the data storage
UpdateOutputData($peakFile);

#Output the resulting data.
print "Bar x0i Ai dAi\n";
for $bar (keys %outData) {
	my $href = $outData{$bar};
	print "$bar ";
	foreach (keys %{$href}) {
		print "@{ $href->{$_} } ";
	}
	print "\n";
}
