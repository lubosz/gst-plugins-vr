#!/usr/bin/env python
import os, sys
usage = "usage: %s [infile [outfile]]" % os.path.basename(sys.argv[0])

if len(sys.argv) < 1:
    print (usage)
else:
    stext = "<insert_a_suppression_name_here>"
    rtext = "memcheck problem #"
    input = sys.stdin
    output = sys.stdout
    hit = 0
    if len(sys.argv) > 1:
        input = open(sys.argv[1])
    if len(sys.argv) > 2:
        output = open(sys.argv[2], 'w')
    for s in input.readlines():
        if s.replace(stext, "") != s:
            hit = hit + 1
            output.write(s.replace(stext, "memcheck problem #%d" % hit))
        else:
          output.write(s)
