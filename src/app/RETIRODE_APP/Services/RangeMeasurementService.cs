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
        private ICharacteristic _firstCharacteristicDataReceive;
        private ICharacteristic _firstCharacteristicDataWrite;
        private IService _firstService;
        private IService _secondService;
        private ICharacteristic _secondServiceSendCommandCharacteristic;
        private ICharacteristic _secondServiceSendQueryCharacteristic;
        private ICharacteristic _secondServiceReceiveQueryCharacteristic;
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
            catch(Exception ex)
            {
                throw;
            }
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task StartMeasurement()
        {
            await WriteToCharacteristic(_firstCharacteristicDataWrite, new[] { (byte)RSL10Command.StartLidar });
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
            await WriteToCharacteristic(_firstCharacteristicDataWrite, new[] { (byte)RSL10Command.StopLidar });
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task SwReset()
        {
            await WriteToCharacteristic(_secondServiceSendCommandCharacteristic, new[] { (byte)Registers.SWReset, Convert.ToByte(0), Convert.ToByte(0)});
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task SetLaserVoltage(int laserVoltage)
        {
            var message = BuildProtocolMessage(Registers.LaserVoltage, (byte)Voltage.Target, laserVoltage);
            await WriteToCharacteristic(_secondServiceSendCommandCharacteristic, message);
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task SetSipmBiasPowerVoltage(int simpBiasPowerVoltage)
        {
            var message = BuildProtocolMessage(Registers.SipmBiasPowerVoltage, (byte)Voltage.Target, simpBiasPowerVoltage);
            await WriteToCharacteristic(_secondServiceSendCommandCharacteristic, message);
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task CalibrateLidar()
        {
            var messageNS0 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.NS0, 0);
            var messageNS62_5 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.NS62_5, 0);
            var messageNS125 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.NS125, 0);

            await WriteToCharacteristic(_secondServiceSendCommandCharacteristic, messageNS0);
            await WriteToCharacteristic(_secondServiceSendCommandCharacteristic, messageNS62_5);
            await WriteToCharacteristic(_secondServiceSendCommandCharacteristic, messageNS125);
        }

        public async Task SetPulseCount(int pulseCount)
        {
            //TODO: change 1 to enum
            var message = BuildProtocolMessage(Registers.PulseCount, 1, pulseCount);
            await WriteToCharacteristic(_secondServiceSendCommandCharacteristic, message);
        }

        public async Task GetLaserVoltage(Voltage voltage)
        {
            var message = BuildProtocolMessage(Registers.LaserVoltage, (byte)voltage, 0);
            await WriteToCharacteristic(_secondServiceSendQueryCharacteristic, message);
        }

        public async Task GetSipmBiasPowerVoltage(Voltage voltage)
        {
            var message = BuildProtocolMessage(Registers.LaserVoltage, (byte)voltage, 0);
            await WriteToCharacteristic(_secondServiceSendQueryCharacteristic, message);
        }

        public async Task GetCalibration(Calibrate calibrate)
        {
            var message = BuildProtocolMessage(Registers.Calibrate, (byte)calibrate, 0);
            await WriteToCharacteristic(_secondServiceSendQueryCharacteristic, message);
        }

        private void QueryResponseHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            var data = e.Characteristic.Value;
            if(data is null)
            {
                return;
            }
            var responseItem = GetQueryResponseItem(data);
            QueryResponseEvent.Invoke(responseItem);
        }

        private void MeasurementDataHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            var data =  e.Characteristic.Value;

            if (data is null)
            {
                return;
            }

            //Empty invoking .. structure of data isn't known yet.
            QueryDataReceivedEvent.Invoke();
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
            try
            {
                //_firstService = await _connectedDevice.GetServiceAsync(Constants.GattFirstServiceId);
                //_firstCharacteristicDataReceive = await _firstService.GetCharacteristicAsync(Constants.GattFirstCharacteristicReceiveId);
                //_firstCharacteristicDataWrite = await _firstService.GetCharacteristicAsync(Constants.GattFirstCharacteristicWriteId);

                _secondService = await _connectedDevice.GetServiceAsync(Constants.QueryCommandServiceUUID);
                _secondServiceSendCommandCharacteristic = await _secondService.GetCharacteristicAsync(Constants.SendCommandCharacteristicUUID);
                _secondServiceSendQueryCharacteristic = await _secondService.GetCharacteristicAsync(Constants.SendQueryCharacteristicUUID);
                _secondServiceReceiveQueryCharacteristic = await _secondService.GetCharacteristicAsync(Constants.ReceiveQueryCharacteristicUUID);

                //Helper.NullCheck(_firstService);
                //Helper.NullCheck(_firstCharacteristicDataReceive);
                //Helper.NullCheck(_firstCharacteristicDataWrite);
                Helper.NullCheck(_secondService);
                Helper.NullCheck(_secondServiceSendCommandCharacteristic);
                Helper.NullCheck(_secondServiceSendQueryCharacteristic);
                Helper.NullCheck(_secondServiceReceiveQueryCharacteristic);

                _secondServiceReceiveQueryCharacteristic.ValueUpdated -= QueryResponseHandler;
                _secondServiceReceiveQueryCharacteristic.ValueUpdated += QueryResponseHandler;
                await _secondServiceReceiveQueryCharacteristic.StartUpdatesAsync();


                // _firstCharacteristicDataReceive.ValueUpdated -= MeasurementDataHandler;
                // _firstCharacteristicDataReceive.ValueUpdated += MeasurementDataHandler;
                // await _firstCharacteristicDataReceive.StartUpdatesAsync();

            }
            catch (Exception ex)
            {
                throw;
            }

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
            _firstCharacteristicDataReceive.ValueUpdated -= MeasurementDataHandler;
            _secondServiceReceiveQueryCharacteristic.ValueUpdated -= QueryResponseHandler;
            _connectedDevice.Dispose();
        }
    }
}
