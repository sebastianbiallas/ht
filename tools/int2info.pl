#!/usr/bin/perl
#
#	int2info.pl
#	converts Ralf Brown's Interrupt List to TexInfo files
#
#	Copyright (C) 2002 Stefan Weyergraf
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#	This program is freeware, which means that you may distribute
#	it as long as this disclaimer and the copyright notice remain
#	unmodified.
#

$max_entries_per_file = 512;

$infoprefix = "";
$infosuffix = ".info";

%int_keys = ();
%table_keys = ();
%memmap_keys = ();
%msr_keys = ();
%port_keys = ();
%farcall_keys = ();

%nodes = ();
%descs = ();
%files = ();
%file_contents = ();
%file_entries = ();

$index_file = "intidx";
$index_file = $infoprefix.$index_file.$infosuffix;

while (<>) {
again:
	if (/^--------(.*)/) {
		$_ = $1;
		$_ =~ s/\r//g;
		if (/^\!/) {
		} elsif (/^(.-[0-9a-fA-F].*?)\-*$/) {
			consume_int($1);
			goto again;
		} elsif (/^(.-[Mm].*?)\-*$/) {
			consume_mem($1);
			goto again;
		} elsif (/^(.-S.*?)\-*$/) {
			consume_msr($1);
			goto again;
		} elsif (/^(.-P.*?)\-*$/) {
			consume_port($1);
			goto again;
		} elsif (/^(.-\@.*?)\-*$/) {
			consume_farcall($1);
			goto again;
		} elsif (/^---/) {
			# ignore
		} else {
			print STDERR "unmatched: $_\n";
			last;
		}
	}
}

partition_all();
gen_index();
gen_all();

foreach $f (keys %file_contents) {
	open(A, ">$f");
	print A $file_contents{$f};
	close(A);
}
exit;

sub write_file
{
	my ($f, $s) = @_;
	$file_contents{$f} .= $s;
}

sub partition_all
{
	partition(\%int_keys, \&compare_wo_section, "int");
	partition(\%table_keys, \&compare, "table");
	partition(\%memmap_keys, \&compare_wo_section, "memmap");
	partition(\%msr_keys, \&compare_wo_section, "msr");
	partition(\%port_keys, \&compare_wo_section, "port");
	partition(\%farcall_keys, \&compare_wo_section, "farcall");
}

sub partition
{
	my ($kref, $cref, $file) = @_;
	my $f = $infoprefix.$file.$infosuffix;
	my $i = 1;
	foreach $key (sort {&$cref} keys %$kref) {
		while ($file_entries{$f} >= $max_entries_per_file-1) {
			$f = sprintf("%s%s%d%s", $infoprefix, $file, $i++, $infosuffix);
		}
		$files{$key} = $f;
		$file_entries{$f}++;
	}
}

sub gen_idx_section
{
	my ($index_file, $file, $desc, $cref, $kref) = @_;
	$file = $infoprefix.$file.$infosuffix;
	write_file($index_file, gen_xref($index_file, $desc, $file, $desc));
	write_file($file, gen_header($file, $desc, "(dir)", "(dir)"));
	write_file($file, "* Menu:\n\n");
	foreach $key (sort { &$cref; } keys %$kref) {
		write_file($file, gen_xref($file, $descs{$key}, $files{$key}, $key));
	}
}

sub gen_index
{
	write_file($index_file, gen_header($index_file, "Top", "(dir)", "(dir)"));
	write_file($index_file, "* Menu:\n\n");

	gen_idx_section($index_file, "ibcidx", "Interrupts By Category", \&compare, \%int_keys);
	gen_idx_section($index_file, "ibnidx", "Interrupts By Number", \&compare_wo_section, \%int_keys);
	gen_idx_section($index_file, "tabidx", "Tables", \&compare, \%table_keys);
	gen_idx_section($index_file, "memidx", "Memory Maps", \&compare_wo_section, \%memmap_keys);
	gen_idx_section($index_file, "portidx", "Ports", \&compare_wo_section, \%port_keys);
	gen_idx_section($index_file, "msridx", "MSRs", \&compare_wo_section, \%msr_keys);
	gen_idx_section($index_file, "fcidx", "Far Calls", \&compare_wo_section, \%farcall_keys);
}

sub gen_all
{
	foreach $key (keys %all) {
		$gthisfile = $files{$key};
		gen_section($files{$key}, $key);
	}
}

sub gen_section
{
	my ($file, $key) = @_;
	write_file($file, gen_header($file, $key, "(Dir)", "(Dir)"));
	if ($int_keys{$key} && ($key =~ /^.-(..).*/)) {
		write_file($file, genlinks($all{$key}, $1));
	} else {
		write_file($file, genlinks($all{$key}));
	}		
}

sub membyaddr
{
	my ($a1, $b1) = ($a, $b);
	$a1=~s/.-(.*)/$1/;
	$b1=~s/.-(.*)/$1/;
	return $a1 cmp $b1;
}

sub compare_wo_section
{
	my ($a1, $b1) = ($a, $b);
	$a1=~s/.-(.*)/$1/;
	$b1=~s/.-(.*)/$1/;
	return $a1 cmp $b1;
}

sub compare
{
	return $a cmp $b;
}

sub gen_xref
{
	my ($thisfile, $desc, $file, $node) = @_;
	if ($thisfile eq $file) {
		if ($desc eq $node) {
			return "* ".$desc."\::\n"
		} else {
			return "* ".$desc."\: $node.\n"
		}
	} else {
		return "* ".$desc."\: ($file)$node.\n"
	}
}
	
sub gen_xref2
{
	my ($thisfile, $desc, $file, $node) = @_;
	if ($thisfile eq $file) {
		if ($desc eq $node) {
			return "*Note ".$desc."\::"
		} else {
			return "*Note ".$desc."\: $node."
		}
	} else {
		return "*Note ".$desc."\: ($file)$node."
	}
}
	
sub gen_header
{
	my ($f, $n, $p, $u)=@_;
	return "\x1f\nFile:$f,Node:$n,Prev:$p,Up:$u\n";
}

sub genmemlink1616
{
	my ($desc, $id) = @_;
	$cs = catsearch(\%memmap_keys, $desc, "-M".$id);
	if ($cs == 42) {
		return $desc;
	}		
	return $cs;
}

sub genfarcalllink
{
	my ($desc, $id) = @_;
	$cs = catsearch(\%farcall_keys, $desc, "-@".$id);
	if ($cs == 42) {
		return $desc;
	}		
	return $cs;
}

sub genmemlink32
{
	my ($desc, $id) = @_;
	$cs = catsearch(\%memmap_keys, $desc, "-m".$id);
	if ($cs == 42) {
		return $desc;
	}		
	return $cs;
}

sub genmsrlink
{
	my ($desc, $id) = @_;
	$cs = catsearch(\%msr_keys, $desc, "-S".$id);
	if ($cs == 42) {
		return $desc;
	}		
	return $cs;
}

sub genportlink
{
	my ($desc, $id) = @_;
	$cs = catsearch(\%port_keys, $desc, "-P".$id);
	if ($cs == 42) {
		return $desc;
	}		
	return $cs;
}

sub gentablelink
{
	my ($id) = @_;
	return gen_xref2($gthisfile, $id, $files{$id}, $id);
}

sub genintlink
{
	my ($k, $int) = @_;
	if ($k =~ /^\*note/i) { return $k; }
	my $os = $k;
	my $matched = 0;
	$k =~ s/(".*")$//;
	my $ax = "----";
	my $extra_name = "", $extra_value = "";
	while (1) {
		if ($k =~ s/INT ([0-9a-fA-F]{2})h?//) {
			$int = $1;
		} elsif ($k =~ s/AH=([0-9a-fA-F]{2})//) {
			$l = $1;
			$ax =~ /(.{2})(.{2})/;
			$ax = $l.$2;
		} elsif ($k =~ s/AL=([0-9a-fA-F]{2})//) {
			$l = $1;
			$ax =~ /(.{2})(.{2})/;
			$ax = $1.$l;
		} elsif ($k =~ s/AX=([0-9a-fA-F]{4})//) {
			$ax = $1;
		} elsif ($k =~ s/([^A].)=([0-9a-fA-F]{2,4})//) {
			$extra_name=$1;
			$extra_value=$2;
		} else {
			last;
		}
		$matched = 1;
	}
	if (!$matched) { return $os; }
	my $extra = $extra_name.$extra_value;
	my $search = sprintf("%s-%2s%4s%s", '*', $int, $ax, $extra);
	$search =~ s/^(.*?)\-*$/$1/;
	$cs = catsearch(\%int_keys, $os, sprintf("-%2s%4s%s", $int, $ax, $extra));
	if ($cs == 42) {
		return $os;
	} else {
		return $cs;
	}		
}

sub catsearch
{
	my ($href, $desc, $suffix) = @_;
	$desc =~ s/:/./g;
	foreach $c ('a'..'z') {
		my $search = $c.$suffix;
		$search =~ s/^(.*?)\-*$/$1/;
		if (defined $$href{$search}) {
			return gen_xref2($gthisfile, $desc, $files{$search}, $search);
		}
	}
	foreach $c ('A'..'Z') {
		my $search = $c.$suffix;
		$search =~ s/^(.*?)\-*$/$1/;
		if (defined $$href{$search}) {
			return gen_xref2($gthisfile, $desc, $files{$search}, $search);
		}
	}
	$c = '-';
		my $search = $c.$suffix;
		$search =~ s/^(.*?)\-*$/$1/;
		if (defined $$href{$search}) {
			return gen_xref2($gthisfile, $desc, $files{$search}, $search);
		}
	$c = '*';
		my $search = $c.$suffix;
		$search =~ s/^(.*?)\-*$/$1/;
		if (defined $$href{$search}) {
			return gen_xref2($gthisfile, $desc, $files{$search}, $search);
		}
	return 42;
}

sub genlinks2
{
	my ($s, $int) = @_;
	my @t = split(/,/, $s);
	my $r = "", $p = "";
	foreach $k (@t) {
		if ($k =~ s/(".*")$//) {
			   $p = $1;
		}
		if ($k =~ s/#(.\d{4})/gentablelink($1)/ge) {
		} elsif ($k =~ s/MEM (.{4})h?:(.{4})h?/genmemlink1616($k, $1.$2)/ge) {
		} elsif ($k =~ s/MEM (.{8})h?/genmemlink32($k, $1)/ge) {
		} elsif ($k =~ s/MSR (.{8})h?/genmsrlink($k, $1)/ge) {
		} elsif ($k =~ s/PORT (.{4})h?-(.{4})h?/genportlink($k, $1.$2)/ge) {
		} elsif ($k =~ s/PORT (.{4})h?/genportlink($k, $1)/ge) {
		} elsif ($k =~ s/@(.{4})h?:(.{4})h?/genfarcalllink($k, $1.$2)/ge) {
		} elsif ($k =~ s/([^,]+)/genintlink($1, $int)/ge) {
		}
		$r .= $k.$p.',';
	}
	chop $r;
	return $r;
}

sub genlinks
{
	my ($s, $int) = @_;
	$s =~ s/(\(see (also )?)(.*)(\))/$1.genlinks2($3, $int).$4/ge;
	$s =~ s/(SeeAlso: )(.*)/$1.genlinks2($2, $int)/ge;
	return $s;
}

sub consume_int
{
	my ($id) = @_;
	my $b = consume_body();
	$int_keys{$id} = 1;
	$all{$id} = $b;
	my $c = $b;
	if ($c =~ /^(.*?)\n/) { $c = $1; }
	$c = sprintf("%-14s %s", $id, $c);
	$c =~ s/:/./g;
	$c =~ s/\n//g;
	$descs{$id} = $c;
}

sub consume_mem
{
	my ($id) = @_;
	my $b = consume_body();
	$memmap_keys{$id} = 1;
	$all{$id} = $b;
	my $c = $b;
	if ($c =~ /^(.*?)\n/) { $c = $1; }
	$c = sprintf("%-14s %s", $id, $c);
	$c =~ s/:/./g;
	$c =~ s/\n//g;
	$descs{$id} = $c;
}

sub consume_msr
{
	my ($id) = @_;
	my $b = consume_body();
	$msr_keys{$id} = 1;
	$all{$id} = $b;
	my $c = $b;
	if ($c =~ /^(.*?)\n/) { $c = $1; }
	$c = sprintf("%-14s %s", $id, $c);
	$c =~ s/:/./g;
	$c =~ s/\n//g;
	$descs{$id} = $c;
}

sub consume_port
{
	my ($id) = @_;
	my $b = consume_body();
	$port_keys{$id} = 1;
	$all{$id} = $b;
	my $c = $b;
	if ($c =~ /^(.*?)\n/) { $c = $1; }
	$c = sprintf("%-14s %s", $id, $c);
	$c =~ s/:/./g;
	$c =~ s/\n//g;
	$descs{$id} = $c;
}

sub consume_farcall
{
	my ($id) = @_;
	my $b = consume_body();
	$farcall_keys{$id} = 1;
	$all{$id} = $b;
	my $c = $b;
	if ($c =~ /^(.*?)\n/) { $c = $1; }
	$c = sprintf("%-14s %s", $id, $c);
	$c =~ s/:/./g;
	$c =~ s/\n//g;
	$descs{$id} = $c;
}

sub consume_table
{
	my ($id, $prefix) = @_;
	$table_keys{$id} = 1;
	my $b = consume_body();
	my $c = $prefix.$b;
	$all{$id} = "Table $id\n".$c;
	if ($c =~ /^(.*?)\n/) {
		$c = $1;
	} else { 
		$c = "";
	}
	$c = "$id ".$c;
	$c =~ s/:/./g;
	$c =~ s/\n//g;
	$descs{$id} = $c;
}

sub consume_body()
{
	my $s="";
	while (<>) {
		$_ =~ s/\r//g;
		last if (/^--------/);
		if (/^\(Table (.\d*)\)/) {
			my $id = $1;
			my $x = <>;
			chomp $x;
			$s .= "$x (see #$id)\n";
			consume_table($id, "$x.\n");
			last;
		} elsif (/\(Table (.\d*)\)$/) {
			my $id = $1;
			$s =~ s/\n(.*?)$//;
			my $x = <>;
			chomp $x;
			$s .= "$x (see #$id)\n";
			consume_table($id, $1."\n$x\n");
			last;
		}
		$s.=$_;
	}
	return $s;
}
