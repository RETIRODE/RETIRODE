using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    public static class Constants
    {
        //Gatt identifiers

        //Measurement Data Receive 
        public static readonly Guid GattFirstServiceId = Guid.Parse("5177db0a-8ce6-11eb-8dcd-0242ac130003");
        public static readonly Guid GattFirstCharacteristicReceiveId = Guid.Parse("e093f3b5-00a3-a9e5-9eca-40036e0edc24"); //TODO: Change
        public static readonly Guid GattFirstCharacteristicWriteId = Guid.Parse("5177de8e-8ce6-11eb-8dcd-0242ac130003");

        //Query and Commands
        public static readonly Guid GattQueryCommandServiceUUID = Guid.Parse("0b5c3ff8-9d7b-11eb-a8b3-0242ac130003");
        public static readonly Guid SendQueryCharacteristicUUID = Guid.Parse("0b5c4372-9d7b-11eb-a8b3-0242ac130003");
        public static readonly Guid SendCommandCharacteristicUUID = Guid.Parse("0b5c4278-9d7b-11eb-a8b3-0242ac130003");
        public static readonly Guid ReceiveQueryCharacteristicUUID = Guid.Parse("0b5c4444-9d7b-11eb-a8b3-0242ac130003");

        public static readonly string RetirodeUniqueMacAddressPart = "60:C0:BF";
        public static readonly string UniqueRetirodeName = "Retirode";
        public static readonly int UniqueMacAddressLength = 8;
    }
}
