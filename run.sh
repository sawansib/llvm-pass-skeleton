# setup env variable 
export PATH="/work/zhang-x1/common/install/llvm-8.0/build/bin/":$PATH
export LLVM_DIR=/work/zhang-x1/common/install/llvm-8.0/build/lib/cmake/llvm/
# compile pass to generate shared lib
rm -rf build; mkdir build; cd build/; cmake ../; make; cd -
# compile to llvm bitcode with clang fromtemd
clang -c -emit-llvm -O0 -Xclang -disable-O0-optnone example.c -o original.bc
# generate llvm ir from bitcode
llvm-dis original.bc 
# optimze with llvm opt
opt -S -load build/skeleton/libSkeletonPass.so -mem2reg -sr original.ll -o opt.ll
opt -S -load build/skeleton/libSkeletonPass.so -mem2reg -sr -dce original.ll -o opt.bc
llc -filetype=obj opt.bc; gcc opt.o; ./a.out
