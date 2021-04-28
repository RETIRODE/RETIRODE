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
            //SetVariables();
            Title = "Settings";

            rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            rangeMeasurementService.QueryResponseEvent += RangeMeasurementService_QueryResponseEvent;

            SWReset = new Command(onSWReset);
            CalibrateCommand = new Command(onCalibrate);
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
                    LaserTargetV = Convert.ToDouble(responseItem.Value);
                    break;

                case RangeFinderValues.LaserVoltageActual:
                    LaserActualV = Convert.ToDouble(responseItem.Value);
                    break;

                case RangeFinderValues.SipmBiasPowerVoltageTarget:
                    SIPMTargetV = Convert.ToDouble(responseItem.Value);
                    break;

                case RangeFinderValues.SipmBiasPowerVoltageActual:
                    SIPMActualV = Convert.ToDouble(responseItem.Value);
                    break;
            }
        }

        public async void onSWReset()
        {
            await rangeMeasurementService.SwReset();
        }

        public async void onCalibrate()
        {
            await rangeMeasurementService.CalibrateLidar();
        }

        public double TCDCal0 { get; set; }
        public double TCDCal62 { get; set; }
        public double TCDCal125 { get; set; }
        public int TriggerPulse { get;set; }
        public double VoltagePulse { get; set; }
        public double SIPMTargetV { get; set; }
        public double SIPMActualV { get; set; }
        public double LaserTargetV { get; set; }
        public double LaserActualV { get; set; }
    }
}
 