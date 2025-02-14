///
/// tests specific to -fintelfpga -fsycl w/ static libs
///

// make dummy archive
// Build a fat static lib that will be used for all tests
// RUN: echo "void foo(void) {}" > %t1.cpp
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fintelfpga %t1.cpp -c -o %t1_bundle.o
// RUN: llvm-ar cr %t.a %t1_bundle.o

/// Check phases with static lib
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fno-sycl-dead-args-optimization -fno-sycl-instrument-device-code -fno-sycl-device-lib=all -fintelfpga %t.a -ccc-print-phases 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK_PHASES %s
// CHECK_PHASES: 0: input, "[[INPUT:.+\.a]]", object, (host-sycl)
// CHECK_PHASES: 1: linker, {0}, image, (host-sycl)
// CHECK_PHASES: 2: linker, {0}, host_dep_image, (host-sycl)
// CHECK_PHASES: 3: clang-offload-deps, {2}, ir, (host-sycl)
// CHECK_PHASES: 4: input, "[[INPUT]]", archive
// CHECK_PHASES: 5: clang-offload-unbundler, {4}, tempfilelist
// CHECK_PHASES: 6: spirv-to-ir-wrapper, {5}, tempfilelist, (device-sycl)
// CHECK_PHASES: 7: linker, {3, 6}, ir, (device-sycl)
// CHECK_PHASES: 8: sycl-post-link, {7}, tempfiletable, (device-sycl)
// CHECK_PHASES: 9: file-table-tform, {8}, tempfilelist, (device-sycl)
// CHECK_PHASES: 10: llvm-spirv, {9}, tempfilelist, (device-sycl)
// CHECK_PHASES: 11: input, "[[INPUT]]", archive
// CHECK_PHASES: 12: clang-offload-unbundler, {11}, fpga_dep_list
// CHECK_PHASES: 13: backend-compiler, {10, 12}, fpga_aocx, (device-sycl)
// CHECK_PHASES: 14: file-table-tform, {8, 13}, tempfiletable, (device-sycl)
// CHECK_PHASES: 15: clang-offload-wrapper, {14}, object, (device-sycl)
// CHECK_PHASES: 16: offload, "host-sycl (x86_64-unknown-linux-gnu)" {1}, "device-sycl (spir64_fpga-unknown-unknown)" {15}, image

/// Check for unbundle and use of deps in static lib
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fno-sycl-device-lib=all -fintelfpga -Xshardware %t.a -### 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK_UNBUNDLE %s
// CHECK_UNBUNDLE: clang-offload-bundler" "-type=aoo" "-targets=sycl-fpga_dep" "-input={{.*}}" "-output=[[DEPFILES:.+\.txt]]" "-unbundle"
// CHECK_UNBUNDLE: aoc{{.*}} "-dep-files=@[[DEPFILES]]"

/// Check for no unbundle and use of deps in static lib when using triple
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fsycl -fno-sycl-device-lib=all -fsycl-targets=spir64_fpga-unknown-unknown %t.a -### 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK_NO_UNBUNDLE %s
// CHECK_NO_UNBUNDLE-NOT: clang-offload-bundler" "-type=aoo" "-targets=sycl-fpga_dep"
// CHECK_NO_UNBUNDLE-NOT: aoc{{.*}} "-dep-files={{.*}}"
