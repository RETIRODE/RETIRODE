using System;
using System.Collections.Generic;
using System.Text;
using SQLite;

namespace RETIRODE_APP.Models
{
    public class CalibrationItem : BaseItem
    {
        [PrimaryKey, AutoIncrement]
        public int Id { get; set; }
        public DateTime DateTime { get; set; }
        public float Tdc_0 { get; set; }
        public float Tdc_62 { get; set; }
        public float Tdc_125 { get; set; }
        public int Pulse_count { get; set; }

    }
}
