#!/bin/sh
############################################################################
#
# This script generates a C header file from a GIMP gradient file.  The header
# just contains the gradient file as a static char buffer.
#
# Authors:
#   Morten Eriksen <mortene@sim.no>
#   Lars J. Aas <larsa@sim.no>
#

UPCASEBASE=`basename $1 .gg | tr '[a-z]' '[A-Z]'`

cat <<HERE_DOC_END
#ifndef COIN_${UPCASEBASE}_GRADIENT_H
#define COIN_${UPCASEBASE}_GRADIENT_H

static const char ${UPCASEBASE}_gradient[] =
HERE_DOC_END

cat $1 | sed -e \
's/\\/\\\\/g
s/"/\\"/g
3,$ s/^[ \t]*#.*//
s/^/  "/
s/$/\\n"/
$ s/$/;/'

# ATTN: the file did not just get corrupted ;-)

cat <<HERE_DOC_END

#endif /* ! COIN_${UPCASEBASE}_GRADIENT_H */
HERE_DOC_END
