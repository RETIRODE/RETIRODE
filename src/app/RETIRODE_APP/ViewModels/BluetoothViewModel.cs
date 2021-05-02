using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services;
using RETIRODE_APP.Views;
using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.CommunityToolkit.ObjectModel;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class BluetoothViewModel : BaseViewModel
    {
        BLEDevice selectedDevice;
        public IRangeMeasurementService rangeMeasurementService;
        public ObservableCollection<BLEDevice> Devices { get; }
        public ICommand LoadDevicesCommand { get; }
        public IAsyncCommand<BLEDevice> DeviceTapped { get; }

        public BluetoothViewModel()
        {
            rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            Title = "Bluetooth";
            Devices = new ObservableCollection<BLEDevice>();
            LoadDevicesCommand = new Command(ExecuteLoadDevicesCommand);
            DeviceTapped = new AsyncCommand<BLEDevice>(async (device) => await OnDeviceSelected(device));
            rangeMeasurementService.DeviceDiscoveredEvent += RangeMeasurementService_DeviceDiscoveredEvent;
        }

        private void RangeMeasurementService_DeviceDiscoveredEvent(BLEDevice device)
        {
            Devices.Add(device);
        }

        private async void ExecuteLoadDevicesCommand()
        {            
            try
            {
                ShowBusy(() => Devices.Count > 0, 5);
                await rangeMeasurementService.StartScanning();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
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

        public async Task OnDeviceSelected(BLEDevice device)
        {
            if (device == null)
                return;

            try
            {
                await WithBusy(() => rangeMeasurementService.ConnectToRSL10(device));
                await Application.Current.MainPage.Navigation.PushAsync(new SettingsPage());
            }
            catch (Exception ex)
            {
                await ShowError($"Connecting to {device.Name} failed");
            }            
        }
    }
}
