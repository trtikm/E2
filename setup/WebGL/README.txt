Building E2 as webassembly WebGL app
====================================

1. Install Emscripten (its sdk)
2. Install vcpkg and inside vcpkg do:
    2.1. Create the triplet: wasm32-emscripten
    2.2. Target architecture: wasm32
    2.3. Install 3rd party libraries (required by E2) under the triplet.
3. Create directory: ../../build/webgl
   The path is relative to the path of this README.txt file.
4. Copy files "initbuild.bat" and "build.bat" from this directory
   to the directory created above.
5. Modify the copied file "initbuild.bat" as described in the
   initial comments.

   
Running WebGL app
=================

1. Copy the file ./webserver.bat into the build
   directory ../../build/webgl created above.
2. Start Python's http server by executing the
   ./webserver.bat script in the build directory.
3. Open your web browser at address:
        http://localhost:8000/code/tools/
4. Navigate to directory of the tool you want to
   execute and double click on HTML file there.
