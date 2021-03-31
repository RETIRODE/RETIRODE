using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services;
using System;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;
using System.Threading.Tasks;
using System.Collections.ObjectModel;
using System.Diagnostics;

namespace RETIRODE_APP.ViewModels
{
    public class BluetoothViewModel : BaseViewModel
    {
        private BLEDevice _selectedDevice;

        public IRangeMeasurementService rangeMeasurementService => TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
        public ObservableCollection<BLEDevice> Devices { get; }
        public Command LoadDevicesCommand { get; }
        public Command<BLEDevice> DeviceTapped { get; }

        public BluetoothViewModel()
        {
            Title = "Bluetooth";
            Devices = new ObservableCollection<BLEDevice>();
            LoadDevicesCommand = new Command(ExecuteLoadDevicesCommand);

            DeviceTapped = new Command<BLEDevice>(OnDeviceSelected);
        }

        public ICommand OpenWebCommand { get; }

        public void ExecuteLoadDevicesCommand()
        {
            IsBusy = true;

            try
            {
                Devices.Clear();
                var devices = rangeMeasurementService.AvailableDevices();
                foreach (var device in devices)
                {
                    Devices.Add(device);
                }
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
