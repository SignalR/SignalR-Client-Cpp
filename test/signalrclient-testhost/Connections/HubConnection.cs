// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.AspNet.SignalR;

namespace SelfHost
{
    public class HubConnection : Hub
    {
        public IEnumerable<int> ForceReconnect()
        {
            yield return 1;
            // throwing here will close the websocket which should trigger reconnect
            throw new Exception();
        }

        public void DisplayMessage(string message)
        {
            Clients.Caller.displayMessage("Send: " + message);
        }

        public string ReturnMessage(string message)
        {
            return message;
        }
    }
}