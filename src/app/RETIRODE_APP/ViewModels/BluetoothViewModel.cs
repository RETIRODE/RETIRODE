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

namespace RETIRODE_APP.ViewModels
{
    public class BluetoothViewModel : BaseViewModel
    {
        private BLEDevice _selectedDevice;

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
            TestDevices = new List<BLEDevice>();

            TestDevices.Add(new BLEDevice
            {
                Name = "device1",
                Identifier = new Guid()
            });
            TestDevices.Add(new BLEDevice
            {
                Name = "device2",
                Identifier = new Guid()
            });
        }

        public ICommand OpenWebCommand { get; }

        private async Task ExecuteLoadDevicesCommand()
        {
            IsBusy = true;

            try
            {                
                await rangeMeasurementService.StartScanning();
                var devices = rangeMeasurementService.AvailableDevices;
                Devices.Clear();
                do
                {
                    devices = rangeMeasurementService.AvailableDevices;
                    foreach (var device in devices)
                    {
                        Devices.Add(device);
                        OnPropertyChanged(nameof(Devices));
                    }
                    Thread.Sleep(2000);
                }
                while (devices == rangeMeasurementService.AvailableDevices);
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
            get => _selectedDevice;
            set
            {
                SetProperty(ref _selectedDevice, value);
                OnDeviceSelected(value);
            }
        }

        public void OnDeviceSelected(BLEDevice device)
        {
            if (device == null)
                return;

            rangeMeasurementService.ConnectToRSL10(device);
        }
    }
}
