#!/usr/bin/perl
# **************************************************
# * filename:     monitor.pm                       *
# * author:       Jo?l Kr?hemann                   *
# * license:      GPLv3                            *
# * date:         25-11-2015                       *
# * title:        Monitor your filesystem          *
# * description:  monitor.pm watches your          *
# *   filesystem for changes and persists          *
# *   added files or modified attributes.          *
# **************************************************

use warnings;
use strict;

my @sw_package = undef;
my $filename = undef;

