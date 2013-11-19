#!/bin/bash

FUNCTION=$1
shift

EXE=$@

echo "FUNCTION: $FUNCTION"
echo "EXE: $EXE"

DEBUGFS_PATH=/sys/kernel/debug
TRACER_PATH=${DEBUGFS_PATH}/tracing

sudo mount -t debugfs nodev ${DEBUGFS_PATH} || echo "debugfs already mounted."
sudo chmod a+rw ${DEBUGFS_PATH}
sudo chmod a+rw ${TRACER_PATH}
sudo chmod a+rw ${TRACER_PATH}/*

echo function_graph > ${TRACER_PATH}/current_tracer

echo 0 > ${TRACER_PATH}/tracing_on
echo > ${TRACER_PATH}/trace
echo '' > ${TRACER_PATH}/set_ftrace_filter
echo '' > ${TRACER_PATH}/set_graph_function

#echo $FUNCTION > ${TRACER_PATH}/set_graph_function
#echo $FUNCTION > ${TRACER_PATH}/set_ftrace_filter
#echo 'ext2_get_xip_mem' > ${TRACER_PATH}/set_ftrace_filter
#echo 'vfs_write' > ${TRACER_PATH}/set_ftrace_filter
echo 'xip_file_write' > ${TRACER_PATH}/set_ftrace_filter
#echo 'ext2_get_xip_mem' > ${TRACER_PATH}/set_graph_function
#echo 'vfs_write' > ${TRACER_PATH}/set_graph_function
echo 'xip_file_write' > ${TRACER_PATH}/set_graph_function

#./write_to_ram_warm Ext2 1 0 1g test.csv

#echo 1 > ${TRACER_PATH}/tracing_on

${EXE}

#echo 0 > ${TRACER_PATH}/tracing_on
echo 0 > ${TRACER_PATH}/tracing_on
echo '' > ${TRACER_PATH}/set_ftrace_filter
echo '' > ${TRACER_PATH}/set_graph_function

cat ${TRACER_PATH}/trace > ${FUNCTION}_graph.txt
