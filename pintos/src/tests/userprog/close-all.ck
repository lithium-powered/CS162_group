# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF', <<'EOF']);
(close-all) begin
(close-all) open "sample.txt"
(close-all) open "sample2.txt"
(close-all) close "sample.txt" and "sample2.txt"
(close-all) try closing "sample.txt" individually
(close-all) end
close-all: exit(0)
EOF
(close-all) begin
(close-all) open "sample.txt"
(close-all) open "sample2.txt"
(close-all) close "sample.txt" and "sample2.txt"
(close-all) try closing "sample.txt" individually
(close-all) end
close-all: exit(-1)
EOF
pass;
