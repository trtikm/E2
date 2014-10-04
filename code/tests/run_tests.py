import argparse
import os
import glob
import subprocess

def parse_cmd_line():
    parser = argparse.ArgumentParser()
    parser.add_argument("-D", "--root_dir", type=str, default="./",
                        help="A directory in which E2 tests are located. Specify this argument "
                             "only in the case you moved this script file aways from the tests files.")
    parser.add_argument("-R","--remove_logs", action="store_true",
                        help="Removes all log files (i.e. all *.html, and *.txt) in the tests directory. "
                             "Then the script terminates.")
    parser.add_argument("-I","--include", nargs='+', type=str, default="",
                        help="A list of tests (names of executable files) which should be included to "
                             "the testing. When the argument is ommmited, then all executables in the "
                             "dirrectory specified by '--root_dir' will be automatically included.")
    parser.add_argument("-E","--exclude", nargs='+', type=str, default="",
                        help="A list of tests (names of executable files) which should be excluded from "
                             "the testing.")
    args = parser.parse_args()
    return args

def collectExecutavblesOfTests(tests_root_dir):
    tests = []
    tests_dbg = []
    for name in os.listdir(tests_root_dir):
        pathname = os.path.join(tests_root_dir,name)
        if os.path.isfile(pathname) and os.access(pathname,os.X_OK):
            if name.endswith("_Release"):
                tests.append(name)
            elif name.endswith("_Debug"):
                tests_dbg.append(name)
    for name in tests_dbg:
        releaseName = name[:-6]+"_Release"
        if not (releaseName in tests):
            tests.append(name)
    return tests

def removeExcludedTests(tests,excluded):
    for fname in excluded:
        if fname in tests:
           tests.remove(fname)
        elif  fname.startswith("./") and (fname[2:] in tests):
            tests.remove(fname[2:])
    return tests

def run(test, tests_root_dir):
    print "*** Starting: " + test
    return subprocess.call([tests_root_dir + test]) == 0

def scriptMain():
    args = parse_cmd_line()
    tests_root_dir = args.root_dir

    if args.remove_logs:
        for fname in glob.glob(tests_root_dir + "*.html"):
            if os.path.isfile(fname):
                print "Deleting: " + fname
                os.remove(fname)
            else:
                print "ERROR: path does not refer to a file: " + fname
        for fname in glob.glob(tests_root_dir + "*.txt"):
            if os.path.isfile(fname):
                print "Deleting: " + fname
                os.remove(fname)
            else:
                print "ERROR: path does not refer to a file: " + fname
        return

    if len(args.include) > 0:
        executables = args.include
    else:
        executables = collectExecutavblesOfTests(tests_root_dir)
    executables = removeExcludedTests(executables,args.exclude)

    print "Configuration:"
    print "test root directory: " + tests_root_dir
    print "tests to be executes: "
    for test in executables:
        print "    " + test
    print "Executing the tests..."

    successfull = []
    failed = []
    for test in executables:
        if run(test, tests_root_dir):
            successfull.append(test)
        else:
            failed.append(test)
    if len(failed) == 0:
        print "*** The testing was SUCCESSFULL."
    else:
        print "*** The testing has FAILED. These tests have failed:"
        for name in failed:
            print "  " + name
        print "*** See log files of these test to get closer details about the failures."

if __name__ == "__main__":
    scriptMain()
