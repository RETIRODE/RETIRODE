using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    public class MeasuredDataItem
    {
        public float Distance { get; }
        public float Time { get; }

        public MeasuredDataItem(float distance, float time)
        {
            Distance = distance;
            Time = time;
        }
    }
}
