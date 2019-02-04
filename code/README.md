This is another version of Yan's coincidence code.

The code is cleaned up and uses the bazel build system.


The code should be compiled from a directory that contains a `WORKSPACE` file.


## compile the source code within the directory that has the BUILD config in it.
```
bazel build co_yan --verbose_failures --sandbox_debug
```

## running the compiled file with this command.
```
bazel-bin/code/co_yan
```


There are still more todos. A binary for `cross-year` still needs to be created.
We need to centralize some of the duplicate code.
Many of the functions created are member functions and should be added as part of a class.
New files with proper structure should be added.

A proper logging system should also be added.



