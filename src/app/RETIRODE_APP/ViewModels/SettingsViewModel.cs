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

        private bool _isSipmBiasPowerVoltageTurnOn;
        public bool IsSipmBiasPowerVoltageTurnOn
        {
            get { return _isSipmBiasPowerVoltageTurnOn; }
            set
            {
                _isSipmBiasPowerVoltageTurnOn = value;
                OnPropertyChanged(nameof(IsSipmBiasPowerVoltageTurnOn));

                if (value is false)
                {
                    BiasVoltageOverload = null;
                }
            }
        }
        public bool _isLaserVoltageTurnOn;
        public bool IsLaserVoltageTurnOn
        {
            get { return _isLaserVoltageTurnOn; }
            set
            {
                _isLaserVoltageTurnOn = value;
                OnPropertyChanged(nameof(IsLaserVoltageTurnOn));

                if (value is false)
                {
                    LaserVoltageOverload = null;
                }
            }
        }
        private float _tcdcal0;
        public float TCDCal0
        {
            get { return _tcdcal0; }
            set
            {
                _tcdcal0 = value;
                OnPropertyChanged(nameof(TCDCal0));
            }
        }
        private float _tcdcal62;
        public float TCDCal62
        {
            get { return _tcdcal62; }
            set
            {
                _tcdcal62 = value;
                OnPropertyChanged(nameof(TCDCal62));
            }
        }
        private float _tcdcal125;
        public float TCDCal125
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
        private float _sipmtargetv;
        public float SIPMTargetV
        {
            get { return _sipmtargetv; }
            set
            {
                _sipmtargetv = value;
                OnPropertyChanged(nameof(SIPMTargetV));
            }
        }
        private float _sipmactualv;
        public float SIPMActualV
        {
            get { return _sipmactualv; }
            set
            {
                _sipmactualv = value;
                OnPropertyChanged(nameof(SIPMActualV));
            }
        }
        private float _lasertargetv;
        public float LaserTargetV
        {
            get { return _lasertargetv; }
            set
            {
                _lasertargetv = value;
                OnPropertyChanged(nameof(LaserTargetV));
            }
        }
        private float _laseractualv;
        public float LaserActualV
        {
            get { return _laseractualv; }
            set
            {
                _laseractualv = value;
                OnPropertyChanged(nameof(LaserActualV));
            }
        }

        private bool? _laserVolatageOverload;
        public bool? LaserVoltageOverload
        {
            get { return _laserVolatageOverload; }
            set
            {
                _laserVolatageOverload = value;
                OnPropertyChanged(nameof(LaserVoltageOverload));
            }
        }

        private bool? _biasVolatageOverload;
        public bool? BiasVoltageOverload
        {
            get { return _biasVolatageOverload; }
            set
            {
                _biasVolatageOverload = value;
                OnPropertyChanged(nameof(BiasVoltageOverload));
            }
        }

        #endregion


        public ICommand SoftwareResetCommand { get; set; }
        public ICommand CalibrateCommand { get; set; }
        public ICommand SetTriggerPulseCommand { get; set; }
        public ICommand SetTargetSimpBiasPowerVoltageCommand { get; set; }
        public ICommand SetTargetLaserPowerVolateCommand { get; set; }
        public ICommand StartDepictionCommand { get; set; }
        public ICommand SipmBiasPowerToggleCommand { get; set; }
        public ICommand LaserVoltageToggleCommand { get; set; }


        private CancellationTokenSource _poolingRoutineCancelation;
        public IRangeMeasurementService _rangeMeasurementService;
        private IDataStore _database;

        // false -> switch set by system
        // true -> switch set by user
        private bool _switchedBySystemLaserVolage = false;
        private bool _switchedBySystemSipmBias = false;

        public SettingsViewModel()
        {
            Title = "Settings";
            _rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            _database = TinyIoCContainer.Current.Resolve<IDataStore>();

            _rangeMeasurementService.QueryResponseEvent -= RangeMeasurementService_QueryResponseEvent;
            _rangeMeasurementService.QueryResponseEvent += RangeMeasurementService_QueryResponseEvent;

            SoftwareResetCommand = new AsyncCommand(async () => await ResetLidar());
            CalibrateCommand = new AsyncCommand(async () => await CalibrateLidar());
            SetTriggerPulseCommand = new AsyncCommand(async () => await SetTriggerPulse());
            SetTargetSimpBiasPowerVoltageCommand = new AsyncCommand(async () => await SetTargetSimpBiasPowerVoltage());
            SetTargetLaserPowerVolateCommand = new AsyncCommand(async () => await SetTargetLaserPowerVoltage());
            StartDepictionCommand = new AsyncCommand(async () => await StartDepiction());
            SipmBiasPowerToggleCommand = new AsyncCommand(async () => await SipmBiasPowerToggle());
            LaserVoltageToggleCommand = new AsyncCommand(async () => await LaserVoltageToggle());
            StartPoolingRoutine();
        }

        private async Task LaserVoltageToggle()
        {
            if (_switchedBySystemLaserVolage)
            {
                _switchedBySystemLaserVolage = false;
                return;
            }

            if (IsLaserVoltageTurnOn)
            {
                try
                {
                    await WithBusy(() => _rangeMeasurementService.SwitchLaserVoltage(SwitchState.TurnOn));
                }
                catch (Exception ex)
                {
                    ChangeLaserVoltagePoweredOn(false);
                    await ShowError("Failed to turn on/off laser voltage");
                }
            }
            else
            {
                try
                {
                    await WithBusy(() => _rangeMeasurementService.SwitchLaserVoltage(SwitchState.TurnOff));
                }
                catch (Exception ex)
                {
                    ChangeLaserVoltagePoweredOn(true);
                    await ShowError("Failed to turn on/off laser voltage");
                }
            }

        }

        private async Task SipmBiasPowerToggle()
        {
            if (_switchedBySystemSipmBias)
            {
                _switchedBySystemSipmBias = false;
                return;
            }
            if (IsSipmBiasPowerVoltageTurnOn)
            {
                try
                {
                    await WithBusy(() => _rangeMeasurementService.SwitchSipmBiasVoltage(SwitchState.TurnOn));
                }
                catch (Exception ex)
                {
                    ChangeSipmBiasVoltagePoweredOn(false);
                    await ShowError("Failed to turn on/off sipm bias power voltage");
                }
            }
            else
            {
                try
                {
                    await WithBusy(() => _rangeMeasurementService.SwitchSipmBiasVoltage(SwitchState.TurnOff));
                }
                catch (Exception ex)
                {
                    ChangeSipmBiasVoltagePoweredOn(true);
                    await ShowError("Failed to turn on/off sipm bias power voltage");
                }
            }
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
            Device.BeginInvokeOnMainThread(async () =>
            {
                while (!cancellationToken.IsCancellationRequested)
                {
                    try
                    {
                        await Task.Delay(1000);
                        await _rangeMeasurementService.GetSipmBiasPowerVoltage(Voltage.Actual);
                        await _rangeMeasurementService.GetLaserVoltage(Voltage.Actual);
                        await _rangeMeasurementService.GetVoltagesStatus();

                        if (TriggerPulse == default(int))
                        {
                            //await _rangeMeasurementService.GetPulseCount();
                        }
                        await Task.Delay(1000);

                        await _rangeMeasurementService.GetSipmBiasPowerVoltage(Voltage.Target);
                        await _rangeMeasurementService.GetLaserVoltage(Voltage.Target);
                    }
                    catch (Exception ex)
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
                    TCDCal0 = (float)(responseItem.Value);
                    break;

                case RangeFinderValues.Calibrate62_5:
                    TCDCal62 = (float)(responseItem.Value);
                    break;

                case RangeFinderValues.Calibrate125:
                    TCDCal125 = (float)(responseItem.Value);
                    break;

                case RangeFinderValues.LaserVoltageTarget:
                    LaserTargetV = (float)responseItem.Value;
                    break;

                case RangeFinderValues.LaserVoltageActual:
                    LaserActualV = (float)responseItem.Value;
                    break;

                case RangeFinderValues.SipmBiasPowerVoltageTarget:
                    SIPMTargetV = (float)responseItem.Value;
                    break;

                case RangeFinderValues.SipmBiasPowerVoltageActual:
                    SIPMActualV = (float)responseItem.Value;
                    break;

                case RangeFinderValues.PulseCount:
                    TriggerPulse = Convert.ToInt32(responseItem.Value);
                    break;
                case RangeFinderValues.SipmBiasPowerVoltageStatus:
                    ChangeSipmBiasVoltagePoweredOn(Convert.ToBoolean(responseItem.Value));
                    break;
                case RangeFinderValues.LaserVoltageStatus:
                    ChangeLaserVoltagePoweredOn(Convert.ToBoolean(responseItem.Value));
                    break;
                case RangeFinderValues.LaserVoltageOverload:
                    if (IsLaserVoltageTurnOn)
                    {
                        LaserVoltageOverload = Convert.ToBoolean(responseItem.Value);
                    }
                    else
                    {
                        LaserVoltageOverload = null;
                    }
                    break;
                case RangeFinderValues.SipmBiasPowerVoltageOverload:
                    if (IsSipmBiasPowerVoltageTurnOn)
                    {
                        BiasVoltageOverload = Convert.ToBoolean(responseItem.Value);
                    }
                    else
                    {
                        BiasVoltageOverload = null;
                    }
                    break;
                default:
                    break;
            }
        }

        public async Task ResetLidar()
        {
            try
            {
                await WithBusy(() => _rangeMeasurementService.SwReset());
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
            if (!App.isConnected)
            {
                await Application.Current.MainPage.Navigation.PushAsync(new BluetoothPage());
            }
            else
            {
                if (TCDCal0 != 0 && TCDCal62 != 0 && TCDCal125 != 0)
                {
                    App.isCalibrated = true;
                    await _database.AddEntityAsync(new CalibrationItem()
                    {
                        DateTime = DateTime.Now,
                        Tdc_0 = TCDCal0,
                        Tdc_62 = TCDCal62,
                        Tdc_125 = TCDCal125,
                        Pulse_count = TriggerPulse
                    });
                    await Application.Current.MainPage.Navigation.PushAsync(new DepictionPage());
                }
                else
                {
                    await ShowError("Lidar not calibrated");
                }
            }


        }
        private void ChangeLaserVoltagePoweredOn(bool value)
        {
            if (IsLaserVoltageTurnOn != value)
            {
                _switchedBySystemLaserVolage = true;
                IsLaserVoltageTurnOn = value;
            }
        }

        private void ChangeSipmBiasVoltagePoweredOn(bool value)
        {
            if (IsSipmBiasPowerVoltageTurnOn != value)
            {
                _switchedBySystemSipmBias = true;
                IsSipmBiasPowerVoltageTurnOn = value;
            }
        }

    }
}
