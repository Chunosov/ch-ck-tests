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


########################################################################
# Colored output

def color_out(color, txt):
  ck.out(color + txt + Colors.ENDC)


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
# Make printable string from an object

def get_obj_str(obj):
  return json.dumps(obj, indent=2)


########################################################################
# Search and select a program by given UOA

def get_program(prog_uoa):
  res = ck.access({'action': 'search',
                   'module_uoa': 'program',
                   'data_uoa': prog_uoa})
  if res['return'] > 0:
    return res

  selected_prog = None

  progs = res['lst']
  if len(progs) > 0:
    if len(progs) == 1:
      selected_prog = progs[0]
    else:
      ck.out('\nMore than one program is found:\n')
      res = ck.access({'action': 'select_uoa',
                       'module_uoa': 'choice',
                       'choices': progs})
      if res['return'] > 0:
        return res

      for d in progs:
        if d['data_uid'] == res['choice']:
          selected_prog = d
          break

  return selected_prog


########################################################################
# Load meta of some ck-entry

def load_meta(entry):
  with open(os.path.join(entry['path'], '.cm', 'meta.json')) as f:
    return json.load(f)


########################################################################
# Get list of all files of all datasets for specified program
# Each item in the list is a pair: (dateset_uoa, dataset_file)

def get_dataset_files(prog):
  prog_meta = load_meta(prog)
  dataset_tags = ','.join(prog_meta['run_cmds']['default']['dataset_tags'])
    
  ck.out('Program dataset tags: ' + dataset_tags)

  res = ck.access({'action': 'search',
                   'module_uoa': 'dataset',
                   'tags': dataset_tags})
  if res['return'] > 0:
    return res
  datasets = res['lst']
  if not datasets:
    return {'return': 1, 'error': 'Datasets not found'}
  ck.out('Datasets found: {}'.format(len(datasets)))
  processing_files = []
  for dataset in datasets:
    dataset_uoa = dataset['data_uoa']
    dataset_meta = load_meta(dataset)
    for dataset_file in dataset_meta['dataset_files']:
      processing_files.append((dataset_uoa, dataset_file))
  ck.out('Files to process: {}'.format(len(processing_files)))
  return processing_files


########################################################################
# Whether or not program run was successful

def is_ok(res):
  return res['misc']['run_success_bool']


########################################################################
# Try to extract fail reason from program run result

def get_fail_reason(res):
  misc = res['misc']
  reason = ''
  failures = misc.get('output_check_failures')
  if failures:
    tmp_json = failures.get('tmp-ck-output.json')
    if tmp_json:
      reason = tmp_json.get('fail_reason')
  if not reason:
    reason = misc['fail_reason']
  return reason
  

########################################################################
# Make reference output

def make(i):
  """
  Input:  {
            (output_validation_repo) - output validation repo UOA (when recording new output)
            (stop_after_fail)        - stop processing after first fail
          }
  Output: {
            return       - return code =  0, if successful
                                       >  0, if error
            (error)      - error text if return > 0
          }
  """

  prog = get_program(i.get('data_uoa'))
  if not prog:
    return {'return': 1, 'error': 'Program not found'}

  prog_uoa = prog['data_uoa']
  ck.out('Make reference output for program ' + prog_uoa)

  validation_repo = i.get('output_validation_repo')
  if not validation_repo:
    validation_repo = 'local'
    color_out(Colors.WARNING, 'Validation repo is not set, using local')

  stop_after_fail = i.get('stop_after_fail') == 'yes'

  processed_files = 0
  processing_files = get_dataset_files(prog)
  failed_shapes = []

  ck.out('\nProcessing...\n')
  for dataset_uoa, dataset_file in processing_files:
    ck.out('\n----------------------------------------')
    color_out(Colors.HEADER, '{}:{} ...'.format(dataset_uoa, dataset_file))

    res = ck.access({'action': 'run',
                     'module_uoa': 'program',
                     'data_uoa': prog_uoa,
                     'cmd_key': 'default',
                     'output_validation_repo': validation_repo,
                     'overwrite_reference_output': 'yes',
                     'dataset_uoa': dataset_uoa,
                     'dataset_file': dataset_file,
                    })
    if res['return'] > 0:
      return res

    processed_files += 1

    if is_ok(res):
      color_out(Colors.OKGREEN, '****** OK ******')
    else:
      color_out(Colors.FAIL, '****** FAILED ******')
      ck.out(get_fail_reason(res))
      failed_shapes.append((dataset_uoa, dataset_file))
      if stop_after_fail:
        break

  ck.out('\n----------------------------------------')
  ck.out('Total shapes: {}'.format(len(processing_files)))
  ck.out('Processed shapes: {}'.format(processed_files))
  if failed_shapes:
    ck.out('Failed shapes: {}'.format(len(failed_shapes)))
    for p in failed_shapes:
      ck.out('{}:{}'.format(p[0], p[1]))
  else:
    ck.out('\nAll shapes OK\n')

  return {'return': 0}


########################################################################
# Validate program against saved reference output

def validate(i):
  """
  Input:  {
            (stop_after_fail)        - stop processing after first fail
          }
  Output: {
            return       - return code =  0, if successful
                                       >  0, if error
            (error)      - error text if return > 0
          }
  """
  prog = get_program(i.get('data_uoa'))
  if not prog:
    return {'return': 1, 'error': 'Program not found'}

  prog_uoa = prog['data_uoa']
  ck.out('Validate output for program ' + prog_uoa)

  stop_after_fail = i.get('stop_after_fail') == 'yes'

  processed_files = 0
  processing_files = get_dataset_files(prog)
  failed_shapes = []

  ck.out('\nProcessing...\n')
  for dataset_uoa, dataset_file in processing_files:
    ck.out('\n----------------------------------------')
    color_out(Colors.HEADER, '{}:{} ...'.format(dataset_uoa, dataset_file))

    res = ck.access({'action': 'run',
                     'module_uoa': 'program',
                     'data_uoa': prog_uoa,
                     'cmd_key': 'default',
                     'dataset_uoa': dataset_uoa,
                     'dataset_file': dataset_file,
                    })
    if res['return'] > 0:
      return res

    processed_files += 1

    if is_ok(res):
      color_out(Colors.OKGREEN, '****** OK ******')
    else:
      color_out(Colors.FAIL, '****** FAILED ******')
      failed_shapes.append({
        'dataset': dataset_uoa,
        'file': dataset_file,
        'reason': get_fail_reason(res)
      })
      if stop_after_fail:
        break

  ck.out('\n----------------------------------------')
  ck.out('Total shapes: {}'.format(len(processing_files)))
  ck.out('Processed shapes: {}'.format(processed_files))
  if failed_shapes:
    ck.out('Failed shapes: {}'.format(len(failed_shapes)))
    for p in failed_shapes:
      ck.out('----------------------------------------')
      color_out(Colors.HEADER, '{}:{}'.format(p['dataset'], p['file']))
      ck.out(p['reason'])
  else:
    ck.out('\nAll shapes OK\n')

  return {'return': 0}
