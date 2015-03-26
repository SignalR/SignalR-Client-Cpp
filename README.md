ASP.NET SignalR C++ Client 
======== 

SignalR C++ Client is a native client for the [ASP.NET SignalR](https://github.com/SignalR/SignalR/). 

####Supported platforms


The bits that ship on NuGet currently can be used in Win32/x64 native windows desktop applications built with Visual Studio 2013. 

####Get it on NuGet

`Install-Package Microsoft.AspNet.SignalR.Client.Cpp.v120.WinDesktop -Pre`

####Use it

The repo contains a separate solution (samples_VS2013.sln) with sample projects showing how to use the client to communicate with a SignalR server using Persistent Connections and Hubs.

####Nightly builds

Signed nigthly builds are available on a separate feed. You can find them [here](https://www.myget.org/gallery/aspnetvnext)

####Building the Code

* Clone the repo:

  `git clone https://github.com/aspnet/SignalR-Client-Cpp.git`

* Build from Visual Studio
  Open the signalrclient_VS2013.sln in Visual Studio 2013 and build. 
  
* Build from command line
  * Open the Developer Command Prompt for VS2013
  * Run:
    * `build.cmd /t:Build` to build the code
    * `build.cmd` to build the code and run tests
    * `build.cmd /t:CreatePackage` to build the code and create a private NuGet package
  
####Running tests

* From Visual Studio
  * to run unit tests select signalrclienttests as a start project and run
  * to run end-to-end test start the test host by selecting the signalrclient-testhost project as a start project and then select the signalrclient-e2e-tests as a startt project and run
  
* From command line
  * Open the Developer Command Prompt for VS2013
  * run `build.cmd`
