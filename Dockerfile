# Use a build argument to specify the LLVM version tag from the silkeh/clang image
FROM docker.io/silkeh/clang:${LLVM_V}

WORKDIR /usr/local/src

CMD ["bash"]
