#!/bin/bash
localfile=$1
remotefile=$2

ftp -n << __END_OF_SCRIPT__
    open 192.168.57.13 8022
    binary
    user upload jpls2s
    put $localfile $remotefile/body.content
    bye
__END_OF_SCRIPT__
