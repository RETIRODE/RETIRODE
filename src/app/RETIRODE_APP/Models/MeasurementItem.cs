using SQLite;
using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    public class MeasurementItem
    {
        [PrimaryKey, AutoIncrement]
        public int Id { get; set; }
        public CalibrationItem CalibrationItem { get; set; }

        public float Tdc_value { get; set; }
        public float Angle { get; set; }

    }
}
