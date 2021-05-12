using System;
using System.Globalization;
using Xamarin.Forms;

namespace RETIRODE_APP.Converters
{
    public class OkNotOkTextConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is bool?)
            {
                var isOverload = (bool?)value;

                if (isOverload is null)
                {
                    return "-";
                }
                else if (isOverload.Value)
                {
                    return "NOT OK";
                }
                else
                {
                    return "OK";
                }
            }

            return "-";
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }
    }

    public class OverloadColorConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is bool?)
            {
                var isOverload = (bool?)value;

                if (isOverload is null)
                {
                    return Color.FromHex("#737475");
                }
                else if (isOverload.Value)
                {
                    return Color.FromHex("#D6676C");
                }
                else
                {
                    return Color.FromHex("#78D778");
                }
            }

            return Color.FromHex("#737475");
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }
    }
}