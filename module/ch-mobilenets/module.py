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

TARGET_REPO = None
REPO_TF = 'ck-tensorflow'
REPO_ARMCL = 'ck-request-asplos18-mobilenets-armcl-opencl'

PACKAGE_FLAVOURS_V1 = [
  ( '0.25', '128' ),
  ( '0.25', '160' ),
  ( '0.25', '192' ),
  ( '0.25', '224' ),
  ( '0.50', '128' ),
  ( '0.50', '160' ),
  ( '0.50', '192' ),
  ( '0.50', '224' ),
  ( '0.75', '128' ),
  ( '0.75', '160' ),
  ( '0.75', '192' ),
  ( '0.75', '224' ),
  ( '1.0',  '128' ),
  ( '1.0',  '160' ),
  ( '1.0',  '192' ),
  ( '1.0',  '224' ),
]

PACKAGE_FLAVOURS_V2 = [
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

########################################################################
# Set target repo

def set_target_repo(repo):
  global TARGET_REPO
  TARGET_REPO = repo


########################################################################
# Generates package name respecting target repo

def get_package_name(version, multiplier, resolution):
  if TARGET_REPO == REPO_TF:
    #return 'tensorflowmodel-mobilenet-v{}-{}-{}-2018_02_22-py'.format(version, multiplier, resolution)
    return 'tensorflowmodel-mobilenet-v{}-{}-{}-py'.format(version, multiplier, resolution)
  if TARGET_REPO == REPO_ARMCL:
    return 'weights-mobilenet-v{}-{}-{}-npy'.format(version, multiplier, resolution)
  raise Exception('Unsupported target repo: ' + TARGET_REPO)


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
# Find package by name in target repo

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
########################################################################
# Initialize module

def init(i):
  """ Input:  {}
      Output: { return  - error code or 0 if successful
                (error) - error text if return > 0 } """
  return {'return': 0}


########################################################################
########################################################################

def make_package(version, multiplier, resolution):
  package_name = get_package_name(version, multiplier, resolution)
  color_out(Colors.HEADER, '\n' + package_name)

  res = find_package(package_name)
  if res['return'] > 0: return res

  if res['package']:
    ck.out('Already exists, skip')
    return {'return': 0}

  ck.out('Making...')

  res = ck.access({'action': 'add',
                   'repo_uoa': TARGET_REPO,
                   'module_uoa': 'package',
                   'data_uoa': package_name})
  if res['return'] > 0: return res

  color_out(Colors.OKGREEN, 'Done')
  return {'return': 0}


# Make set of mobilenet-v1 packages
def make_v1(i):
  """ Input:  {}
      Output: { return  - error code or 0 if successful
                (error) - error text if return > 0 } """
  set_target_repo(REPO_TF)
  for multiplier, resolution in PACKAGE_FLAVOURS_V1:
    res = make_package('1', multiplier, resolution)
    if res['return'] > 0: return res
  return {'return': 0}


# Make set of mobilenet-v2 packages
def make_v2(i):
  """ Input:  {}
      Output: { return  - error code or 0 if successful
                (error) - error text if return > 0 } """
  set_target_repo(REPO_TF)
  for multiplier, resolution in PACKAGE_FLAVOURS_V2:
    res = make_package('2', multiplier, resolution)
    if res['return'] > 0: return res
  return {'return': 0}


########################################################################
########################################################################

# Remove set of mobilenet-v2 packages
def rm_v2(i):
  """ Input:  {}
      Output: { return  - error code or 0 if successful
                (error) - error text if return > 0 } """
  ck.out('Remove set of mobilenet-v2 packages')
  set_target_repo(REPO_TF)

  for multiplier, resolution in PACKAGE_FLAVOURS_V2:
    package_name = get_package_name('2', multiplier, resolution)
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
########################################################################

# Makes meta.json for ck-tensorflow:package:tensorflowmodel-mobilenet-*-py packages
def make_meta_v1_2017_06_14(multiplier, resolution):
  return {
    "check_exit_status": "yes", 
    "customize": {
      "install_env": {
        "MODEL_MOBILENET_MULTIPLIER": multiplier, 
        "MODEL_MOBILENET_RESOLUTION": resolution, 
        "MODEL_MOBILENET_VERSION": "1", 
        "MODEL_WEIGHTS_ARE_CHECKPOINTS": "YES", 
        "MODULE_FILE": "mobilenet-model.py", 
        "PACKAGE_NAME": "mobilenet_v1_{}_{}_2017_06_14.tar.gz".format(multiplier, resolution), 
        "PACKAGE_URL": "http://download.tensorflow.org/models",
        "WEIGHTS_FILE": "mobilenet_v1_{}_{}.ckpt".format(multiplier, resolution),
        "MODEL_IMAGE_HEIGHT": resolution,
        "MODEL_IMAGE_WIDTH": resolution,
        "MODEL_NORMALIZE_DATA": "YES"
      }, 
      "no_os_in_suggested_path": "yes", 
      "no_ver_in_suggested_path": "yes", 
      "skip_file_check": "yes", 
      "version": "1_{}_{}".format(multiplier, resolution)
    }, 
    "end_full_path": {
      "linux": "mobilenet-model.py"
    }, 
    "only_for_host_os_tags": [
      "linux"
    ], 
    "only_for_target_os_tags": [
      "linux"
    ], 
    "package_extra_name": " (mobilenet-v1-{}-{})".format(multiplier, resolution), 
    "process_script": "install", 
    "soft_uoa": "439b9f1757f27091", 
    "suggested_path": "tensorflowmodel-mobilenet-v1-{}-{}-py".format(multiplier, resolution),
    "tags": [
      "tensorflowmodel", 
      "weights", 
      "python", 
      "mobilenet", 
      "mobilenet-v1",
      "mobilenet-v1-{}-{}".format(multiplier, resolution),
      "2017_06_14"
    ], 
    "use_scripts_from_another_entry": {
      "data_uoa": "6b1b2b254718b69a",
      "module_uoa": "script"
    }
  }


def make_meta_2018_02_22(multiplier, resolution):
  date = "2018_02_22"
  return {
    "check_exit_status": "yes",
    "customize": {
      "install_env": {
        "MODEL_MOBILENET_MULTIPLIER": multiplier, 
        "MODEL_MOBILENET_RESOLUTION": resolution, 
        "MODEL_MOBILENET_VERSION": "1", 
        "MODEL_WEIGHTS_ARE_CHECKPOINTS": "YES", 
        "MODULE_FILE": "mobilenet-model.py", 
        "PACKAGE_NAME": "mobilenet_v1_{}_{}.tgz".format(multiplier, resolution), 
        "PACKAGE_URL": "http://download.tensorflow.org/models/mobilenet_v1_2018_02_22",
        "WEIGHTS_FILE": "mobilenet_v1_{}_{}.ckpt".format(multiplier, resolution),
        "MODEL_IMAGE_HEIGHT": resolution,
        "MODEL_IMAGE_WIDTH": resolution,
        "MODEL_NORMALIZE_DATA": "YES"
      }, 
      "no_os_in_suggested_path": "yes", 
      "no_ver_in_suggested_path": "yes", 
      "skip_file_check": "yes", 
      "version": "1_{}_{}_{}".format(multiplier, resolution, date)
    }, 
    "end_full_path": {
      "linux": "mobilenet-model.py"
    }, 
    "only_for_host_os_tags": [
      "linux"
    ], 
    "only_for_target_os_tags": [
      "linux"
    ], 
    "package_extra_name": " (mobilenet-v1-{}-{}-{})".format(multiplier, resolution, date), 
    "process_script": "install", 
    "soft_uoa": "439b9f1757f27091", 
    "suggested_path": "tensorflowmodel-mobilenet-v1-{}-{}-{}-py".format(multiplier, resolution, date),
    "tags": [
      "tensorflowmodel", 
      "weights", 
      "python", 
      "mobilenet", 
      "mobilenet-v1",
      "mobilenet-v1-{}-{}".format(multiplier, resolution),
      date
    ], 
    "use_scripts_from_another_entry": {
      "data_uoa": "6b1b2b254718b69a",
      "module_uoa": "script"
    }
  }


def make_meta_v2(multiplier, resolution):
  return {
    "check_exit_status": "yes", 
    "customize": {
      "install_env": {
        "MODEL_MOBILENET_MULTIPLIER": multiplier, 
        "MODEL_MOBILENET_RESOLUTION": resolution, 
        "MODEL_MOBILENET_VERSION": "2", 
        "MODEL_WEIGHTS_ARE_CHECKPOINTS": "YES", 
        "MODULE_FILE": "mobilenet-model.py", 
        "PACKAGE_NAME": "mobilenet_v2_{}_{}.tgz".format(multiplier, resolution), 
        "PACKAGE_URL": "https://storage.googleapis.com/mobilenet_v2/checkpoints", 
        "WEIGHTS_FILE": "mobilenet_v2_{}_{}.ckpt".format(multiplier, resolution),
        "MODEL_IMAGE_HEIGHT": resolution,
        "MODEL_IMAGE_WIDTH": resolution,
        "MODEL_NORMALIZE_DATA": "YES"
      }, 
      "no_os_in_suggested_path": "yes", 
      "no_ver_in_suggested_path": "yes", 
      "skip_file_check": "yes", 
      "version": "2_{}_{}".format(multiplier, resolution)
    }, 
    "end_full_path": {
      "linux": "mobilenet-model.py"
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
      "tensorflowmodel", 
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

# Makes meta.json for ck-request-asplos18-mobilenets-armcl-opencl:package:weights-mobilenet-*-npy packages
def make_meta_npy(version, multiplier, resolution):
  urls = {
    '1_0.25_128': "https://www.dropbox.com/s/1nr0k4qr3yc1e3f/mobilenet_v1_0.25_128.npy.zip",
    '1_0.25_160': "https://www.dropbox.com/s/5lzc1szu7hcuf1t/mobilenet_v1_0.25_160.npy.zip",
    '1_0.25_192': "https://www.dropbox.com/s/gzd1jlejurfyby7/mobilenet_v1_0.25_192.npy.zip",
    '1_0.25_224': "https://www.dropbox.com/s/7wedo4s5eqsktp0/mobilenet_v1_0.25_224.npy.zip",
    '1_0.50_128': "https://www.dropbox.com/s/iyc443hi9ot6hjx/mobilenet_v1_0.50_128.npy.zip",
    '1_0.50_160': "https://www.dropbox.com/s/loqnhsu5iz0q5qd/mobilenet_v1_0.50_160.npy.zip",
    '1_0.50_192': "https://www.dropbox.com/s/p16g13az73g5979/mobilenet_v1_0.50_192.npy.zip",
    '1_0.50_224': "https://www.dropbox.com/s/3nj6lkeupoyyn8w/mobilenet_v1_0.50_224.npy.zip",
    '1_0.75_128': "https://www.dropbox.com/s/xna5hegqlasodrg/mobilenet_v1_0.75_128.npy.zip",
    '1_0.75_160': "https://www.dropbox.com/s/n1ag6zmwtv63uhz/mobilenet_v1_0.75_160.npy.zip",
    '1_0.75_192': "https://www.dropbox.com/s/ctp5gj44vqer7fu/mobilenet_v1_0.75_192.npy.zip",
    '1_0.75_224': "https://www.dropbox.com/s/u8xrtub8xn05mak/mobilenet_v1_0.75_224.npy.zip",
    '1_1.0_128': "https://www.dropbox.com/s/982bl2xtb9031do/mobilenet_v1_1.0_128.npy.zip",
    '1_1.0_160': "https://www.dropbox.com/s/hotm830uboxefgi/mobilenet_v1_1.0_160.npy.zip",
    '1_1.0_192': "https://www.dropbox.com/s/0qdfh7v2ipuwio0/mobilenet_v1_1.0_192.npy.zip",
    '1_1.0_224': "https://www.dropbox.com/s/8nsydl1xscheyx7/mobilenet_v1_1.0_224.npy.zip",
  }
  key = '{}_{}_{}'.format(version, multiplier, resolution)
  if key not in urls:
    raise Exception('Download URL not set for ' + key)
  return {
    "customize": {
      "extra_dir": "", 
      "install_env": {
        "MOBILENET_MULTIPLIER": multiplier, 
        "MOBILENET_RESOLUTION": resolution, 
        "MOBILENET_VERSION": version, 
        "PACKAGE_NAME": "mobilenet_v{}_{}_{}.npy.zip".format(version, multiplier, resolution), 
        "PACKAGE_SKIP_CLEAN_INSTALL": "YES", 
        "PACKAGE_SKIP_CLEAN_OBJ": "YES", 
        "PACKAGE_SKIP_LINUX_MAKE": "YES", 
        "PACKAGE_UNZIP": "YES", 
        "PACKAGE_URL": urls[key], 
        "PACKAGE_WGET": "YES"
      }, 
      "no_os_in_suggested_path": "yes", 
      "no_ver_in_suggested_path": "yes", 
      "version": "{}_{}_{}".format(version, multiplier, resolution)
    }, 
    "end_full_path": {
      "linux": "Conv2d_0_BatchNorm_beta.npy"
    }, 
    "process_script": "install", 
    "soft_uoa": "50b442db9bde0a0c", 
    "suggested_path": "weights-mobilenet-v{}_{}_{}-npy".format(version, multiplier, resolution), 
    "tags": [
      "mobilenet",
      "mobilenet-v1",
      "mobilenet-v{}-{}-{}".format(version, multiplier, resolution),
      "weights", 
      "npy"
    ], 
    "use_scripts_from_another_entry": {
      "data_uoa": "cd9ccc74060b3d18", 
      "module_uoa": "script"
    }
  }


def update_meta(version, multiplier, resolution, make_meta_func):
  package_name = get_package_name(version, multiplier, resolution)
  color_out(Colors.HEADER, '\n' + package_name)

  res = find_package(package_name)
  if res['return'] > 0: return res

  package = res['package']
  if not package:
    ck.out('Skip')
    return {'return': 0}

  ck.out('Processing in {} ...'.format(package['path']))

  meta = make_meta_func(multiplier, resolution)
  save_meta(package, meta)

  color_out(Colors.OKGREEN, 'Done')
  return {'return': 0}


# Update meta.json of mobilenet-v1 packages
def update_meta_v1(i):
  """ Input:  {}
      Output: { return  - error code or 0 if successful
                (error) - error text if return > 0 } """
  set_target_repo(REPO_TF)
  for multiplier, resolution in PACKAGE_FLAVOURS_V1:
    #res = update_meta('1', multiplier, resolution, make_meta_v1_2017_06_14)
    res = update_meta('1', multiplier, resolution, make_meta_2018_02_22)
    if res['return'] > 0: return res
  return {'return': 0}


# Update meta.json of mobilenet-v2 packages
def update_meta_v2(i):
  """ Input:  {}
      Output: { return  - error code or 0 if successful
                (error) - error text if return > 0 } """
  set_target_repo(REPO_TF)
  for multiplier, resolution in PACKAGE_FLAVOURS_V2:
    res = update_meta('2', multiplier, resolution, make_meta_v2)
    if res['return'] > 0: return res
  return {'return': 0}


# Update meta.json for mobilenet-v1 npy weights packages
def update_meta_v1_npy(i):
  """ Input:  {}
      Output: { return  - error code or 0 if successful
                (error) - error text if return > 0 } """
  set_target_repo(REPO_ARMCL)
  for multiplier, resolution in PACKAGE_FLAVOURS_V1:
    res = update_meta('1', multiplier, resolution, make_meta_npy)
    if res['return'] > 0: return res
  return {'return': 0}


##############################################################################
##############################################################################

def add_dep(deps, sort, version, multiplier, resolution):
  package_name = 'mobilenet-v{}-{}-{}'.format(version, multiplier, resolution)
  deps[package_name] = {
    "local": "yes", 
    "name": package_name, 
    "sort": sort, 
    "tags": "tensorflowmodel,python," + package_name
  }


# Generate deps list for aggregate package
def gen_deps_v1(i):
  """ Input:  {}
      Output: { return  - error code or 0 if successful
                (error) - error text if return > 0 } """
  root = {}
  deps = {}
  sort = 1
  for multiplier, resolution in PACKAGE_FLAVOURS_V1:
    add_dep(deps, sort, '1', multiplier, resolution)
    sort += 1
  root['deps'] = deps
  print(json.dumps(root, indent=2, sort_keys=False))
  return {'return': 0}


# Generate deps list for aggregate package
def gen_deps_v2(i):
  """ Input:  {}
      Output: { return  - error code or 0 if successful
                (error) - error text if return > 0 } """
  root = {}
  deps = {}
  sort = 1
  for multiplier, resolution in PACKAGE_FLAVOURS_V2:
    add_dep(deps, sort, '2', multiplier, resolution)
    sort += 1
  root['deps'] = deps
  print(json.dumps(root, indent=2, sort_keys=False))
  return {'return': 0}
