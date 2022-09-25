@rem replace <path-to-emsdk> by the install path of emscripten's sdk
@rem replace <path-to-vcpkg> by the install path of vcpkg
@rem replace <build-type> by either Debug or Release
@emcmake cmake ../.. "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=<path-to-emsdk>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" "-DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake" "-DVCPKG_TARGET_ARCHITECTURE=wasm32" "-DVCPKG_TARGET_TRIPLET=wasm32-emscripten" "-DCMAKE_BUILD_TYPE=<build-type>" "-DCMAKE_INSTALL_PREFIX=../../dist/"
