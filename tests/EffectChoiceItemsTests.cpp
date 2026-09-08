#include "../src/Magpie/EffectChoiceItems.h"
#include "../src/Magpie/EffectPickerLayout.h"
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.Data.Json.h>
#include <fstream>
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
				assert(size.Width <= 820 && size.Height <= 800);
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
		assert(Magpie::EffectPickerSize(1920, 1040, 1).Height == 800);
		for (const Size size : { Size{820, 800}, Size{820, 640}, Size{480, 360}, Size{280, 220} }) {
			layout.root.Width(size.Width); layout.root.Height(size.Height);
			layout.root.ColumnDefinitions().GetAt(0).Width({ size.Width < 560 ? 160.0 : 220.0,
				Windows::UI::Xaml::GridUnitType::Pixel });
			layout.root.Measure(size);
			layout.root.Arrange({ 0, 0, size.Width, size.Height });
			assert(layout.details.ActualWidth() == size.Width);
			assert(layout.list.ActualWidth() > 0 && layout.categories.ActualWidth() < size.Width);
			if (size.Height == 800) {
				assert(layout.details.ActualHeight() == 144);
				assert(layout.list.ActualHeight() == 656);
			}
		}
		StackPanel detailContent;
		detailContent.Spacing(6);
		TextBlock detailTitle, detailBody;
		detailTitle.FontSize(15); detailTitle.TextWrapping(TextWrapping::Wrap);
		detailBody.FontSize(12); detailBody.TextWrapping(TextWrapping::Wrap);
		detailContent.Children().Append(detailTitle); detailContent.Children().Append(detailBody);
		ScrollViewer detailScroll;
		detailScroll.HorizontalScrollMode(ScrollMode::Disabled);
		detailScroll.HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
		detailScroll.VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
		detailScroll.Content(detailContent);
		layout.details.Child(detailScroll);
		double lastMeasuredContent = 0;
		auto measureDetails = [&](hstring const& title, hstring const& text, float width, float height) {
			detailTitle.Text(title); detailBody.Text(text);
			// Opening at a new monitor size resets the pixel row in production.
			if (layout.root.Width() != width || layout.root.Height() != height)
				layout.root.RowDefinitions().GetAt(1).Height({18, GridUnitType::Star});
			layout.root.Width(width); layout.root.Height(height);
			auto arrange = [&] {
				layout.root.Measure({width, height}); layout.root.Arrange({0, 0, width, height});
			};
			arrange();
			const auto required = Magpie::MeasureEffectPickerDetailsHeight(layout.root, layout.details, detailContent);
			const double measuredContent = detailContent.DesiredSize().Height;
			lastMeasuredContent = measuredContent;
			layout.root.RowDefinitions().GetAt(1).Height({required, GridUnitType::Pixel});
			arrange();
			if (layout.categories.ActualHeight() < std::min(180.0, height * 0.45) - 1)
				std::cerr << "Adaptive allocation: root=" << layout.root.ActualHeight() << " requested=" << height
					<< " upper=" << layout.categories.ActualHeight() << " details=" << layout.details.ActualHeight()
					<< " computed=" << required << "\n";
			assert(layout.categories.ActualHeight() >= std::min(180.0, height * 0.45) - 1);
			assert(layout.details.ActualHeight() >= height * 0.18 - 1);
			assert(layout.details.ActualHeight() + layout.categories.ActualHeight() <= height + 1);
			// The native text is measured even without a loaded ScrollContentPresenter.
			// Compare its full wrapped extent with the actual allocated viewport.
			if (required < height - std::min(180.0, height * 0.45) - 1)
				assert(measuredContent + 25 <= layout.details.ActualHeight() + 1);
			return required;
		};
		std::ifstream input("src/Magpie/EffectCatalog/zh-Hans.json", std::ios::binary);
		const std::string bytes{std::istreambuf_iterator<char>(input), {}};
		const auto entries = Windows::Data::Json::JsonObject::Parse(to_hstring(bytes)).GetNamedArray(L"effects");
		assert(entries.Size() == 155);
		size_t expanded = 0;
		for (const auto value : entries) {
			const auto entry = value.GetObject();
			for (const auto width : {820.0f, 480.0f}) {
				const double used = measureDetails(entry.GetNamedString(L"name"), entry.GetNamedString(L"details"), width, 800);
				if (used > 144) ++expanded;
			}
		}
		assert(expanded > 0);
		assert(measureDetails(L"入门", L"从一个效果器开始比较画面。", 820, 800) == 144);
		std::wstring veryLong;
		for (int i = 0; i < 200; ++i) veryLong += L"极长说明仍可滚动阅读。\n";
		assert(measureDetails(L"长说明", hstring(veryLong), 820, 800) == 620);
		assert(lastMeasuredContent > layout.details.ActualHeight());
		measureDetails(L"长说明", hstring(veryLong), 280, 220);
		assert(lastMeasuredContent > layout.details.ActualHeight());
		assert(detailScroll.VerticalScrollBarVisibility() == ScrollBarVisibility::Auto);
		assert(measureDetails(L"入门", L"短说明", 820, 800) == 144);
		std::cout << "Adaptive details: all 155 catalog entries at two widths, long-to-short reset, and small-screen scroll fallback passed.\n";
		manager.Close();
		std::cout << "Real XAML: old ItemsSource throws E_INVALIDARG; fixed choices accept four values; production layout passes at four sizes, with 800-DIP height and 144-DIP details.\n";
	}
}
