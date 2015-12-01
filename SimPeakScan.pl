#!/usr/bin/perl
#Author: Karl Smith
#Date: Dec 1, 2015
#This script scans text files containing a table of simulation results for the
# 19F(a,n) experiment at ND using VANDLE.
#
#Usage: ./SimPeakScan.pl file1 [file2] [file3] ...
#
#This script will scan each file and append the output data with the simulation 
#data provided. The data is expected in the following comma separated format:
#BeamEnergy, ExcitedState, BarNum, Counts(perRx), Uncertainty(%)
#All lines containing anything other than a literla contained in an exponential
#number or a comma is ignored, the others are considered valid data to be parsed.
#The output is given as bar number followed by a set of data for each excited 
#state containing the following: Excited State, Counts(perRx), Uncertainty(%).
#The values for a single bar are contained in a single line.

use strict;

#Get list of files
my @files = @ARGV;
for (@files) {
	#Check that each file exists.
	if (!-f $_) {
		print STDERR "ERROR: One of the files specifed '$_' is not a regular file!\n";
		exit;
	}
}

#Create a hash to store the data
my %outData;

#Loop over all files and scan them individually
for (@files) {
	ScanFile($_);
}

#Output the resulting data.
print "Bar ExcitedStatei Counts(perRx)i Uncertainty(%)i\n";
for my $bar (sort {$a <=> $b} keys %outData) {
	my $href = $outData{$bar};
	print "$bar ";
	foreach (sort {$a <=> $b} keys %{$href}) {
		print "@{ $href->{$_} } ";
	}
	print "\n";
}

sub ScanFile {
	open FILE, $_[0] or die $!;

	while (<FILE>) {
		#We ignore any line contating anything execpt ([^]) a digit (\d), comma, 
		# period (\.), capital E, plus sign (\+), minus sign, or any type of 
		# white space (\s).
		if ($_ !=~ m/[^\d,\.E\+-\s]/) {
			#Remove leading and trailing space
			$_ =~ s/^\s+|\s+$//g;
			#Separate the values by the comma delimiter.
			my @values = split /,/, $_;

			my $bar = $values[2];
			my $state = $values[1];

			#Create an array of the values to store.
			${$outData{$bar}}{$state} = [ $values[1], $values[3], $values[4] ];

		}
	}

	close FILE;
}
