﻿using Nancy.TinyIoc;
using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.EventArgs;
using Plugin.BLE.Abstractions.Exceptions;
using Polly;
using RETIRODE_APP.Exceptions;
using RETIRODE_APP.Helpers;
using RETIRODE_APP.Models;
using RETIRODE_APP.Models.Enums;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    /// <inheritdoc cref="IRangeMeasurementService"/>
    public class RangeMeasurementService : IRangeMeasurementService
    {
        private readonly IBluetoothService _bluetoothService;
        private IList<IDevice> _availableDevices;
        private IDevice _connectedDevice;
        private ICharacteristic _RMTTimeOfFlightDataCharacteristic;
        private ICharacteristic _RMTControlPointCharacteristic;
        private ICharacteristic _RMTInfoCharacteristic;
        private IService _rangeMeasurementTransferService;
        private IService _queryCommandService;
        private ICharacteristic _sendCommandCharacteristic;
        private ICharacteristic _sendQueryCharacteristic;
        private ICharacteristic _receiveQueryCharacteristic;

        private bool _isDataSize = false;
        private List<byte> _TOFData;
        private int _dataSize;
        private SemaphoreSlim _semaphoreSlim;
        private CalibrationState _calibrationState = CalibrationState.NoState;
        public RangeMeasurementService()
        {
            _availableDevices = new List<IDevice>();
            _semaphoreSlim = new SemaphoreSlim(1);
            _bluetoothService = TinyIoCContainer.Current.Resolve<IBluetoothService>();
            _bluetoothService.DeviceFound = DeviceDiscovered;
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public event Action<BLEDevice> DeviceDiscoveredEvent;

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public event Action<ResponseItem> QueryResponseEvent;

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public event Action<List<float>> MeasuredDataResponseEvent;

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task ConnectToRSL10(BLEDevice bleDevice)
        {
            var device = _availableDevices.FirstOrDefault(foundDevice => foundDevice.Name == bleDevice.Name);
            if (device == null)
            {
                throw new InvalidOperationException(String.Format("Device {0} with ID {1} is not available",
                    bleDevice.Name, bleDevice.Identifier));
            }

            if(_connectedDevice != null && _connectedDevice != device)
            {
                throw new AlreadyConnectedDeviceException("Device is already connected to another BLE device");
            }
            if(_connectedDevice != null && _connectedDevice == device)
            {
                return;
            }

            await Policy
                .Handle<DeviceConnectionException>()
                .WaitAndRetryAsync(5, time => TimeSpan.FromMilliseconds(100))
                .ExecuteAsync(() => _bluetoothService.ConnectToDeviceAsync(device));

            _connectedDevice = device;
            await InitializeBluetoothConnection();
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task StartMeasurement()
        {
            if (!_isDataSize)
            {
                await WriteToCharacteristic(_RMTControlPointCharacteristic, new[] { (byte)RSL10Command.StartLidar });
            }
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task StartScanning()
        {
            if (_bluetoothService.IsScanning)
            {
                return;
            }
            _availableDevices.Clear();
            await _bluetoothService.StartScanning();
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task StopScanning()
        {
            await _bluetoothService.StopScanning();
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task StopMeasurement()
        {
            await WriteToCharacteristic(_RMTInfoCharacteristic, new[] { (byte)RSL10Command.StopLidar });
            _isDataSize = false;
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task SwReset()
        {
            await WriteToCharacteristic(_sendCommandCharacteristic, new[] { (byte)Registers.SWReset, Convert.ToByte(0), Convert.ToByte(0) });
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task SetLaserVoltage(int laserVoltage)
        {
            var message = BuildProtocolMessage(Registers.LaserVoltage, (byte)Voltage.Target, laserVoltage);
            await WriteToCharacteristic(_sendCommandCharacteristic, message);
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task SetSipmBiasPowerVoltage(int simpBiasPowerVoltage)
        {
            var message = BuildProtocolMessage(Registers.SipmBiasPowerVoltage, (byte)Voltage.Target, simpBiasPowerVoltage);
            await WriteToCharacteristic(_sendCommandCharacteristic, message);
        }

        public async Task SwitchLaserVoltage(Switch s)
        {
            var message = BuildProtocolMessage(Registers.LaserVoltage, (byte)Voltage.Switch, Convert.ToInt32(s));
            await WriteToCharacteristic(_sendCommandCharacteristic, message);
        }

        public async Task SwitchSipmBiasVoltage(Switch s)
        {
            var message = BuildProtocolMessage(Registers.SipmBiasPowerVoltage, (byte)Voltage.Switch, Convert.ToInt32(s));
            await WriteToCharacteristic(_sendCommandCharacteristic, message);
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task CalibrateLidar()
        {
            if (_calibrationState == CalibrationState.NoState)
            {
                var messageNS0 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.NS0, 0);
                await WriteToCharacteristic(_sendCommandCharacteristic, messageNS0);
                _calibrationState = CalibrationState.NS0;
            }
            else
            {
                throw new CalibrationLidarException("Currently calibrating LIDAR");
            }
        }
        public async Task SetPulseCount(int pulseCount)
        {
            var message = BuildProtocolMessage(Registers.PulseCount, Convert.ToByte(1), pulseCount);
            await WriteToCharacteristic(_sendCommandCharacteristic, message);
        }

        public async Task GetPulseCount()
        {
            var message = BuildProtocolMessage(Registers.PulseCount, Convert.ToByte(0), 0);
            await WriteToCharacteristic(_sendQueryCharacteristic, message);
        }

        public async Task GetLaserVoltage(Voltage voltage)
        {
            var message = BuildProtocolMessage(Registers.LaserVoltage, (byte)voltage, 0);
            await WriteToCharacteristic(_sendQueryCharacteristic, message);
        }

        public async Task GetSipmBiasPowerVoltage(Voltage voltage)
        {
            var message = BuildProtocolMessage(Registers.SipmBiasPowerVoltage, (byte)voltage, 0);
            await WriteToCharacteristic(_sendQueryCharacteristic, message);
        }

        public async Task GetVoltagesStatus()
        {
            var message = BuildProtocolMessage(Registers.VoltageStatus, Convert.ToByte(0), 0);
            await WriteToCharacteristic(_sendQueryCharacteristic, message);
        }

        private async void DataSizeHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            _isDataSize = true;
            _dataSize = BitConverter.ToInt32(e.Characteristic.Value, 0);

            await WriteToCharacteristic(_RMTControlPointCharacteristic, new[] { (byte)RSL10Command.StartTransfer });
        }

        private async void QueryResponseHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            if (_calibrationState != CalibrationState.NoState)
            {
                await CalibratingLidar();
            }

            var data = e.Characteristic.Value;
            if (data is null)
            {
                return;
            }

            var responseItems = GetQueryResponseItem(data);
            foreach (var item in responseItems)
            {
                QueryResponseEvent.Invoke(item);
            }
           
        }

        private void MeasurementDataHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            var data = e.Characteristic.Value;

            if (data is null)
            {
                return;
            }

            if (_dataSize >= _TOFData.Count)
            {
                _TOFData.AddRange(data);
            }
            else
            {
                List<float> parsedData = new List<float>();
                for (int i = 0; i < _TOFData.Count; i++)
                {
                    parsedData.Add(Convert.ToInt32(_TOFData.ElementAt(i)));
                }
                MeasuredDataResponseEvent.Invoke(parsedData);
                _dataSize = 0;
                _isDataSize = false;
                _TOFData.Clear();
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

        private async Task CalibratingLidar()
        {
            if (_calibrationState == CalibrationState.NS0)
            {
                var messageNS62_5 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.NS62_5, 0);
                await WriteToCharacteristic(_sendCommandCharacteristic, messageNS62_5);
                _calibrationState = CalibrationState.NS62_5;
            }
            else if (_calibrationState == CalibrationState.NS62_5)
            {
                var messageNS125 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.NS125, 0);
                await WriteToCharacteristic(_sendCommandCharacteristic, messageNS125);
                _calibrationState = CalibrationState.NS125;
            }
            else if (_calibrationState == CalibrationState.NS125)
            {
                var messageNS125 = BuildProtocolMessage(Registers.Calibrate, (byte)Calibrate.EndCommand, 0);
                await WriteToCharacteristic(_sendCommandCharacteristic, messageNS125);
                _calibrationState = CalibrationState.NoState;
            }
        }
        private List<ResponseItem> GetQueryResponseItem(byte[] data)
        {
            var list = new List<ResponseItem>();
            switch ((Registers)data[0])
            {
                case Registers.LaserVoltage:

                    if (data[2] == (byte)Voltage.Actual)
                    {
                        list.Add(new ResponseItem()
                        {
                            Identifier = RangeFinderValues.LaserVoltageActual,
                            Value = GetDataFromResponse(data)
                        });
                    }
                    else
                    {
                        list.Add(new ResponseItem()
                        {
                            Identifier = RangeFinderValues.LaserVoltageTarget,
                            Value = GetDataFromResponse(data)
                        });
                    }
                    break;
                case Registers.SipmBiasPowerVoltage:

                    if (data[2] == (byte)Voltage.Actual)
                    {
                        list.Add(new ResponseItem()
                        {
                            Identifier = RangeFinderValues.SipmBiasPowerVoltageActual,
                            Value = GetDataFromResponse(data)
                        });
                    }
                    else
                    {
                        list.Add(new ResponseItem()
                        {
                            Identifier = RangeFinderValues.SipmBiasPowerVoltageTarget,
                            Value = GetDataFromResponse(data)
                        });
                    }
                    break;
                case Registers.Calibrate:
                    if (data[2] == (byte)Calibrate.NS0)
                    {
                        list.Add(new ResponseItem()
                        {
                            Identifier = RangeFinderValues.CalibrateNS0,
                            Value = GetDataFromResponse(data)
                        });
                    }
                    else if (data[2] == (byte)Calibrate.NS62_5)
                    {
                        list.Add(new ResponseItem()
                        {
                            Identifier = RangeFinderValues.Calibrate62_5,
                            Value = GetDataFromResponse(data)
                        });
                    }
                    else
                    {
                        list.Add(new ResponseItem()
                        {
                            Identifier = RangeFinderValues.Calibrate125,
                            Value = GetDataFromResponse(data)
                        });
                    }
                    break;
                case Registers.PulseCount:
                    list.Add(new ResponseItem()
                    {
                        Identifier = RangeFinderValues.PulseCount,
                        Value = GetDataFromResponse(data)
                    });
                    break;

                case Registers.VoltageStatus:
                    list.Add(new ResponseItem()
                    {
                        Identifier = RangeFinderValues.LaserVoltageStatus,
                        Value = Convert.ToInt32(data[1])
                    });
                    list.Add(new ResponseItem()
                    {
                        Identifier = RangeFinderValues.LaserVoltageOverload,
                        Value = Convert.ToInt32(data[2])
                    });
                    list.Add(new ResponseItem()
                    {
                        Identifier = RangeFinderValues.SipmBiasPowerVoltageStatus,
                        Value = Convert.ToInt32(data[3])
                    });
                    list.Add(new ResponseItem()
                    {
                        Identifier = RangeFinderValues.SipmBiasPowerVoltageOverload,
                        Value = Convert.ToInt32(data[4])
                    });

                    break;
                default:
                    break;
            }
            return list;
        }

        private static int GetDataFromResponse(byte[] data)
        {
            var result = BitConverter.ToString(new[] { data[4], data[5] }).Replace("-", "");
            return Convert.ToInt32(result, 16);
        }

        private async Task InitializeBluetoothConnection()
        {
                _rangeMeasurementTransferService = await _connectedDevice.GetServiceAsync(Constants.RangeMeasurementTransferServiceUUID);
                _RMTTimeOfFlightDataCharacteristic = await _rangeMeasurementTransferService.GetCharacteristicAsync(Constants.RMTTimeOfFlightDataCharacteristic);
                _RMTControlPointCharacteristic = await _rangeMeasurementTransferService.GetCharacteristicAsync(Constants.RMTControlPointCharacteristic);
                _RMTInfoCharacteristic = await _rangeMeasurementTransferService.GetCharacteristicAsync(Constants.RMTInfoCharacteristic);

                _queryCommandService = await _connectedDevice.GetServiceAsync(Constants.QueryCommandServiceUUID);
                _sendCommandCharacteristic = await _queryCommandService.GetCharacteristicAsync(Constants.SendCommandCharacteristicUUID);
                _sendQueryCharacteristic = await _queryCommandService.GetCharacteristicAsync(Constants.SendQueryCharacteristicUUID);
                _receiveQueryCharacteristic = await _queryCommandService.GetCharacteristicAsync(Constants.ReceiveQueryCharacteristicUUID);

                Helper.NullCheck(_rangeMeasurementTransferService);
                Helper.NullCheck(_RMTTimeOfFlightDataCharacteristic);
                Helper.NullCheck(_RMTControlPointCharacteristic);
                Helper.NullCheck(_RMTInfoCharacteristic);
                Helper.NullCheck(_queryCommandService);
                Helper.NullCheck(_sendCommandCharacteristic);
                Helper.NullCheck(_sendQueryCharacteristic);
                Helper.NullCheck(_receiveQueryCharacteristic);

                _receiveQueryCharacteristic.ValueUpdated -= QueryResponseHandler;
                _receiveQueryCharacteristic.ValueUpdated += QueryResponseHandler;
                await _receiveQueryCharacteristic.StartUpdatesAsync();

                _RMTInfoCharacteristic.ValueUpdated -= DataSizeHandler;
                _RMTInfoCharacteristic.ValueUpdated += DataSizeHandler;
                await _RMTInfoCharacteristic.StartUpdatesAsync();

                _RMTTimeOfFlightDataCharacteristic.ValueUpdated -= MeasurementDataHandler;
                _RMTTimeOfFlightDataCharacteristic.ValueUpdated += MeasurementDataHandler;
                await _RMTTimeOfFlightDataCharacteristic.StartUpdatesAsync();
        }

        private async Task WriteToCharacteristic(ICharacteristic characteristic, byte[] command)
        {
            try
            {
                await _semaphoreSlim.WaitAsync();
                await Policy
                   .Handle<NullReferenceException>()
                   .OrResult<bool>(result => result == false)
                   .WaitAndRetryAsync(5, time => TimeSpan.FromMilliseconds(100))
                   .ExecuteAsync(() => _bluetoothService.WriteToCharacteristic(characteristic, command));
            }
            finally
            {
                _semaphoreSlim.Release();
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
            _RMTTimeOfFlightDataCharacteristic.ValueUpdated -= MeasurementDataHandler;
            _RMTInfoCharacteristic.ValueUpdated -= DataSizeHandler;
            _receiveQueryCharacteristic.ValueUpdated -= QueryResponseHandler;
            _semaphoreSlim.Dispose();
            _connectedDevice.Dispose();
        }
    }
}
