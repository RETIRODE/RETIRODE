using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Bluetooth
{
    public class GattIdentifiers
    {
        public static Guid UartGattServiceId = Guid.Parse("");
        public static Guid UartGattCharacteristicReceiveId = Guid.Parse("");
        public static Guid UartGattCharacteristicSendId = Guid.Parse("");
        public static Guid SpecialNotificationDescriptorId = Guid.Parse("");
    }
}
