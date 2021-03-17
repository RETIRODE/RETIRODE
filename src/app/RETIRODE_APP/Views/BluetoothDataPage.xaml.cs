using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Bluetooth;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Essentials;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace RETIRODE_APP.Views
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class BluetoothDataPage : ContentPage
    {
        private readonly IDevice _connectedDevice;
        private ICharacteristic _sendCharacteristic;
        private ICharacteristic _receiveCharacteristic;

        public BluetoothDataPage(IDevice connectedDevice)
        {
            InitializeComponent();
            _connectedDevice = connectedDevice;
            InitButton.IsEnabled = !(ScanButton.IsEnabled = false);  
        }

        private async void InitializeCommandButton_Clicked(object sender, EventArgs e)
        {
            try
            {
                var service = await _connectedDevice.GetServiceAsync(GattIdentifiers.UartGattServiceId);
                // get more services .. helps to find out available services of device
                // var services = await _connectedDevice.GetServicesAsync();
               
                if(service != null)
                {       
                    _sendCharacteristic = await service.GetCharacteristicAsync(GattIdentifiers.UartGattCharacteristicSendId);
                    _receiveCharacteristic = await service.GetCharacteristicAsync(GattIdentifiers.UartGattCharacteristicReceiveId);

                    if(_receiveCharacteristic != null)
                    {
                        var descriptors = await _receiveCharacteristic.GetDescriptorsAsync();
                        _receiveCharacteristic.ValueUpdated += (o, args) =>
                        {
                            var receivedBytes = args.Characteristic.Value;
                            MainThread.BeginInvokeOnMainThread(() =>
                            {
                                Output.Text += Encoding.UTF8.GetString(receivedBytes, 0, receivedBytes.Length) + Environment.NewLine;
                            });
                        };

                        await _receiveCharacteristic.StartUpdatesAsync();
                        InitButton.IsEnabled = !(ScanButton.IsEnabled = true);
                    }
                }
                else
                {
                    Output.Text += "UART GATT service not found." + Environment.NewLine;
                }
            }
            catch
            {
                Output.Text += "Error initializing UART GATT service." + Environment.NewLine;
            }
        }

        private async void SendCommandButton_Clicked(object sender, EventArgs e)
        {
            try
            {
                if(_sendCharacteristic != null)
                {
                    var bytes = await _sendCharacteristic.WriteAsync(Encoding.ASCII.GetBytes($"{CommandTxt.Text}\r\n"));
                }
            }
            catch
            {
                Output.Text += "Error sending command to UART." + Environment.NewLine;
            }
        }

        private async void Receive()
        {
            
        }
    }
}