# PyTiLT

Python bindings for TiLT

## Building PyTiLT From Source

Ensure that LLVM and TiLT have been installed.
Update your ``LD_LIBRARY_PATH`` variable to include the directory containing the TiLT shared library object file (i.e. ``libtilt.so``).

(Temporary) Update the ``LLVM_DIR`` and ``TILT_DIR`` variables in setup.py and run

```
python3 setup.py install
```