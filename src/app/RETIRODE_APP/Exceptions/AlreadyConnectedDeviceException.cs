using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.Text;

namespace RETIRODE_APP.Exceptions
{
    public class AlreadyConnectedDeviceException : Exception
    {
        public AlreadyConnectedDeviceException() { }

        public AlreadyConnectedDeviceException(string message) : base(message)
        {
        }

        public AlreadyConnectedDeviceException(string message, Exception innerException) : base(message, innerException)
        {
        }

        protected AlreadyConnectedDeviceException(SerializationInfo info, StreamingContext context) : base(info, context)
        {
        }
    }
}
