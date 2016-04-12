# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(open-diff-fd) begin
(open-diff-fd) open "sample.txt"
(open-diff-fd) open "sample2.txt"
(open-diff-fd) end
open-diff-fd: exit(0)
EOF
pass;
