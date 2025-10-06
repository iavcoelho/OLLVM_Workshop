# Workshop: OLLVM for the rest of Us

This repository contains slides and hands-on materials for the OLLVM workshop. The workshop targets a technical audience with minimal security experience and teaches the fundamentals through practical, self-contained tasks. Topics include:

* Simple LLVM Hello World 
* Functions names listing
* Function, Instruction and Basic Block Body Listing
* Simple program operation modification
* Arithmetic Obfuscation
* Basic Block Duplicate
* Opaque Predicate Insertion
* Control Flow Flattening

## Prerequisites
* Basic knowledge of C/C++ programming
* Docker/Podman installed on your machine

## Getting Started
1. Clone the repository:
```bash
git clone https://github.com/yourusername/OLLVM_Workshop.git
cd OLLVM_Workshop
```

2. Build the Docker image for OLLVM pass compilation:
```bash
make pod-build
```

3. Compile passes:
```bash
make <pass-name>
```

4. Test the pass against the provided test program:
```bash
make test
```

## License
This project is licensed under the MIT License. See the `LICENSE` file for details.
