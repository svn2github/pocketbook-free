#
# This spec file is read by gcj when linking.
# It is used to specify the standard libraries we need in order
# to link with libgcj.
#
%rename lib liborig
*lib: -lgcj -lm  -liconv    -lgdi32 -lws2_32  %(libgcc) %(liborig)

*jc1: -fhash-synchronization -fuse-divide-subroutine -fcheck-references -fuse-boehm-gc -fnon-call-exceptions -fno-omit-frame-pointer  -fkeep-inline-functions
