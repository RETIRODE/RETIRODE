using System;

namespace RETIRODE_APP.Models
{
    public static class Constants
    {
        //Gatt identifiers

        //Range Measurement Transfer 
        public static readonly Guid RangeMeasurementTransferServiceUUID = Guid.Parse("5177db0a-8ce6-11eb-8dcd-0242ac130003");
        public static readonly Guid RMTTimeOfFlightDataCharacteristic = Guid.Parse("5177dd8a-8ce6-11eb-8dcd-0242ac130003");
        public static readonly Guid RMTControlPointCharacteristic = Guid.Parse("5177de8e-8ce6-11eb-8dcd-0242ac130003");
        public static readonly Guid RMTInfoCharacteristic = Guid.Parse("5177df8f-8ce6-11eb-8dcd-0242ac130003");

        //Query and Commands
        public static readonly Guid QueryCommandServiceUUID = Guid.Parse("0b5c3ff8-9d7b-11eb-a8b3-0242ac130003");
        public static readonly Guid SendQueryCharacteristicUUID = Guid.Parse("0b5c4372-9d7b-11eb-a8b3-0242ac130003");
        public static readonly Guid SendCommandCharacteristicUUID = Guid.Parse("0b5c4278-9d7b-11eb-a8b3-0242ac130003");
        public static readonly Guid ReceiveQueryCharacteristicUUID = Guid.Parse("0b5c4444-9d7b-11eb-a8b3-0242ac130003");

        public static readonly string RetirodeUniqueMacAddressPart = "60:C0:BF";
        public static readonly string UniqueRetirodeName = "Retirode";
        public static readonly int UniqueMacAddressLength = 8;
    }
}
