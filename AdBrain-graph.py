#!/usr/bin/python
#coding=utf-8

import time
import os
import sys

def do_work(batchsize):
  os.system('blaze run -c opt -- learning/brain/experimental/grappler/costs/tools/assist:grappler_assist graph_to_metagraph --input_file=/tmp/AdBrain-bs%s/dense_layers_tpu-inference.pbtxt --train_ops_list=output_0_shard_0,output_0_shard_0_1 --output_file=/tmp/AdBrain-bs%s/AdBrain-inference.textproto' %(batchsize, batchsize))

def main():
  batchsizes = [256]
  for batchsize in batchsizes:
    print("Do batchsize", batchsize, "...")
    do_work(batchsize)

main()
