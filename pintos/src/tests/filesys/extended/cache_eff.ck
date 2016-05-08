# -*- perl -*-
use strict;
use warnings;
use tests::tests;
use tests::random;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(cache_eff) begin
(cache_eff) create "testfile"
(cache_eff) open "testfile"
(cache_eff) writing "testfile"
(cache_eff) close "testfile"
(cache_eff) open "testfile" for verification
(cache_eff) verified contents of "testfile"
(cache_eff) close "testfile"
(cache_eff) less reads than writes
(cache_eff) end
EOF
pass;
