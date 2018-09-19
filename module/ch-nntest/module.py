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


class CKException(Exception):
  def __init__(self, ck_result):
    self.ck_result = ck_result

  @staticmethod
  def throw(message, code=1):
    raise CKException({'return': code, 'error': message})


def ck_access(params_json, skip_error_codes = []):
  '''
  Performs call to ck-kernel and raises an exception when an error.
  Returns the result of ck-operation.
  '''
  r = ck.access(params_json)
  error_code = r['return']
  if error_code > 0 and not (error_code in skip_error_codes):
    ck.out('CK error details:')
    ck.out('    action: ' + params_json.get('action',''))
    ck.out('    param: module_uoa=' + params_json.get('module_uoa',''))
    ck.out('    param: data_uoa=' + params_json.get('data_uoa',''))
    import traceback
    stack_lines = traceback.format_stack()
    if len(stack_lines) >= 2:
       # The last line of the stack is the current line (`traceback.format_stack`),
       # so we need a line before the last - location of call of `ck_access` function.
       location_and_code = stack_lines[-2].strip().split('\n')
       ck.out('    location: ' + location_and_code[0].strip())
       if len(location_and_code) > 1:
         ck.out('    code: ' + location_and_code[1].strip())
    raise CKException(r)
  return r


def color_out(color, txt):
  '''
  Colored output
  '''
  ck.out(color + txt + Colors.ENDC)


def get_obj_str(obj):
  '''
  Make printable string from an object
  '''
  return json.dumps(obj, indent=2)


def get_program(prog_uoa):
  '''
  Search and select a program by given UOA
  '''
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


def load_meta(entry):
  '''
  Load meta of some ck-entry
  '''
  with open(os.path.join(entry['path'], '.cm', 'meta.json')) as f:
    return json.load(f)


def get_dataset_files(prog, with_path=False):
  '''
  Get list of all files of all datasets for specified program
  Each item in the list is a pair: (dataset_uoa, dataset_file)
  '''
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
      if with_path:
        dataset_file_path = os.path.join(dataset['path'], dataset_file+'.json')
        item = (dataset_uoa, dataset_file, dataset_file_path)
      else:
        item = (dataset_uoa, dataset_file)
      processing_files.append(item)
  ck.out('Files to process: {}'.format(len(processing_files)))
  return processing_files


def is_ok(res):
  '''
  Whether or not program run was successful
  '''
  return res['misc']['run_success_bool']


def get_fail_reason(res):
  '''
  Try to extract fail reason from program run result
  '''
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


def get_target_prog(i):
  '''
  Get target program from action input parameters
  '''
  data_uoa = i.get('data_uoa')
  if not data_uoa:
    CKException.throw('Program uoa is not specified')

  prog = get_program(data_uoa)
  if not prog:
    CKException.throw('Program not found')

  return prog


def get_validation_repo(i):
  '''
  Get name of valuidation repo from action input parameters
  '''
  validation_repo = i.get('output_validation_repo')
  if not validation_repo:
    validation_repo = 'local'
    color_out(Colors.WARNING, 'Validation repo is not set, using local')
  return validation_repo

  
########################################################################
#
#                        Module Actions
#
########################################################################

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
def make_reference_outputs(i):
  '''
  Create reference outputs for specified program
  
  Params:
  output_validation_repo - output validation repo UOA (when recording new output)
  stop_after_fail        - stop processing after first fail
  '''
  try:
    prog = get_target_prog(i)
    ck.out('Make reference output for program ' + prog['data_uoa'])

    validation_repo = get_validation_repo(i)

    stop_after_fail = i.get('stop_after_fail') == 'yes'

    processed_files = 0
    processing_files = get_dataset_files(prog)
    failed_shapes = []

    ck.out('\nProcessing...\n')
    for dataset_uoa, dataset_file in processing_files:
      ck.out('\n----------------------------------------')
      color_out(Colors.HEADER, '[{}/{}] {}:{} ...'.format(
        processed_files+1, len(processing_files), dataset_uoa, dataset_file))

      res = ck_access({'action': 'run',
                       'module_uoa': 'program',
                       'data_uoa': prog['data_uoa'],
                       'cmd_key': 'default',
                       'output_validation_repo': validation_repo,
                       'overwrite_reference_output': 'yes',
                       'dataset_uoa': dataset_uoa,
                       'dataset_file': dataset_file,
                      })

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

  except CKException as e:
    return e.ck_result
  return {'return': 0}


##############################################################################
def check_reference_outputs(i):
  '''
  Check which data-files of a specified program have saved reference ouputs.

  Params:
  output_validation_repo - output validation repo UOA
  '''
  try:
    prog = get_target_prog(i)
    ck.out('Check reference output for program ' + prog['data_uoa'])

    validation_repo = get_validation_repo(i)

    res = ck_access({'action': 'search',
                     'module_uoa': 'program.output',
                     'data_uoa': 'program-uid-' + prog['data_uid']})

    outputs_path = ''
    for item in res['lst']:
      if item['repo_uoa'] == validation_repo:
        outputs_path = item['path']
        break

    if not outputs_path:
      ck.out('No reference output found for the program')
      return {'return': 0}

    ck.out('Reference outputs are stored in ' + outputs_path)

    # TODO
    # https://github.com/ctuning/ck-nntest#output-naming-convention

  except CKException as e:
    return e.ck_result
  return {'return': 0}


########################################################################
def validate(i):
  '''
  Validate program against saved reference output

  Params
  stop_after_fail        - stop processing after first fail
  show_only_successful
  '''
  try:
    prog = get_target_prog(i)
    ck.out('Validate output for program ' + prog['data_uoa'])

    stop_after_fail = i.get('stop_after_fail') == 'yes'

    processed_files = 0
    processing_files = get_dataset_files(prog)
    failed_shapes = []
    success_shapes = []

    ck.out('\nProcessing...\n')
    for dataset_uoa, dataset_file in processing_files:
      ck.out('\n----------------------------------------')
      color_out(Colors.HEADER, '[{}/{}] {}:{} ...'.format(
        processed_files+1, len(processing_files), dataset_uoa, dataset_file))

      params = {'action': 'run',
                'module_uoa': 'program',
                'data_uoa': prog['data_uoa'],
                'cmd_key': 'default',
                'dataset_uoa': dataset_uoa,
                'dataset_file': dataset_file,
      }

      for key in i:
        if key.startswith('env.'):
          params[key] = i[key]

      res = ck_access(params)

      processed_files += 1

      if is_ok(res):
        color_out(Colors.OKGREEN, '****** OK ******')
        success_shapes.append((dataset_uoa, dataset_file))
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
    if not failed_shapes:
      ck.out('\nAll shapes OK\n')
    else:
      ck.out('Sucessfull shapes: {}'.format(len(success_shapes)))
      ck.out('Failed shapes: {}'.format(len(failed_shapes)))
      if i.get('show_only_successful'):
        ck.out('Successful shapes:')
        for p in success_shapes:
          ck.out('{}:{}'.format(p[0], p[1]))
      else:
        if 'reasons' in i:
          ck.out('Failed shapes:')
          for p in failed_shapes:
            ck.out('----------------------------------------')
            color_out(Colors.HEADER, '{}:{}'.format(p['dataset'], p['file']))
            ck.out(p['reason'])
        ck.out('\n----------------------------------------')
        ck.out('Failed shapes (summary): {}'.format(len(failed_shapes)))
        for p in failed_shapes:
          ck.out('{}:{}'.format(p['dataset'], p['file']))

  except CKException as e:
    return e.ck_result
  return {'return': 0}


##############################################################################
def list_datasets(i):
  '''
  Show datasets of program as CSV text
  '''
  try:
    prog = get_target_prog(i)
    ck.out('Datasets for program ' + prog['data_uoa'])

    processed_files = 0
    processing_files = get_dataset_files(prog, with_path = True)

    keys = []
    for dataset_uoa, dataset_file, dataset_file_path in processing_files:
      with open(dataset_file_path) as f:
        data = json.load(f)
      if not keys:
        for key in data:
          keys.append(key)
        ck.out('FILE,' + ','.join(keys))
      vals = []
      vals.append('{}:{}'.format(dataset_uoa, dataset_file))
      for key in keys:
        vals.append(str(data[key]))
      ck.out(','.join(vals))

  except CKException as e:
    return e.ck_result
  return {'return': 0}
