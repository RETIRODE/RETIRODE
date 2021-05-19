using static RETIRODE_APP.Models.Enums.ApplicationEnums;

namespace RETIRODE_APP.Models
{
    public class ResponseItem
    {
        public RangeFinderValues Identifier { get; set; }

        public object Value { get; set; }
    }
}
