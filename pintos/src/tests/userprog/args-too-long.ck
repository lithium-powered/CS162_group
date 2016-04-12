# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(args) begin
(args) argc = 2
(args) argv[0] = 'args-too-long'
(args) argv[1] = 'a b c d e f g h i j k l m n o p q r s t u v a b c d e f g h i j k l m n o p q r s t u v'
(args) argv[2] = null
(args) end
args-too-long: exit(0)
EOF
pass;
