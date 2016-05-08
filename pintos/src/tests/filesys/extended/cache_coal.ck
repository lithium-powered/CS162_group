# -*- perl -*-
use strict;
use warnings;
use tests::tests;
use tests::random;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(cache_coal) begin
(cache_coal) create "testfile"
(cache_coal) open "testfile"
(cache_coal) writing "testfile"
(cache_coal) close "testfile"
(cache_coal) open "testfile" for verification
(cache_coal) verified contents of "testfile"
(cache_coal) close "testfile"
(cache_coal) writes were coalesced
(cache_coal) end
EOF
pass;
