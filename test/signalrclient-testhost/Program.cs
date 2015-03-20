// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System;
using System.Globalization;
using System.Threading;
using Microsoft.Owin.Hosting;

namespace SelfHost
{
    public class Program
    {
        static void Main(string[] args)
        {
            using (WebApp.Start<Startup>("http://localhost:8081"))
            {
                Console.WriteLine("Server running at http://localhost:8081/");
                Thread.Sleep(args.Length > 0 ? int.Parse(args[0]) : Timeout.Infinite);
            }
        }
    }
}
