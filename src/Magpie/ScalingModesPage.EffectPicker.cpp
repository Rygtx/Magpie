#include "pch.h"
#include "ScalingModesPage.h"
#include "EffectCatalog.h"
#include "EffectHelper.h"
#include "EffectsService.h"
#include "EffectPickerLayout.h"
#include <winrt/Windows.UI.Xaml.Automation.h>
using namespace ::Magpie;
using namespace winrt;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Input;
using Windows::UI::Xaml::Automation::AutomationProperties;
namespace winrt::Magpie::implementation {
namespace {
TextBlock PickerText(std::wstring_view text, double size = 14, bool singleLine = false) {
	TextBlock block;
	block.Text(text);
	block.FontSize(size);
	block.TextWrapping(singleLine ? TextWrapping::NoWrap : TextWrapping::Wrap);
	if (singleLine) {
		block.MaxLines(1);
		block.TextTrimming(TextTrimming::CharacterEllipsis);
	}
	return block;
}
ScrollViewer PickerScroll(UIElement const &content) {
	ScrollViewer scroll;
	scroll.Content(content);
	scroll.HorizontalScrollMode(ScrollMode::Disabled);
	scroll.HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
	scroll.VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
	scroll.IsTabStop(false);
	return scroll;
}
Button PickerButton(UIElement const &content) {
	Button button;
	button.Content(content);
	button.HorizontalAlignment(HorizontalAlignment::Stretch);
	button.HorizontalContentAlignment(HorizontalAlignment::Stretch);
	button.Padding({10, 8, 10, 8});
	button.BorderThickness({0});
	button.Background(SolidColorBrush(Windows::UI::Color{0, 0, 0, 0}));
	return button;
}
void Row(Grid const &grid, double height, GridUnitType type) {
	RowDefinition row;
	row.Height({height, type});
	grid.RowDefinitions().Append(row);
}
void Column(Grid const &grid, double width, GridUnitType type) {
	ColumnDefinition column;
	column.Width({width, type});
	grid.ColumnDefinitions().Append(column);
}
} // namespace
void ScalingModesPage::_BuildEffectPicker() {
	_pickerRows.clear();
	_pickerCategories.clear();
	const auto weak = get_weak();
	const auto &catalog = EffectCatalog::Get();
	for (const auto &effect : EffectsService::Get().Effects()) {
		if (RTXVideoStrength(effect.name) >= 0)
			continue;
		PickerRow row;
		if (const auto metadata = catalog.Find(effect.name))
			row.entry = *metadata;
		else {
			row.entry.id = effect.name;
			row.entry.name = EffectHelper::GetDisplayName(effect.name);
			row.entry.category = L"custom";
			row.entry.summary = L"自定义效果器；请参考作者说明。";
			row.entry.details =
				L"此效果器尚无用途 review。请参考作者提供的用途、参数与组合说明。HDR 兼容性待验证。";
			row.entry.searchText = NormalizeEffectSearch(effect.name);
		}
		_pickerRows.push_back(std::move(row));
	}
	std::ranges::sort(_pickerRows, {},
					  [](const PickerRow &row) { return NormalizeEffectSearch(row.entry.name); });
	auto layout = MakeEffectPickerLayout();
	_pickerRoot = layout.root;
	_pickerCategoryPane = layout.categories;
	_pickerListPane = layout.list;
	_pickerDetailPane = layout.details;
	StackPanel details;
	details.Spacing(6);
	_pickerDetailTitle = PickerText(L"全部", 16);
	_pickerDetailTitle.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
	_pickerDetails = PickerText(L"查看已安装效果器的用途、适用场景和组合建议。", 13);
	_pickerDetails.IsTextSelectionEnabled(true);
	details.Children().Append(_pickerDetailTitle);
	details.Children().Append(_pickerDetails);
	_pickerDetailScroll = PickerScroll(details);
	_pickerDetailPane.Child(_pickerDetailScroll);
	StackPanel categories;
	categories.Spacing(2);
	auto addCategory = [&](std::wstring name, std::wstring category, std::wstring subcategory,
						   std::wstring description, int parent, bool hasChildren) {
		const size_t index = _pickerCategories.size();
		PickerCategoryRow row;
		row.name = name;
		row.category = category;
		row.subcategory = subcategory;
		row.description = description;
		row.parent = parent;
		Grid layout;
		Column(layout, hasChildren ? 28 : 16, GridUnitType::Pixel);
		Column(layout, 1, GridUnitType::Star);
		row.arrow = PickerText(L"›", 18);
		if (hasChildren) {
			row.toggle = PickerButton(row.arrow);
			row.toggle.Padding({7, 4, 7, 4});
			AutomationProperties::SetName(row.toggle, L"展开" + name);
			row.toggle.Click([weak, index](auto const &, auto const &) {
				if (auto page = weak.get())
					page->_SetEffectCategoryExpanded(index, !page->_pickerCategories[index].expanded);
			});
			layout.Children().Append(row.toggle);
		}
		Grid label;
		Column(label, 1, GridUnitType::Star);
		Column(label, 1, GridUnitType::Auto);
		label.Children().Append(PickerText(name, 13, true));
		const auto count = std::ranges::count_if(_pickerRows, [&](const auto &effect) {
			return MatchesEffectCategory(effect.entry, category, subcategory);
		});
		auto number = PickerText(std::to_wstring(count), 11);
		number.Opacity(0.65);
		number.Margin({5, 0, 0, 0});
		number.VerticalAlignment(VerticalAlignment::Center);
		Grid::SetColumn(number, 1);
		label.Children().Append(number);
		row.button = PickerButton(label);
		row.button.Padding({4, 8, 6, 8});
		AutomationProperties::SetName(row.button, name + L"，" + std::to_wstring(count) + L" 个效果器");
		AutomationProperties::SetHelpText(row.button, description);
		row.button.Click([weak, index](auto const &, auto const &) {
			if (auto page = weak.get()) {
				const auto &item = page->_pickerCategories[index];
				page->_SetEffectCategoryExpanded(index, true);
				page->_ChooseEffectCategory(item.category, item.subcategory, item.description);
			}
		});
		auto show = [weak, index](auto const &, auto const &) {
			if (auto page = weak.get()) {
				const auto &item = page->_pickerCategories[index];
				page->_pickerDetailTitle.Text(item.name);
				page->_pickerDetails.Text(item.description);
				page->_pickerDetailScroll.ChangeView(nullptr, 0.0, nullptr, true);
			}
		};
		row.button.PointerEntered(show);
		row.button.GotFocus(show);
		row.button.KeyDown([weak, index](auto const &, KeyRoutedEventArgs const &args) {
			const auto page = weak.get();
			if (!page)
				return;
			auto &rows = page->_pickerCategories;
			auto &item = rows[index];
			const auto key = args.Key();
			if (key == VirtualKey::Right && item.toggle) {
				if (!item.expanded)
					page->_SetEffectCategoryExpanded(index, true);
				else if (index + 1 < rows.size())
					rows[index + 1].button.Focus(FocusState::Keyboard);
				args.Handled(true);
			} else if (key == VirtualKey::Left) {
				if (item.toggle && item.expanded)
					page->_SetEffectCategoryExpanded(index, false);
				else if (item.parent >= 0)
					rows[item.parent].button.Focus(FocusState::Keyboard);
				args.Handled(true);
			} else if (key == VirtualKey::Down || key == VirtualKey::Up) {
				const int step = key == VirtualKey::Down ? 1 : -1;
				for (int i = int(index) + step; i >= 0 && i < int(rows.size()); i += step)
					if (rows[i].container.Visibility() == Visibility::Visible) {
						rows[i].button.Focus(FocusState::Keyboard);
						break;
					}
				args.Handled(true);
			}
		});
		Grid::SetColumn(row.button, 1);
		layout.Children().Append(row.button);
		row.container = Border();
		row.container.CornerRadius({3});
		row.container.Child(layout);
		if (parent >= 0) {
			row.container.Margin({13, 0, 0, 0});
			row.container.BorderThickness({1, 0, 0, 0});
			row.container.Visibility(Visibility::Collapsed);
		}
		categories.Children().Append(row.container);
		_pickerCategories.push_back(std::move(row));
	};
	addCategory(L"推荐入门", L"first_try", L"", L"按适用场景选择容易上手的效果器，先从一个开始比较画面。", -1,
				false);
	addCategory(L"全部", L"", L"", L"查看已安装效果器的用途、适用场景和组合建议。", -1, false);
	for (const auto &category : catalog.Categories()) {
		const int parent = int(_pickerCategories.size());
		addCategory(category.name, category.id, L"", category.description, -1,
					!category.subcategories.empty());
		for (const auto &[name, description] : category.subcategories)
			addCategory(name, category.id, name, description, parent, false);
	}
	addCategory(L"自定义／未归类", L"custom", L"",
				L"已安装但尚无用途说明的效果器。请参考作者说明选择参数和组合位置。", -1, false);
	_pickerCategoryPane.Child(PickerScroll(categories));
	Grid right;
	Row(right, 1, GridUnitType::Auto);
	Row(right, 1, GridUnitType::Auto);
	Row(right, 1, GridUnitType::Star);
	_pickerListPane.Child(right);
	_pickerSearch = TextBox();
	_pickerSearch.PlaceholderText(L"搜索效果器、用途或关键词……");
	AutomationProperties::SetName(_pickerSearch, L"搜索全部效果器");
	right.Children().Append(_pickerSearch);
	_pickerCount = PickerText(L"", 12);
	_pickerCount.Margin({2, 7, 0, 7});
	_pickerCount.Opacity(0.7);
	Grid::SetRow(_pickerCount, 1);
	right.Children().Append(_pickerCount);
	StackPanel results;
	results.Spacing(2);
	_pickerListScroll = PickerScroll(results);
	Grid::SetRow(_pickerListScroll, 2);
	right.Children().Append(_pickerListScroll);
	for (size_t index = 0; index < _pickerRows.size(); ++index) {
		auto &row = _pickerRows[index];
		row.container = Grid();
		StackPanel content;
		content.Spacing(3);
		auto name = PickerText(row.entry.name, 14, true);
		name.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
		content.Children().Append(name);
		auto summary = PickerText(row.entry.summary, 12, true);
		summary.Opacity(0.75);
		content.Children().Append(summary);
		row.button = PickerButton(content);
		AutomationProperties::SetName(row.button, row.entry.name);
		AutomationProperties::SetHelpText(row.button, row.entry.summary);
		auto show = [weak, index](auto const &, auto const &) {
			if (auto page = weak.get())
				page->_ShowEffectPickerDetails(index);
		};
		row.button.PointerEntered(show);
		row.button.GotFocus(show);
		row.button.Click([weak, index](auto const &, auto const &) {
			if (auto page = weak.get())
				page->_AddPickedEffect(index);
		});
		row.button.KeyDown([weak, index](auto const &, KeyRoutedEventArgs const &args) {
			const auto page = weak.get();
			if (!page)
				return;
			if (args.Key() == VirtualKey::Enter) {
				args.Handled(true);
				page->_AddPickedEffect(index);
			} else if (args.Key() == VirtualKey::Down || args.Key() == VirtualKey::Up) {
				const int step = args.Key() == VirtualKey::Down ? 1 : -1;
				for (int i = int(index) + step; i >= 0 && i < int(page->_pickerRows.size()); i += step)
					if (page->_pickerRows[i].container.Visibility() == Visibility::Visible) {
						page->_pickerRows[i].button.Focus(FocusState::Keyboard);
						args.Handled(true);
						break;
					}
			}
		});
		row.container.Children().Append(row.button);
		results.Children().Append(row.container);
	}
	_pickerSearch.TextChanged([weak](auto const &, auto const &) {
		if (const auto page = weak.get(); page && !page->_pickerChangingCategory)
			page->_RefreshEffectPicker();
	});
	_pickerRoot.ActualThemeChanged([weak](auto const &, auto const &) {
		if (auto page = weak.get())
			page->_UpdateEffectPickerColors();
	});
	_UpdateEffectPickerColors();
	_effectPicker = Flyout();
	_effectPicker.Content(_pickerRoot);
	Windows::UI::Xaml::Style presenter(xaml_typename<FlyoutPresenter>());
	presenter.Setters().Append(Setter(FrameworkElement::MaxWidthProperty(), box_value(1000.0)));
	presenter.Setters().Append(Setter(FrameworkElement::MaxHeightProperty(), box_value(900.0)));
	presenter.Setters().Append(Setter(Control::PaddingProperty(), box_value(Thickness{0})));
	_effectPicker.FlyoutPresenterStyle(presenter);
	_effectPicker.Opened([weak](auto const &, auto const &) {
		if (auto page = weak.get())
			page->_pickerSearch.Focus(FocusState::Programmatic);
	});
	_effectPicker.Closed([weak](auto const &, auto const &) {
		if (auto page = weak.get())
			page->_pickerMode = nullptr;
	});
}
void ScalingModesPage::_SetEffectCategoryExpanded(size_t index, bool expanded) {
	if (index >= _pickerCategories.size() || !_pickerCategories[index].toggle)
		return;
	auto &item = _pickerCategories[index];
	item.expanded = expanded;
	item.arrow.Text(expanded ? L"⌄" : L"›");
	AutomationProperties::SetName(item.toggle, (expanded ? L"收起" : L"展开") + item.name);
	for (auto &child : _pickerCategories)
		if (child.parent == int(index))
			child.container.Visibility(expanded ? Visibility::Visible : Visibility::Collapsed);
	_UpdateEffectPickerColors();
}
void ScalingModesPage::_UpdateEffectPickerColors() {
	if (!_pickerRoot || !_pickerCategoryPane || !_pickerDetailPane)
		return;
	const bool dark = _pickerRoot.ActualTheme() == ElementTheme::Dark;
	auto brush = [](uint32_t rgb) {
		return SolidColorBrush(Windows::UI::Color{255, uint8_t(rgb >> 16), uint8_t(rgb >> 8), uint8_t(rgb)});
	};
	const auto line = brush(dark ? 0x4B525A : 0xBAC5D0);
	_pickerCategoryPane.Background(brush(dark ? 0x292E34 : 0xEDF1F5));
	_pickerListPane.Background(brush(dark ? 0x1D2126 : 0xFFFFFF));
	_pickerDetailPane.Background(brush(dark ? 0x353D45 : 0xDEE6EF));
	_pickerCategoryPane.BorderBrush(line);
	_pickerDetailPane.BorderBrush(line);
	const bool searching = _pickerSearch && !NormalizeEffectSearch(_pickerSearch.Text()).empty();
	for (auto &item : _pickerCategories) {
		const bool selected =
			!searching && item.category == _pickerCategory && item.subcategory == _pickerSubcategory;
		const bool hiddenSelection = !searching && item.toggle && !item.expanded &&
									 item.category == _pickerCategory && !_pickerSubcategory.empty();
		item.container.BorderBrush(line);
		item.container.Background(selected			? brush(dark ? 0x176077 : 0xB6DCEC)
								  : hiddenSelection ? brush(dark ? 0x3B4E59 : 0xD2E3EB)
													: SolidColorBrush(Windows::UI::Color{0, 0, 0, 0}));
		AutomationProperties::SetItemStatus(item.button, selected		   ? L"已选中"
														 : hiddenSelection ? L"已选中其子分组"
																		   : L"");
	}
}
void ScalingModesPage::_ChooseEffectCategory(std::wstring category, std::wstring subcategory,
											 std::wstring description) {
	_pickerCategory = std::move(category);
	_pickerSubcategory = std::move(subcategory);
	_pickerChangingCategory = true;
	_pickerSearch.Text(L"");
	_pickerChangingCategory = false;
	_RefreshEffectPicker();
	_pickerDetails.Text(description);
	_pickerDetailScroll.ChangeView(nullptr, 0.0, nullptr, true);
}
void ScalingModesPage::_RefreshEffectPicker() {
	const auto query = _pickerSearch.Text();
	const bool searching = !NormalizeEffectSearch(query).empty();
	size_t count = 0, first = _pickerRows.size();
	for (size_t i = 0; i < _pickerRows.size(); ++i) {
		auto &row = _pickerRows[i];
		const bool visible = searching
								 ? MatchesEffectSearch(row.entry.searchText, query)
								 : MatchesEffectCategory(row.entry, _pickerCategory, _pickerSubcategory);
		row.container.Visibility(visible ? Visibility::Visible : Visibility::Collapsed);
		if (visible) {
			++count;
			if (first == _pickerRows.size())
				first = i;
		}
		const auto problem =
			_pickerMode ? get_self<ScalingModeItem>(_pickerMode)->EffectAddProblem(hstring(row.entry.id))
						: hstring{};
		AutomationProperties::SetHelpText(row.button, problem.empty() ? hstring(row.entry.summary) : problem);
	}
	std::wstring scope = searching ? L"搜索全部分类" : L"全部";
	if (!searching)
		for (const auto &item : _pickerCategories)
			if (item.category == _pickerCategory && item.subcategory == _pickerSubcategory) {
				scope = item.name;
				if (item.parent >= 0)
					scope = _pickerCategories[item.parent].name + L" › " + item.name;
				_pickerDetailTitle.Text(item.name);
				_pickerDetails.Text(item.description);
				break;
			}
	_pickerCount.Text(scope + L" · " + std::to_wstring(count));
	if (!count) {
		_pickerDetailTitle.Text(L"没有匹配的效果器");
		_pickerDetails.Text(L"尝试缩短关键词、搜索算法家族名，或选择“全部”查看已安装效果器。");
	} else if (searching)
		_ShowEffectPickerDetails(first);
	_UpdateEffectPickerColors();
	_pickerListScroll.ChangeView(nullptr, 0.0, nullptr, true);
}
void ScalingModesPage::_ShowEffectPickerDetails(size_t index) {
	if (index >= _pickerRows.size())
		return;
	const auto &entry = _pickerRows[index].entry;
	const auto metadata = EffectCatalog::Get().Find(entry.id);
	_pickerDetailTitle.Text(entry.name);
	std::wstring text = metadata ? metadata->details : entry.details;
	if (_pickerMode) {
		const auto problem = get_self<ScalingModeItem>(_pickerMode)->EffectAddProblem(hstring(entry.id));
		if (!problem.empty())
			text = std::wstring(problem) + L"\n\n" + text;
	}
	_pickerDetails.Text(text);
	_pickerDetailScroll.ChangeView(nullptr, 0.0, nullptr, true);
}

void ScalingModesPage::_AddPickedEffect(size_t index) {
	if (!_pickerMode || index >= _pickerRows.size())
		return;
	const auto mode = _pickerMode;
	const hstring id(_pickerRows[index].entry.id);
	if (!get_self<ScalingModeItem>(mode)->EffectAddProblem(id).empty()) {
		_ShowEffectPickerDetails(index);
		return;
	}
	// Clear the target before dismissing so a second activation cannot append twice.
	_pickerMode = nullptr;
	_effectPicker.Hide();
	get_self<ScalingModeItem>(mode)->AddEffect(id);
}

} // namespace winrt::Magpie::implementation
