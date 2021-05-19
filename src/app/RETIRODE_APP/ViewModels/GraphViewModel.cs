using Nancy.TinyIoc;

using Plugin.Permissions;
using Plugin.Permissions.Abstractions;
using Plugin.BLE.Abstractions.EventArgs;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services.Interfaces;
using RETIRODE_APP.Views;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.CommunityToolkit.ObjectModel;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    class GraphViewModel : BaseViewModel
    {
        private IDataStore _dataStore;
        private IRangeMeasurementService _rangeMeasurementService;
        protected readonly IApplicationStateProvider _applicationStateProvider;

        private DateTime StartMeasuringTime { get; set; }
        private CalibrationItem Calibration { get; set; }
        public ObservableCollection<MeasuredDataItem> MeasuredDataItems { get; set; }
        public ICommand StartStopCommand { get; set; }
        public ICommand GraphResetCommand { get; set; }
        public ICommand ExportCommand { get; set; }
        public string CurrentIcon { get; set; }

        private bool _measurement;
        public bool Measurement
        {
            get { return _measurement; }
            set
            {
                _measurement = value;
                OnPropertyChanged(nameof(Measurement));
            }
        }

        public GraphViewModel()
        {
            StartMeasuringTime = DateTime.Now;
            MeasuredDataItems = new ObservableCollection<MeasuredDataItem>();
            StartStopCommand = new AsyncCommand(async () => await StartStopMeasurement());
            GraphResetCommand = new AsyncCommand(async () => await GraphReset());
            ExportCommand = new AsyncCommand(async () => await ExportToFile());
            _rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            _dataStore = TinyIoCContainer.Current.Resolve<IDataStore>();

            _applicationStateProvider = TinyIoCContainer.Current.Resolve<IApplicationStateProvider>();
            Measurement = true;

            _rangeMeasurementService.MeasuredDataResponseEvent -= _rangeMeasurementService_MeasuredDataResponseEvent;
            _rangeMeasurementService.MeasuredDataResponseEvent += _rangeMeasurementService_MeasuredDataResponseEvent;

            _rangeMeasurementService.DeviceDisconnectedEvent -= _rangeMeasurementService_DeviceDisconnectedEvent;
            _rangeMeasurementService.DeviceDisconnectedEvent += _rangeMeasurementService_DeviceDisconnectedEvent;

            _rangeMeasurementService.MeasurementErrorEvent -= _rangeMeasurementService_MeasurementErrorEvent;
            _rangeMeasurementService.MeasurementErrorEvent += _rangeMeasurementService_MeasurementErrorEvent;
            _rangeMeasurementService.StartMeasurement();
        }

        public GraphViewModel(CalibrationItem calibration)
        {
            Calibration = calibration;
            StartMeasuringTime = DateTime.Now;
            MeasuredDataItems = new ObservableCollection<MeasuredDataItem>();
            _dataStore = TinyIoCContainer.Current.Resolve<IDataStore>();
            ExportCommand = new AsyncCommand(async () => await ExportToFile());
            _applicationStateProvider = TinyIoCContainer.Current.Resolve<IApplicationStateProvider>();
        }

        private async void _rangeMeasurementService_DeviceDisconnectedEvent(object arg1, DeviceEventArgs arg2)
        {
            var deviceName = arg2.Device.Name;
            await ShowError(String.Format($"Device {deviceName} has been disconnected"));
        }

        private async void _rangeMeasurementService_MeasurementErrorEvent()
        {
            //await ShowError("Something went wrong with LIDAR. You need to Software Reset on LIDAR, otherwise application will not work correctly");
            //if(await ShowDialog("Do you want to Software reset?"))
            //{
            //    var settingPage = TinyIoCContainer.Current.Resolve<SettingsPage>();
            //    await _rangeMeasurementService.SwReset();
            //    await Application.Current.MainPage.Navigation.PushAsync(settingPage);
            //}
        }

        private Task GraphReset()

        {
            MeasuredDataItems.Clear();
            return Task.CompletedTask;
        }

        public async Task StartStopMeasurement()
        {
            
            if (Measurement)
            {
                await WithBusy(() => _rangeMeasurementService.StopMeasurement());
                Measurement = false;
            } else
            {
                await WithBusy(() => _rangeMeasurementService.StartMeasurement());
                Measurement = true;
            }
            
        }

        private async void _rangeMeasurementService_MeasuredDataResponseEvent(List<float> obj)
        {
            int i = 0;
            foreach (var item in obj)
            {
                i++;
                var distance = CalculateDistanceFromTdc(item);
                TimeSpan span = (DateTime.Now - StartMeasuringTime);
                var timeDifference = (float)span.TotalMilliseconds + TimeSpan.FromMilliseconds(100 * i).Milliseconds;
                MeasuredDataItems.Add(new MeasuredDataItem(distance, timeDifference));
                OnPropertyChanged(nameof(MeasuredDataItems));

                await _dataStore.AddEntityAsync(new MeasurementItem()
                {
                    Calibration_id = Calibration.Id,
                    Tdc_value = item,
                    TimeDifference = timeDifference
                });
            }
        }

        public async void Init()
        {
            await SetCalibration();
           // await LoadValues();
        }

        public async Task LoadValues()
        {
            var list = await _dataStore.ListMeasurementByCalibrationAsync(Calibration.Id);

            foreach (var item in list)
            {
                TimeSpan span = DateTime.Now - StartMeasuringTime;
                var distance = CalculateDistanceFromTdc(item.Tdc_value);
                MeasuredDataItems.Add(new MeasuredDataItem(distance, (float)span.TotalMilliseconds));
                OnPropertyChanged(nameof(MeasuredDataItems));
            }
        }

        private async Task ExportToFile()
        {
            if(await _applicationStateProvider.GetStoragePermissionStatus() != Plugin.Permissions.Abstractions.PermissionStatus.Granted)
            {
                if (await CrossPermissions.Current.ShouldShowRequestPermissionRationaleAsync(Permission.Storage))
                {
                    await ShowError("You must allow permission for storage to export file.");
                }

            }
            if (await _applicationStateProvider.GetStoragePermissionStatus() == Plugin.Permissions.Abstractions.PermissionStatus.Granted)
            {
                await Export();
            }
        }

        public async Task Export()
        {
            List<string> lines = new List<string>() { "---Calibration---", $"Tdc 0: {Calibration.Tdc_0}", $"Tdc 62.5: {Calibration.Tdc_62}", $"Tdc 125: {Calibration.Tdc_125}", "", "---Data----", "Distance Time" };
            foreach (var item in MeasuredDataItems)
            {
                lines.Add($"{item.Distance} {item.Time}");
            }
            var file = Path.Combine(FileSystem.CacheDirectory, "data.txt");

            File.WriteAllLines(file, lines.ToArray());
            try
            {
                var message = new EmailMessage
                {
                    Subject = "RETIRODE Data",
                };
                message.Attachments.Add(new EmailAttachment(file));
                await Email.ComposeAsync(message);
            }
            catch (FeatureNotSupportedException fbsEx)
            {
                await ShowError("Email is not supported on this device");
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public async static Task SaveTextAsync(string content, string fileName)
        {
            var backingFile = Path.Combine(Xamarin.Essentials.FileSystem.AppDataDirectory, fileName);
            using (var writer = File.CreateText(backingFile))
            {
                await writer.WriteAsync(content);
            }
        }


        private float CalculateDistanceFromTdc(float tdcValue)
        {
            float tofValue;

            if (tdcValue < 0)
            {
                tofValue = 0f;
            }
            else if (tdcValue < 62.5f)
            {
                tofValue = 0f + (62.5f * ((tdcValue - Calibration.Tdc_0) / (Calibration.Tdc_62 - Calibration.Tdc_0)));
            }
            else if (tdcValue < 125f)
            {
                tofValue = 62.5f + (62.5f * ((tdcValue - Calibration.Tdc_62) / (Calibration.Tdc_125 - Calibration.Tdc_62)));
            }
            else
            {
                tofValue = 125f + (62.5f * ((tdcValue - Calibration.Tdc_125) / (Calibration.Tdc_125 - Calibration.Tdc_62)));
            }

            float distance = 0.15f * tofValue;
            return distance;
        }
        private async Task SetCalibration()
        {
            var calibrationList = await _dataStore.GetEntitiesAsync<CalibrationItem>();
            var calibration = new List<CalibrationItem>(calibrationList).FindLast(x => x.Id > 0);
            Calibration = calibration;
        }
    }
}
