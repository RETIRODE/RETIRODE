using SQLite;
using SQLiteNetExtensions.Attributes;
using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    public class MeasurementItem
    {
        [PrimaryKey, AutoIncrement]
        public int Id { get; set; }

        [ForeignKey(typeof(CalibrationItem))]
        public int Calibration_id { get; set; }
        public float Tdc_value { get; set; }
        public float Angle { get; set; }

    }
}
