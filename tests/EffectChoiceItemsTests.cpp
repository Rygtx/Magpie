#include "../src/Magpie/EffectChoiceItems.h"
#include "../src/Magpie/EffectPickerLayout.h"
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Media.h>
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
		using namespace Windows::UI::Xaml;
		Flyout flyout;
		assert(flyout.ShouldConstrainToRootBounds());
		flyout.ShouldConstrainToRootBounds(false);
		assert(!flyout.ShouldConstrainToRootBounds());
		for (int hidden : {0, 1, 10, 100}) {
			for (int placement : {0, 1, 2}) {
				StackPanel panel;
				Grid first, last;
				first.Height(50); last.Height(50);
				Magpie::SetEffectPickerRowMargin(first); Magpie::SetEffectPickerRowMargin(last);
				if (placement != 0) panel.Children().Append(first);
				if (placement == 2) panel.Children().Append(last);
				for (int i = 0; i < hidden; ++i) {
					Grid row;
					row.Height(50); Magpie::SetEffectPickerRowMargin(row, 13);
					row.Visibility(Visibility::Collapsed);
					panel.Children().Append(row);
				}
				if (placement == 0) panel.Children().Append(first);
				if (placement != 2) panel.Children().Append(last);
				auto measure = [&] {
					panel.Measure({300, 10000}); panel.Arrange({0, 0, 300, panel.DesiredSize().Height});
				};
				measure();
				assert(panel.DesiredSize().Height == 104);
				assert(last.TransformToVisual(panel).TransformPoint({0, 0}).Y == 52);
				last.Visibility(Visibility::Collapsed); measure();
				assert(panel.DesiredSize().Height == 52);
				first.Visibility(Visibility::Collapsed); measure();
				assert(panel.DesiredSize().Height == 0);
			}
		}
		Button toggle;
		const auto icon = Magpie::MakeEffectPickerToggleIcon(toggle);
		for (bool expanded : {false, true, false}) {
			icon.Expanded(expanded); icon.root.Measure({100, 100}); icon.root.Arrange({0, 0, 12, 12});
			assert(icon.root.DesiredSize().Width == 12 && icon.root.DesiredSize().Height == 12);
			assert(icon.vertical.Visibility() == (expanded ? Visibility::Collapsed : Visibility::Visible));
		}
		for (double dpi : {96.0, 144.0, 192.0, 288.0}) {
			for (Size work : {Size{1920, 1040}, Size{1280, 680}, Size{800, 560}}) {
				const auto size = Magpie::EffectPickerSize(work.Width, work.Height, dpi / 96);
				assert(size.Width <= 820 && size.Height <= 640);
				assert((size.Width + 64) * dpi / 96 <= work.Width);
				assert((size.Height + 64) * dpi / 96 <= work.Height);
			}
		}
		std::cout << "Windowless XAML: hidden 0/1/10/100 rows at head/middle/tail, all-hidden/single result, +/- geometry and monitor sizing passed.\n";
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
