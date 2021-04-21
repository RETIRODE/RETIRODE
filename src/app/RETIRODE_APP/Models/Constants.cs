using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    public static class Constants
    {
        //Gatt identifiers
        public static readonly Guid GattFirstServiceId = Guid.Parse("5177db0a-8ce6-11eb-8dcd-0242ac130003");
        public static readonly Guid GattFirstCharacteristicReceiveId = Guid.Parse("00000000000000000000000000000000");
        public static readonly Guid GattFirstCharacteristicWriteId = Guid.Parse("5177de8e-8ce6-11eb-8dcd-0242ac130003");
        public static readonly Guid GattSecondCharacteristicReceiveId = Guid.Parse("00000000000000000000000000000000");
        public static readonly Guid GattSecondCharacteristicWriteId = Guid.Parse("00000000000000000000000000000000");
        public static readonly Guid GattSecondServiceId = Guid.Parse("00000000000000000000000000000000");
        public static readonly string RetirodeUniqueMacAddressPart = "60:C0:BF";
        public static readonly string UniqueRetirodeName = "Retirode";
        public static readonly int UniqueMacAddressLength = 8;
    }
}
