# nntest helper module

The helper module for [ck nntest](https://github.com/ctuning/ck-nntest) project.

## Commands

### `make_reference_outputs`

```bash
ck make_reference_outputs ch-nntest:${PROGRAM_UOA} --output_validation_repo=ck-nntest-outputs
```

Iterates over all files of all datasets of given program and stores reference output into `output_validation_repo` if specified or into `local` otherwise. It implements this simple algorithm:

```bash
for $dataset in `ck search dataset --tags=tags,from,program,meta`:
  for $file in dataset:
    ck run program:program-uoa \
      --cmd_key=default \
      --dataset_uoa=$dataset \
      --dataset_file=$file \
      --output_validation_repo=ck-nntest-outputs \
      --overwrite_reference_output
```

Example:

```bash
ck make_reference_outputs ch-nntest:conv-tensorflow-cpu --output_validation_repo=ck-nntest-outputs
```

### `check_reference_outputs (TBD)`

```bash
ck check_reference_outputs ch-nntest:${PROGRAM_UOA} --output_validation_repo=ck-nntest-outputs
```

Check which data-files of a specified program have saved reference ouputs.

Example:

```bash
$ ck check_reference_outputs ch-nntest:conv-tensorflow-cpu --output_validation_repo=ck-nntest-outputs

```

### `validate`
```
ck validate ch-nntest:${PROGRAM_UOA}
```
Iterates over all files of all datasets of given program and runs program against each file, then analyzes program run result and calculates success statistics and fail reasons.

### `list_datasets`

```bash
ck list_datasets ch-nntest:${PROGRAM_UOA}
```

Prints content of all files of all datasets of given program as one CSV-text.

Example:

```bash
$ ck datasets ch-nntest:softmax-tensorflow-cpu
Datasets for program softmax-tensorflow-cpu
Program dataset tags: dataset,nntest,tensor-softmax
Datasets found: 2
Files to process: 4
FILE,CK_IN_SHAPE_W,CK_IN_SHAPE_C,CK_IN_SHAPE_H
tensor-softmax-imagenet:shape-1000-1-1,1,1000,1
tensor-softmax-0001:shape-1024-1-1,1,1024,1
tensor-softmax-0001:shape-4096-1-1,1,4096,1
tensor-softmax-0001:shape-5250000-1-1,1,5250000,1
```