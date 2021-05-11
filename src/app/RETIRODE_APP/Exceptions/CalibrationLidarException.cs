using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.Text;

namespace RETIRODE_APP.Exceptions
{
    public class CalibrationLidarException : Exception
    {
        public CalibrationLidarException()
        {
        }

        public CalibrationLidarException(string message) : base(message)
        {
        }

        public CalibrationLidarException(string message, Exception innerException) : base(message, innerException)
        {
        }

        protected CalibrationLidarException(SerializationInfo info, StreamingContext context) : base(info, context)
        {
        }
    }
}
