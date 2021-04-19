using System;
using System.Windows.Input;
using Xamarin.Forms;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Collections.Generic;
using Xamarin.CommunityToolkit.ObjectModel;
using System.Threading;

namespace RETIRODE_APP.ViewModels
{
    class SettingsViewModel : BaseViewModel
    {
        public Command SWReset { get; set; }
        public Command Calibrate { get; set; }

        public SettingsViewModel()
        {
            Title = "Settings";
            SWReset = new Command(onSWReset);
            Calibrate = new Command(onCalibrate);
        }

        public void onSWReset()
        {
            throw new NotImplementedException();
        }

        public void onCalibrate()
        {
            throw new NotImplementedException();
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
 