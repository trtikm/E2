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
   to the directory created above. Optionally, you can also copy
   the file "install.bat", in case you want to install the binaries
   from the build to the "dist" directory.
5. Modify the copied file "initbuild.bat" as described in the
   initial comments.
6. Open terminal (cmd.exe) in the created directory "../../build/webgl".
   Then execute there these commands:
        initbuild.bat
        build.bat
        install.bat     <--- this is optional
   
Running WebGL app
=================

A) Using Emscripten's 'emrun' utility:
    1. Copy the file "run.bat" into the build
       directory "../../build/webgl" created above.
    2. Modify the copied file "run.bat" as described in the
       initial comment.
    3. Run the file "run.bat"
    4. Once you close the browser's window, you can close
       also the corresponding terminal.

B) Using Python's web server:
    1. Copy the file ./webserver.bat into the build
       directory ../../build/webgl created above.
    2. Start Python's http server by executing the
       ./webserver.bat script in the build directory.
    3. Open your web browser at address:
            http://localhost:8000/code/tools/
    4. Navigate to directory of the tool you want to
       execute and double click on HTML file there.
