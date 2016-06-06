#!/usr/bin/sh

export G_SLICE=always-malloc
export G_DEBUG=gc-friendly

SCRIPT_DIR=valgrind
SUPP_DIR=$SCRIPT_DIR/suppressions

GST_VALIDATE_DIR=/home/bmonkey/pitivi-git/gst-devtools/validate

export GST_GL_XINITTHREADS=1
export GST_VALIDATE_SCENARIOS_PATH=$GST_VALIDATE_DIR/data/scenarios/
export GST_VALIDATE_SCENARIO=play_15s
export GST_VALIDATE_CONFIG=$GST_VALIDATE_DIR/validate/data/valgrind.config
export DISPLAY=:1

PLAIN_GL_PIPELINE="gltestsrc ! video/x-raw(memory:GLMemory), framerate=(fraction)30/1, width=1280, height=720 ! glimagesink"
FREENECT_PIPELINE="freenect2src sourcetype=0 ! glimagesink"
HMDWARP_PIPELINE="gltestsrc ! video/x-raw(memory:GLMemory), framerate=(fraction)30/1, width=1280, height=720 ! hmdwarp ! glimagesink"

PIPELINE=$FREENECT_PIPELINE

ACTIVE_SUPPS=("--suppressions=$GST_VALIDATE_DIR/data/gstvalidate.supp"
              "--suppressions=$GST_VALIDATE_DIR/common/gst.supp"
              "--suppressions=$SUPP_DIR/gstgl.supp"
              "--suppressions=$SUPP_DIR/freenect-deps.supp")

VALGRIND_ARGS=("--leak-check=full"
               "--show-leak-kinds=all"
               "--error-limit=no"
               "--tool=memcheck"
               "--show-reachable=yes"
               "--track-origins=yes"
               "--leak-resolution=high"
               "--num-callers=20"
               "--errors-for-leak-kinds=definite")
: '
  "--error-exitcode=20"
'

if [ "$1" == "run" ]; then
  echo "Running"
  valgrind -v ${VALGRIND_ARGS[@]} ${ACTIVE_SUPPS[@]} gst-validate-1.0-debug $PIPELINE
elif [ "$1" == "capture" ]; then
  echo "capture"
  valgrind -v ${VALGRIND_ARGS[@]} \
     --gen-suppressions=all \
     --log-file=$SUPP_DIR/suppressions.log \
    ${ACTIVE_SUPPS[@]} \
    gst-validate-1.0-debug $PIPELINE

  cat $SUPP_DIR/suppressions.log | $SCRIPT_DIR/valgrind-parse-suppressions.awk > $SUPP_DIR/current.supp
  ./valgrind/valgrind-make-fix-list.py $SUPP_DIR/current.supp > $SUPP_DIR/fix.supp
  #./valgrind/valgrind-make-lib-suppressions.py > $SUPP_DIR/libs.supp  
else
  echo "Usage: $0 run | capture"
  exit
fi


