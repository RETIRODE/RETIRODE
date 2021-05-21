using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models.Enums
{
    public class ApplicationEnums
    {
        public enum Calibrate : byte
        {
            //0.0 ns
            NS0 = 0x01,

            //62.5 ns
            NS62_5 = 0x02,

            //125.0 ns
            NS125 = 0x03,

            //End command
            EndCommand = 0x04
        }

        public enum CalibrationState
        {
            NoState,
            NS0,
            NS62_5,
            NS125
        }

        public enum InfoCharacteristicResponseType : byte
        {
            Error = 0x00,
            DataSize = 0x01
        }

        public enum InfoCharacteristicErrorType : byte
        {
            CancelledByServer = 0x00,
            CancelledByClient = 0x01
        }

        public enum ProtocolGenerics : byte
        {
            DefaultByte = 0x00,
            DefaultValueType = 0x01
        }

        public enum RangeFinderValues
        {
            LaserVoltageTarget,
            LaserVoltageActual,
            SipmBiasPowerVoltageTarget,
            SipmBiasPowerVoltageActual,
            CalibrateNS0,
            Calibrate62_5,
            Calibrate125,
            PulseCount,
            LaserVoltageStatus,
            LaserVoltageOverload,
            SipmBiasPowerVoltageStatus,
            SipmBiasPowerVoltageOverload
        }

        public enum Registers : byte
        {
            SWReset = 0x00,
            LaserVoltage = 0x01,
            SipmBiasPowerVoltage = 0x02,
            Calibrate = 0x03,
            PulseCount = 0x04,
            VoltageStatus = 0x05
        }

        public enum RSL10Command : byte
        {
            StartLidar = 0x01,
            StopLidar = 0x02,
            StartTransfer = 0x03
        }

        public enum SwitchState : byte
        {
            TurnOff = 0x00,
            TurnOn = 0x01
        }

        public enum Voltage : byte
        {
            Target = 0x01,
            Actual = 0x02,
            Switch = 0x03
        }

        public enum RangeMeasurementErrorMessages
        {
            DeviceDisconnected,
            RangeFinderError
        }
    }
}
