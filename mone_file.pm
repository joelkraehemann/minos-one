#!/usr/bin/perl

use warnings;
use strict;
use Digest::MD5;

package MinosOne::SwFile;

my $sw_file {
    path => undef,
    last_modified => undef,
    access_mode => -1,
    owner => undef,
    group => undef,
    symlink => undef,
    md5_checksum => undef,
};

bless $sw_file, "SwFile";

sub new {
    my $class = shift;
    my $self = {@_};
    
    bless ($self, $class);
    
    return($self);
}

sub load_info {
    my $self = shift;

    if(defined $self->path &&
       -e $self->path){

	if(-l $self->path){
	    $self->last_modified = lstat($self->path)[9];
	    $self->access_mode = lstat($self->path)[2];
	    $self->owner = lstat($self->path)[4];
	    $self->group = lstat($self->path)[5];
	    $self->symlink = readlink($self->path);
	}else{
	    open $FH, $self->path;
	    
	    $ctx = Digest::MD5->new();
	    $ctx->addfile($FH);
	    
	    $self->last_modified = stat($self->path)[9];
	    $self->access_mode = stat($self->path)[2];
	    $self->owner = stat($self->path)[4];
	    $self->group = stat($self->path)[5];
	    $self->md5_checksum = $ctx->b64digest;

	    close $FH;
	}
    }
}

1;
