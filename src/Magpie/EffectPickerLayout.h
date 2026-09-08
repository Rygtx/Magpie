#pragma once
#include <winrt/Windows.UI.Xaml.Controls.h>

namespace Magpie {

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
