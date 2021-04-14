using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.ViewModels
{
    class SettingsViewModel : BaseViewModel
    {
        public SettingsViewModel()
        {
            Title = "Settings";
        }

        public double TCDCal0 { get; set; }
        public double TCDCal62 { get; set; }
        public double TCDCal125 { get; set; }
        public double TriggerPulse { get; set; }
        public double VoltagePulse { get; set; }
        public double SIPMTargetV { get; set; }
        public double SIPMActualV { get; set; }
        public double LaserTargetV { get; set; }
        public double LaserActualV { get; set; }
    }
}
