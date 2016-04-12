# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(filesize) begin
(filesize) file size is 239
(filesize) filesize is correct
(filesize) end
filesize: exit(0)
EOF
pass;
