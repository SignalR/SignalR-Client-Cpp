// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System.Globalization;
using Owin;
using Microsoft.AspNet.SignalR;

namespace SelfHost
{
    public class Startup
    {
        public void Configuration(IAppBuilder app)
        {
            app.Map("/raw-connection", map =>
            {
                map.RunSignalR<RawConnection>();
            });

            app.Map("/signalr", map =>
            {
                map.RunSignalR();
            });
        }
    }
}