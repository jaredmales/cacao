#!/usr/bin/env bash

act1Dval=$1
act2Dim=$2

# Remap to 2D, loop
#
milk << EOF
readshmim ${act1Dval}
loadfits mapping.fits mapping
# make output shared memory stream
pixremap .outim.shared 1
pixremap ..procinfo 1

# trigger on semaphore of stream act1Dval
pixremap ..triggermode 3
pixremap ..triggersname ${act1Dval}

# run forever, until killed
pixremap ..loopcntMax -1

pixremap ${act1Dval} mapping ${act2Dim}
exitCLI
EOF
