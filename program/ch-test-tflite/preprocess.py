#
# Copyright (c) 2018 cTuning foundation.
# See CK COPYRIGHT.txt for copyright details.
#
# SPDX-License-Identifier: BSD-3-Clause.
# See CK LICENSE.txt for licensing details.
#

import os

def ck_preprocess(i):
  MODEL_FILE = 'mobilenet_quant_v1_224.tflite'
  IMAGE_FILE = 'test-image.bmp'
  LABELS_FILE = 'labels.txt'

  new_env = {}
  files_to_push = []
  
  os_name = i.get('target_os_dict', {}).get('ck_name2', '')
  
  if os_name == 'android':
    # Set list of additional files to be copied to Android device.
    # We have to set these files via env variables with full paths 
    # in order to they will be copied into remote program dir without sub-paths.
    new_env['RUN_OPT_MODEL_PATH'] = os.path.join(os.getcwd(), '..', MODEL_FILE)
    new_env['RUN_OPT_IMAGE_PATH'] = os.path.join(os.getcwd(), '..', IMAGE_FILE)
    new_env['RUN_OPT_LABELS_PATH'] = os.path.join(os.getcwd(), '..', LABELS_FILE)
    files_to_push.append('$<<RUN_OPT_MODEL_PATH>>$')
    files_to_push.append('$<<RUN_OPT_IMAGE_PATH>>$')
    files_to_push.append('$<<RUN_OPT_LABELS_PATH>>$')
    new_env['RUN_OPT_MODEL'] = MODEL_FILE
    new_env['RUN_OPT_IMAGE'] = IMAGE_FILE
    new_env['RUN_OPT_LABELS'] = LABELS_FILE

  elif os_name == "linux":
    new_env['RUN_OPT_MODEL'] = os.path.join('..', MODEL_FILE)
    new_env['RUN_OPT_IMAGE'] = os.path.join('..', IMAGE_FILE)
    new_env['RUN_OPT_LABELS'] = os.path.join('..', LABELS_FILE)

  return {'return': 0, 'new_env': new_env, 'run_input_files': files_to_push}

