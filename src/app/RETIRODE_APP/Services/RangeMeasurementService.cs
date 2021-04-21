using Nancy.TinyIoc;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.EventArgs;
using RETIRODE_APP.Helpers;
using RETIRODE_APP.Models;
using RETIRODE_APP.Models.Enums;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    /// <inheritdoc cref="IRangeMeasurementService"/>
    public class RangeMeasurementService : IRangeMeasurementService
    {
        private readonly IBluetoothService _bluetoothService;
        private IList<IDevice> _availableDevices;
        private IDevice _connectedDevice;
        private IList<BLEDevice> AvailableDevices => _availableDevices.Select(x => new BLEDevice()
        {
            Identifier = x.Id,
            Name = x.Name,
            State = x.State
        }).ToList();

        IList<BLEDevice> IRangeMeasurementService.AvailableDevices => throw new NotImplementedException();

        private ICharacteristic _firstCharacteristicDataReceive;
        private ICharacteristic _firstCharacteristicDataWrite;
        private IService _firstService;
        private ICharacteristic _secondCharacteristicDataReceive;
        private ICharacteristic _secondCharacteristicDataWrite;
        private IService _secondService;


        public RangeMeasurementService()
        {
            _availableDevices = new List<IDevice>();
            _bluetoothService = TinyIoCContainer.Current.Resolve<IBluetoothService>();
            _bluetoothService.DeviceFound = DeviceDiscovered;
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public event Action<BLEDevice> DeviceDiscoveredEvent;

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public event Action<ResponseItem> QueryResponseEvent;

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public event Action QueryDataReceivedEvent;

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task ConnectToRSL10(BLEDevice bleDevice)
        {
            try
            {
                var device = _availableDevices.FirstOrDefault(foundDevice => foundDevice.Name == bleDevice.Name);
                if (device == null)
                {
                    throw new Exception("Cannot connect to device");
                }
                
                _connectedDevice = device;
                await _bluetoothService.ConnectToDeviceAsync(_connectedDevice);
                await InitializeBluetoothConnection();
            }
            catch
            {
                throw;
            }
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public Task Disconnect(BLEDevice device)
        {
            throw new NotImplementedException();
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task StartMeasurement()
        {
            await WriteToCharacteristic(_firstCharacteristicDataWrite, (byte)RSL10Command.StartMeasurement);
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task StartScanning()
        {
            _availableDevices.Clear();
            await _bluetoothService.StartScanning();
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task StopMeasurement()
        {
            await WriteToCharacteristic(_firstCharacteristicDataWrite, (byte)RSL10Command.StopMeasurement);
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task SwReset()
        {
            await WriteToCharacteristic(_firstCharacteristicDataWrite, new[] { (byte)Registers.SWReset, Convert.ToByte(0), Convert.ToByte(0)});
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task SetLaserVoltage(int laserVoltage)
        {
            var message = BuildProtocolMessage(Registers.LaserVoltage, (byte)Voltage.Target, laserVoltage);
            await WriteToCharacteristic(_firstCharacteristicDataWrite, message);
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task SetSipmBiasPowerVoltage(int simpBiasPowerVoltage)
        {
            var message = BuildProtocolMessage(Registers.SipmBiasPowerVoltage, (byte)Voltage.Target, simpBiasPowerVoltage);
            await WriteToCharacteristic(_firstCharacteristicDataWrite, message);
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task CalibrateLidar()
        {
            var messageNS0 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.NS0, 0);
            var messageNS62_5 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.NS62_5, 0);
            var messageNS125 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.NS125, 0);

            await WriteToCharacteristic(_firstCharacteristicDataWrite, messageNS0);
            await WriteToCharacteristic(_firstCharacteristicDataWrite, messageNS62_5);
            await WriteToCharacteristic(_firstCharacteristicDataWrite, messageNS125);
        }

        private async void QueryResponseHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            var data = await e.Characteristic.ReadAsync();
            if(data is null)
            {
                return;
            }
            var responseItem = GetQueryResponseItem(data);
            QueryResponseEvent.Invoke(responseItem);
        }

        private async void MeasurementDataHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            var data = await e.Characteristic.ReadAsync();

            if (data is null)
            {
                return;
            }
            //var responseItem = GetQueryResponseItem(data);
            //QueryDataReceivedEvent.Invoke(responseItem);
        }
        private ResponseItem GetQueryResponseItem(byte[] data)
        {
            switch ((Registers)data[0])
            {
                case Registers.LaserVoltage:
                    
                    if (data[1] == (byte)Voltage.Actual)
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.LaserVoltageActual,
                            Value = Encoding.UTF8.GetString(new[] { data[2] })
                        };
                    }
                    else
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.LaserVoltageTarget,
                            Value = Encoding.UTF8.GetString(new[] { data[2] })
                        };
                    }
                case Registers.SipmBiasPowerVoltage:

                    if(data[1] == (byte)Voltage.Actual)
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.SipmBiasPowerVoltageActual,
                            Value = Encoding.UTF8.GetString(new[] { data[2] })
                        };
                    }
                    else
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.SipmBiasPowerVoltageTarget,
                            Value = Encoding.UTF8.GetString(new[] { data[2] })
                        };
                    }
                    
                case Registers.Calibrate:
                    if (data[1] == (byte)Calibrate.NS0)
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.CalibrateNS0,
                            Value = Encoding.UTF8.GetString(new[] { data[2] })
                        };
                    }
                    else if(data[1] == (byte)Calibrate.NS62_5)
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.Calibrate62_5,
                            Value = Encoding.UTF8.GetString(new[] { data[2] })
                        };
                    }
                    else
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.Calibrate125,
                            Value = Encoding.UTF8.GetString(new[] { data[2] })
                        };
                    }
                default:
                    break;
            }
            return new ResponseItem();
        }

        private async Task InitializeBluetoothConnection()
        {
            _firstService = await _connectedDevice.GetServiceAsync(Constants.GattFirstServiceId);
            _firstCharacteristicDataReceive = await _firstService.GetCharacteristicAsync(Constants.GattFirstCharacteristicReceiveId);
            _firstCharacteristicDataWrite = await _firstService.GetCharacteristicAsync(Constants.GattFirstCharacteristicWriteId);

            _secondService = await _connectedDevice.GetServiceAsync(Constants.GattSecondServiceId);
            _secondCharacteristicDataReceive = await _secondService.GetCharacteristicAsync(Constants.GattSecondCharacteristicReceiveId);
            _secondCharacteristicDataWrite = await _secondService.GetCharacteristicAsync(Constants.GattSecondCharacteristicWriteId);

            Helper.NullCheck(_firstService);
            Helper.NullCheck(_firstCharacteristicDataReceive);
            Helper.NullCheck(_firstCharacteristicDataWrite);
            Helper.NullCheck(_secondService);
            Helper.NullCheck(_secondCharacteristicDataReceive);
            Helper.NullCheck(_secondCharacteristicDataWrite);

            _firstCharacteristicDataReceive.ValueUpdated += QueryResponseHandler;
            _secondCharacteristicDataReceive.ValueUpdated += MeasurementDataHandler;
        }

        private async Task WriteToCharacteristic(ICharacteristic characteristic, byte[] command)
        {
            try
            {
                if (!await _bluetoothService.WriteToCharacteristic(characteristic, command))
                {
                    throw new Exception("Error with send command to characteristic");
                }
            }
            catch (Exception e)
            {
                throw new Exception(e.Message);
            }
        }

        private async void DeviceDiscovered(object sender, IDevice device)
        {
            if (await IsWhiteList(device))
            {
                _availableDevices.Add(device);
                DeviceDiscoveredEvent.Invoke(new BLEDevice()
                {
                    Name = device.Name,
                    Identifier = device.Id,
                    State = device.State
                });
            }
        }

        private Task<bool> IsWhiteList(IDevice device)
        {
            if (device.Name == Constants.UniqueRetirodeName && IsMacAddressEquals(device.NativeDevice))
            {
                return Task.FromResult(true);
            }
            return Task.FromResult(false);
        }

        private bool IsMacAddressEquals(object device)
        {
            PropertyInfo propertyInfo = device.GetType().GetProperty("Address");
            var macAddress = (string)propertyInfo.GetValue(device, null);

            return macAddress.Substring(0, Constants.UniqueMacAddressLength).Equals(Constants.RetirodeUniqueMacAddressPart);
        }

        private byte[] BuildProtocolMessage(Registers register, byte subRegister, int value)
        {
            return new[] { (byte)register, subRegister, Convert.ToByte(value) };
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public void Dispose()
        {
            _firstCharacteristicDataReceive.ValueUpdated -= QueryResponseHandler;
            _connectedDevice.Dispose();
        }

    }
}
