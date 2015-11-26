#!/usr/bin/perl

use warnings;
use strict;
use MinosOne::SwFile 'blessed';

package MinosOne::SwPackage;

my $root_dir = '/';
my $sw_package {
    package_name => undef,
    package_version => undef,
    tarball_path => undef,
    sw_file => undef,
};

bless $sw_package, "SwPackage";

sub new {
    my $class = shift;
    my $self = {@_};
    
    bless ($self, $class);
    
    return($self);
}

sub scan_root {
    my $self = shift;

    scan_dir($self, $root_dir);
}

sub scan_dir {
    my $self = shift;
    my $path = shift;

    opendir DH, $path;

    while($_ = readdir(DH)){
	next if $_ eq "." or $_ eq "..";

	push($self->sw_file, SwFile::new("SwFile",
					 path => $_,
	     ));
	     
	load_info($self->sw_file[-1]);
    }	
}

1;
