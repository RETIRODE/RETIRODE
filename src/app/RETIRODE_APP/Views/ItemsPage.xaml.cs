using RETIRODE_APP.ViewModels;
using Xamarin.Forms;

namespace RETIRODE_APP.Views
{
    public partial class ItemsPage : ContentPage
    {
        ItemsViewModel _viewModel;

        public ItemsPage()
        {
            InitializeComponent();
            BindingContext = _viewModel = new ItemsViewModel();
        }

        protected override void OnAppearing()
        {
            base.OnAppearing();
            itemsRefreshView.IsRefreshing = true;
        }
    }
}