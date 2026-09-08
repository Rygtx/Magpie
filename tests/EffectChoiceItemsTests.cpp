#include "../src/Magpie/EffectChoiceItems.h"
#include "../src/Magpie/EffectPickerLayout.h"
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <cassert>
#include <iostream>

int main(int argc, char**) {
	using namespace winrt;
	using namespace Windows::Foundation;
	using namespace Windows::Foundation::Collections;
	init_apartment(apartment_type::single_threaded);
	const auto broken = single_threaded_vector(std::vector<hstring>{ L"低", L"中", L"高", L"极高" });
	assert(!broken.try_as<IVector<IInspectable>>());
	assert(!broken.try_as<IIterable<IInspectable>>());
	const auto fixed = Magpie::MakeEffectChoiceItems({ L"低", L"中", L"高", L"极高" });
	const auto bindable = fixed.as<IVector<IInspectable>>();
	assert(bindable.Size() == 4);
	uint32_t index = 0;
	assert(bindable.IndexOf(box_value(L"高"), index) && index == 2);
	assert(!bindable.IndexOf(nullptr, index));
	assert(unbox_value<hstring>(bindable.GetAt(3)) == L"极高");
	std::cout << "Old string vector lacks XAML collection interfaces; production choices expose boxed values correctly.\n";
	if (argc > 1) {
		// No visible window or Magpie instance. Exercise the real XAML setter that
		// runs both during x:Bind initialization and during picker construction.
		const auto manager = Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
		Windows::UI::Xaml::Controls::ComboBox brokenCombo;
		bool rejected = false;
		try { brokenCombo.ItemsSource(broken); }
		catch (const hresult_error& error) { rejected = error.code().value == static_cast<int32_t>(0x80070057); }
		assert(rejected);
		Windows::UI::Xaml::Controls::ComboBox combo;
		combo.ItemsSource(fixed);
		for (int tier = 0; tier < 4; ++tier) {
			combo.SelectedIndex(tier);
			assert(combo.SelectedIndex() == tier);
			assert(unbox_value<hstring>(combo.SelectedItem()) == fixed.GetAt(tier));
		}
		using namespace Windows::UI::Xaml::Controls;
		const auto layout = Magpie::MakeEffectPickerLayout();
		assert(Grid::GetColumn(layout.list) == 1);
		assert(Grid::GetRow(layout.details) == 1 && Grid::GetColumnSpan(layout.details) == 2);
		assert(layout.root.Children().Size() == 3);
		TextBox search;
		layout.list.Child(search);
		TextBlock description;
		description.Text(L"用途与组合建议");
		layout.details.Child(description);
		for (const Size size : { Size{820, 640}, Size{480, 360}, Size{280, 220} }) {
			layout.root.Width(size.Width); layout.root.Height(size.Height);
			layout.root.ColumnDefinitions().GetAt(0).Width({ size.Width < 560 ? 160.0 : 220.0,
				Windows::UI::Xaml::GridUnitType::Pixel });
			layout.root.Measure(size);
			layout.root.Arrange({ 0, 0, size.Width, size.Height });
			assert(layout.details.ActualWidth() == size.Width);
			assert(layout.list.ActualWidth() > 0 && layout.categories.ActualWidth() < size.Width);
		}
		manager.Close();
		std::cout << "Real XAML: old ItemsSource throws E_INVALIDARG; fixed choices accept four values; three-region production layout passes at three sizes.\n";
	}
}
