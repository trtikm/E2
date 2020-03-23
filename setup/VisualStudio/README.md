# CUSTOM DEBUG VISUALISERS

In order to maximise effectiveness of debugging E2 in Visual Studio it is
highly recommeded to copy custom visualisers:
    ./E2.natvis
    .{Eigen-install-dir}/debug/msvc/eigen.natvis
to the directory: 
    c:/Users/{your-user-name}/Documents/Visual Studio 2019/Visualizers
If the directory does not exist, then create it. The visualisers should
also work for other version of Visual Studio than 2019.

NOTE: If you are building your own visualiser, then you can force Visual
Studio to reload all visualisers and reload all debugger windows by typing
the command:
    .natvisreload
in a Watch window.

# SETUP LAUNCH CONFIGURATIONS

 In main menu choose `Debug/Debug and Launch Settings for {AnExecutableTarget}`
 to open the file:
    {E2}\.vs\launch.vs.json
Insert your desired launch configurations into that file. You can find example
content of the file in:
    ./launch.vs.json
