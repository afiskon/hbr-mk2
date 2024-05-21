#!/usr/bin/env perl

use strict;
use warnings;

my %list;
$list{"---;BN61-202"} = 1;

my $skiplines = 9;
my @flist = <../pcbs/*/hbr-mk2-*.csv>;

my $linere = '^';
for (1..6) {
    $linere .= '"([^"]+)",';
}

for my $fname (@flist) {
    print STDERR "Processing $fname\n";
    open FID, $fname or die "Failed to open $fname: $!\n";
    my $nline = 0;
    while(my $line = <FID>) {
        $nline++;
        next if($nline <= $skiplines);
        my ($nitem, $qty, $refs, $val, $libpart, $fp) = $line =~ $linere;
        if(($val eq "") or ($qty eq "") or ($fp eq "")) {
            die "Error processing: $line\n"
        }
        if($val eq "FT37-43") {
            $fp = "---";
        }
        if($val eq "KT3142A") {
            $val .= " (2N2369A)";
        }
        if($val eq "ADE-6") {
            $val .= " (ADE-1,ADE-2)";
        }
        if($val eq "G6K-2F-Y") {
            $val .= " (HFD4/12)";
        }
        if($val eq "NE5532") {
            $val .= " (TL072)";
        }
        if($val =~ /^1N34/) {
            $val = "1N34 (Д311,Д9Б)";
        }

        $list{"$fp;$val"} += $qty;
    }
    close FID;
}

print "\"Qty\",\"Value\",\"Footprint\"\n";
foreach my $k (sort keys %list) {
    my ($fp, $val) = split ";", $k;
    my $qty = $list{$k};
    print "\"$qty\",\"$val\",\"$fp\"\n";
}
