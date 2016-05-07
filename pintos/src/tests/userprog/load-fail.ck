# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(load-fail) begin
load failed, too many arguments
(load-fail) exec("child-args"): -1
(load-fail) end
load-fail: exit(0)
EOF
pass;
