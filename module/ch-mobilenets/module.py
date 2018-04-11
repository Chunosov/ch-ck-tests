#
# Copyright (c) 2018 cTuning foundation.
# See CK COPYRIGHT.txt for copyright details.
#
# SPDX-License-Identifier: BSD-3-Clause.
# See CK LICENSE.txt for licensing details.
#     

import os
import json

cfg = {}  # Will be updated by CK (meta description of this module)
work = {} # Will be updated by CK (temporal data)
ck = None # Will be updated by CK (initialized CK kernel)

class Colors:
  HEADER = '\033[95m'
  OKBLUE = '\033[94m'
  OKGREEN = '\033[92m'
  WARNING = '\033[93m'
  FAIL = '\033[91m'
  ENDC = '\033[0m'
  BOLD = '\033[1m'
  UNDERLINE = '\033[4m'

TARGET_REPO = 'ck-tensorflow'

PACKAGE_FLAVOURS = [
  ( '1.4',  '224' ),
  ( '1.3',  '224' ),
  ( '1.0',  '224' ),
  ( '1.0',  '192' ),
  ( '1.0',  '160' ),
  ( '1.0',  '128' ),
  ( '1.0',  '96' ),
  ( '0.75', '224' ),
  ( '0.75', '192' ),
  ( '0.75', '160' ),
  ( '0.75', '128' ),
  ( '0.75', '96' ),
  ( '0.5',  '224' ),
  ( '0.5',  '192' ),
  ( '0.5',  '160' ),
  ( '0.5',  '128' ),
  ( '0.5',  '96' ),
  ( '0.35', '224' ),
  ( '0.35', '192' ),
  ( '0.35', '160' ),
  ( '0.35', '128' ),
  ( '0.35', '96' ),
]

def package_name_v2(multiplier, resolution):
  return 'tensorflowmodel-mobilenet-v2-{}-{}-py'.format(multiplier, resolution)

########################################################################
# Colored output

def color_out(color, txt):
  ck.out(color + txt + Colors.ENDC)


########################################################################
# Returns path to meta for some ck-entry

def meta_path(entry):
  return os.path.join(entry['path'], '.cm', 'meta.json')


########################################################################
# Loads meta of some ck-entry

def load_meta(entry):
  with open(meta_path(entry), 'r') as f:
    return json.load(f) 


########################################################################
# Saves meta of some ck-entry

def save_meta(entry, meta):
  with open(meta_path(entry), 'w') as f:
    json.dump(meta, f, indent=2, sort_keys=True)


########################################################################

def find_package(package_name):
  res = ck.access({'action': 'search',
                   'repo_uoa': TARGET_REPO,
                   'module_uoa': 'package',
                   'data_uoa': package_name})
  if res['return'] > 0:
    return res

  result_count = len(res['lst'])
  if result_count == 0:
    color_out(Colors.WARNING, 'No packages found')
    return {'return': 0, 'package': None}
  if result_count > 1:
    color_out(Colors.WARNING, 'More than one package found ({}), unsupported'.format(result_count))
    return {'return': 0, 'package': None}

  return {'return': 0, 'package': res['lst'][0]}


########################################################################

def make_meta(multiplier, resolution):
  return {
    "check_exit_status": "yes", 
    "customize": {
      "force_ask_path": "yes", 
      "install_env": {
        "MODEL_MOBILENET_MULTIPLIER": multiplier, 
        "MODEL_MOBILENET_RESOLUTION": resolution, 
        "MODEL_MOBILENET_VERSION": "2", 
        "MODEL_WEIGHTS_ARE_CHECKPOINTS": "YES", 
        "MODULE_FILE": "tf-mobilenet-model.py", 
        "PACKAGE_NAME": "mobilenet_v2_{}_{}.tgz".format(multiplier, resolution), 
        "PACKAGE_URL": "https://storage.googleapis.com/mobilenet_v2/checkpoints", 
        "WEIGHTS_FILE": "mobilenet_v2_{}_{}.ckpt".format(multiplier, resolution)
      }, 
      "no_os_in_suggested_path": "yes", 
      "no_ver_in_suggested_path": "yes", 
      "skip_file_check": "yes", 
      "version": "ImageNet"
    }, 
    "end_full_path": {
      "linux": "tf-mobilenet-model.py"
    }, 
    "only_for_host_os_tags": [
      "linux"
    ], 
    "only_for_target_os_tags": [
      "linux"
    ], 
    "package_extra_name": " (mobilenet-v2-{}-{})".format(multiplier, resolution), 
    "process_script": "install", 
    "soft_uoa": "439b9f1757f27091", 
    "suggested_path": "tensorflowmodel-mobilenet-v2-{}-{}-py".format(multiplier, resolution),
    "tags": [
      "tensorflow-model", 
      "weights", 
      "python", 
      "mobilenet", 
      "mobilenet-v2", 
      "mobilenet-v2-{}-{}".format(multiplier, resolution)
    ], 
    "use_scripts_from_another_entry": {
      "data_uoa": "6b1b2b254718b69a",
      "module_uoa": "script"
    }
  }


########################################################################
# Initialize module

def init(i):
  """
  Input:  {}
  Output: {
            return       - return code =  0, if successful
                                       >  0, if error
            (error)      - error text if return > 0
          }
  """
  return {'return': 0}


########################################################################
# Make set of mobilenet-v2 packages

def make_v2(i):
  """
  Input:  {}
  Output: {
            return       - return code =  0, if successful
                                       >  0, if error
            (error)      - error text if return > 0
          }
  """
  ck.out('Make set of mobilenet-v2 packages')

  for multiplier, resolution in PACKAGE_FLAVOURS:
    package_name = package_name_v2(multiplier, resolution)
    color_out(Colors.HEADER, '\n' + package_name)

    res = find_package(package_name)
    if res['return'] > 0:
      return res

    if res['package']:
      ck.out('Already exists, skip')
      continue

    ck.out('Making...')

    res = ck.access({'action': 'add',
                     'repo_uoa': TARGET_REPO,
                     'module_uoa': 'package',
                     'data_uoa': package_name})
    if res['return'] > 0:
      return res

    color_out(Colors.OKGREEN, 'Done')

  return {'return': 0}


##############################################################################
# Remove set of mobilenet-v2 packages

def rm_v2(i):
  """
  Input:  {}
  Output: {
            return       - return code =  0, if successful
                                       >  0, if error
            (error)      - error text if return > 0
          }
  """
  ck.out('Remove set of mobilenet-v2 packages')

  for multiplier, resolution in PACKAGE_FLAVOURS:
    package_name = package_name_v2(multiplier, resolution)
    color_out(Colors.HEADER, '\n' + package_name)

    res = find_package(package_name)
    if res['return'] > 0:
      return res

    package = res['package']
    if not package:
      ck.out('Skip')
      continue

    ck.out('Removing...')
    
    res = ck.access({'action': 'rm',
                     'repo_uoa': TARGET_REPO,
                     'module_uoa': 'package',
                     'data_uoa': package_name})
    if res['return'] > 0:
      return res

    color_out(Colors.OKGREEN, 'Done')

  return {'return': 0}


########################################################################
# Update meta.json of mobilenet-v2 packages

def update_meta_v2(i):
  """
  Input:  {}
  Output: {
            return       - return code =  0, if successful
                                       >  0, if error
            (error)      - error text if return > 0
          }
  """
  ck.out('Update meta.json of mobilenet-v2 packages')

  for multiplier, resolution in PACKAGE_FLAVOURS:
    package_name = package_name_v2(multiplier, resolution)
    color_out(Colors.HEADER, '\n' + package_name)

    res = find_package(package_name)
    if res['return'] > 0:
      return res

    package = res['package']
    if not package:
      ck.out('Skip')
      continue

    ck.out('Processing in {} ...'.format(package['path']))

    meta = make_meta(multiplier, resolution)
    save_meta(package, meta)

    color_out(Colors.OKGREEN, 'Done')

  return {'return': 0}
