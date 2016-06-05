#!/usr/bin/python2
#coding: UTF-8

import sys, os, operator, subprocess

def shell(command):
  output = ""
  try:
    output = subprocess.check_output(command, stderr=subprocess.STDOUT, shell=True)
  except subprocess.CalledProcessError:
    #print(colors['red']+command + " has no output"+ colors['clear'])
    pass
  
  if output:
    fileList = output.strip().split("\n")
  else:
    fileList = []
    
  return fileList

foo = shell('grep "obj:" valgrind/suppressions/current.supp | sort | uniq')

for bar in foo:

  name = bar.strip()
  name = name.replace("obj:/lib/","")
  name = name.replace("obj:/usr/lib/","")
  name = name.replace(".so","")
  
  names = name.split(".")
  name = names[0]

  suppression ="{\n\
   %s\n\
   Memcheck:Leak\n\
   ...\n\
   %s\n\
   ...\n\
}" % (name, bar.strip())
  
  print suppression
