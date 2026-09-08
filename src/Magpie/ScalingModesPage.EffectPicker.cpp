#include "pch.h"
#include "ScalingModesPage.h"
#include "EffectCatalog.h"
#include "EffectHelper.h"
#include "EffectsService.h"
#include "EffectPickerLayout.h"
#include "App.h"
#include "MainWindow.h"
#include "XamlHelper.h"
#include <shellscalingapi.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
using namespace ::Magpie;
using namespace winrt;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Input;
using Windows::UI::Xaml::Automation::AutomationProperties;
namespace winrt::Magpie::implementation {
namespace {
TextBlock PickerText(std::wstring_view text, double size = 13, bool singleLine = false) {
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
Button ScalingModesPage::_EffectPickerButton(UIElement const& content) {
	Button button;
	button.Style(Resources().Lookup(box_value(L"EffectPickerButtonStyle")).as<Windows::UI::Xaml::Style>());
	button.Content(content);
	return button;
}
void ScalingModesPage::_SizeEffectPicker(Button const& anchor) {
	const auto& window = App::Get().MainWindow();
	const auto center = anchor.TransformToVisual(XamlRoot().Content()).TransformPoint(
		{float(anchor.ActualWidth() / 2), float(anchor.ActualHeight() / 2)});
	const auto monitor = MonitorFromPoint(window.XamlRootPointToScreen(center), MONITOR_DEFAULTTONEAREST);
	MONITORINFO info{sizeof(info)};
	check_bool(GetMonitorInfo(monitor, &info));
	UINT dpiX = window.CurrentDpi(), dpiY = dpiX;
	GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
	// Windowed XAML popups still use their owning XamlRoot's rasterization scale.
	// The greater scale also bounds the first show across a mixed-DPI seam.
	const double scale = std::max(dpiX / 96.0, XamlRoot().RasterizationScale());
	const auto size = EffectPickerSize(info.rcWork.right - info.rcWork.left,
		info.rcWork.bottom - info.rcWork.top, scale);
	_pickerRoot.Width(size.Width); _pickerRoot.Height(size.Height);
	_pickerRoot.ColumnDefinitions().GetAt(0).Width({std::min(size.Width * 0.4,
		size.Width < 560 ? 160.0 : 220.0), GridUnitType::Pixel});
}
void ScalingModesPage::_BuildEffectPicker() {
	_pickerRows.clear();
	_pickerEntries.clear();
	_pickerCategories.clear();
	_pickerRailLetters.clear();
	_pickerGridLetters.clear();
	const auto weak = get_weak();
	const auto &catalog = EffectCatalog::Get();
	for (const auto &effect : EffectsService::Get().Effects()) {
		if (RTXVideoStrength(effect.name) >= 0)
			continue;
		EffectPickerEntry entry;
		if (const auto metadata = catalog.Find(effect.name))
			entry = *metadata;
		else {
			entry.id = effect.name;
			entry.name = EffectHelper::GetDisplayName(effect.name);
			entry.category = L"custom";
			entry.summary = L"自定义效果器；请参考作者说明。";
			entry.details =
				L"此效果器尚无用途 review。请参考作者提供的用途、参数与组合说明。HDR 兼容性待验证。";
			entry.searchText = NormalizeEffectSearch(effect.name);
		}
		_pickerEntries.push_back(std::move(entry));
	}
	auto layout = MakeEffectPickerLayout();
	_pickerRoot = layout.root;
	_pickerCategoryPane = layout.categories;
	_pickerListPane = layout.list;
	_pickerDetailPane = layout.details;
	StackPanel details;
	details.Spacing(6);
	_pickerDetailTitle = PickerText(L"全部", 15);
	_pickerDetailTitle.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
	_pickerDetails = PickerText(L"查看已安装效果器的用途、适用场景和组合建议。", 12);
	_pickerDetails.IsTextSelectionEnabled(true);
	details.Children().Append(_pickerDetailTitle);
	details.Children().Append(_pickerDetails);
	_pickerDetailScroll = PickerScroll(details);
	_pickerDetailPane.Child(_pickerDetailScroll);
	StackPanel categories;
	// Spacing belongs to visible row containers, including collapsed categories.
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

		if (hasChildren) {
			row.toggle = _EffectPickerButton(nullptr);
			row.icon = MakeEffectPickerToggleIcon(row.toggle);
			row.toggle.Content(row.icon.root);
			row.toggle.Padding({8, 4, 8, 4});
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
		label.Children().Append(PickerText(name, 12, true));
		const auto count = std::ranges::count_if(_pickerEntries, [&](const auto &effect) {
			return MatchesEffectCategory(effect, category, subcategory);
		});
		auto number = PickerText(std::to_wstring(count), 10);
		number.Opacity(0.65);
		number.Margin({5, 0, 0, 0});
		number.VerticalAlignment(VerticalAlignment::Center);
		Grid::SetColumn(number, 1);
		label.Children().Append(number);
		row.button = _EffectPickerButton(label);
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
		row.container.Style(Resources().Lookup(box_value(L"EffectPickerCategoryStyle")).as<Windows::UI::Xaml::Style>());
		SetEffectPickerRowMargin(row.container);
		row.selectionMark = Resources().Lookup(box_value(L"EffectPickerSelectionMark")).as<DataTemplate>().LoadContent().as<FrameworkElement>();
		row.selectionMark.IsHitTestVisible(false);
		Grid::SetColumnSpan(row.selectionMark, 2);
		layout.Children().Append(row.selectionMark);
		row.container.Child(layout);
		if (parent >= 0) {
			SetEffectPickerRowMargin(row.container, 13);
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
	_pickerCategoryScroll = PickerScroll(categories);
	_pickerCategoryPane.Child(_pickerCategoryScroll);
	Grid right;
	Row(right, 1, GridUnitType::Auto);
	Row(right, 1, GridUnitType::Auto);
	Row(right, 1, GridUnitType::Star);
	_pickerListPane.Child(right);
	_pickerSearch = TextBox();
	_pickerSearch.FontSize(13);
	_pickerSearch.PlaceholderText(L"搜索效果器、用途或关键词……");
	AutomationProperties::SetName(_pickerSearch, L"搜索全部效果器");
	right.Children().Append(_pickerSearch);
	_pickerCount = PickerText(L"", 11);
	_pickerCount.Margin({2, 7, 0, 7});
	_pickerCount.Opacity(0.7);
	Grid::SetRow(_pickerCount, 1);
	right.Children().Append(_pickerCount);
	Grid resultsArea;
	Column(resultsArea, 1, GridUnitType::Star);
	Column(resultsArea, 1, GridUnitType::Auto);
	Grid::SetRow(resultsArea, 2);
	right.Children().Append(resultsArea);
	_pickerResults = StackPanel();
	_pickerListScroll = PickerScroll(_pickerResults);
	resultsArea.Children().Append(_pickerListScroll);
	_pickerIndexPane = Grid();
	_pickerIndexPane.Margin({6, 0, 0, 0});
	Grid::SetColumn(_pickerIndexPane, 1);
	resultsArea.Children().Append(_pickerIndexPane);
	_pickerIndexRail = Grid();
	_pickerIndexRail.VerticalAlignment(VerticalAlignment::Top);
	for (int i = 0; i < 2; ++i) Column(_pickerIndexRail, 24, GridUnitType::Pixel);
	for (int i = 0; i < 14; ++i) Row(_pickerIndexRail, 24, GridUnitType::Pixel);
	_pickerIndexPane.Children().Append(_pickerIndexRail);
	_pickerIndexButton = _EffectPickerButton(PickerText(L"A–Z\n#", 10));
	_pickerIndexButton.Padding({4, 6, 4, 6});
	_pickerIndexButton.VerticalAlignment(VerticalAlignment::Top);
	AutomationProperties::SetName(_pickerIndexButton, L"按首字母定位效果器");
	_pickerIndexPane.Children().Append(_pickerIndexButton);
	Grid letters;
	for (int i = 0; i < 6; ++i) Column(letters, 36, GridUnitType::Pixel);
	for (int i = 0; i < 5; ++i) Row(letters, 36, GridUnitType::Pixel);
	for (int i = 0; i < 27; ++i) {
		const std::wstring name(1, i == 26 ? L'#' : wchar_t(L'A' + i));
		for (bool rail : {true, false}) {
			auto button = _EffectPickerButton(PickerText(name, rail ? 10 : 13, true));
			button.Padding({0});
			button.HorizontalContentAlignment(HorizontalAlignment::Center);
			button.Width(rail ? 24 : 36); button.Height(rail ? 24 : 36);
			AutomationProperties::SetName(button, L"定位 " + name);
			button.Click([weak, i](auto const&, auto const&) {
				if (auto page = weak.get()) page->_JumpEffectPickerLetter(i);
			});
			button.KeyDown([weak, i, rail](auto const&, KeyRoutedEventArgs const& args) {
				const auto page = weak.get();
				if (!page) return;
				const auto key = args.Key();
				int step = 0;
				if (key == VirtualKey::Down) step = rail ? 1 : 6;
				else if (key == VirtualKey::Up) step = rail ? -1 : -6;
				else if (key == VirtualKey::Right) step = rail ? 14 : 1;
				else if (key == VirtualKey::Left) step = rail ? -14 : -1;
				if (!step) return;
				const auto& letters = rail ? page->_pickerRailLetters : page->_pickerGridLetters;
				for (int next = i + step; next >= 0 && next < 27; next += step) {
					if (letters[next].IsEnabled()) { letters[next].Focus(FocusState::Keyboard); break; }
				}
				args.Handled(true);
			});
			if (rail) {
				Grid::SetColumn(button, i / 14); Grid::SetRow(button, i % 14);
				_pickerIndexRail.Children().Append(button);
				_pickerRailLetters.push_back(button);
			} else {
				Grid::SetColumn(button, i % 6); Grid::SetRow(button, i / 6);
				letters.Children().Append(button);
				_pickerGridLetters.push_back(button);
			}
		}
	}
	_pickerIndexFlyout = Flyout();
	_pickerIndexFlyout.ShouldConstrainToRootBounds(false);
	_pickerIndexFlyout.Content(letters);
	_pickerIndexFlyout.Opened([weak](auto const&, auto const&) {
		if (auto page = weak.get()) page->_pickerIndexOpen = true;
	});
	_pickerIndexFlyout.Closed([weak](auto const&, auto const&) {
		if (auto page = weak.get()) {
			page->_pickerIndexOpen = false;
			const int letter = std::exchange(page->_pickerPendingLetter, -1);
			if (page->_pickerMode && letter >= 0) page->_JumpEffectPickerLetter(letter);
		}
	});
	_pickerIndexButton.Flyout(_pickerIndexFlyout);
	_pickerIndexPane.SizeChanged([weak](auto const&, auto const&) {
		if (auto page = weak.get()) page->_UpdateEffectPickerIndex();
	});
	_pickerSearch.TextChanged([weak](auto const &, auto const &) {
		if (const auto page = weak.get(); page && !page->_pickerChangingCategory)
			page->_RefreshEffectPicker();
	});
	const auto paneStyle = Resources().Lookup(box_value(L"EffectPickerPaneStyle")).as<Windows::UI::Xaml::Style>();
	_pickerCategoryPane.Style(paneStyle);
	_pickerDetailPane.Style(paneStyle);
	_UpdateEffectPickerColors();
	_effectPicker = Flyout();
	_effectPicker.ShouldConstrainToRootBounds(false);
	_effectPicker.Content(_pickerRoot);
	_effectPicker.FlyoutPresenterStyle(Resources().Lookup(box_value(L"EffectPickerPresenterStyle")).as<Windows::UI::Xaml::Style>());
	_effectPicker.Opened([weak](auto const &, auto const &) {
		if (auto page = weak.get())
			page->_pickerSearch.Focus(FocusState::Programmatic);
	});
	_effectPicker.Closed([weak](auto const &, auto const &) {
		if (auto page = weak.get()) {
			page->_pickerMode = nullptr;
			page->_pickerPendingLetter = -1;
			page->_pickerIndexFlyout.Hide();
		}
	});
}
void ScalingModesPage::_SetEffectCategoryExpanded(size_t index, bool expanded) {
	if (index >= _pickerCategories.size() || !_pickerCategories[index].toggle)
		return;
	auto &item = _pickerCategories[index];
	const double anchorTop = item.container.TransformToVisual(_pickerCategoryScroll).TransformPoint({0, 0}).Y;
	const auto focused = FocusManager::GetFocusedElement(XamlRoot()).try_as<DependencyObject>();
	bool focusHidden = false;
	item.expanded = expanded;
	item.icon.Expanded(expanded);
	AutomationProperties::SetItemStatus(item.toggle, expanded ? L"已展开" : L"已收起");
	AutomationProperties::SetName(item.toggle, (expanded ? L"收起" : L"展开") + item.name);
	for (auto &child : _pickerCategories) {
		if (child.parent == int(index)) {
			focusHidden |= !expanded && focused && XamlHelper::ContainsControl(child.container, focused);
			child.container.Visibility(expanded ? Visibility::Visible : Visibility::Collapsed);
		}
	}
	if (focusHidden) item.button.Focus(FocusState::Keyboard);
	_pickerRoot.UpdateLayout();
	const double moved = item.container.TransformToVisual(_pickerCategoryScroll).TransformPoint({0, 0}).Y - anchorTop;
	_pickerCategoryScroll.ChangeView(nullptr, std::max(0.0, _pickerCategoryScroll.VerticalOffset() + moved), nullptr, true);
	_UpdateEffectPickerColors();
}
void ScalingModesPage::_UpdateEffectPickerColors() {
	const bool searching = _pickerSearch && !NormalizeEffectSearch(_pickerSearch.Text()).empty();
	for (auto& item : _pickerCategories) {
		const bool selected = !searching && item.category == _pickerCategory && item.subcategory == _pickerSubcategory;
		const bool hiddenSelection = !searching && item.toggle && !item.expanded &&
			item.category == _pickerCategory && !_pickerSubcategory.empty();
		item.container.Style(Resources().Lookup(box_value(selected || hiddenSelection
			? L"EffectPickerSelectedCategoryStyle" : L"EffectPickerCategoryStyle")).as<Windows::UI::Xaml::Style>());
		item.selectionMark.Visibility(selected || hiddenSelection ? Visibility::Visible : Visibility::Collapsed);
		AutomationProperties::SetItemStatus(item.button, selected ? L"已选中" : hiddenSelection ? L"已选中其子分组" : L"");
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
void ScalingModesPage::_RefreshEffectPicker(std::wstring anchor) {
	double anchorTop = 0;
	Windows::UI::Xaml::FocusState anchorFocus = FocusState::Programmatic;
	for (const auto& row : _pickerRows) {
		if (row.entry.key == anchor) {
			anchorTop = row.button.TransformToVisual(_pickerResults).TransformPoint({0, 0}).Y
				- _pickerListScroll.VerticalOffset();
			if (row.button.FocusState() == FocusState::Keyboard) anchorFocus = FocusState::Keyboard;
			break;
		}
	}
	const std::wstring query(_pickerSearch.Text());
	const bool searching = !NormalizeEffectSearch(query).empty();
	if (_pickerLastQuery != query) {
		_pickerSearchCollapsedFamilies.clear();
		_pickerLastQuery = query;
	}
	const auto tree = BuildEffectPickerTree(_pickerEntries, _pickerCategory, _pickerSubcategory,
		query, _pickerExpandedFamilies, _pickerSearchCollapsedFamilies);
	_pickerResults.Children().Clear();
	_pickerRows.clear();
	_pickerLetterTargets.fill({});
	const auto weak = get_weak();
	for (const auto& entry : tree.rows) {
		PickerRow row;
		row.entry = entry;
		row.button = _EffectPickerButton(nullptr);
		SetEffectPickerRowMargin(row.button, entry.depth * 14.0);
		Grid content;
		Column(content, 20, GridUnitType::Pixel);
		Column(content, 1, GridUnitType::Star);
		if (entry.IsFamily()) {
			auto icon = MakeEffectPickerToggleIcon(row.button);
			icon.Expanded(entry.expanded);
			icon.root.VerticalAlignment(VerticalAlignment::Top);
			icon.root.Margin({0, 4, 6, 0});
			content.Children().Append(icon.root);
		}
		StackPanel text;
		Grid label;
		Column(label, 1, GridUnitType::Star);
		Column(label, 1, GridUnitType::Auto);
		auto name = PickerText(entry.name, 13, true);
		name.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
		label.Children().Append(name);
		if (entry.IsFamily()) {
			auto count = PickerText(std::to_wstring(entry.count), 10);
			count.Opacity(0.7); count.Margin({8, 0, 0, 0});
			count.VerticalAlignment(VerticalAlignment::Center);
			Grid::SetColumn(count, 1);
			label.Children().Append(count);
			AutomationProperties::SetItemStatus(row.button, entry.expanded ? L"已展开" : L"已收起");
		}
		text.Children().Append(label);
		if (!entry.summary.empty()) {
			auto summary = PickerText(entry.summary, 11, true);
			summary.Opacity(0.75); summary.Margin({0, 3, 0, 0});
			text.Children().Append(summary);
		}
		Grid::SetColumn(text, 1);
		content.Children().Append(text);
		row.button.Content(content);
		AutomationProperties::SetName(row.button, entry.IsFamily()
			? entry.name + L"，" + std::to_wstring(entry.count) + L" 个效果器" : entry.name);
		const auto problem = !entry.IsFamily() && _pickerMode
			? get_self<ScalingModeItem>(_pickerMode)->EffectAddProblem(hstring(entry.effectId)) : hstring{};
		AutomationProperties::SetHelpText(row.button, problem.empty() ? hstring(entry.summary) : problem);
		auto show = [weak, key = entry.key](auto const&, auto const&) {
			if (auto page = weak.get()) page->_ShowEffectPickerDetails(key);
		};
		row.button.PointerEntered(show);
		row.button.GotFocus(show);
		row.button.Click([weak, key = entry.key, id = entry.effectId](auto const&, auto const&) {
			if (auto page = weak.get()) {
				if (id.empty()) page->_ToggleEffectFamily(key);
				else page->_AddPickedEffect(id);
			}
		});
		row.button.KeyDown([weak, key = entry.key](auto const&, KeyRoutedEventArgs const& args) {
			if (auto page = weak.get()) page->_EffectPickerRowKeyDown(key, args);
		});
		if (entry.depth == 0) {
			auto& target = _pickerLetterTargets[EffectPickerLetter(entry.name)];
			if (target.empty()) target = entry.key;
		}
		_pickerResults.Children().Append(row.button);
		_pickerRows.push_back(std::move(row));
	}
	std::wstring scope = searching ? L"搜索全部分类" : L"全部";
	if (!searching && anchor.empty()) {
		for (const auto& item : _pickerCategories) {
			if (item.category != _pickerCategory || item.subcategory != _pickerSubcategory) continue;
			scope = item.parent >= 0 ? _pickerCategories[item.parent].name + L" › " + item.name : item.name;
			_pickerDetailTitle.Text(item.name);
			_pickerDetails.Text(item.description);
			break;
		}
	} else if (!searching) {
		for (const auto& item : _pickerCategories)
			if (item.category == _pickerCategory && item.subcategory == _pickerSubcategory)
				scope = item.parent >= 0 ? _pickerCategories[item.parent].name + L" › " + item.name : item.name;
	}
	_pickerCount.Text(scope + L" · " + std::to_wstring(tree.effectCount));
	if (!tree.effectCount) {
		_pickerDetailTitle.Text(L"没有匹配的效果器");
		_pickerDetails.Text(L"尝试缩短关键词、搜索算法家族名，或选择“全部”查看已安装效果器。");
	} else if (searching && anchor.empty()) {
		_ShowEffectPickerDetails(_pickerRows.front().entry.key);
	}
	_UpdateEffectPickerColors();
	_UpdateEffectPickerIndex();
	_pickerRoot.UpdateLayout();
	if (anchor.empty()) {
		_pickerListScroll.ChangeView(nullptr, 0.0, nullptr, true);
		_pickerDetailScroll.ChangeView(nullptr, 0.0, nullptr, true);
	} else {
		for (const auto& row : _pickerRows) {
			if (row.entry.key != anchor) continue;
			row.button.Focus(anchorFocus);
			const double top = row.button.TransformToVisual(_pickerResults).TransformPoint({0, 0}).Y;
			_pickerListScroll.ChangeView(nullptr, std::max(0.0, top - anchorTop), nullptr, true);
			break;
		}
	}
}

void ScalingModesPage::_ShowEffectPickerDetails(std::wstring_view key) {
	const auto it = std::ranges::find(_pickerRows, key, [](const auto& row) { return std::wstring_view(row.entry.key); });
	if (it == _pickerRows.end()) return;
	const auto& row = it->entry;
	_pickerDetailTitle.Text(row.name);
	std::wstring text = row.summary;
	if (!row.IsFamily()) {
		const auto entry = std::ranges::find(_pickerEntries, row.effectId, &EffectPickerEntry::id);
		if (entry != _pickerEntries.end()) text = entry->details;
		if (_pickerMode) {
			const auto problem = get_self<ScalingModeItem>(_pickerMode)->EffectAddProblem(hstring(row.effectId));
			if (!problem.empty()) text = std::wstring(problem) + L"\n\n" + text;
		}
	}
	_pickerDetails.Text(text);
	_pickerDetailScroll.ChangeView(nullptr, 0.0, nullptr, true);
}

void ScalingModesPage::_ToggleEffectFamily(std::wstring_view key) {
	const auto it = std::ranges::find(_pickerRows, key, [](const auto& row) { return std::wstring_view(row.entry.key); });
	if (it == _pickerRows.end() || !it->entry.IsFamily()) return;
	const auto stableKey = it->entry.key;
	const bool searching = !NormalizeEffectSearch(_pickerSearch.Text()).empty();
	auto& states = searching ? _pickerSearchCollapsedFamilies : _pickerExpandedFamilies;
	if (searching ? it->entry.expanded : !it->entry.expanded) states.insert(stableKey);
	else states.erase(stableKey);
	_RefreshEffectPicker(stableKey);
}

void ScalingModesPage::_EffectPickerRowKeyDown(std::wstring_view key, KeyRoutedEventArgs const& args) {
	const auto it = std::ranges::find(_pickerRows, key, [](const auto& row) { return std::wstring_view(row.entry.key); });
	if (it == _pickerRows.end()) return;
	const size_t index = size_t(it - _pickerRows.begin());
	const auto entry = it->entry;
	const auto pressed = args.Key();
	if (pressed == VirtualKey::Enter) {
		args.Handled(true);
		if (entry.IsFamily()) _ToggleEffectFamily(entry.key);
		else _AddPickedEffect(entry.effectId);
	} else if (pressed == VirtualKey::Down || pressed == VirtualKey::Up) {
		const int next = int(index) + (pressed == VirtualKey::Down ? 1 : -1);
		if (next >= 0 && next < int(_pickerRows.size())) _pickerRows[next].button.Focus(FocusState::Keyboard);
		args.Handled(true);
	} else if (pressed == VirtualKey::Right && entry.IsFamily()) {
		if (!entry.expanded) _ToggleEffectFamily(entry.key);
		else if (index + 1 < _pickerRows.size()) _pickerRows[index + 1].button.Focus(FocusState::Keyboard);
		args.Handled(true);
	} else if (pressed == VirtualKey::Left) {
		if (entry.IsFamily() && entry.expanded) _ToggleEffectFamily(entry.key);
		else for (const auto& row : _pickerRows)
			if (row.entry.key == entry.parent) { row.button.Focus(FocusState::Keyboard); break; }
		args.Handled(true);
	}
	// Space uses Button's normal press/release Click behavior, including families.
}

void ScalingModesPage::_UpdateEffectPickerIndex() {
	// Two narrow alphabet columns fit the preferred 800-DIP popup while retaining
	// 24-DIP click targets. Small monitor work areas use the grid flyout instead.
	const bool rail = _pickerIndexPane.ActualHeight() >= 14 * 24;
	_pickerIndexRail.Visibility(rail ? Visibility::Visible : Visibility::Collapsed);
	_pickerIndexButton.Visibility(rail ? Visibility::Collapsed : Visibility::Visible);
	bool any = false;
	for (int i = 0; i < 27; ++i) {
		const bool enabled = !_pickerLetterTargets[i].empty();
		_pickerRailLetters[i].IsEnabled(enabled);
		_pickerGridLetters[i].IsEnabled(enabled);
		any |= enabled;
	}
	_pickerIndexButton.IsEnabled(any);
}

void ScalingModesPage::_JumpEffectPickerLetter(int letter) {
	if (letter < 0 || letter >= 27 || _pickerLetterTargets[letter].empty()) return;
	if (_pickerIndexOpen) {
		_pickerPendingLetter = letter;
		_pickerIndexFlyout.Hide();
		return;
	}
	_pickerRoot.UpdateLayout();
	for (const auto& row : _pickerRows) {
		if (row.entry.key != _pickerLetterTargets[letter]) continue;
		row.button.Focus(FocusState::Keyboard);
		const double top = row.button.TransformToVisual(_pickerResults).TransformPoint({0, 0}).Y;
		_pickerListScroll.ChangeView(nullptr, top, nullptr, true);
		break;
	}
}

void ScalingModesPage::_AddPickedEffect(std::wstring_view id) {
	if (!_pickerMode) return;
	const auto entry = std::ranges::find(_pickerEntries, id, [](const auto& effect) { return std::wstring_view(effect.id); });
	if (entry == _pickerEntries.end()) return;
	const auto mode = _pickerMode;
	const hstring stableId(entry->id);
	if (!get_self<ScalingModeItem>(mode)->EffectAddProblem(stableId).empty()) {
		_ShowEffectPickerDetails(L"effect:" + entry->id);
		return;
	}
	_pickerMode = nullptr;
	_effectPicker.Hide();
	get_self<ScalingModeItem>(mode)->AddEffect(stableId);
}

} // namespace winrt::Magpie::implementation
