using System;
using System.Windows.Input;
using Xamarin.Forms;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Collections.Generic;
using Xamarin.CommunityToolkit.ObjectModel;
using System.Threading;
using RETIRODE_APP.Services;
using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Models.Enums;

namespace RETIRODE_APP.ViewModels
{
    class SettingsViewModel : BaseViewModel
    {
        public IRangeMeasurementService rangeMeasurementService;

        public Command SWReset { get; set; }
        public Command CalibrateCommand { get; set; }

        public SettingsViewModel()
        {
            SetVariables();
            Title = "Settings";

            rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            rangeMeasurementService.QueryResponseEvent += RangeMeasurementService_QueryResponseEvent;

            SWReset = new Command(OnSWReset);
            CalibrateCommand = new Command(OnCalibrate);
        }

        private async void SetVariables()
        {
            await rangeMeasurementService.GetCalibration(Calibrate.NS0);
            await rangeMeasurementService.GetCalibration(Calibrate.NS62_5);
            await rangeMeasurementService.GetCalibration(Calibrate.NS125);

            await rangeMeasurementService.GetSipmBiasPowerVoltage(Voltage.Actual);
            await rangeMeasurementService.GetSipmBiasPowerVoltage(Voltage.Target);
            await rangeMeasurementService.GetLaserVoltage(Voltage.Actual);
            await rangeMeasurementService.GetLaserVoltage(Voltage.Target);
        }

        private void RangeMeasurementService_QueryResponseEvent(ResponseItem responseItem)
        {
            switch (responseItem.Identifier)
            {
                case RangeFinderValues.CalibrateNS0:
                    TCDCal0 = Convert.ToDouble(responseItem.Value);
                    break;

                case RangeFinderValues.Calibrate62_5:
                    TCDCal62 = Convert.ToDouble(responseItem.Value);
                    break;

                case RangeFinderValues.Calibrate125:
                    TCDCal125 = Convert.ToDouble(responseItem.Value);
                    break;

                case RangeFinderValues.LaserVoltageTarget:
                    LaserTargetV = Convert.ToInt32(responseItem.Value);
                    break;

                case RangeFinderValues.LaserVoltageActual:
                    LaserActualV = Convert.ToInt32(responseItem.Value);
                    break;

                case RangeFinderValues.SipmBiasPowerVoltageTarget:
                    SIPMTargetV = Convert.ToInt32(responseItem.Value);
                    break;

                case RangeFinderValues.SipmBiasPowerVoltageActual:
                    SIPMActualV = Convert.ToInt32(responseItem.Value);
                    break;
            }
        }

        public async void OnSWReset()
        {
            await rangeMeasurementService.SwReset();
        }

        public async void OnCalibrate()
        {
            await rangeMeasurementService.CalibrateLidar();
        }

        public async void OnSetPulseCount(object sender, FocusEventArgs e)
        {
            await rangeMeasurementService.SetPulseCount(TriggerPulse);
        }

        public async void OnSetSipmVoltage(object sender, FocusEventArgs e)
        {
            await rangeMeasurementService.SetSipmBiasPowerVoltage(SIPMTargetV);
        }

        public async void OnSetLaserVoltage(object sender, FocusEventArgs e)
        {
            await rangeMeasurementService.SetLaserVoltage(LaserTargetV);
        }



        public double TCDCal0 { get; set; }
        public double TCDCal62 { get; set; }
        public double TCDCal125 { get; set; }
        public int TriggerPulse { get; set; }
        public int SIPMTargetV { get; set; }
        public int SIPMActualV { get; set; }
        public int LaserTargetV { get; set; }
        public int LaserActualV { get; set; }
    }
}
 