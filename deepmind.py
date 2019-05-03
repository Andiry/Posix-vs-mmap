#!/usr/bin/python
#coding=utf-8

import time
import os
import sys

def do_work(model, GPU, TYPE):
  if model == 'wavenet' and TYPE == 'half':
    # No FP16 for WaveNet
    return
  logfile = model + "-" + str(GPU) + "-" + TYPE + ".log"
  os.system('LD_LIBRARY_PATH=/var/google/persistent/kibbles/libcuda_running/ ./%s.par --uid=root --replicator_type=gpu --num_steps_per_run=1 --num_devices_per_worker=%s --master=local --brain_port=7077 --dtype=%s 2>&1 | tee %s' %(model, GPU, TYPE, logfile))

def main():
  models = ["conv_draw", "pixel_cnn", "unet", "wavenet"]
  GPUs = [1, 2, 4, 8, 16]
  TYPEs = ["float32", "half"]

  for model in models:
    for GPU in GPUs:
      for TYPE in TYPEs:
        do_work(model, GPU, TYPE)

main()
