using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Models.Enums;
using RETIRODE_APP.Services;
using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.CommunityToolkit.ObjectModel;

namespace RETIRODE_APP.ViewModels
{
    public class SettingsViewModel : BaseViewModel
    {        
        public double TCDCal0 { get; set; }
        public double TCDCal62 { get; set; }
        public double TCDCal125 { get; set; }
        public int TriggerPulse { get; set; }
        public int SIPMTargetV { get; set; }
        public int SIPMActualV { get; set; }
        public int LaserTargetV { get; set; }
        public int LaserActualV { get; set; }
        public ICommand SoftwareResetCommand { get; set; }
        public ICommand CalibrateCommand { get; set; }
        public ICommand SetTriggerPulseCommand { get; set; }
        public ICommand SetTargetSimpBiasPowerVoltageCommand { get; set; }
        public ICommand SetTargetLaserPowerVolateCommand { get; set; }


        private CancellationTokenSource _poolingRoutineCancelation;
        public IRangeMeasurementService _rangeMeasurementService;
        public SettingsViewModel()
        {
            Title = "Settings";
            _rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            _rangeMeasurementService.QueryResponseEvent += RangeMeasurementService_QueryResponseEvent;

            SoftwareResetCommand = new AsyncCommand(async () => await ResetLidar());
            CalibrateCommand = new AsyncCommand(async () => await CalibrateLidar());
            SetTriggerPulseCommand = new AsyncCommand(async () => await SetTriggerPulse());
            SetTargetSimpBiasPowerVoltageCommand = new AsyncCommand(async () => await SetTargetSimpBiasPowerVoltage());
            SetTargetLaserPowerVolateCommand = new AsyncCommand(async () => await SetTargetLaserPowerVoltage());
            StartPoolingRoutine();
        }

        private async Task SetTriggerPulse()
        {
            try
            {
                await WithBusy(() => _rangeMeasurementService.SetPulseCount(TriggerPulse));
            }
            catch (Exception ex)
            {
                await ShowError("Setting pulse count failed");
            }
        }

        private void StartPoolingRoutine()
        {
            _poolingRoutineCancelation = new CancellationTokenSource();
            PoolingRoutine(_poolingRoutineCancelation.Token);
        }

        private void PoolingRoutine(CancellationToken cancellationToken)
        {
            Task.Run(async () =>
            {
                while (!cancellationToken.IsCancellationRequested)
                {
                    await _rangeMeasurementService.GetSipmBiasPowerVoltage(Voltage.Actual);
                    await _rangeMeasurementService.GetLaserVoltage(Voltage.Actual);

                    await Task.Delay(1000);
                }
            });
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

        public async Task ResetLidar()
        {            
            try
            {
                await WithBusy(() =>_rangeMeasurementService.SwReset());
            }
            catch (Exception ex)
            {
                await ShowError("Calibrating lidar failed");
            }
        }

        public async Task CalibrateLidar()
        {
            try
            {
                await WithBusy(() => _rangeMeasurementService.CalibrateLidar());
            }
            catch (Exception ex)
            {
                await ShowError("Calibrating lidar failed");
            }            
        }


        public async Task SetTargetSimpBiasPowerVoltage()
        {
            try
            {
                await WithBusy(() => _rangeMeasurementService.SetSipmBiasPowerVoltage(SIPMTargetV));
            }
            catch (Exception ex)
            {
                await ShowError("Setting SiMP bias power voltage failed");
            }            
        }

        public async Task SetTargetLaserPowerVoltage()
        {
            try
            {
                await WithBusy(() => _rangeMeasurementService.SetLaserVoltage(LaserTargetV));
            }
            catch (Exception ex)
            {
                await ShowError("Setting laser voltage failed");
            }            
        }       
    }
}
