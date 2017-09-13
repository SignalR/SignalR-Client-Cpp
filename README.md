ASP.NET SignalR C++ Client
========

SignalR C++ Client is a native client for the [ASP.NET SignalR](https://github.com/SignalR/SignalR/).

### Supported platforms

The bits that ship on NuGet currently can be used in Win32/x64 native windows desktop applications built with Visual Studio 2013 or Visual Studio 2015. Note that you need to download the package that matches your Visual Studio version. If you work with Visual Studio 2013 the matching package is Microsoft.AspNet.SignalR.Client.Cpp.v120.WinDesktop if you use Visual Studio 2015 the matching package is Microsoft.AspNet.SignalR.Client.Cpp.v140.WinDesktop.

### Get it on NuGet

`Install-Package Microsoft.AspNet.SignalR.Client.Cpp.v120.WinDesktop -Pre` (version for Visual Studio 2013)

`Install-Package Microsoft.AspNet.SignalR.Client.Cpp.v140.WinDesktop -Pre` (version for Visual Studio 2015)


### Use it

The repo contains a separate solution (samples_VS2013.sln) with sample projects showing how to use the client to communicate with a SignalR server using Persistent Connections and Hubs.

### Nightly builds

Signed nigthly builds are available on a separate feed. You can find them [here](https://dotnet.myget.org/gallery/aspnetcore-ci-dev)

### Building the Code

* Clone the repo:
  `git clone https://github.com/aspnet/SignalR-Client-Cpp.git`

#### Building for Windows

* Building from Visual Studio:

  Open the signalrclient.sln in Visual Studio 2013 or Visual Studio 2015 and build.

* Building from command line:
  * Open the Developer Command Prompt for Visual Studio 2013 or Visual Studio 2015
  * Run:
    * `build.cmd /t:Build` to build the code
    * `build.cmd` to build the code and run tests
    * `build.cmd /t:CreatePackage` to build the code and create a private NuGet package for the Visual Studio version the package was built with. The package will be placed in the `artifacts\build` directory.

#### Building for Linux (Linux support is currently only experimental)
 * Clone C++ REST SDK code
 * Sync the code to a tag - e.g. `git checkout v2.7.0` (optional)
 * Build C++ REST SDK code as described [here](https://github.com/Microsoft/cpprestsdk/wiki/How-to-build-for-Linux)
 * From the root of SignalR C++ Client repo:
   * `mkdir build.release`
   * `cd build.release`
   * `CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release -DCPPREST_INCLUDE_DIR={C++ REST SDK include directory} -DCPPREST_LIB_DIR={C++ REST SDK lib directory}`

      replace `{C++ REST SDK include directory}` and `{C++ REST SDK lib directory}` with paths to corresponding C++ REST SDK folders - e.g.:

      `CXX=g++-4.8 cmake .. -DCMAKE_BUILD_TYPE=Release -DCPPREST_INCLUDE_DIR=~/source/casablanca/Release/include/ -DCPPREST_LIB_DIR=~/source/casablanca/Release/build.release/Binaries`
   * `make`
 * The binaries will be placed in the `bin` folder

### Running tests

#### Running tests on Windows

* From Visual Studio
  * to run unit tests select signalrclienttests as a start project and run. Alternatively you can install the Google Test runner extension for Visual Studio and run the tests from the test explorer.
  * to run end-to-end test start the test host by selecting the signalrclient-testhost project as a start project and then select the signalrclient-e2e-tests as a start project and run.

* From command line
  * Open the Developer Command Prompt for Visual Studio 2013 or Visual Studio 2015
  * run `build.cmd`

#### Running tests on Linux
* Build the code
* Run `signalrclienttests`
* Known issues:
  * A few tests randomly fail (needs investigation)
  * Oftentimes the process does not exit even though all tests appear to have completed (needs investigation)

