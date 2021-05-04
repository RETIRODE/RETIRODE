using Nancy.TinyIoc;
using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.EventArgs;
using Plugin.BLE.Abstractions.Exceptions;
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
        private ICharacteristic _RMTTimeOfFlightDataCharacteristic;
        private ICharacteristic _RMTControlPointCharacteristic;
        private ICharacteristic _RMTInfoCharacteristic;
        private IService _rangeMeasurementTransferService;
        private IService _queryCommandService;
        private ICharacteristic _sendCommandCharacteristic;
        private ICharacteristic _sendQueryCharacteristic;
        private ICharacteristic _receiveQueryCharacteristic;

        private bool _isDataSize = false;
        private int _dataSize = 0;
        private CalibrationState _calibrationState = CalibrationState.NoState;
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
        public event Action<int[]> MeasuredDataResponseEvent;

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task ConnectToRSL10(BLEDevice bleDevice)
        {
            try
            {
                var device = _availableDevices.FirstOrDefault(foundDevice => foundDevice.Name == bleDevice.Name);
                if (device == null)
                {
                    throw new Exception("No device found");
                }

                _connectedDevice = device;
                await _bluetoothService.ConnectToDeviceAsync(_connectedDevice);
                await InitializeBluetoothConnection();
            }
            catch (DeviceConnectionException ex)
            {
                throw;
            }
        }

        /// <inheritdoc cref="IRangeMeasurementService"/>
        public async Task StartMeasurement()
        {
            if (!_isDataSize)
            {
                await _RMTInfoCharacteristic.StartUpdatesAsync();
                await WriteToCharacteristic(_RMTControlPointCharacteristic, new[] { (byte)RSL10Command.StartLidar });
            }
        }

        private async void DataSizeHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            _isDataSize = true;
            _dataSize = Convert.ToInt32(e.Characteristic.Value);

            //request any value from offered interval <0, {_dataSize}>
            var RandomValueInRange = new Random().Next(0, _dataSize);
            await WriteToCharacteristic(_RMTControlPointCharacteristic, new[] { (byte)RSL10Command.StartTransfer, Convert.ToByte(RandomValueInRange) });
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
                throw new Exception("Currently calibrating LIDAR");
            }
        }
        public async Task SetPulseCount(int pulseCount)
        {
            //TODO: change 1 to enum
            var message = BuildProtocolMessage(Registers.PulseCount, 1, pulseCount);
            await WriteToCharacteristic(_sendCommandCharacteristic, message);
        }

        public async Task GetLaserVoltage(Voltage voltage)
        {
            var message = BuildProtocolMessage(Registers.LaserVoltage, (byte)voltage, 0);
            await WriteToCharacteristic(_sendQueryCharacteristic, message);
        }

        public async Task GetSipmBiasPowerVoltage(Voltage voltage)
        {
            var message = BuildProtocolMessage(Registers.LaserVoltage, (byte)voltage, 0);
            await WriteToCharacteristic(_sendQueryCharacteristic, message);
        }

        public async Task GetCalibration(Calibrate calibrate)
        {
            var message = BuildProtocolMessage(Registers.Calibrate, (byte)calibrate, 0);
            await WriteToCharacteristic(_sendQueryCharacteristic, message);
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

            var responseItem = GetQueryResponseItem(data);
            QueryResponseEvent.Invoke(responseItem);
        }

        private void MeasurementDataHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            var data = e.Characteristic.Value;
            int[] parsedData = { };

            if (data is null)
            {
                return;
            }

            for (int i = 0; i < data.Length; i++)
            {
                parsedData[i] = Convert.ToInt32(data[i]);
            }
            MeasuredDataResponseEvent.Invoke(parsedData);
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
        private ResponseItem GetQueryResponseItem(byte[] data)
        {
            switch ((Registers)data[0])
            {
                case Registers.LaserVoltage:

                    if (data[2]== (byte)Voltage.Actual)
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.LaserVoltageActual,
                            Value = Encoding.UTF8.GetString(new[] { data[4], data[5] })
                        };
                    }
                    else
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.LaserVoltageTarget,
                            Value = Encoding.UTF8.GetString(new[] { data[4], data[5] })
                        };
                    }
                case Registers.SipmBiasPowerVoltage:

                    if (data[2] == (byte)Voltage.Actual)
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.SipmBiasPowerVoltageActual,
                            Value = Encoding.UTF8.GetString(new[] { data[4], data[5] })
                        };
                    }
                    else
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.SipmBiasPowerVoltageTarget,
                            Value = Encoding.UTF8.GetString(new[] { data[4], data[5] })
                        };
                    }

                case Registers.Calibrate:
                    if (data[2] == (byte)Calibrate.NS0)
                    {
                        return new ResponseItem()
                        {
                            Identifier = RangeFinderValues.CalibrateNS0,
                            Value = Encoding.UTF8.GetString(new[] { data[2] })
                        };
                    }
                    else if (data[3] == (byte)Calibrate.NS62_5)
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

                _RMTTimeOfFlightDataCharacteristic.ValueUpdated -= MeasurementDataHandler;
                _RMTTimeOfFlightDataCharacteristic.ValueUpdated += MeasurementDataHandler;
                await _RMTTimeOfFlightDataCharacteristic.StartUpdatesAsync();

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
            _RMTTimeOfFlightDataCharacteristic.ValueUpdated -= MeasurementDataHandler;
            _RMTInfoCharacteristic.ValueUpdated -= DataSizeHandler;
            _receiveQueryCharacteristic.ValueUpdated -= QueryResponseHandler;
            _connectedDevice.Dispose();
        }
    }
}
