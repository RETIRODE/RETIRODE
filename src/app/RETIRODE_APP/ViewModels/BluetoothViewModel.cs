using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services;
using System;
using System.Windows.Input;
using Xamarin.Forms;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Collections.Generic;
using Xamarin.CommunityToolkit.ObjectModel;
using System.Threading;
using RETIRODE_APP.Views;

namespace RETIRODE_APP.ViewModels
{
    public class BluetoothViewModel : BaseViewModel
    {
        BLEDevice selectedDevice;

        public IRangeMeasurementService rangeMeasurementService;
        public ObservableCollection<BLEDevice> Devices { get; }
        public IAsyncCommand LoadDevicesCommand { get; }
        public Command<BLEDevice> DeviceTapped { get; }

        public IList<BLEDevice> TestDevices { get; }

        public BluetoothViewModel()
        {
            rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            Title = "Bluetooth";
            Devices = new ObservableCollection<BLEDevice>();
            LoadDevicesCommand = new AsyncCommand(async () => await ExecuteLoadDevicesCommand());
            DeviceTapped = new Command<BLEDevice>(OnDeviceSelected);
            rangeMeasurementService.DeviceDiscoveredEvent += RangeMeasurementService_DeviceDiscoveredEvent;
            TestDevices = new List<BLEDevice>();
            TestDevices.Add(new BLEDevice
            {
                Name = "LIDAR",
                Identifier = new Guid()
            });
            TestDevices.Add(new BLEDevice
            {
                Name = "NotLIDAR",
                Identifier = new Guid()
            });
        }

        private void RangeMeasurementService_DeviceDiscoveredEvent(BLEDevice device)
        {
            Devices.Add(device);
        }

        public ICommand OpenWebCommand { get; }

        private async Task ExecuteLoadDevicesCommand()
        {
            IsBusy = true;

            try
            {                
                await rangeMeasurementService.StartScanning();
                Devices.Clear();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
            }
            finally
            {
                IsBusy = false;
            }
        }

        public void OnAppearing()
        {
            IsBusy = true;
            SelectedDevice = null;
        }

        public BLEDevice SelectedDevice
        {
            get => selectedDevice;
            set
            {
                SetProperty(ref selectedDevice, value);
                OnDeviceSelected(value);
            }
        }

        public async void OnDeviceSelected(BLEDevice device)
        {
            if (device == null)
                return;
            await rangeMeasurementService.ConnectToRSL10(device);
            await Application.Current.MainPage.DisplayAlert("Connecting...", "Connecting to device " + device.Name, "OK");
            await Application.Current.MainPage.Navigation.PushAsync(new SettingsPage());
        }
    }
}
