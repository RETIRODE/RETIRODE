using Plugin.BLE;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Threading.Tasks;
using Xamarin.Essentials;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace RETIRODE_APP.Views
{
    [DesignTimeVisible(false)]
    public partial class HomePage : ContentPage
    {
        private readonly IAdapter _bluetoothAdapter;
        private List<IDevice> _gattDevices = new List<IDevice>();
        public HomePage()
        {
            InitializeComponent();

            _bluetoothAdapter = CrossBluetoothLE.Current.Adapter;
            _bluetoothAdapter.DeviceDiscovered += (sender, foundBLEDevice) =>
            {
                if (foundBLEDevice.Device != null && !string.IsNullOrEmpty(foundBLEDevice.Device.Name))
                {
                    _gattDevices.Add(foundBLEDevice.Device);
                }
            };
        }

        private async Task<bool> PermissionGrantedAsync()
        {
            var locationPermissionStatus = await Permissions.CheckStatusAsync<Permissions.LocationAlways>();

            if(locationPermissionStatus != PermissionStatus.Granted)
            {
                var status = await Permissions.RequestAsync<Permissions.LocationAlways>();
                return status == PermissionStatus.Granted;
            }

            return true;
        }

        private async void ScanBluetoothLE_Clicked(object sender, EventArgs e)
        {
            IsBusyIndicator.IsVisible = IsBusyIndicator.IsRunning = !(ScanBluetoothLE.IsEnabled = false);
            foundBLEDevicesListView.ItemsSource = null;

            if(!await PermissionGrantedAsync())
            {
                await DisplayAlert("Permission Granted", "Application needs location permission", "OK");
                IsBusyIndicator.IsVisible = IsBusyIndicator.IsRunning = !(ScanBluetoothLE.IsEnabled = true);
                return;
            }

            _gattDevices.Clear();

            foreach(var device in _bluetoothAdapter.ConnectedDevices)
            {
                _gattDevices.Add(device);
            }

            await _bluetoothAdapter.StartScanningForDevicesAsync();

            foundBLEDevicesListView.ItemsSource = _gattDevices.ToArray();
            IsBusyIndicator.IsVisible = IsBusyIndicator.IsRunning = !(ScanBluetoothLE.IsEnabled = true);
        }

        private async void FoundBluetoothDevicesListView_ItemTapped(object sender, ItemTappedEventArgs e)
        {
            IsBusyIndicator.IsVisible = IsBusyIndicator.IsRunning = !(ScanBluetoothLE.IsEnabled = false);
            IDevice selectedItem = e.Item as IDevice;

            if(selectedItem.State == DeviceState.Connected)
            {
                await Navigation.PushAsync(new BluetoothDataPage(selectedItem));
            }
            else
            {
                try
                {
                    var connectParameters = new ConnectParameters(false, true);
                    await _bluetoothAdapter.ConnectToDeviceAsync(selectedItem, connectParameters);
                    await Navigation.PushAsync(new BluetoothDataPage(selectedItem));
                }
                catch
                {
                    await DisplayAlert("Error connecting", $"Error connecting to BLE device: {selectedItem.Name ?? "N/A"}", "Retry");
                }
            }
            IsBusyIndicator.IsVisible = IsBusyIndicator.IsRunning = !(ScanBluetoothLE.IsEnabled = true);
        }
    }
}