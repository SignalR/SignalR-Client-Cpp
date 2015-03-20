// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Threading;
using Microsoft.AspNet.SignalR;
using Newtonsoft.Json;

namespace SelfHost
{
    public class RawConnection : PersistentConnection
    {
        protected override Task OnReceived(IRequest request, string connectionId, string data)
        {
            var message = JsonConvert.DeserializeObject<Message>(data);

            switch (message.Type)
            {
                case MessageType.String:
                    Connection.Send(connectionId, new { type = MessageType.String, data = message.Value });
                    break;
                default:
                    break;
            }

            return base.OnReceived(request, connectionId, data);
        }

        enum MessageType
        {
            String
        }

        class Message
        {
            public MessageType Type { get; set; }
            public string Value { get; set; }
        }
    }
}