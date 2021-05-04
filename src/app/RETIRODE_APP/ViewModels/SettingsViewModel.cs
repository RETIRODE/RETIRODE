using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Models.Enums;
using RETIRODE_APP.Services;
using RETIRODE_APP.Views;
using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.CommunityToolkit.ObjectModel;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class SettingsViewModel : BaseViewModel
    {
        #region GUI Properties

        private double _tcdcal0;
        public double TCDCal0 { 
            get { return _tcdcal0;  }
            set
            {
                _tcdcal0 = value;
                OnPropertyChanged(nameof(TCDCal0));
            }
        }
        private double _tcdcal62;
        public double TCDCal62
        {
            get { return _tcdcal62; }
            set
            {
                _tcdcal62 = value;
                OnPropertyChanged(nameof(TCDCal62));
            }
        }
        private double _tcdcal125;
        public double TCDCal125
        {
            get { return _tcdcal125; }
            set
            {
                _tcdcal125 = value;
                OnPropertyChanged(nameof(TCDCal125));
            }
        }
        private int _triggerpulse;
        public int TriggerPulse
        {
            get { return _triggerpulse; }
            set
            {
                _triggerpulse = value;
                OnPropertyChanged(nameof(TriggerPulse));
            }
        }
        private int _sipmtargetv;
        public int SIPMTargetV
        {
            get { return _sipmtargetv; }
            set
            {
                _sipmtargetv = value;
                OnPropertyChanged(nameof(SIPMTargetV));
            }
        }
        private int _sipmactualv;
        public int SIPMActualV
        {
            get { return _sipmactualv; }
            set
            {
                _sipmactualv = value;
                OnPropertyChanged(nameof(SIPMActualV));
            }
        }
        private int _lasertargetv;
        public int LaserTargetV
        {
            get { return _lasertargetv; }
            set
            {
                _lasertargetv = value;
                OnPropertyChanged(nameof(LaserTargetV));
            }
        }
        private int _laseractualv;
        public int LaserActualV
        {
            get { return _laseractualv; }
            set
            {
                _laseractualv = value;
                OnPropertyChanged(nameof(LaserActualV));
            }
        }

        #endregion

    {        
        public ICommand SoftwareResetCommand { get; set; }
        public ICommand CalibrateCommand { get; set; }
        public ICommand SetTriggerPulseCommand { get; set; }
        public ICommand SetTargetSimpBiasPowerVoltageCommand { get; set; }
        public ICommand SetTargetLaserPowerVolateCommand { get; set; }
        public ICommand StartDepictionCommand { get; set; }


        private CancellationTokenSource _poolingRoutineCancelation;
        public IRangeMeasurementService _rangeMeasurementService;
        public SettingsViewModel()
        {
            Title = "Settings";
            _rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();

            _rangeMeasurementService.QueryResponseEvent -= RangeMeasurementService_QueryResponseEvent;
            _rangeMeasurementService.QueryResponseEvent += RangeMeasurementService_QueryResponseEvent;

            SoftwareResetCommand = new AsyncCommand(async () => await ResetLidar());
            CalibrateCommand = new AsyncCommand(async () => await CalibrateLidar());
            SetTriggerPulseCommand = new AsyncCommand(async () => await SetTriggerPulse());
            SetTargetSimpBiasPowerVoltageCommand = new AsyncCommand(async () => await SetTargetSimpBiasPowerVoltage());
            SetTargetLaserPowerVolateCommand = new AsyncCommand(async () => await SetTargetLaserPowerVoltage());
            StartDepictionCommand = new AsyncCommand(async () => await StartDepiction());
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
                    try
                    {
                        await _rangeMeasurementService.GetSipmBiasPowerVoltage(Voltage.Actual);
                        await _rangeMeasurementService.GetLaserVoltage(Voltage.Actual);
                        await Task.Delay(1000);
                    }
                    catch ( Exception ex)
                    {
                        await ShowError("Could not get data");
                        if (await ShowDialog("Load data again?"))
                        {
                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }
                    
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
                await ShowError("Reseting lidar failed");
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
        

        private async Task StartDepiction()
        {
            if(TCDCal0 != 0 && TCDCal62 != 0 && TCDCal125 != 0)
            {
                App.isCalibrated = true;
                await Application.Current.MainPage.Navigation.PushAsync(new DepictionPage());
            }
            else
            {
                await ShowError("Lidar not calibrated");
            }

        }
    }
}
