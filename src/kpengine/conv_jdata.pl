#!/usr/bin/perl -w

@chars = ();
$line = 0;

binmode STDOUT;

while (<>) {
    $line++;

    next if !/^\s*\"/;
    $data = eval $_;
    if (!defined $data) {
	die "Could not parse line $line: $!";
    }
    $strokecount = ord(substr($data,0,1)) - ord('A') + 1;

    if (!defined $chars[$strokecount]) {
	$chars[$strokecount] = "";
    }

    $chars[$strokecount] .= substr($data,1,-1);
}

for (0..$#chars) {
    if (defined $chars[$_]) {
	print pack("NN",$_,length($chars[$_])+1);
	print $chars[$_];
	print "\0";
    }
}
print pack("NN",0,0);
