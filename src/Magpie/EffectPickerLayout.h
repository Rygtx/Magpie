#pragma once
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <algorithm>
#include <cmath>

namespace Magpie {

inline winrt::Windows::Foundation::Size EffectPickerSize(double workWidth, double workHeight, double scale) {
	// Reserve presenter padding, border, shadow and screen-edge clearance.
	scale = std::max(1.0, scale);
	return {float(std::floor(std::clamp(workWidth / scale - 64.0, 1.0, 820.0))),
		float(std::floor(std::clamp(workHeight / scale - 64.0, 1.0, 640.0)))};
}

inline void SetEffectPickerRowMargin(winrt::Windows::UI::Xaml::FrameworkElement const& row, double indent = 0) {
	// Collapsed children retain StackPanel.Spacing in Windows XAML. A collapsed
	// element's own margin, however, does not contribute to the panel's extent.
	row.Margin({indent, 0, 0, 2});
}

struct EffectPickerToggleIcon {
	winrt::Windows::UI::Xaml::Controls::Grid root;
	winrt::Windows::UI::Xaml::Shapes::Rectangle vertical;
	void Expanded(bool expanded) const {
		vertical.Visibility(expanded ? winrt::Windows::UI::Xaml::Visibility::Collapsed
			: winrt::Windows::UI::Xaml::Visibility::Visible);
	}
};

inline EffectPickerToggleIcon MakeEffectPickerToggleIcon(winrt::Windows::UI::Xaml::Controls::Button const& button) {
	using namespace winrt::Windows::UI::Xaml;
	EffectPickerToggleIcon icon;
	icon.root.Width(12); icon.root.Height(12);
	icon.root.VerticalAlignment(VerticalAlignment::Center);
	icon.root.HorizontalAlignment(HorizontalAlignment::Center);
	Shapes::Rectangle horizontal;
	horizontal.Width(10); horizontal.Height(1);
	icon.vertical.Width(1); icon.vertical.Height(10);
	for (const auto& line : {horizontal, icon.vertical}) {
		line.VerticalAlignment(VerticalAlignment::Center);
		line.HorizontalAlignment(HorizontalAlignment::Center);
		Data::Binding color;
		color.Source(button); color.Path(PropertyPath(L"Foreground"));
		color.Mode(Data::BindingMode::OneWay);
		line.SetBinding(Shapes::Shape::FillProperty(), color);
		icon.root.Children().Append(line);
	}
	return icon;
}

struct EffectPickerLayout {
	winrt::Windows::UI::Xaml::Controls::Grid root;
	winrt::Windows::UI::Xaml::Controls::Border categories, list, details;
};

inline EffectPickerLayout MakeEffectPickerLayout() {
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::UI::Xaml::Controls;
	EffectPickerLayout layout;
	ColumnDefinition left, right;
	left.Width({220, GridUnitType::Pixel});
	right.Width({1, GridUnitType::Star});
	layout.root.ColumnDefinitions().Append(left);
	layout.root.ColumnDefinitions().Append(right);
	RowDefinition upper, lower;
	upper.Height({7, GridUnitType::Star});
	lower.Height({3, GridUnitType::Star});
	layout.root.RowDefinitions().Append(upper);
	layout.root.RowDefinitions().Append(lower);
	layout.categories.Padding({6, 8, 6, 8});
	layout.categories.BorderThickness({0, 0, 1, 0});
	layout.list.Padding({10, 10, 10, 6});
	layout.details.Padding({16, 12, 16, 12});
	layout.details.BorderThickness({0, 1, 0, 0});
	Grid::SetColumn(layout.list, 1);
	Grid::SetRow(layout.details, 1);
	Grid::SetColumnSpan(layout.details, 2);
	layout.root.Children().Append(layout.categories);
	layout.root.Children().Append(layout.list);
	layout.root.Children().Append(layout.details);
	return layout;
}

} // namespace Magpie
