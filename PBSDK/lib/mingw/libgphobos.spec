#
# This spec file is read by gdc when linking.
# It is used to specify the standard libraries we need in order
# to link with libphobos.
#
%rename lib liborig
*lib: -lm  %(liborig)
